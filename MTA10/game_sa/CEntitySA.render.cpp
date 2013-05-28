/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        game_sa/CEntitySA.render.cpp
*  PURPOSE:     Entity render utilities
*  DEVELOPERS:  Martin Turski <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#include "StdInc.h"

extern CBaseModelInfoSAInterface **ppModelInfo;

void __cdecl HOOK_InitRenderChains( void )
{
    new ((void*)0x00C88070) atomicRenderChain_t( 50 );      // vehicleRenderChains, orig 20
    new ((void*)0x00C880C8) atomicRenderChain_t( 50 );      // boatRenderChains, orig 20
    new ((void*)0x00C88120) entityRenderChain_t( 4000 );    // ???, orig 200
    new ((void*)0x00C88178) entityRenderChain_t( 3000 );    // ???, orig 100
    new ((void*)0x00C881D0) entityRenderChain_t( 1500 );    // ???, orig 50
    new ((void*)0x00C88224) pedRenderChain_t( 3000 );       // ???, orig 100
}

static bool __cdecl AreScreenEffectsEnabled( void )
{
    return !*(bool*)0x00C402B7 && ( *(bool*)0x00C402B8 || *(bool*)0x00C402B9 );
}

static void __cdecl SetupNightVisionLighting( void )
{
    // Check whether effects and night vision are enabled
    if ( !*(bool*)0x00C402B8 || *(bool*)0x00C402B7 )
        return;

    RwColorFloat color( 0, 1.0f, 0, 1.0f );

    (*(RpLight**)0x00C886E8)->SetColor( color );    // first light
    (*(RpLight**)0x00C886EC)->SetColor( color );    // second light
}

static void __cdecl SetupInfraRedLighting( void )
{
    // Check if effects and infra red are enabled
    if ( !*(bool*)0x00C402B9 || *(bool*)0x00C402B7 )
        return;

    RwColorFloat color( 0, 0, 1.0f, 1.0f );

    (*(RpLight**)0x00C886E8)->SetColor( color );    // first light
    (*(RpLight**)0x00C886EC)->SetColor( color );    // second light
}

float _tempDayNightBalance = 0;

static void __cdecl NormalizeDayNight( void )
{
    if ( *(bool*)0x00C402B9 )
        return;

    _tempDayNightBalance = *(float*)0x008D12C0;
    *(float*)0x008D12C0 = 1.0f;
}

static void __cdecl RestoreDayNight( void )
{
    if ( *(bool*)0x00C402B9 )
        return;

    *(float*)0x008D12C0 = _tempDayNightBalance;
}

