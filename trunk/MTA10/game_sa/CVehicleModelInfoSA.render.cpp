/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        game_sa/CVehicleModelInfoSA.render.cpp
*  PURPOSE:     Vehicle model info rendering module
*  DEVELOPERS:  Martin Turski <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#include <StdInc.h>
#include "gamesa_renderware.h"

#define VEHICLE_RENDER_DISABLE_LOD
//#define VEHICLE_RENDER_HIGH_QUALITY       // GTA:SA is not optimized for high quality atomic z-buffering, so leave this disabled.
                                            // If you find a way to fix this, thank you.

#define VAR_CameraPositionVector                0x00C88050

/*=========================================================
    GetObjectDistanceToCameraSq

    Arguments:
        obj - RenderWare object to specify distance to camera to
    Purpose:
        Returns the distance to camera of the given object.
=========================================================*/
inline static float GetObjectDistanceToCameraSq( RwObject *obj )
{
    return ( obj->m_parent->GetLTM().pos - **(CVector**)0x00C88050 ).LengthSquared();
}

/*=========================================================
    GetObjectOffsetRotation

    Arguments:
        obj - RenderWare object to specify special rotation of
    Purpose:
        Returns the special render euler radian rotation value
        between camera position and RenderWare object modelling
        offset [sic].
=========================================================*/
inline static float GetObjectOffsetRotation( RwObject *obj )
{
    CVector dist = **(CVector**)0x00C88050 - obj->m_parent->GetModelling().pos;
    return atan2( dist.fZ, sqrt( dist.fX * dist.fX + dist.fY + dist.fY ) );
}

/*=========================================================
    CacheVehicleRenderCameraSettings

    Arguments:
        obj - vehicle rendering object
    Purpose:
        Sets up cached camera details for the rendering of
        the given vehicle clump.
    Binary offsets:
        (1.0 US and 1.0 EU): 0x00733160
=========================================================*/
#ifndef VEHICLE_RENDER_HIGH_QUALITY
static float cameraRenderDistanceSq = 0;
static float cameraRenderAngleRadians = 0;
#endif //VEHICLE_RENDER_HIGH_QUALITY

void __cdecl CacheVehicleRenderCameraSettings( RwObject *obj )
{
#ifndef VEHICLE_RENDER_HIGH_QUALITY
    if ( obj->m_type != RW_CLUMP )
        return;

    cameraRenderDistanceSq = GetObjectDistanceToCameraSq( obj );
    cameraRenderAngleRadians = GetObjectOffsetRotation( obj );
#endif //VEHICLE_RENDER_HIGH_QUALITY
}

/*=========================================================
    GetRenderObjectCameraDistanceSq

    Arguments:
        obj - RenderWare object to specify distance to camera to
    Purpose:
        Common routine for the vehicle atomics to retrieve distance
        to camera. Returns either a cached value or the real
        distance depending on configuration.
=========================================================*/
static inline float GetRenderObjectCameraDistanceSq( RpAtomic *obj )
{
#ifdef VEHICLE_RENDER_HIGH_QUALITY
    float radius = obj->bsphereWorld.radius;

    return GetObjectDistanceToCameraSq( obj ) - radius * radius;
#else
    return cameraRenderDistanceSq;
#endif //VEHICLE_RENDER_HIGH_QUALITY
}

/*=========================================================
    GetRenderObjectOffsetRotation

    Arguments:
        obj - RenderWare object to specify distance to camera to
    Purpose:
        Common routine for the vehicle atomics to retrieve a special
        camera rotation value. Returns either a cached value or the real
        distance depending on configuration.
=========================================================*/
static inline float GetRenderObjectOffsetRotation( RpAtomic *obj )
{
#ifdef VEHICLE_RENDER_HIGH_QUALITY
    return GetObjectOffsetRotation( obj );
#else
    return cameraRenderAngleRadians;
#endif //VEHICLE_RENDER_HIGH_QUALITY
}

/*=========================================================
    ClearVehicleRenderChains

    Purpose:
        Called by the render manager to clear previously
        listed-for-rendering vehicle atomics.
    Binary offsets:
        (1.0 US and 1.0 EU): 0x00734530
=========================================================*/
// Render chains for vehicles only
static atomicRenderChain_t *const vehicleRenderChains = (atomicRenderChain_t*)0x00C88070;

void __cdecl ClearVehicleRenderChains( void )
{
    vehicleRenderChains->Clear();
}

/*=========================================================
    ExecuteVehicleRenderChains

    Purpose:
        Renders all atomics which were listed for delayed
        rendering (usually alpha/transparent/opaque).
=========================================================*/
void __cdecl ExecuteVehicleRenderChains( void )
{
    vehicleRenderChains->Execute();
}

// LOD distances for models, squared (measured against camera distance from atomic, 0x00733160 is rendering wrapper)
static float trainLODDistance = 45000;          // ~212 units
static float boatLODDistance = 9800;            // ~100 units
static float heliLODDistance = 9800;            // ~100 units, same as boat
static float heliRotorRenderDistance = 45000;   // ~212 units, same as train
static float planeLODDistance = 45000;          // ~212 units, same as train
static float vehicleLODDistance = 9800;         // ~100 units, same as boat
static float highDetailDistance = 4050;         // ~64 units

