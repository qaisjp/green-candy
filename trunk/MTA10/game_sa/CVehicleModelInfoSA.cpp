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

#define FUNC_InitVehicleData                0x005B8F00
#define FUNC_LoadVehicleColors              0x005B6890
#define FUNC_LoadCarMods                    0x005B65A0
#define FUNC_LoadVehicleParticles           0x004C8780

static float trainLODDistance = 209 * RW_RENDER_UNIT;
static float highDetailDistance = 18 * RW_RENDER_UNIT;

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
        txdEntry = pTxdPool->Get(pGame->GetTextureManager()->LoadDictionaryEx( "vehicle", "MODELS\\GENERIC\\VEHICLE.TXD" ));

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
}

static bool RwAtomicRenderTrainLOD( RpAtomic *atomic )
{
    if (*(float*)VAR_ATOMIC_RENDER_OFFSET > fTrainLODDistance)
        return true;

    RpAtomicRender( atomic );
    return true;
}

static bool RwAtomicRenderTranslucentTrain( RpAtomic *atomic )
{
    float calc;

    if ( *(float*)VAR_ATOMIC_RENDER_OFFSET >= trainLODDistance )
        return true;

    if ( *(float*)VAR_ATOMIC_RENDER_OFFSET >= highDetailDistance )
        atomic->m_renderFlags &= ~0x20;
    else
        atomic->m_renderFlags |= 0x20;

    calc = RwMatrixUnknown( atomic->m_parent->m_ltm, atomic->m_geometry->m_parent->m_ltm, atomic->m_matrixFlags );

    // Lol, serious checking going on here!
    if ( *(float*)0x00C88024 < *(float*)VAR_ATOMIC_RENDER_OFFSET && !(atomic->m_matrixFlags & 0x04)
        && *(float*)0x00C88020 > 0.2f
        && calc < 0.0f
        && !(atomic->m_matrixFlags & 0x80)
        && *(float*)VAR_ATOMIC_RENDER_OFFSET < 0.1f )
        return true;

    if ( atomic->m_matrixFlags & 0x40 )
    {

    }
    else
    {

    }
}

static bool RwAtomicRenderTrain( RpAtomic *atomic )
{

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
    {

    }

    RwAtomicSetupVehicleDamaged( child );
    return true;
}

void CVehicleModelInfoSAInterface::RegisterRenderCallbacks()
{
    switch (m_vehicleType)
    {
    case VEHICLE_TRAIN:
        
        break;
    case VEHICLE_PLANE:
    case VEHICLE_FAKEPLANE:

        break;
    case VEHICLE_BOAT:

        break;
    default:

        break;
    }
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

    memset(&m_unknown, 0, sizeof(m_unknown));

    m_unknown2 = 0;
    m_unknown3 = 0;
}