void __cdecl RenderEntity( CEntitySAInterface *entity )
{
    // Do not render peds in the game world if they are inside a vehicle
    // (they need a special render pass to prevent drawing errors)
    if ( entity->m_type == ENTITY_TYPE_PED && ((CPedSAInterface*)entity)->IsDrivingVehicle() )
        return;

    unsigned char id = entity->SetupLighting();

    // This way we skip heap allocations
    StaticAllocator <RwRenderStateLock, 1> rsAlloc;
    RwRenderStateLock *alphaRef;

    // Grab the MTA entity
    CEntitySA *mtaEntity = pGame->GetPools()->GetEntity( entity );

    unsigned char alpha = 255;

    if ( mtaEntity )
    {
        // Perform alpha calculations
        if ( entity->m_type == ENTITY_TYPE_VEHICLE )
            alpha = ((CVehicleSA*)mtaEntity)->GetAlpha();
        else if ( entity->m_type == ENTITY_TYPE_OBJECT )
            alpha = ((CObjectSA*)mtaEntity)->GetAlpha();
        else if ( entity->m_type == ENTITY_TYPE_PED )
            alpha = ((CPedSA*)mtaEntity)->GetAlpha();
    }
    else if ( entity->m_rwObject && entity->m_rwObject->m_type == RW_CLUMP )    // the entity may not have a RenderWare object?!
    {
        // This value will be used for internal GTA:SA vehicles.
        // We might aswell use this alpha value of the clump.
        alpha = (unsigned char)((RpClump*)entity->m_rwObject)->m_alpha;
    }

    if ( alpha != 255 )
    {
        // This has to stay enabled anyway
        RwD3D9SetRenderState( D3DRS_ALPHABLENDENABLE, true );
        RwD3D9SetRenderState( D3DRS_ALPHAFUNC, D3DCMP_GREATER );
        RwD3D9SetRenderState( D3DRS_ALPHATESTENABLE, true );
        RwD3D9SetRenderState( D3DRS_ALPHAREF, 100 );

        // Ensure the RenderStates necessary for proper alpha blending
        alphaRef = new (rsAlloc.Allocate()) RwRenderStateLock( D3DRS_ALPHAREF, 0x00 );
        RwD3D9ApplyDeviceStates();
    }

    // Prepare entity for rendering/enter frame
    if ( entity->m_type == ENTITY_TYPE_VEHICLE )
    {
        // Set up global variables for fast rendering
        CacheVehicleRenderCameraSettings( alpha, entity->m_rwObject );

        // Clear previous vehicle atomics from render chains
        ClearVehicleRenderChains();

        CVehicleSAInterface *veh = (CVehicleSAInterface*)entity;

        // Render the passengers
        veh->RenderPassengers();

        // Prepare vehicle rendering (i.e. texture replacement)
        veh->SetupRender( (CVehicleSA*)mtaEntity );
    }
    else if ( !( entity->m_entityFlags & ENTITY_BACKFACECULL ) )
    {
        // Change texture stage
        (*ppRwInterface)->m_deviceCommand( (eRwDeviceCmd)20, 1 );
    }

    if ( AreScreenEffectsEnabled() )
    {
        if ( *(bool*)0x00C402B8 )
            SetupNightVisionLighting();

        if ( *(bool*)0x00C402B9 )
            SetupInfraRedLighting();

        NormalizeDayNight();

        entity->Render();

        RestoreDayNight();
    }
    else
        entity->Render();

    // Restore values and unset entity rendering status/leave frame
    if ( entity->m_type == ENTITY_TYPE_VEHICLE )
    {
        BOOL_FLAG( entity->m_entityFlags, ENTITY_RENDERING, true );

        // Render the delayed atomics
        ExecuteVehicleRenderChains( alpha );

        BOOL_FLAG( entity->m_entityFlags, ENTITY_RENDERING, false );

        CVehicleSAInterface *veh = (CVehicleSAInterface*)entity;

        // Leave rendering stage
        veh->LeaveRender();
    }   
    else if ( !( entity->m_entityFlags & ENTITY_BACKFACECULL ) )
    {
        // Set texture stage back to two
        (*ppRwInterface)->m_deviceCommand( (eRwDeviceCmd)20, 2 );
    }

    // Does the entity have alpha enabled?
    if ( alpha != 255 )
    {
        // Remove the RenderState locks
        alphaRef->~RwRenderStateLock();
        RwD3D9ApplyDeviceStates();
    }

    entity->RemoveLighting( id );
}

static float CalculateFadingAlpha( CBaseModelInfoSAInterface *info, const CEntitySAInterface *entity, float camDistanceSq )
{
    float sectorDivide = 20.0f;
    float distAway = info->pColModel->m_bounds.fRadius + *(float*)0x00B76848;
    float scaledLODDistance = info->fLodDistanceUnscaled * *(float*)0x00B6F118;

    float useDist = distAway;

    if ( scaledLODDistance < distAway )
        useDist = scaledLODDistance;

    if ( !entity->m_lod )
    {
        float useDist2 = info->fLodDistanceUnscaled;

        if ( info->fLodDistanceUnscaled > useDist )
            useDist2 = useDist;

        if ( useDist2 > 150.0f )
            sectorDivide = useDist2 * 0.06666667f + 10.0f;

        if ( entity->m_entityFlags & ENTITY_BIG )
            useDist *= *(float*)0x008CD804;
    }

    useDist += 20.0f;
    useDist -= camDistanceSq;
    useDist /= sectorDivide;

    if ( useDist < 0.0f )
        return 0;

    if ( useDist > 1.0f )
        useDist = 1;

    return useDist * info->ucAlpha;
}

float CEntitySAInterface::GetFadingAlpha( void ) const
{
    const CVector& pos = Placeable.GetPosition();

    float camDistance = ( pGame->GetCamera()->GetInterface()->Placeable.GetPosition() - pos ).Length();
    float camDistance2 = ( *pGame->GetCamera()->GetCam( pGame->GetCamera()->GetActiveCam() )->GetSource() - pos ).Length();
    float camDistance3 = ( *(CVector*)0x00B76870 - pos ).Length();

    unsigned char alpha1 = (unsigned char)CalculateFadingAlpha( ppModelInfo[m_model], this, camDistance );

    unsigned char alpha2 = ((unsigned char (__cdecl*)( CBaseModelInfoSAInterface*, const CEntitySAInterface*, float ))0x00732500)( ppModelInfo[m_model], this, camDistance );

    return alpha1;
}