/*=========================================================
    _renderAtomicCommon

    Arguments:
        atom - the z-layered atomic to render
    Purpose:
        Composite callback to render atomics in the z-layered
        render chains.
    Binary offsets:
        (1.0 US and 1.0 EU): 0x00732B30
=========================================================*/
static void __cdecl _renderAtomicCommon( RpAtomic *atom )
{
    RpAtomicRender( atom );
}

/*=========================================================
    RwAtomicGetVisibilityCalculation

    Arguments:
        atomic - rendering object to calculate for
    Purpose:
        Returns a special distance calculated from the actual atomic
        in relation to the model rotation.
=========================================================*/
inline static float RwAtomicGetVisibilityCalculation( RpAtomic *atomic )
{
    return RwMatrixUnknown( atomic->m_parent->GetLTM(), atomic->m_clump->m_parent->GetLTM(), atomic->m_componentFlags );
}

/*=========================================================
    RwAtomicIsVisibleBasic

    Arguments:
        atomic - rendering object to check visibility of
        calc - special rendering distance
    Purpose:
        Returns whether the atomic is visible. It is directly
        used by non-translucent plane atomics.
=========================================================*/
inline static bool RwAtomicIsVisibleBasic( RpAtomic *atomic, float calc )
{
    return !( atomic->m_componentFlags & 0x0400 ) || GetRenderObjectOffsetRotation( atomic ) < 0.2f || calc > 0.0f;
}

/*=========================================================
    RwAtomicIsVisible

    Arguments:
        atomic - rendering object to check visibility of
        calc - special rendering distance
    Purpose:
        Returns whether the atomic is visible. This function does
        additional checks to RwAtomicIsVisibleBasic.
=========================================================*/
inline static bool RwAtomicIsVisible( RpAtomic *atomic, float calc, float camDistanceSq )
{
    return RwAtomicIsVisibleBasic( atomic, calc ) || !( atomic->m_componentFlags & 0x80 ) && ( calc * calc ) >= camDistanceSq * 0.1f;
}

/*=========================================================
    RwAtomicHandleHighDetail

    Arguments:
        atomic - rendering object to set flags for
    Purpose:
        If the atomic enters a very close distance (~64 units), a
        special flag is set to it. I assume it increases the rendering
        quality.
=========================================================*/
inline static void RwAtomicHandleHighDetail( RpAtomic *atomic, float camDistanceSq )
{
    if ( camDistanceSq < highDetailDistance )
        atomic->m_componentFlags &= ~0x2000;
    else
        atomic->m_componentFlags |= 0x2000;
}

/*=========================================================
    RwAtomicRenderTrainLOD

    Arguments:
        atomic - object being rendered
    Purpose:
        Renders train LOD atomics.
    Binary offsets:
        (1.0 US and 1.0 EU): 0x00732820
=========================================================*/
static RpAtomic* RwAtomicRenderTrainLOD( RpAtomic *atomic )
{
#ifndef VEHICLE_RENDER_DISABLE_LOD
    if ( GetRenderObjectCameraDistanceSq( atomic ) <= trainLODDistance )
        return atomic;

    RpAtomicRender( atomic );
#endif
    return atomic;
}

/*=========================================================
    RwAtomicRenderTranslucentTrain

    Arguments:
        atomic - object being rendered
    Purpose:
        Renders transparent train atomics.
    Binary offsets:
        (1.0 US and 1.0 EU): 0x00734240
=========================================================*/
static RpAtomic* RwAtomicRenderTranslucentTrain( RpAtomic *atomic )
{
    float camDistanceSq = GetRenderObjectCameraDistanceSq( atomic );

#ifndef VEHICLE_RENDER_DISABLE_LOD
    if ( camDistanceSq >= trainLODDistance )
        return atomic;
#endif

    RwAtomicHandleHighDetail( atomic, camDistanceSq );

    float calc = RwAtomicGetVisibilityCalculation( atomic );

    // Lol, serious checking going on here!
    if ( !( camDistanceSq <= *(float*)0x00C8802C || RwAtomicIsVisible( atomic, calc, camDistanceSq ) ) )
        return atomic;

    // Set up rendering
    atomicRenderChain_t::depthLevel level;
    level.callback = _renderAtomicCommon;
    level.atomic = atomic;
    level.distance = camDistanceSq;

    if ( !(atomic->m_componentFlags & 0x40) )
        level.distance += calc;

    if ( !vehicleRenderChains->PushRender( &level ) )
        RpAtomicRender( atomic );

    return atomic;
}

/*=========================================================
    RwAtomicRenderTrain

    Arguments:
        atomic - object being rendered
    Purpose:
        Renders train atomics.
    Binary offsets:
        (1.0 US and 1.0 EU): 0x00733330
=========================================================*/
static RpAtomic* RwAtomicRenderTrain( RpAtomic *atomic )
{
    float camDistanceSq = GetRenderObjectCameraDistanceSq( atomic );

#ifndef VEHICLE_RENDER_DISABLE_LOD
    if ( camDistanceSq >= trainLODDistance )
        return atomic;
#endif

    RwAtomicHandleHighDetail( atomic, camDistanceSq );

    // Lol, serious checking going on here!
    if ( camDistanceSq <= *(float*)0x00C8802C || RwAtomicIsVisible( atomic, RwAtomicGetVisibilityCalculation( atomic ), camDistanceSq ) )
        RpAtomicRender( atomic );

    return atomic;
}

