/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.0
*               (Shared logic for modifications)
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        mods/shared_logic/CClientPad.cpp
*  PURPOSE:     Ped entity controller pad handler class
*  DEVELOPERS:  Jax <>
*
*****************************************************************************/

/* This class is used for setting controller states on script-created peds,
   through scripting functions.
*/

#include <StdInc.h>

const char * g_AnalogGTAControls [ MAX_GTA_ANALOG_CONTROLS ] =
{
    "left",
    "right",
    "forwards",
    "backwards",
    "vehicle_left",
    "vehicle_right",
    "steer_forward",
    "steer_back",
    "accelerate",
    "brake_reverse",
    "special_control_left",
    "special_control_right",
    "special_control_up",
    "special_control_down",
};


bool CClientPad::GetControlIndex( const char *name, unsigned int& index )
{ 
    return g_pCore->GetKeyBinds()->GetBindableIndex( name, (eBindableControl&)index );
}

const char* CClientPad::GetControlName( unsigned int index )
{
    SBindableGTAControl *cntrl = g_pCore->GetKeyBinds()->GetBindable( (eBindableControl)index );

    if ( !cntrl )
        return NULL;

    return cntrl->szControl;
}

CClientPad::CClientPad()
{
    memset( m_states, 0, sizeof( m_states ) );
}

bool CClientPad::GetControlState( eBindableControl control )
{
    return m_states[control];
}

bool CClientPad::GetControlState( const char *name, bool& state )
{
    unsigned int index;

    if ( !GetControlIndex( szName, index ) )
        return false;

    state = m_states[index];
    return true;
}


bool CClientPad::SetControlState( const char *name, bool state )
{
    unsigned int index;

    if ( !GetControlIndex( name, index ) )
        return false;

    m_states[index] = state;
    return true;
}

bool CClientPad::GetControlState( const char *name, CControllerState& state, bool onFoot )
{
    unsigned int index;

    if ( !GetControlIndex( name, index ) )
        return false;

    if ( onFoot )
    {
        switch( index )
        {
        case CONTROL_FIRE: return state.IsCircleDown();
        case CONTROL_NEXT_WEAPON: return state.IsRightShoulder1Down();
        case CONTROL_PREVIOUS_WEAPON: return state.IsLeftShoulder2Down();
        case CONTROL_FORWARDS: return state.m_leftAxisY == -128;
        case CONTROL_BACKWARDS: return state.m_leftAxisY == 128;
        case CONTROL_LEFT: return state.m_leftAxisX == -128;
        case CONTROL_RIGHT: return state.m_leftAxisX == 128;
        case CONTROL_ZOOM_IN: return state.IsRightShoulder2Down();
        case CONTROL_ZOOM_OUT: return state.IsLeftShoulder2Down();
        case CONTROL_ENTER_EXIT: return false;
        case CONTROL_CHANGE_CAMERA: return false;
        case CONTROL_JUMP: return state.IsSquareDown();
        case CONTROL_SPRINT: return state.IsCrossDown();
        case CONTROL_LOOK_BEHIND: return state.m_action6 = 255;
        case CONTROL_CROUCH: return state.m_action5 == 255;
        case CONTROL_ACTION: return state.IsLeftShoulder1Down();
        case CONTROL_WALK: return state.m_pedWalk > 0;
        // vehicle keys
        case CONTROL_AIM_WEAPON: return state.IsRightShoulder1Down();
        case CONTROL_CONVERSATION_YES: return state.m_digitalRight == 255;
        case CONTROL_CONVERSATION_NO: return state.m_digitalLeft == 255;
        case CONTROL_GROUP_CONTROL_FORWARDS: return state.m_digitalUp == 255;
        case CONTROL_GROUP_CONTROL_BACK: return state.m_digitalDown == 255;
        }
        return false;
    }
 
    switch( index )
    {
    case CONTROL_VEHICLE_FIRE: return state.IsCircleDown();
    case CONTROL_VEHICLE_SECONDARY_FIRE: return state.IsLeftShoulder1Down();
    case CONTROL_VEHICLE_LEFT: return state.m_leftAxisX == -128;
    case CONTROL_VEHICLE_RIGHT: return state.m_leftAxisX == 128;
    case CONTROL_STEER_FORWARD: return state.m_leftAxisY == -128;
    case CONTROL_STEER_BACK: return state.m_leftAxisY == 128;
    case CONTROL_ACCELERATE: return state.IsCrossDown();
    case CONTROL_BRAKE_REVERSE: return state.IsSquareDown();
    case CONTROL_RADIO_NEXT: return state.m_digitalUp == 255;
    case CONTROL_RADIO_PREVIOUS: return state.m_digitalDown == 255;
    case CONTROL_RADIO_USER_TRACK_SKIP: return state.m_radioTrackSkip == 255;
    case CONTROL_HORN: return state.m_action5 == 255;
    case CONTROL_SUB_MISSION: return state.m_action6 == 255;
    case CONTROL_HANDBRAKE: return state.IsRightShoulder1Down();
    case CONTROL_VEHICLE_LOOK_LEFT: return state.IsLeftShoulder2Down();
    case CONTROL_VEHICLE_LOOK_RIGHT: return state.IsRightShoulder2Down();
    case CONTROL_VEHICLE_LOOK_BEHIND: return false;
    case CONTROL_VEHICLE_MOUSE_LOOK: return false;
    case CONTROL_SPECIAL_CONTROL_LEFT: return state.m_rightAxisX == 128;
    case CONTROL_SPECIAL_CONTROL_RIGHT: return state.m_rightAxisX == -128;
    case CONTROL_SPECIAL_CONTROL_DOWN: return state.m_rightAxisY == 128;
    case CONTROL_SPECIAL_CONTROL_UP: return state.m_rightAxisY == -128;
    }

    return false;
}

