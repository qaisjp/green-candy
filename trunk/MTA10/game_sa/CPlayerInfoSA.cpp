/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.0
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        game_sa/CPlayerInfoSA.cpp
*  PURPOSE:     Player ped type information
*  DEVELOPERS:  Ed Lyons <eai@opencoding.net>
*               Christian Myhre Lundheim <>
*               Cecill Etheredge <ijsf@gmx.net>
*               Jax <>
*               The_GTA <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#include "StdInc.h"

extern CBaseModelInfoSAInterface **ppModelInfo;

CPlayerPedDataSAInterface::CPlayerPedDataSAInterface()
{
    Serialize();
}

void CPlayerPedDataSAInterface::Serialize()
{
    // Remote players have their data serialized, because they do not own references or allocations
    m_meleeAnimRef = 0;
    m_meleeAnimExtraRef = 0;
}

CPlayerPedSA* CPlayerInfoSA::GetPlayerPed()
{
    DEBUG_TRACE("CPlayerPedSA* CPlayerInfoSA::GetPlayerPed()");

    return (CPlayerPedSA*)mtaPeds[0];
}

CWantedSA* CPlayerInfoSA::GetWanted()
{
    DEBUG_TRACE("CWantedSA* CPlayerInfoSA::GetWanted()");

    if ( !m_wanted )
        m_wanted = new CWantedSA( m_interface->m_pedData.m_Wanted ); 

    return m_wanted;
}

bool CPlayerInfoSA::GetCrossHair( float& tarX, float& tarY )
{
    DEBUG_TRACE("bool CPlayerInfoSA::GetCrossHair( float &tarX, float &tarY )");

    tarX = m_interface->m_crossHair.m_tarX;
    tarY = m_interface->m_crossHair.m_tarY;
    return m_interface->m_crossHair.m_active;
}

long CPlayerInfoSA::GetPlayerMoney()
{
    DEBUG_TRACE("long CPlayerInfoSA::GetPlayerMoney()");
    return *(long*)0xB7CE50;
}

void CPlayerInfoSA::SetPlayerMoney( long value )
{
    DEBUG_TRACE("void CPlayerInfoSA::SetPlayerMoney( long value )");

    *(long*)0xB7CE50 = value;
}

void CPlayerInfoSA::GivePlayerParachute()
{
    DEBUG_TRACE("VOID CPlayerInfoSA::GivePlayerParachute()");

    DWORD dwFunction = FUNC_GivePlayerParachute;
    _asm
    {
        call dwFunction
    }
}

void CPlayerInfoSA::StreamParachuteWeapon( bool allow )
{
    DEBUG_TRACE("void CPlayerInfoSA::StreamParachuteWeapon( bool allow )");

    DWORD dwFunction = FUNC_StreamParachuteWeapon;
    _asm {
        push allow
        call dwFunction
    }
}

void CPlayerInfoSA::MakePlayerSafe( bool safe )
{
    DEBUG_TRACE("void CPlayerInfoSA::MakePlayerSafe( bool safe )");

    DWORD dwFunction = FUNC_MakePlayerSafe;
    _asm
    {
        push    safe
        call    dwFunction
    }
}

void CPlayerInfoSA::CancelPlayerEnteringCars( CVehicleSA* veh )
{
    DEBUG_TRACE("void CPlayerInfoSA::CancelPlayerEnteringCars( CVehicle *veh )");

    DWORD dwFunction = FUNC_CancelPlayerEnteringCars;
    _asm
    {
        push    veh
        call    dwFunction
    }
}

/**
 * Make the player arrested - displays "Busted" text are they are respawned at the nearest police station
 * @see CRestartSA::OverrideNextRestart
 * @see CRestartSA::AddPoliceRestartPoint
 */
void CPlayerInfoSA::ArrestPlayer()
{
    DEBUG_TRACE("void CPlayerInfoSA::ArrestPlayer()");

    DWORD dwFunction = FUNC_ArrestPlayer;
    _asm
    {
        call    dwFunction
    }
}

/**
 * Kill the player - displays "Wasted" text are they are respawned at the nearest hospital station
 * @see CRestartSA::OverrideNextRestart
 * @see CRestartSA::AddHospitalRestartPoint
 */
void CPlayerInfoSA::KillPlayer()
{
    DEBUG_TRACE("void CPlayerInfoSA::KillPlayer()");

    DWORD dwFunction = FUNC_KillPlayer;
    _asm
    {
        call    dwFunction
    }
}

/**
 * Creates a remote controlled vehicle of a specific type at a specified point
 * @param vehicletype This is the type of vehicle to create
 * @return CVehicle * for the created remote controlled vehicle
 */
CVehicle* CPlayerInfoSA::GiveRemoteVehicle( unsigned short model, float x, float y, float z )
{
    DEBUG_TRACE("CVehicleSA* CPlayerInfoSA::GivePlayerRemoteVehicle( unsigned short mode, float x, float y, float z )");

    CModelInfoSA *info = pGame->GetModelInfo( model );

    if ( !info || !info->IsVehicle() )
        return NULL;

    DWORD dwFunction = FUNC_GivePlayerRemoteControlledCar;
    _asm
    {
        push    model
        push    0   // rotation
        push    x
        push    y
        push    z
        call    dwFunction
        add     esp, 0x14
    }

    return GetRemoteVehicle();
}

void CPlayerInfoSA::StopRemoteControl()
{
    DEBUG_TRACE("void CPlayerInfoSA::StopRemoteControl()");

    DWORD dwFunction = FUNC_TakeRemoteControlledCarFromPlayer;
    _asm
    {
        push    ecx
        push    1
        call    dwFunction
        pop     ecx
    }
}

CVehicle* CPlayerInfoSA::GetRemoteVehicle()
{
    DEBUG_TRACE("CVehicleSA* CPlayerInfoSA::GetRemoteVehicle()");

    return pGame->GetPools()->GetVehicle( (CVehicleSAInterface*)VAR_PlayerRCCar );
}

float CPlayerInfoSA::GetFPSMoveHeading()
{
    return 0;//m_interface->m_pedData.m_fFPSMoveHeading;
}

bool CPlayerInfoSA::GetDoesNotGetTired()
{
    return m_interface->m_awardNoTiredness;
}

void CPlayerInfoSA::SetDoesNotGetTired( bool award )
{
    m_interface->m_awardNoTiredness = award;
}

unsigned short CPlayerInfoSA::GetLastTimeEaten()
{
    return m_interface->m_starvingTimer;
}

void CPlayerInfoSA::SetLastTimeEaten( unsigned short timer )
{
    m_interface->m_starvingTimer = timer;
}

unsigned int CPlayerInfoSA::GetLastTimeBigGunFired()
{
    return m_interface->m_vehicleFireTimer;
}

void CPlayerInfoSA::SetLastTimeBigGunFired( unsigned int time )
{
    m_interface->m_vehicleFireTimer = time;
}

void* CPlayerPedDataSAInterface::operator new( size_t )
{
    return mtaPlayerDataPool->Allocate();
}

void CPlayerPedDataSAInterface::operator delete( void *ptr )
{
    mtaPlayerDataPool->Free( (CPlayerPedDataSAInterface*)ptr );
}