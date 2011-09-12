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

void    Hook_InitVehicleData()
{
    DWORD dwFunc;
    CTxdInstanceSA *txdEntry;

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

    m_pVehicleModelPool = new CVehicleModelPool;

    __asm
    {
        mov ecx,0x005D5BC0
        call ecx
    }
}

void    VehicleModels_Init()
{
    HookInstall(FUNC_InitVehicleData, HOOK_InitVehicleData, 5);
}

CVehicleModelInfoSAInterface::CVehicleModelInfoSAInterface()
{

}

CVehicleModelInfoSAInterface::~CVehicleModelInfoSAInterface()
{

}

CAtomicModelInfoSA* CVehicleModelInfoSAInterface::GetAtomicModelInfo()
{
    return NULL;
}

CDamageAtomicModelInfoSA* CVehicleModelInfoSAInterface::GetDamageAtomicModelInfo()
{
    return NULL;
}

CLODAtomicModelInfoSA* CVehicleModelInfoSAInterface::GetLODAtomicModelInfo()
{
    return NULL;
}

eModelType CVehicleModelInfoSAInterface::GetModelType()
{
    return MODEL_VEHICLE;
}

unsigned int CVehicleModelInfoSAInterface::GetTimeInfo()
{
    return 0;
}

bool CVehicleModelInfoSAInterface::SetAnimFile( const char *name )
{
    return false;
}

void CVehicleModelInfoSAInterface::ConvertAnimFileIndex()
{

}

int CVehicleModelInfoSAInterface::GetAnimFileIndex()
{
    return -1;
}