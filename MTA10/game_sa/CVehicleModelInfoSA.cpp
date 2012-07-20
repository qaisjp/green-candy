/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.0
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        game_sa/CPedModelInfoSA.cpp
*  PURPOSE:     Vehicle model info
*  DEVELOPERS:  The_GTA <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#include "StdInc.h"
#include "gamesa_renderware.h"

struct _licensePlate
{
    char text[8];
    unsigned int style;
    RpMaterial *plate;  // ext
};

typedef RpMaterial*     (*HandleVehicleFrontNameplate_t)    ( RpMaterial *mat, _licensePlate *info, unsigned char design );
typedef RpMaterial*     (*HandleVehicleBackNameplate_t)     ( RpMaterial *mat, unsigned char design );

HandleVehicleFrontNameplate_t   HandleVehicleFrontNameplate         = ( HandleVehicleFrontNameplate_t )         0x006FE020;
HandleVehicleBackNameplate_t    HandleVehicleBackNameplate          = ( HandleVehicleBackNameplate_t )          0x006FDE50;

#define FUNC_InitVehicleData                0x005B8F00
#define FUNC_LoadVehicleColors              0x005B6890
#define FUNC_LoadCarMods                    0x005B65A0
#define FUNC_LoadVehicleParticles           0x004C8780

static float trainLODDistance = 45000;      // In RenderWare render units
static float boatLODDistance = 9800;
static float heliLODDistance = 9800;        // same as boat
static float heliRotorRenderDistance = 45000;   // same as train
static float planeLODDistance = 45000;      // same as train
static float vehicleLODDistance = 9800;     // same as boat
static float highDetailDistance = 4050;

void    VehicleModels_Init()
{
    DWORD dwFunc;
    CTxdInstanceSA *txdEntry;

    // Do not execute it
    *(unsigned char*)FUNC_InitVehicleModels = 0xC3;

    __asm
    {
        mov dwFunc,FUNC_LoadVehicleColors
        call dwFunc
        mov dwFunc,FUNC_LoadCarMods
        call dwFunc
        mov dwFunc,FUNC_LoadVehicleParticles
        call dwFunc
    }

    // Load the generic vehicle textures
    txdEntry = pGame->GetTextureManager()->FindTxdEntry( "vehicle" );

    if ( txdEntry )
        txdEntry->LoadTXD( "MODELS\\GENERIC\\VEHICLE.TXD" );
    else
        txdEntry = (*ppTxdPool)->Get( pGame->GetTextureManager()->LoadDictionaryEx( "vehicle", "MODELS\\GENERIC\\VEHICLE.TXD" ) );

    // Reference it
    txdEntry->Reference();

    *(RwTexture*)0x00B4E68C = RwTexDictionaryFindNamedTexture( txdEntry->m_txd, "vehiclelights128" );
    *(RwTexture*)0x00B4E690 = RwTexDictionaryFindNamedTexture( txdEntry->m_txd, "vehiclelightson128" );

    // Allocate the seat placement pool
    pVehicleSeatPlacementPool = new CVehicleSeatPlacementPool;

    __asm
    {
        mov ecx,0x005D5BC0
        call ecx
    }
}

CVehicleModelInfoSAInterface::CVehicleModelInfoSAInterface()
{
    Init();
}

CVehicleModelInfoSAInterface::~CVehicleModelInfoSAInterface()
{

}

eModelType CVehicleModelInfoSAInterface::GetModelType()
{
    return MODEL_VEHICLE;
}

void CVehicleModelInfoSAInterface::SetAnimFile( const char *name )
{
    char *anim;

    if ( strcmp(name, "null") == 0 )
        return;

    anim = malloc( strlen( name ) + 1 );

    strcpy(anim, name);

    // this is one nasty hack
    m_animFileIndex = (int)anim;
}

void CVehicleModelInfoSAInterface::ConvertAnimFileIndex()
{
    int animBlock;

    if ( m_animFileIndex == -1 )
        return;

    animBlock = pGame->GetAnimManager()->GetAnimBlockIndex( (const char*)m_animBlock );

    free( (void*)m_animFileIndex );

    // Yeah, weird
    m_animFileIndex = animBlock;
}

int CVehicleModelInfoSAInterface::GetAnimFileIndex()
{
    return m_animFileIndex;
}