bool CClientPad::GetAnalogControlIndex( const char *name, unsigned int& index )
{
    for ( unsigned int i = 0 ; i < MAX_GTA_ANALOG_CONTROLS ; i++ )
    {
        if ( !stricmp( g_AnalogGTAControls[i], name ) )
        {
            index = i;
            return true;
        }
    }
    return false;
}

bool CClientPad::GetAnalogControlState( const char *name, CControllerState& cs, bool onFoot, float& state )
{
    unsigned int index;

    if ( !GetAnalogControlIndex( name, index ) )
        return false;

    if ( onFoot )
    {
        switch( index )
        {
        case 0: state = cs.m_leftAxisX < 0 ? cs.m_leftAxisX/-128.0f : 0 ; return true;  //Left
        case 1: state = cs.m_leftAxisX > 0 ? cs.m_leftAxisX/128.0f : 0 ; return true;  //Right
        case 2: state = cs.m_leftAxisY < 0 ? cs.m_leftAxisY/-128.0f : 0 ; return true;  //Up
        case 3: state = cs.m_leftAxisY > 0 ? cs.m_leftAxisY/128.0f : 0 ; return true;  //Down
        }
        return false;
    }

    switch( index )
    { 
    case 4: state = cs.m_leftAxisX  < 0 ? cs.m_leftAxisX/-128.0f : 0 ; return true;  //Left
    case 5: state = cs.m_leftAxisX  > 0 ? cs.m_leftAxisX/128.0f : 0 ; return true;  //Right 
    case 6: state = cs.m_leftAxisY  < 0 ? cs.m_leftAxisY/-128.0f : 0 ; return true;  //Up
    case 7: state = cs.m_leftAxisY  > 0 ? cs.m_leftAxisY/128.0f : 0 ; return true;  //Down
    case 8: state = cs.m_action3 > 0 ? cs.m_action3/255.0f : 0 ; return true;  //Accel
    case 9: state = cs.m_action1 > 0 ? cs.m_action1/255.0f : 0 ; return true;  //Reverse
    case 10: state = cs.m_rightAxisX < 0 ? cs.m_rightAxisX/-128.0f : 0 ; return true;  //Special Left
    case 11: state = cs.m_rightAxisX > 0 ? cs.m_rightAxisX/128.0f : 0 ; return true;  //Special Right
    case 12: state = cs.m_rightAxisY < 0 ? cs.m_rightAxisY/-128.0f : 0 ; return true;  //Special Up
    case 13: state = cs.m_rightAxisY > 0 ? cs.m_rightAxisY/128.0f : 0 ; return true;  //Special Down
    }

    return false;
}
