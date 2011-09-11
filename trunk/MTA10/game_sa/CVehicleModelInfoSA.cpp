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