void CVehicleModelInfoSAInterface::SetClump( RpClump *clump )
{
    m_seatPlacement = new (pVehicleSeatPlacementPool->Allocate()) CVehicleSeatPlacementSAInterface();

    CClumpModelInfoSAInterface::SetClump( clump );

    RegisterRenderCallbacks();

    // Correctly assign vehicle atomics
    AssignAtomics( ((CAtomicHierarchySAInterface**)0x008A7740)[m_vehicleType] );

    RegisterRoot();

    Setup();

    SetupMateria();

    InitNameplate();
}

static bool RwAtomicRenderTrainLOD( RpAtomic *atomic )
{
    if (*(float*)VAR_ATOMIC_RENDER_OFFSET <= trainLODDistance)
        return true;

    RpAtomicRender( atomic );
    return true;
}

static bool RwAtomicRenderTranslucentTrain( RpAtomic *atomic )
{
    RwAtomicZBufferEntry level;
    float calc;

    if ( *(float*)VAR_ATOMIC_RENDER_OFFSET >= trainLODDistance )
        return true;

    if ( *(float*)VAR_ATOMIC_RENDER_OFFSET >= highDetailDistance )
        atomic->m_renderFlags &= ~0x20;
    else
        atomic->m_renderFlags |= 0x20;

    calc = RwMatrixUnknown( atomic->m_parent->m_ltm, atomic->m_geometry->m_parent->m_ltm, atomic->m_matrixFlags );

    // Lol, serious checking going on here!
    if ( *(float*)VAR_ATOMIC_RENDER_OFFSET < *(float*)0x00C8802C && !(atomic->m_matrixFlags & 0x04)
        && *(float*)0x00C88020 > 0.2f
        && calc < 0.0f
        && !(atomic->m_matrixFlags & 0x80)
        && *(float*)VAR_ATOMIC_RENDER_OFFSET < 0.1f )
        return true;

    // Set up rendering
    level.m_render = RpAtomicRender;
    level.m_atomic = atomic;
    level.m_distance = *(float*)VAR_ATOMIC_RENDR_OFFSET;

    if ( !(atomic->m_matrixFlags & 0x40) )
        level.m_lod += calc;

    if ( !rwRenderChains->PushRender( &level ) )
        RpAtomicRender( atomic );

    return true;
}

static bool RwAtomicRenderTrain( RpAtomic *atomic )
{
    float calc;

    if ( *(float*)VAR_ATOMIC_RENDER_OFFSET >= trainLODDistance )
        return true;

    if ( *(float*)VAR_ATOMIC_RENDER_OFFSET >= highDetailDistance )
        atomic->m_renderFlags &= ~0x20;
    else
        atomic->m_renderFlags |= 0x20;

    // Lol, serious checking going on here!
    if ( *(float*)VAR_ATOMIC_RENDER_OFFSET < *(float*)0x00C8802C && !(atomic->m_matrixFlags & 0x04)
        && *(float*)0x00C88020 > 0.2f
        && (calc = RwMatrixUnknown( atomic->m_parent->m_ltm, atomic->m_geometry->m_parent->m_ltm, atomic->m_matrixFlags )) < 0.0f
        && !(atomic->m_matrixFlags & 0x80)
        && calc * calc > *(float*)VAR_ATOMIC_RENDER_OFFSET * 0.1 )
        return true;

    RpAtomicRender( atomic );
    return true;
}

static bool RwAtomicRenderBoatLOD( RpAtomic *atomic )
{
    if (*(float*)VAR_ATOMIC_RENDER_OFFSET <= boatLODDistance)
        return true;

    atomic->m_renderFlags |= 0x20;

    if ( atomic->m_clump->m_renderFlags & 0xFF )
    {
        RpAtomicRenderEx( atomic, atomic->m_clump->m_renderFlags );
        return true;
    }

    RpAtomicRender( atomic );
    return true;
}

static bool RwAtomicRenderBoat( RpAtomic *atomic )
{
    if ( *(float*)VAR_ATOMIC_RENDER_OFFSET >= boatLODDistance )
        return true;

    if ( *(float*)VAR_ATOMIC_RENDER_OFFSET >= highDetailDistance )
        atomic->m_renderFlags &= ~0x20;
    else
        atomic->m_renderFlags |= 0x20;

    RpAtomicRender( atomic );
    return true;
}