/*=========================================================
    RwAtomicRenderLODAlpha

    Arguments:
        atomic - object being rendered
    Purpose:
        Renders atomics while accounting for clump alpha.
=========================================================*/
inline static void RwAtomicRenderLODAlpha( RpAtomic *atomic )
{
    atomic->m_componentFlags |= 0x2000;

    if ( atomic->m_clump->m_alpha == 0xFF )
        RpAtomicRenderAlpha( atomic, atomic->m_clump->m_alpha );
    else
        RpAtomicRender( atomic );
}

/*=========================================================
    RwAtomicRenderBoatLOD

    Arguments:
        atomic - object being rendered
    Purpose:
        Renders LOD boat atomics with clump alpha.
    Binary offsets:
        (1.0 US and 1.0 EU): 0x007334F0
=========================================================*/
static RpAtomic* RwAtomicRenderBoatLOD( RpAtomic *atomic )
{
#ifndef VEHICLE_RENDER_DISABLE_LOD
    float camDistanceSq = GetRenderObjectCameraDistanceSq( atomic );

    if ( camDistance <= boatLODDistance )
        return atomic;

    RwAtomicRenderLODAlpha( atomic );
#endif
    return atomic;
}

/*=========================================================
    RwAtomicRenderBoat

    Arguments:
        atomic - object being rendered
    Purpose:
        Renders boat atomics.
    Binary offsets:
        (1.0 US and 1.0 EU): 0x00733550
=========================================================*/
static RpAtomic* RwAtomicRenderBoat( RpAtomic *atomic )
{
    float camDistanceSq = GetRenderObjectCameraDistanceSq( atomic );

#ifndef VEHICLE_RENDER_DISABLE_LOD
    if ( camDistanceSq >= boatLODDistance )
        return atomic;
#endif

    RwAtomicHandleHighDetail( atomic, camDistanceSq );

    RpAtomicRender( atomic );
    return atomic;
}

/*=========================================================
    RwAtomicRenderTranslucentBoat

    Arguments:
        atomic - object being rendered
    Purpose:
        Renders transparent boat atomics with a special render chain.
    Binary offsets:
        (1.0 US and 1.0 EU): 0x007344A0
=========================================================*/
static atomicRenderChain_t *const boatRenderChain = (atomicRenderChain_t*)0x00C880C8;

static RpAtomic* RwAtomicRenderTranslucentBoat( RpAtomic *atomic )
{
    float camDistanceSq = GetRenderObjectCameraDistanceSq( atomic );

#ifndef VEHICLE_RENDER_DISABLE_LOD
    if ( camDistanceSq >= boatLODDistance )
        return atomic;
#endif

    RwAtomicHandleHighDetail( atomic, camDistanceSq );

    if ( atomic->m_componentFlags & 0x40 )
    {
        atomicRenderChain_t::depthLevel level;
        level.callback = _renderAtomicCommon;
        level.atomic = atomic;
        level.distance = camDistanceSq;

        if ( boatRenderChain->PushRender( &level ) )
            return atomic;
    }
    
    RpAtomicRender( atomic );
    return atomic;
}

/*=========================================================
    RwAtomicRenderHeliLOD

    Arguments:
        atomic - object being rendered
    Purpose:
        Renders LOD helicopter atomics with clump alpha.
    Binary offsets:
        (1.0 US and 1.0 EU): 0x007331E0
=========================================================*/
static RpAtomic* RwAtomicRenderHeliLOD( RpAtomic *atomic )
{
#ifndef VEHICLE_RENDER_DISABLE_LOD
    float camDistanceSq = GetRenderObjectCameraDistanceSq( atomic );

    if ( camDistanceSq <= heliLODDistance )
        return atomic;

    RwAtomicRenderLODAlpha( atomic );
#endif
    return atomic;
}

/*=========================================================
    RwAtomicRenderHeli

    Arguments:
        atomic - object being rendered
    Purpose:
        Renders helicopter atomics.
    Binary offsets:
        (1.0 US and 1.0 EU): 0x00733240
=========================================================*/
static RpAtomic* RwAtomicRenderHeli( RpAtomic *atomic )
{
    float camDistanceSq = GetRenderObjectCameraDistanceSq( atomic );

#ifndef VEHICLE_RENDER_DISABLE_LOD
    if ( camDistanceSq >= heliLODDistance )
        return atomic;
#endif

    RwAtomicHandleHighDetail( atomic, camDistanceSq );

    // Lol, serious checking going on here!
    if ( camDistanceSq <= *(float*)0x00C8802C || RwAtomicIsVisible( atomic, RwAtomicGetVisibilityCalculation( atomic ), camDistanceSq ) )
        RpAtomicRender( atomic );

    return atomic;
}

