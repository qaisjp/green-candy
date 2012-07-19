/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        game_sa/CCamSA.cpp
*  PURPOSE:     Camera entity
*  DEVELOPERS:  Ed Lyons <eai@opencoding.net>
*               Jax <>
*               Cecill Etheredge <ijsf@gmx.net>
*               The_GTA <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#include "StdInc.h"

CEntity* CCamSA::GetTargetEntity() const
{
    CPoolsSA* pPools = (CPoolsSA*)pGame->GetPools();
    CEntity* pReturn = NULL;

    switch( m_pInterface->CamTargetEntity->m_type )
    {
    case ENTITY_TYPE_PED:
        return pPools->GetPed( m_pInterface->CamTargetEntity );
    case ENTITY_TYPE_VEHICLE:
        return pPools->GetVehicle( m_pInterface->CamTargetEntity );
    case ENTITY_TYPE_OBJECT:
        return pPools->GetObject( m_pInterface->CamTargetEntity );
    }

    return NULL;
}

void CCamSA::SetVehicleInterpolationSource( CVehicleSA *veh )
{
    m_pInterface->m_pLastCarEntered = veh->GetInterface();
}

void CCamSA::SetPedInterpolationSource( CPedSA *ped )
{
    m_pInterface->m_pLastPedLookedAt = ped->GetInterface();
}