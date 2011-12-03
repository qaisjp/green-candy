/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        game_sa/CPadManagerSA.cpp
*  PURPOSE:     Joypad management
*  DEVELOPERS:  The_GTA <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#include "StdInc.h"

CPadManagerSA::CPadManagerSA()
{
    m_keys = core->GetKeyBinds();
}

CPadManagerSA::~CPadManagerSA()
{

}

CPadSAInterface* CPadManagerSA::GetJoypad( unsigned int index )
{
    if ( index != 0 )
        return NULL;

    return (CPadSAInterface*)0x00B73458;
}

void CPadManagerSA::GetFootControl( CControllerState& cs )
{
    bool detonator = ped->GetCurrentWeaponSlot() == 12;
    bool aimingWeapon = m_keys->GetControlState( CONTROL_AIM_WEAPON );

    cs.m_action4 = DIGITAL_BUTTON( m_keys->GetControlState( CONTROL_FIRE ) && !detonator );
    cs.m_rs2 = DIGITAL_BUTTON( m_keys->GetControlState( CONTROL_NEXT_WEAPON ) || ( aimingWeapon && m_keys->GetControlState( CONTROL_ZOOM_IN ) ) );
    cs.m_ls2 = DIGITAL_BUTTON( m_keys->GetControlState( CONTROL_PREVIOUS_WEAPON ) || ( aimingWeapon && m_keys->GetControlState( CONTROL_ZOOM_OUT ) ) );
    cs.m_leftAxisY = DIGITAL_AXIS( m_keys->GetControlState( CONTROL_FORWARDS ), m_keys->GetControlState( CONTROL_BACKWARDS ) );
    cs.m_leftAxisX = DIGITAL_AXIS( m_keys->GetControlState( CONTROL_LEFT ), m_keys->GetControlState( CONTROL_RIGHT ) );

    cs.m_action1 = DIGITAL_BUTTON( !bEnteringVehicle && m_keys->GetControlState( CONTROL_JUMP ) );
    cs.m_action3 = DIGITAL_BUTTON( m_keys->GetControlState( CONTROL_SPRINT ) );
    cs.m_action6 = DIGITAL_BUTTON( m_keys->GetControlState( CONTROL_LOOK_BEHIND ) );
    cs.m_action5 = DIGITAL_BUTTON( m_keys->GetControlState( CONTROL_CROUCH ) );
    cs.m_ls1 = DIGITAL_BUTTON( m_keys->GetControlState( CONTROL_ACTION ) );
    cs.m_pedWalk = DIGITAL_BUTTON( m_keys->GetControlState( CONTROL_WALK ) );

    cs.m_rs1 = DIGITAL_BUTTON( m_keys->GetControlState( CONTROL_AIM_WEAPON ) );
    cs.m_digitalRight = DIGITAL_BUTTON( m_keys->GetControlState( CONTROL_CONVERSATION_YES ) );
    cs.m_digitalLeft = DIGITAL_BUTTON( m_keys->GetControlState( CONTROL_CONVERSATION_NO ) );
    cs.m_digitalUp = DIGITAL_BUTTON( m_keys->GetControlState( CONTROL_GROUP_CONTROL_FORWARDS ) );
    cs.m_digitalDown = DIGITAL_BUTTON( m_keys->GetControlState( CONTROL_GROUP_CONTROL_BACK ) );
}

void CPadManagerSA::GetVehicleControl( CControllerState& cs )
{
    cs.m_action4 = DIGITAL_BUTTON( m_keys->GetControlState( CONTROL_VEHICLE_FIRE ) );
    cs.m_ls1 = DIGITAL_BUTTON( m_keys->GetControlState( CONTROL_VEHICLE_SECONDARY_FIRE ) );
    cs.m_leftAxisX = DIGITAL_AXIS( m_keys->GetControlState( CONTROL_LEFT ), m_keys->GetControlState( CONTROL_RIGHT ) );
    cs.m_leftAxisY = DIGITAL_AXIS( m_keys->GetControlState( CONTROL_STEER_FORWARDS ), m_keys->GetControlState( CONTROL_STEER_BACK ) );

    cs.m_action3 = DIGITAL_BUTTON( m_keys->GetControlState( CONTROL_ACCELERATE ) );
    cs.m_action1 = DIGITAL_BUTTON( m_keys->GetControlState( CONTROL_BRAKE_REVERSE ) );
    cs.m_digitalUp = DIGITAL_BUTTON( m_keys->GetControlState( CONTROL_RADIO_NEXT ) );
    cs.m_digitalDown = DIGITAL_BUTTON( m_keys->GetControlState( CONTROL_RADIO_PREVIOUS ) );
    cs.m_radioTrackSkip = DIGITAL_BUTTON( m_keys->GetControlState( CONTROL_RADIO_USER_TRACK_SKIP ) );
    cs.m_action5 = DIGITAL_BUTTON( m_keys->GetControlState( CONTROL_HORN ) );
    cs.m_action6 = DIGITAL_BUTTON( m_keys->GetControlState( CONTROL_SUB_MISSION ) );
    cs.m_rs1 = DIGITAL_BUTTON( m_keys->GetControlState( CONTROL_HANDBRAKE ) );
    cs.m_ls2 = DIGITAL_BUTTON( m_keys->GetControlState( CONTROL_VEHICLE_LOOK_LEFT ) || m_keys->GetControlState( CONTROL_VEHICLE_LOOK_BEHIND ) );
    cs.m_rs2 = DIGITAL_BUTTON( m_keys->GetControlState( CONTROL_VEHICLE_LOOK_RIGHT ) || m_keys->GetControlState( CONTROL_VEHICLE_LOOK_BEHIND ) );

    cs.m_rightAxisX = DIGITAL_AXIS( m_keys->GetControlState( CONTROL_SPECIAL_CONTROL_LEFT ), m_keys->GetControlState( CONTROL_SPECIAL_CONTROL_RIGHT ) );
    cs.m_rightAxisY = DIGITAL_AXIS( m_keys->GetControlState( CONTROL_SPECIAL_CONTROL_DOWN ), m_keys->GetControlState( CONTROL_SPECIAL_CONTROL_UP ) );
}

void CPadManagerSA::UpdateLocalJoypad( CPedSA *ped )
{
    bool inVehicle = ped->GetVehicle() != NULL;
    bool enteringVehicle = ped->IsEnteringVehicle();
    CPadSAInterface *pad = ped->GetInterface()->GetJoypad();

    // Retrive the current controls
    CControllerState cs;

    if ( !ped->IsDead() )
    {
        if ( !inVehicle )
            GetFootControl( cs );
        else
            GetVehicleControl( cs );
    }

    // Global controls
    cs.m_action2 = DIGITAL_BUTTON( m_keys->GetControlState( CONTROL_ENTER_EXIT ) );
    cs.m_select = DIGITAL_BUTTON( m_keys->GetControlState( CONTROL_CHANGE_CAMERA ) );  

    GetJoystickManager ()->ApplyAxes ( cs, bInVehicle );

    m_pCore->GetGame ()->GetPad ()->SetCurrentControllerState ( &cs );

    // Sirens
    pad->SetHornHistory( cs.m_action5 == 255 );
}