/*=========================================================
    RwAtomicRenderTranslucentCommon

    Arguments:
        atomic - object being rendered
    Purpose:
        Common routine for rendering translucent atomics.
=========================================================*/
inline static void RwAtomicRenderTranslucentCommon( RpAtomic *atomic, float camDistanceSq )
{
    RwAtomicHandleHighDetail( atomic, camDistanceSq );

    float calc = RwAtomicGetVisibilityCalculation( atomic );

    // Lol, serious checking going on here!
    if ( !( camDistanceSq <= *(float*)0x00C8802C || RwAtomicIsVisible( atomic, calc, camDistanceSq ) ) )
        return;

    // Set up rendering
    atomicRenderChain_t::depthLevel level;
    level.callback = _renderAtomicCommon;
    level.atomic = atomic;
    level.distance = camDistanceSq;

    if ( atomic->m_componentFlags & 0x40 )
        level.distance -= 0.0001f;
    else
        level.distance += calc;

    if ( !vehicleRenderChains->PushRender( &level ) )
        RpAtomicRender( atomic );
}

/*=========================================================
    RwAtomicRenderTranslucentHeli

    Arguments:
        atomic - object being rendered
    Purpose:
        Renders transparent helicopter atomics.
    Binary offsets:
        (1.0 US and 1.0 EU): 0x00733F80
=========================================================*/
static RpAtomic* RwAtomicRenderTranslucentHeli( RpAtomic *atomic )
{
    float camDistanceSq = GetRenderObjectCameraDistanceSq( atomic );

#ifndef VEHICLE_RENDER_DISABLE_LOD
    if ( camDistanceSq >= heliLODDistance )
        return atomic;
#endif

    RwAtomicRenderTranslucentCommon( atomic, camDistanceSq );
    return atomic;
}

/*=========================================================
    RwAtomicRenderHeliMovingRotor

    Arguments:
        atomic - object being rendered
    Purpose:
        Renders the 'moving_rotor' atomic of helicopter models.
    Binary offsets:
        (1.0 US and 1.0 EU): 0x007340B0
=========================================================*/
static RpAtomic* RwAtomicRenderHeliMovingRotor( RpAtomic *atomic )
{
    float camDistanceSq = GetRenderObjectCameraDistanceSq( atomic );

#ifndef VEHICLE_RENDER_DISABLE_LOD
    if ( camDistanceSq >= heliRotorRenderDistance )
        return atomic;
#endif

    const RwMatrix& ltm = atomic->m_parent->GetLTM();
    CVector vecRotor = ltm.pos - **(CVector**)VAR_CameraPositionVector;

    // Calculate rotor details
    atomicRenderChain_t::depthLevel level;
    level.distance = vecRotor.DotProduct( ltm.up ) * 20 + camDistanceSq;
    level.callback = _renderAtomicCommon;
    level.atomic = atomic;

    if ( !vehicleRenderChains->PushRender( &level ) )
        RpAtomicRender( atomic );

    return atomic;
}

/*=========================================================
    RwAtomicRenderHeliMovingRotor2

    Arguments:
        atomic - object being rendered
    Purpose:
        Renders the 'moving_rotor2' atomic of helicopter models.
    Binary offsets:
        (1.0 US and 1.0 EU): 0x00734170
=========================================================*/
static RpAtomic* RwAtomicRenderHeliMovingRotor2( RpAtomic *atomic )
{
    float camDistanceSq = GetRenderObjectCameraDistanceSq( atomic );

#ifndef VEHICLE_RENDER_DISABLE_LOD
    if ( camDistanceSq >= heliRotorRenderDistance )
        return atomic;
#endif

    const RwMatrix& ltm = atomic->m_parent->GetLTM();
    CVector vecRotor = ltm.pos - **(CVector**)VAR_CameraPositionVector;

    // Lulz, heavy math, much assembly, small C++ code
    atomicRenderChain_t::depthLevel level;
    level.distance = camDistanceSq - vecRotor.DotProduct( ltm.right ) - vecRotor.DotProduct( ltm.at );
    level.callback = _renderAtomicCommon;
    level.atomic = atomic;

    if ( !vehicleRenderChains->PushRender( &level ) )
        RpAtomicRender( atomic );

    return atomic;
}

/*=========================================================
    RwAtomicRenderPlaneLOD

    Arguments:
        atomic - object being rendered
    Purpose:
        Renders LOD plane atomics.
    Note:
        The GTA:SA function equals RwAtomicRenderTrainLOD.
=========================================================*/
static RpAtomic* RwAtomicRenderPlaneLOD( RpAtomic *atomic )
{
#ifndef VEHICLE_RENDER_DISABLE_LOD
    float camDistanceSq = GetRenderObjectCameraDistanceSq( atomic );

    if ( camDistanceSq <= planeLODDistance )
        return atomic;

    RpAtomicRender( atomic );
#endif
    return atomic;
}

/*=========================================================
    RwAtomicRenderPlane

    Arguments:
        atomic - object being rendered
    Purpose:
        Renders plane atomics.
    Binary offsets:
        (1.0 US and 1.0 EU): 0x00733420
=========================================================*/
static RpAtomic* RwAtomicRenderPlane( RpAtomic *atomic )
{
    float camDistanceSq = GetRenderObjectCameraDistanceSq( atomic );

#ifndef VEHICLE_RENDER_DISABLE_LOD
    if ( camDistanceSq >= planeLODDistance )
        return atomic;
#endif

    RwAtomicHandleHighDetail( atomic, camDistanceSq );

    // Lol, serious checking going on here!
    if ( camDistanceSq <= *(float*)0x00C88028 || RwAtomicIsVisibleBasic( atomic, RwAtomicGetVisibilityCalculation( atomic ) ) )
        RpAtomicRender( atomic );

    return atomic;
}