inline void InitModelRendering( CBaseModelInfoSAInterface *info )
{
    if ( info->renderFlags & RENDER_ADDITIVE )
        (*ppRwInterface)->m_deviceCommand( (eRwDeviceCmd)11, 2 );
}

inline void ShutdownModelRendering( CBaseModelInfoSAInterface *info )
{
    if ( info->renderFlags & RENDER_ADDITIVE )
        (*ppRwInterface)->m_deviceCommand( (eRwDeviceCmd)11, 6 );
}

static void __cdecl RenderAtomicWithAlpha( CBaseModelInfoSAInterface *info, RpAtomic *atom, unsigned int alpha )
{
    InitModelRendering( info );

    RpAtomicRenderAlpha( atom, alpha );

    ShutdownModelRendering( info );
}

static void __cdecl RenderClumpWithAlpha( CBaseModelInfoSAInterface *info, RpClump *clump, unsigned int alpha )
{
    InitModelRendering( info );

    LIST_FOREACH_BEGIN( RpAtomic, clump->m_atomics.root, m_atomics )
        if ( item->IsVisible() )
            RpAtomicRenderAlpha( item, alpha );
    LIST_FOREACH_END

    ShutdownModelRendering( info );
}

static void __cdecl DefaultRenderEntityHandler( CEntitySAInterface *entity, float camDistance )
{
    RwObject *rwobj = entity->m_rwObject;

    if ( !rwobj )
        return;

    CBaseModelInfoSAInterface *info = ppModelInfo[entity->m_model];

    if ( info->renderFlags & RENDER_NOSHADOW )
        (*ppRwInterface)->m_deviceCommand( (eRwDeviceCmd)8, 0 );

    if ( entity->m_entityFlags & ENTITY_FADE )
    {
        (*ppRwInterface)->m_deviceCommand( (eRwDeviceCmd)30, 0 );

        unsigned int alpha = (unsigned char)CalculateFadingAlpha( info, entity, camDistance );

        if ( CObjectSA *obj = pGame->GetPools()->GetObject( entity ) )
            __asm nop

        unsigned int flags = entity->m_entityFlags;
        flags |= ENTITY_RENDERING;

        entity->m_entityFlags = flags;

        if ( flags & ENTITY_BACKFACECULL )
            (*ppRwInterface)->m_deviceCommand( (eRwDeviceCmd)20, 1 );

        unsigned char lightIndex = entity->SetupLighting();

        if ( rwobj->m_type == RW_ATOMIC )
            RenderAtomicWithAlpha( info, (RpAtomic*)rwobj, alpha );
        else
            RenderClumpWithAlpha( info, (RpClump*)rwobj, alpha );

        entity->RemoveLighting( lightIndex );

        flags &= ~ENTITY_RENDERING;
        entity->m_entityFlags = flags;

        if ( flags & ENTITY_BACKFACECULL )
            (*ppRwInterface)->m_deviceCommand( (eRwDeviceCmd)20, 2 );

        (*ppRwInterface)->m_deviceCommand( (eRwDeviceCmd)30, 100 );
    }
    else
    {
        if ( !*(unsigned int*)VAR_currArea && info->renderFlags & RENDER_NOSHADOW )
            (*ppRwInterface)->m_deviceCommand( (eRwDeviceCmd)30, 100 );
        else
            (*ppRwInterface)->m_deviceCommand( (eRwDeviceCmd)30, 0 );

        RenderEntity( entity );
    }

    if ( info->renderFlags & RENDER_NOSHADOW )
        (*ppRwInterface)->m_deviceCommand( (eRwDeviceCmd)8, 1 );
}

void EntityRender_Init( void )
{
    // Do some interesting patches
    HookInstall( 0x00733A20, (DWORD)HOOK_InitRenderChains, 5 );
    HookInstall( 0x00732B40, (DWORD)DefaultRenderEntityHandler, 5 );
    HookInstall( 0x00553260, (DWORD)RenderEntity, 5 );
}

void EntityRender_Shutdown( void )
{
}