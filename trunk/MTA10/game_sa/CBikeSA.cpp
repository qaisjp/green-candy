/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.0
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        game_sa/CBikeSA.cpp
*  PURPOSE:     Bike vehicle entity
*  DEVELOPERS:  Ed Lyons <eai@opencoding.net>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#include "StdInc.h"

CBikeSA::CBikeSA( unsigned short modelId ) : CVehicleSA( modelId )
{
    DEBUG_TRACE("CBikeSA::CBikeSA( eVehicleTypes dwModelID ):CVehicleSA( dwModelID )");
}

CBikeSA::~CBikeSA()
{

}

void CBikeSA::PlaceOnRoadProperly()
{
    DEBUG_TRACE("void CBikeSA::PlaceBikeOnRoadProperly()");
    DWORD dwFunc = FUNC_Bike_PlaceOnRoadProperly;
    DWORD dwBike = (DWORD)GetInterface();

    _asm
    {
        mov     ecx, dwBike
        call    dwFunc
    }
}

unsigned char CVehicleSA::GetBikeWheelStatus( unsigned char id )
{
    if ( id > 1 )
        return 0;

    return GetInterface()->m_wheelStatus[id];
}

void CVehicleSA::SetBikeWheelStatus( unsigned char id, unsigned char status )
{
    if ( id > 1 )
        return;

    GetInterface()->m_wheelStatus[id] = status;
}