/*=========================================================
    RwAtomicRenderTranslucentPlane

    Arguments:
        atomic - object being rendered
    Purpose:
        Renders translucent plane atomics.
    Binary offsets:
        (1.0 US and 1.0 EU): 0x00734370
=========================================================*/
static RpAtomic* RwAtomicRenderTranslucentPlane( RpAtomic *atomic )
{
    float camDistanceSq = GetRenderObjectCameraDistanceSq( atomic );

#ifndef VEHICLE_RENDER_DISABLE_LOD
    if ( camDistanceSq >= heliLODDistance )
        return atomic;
#endif

    RwAtomicHandleHighDetail( atomic, camDistanceSq );

    float calc = RwAtomicGetVisibilityCalculation( atomic );

    // Lol, serious checking going on here!
    if ( !( camDistanceSq <= *(float*)0x00C88028 || RwAtomicIsVisible( atomic, calc, camDistanceSq ) ) )
        return atomic;

    // Set up rendering
    atomicRenderChain_t::depthLevel level;
    level.callback = _renderAtomicCommon;
    level.atomic = atomic;
    level.distance = camDistanceSq;

    if ( atomic->m_componentFlags & 0x40 )
        level.distance -= 0.0001f;
    else
        level.distance += calc;

    if ( !vehicleRenderChains->PushRender( &level ) )
        RpAtomicRender( atomic );
    
    return atomic;
}

/*=========================================================
    RwAtomicRenderTranslucentDefaultVehicle

    Arguments:
        atomic - object being rendered
    Purpose:
        Renders translucent any vehicle atomics.
    Note:
        The GTA:SA function equals RwAtomicRenderTranslucentHeli.
=========================================================*/
static RpAtomic* RwAtomicRenderTranslucentDefaultVehicle( RpAtomic *atomic )
{
    float camDistanceSq = GetRenderObjectCameraDistanceSq( atomic );

#ifndef VEHICLE_RENDER_DISABLE_LOD
    if ( camDistanceSq >= vehicleLODDistance )
        return atomic;
#endif

    RwAtomicRenderTranslucentCommon( atomic, camDistanceSq );
    return atomic;
}

/*=========================================================
    RwAtomicRenderDefaultVehicle

    Arguments:
        atomic - object being rendered
    Purpose:
        Renders any vehicle atomics.
    Note:
        The GTA:SA function equals RwAtomicRenderHeli.
=========================================================*/
static RpAtomic* RwAtomicRenderDefaultVehicle( RpAtomic *atomic )    // actually equals heli render
{
    float camDistanceSq = GetRenderObjectCameraDistanceSq( atomic );

#ifndef VEHICLE_RENDER_DISABLE_LOD
    if ( camDistanceSq >= vehicleLODDistance )
        return atomic;
#endif

    RwAtomicHandleHighDetail( atomic, camDistanceSq );

    // Lol, serious checking going on here!
    if ( camDistanceSq <= *(float*)0x00C8802C || RwAtomicIsVisible( atomic, RwAtomicGetVisibilityCalculation( atomic ), camDistanceSq ) )
        RpAtomicRender( atomic );

    return atomic;
}

/*=========================================================
    RwAtomicRenderDefaultVehicleLOD

    Arguments:
        atomic - object being rendered
    Purpose:
        Renders any vehicle LOD atomics.
    Note:
        The GTA:SA function equals RwAtomicRenderHeliLOD.
=========================================================*/
static RpAtomic* RwAtomicRenderDefaultVehicleLOD( RpAtomic *atomic )
{
#ifndef VEHICLE_RENDER_DISABLE_LOD
    float camDistanceSq = GetRenderObjectCameraDistanceSq( atomic );

    if ( camDistanceSq <= vehicleLODDistance )
        return atomic;

    RwAtomicRenderLODAlpha( atomic );
#endif
    return atomic;
}

/*=========================================================
    RwAtomicSetupVehicleDamaged

    Arguments:
        child - atomic which shall be checked for damage
                component status
    Purpose:
        Sets the atomic up for the GTA:SA damage system.
    Binary offsets:
        (1.0 US and 1.0 EU): 0x004C7720
=========================================================*/
static bool RwAtomicSetupVehicleDamaged( RpAtomic *child )
{
    if ( strstr(child->m_parent->m_nodeName, "_dam") )
    {
        child->m_flags = 0;

        child->m_componentFlags = 2;
        return true;
    }

    if ( strstr(child->m_parent->m_nodeName, "_ok") )
    {
        child->m_componentFlags = 1;
        return true;
    }

    return true;
}

/*=========================================================
    RwAtomicRegisterTrain

    Arguments:
        child - atomic to set up rendering for
    Purpose:
        Registers the appropriate rendering callback for the
        atomic.
    Binary offsets:
        (1.0 US and 1.0 EU): 0x004C7AA0
=========================================================*/
static bool RwAtomicRegisterTrain( RpAtomic *child, int )
{
    if ( strstr( child->m_parent->m_nodeName, "_vlo" ) )
    {
        child->SetRenderCallback( RwAtomicRenderTrainLOD );
        return true;
    }
    else if ( child->m_geometry->IsAlpha() )
        child->SetRenderCallback( RwAtomicRenderTranslucentTrain ); // translucent polys need second render pass
    else
        child->SetRenderCallback( RwAtomicRenderTrain );

    RwAtomicSetupVehicleDamaged( child );
    return true;
}