static bool RwAtomicRenderTranslucentBoat( RpAtomic *atomic )
{
    if ( *(float*)VAR_ATOMIC_RENDER_OFFSET >= boatLODDistance )
        return true;

    if ( *(float*)VAR_ATOMIC_RENDER_OFFSET >= highDetailDistance )
        atomic->m_renderFlags &= ~0x20;
    else
        atomic->m_renderFlags |= 0x20;

    if ( atomic->m_matrixFlags & 0x40 )
    {
        RwAtomicRenderDetailLevel level;

        level.m_atomic = atomic;
        level.m_render = RpAtomicRender;
        level.m_distance = *(float*)VAR_ATOMIC_RENDER_OFFSET;

        if ( rwRenderChains->PushRender( &level ) )
            return true;
    }
    
    RpAtomicRender( atomic );
    return true;
}

static bool RwAtomicRenderHeliLOD( RpAtomic *atomic )
{
    if (*(float*)VAR_ATOMIC_RENDER_OFFSET <= heliLODDistance)
        return true;

    atomic->m_renderFlags |= 0x20;

    if ( atomic->m_clump->m_renderFlags & 0xFF )
    {
        RpAtomicRenderEx( atomic, atomic->m_clump->m_renderFlags );
        return true;
    }

    RpAtomicRender( atomic );
    return true;
}

static bool RwAtomicRenderHeli( RpAtomic *atomic )
{
    float calc;
    
    if ( *(float*)VAR_ATOMIC_RENDER_OFFSET >= heliLODDistance )
        return true;

    if ( *(float*)VAR_ATOMIC_RENDER_OFFSET >= highDetailDistance )
        atomic->m_renderFlags &= ~0x20;
    else
        atomic->m_renderFlags |= 0x20;

    // Lol, serious checking going on here!
    if ( *(float*)VAR_ATOMIC_RENDER_OFFSET < *(float*)0x00C8802C && !(atomic->m_matrixFlags & 0x04)
        && *(float*)0x00C88020 > 0.2f
        && (calc = RwMatrixUnknown( atomic->m_parent->m_ltm, atomic->m_geometry->m_parent->m_ltm, atomic->m_matrixFlags )) < 0.0f
        && !(atomic->m_matrixFlags & 0x80)
        && calc * calc > *(float*)VAR_ATOMIC_RENDER_OFFSET * 0.1 )
        return true;

    RpAtomicRender( atomic );
    return true;
}

static bool RwAtomicRenderTranslucentHeli( RpAtomic *atomic )
{
    float calc;

    if ( *(float*)VAR_ATOMIC_RENDER_OFFSET >= heliLODDistance )
        return true;

    if ( *(float*)VAR_ATOMIC_RENDER_OFFSET >= highDetailDistance )
        atomic->m_renderFlags &= ~0x20;
    else
        atomic->m_renderFlags |= 0x20;

    calc = RwMatrixUnknown( atomic->m_parent->m_ltm, atomic->m_geometry->m_parent->m_ltm, atomic->m_matrixFlags );

    // Lol, serious checking going on here!
    if ( *(float*)VAR_ATOMIC_RENDER_OFFSET < *(float*)0x00C8802C && !(atomic->m_matrixFlags & 0x04)
        && *(float*)0x00C88020 > 0.2f
        && calc < 0.0f
        && !(atomic->m_matrixFlags & 0x80)
        && calc * calc > *(float*)VAR_ATOMIC_RENDER_OFFSET * 0.1 )
        return true;

    // Set up rendering
    level.m_render = RpAtomicRender;
    level.m_atomic = atomic;
    level.m_distance = *(float*)VAR_ATOMIC_RENDR_OFFSET;

    if ( atomic->m_matrixFlags & 0x40 )
        level.m_distance -= 0.00009999997;
    else
        level.m_distance += calc;

    if ( !rwRenderChains->PushRender( &level ) )
        RpAtomicRender( atomic );
    
    return true;
}

static bool RwAtomicRenderHeliMovingRotor( RpAtomic *atomic )
{
    CVector *vecRotor;
    RwAtomicZBufferEntry level;

    if ( *(float*)VAR_ATOMIC_RENDER_OFFSET >= heliRotorRenderDistance )
        return true;

    vecRotor = (CVector*)&atomic->m_parent->m_ltm.pos - (CVector*)0x00C88050;

    // Fun Fact: The top rotor has a 20 unit radius!
    level.m_distance = vecRotor->DotProduct( (CVector*)&atomic->m_geometry->m_parent->m_ltm.at ) * 20 + *(float*)VAR_ATOMIC_RENDER_OFFSET;
    level.m_render = RpAtomicRender;
    level.m_atomic = atomic;

    if ( !rwRenderChains->PushRender( &level ) )
        RpAtomicRender( atomic );

    return true;
}

