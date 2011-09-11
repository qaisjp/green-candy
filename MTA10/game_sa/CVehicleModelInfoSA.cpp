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

    __asm
    {
        mov dwFunc,FUNC_LoadVehicleColors
        call dwFunc
        mov dwFunc,FUNC_LoadCarMods
        call dwFunc
        mov dwFunc,FUNC_LoadVehicleParticles
        call dwFunc
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