/*=========================================================
    RwAtomicRegisterBoat

    Arguments:
        child - atomic to set up rendering for
    Purpose:
        Registers the appropriate rendering callback for the
        atomic.
    Binary offsets:
        (1.0 US and 1.0 EU): 0x004C79A0
=========================================================*/
static bool RwAtomicRegisterBoat( RpAtomic *child, int )
{
    if ( strcmp( child->m_parent->m_nodeName, "boat_hi" ) == 0 )
        child->SetRenderCallback( RwAtomicRenderBoat );         // boat_hi does not support alpha?
    else if ( strstr( child->m_parent->m_nodeName, "_vlo" ) )
        child->SetRenderCallback( RwAtomicRenderBoatLOD );
    else if ( child->m_geometry->IsAlpha() )
        child->SetRenderCallback( RwAtomicRenderTranslucentBoat );
    else
        child->SetRenderCallback( RwAtomicRenderBoat );

    RwAtomicSetupVehicleDamaged( child );
    return true;
}

/*=========================================================
    RwAtomicRegisterHeli

    Arguments:
        child - atomic to set up rendering for
    Purpose:
        Registers the appropriate rendering callback for the
        atomic.
    Binary offsets:
        (1.0 US and 1.0 EU): 0x004C7870
=========================================================*/
static bool RwAtomicRegisterHeli( RpAtomic *child, int )
{
    if ( strcmp( child->m_parent->m_nodeName, "moving_rotor" ) == 0 )
        child->SetRenderCallback( RwAtomicRenderHeliMovingRotor );
    else if ( strcmp( child->m_parent->m_nodeName, "moving_rotor2" ) == 0 )
        child->SetRenderCallback( RwAtomicRenderHeliMovingRotor2 );
    else if ( strstr( child->m_parent->m_nodeName, "_vlo" ) )
        child->SetRenderCallback( RwAtomicRenderHeliLOD );
    else if ( child->m_geometry->IsAlpha() || strncmp( child->m_parent->m_nodeName, "windscreen", 10) == 0 )
        child->SetRenderCallback( RwAtomicRenderTranslucentHeli );
    else
        child->SetRenderCallback( RwAtomicRenderHeli );

    RwAtomicSetupVehicleDamaged( child );
    return true;
}

/*=========================================================
    RwAtomicRegisterPlane

    Arguments:
        child - atomic to set up rendering for
    Purpose:
        Registers the appropriate rendering callback for the
        atomic.
    Binary offsets:
        (1.0 US and 1.0 EU): 0x004C7930
=========================================================*/
static bool RwAtomicRegisterPlane( RpAtomic *child, int )
{
    if ( strstr( child->m_parent->m_nodeName, "_vlo" ) )
    {
        child->SetRenderCallback( RwAtomicRenderPlaneLOD );
        return true;
    }
    else if ( child->m_geometry->IsAlpha() )
        child->SetRenderCallback( RwAtomicRenderTranslucentPlane );
    else
        child->SetRenderCallback( RwAtomicRenderPlane );

    RwAtomicSetupVehicleDamaged( child );
    return true;
}

/*=========================================================
    RwAtomicRegisterDefaultVehicle

    Arguments:
        child - atomic to set up rendering for
    Purpose:
        Registers the appropriate rendering callback for the
        atomic.
    Binary offsets:
        (1.0 US and 1.0 EU): 0x004C77E0
=========================================================*/
static bool RwAtomicRegisterDefaultVehicle( RpAtomic *child, int )
{
    if ( strstr( child->m_parent->m_nodeName, "_vlo" ) )
        child->SetRenderCallback( RwAtomicRenderDefaultVehicleLOD );
    else if ( child->m_geometry->IsAlpha() || strnicmp( child->m_parent->m_nodeName, "windscreen", 10 ) == 0 )
        child->SetRenderCallback( RwAtomicRenderTranslucentDefaultVehicle );
    else
        child->SetRenderCallback( RwAtomicRenderDefaultVehicle );

    RwAtomicSetupVehicleDamaged( child );
    return true;
}

/*=========================================================
    CVehicleModelInfoSAInterface::RegisterRenderCallbacks

    Purpose:
        Scans the clump according to vehicle type and applies
        attributes such as rendering callback and damage
        component flags.
    Binary offsets:
        (1.0 US and 1.0 EU): 0x004C7B10
=========================================================*/
void CVehicleModelInfoSAInterface::RegisterRenderCallbacks( void )
{
    switch( m_vehicleType )
    {
    case VEHICLE_TRAIN:
        GetRwObject()->ForAllAtomics( RwAtomicRegisterTrain, 0 );
        return;
    case VEHICLE_PLANE:
    case VEHICLE_FAKEPLANE:
        GetRwObject()->ForAllAtomics( RwAtomicRegisterPlane, 0 );
        return;
    case VEHICLE_BOAT:
        GetRwObject()->ForAllAtomics( RwAtomicRegisterBoat, 0 );
        return;
    case VEHICLE_HELI:
        GetRwObject()->ForAllAtomics( RwAtomicRegisterHeli, 0 );
        return;
    }

    GetRwObject()->ForAllAtomics( RwAtomicRegisterDefaultVehicle, 0 );
}