static bool RwAtomicRenderHeliMovingRotor2( RpAtomic *atomic )
{
    CVector *vecRotor;
    RwAtomicZBufferEntry level;

    if ( *(float*)VAR_ATOMIC_RENDER_OFFSET >= heliRotorRenderDistance )
        return true;

    vecRotor = (CVector)&atomic->m_parent->m_ltm.pos - (CVector)0x00C88050;

    // Lulz, heavy math, much assembly, small C++ code
    level.m_distance = *(float*)VAR_ATOMIC_RENDER_OFFSET - vecRotor->DotProduct( (CVector*)&atomic->m_geometry->m_parent->m_ltm.right ) - vecRotor->DotProduct( (CVector*)&atomic->m_geometry->m_parent->m_ltm.up );
    level.m_render = RpAtomicRender;
    level.m_atomic = atomic;

    if ( !rwRenderChains->PushRender( &level ) )
        RpAtomicRender( atomic );

    return true;
}

static bool RwAtomicRenderPlane( RpAtomic *atomic )
{
    if ( *(float*)VAR_ATOMIC_RENDER_OFFSET >= planeLODDistance )
        return true;

    if ( *(float*)VAR_ATOMIC_RENDER_OFFSET >= highDetailDistance )
        atomic->m_renderFlags &= ~0x20;
    else
        atomic->m_renderFlags |= 0x20;

    // Lol, serious checking going on here!
    if ( *(float*)VAR_ATOMIC_RENDER_OFFSET < *(float*)0x00C88028 && !(atomic->m_matrixFlags & 0x04)
        && *(float*)0x00C88020 > 0.2f
        && RwMatrixUnknown( atomic->m_parent->m_ltm, atomic->m_geometry->m_parent->m_ltm, atomic->m_matrixFlags ) >= 0.0f
        return true;

    RpAtomicRender( atomic );
    return true;
}

static bool RwAtomicRenderTranslucentPlane( RpAtomic *atomic )
{
    float calc;

    if ( *(float*)VAR_ATOMIC_RENDER_OFFSET >= heliLODDistance )
        return true;

    if ( *(float*)VAR_ATOMIC_RENDER_OFFSET >= highDetailDistance )
        atomic->m_renderFlags &= ~0x20;
    else
        atomic->m_renderFlags |= 0x20;

    calc = RwMatrixUnknown( atomic->m_parent->m_ltm, atomic->m_geometry->m_parent->m_ltm, atomic->m_matrixFlags );

    // Lol, serious checking going on here!
    if ( *(float*)VAR_ATOMIC_RENDER_OFFSET < *(float*)0x00C88028 && !(atomic->m_matrixFlags & 0x04)
        && *(float*)0x00C88020 > 0.2f
        && calc < 0.0f
        && !(atomic->m_matrixFlags & 0x80)
        && calc * calc > *(float*)VAR_ATOMIC_RENDER_OFFSET * 0.1 )
        return true;

    // Set up rendering
    level.m_render = RpAtomicRender;
    level.m_atomic = atomic;
    level.m_distance = *(float*)VAR_ATOMIC_RENDR_OFFSET;

    if ( atomic->m_matrixFlags & 0x40 )
        level.m_distance -= 0.00009999997;
    else
        level.m_distance += calc;

    if ( !rwRenderChains->PushRender( &level ) )
        RpAtomicRender( atomic );
    
    return true;
}

static bool RwAtomicRenderTranslucentDefaultVehicle( RpAtomic *atomic ) // actually equals heli render
{
    float calc;

    if ( *(float*)VAR_ATOMIC_RENDER_OFFSET >= vehicleLODDistance )
        return true;

    if ( *(float*)VAR_ATOMIC_RENDER_OFFSET >= highDetailDistance )
        atomic->m_renderFlags &= ~0x20;
    else
        atomic->m_renderFlags |= 0x20;

    calc = RwMatrixUnknown( atomic->m_parent->m_ltm, atomic->m_geometry->m_parent->m_ltm, atomic->m_matrixFlags );

    // Lol, serious checking going on here!
    if ( *(float*)VAR_ATOMIC_RENDER_OFFSET < *(float*)0x00C8802C && !(atomic->m_matrixFlags & 0x04)
        && *(float*)0x00C88020 > 0.2f
        && calc < 0.0f
        && !(atomic->m_matrixFlags & 0x80)
        && calc * calc > *(float*)VAR_ATOMIC_RENDER_OFFSET * 0.1 )
        return true;

    // Set up rendering
    level.m_render = RpAtomicRender;
    level.m_atomic = atomic;
    level.m_distance = *(float*)VAR_ATOMIC_RENDR_OFFSET;

    if ( atomic->m_matrixFlags & 0x40 )
        level.m_distance -= 0.00009999997;
    else
        level.m_distance += calc;

    if ( !rwRenderChains->PushRender( &level ) )
        RpAtomicRender( atomic );
    
    return true;
}

static bool RwAtomicRenderDefaultVehicle( RpAtomic *atomic )    // actually equals heli render
{
    float calc;
    
    if ( *(float*)VAR_ATOMIC_RENDER_OFFSET >= vehicleLODDistance )
        return true;

    if ( *(float*)VAR_ATOMIC_RENDER_OFFSET >= highDetailDistance )
        atomic->m_renderFlags &= ~0x20;
    else
        atomic->m_renderFlags |= 0x20;

    // Lol, serious checking going on here!
    if ( *(float*)VAR_ATOMIC_RENDER_OFFSET < *(float*)0x00C8802C && !(atomic->m_matrixFlags & 0x04)
        && *(float*)0x00C88020 > 0.2f
        && (calc = RwMatrixUnknown( atomic->m_parent->m_ltm, atomic->m_geometry->m_parent->m_ltm, atomic->m_matrixFlags )) < 0.0f
        && !(atomic->m_matrixFlags & 0x80)
        && calc * calc > *(float*)VAR_ATOMIC_RENDER_OFFSET * 0.1 )
        return true;

    RpAtomicRender( atomic );
    return true;
}

static bool RwAtomicSetupVehicleDamaged( RpAtomic *child )
{
    if ( strstr(child->m_parent->m_nodeName, "_dam") )
    {
        child->m_flags = 0;

        child->m_visibility = 2;
        return true;
    }

    if ( strstr(child->m_parent->m_nodeName, "_ok") )
        child->m_visibility = 1;

    return true;
}

static bool RwAtomicRegisterTrain( RpAtomic *child, void *data )
{
    if ( strstr(child->m_parent->m_nodeName, "_vlo") )
    {
        child->SetRenderCallback( child );
        return true;
    }
    else if ( child->m_geometry->IsAlpha() )
        child->SetRenderCallback( RwAtomicRenderTranslucentTrain ); // translucent polys need second render pass
    else
        child->SetRenderCallback( RwAtomicRenderTrain );

    RwAtomicSetupVehicleDamaged( child );
    return true;
}

static bool RwAtomicRegisterBoat( RpAtomic *child, void *data )
{
    if ( strcmp( child->m_parent->m_nodeName, "boat_hi" ) )
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

static bool RwAtomicRegisterHeli( RpAtomic *child, void *data )
{
    if ( strcmp( child->m_parent->m_nodeName, "moving_rotor" ) == 0 )
        child->SetRenderCallback( RwAtomicRenderHeliMovingRotor );
    else if ( strcmp( child->m_parent->m_nodeName, "moving_rotoz2" ) == 0 )
        child->SetRenderCallback( RwAtomicRenderHeliMovingRotor2 );
    else if ( strstr( child->m_parent->m_nodeName, "_vlo" ) == 0 )
        child->SetRenderCallback( RwAtomicRenderHeliLOD );
    else if ( child->m_geometry->IsAlpha() || strncmp( child->m_parent->m_nodeName, "windscreen", 10) == 0 )
        child->SetRenderCallback( RwAtomicRenderTranslucentHeli );
    else
        child->SetRenderCallback( RwAtomicRenderHeli );

    RwAtomicSetupVehicleDamaged( child );
    return true;
}

static bool RwAtomicRegisterPlane( RpAtomic *child, void *data )
{
    if ( strstr(child->m_parent->m_nodeName, "_vlo") )
    {
        child->SetRenderCallback( child );
        return true;
    }
    else if ( child->m_geometry->IsAlpha() )
        child->SetRenderCallback( RwAtomicRenderTranslucentPlane );
    else
        child->SetRenderCallback( RwAtomicRenderPlane );

    RwAtomicSetupVehicleDamaged( child );
    return true;
}

static bool RwAtomicRegisterDefaultVehicle( RpAtomic *child, void *data )
{
    if ( strstr( child->m_parent->m_nodeName, "_vlo" ) == 0 )
        child->SetRenderCallback( RwAtomicRenderHeliLOD );
    else if ( child->m_geometry->IsAlpha() || strncmp( child->m_parent->m_nodeName, "windscreen", 10) == 0 )
        child->SetRenderCallback( RwAtomicRenderTranslucentDefaultVehicle );
    else
        child->SetRenderCallback( RwAtomicRenderDefaultVehicle );

    RwAtomicSetupVehicleDamaged( child );
    return true;
}

void CVehicleModelInfoSAInterface::RegisterRenderCallbacks()
{
    switch( m_vehicleType )
    {
    case VEHICLE_TRAIN:
        m_rwClump->ForAllAtomics( RwAtomicRegisterTrain, 0 );
        return;
    case VEHICLE_PLANE:
    case VEHICLE_FAKEPLANE:
        m_rwClump->ForAllAtomics( RwAtomicRegisterPlane, 0 );
        return;
    case VEHICLE_BOAT:
        m_rwClump->ForAllAtomics( RwAtomicRegisterBoat, 0 );
        return;
    case VEHICLE_HELI:
        m_rwClump->ForAllAtomics( RwAtomicRegisterHeli, 0 );
        return;
    }

    m_rwClump->ForAllAtomics( RwAtomicRegisterDefaultVehicle, 0 );
}

CVehicleSeatPlacementSAInterface::CVehicleSeatPlacementSAInterface()
{
    unsigned int n;

    for (n=0; n<MAX_SEATS; n++)
    {
        m_seatOffset[n].fX = 0;
        m_seatOffset[n].fY = 0;
        m_seatOffset[n].fZ = 0;
    }

    for (n=0; n<18; n++)
    {
        m_info[n].m_id = -1;
    }

    // We have no atomics in the beginning
    memset(&m_atomics, 0, sizeof(m_atomics));

    m_atomicCount = 0;

    m_unknown4 = 0;
    m_unknown3 = 0;
}

void* CVehicleSeatPlacementSAInterface::operator new( size_t )
{
    return (*ppVehicleSeatPlacementPool)->Allocate();
}

void CVehicleSeatPlacementSAInterface::operator delete( void *ptr )
{
    (*ppVehicleSeatPlacementPool)->Free( (CVehicleSeatPlacementSAInterface*)ptr );
}

void CVehicleSeatPlacementSAInterface::AddAtomic( RpAtomic *atomic )
{
    if ( m_atomicCount == 6 )
        return;

    m_atomics[ m_atomicCount++ ] = atomic;
}

void CVehicleModelInfoSAInterface::Setup()
{
    tHandlingDataSA *handling = &m_OriginalHandlingData[ m_handlingID ];
    CAtomicHierarchySAInterface *info = ((CAtomicHierarchySAInterface**)0x008A7740)[ m_vehicleType ];
    RpAtomic *obj1 = NULL;
    RpAtomic *obj2 = NULL;
    RwFrame *hier;

    for (info; info->m_name; info++)
    {
        if ( info->m_flags & (ATOMIC_HIER_FRONTSEAT | ATOMIC_HIER_SEAT | ATOMIC_HIER_UNKNOWN3) && hier = m_rwClump->m_parent->FindFreeChildByName( info->m_name ) )
        {
            if ( info->m_flags & ATOMIC_HIER_FRONTSEAT )
            {
                RwFrame *parent = hier;

                // Position the seats
                m_seatPlacement->m_seatOffset = (CVector)hier->m_parent->m_ltm.pos;

                while ( parent = parent->m_parent && parent->m_parent )
                    pRwInterface->m_matrixTransform3( m_seatPlacement->m_seatOffset[ info->m_frameHierarchy ], m_seatPlacement->m_seatOffset[ info->m_frameHierarchy ], 1, parent->m_parent->m_modelling );

                RwFrameCloneHierarchy( hier );
            }
            else if ( info->m_flags & ATOMIC_HIER_UNKNOWN3 )
            {
                 CVehicleSeatInfoSA *seat = &m_seatPlacement->m_info[ info->m_frameHierarchy ];

                 seat->m_offset = (CVector)hier->m_modelling.pos;

                 // Calculate the quat for rotation
                 new (&seat->m_quat) CQuat( hier->m_modelling );

                 seat->m_id = hier->m_hierarchyId;
            }
            else
            {
                RpAtomic *atomic = hier->GetFirstAtomic();

                RpClumpRemoveAtomic( m_rwClump, atomic );

                RwFrameRemoveChild( hier );

                // Apply the seat flags
                SetComponentFlags( hier, info->m_flags );

                // Append the atomic onto the seat interface
                m_seatPlacement->AddAtomic( atomic );
            }
        }

        if ( info->m_flags & (ATOMIC_HIER_UNKNOWN4 | ATOMIC_HIER_UNKNOWN5) && hier = m_rwClump->m_parent->FindChildByHierarchy( info->m_frameHierarchy ) )
        {
            for ( hier; hier; hier = hier->GetFirstChild() )
            {
                RpAtomic *obj = hier->GetFirstAtomic();

                if ( !obj )
                    continue;

                if ( hier->m_flags & ATOMIC_HIER_UNKNOWN4 )
                    obj1 = obj;
                else
                    obj2 = obj;

                break;
            }
        }
    }

    info = ((CAtomicHierarchySAInterface**)0x008A7740)[ m_vehicleType ];

    for (info; info->m_name; info++)
    {
        if ( info->m_flags & (ATOMIC_HIER_FRONTSEAT | ATOMIC_HIER_SEAT | ATOMIC_HIER_UNKNOWN3) )
            continue;

        hier = m_rwClump->m_parent->FindChildByHierarchy( info->m_frameHierarchy );

        if ( !hier )
            continue;

        if ( info->m_flags & ATOMIC_HIER_DOOR )
            m_numberOfDoors++;

        if ( info->m_flags & 0x02 )
        {
            RpAtomic *primary = NULL;
            RpAtomic *secondary = NULL;

            hier->BaseAtomicHierarchy();

            hier->RegisterRoot();

            hier->FindVisibilityAtomics( &primary, &secondary );

            if ( primary && secondary )
                secondary->SetRenderCallback( primary->m_renderCallback );

            m_seatPlacement->m_usageFlags |= 1 << info->m_frameHierarchy;
        }

        SetComponentFlags( hier, info->m_flags );

        if ( info->m_flags & (ATOMIC_HIER_UNKNOWN4 | 0x04) )
        {
            RpAtomic *clone;
            RwFrame *frame;

            if ( !obj1 )
                continue;

            if ( info->m_flags & ATOMIC_HIER_UNKNOWN4 )
            {
                clone = RpAtomicClone( obj1 )

                RpAtomicSetFrame( clone, hier );
                RpClumpAddAtomic( m_rwClump, clone );

                // Default the render callback
                clone->SetRenderCallback( NULL );

                if ( info->m_frameHierarchy == 2 || info->m_frameHierarchy == 5 || !( handling->uiModelFlags & 0x20000000 ) )
                    continue;

                clone = RpAtomicClone( obj1 );

                // Create a new rotation frame
                frame = RwFrameCreate();
                RpAtomicSetFrame( clone, frame );

                RwFrameAddChild( hier, frame );

                new (&frame->m_modelling) RwMatrix();

                RpClumpAddAtomic( m_rwClump, clone );

                clone->SetRenderCallback( NULL );
                continue;
            }

            hier->BaseAtomicHierarchy();

            hier->RegisterRoot();

            obj1->SetRenderCallback( NULL );
        }
        else if ( info->m_flags & ATOMIC_HIER_UNKNOWN6 )
        {
            if ( !obj2 )
                continue;

            clone = RpAtomicClone( obj2 );

            RpAtomicSetFrame( hier, clone );

            RpClumpAddAtomic( m_rwClump, clone );

            clone->SetRenderCallback( NULL );
        }
    }
}

void CVehicleModelInfoSAInterface::SetComponentFlags( RwFrame *frame, unsigned int flags )
{
    tHandlingDataSA *handling = &m_OriginalHandlingData[ m_handlingID ];

    if ( flags & 0x1000 )
        frame->SetAtomicVisibility( 0x80 );

    if ( flags & 0x400000 )
        frame->SetAtomicVisibility( 0x400 );

    if ( flags & 0x40000 )
        frame->SetAtomicVisibility( 0x2000 );

    if ( flags & 0x80 )
        frame->SetAtomicVisibility( 0x10 );
    else if ( flags & 0x0100 && ( handling->uiModelFlags & 0x01 || !( flags & (0x20 | 0x40) ) ) )
        frame->SetAtomicVisibility( 0x20 );
    else if ( flags & 0x20 )
        frame->SetAtomicVisibility( 0x04 );
    else if ( flags & 0x40 )
        frame->SetAtomicVisibility( 0x08 );

    if ( flags & 0x8000 && ( handling->uiModelFlags & 0x80000000 || flags & (0x20 | 0x40) ) )
        frame->SetAtomicVisibility( 0x8000 );

    if ( flags & 0x2000 )
        frame->SetAtomicVisibility( 0x100 );
    else if ( flags & 0x4000 )
        frame->SetAtomicVisibility( 0x200 );

    if ( flags & 0x0400 )
        frame->SetAtomicVisibility( 0x40 );
}

static bool RwClumpAtomicSetupVehiclePipeline( RpAtomic *child, void *data )
{
    RpAtomicSetupVehiclePipeline( child );
    return true;
}

void CVehicleModelInfoSAInterface::RegisterRoot()
{
    RwFrame *frame;
    CVector normal = { 1.0f, 0, 0 };

    // Make sure we render using the vehicle pipeline
    m_rwClump->ForAllAtomics( RwClumpAtomicSetupVehiclePipeline, 0 );

    // Do not do stuff if we have a root already
    if ( *(RwFrame*)0x00B4E6B8 )
        return;

    frame = RwFrameCreate();

    *(RwFrame*)0x00B4E6B8 = frame;

    RwFrameOrient( frame, 60, 0, normal );

    frame->RegisterRoot();

    // Cache the matrix
    RwFrameGetLTM( frame );
}

void CVehicleModelInfoSAInterface::SetupMateria()
{
    RpMaterials mats;
    unsigned int n;

    RwPrefetch();

    new (&mats) RpMaterials( 20 );

    m_rwClump->FetchMateria( &mats );

    for (n=0; n<m_seatPlacement->m_atomicCount; n++)
        m_seatPlacement->m_atomics[n]->FetchMateria( &mats );

    delete &mats;

    m_rwClump->RemoveAtomicVisibilityFlags( 0x2000 );
}

#define RAND        (double)(rand() & 0xFFFF) / 0x7FFF
#define RANDCHAR    RAND * 0.23
#define RANDNUM     RAND * -9

bool GetRandomNameplateText( char *buffer, size_t max )
{
    unsigned int n;

    if ( max < 4 )
        return false;

    buffer[0] = 'A' - RANDCHAR;
    buffer[1] = 'A' - RANDCHAR;
    buffer[2] = '0' - RANDNUM;
    buffer[3] = '0' - RANDNUM;

    for (n=4; n<max; )
    {
        buffer[n++] = '0' - RANDNUM;
        buffer[n++] = 'A' - RANDCHAR;
        buffer[n++] = 'A' - RANDCHAR;
    }

    return true;
}

static bool RwMaterialSetLicensePlate( RpMaterial *mat, _licensePlate *plate )
{
    if ( !mat->m_texture )
        return true;

    if ( strcmp( mat->m_texture->name, "carplate" ) == 0 )
    {
        plate->plate = mat;

        HandleVehicleFrontNameplate( mat, plate, *(unsigned char*)0x00C3EF80 );
        return true;
    }

    if ( strcmp( mat->m_texture->name, "carpback" ) == 0 )
        HandleVehicleBackNameplate( mat, *(unsigned char*)0x00C3EF80 );

    return true;
}

static bool RwAtomicSetLicensePlate( RpAtomic *child, _licensePlate *plate )
{
    child->m_geometry->ForAllMateria( RwMaterialSetLicensePlate, plate );
    return true;
}

void CVehicleModelInfoSAInterface::InitNameplate()
{
    _licensePlate plate;

    // Get some random stuff into nameplate
    GetRandomNameplateText( plate.text, 8 );

    plate.style = m_plateDesign;
    plate.plate = NULL;

    m_rwClump->ForAllAtomics( RwAtomicSetLicensePlate, &plate );

    if ( plate.plate )
        m_plateMaterial = plate.plate;
}