/*=========================================================
    CVehicleModelInfoSAInterface::SetRenderColor

    Arguments:
        color[1-4] - vehicle color indexes
    Purpose:
        Sets the vehicle render colors which will be applied
        be applied to materials.
    Binary offsets:
        (1.0 US and 1.0 EU): 0x004C84B0
=========================================================*/
static unsigned char _vehColor1, _vehColor2, _vehColor3, _vehColor4;

void CVehicleModelInfoSAInterface::SetRenderColor( unsigned char color1, unsigned char color2, unsigned char color3, unsigned char color4 )
{
    // Store the color values
    _vehColor1 = color1;
    _vehColor2 = color2;
    _vehColor3 = color3;
    _vehColor4 = color4;

    // Also store it in the vehicle model info
    m_color1 = color1;
    m_color2 = color2;
    m_color3 = color3;
    m_color4 = color4;
}

/*=========================================================
    SetVehicleColorFlags

    Arguments:
        vehicle - vehicle entity interface to set bools for
    Purpose:
        Caches color boolean flags for the given vehicle
        interface.
    Binary offsets:
        (1.0 US and 1.0 EU): 0x004C8C90
=========================================================*/
bool vehColorFlags[4];
static CVehicleSA *gameVehicle;

void __cdecl SetVehicleColorFlags( CVehicleSAInterface *veh )
{
    // Store render flags
    vehColorFlags[1] = ( veh->m_colorFlags > 1 ) & 1;
    vehColorFlags[0] = ( veh->m_colorFlags ) & 1;
    vehColorFlags[3] = ( veh->m_colorFlags > 2 ) & 1;
    vehColorFlags[2] = ( veh->m_colorFlags > 3 ) & 1;
}

/*=========================================================
    RpGeometryMaterialSetupColor

    Arguments:
        mat - material with vehicle color information of
              special component
        storage - pointer to an array which can store
                  material color information
    Purpose:
        Stores color information of the materials of said
        geometry. The model colors may be restored from
        the same array later.
    Binary offsets:
        (1.0 US and 1.0 EU): 0x004C83B0
=========================================================*/
struct _colorTextureStorage
{
    union
    {
        // Has to match this size!
        struct
        {
            DWORD*      _matUnk;
            DWORD       unk;
        };

        // User defines
        struct
        {
            RwColor*    matColor;
            RwColor     color;
        };
        struct
        {
            RwTexture** matTexture;
            RwTexture*  texture;
        };
        struct
        {
            float*      matAmbient;
            float       ambient;
        };
    };
};

static inline void _StoreColorInfo( _colorTextureStorage **storage, RpMaterial *mat )
{
    // Pop an entry from the array
    _colorTextureStorage *entry = (*storage)++;
    entry->matColor = &mat->m_color;
    entry->color = mat->m_color;
}

static inline void _StoreTextureInfo( _colorTextureStorage **storage, RpMaterial *mat )
{
    // Store the original texture information
    _colorTextureStorage *entry = (*storage)++;
    entry->matTexture = &mat->m_texture;
    entry->texture = mat->m_texture;
}

static inline void _StoreAmbientInfo( _colorTextureStorage **storage, RpMaterial *mat )
{
    // Store the ambient information
    _colorTextureStorage *entry = (*storage)++;
    entry->matAmbient = &mat->m_lighting.ambient;
    entry->ambient = mat->m_lighting.ambient;
}

static bool RpGeometryMaterialSetupColor( RpMaterial *mat, _colorTextureStorage **storage )
{
    if ( mat->m_color.a == 255 )
        return true;

    // Pop an entry from the array
    _StoreColorInfo( storage, mat );

    // Set default color to black
    mat->m_color = 0;
    return true;
}

/*=========================================================
    RpGeometryMaterialApplyVehicleColor

    Arguments:
        mat - material with vehicle color + texture information
        storage - pointer to an array which can store
                  material color and texture information
    Purpose:
        Sets the per-vehicle color to the material of the
        vehicle clump and replaces textures.
    Binary offsets:
        (1.0 US and 1.0 EU): 0x004C8220
=========================================================*/
static bool RpGeometryMaterialApplyVehicleColor( RpMaterial *mat, _colorTextureStorage **storage )
{
    unsigned int color = mat->m_color & 0x00FFFFFF;
    RwColor& colorRef = mat->m_color;
    
    RwTexture *matTex = mat->m_texture;

    if ( RwTexture *bodyTex = *(RwTexture**)0x00B4E47C )
    {
        // The_GTA: remember the '#' flag at VEHICLE.TXD RwRemapScan we removed?
        // We can replace this logic by checking that this Texture belongs to VEHICLE.TXD!
        // After all, the texture has been picked up by the model loader, or it would not be here.
        if ( matTex->txd == g_vehicleTxd )
        {
            // Store the original texture information
            _StoreTextureInfo( storage, mat );

            // Set it to the paintjob texture
            mat->m_texture = bodyTex;
        }
    }

    if ( matTex == *(RwTexture**)0x00B4E68C )
    {
        unsigned int id = 0xFFFFFFFF;

        if ( color == 0xAFFF )
            id = 0;
        else if ( color == 0xC8FF00 )
            id = 1;
        else if ( color == 0xFFB9 )
            id = 2;
        else if ( color == 0x3CFF )
            id = 3;

        // Replace the light color
        _StoreColorInfo( storage, mat );

        // Make it white (keep alpha)
        mat->m_color = color = mat->m_color | 0xFFFFFF;
        
        if ( id != 0xFFFFFFFF && vehColorFlags[id] )
        {
            // Replace the lights texture
            _StoreTextureInfo( storage, mat );
            _StoreAmbientInfo( storage, mat );

            mat->m_texture = *(RwTexture**)0x00B4E690;
            mat->m_lighting.ambient = 16;
            mat->m_lighting.specular = 0;
            mat->m_lighting.diffuse = 0;
        }
    }
    else
    {
        // Decide which game logic to use.
        if ( gameVehicle )
        {
            SColor useColor;
            SColor color1, color2, color3, color4;
            gameVehicle->GetColor( color1, color2, color3, color4, 0 );

            // Each material seems to be individually colored
            if ( color == 0xFF3C )
                useColor = color1;
            else if ( color == 0xAF00FF )
                useColor = color2;
            else if ( color == 0xFFFF00 )
                useColor = color3;
            else if ( color == 0xFF00FF )
                useColor = color4;
            else
                goto toReturn;

            // We replace the color of this material.
            _StoreColorInfo( storage, mat );

            // MTA extension: apply our custom vehicle color
            mat->m_color.r = useColor.R;
            mat->m_color.g = useColor.G;
            mat->m_color.b = useColor.B;
        }
        else
        {
            unsigned char id;

            // Each material seems to be individually colored
            if ( color == 0xFF3C )
                id = _vehColor1;
            else if ( color == 0xAF00FF )
                id = _vehColor2;
            else if ( color == 0xFFFF00 )
                id = _vehColor3;
            else if ( color == 0xFF00FF )
                id = _vehColor4;
            else
                goto toReturn;

            // We replace the color of this material.
            _StoreColorInfo( storage, mat );

            // Default logic; choose a palette color
            RwColor& paletteColor = ((RwColor*)0x00B4E480)[id];
            mat->m_color.r = paletteColor.r;
            mat->m_color.g = paletteColor.g;
            mat->m_color.b = paletteColor.b;
        }
    }

toReturn:
    return true;
}

/*=========================================================
    RpClumpAtomicSetupVehicleMaterials

    Arguments:
        atomic - RenderWare atomic to set up for rendering
    Purpose:
        Prepares the color and texture information of the
        given atomic.
    Binary offsets:
        (1.0 US and 1.0 EU): 0x004C83E0
=========================================================*/
static bool RpClumpAtomicSetupVehicleMaterials( RpAtomic *atomic, _colorTextureStorage **storage )
{
    if ( !atomic->IsVisible() )
        return true;

    if ( atomic->m_componentFlags & 0x1000 )
        atomic->m_geometry->ForAllMateria( RpGeometryMaterialSetupColor, storage );

    atomic->m_geometry->ForAllMateria( RpGeometryMaterialApplyVehicleColor, storage );
    return true;
}

/*=========================================================
    RpClumpSetupVehicleMaterials

    Arguments:
        clump - vehicle clump to setup for rendering
        veh - MTA vehicle pointer to retrieve data from (vehicle color, ...)
    Purpose:
        Prepares the materials of all atomics in this clump
        for rendering by setting their color and replacing
        textures.
    Binary offsets:
        (1.0 US and 1.0 EU): 0x004C8430
=========================================================*/
static _colorTextureStorage _vehColorStorage[2048]; //original 256

void __cdecl RpClumpSetupVehicleMaterials( RpClump *clump, CVehicleSA *veh )
{
    _colorTextureStorage *storage = _vehColorStorage;

    // Set the game vehicle for this setup process
    gameVehicle = veh;

    clump->ForAllAtomics( RpClumpAtomicSetupVehicleMaterials, &storage );
}

/*=========================================================
    RpClumpRestoreVehicleMaterials

    Arguments:
        clump - vehicle clump to leave rendering stage
    Purpose:
        Restores the clump data to the status prior to rendering
        (prior to RpClumpSetupVehicleMaterials).
    Binary offsets:
        (1.0 US and 1.0 EU): 0x004C8460
=========================================================*/
void __cdecl RpClumpRestoreVehicleMaterials( RpClump *clump )
{
    // We do not have to use the clump, since all pointers are pointing to it's data
    // Rockstar made use of a clever registering system.

    for ( _colorTextureStorage *entry = &_vehColorStorage[0]; entry->_matUnk != NULL; entry++ )
    {
        // Restore the value.
        *entry->_matUnk = entry->unk;

        // Bugfix: we should clean up after usage, so that the engine does not restore data it should keep alone.
        entry->_matUnk = NULL;
    }
}

void VehicleModelInfoRender_Init( void )
{
    // Apply our own rendering logic ;)
    HookInstall( 0x004C7B10, h_memFunc( &CVehicleModelInfoSAInterface::RegisterRenderCallbacks ), 5 );
}

void VehicleModelInfoRender_Shutdown( void )
{
}