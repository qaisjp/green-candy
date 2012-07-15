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


bool CClientPad::GetControlIndex ( const char * szName, unsigned int & uiIndex )
{ 
    return g_pCore->GetKeyBinds()->GetBindableIndex( szName, (eBindableControl&)uiIndex );
}

const char * CClientPad::GetControlName ( unsigned int uiIndex )
{
    SBindableGTAControl *cntrl = g_pCore->GetKeyBinds()->GetBindable( (eBindableControl)uiIndex );

    if ( !cntrl )
        return NULL;

    return cntrl->szControl;
}


CClientPad::CClientPad ( void )
{
    memset ( m_bStates, 0, sizeof ( m_bStates ) );
}


bool CClientPad::GetControlState ( const char * szName, bool & bState )
{
    unsigned int uiIndex;
    if ( GetControlIndex ( szName, uiIndex ) )
    {
        bState = m_bStates [ uiIndex ];
        return true;
    }
    return false;
}


bool CClientPad::SetControlState ( const char * szName, bool bState )
{
    unsigned int uiIndex;
    if ( GetControlIndex ( szName, uiIndex ) )
    {
        m_bStates [ uiIndex ] = bState;
        return true;
    }
    return false;
}

bool CClientPad::GetControlState ( const char * szName, CControllerState & State, bool bOnFoot )
{
    unsigned int uiIndex;
    if ( GetControlIndex ( szName, uiIndex ) )
    {
        if ( bOnFoot )
        {
            switch ( uiIndex )
            {
            case CONTROL_FIRE: return State.IsCircleDown();
            case CONTROL_NEXT_WEAPON: return State.IsRightShoulder1Down();
            case CONTROL_PREVIOUS_WEAPON: return State.IsLeftShoulder2Down();
            case CONTROL_FORWARDS: return State.m_leftAxisY == -128;
            case CONTROL_BACKWARDS: return State.m_leftAxisY == 128;
            case CONTROL_LEFT: return State.m_leftAxisX == -128;
            case CONTROL_RIGHT: return State.m_leftAxisX == 128;
            case CONTROL_ZOOM_IN: return State.IsRightShoulder2Down();
            case CONTROL_ZOOM_OUT: return State.IsLeftShoulder2Down();
            case CONTROL_ENTER_EXIT: return false;
            case CONTROL_CHANGE_CAMERA: return false;
            case CONTROL_JUMP: return State.IsSquareDown();
            case CONTROL_SPRINT: return State.IsCrossDown();
            case CONTROL_LOOK_BEHIND: return State.m_action6 = 255;
            case CONTROL_CROUCH: return State.m_action5 == 255;
            case CONTROL_ACTION: return State.IsLeftShoulder1Down();
            case CONTROL_WALK: return State.m_pedWalk > 0;
            // vehicle keys
            case CONTROL_AIM_WEAPON: return State.IsRightShoulder1Down();
            case CONTROL_CONVERSATION_YES: return State.m_digitalRight == 255;
            case CONTROL_CONVERSATION_NO: return State.m_digitalLeft == 255;
            case CONTROL_GROUP_CONTROL_FORWARDS: return State.m_digitalUp == 255;
            case CONTROL_GROUP_CONTROL_BACK: return State.m_digitalDown == 255;
            }
        }
        else
        {
            switch ( uiIndex )
            {
            case CONTROL_VEHICLE_FIRE: return State.IsCircleDown();
            case CONTROL_VEHICLE_SECONDARY_FIRE: return State.IsLeftShoulder1Down();
            case CONTROL_VEHICLE_LEFT: return State.m_leftAxisX == -128;
            case CONTROL_VEHICLE_RIGHT: return State.m_leftAxisX == 128;
            case CONTROL_STEER_FORWARD: return State.m_leftAxisY == -128;
            case CONTROL_STEER_BACK: return State.m_leftAxisY == 128;
            case CONTROL_ACCELERATE: return State.IsCrossDown();
            case CONTROL_BRAKE_REVERSE: return State.IsSquareDown();
            case CONTROL_RADIO_NEXT: return State.m_digitalUp == 255;
            case CONTROL_RADIO_PREVIOUS: return State.m_digitalDown == 255;
            case CONTROL_RADIO_USER_TRACK_SKIP: return State.m_radioTrackSkip == 255;
            case CONTROL_HORN: return State.m_action5 == 255;
            case CONTROL_SUB_MISSION: return State.m_action6 == 255;
            case CONTROL_HANDBRAKE: return State.IsRightShoulder1Down();
            case CONTROL_VEHICLE_LOOK_LEFT: return State.IsLeftShoulder2Down();
            case CONTROL_VEHICLE_LOOK_RIGHT: return State.IsRightShoulder2Down();
            case CONTROL_VEHICLE_LOOK_BEHIND: return false;
            case CONTROL_VEHICLE_MOUSE_LOOK: return false;
            case CONTROL_SPECIAL_CONTROL_LEFT: return State.m_rightAxisX == 128;
            case CONTROL_SPECIAL_CONTROL_RIGHT: return State.m_rightAxisX == -128;
            case CONTROL_SPECIAL_CONTROL_DOWN: return State.m_rightAxisY == 128;
            case CONTROL_SPECIAL_CONTROL_UP: return State.m_rightAxisY == -128;
            }
        }
    }
    return false;
}

bool CClientPad::GetAnalogControlIndex ( const char * szName, unsigned int & uiIndex )
{
    for ( unsigned int i = 0 ; i < MAX_GTA_ANALOG_CONTROLS ; i++ )
    {
        if ( !stricmp ( g_AnalogGTAControls [ i ], szName ) )
        {
            uiIndex = i;
            return true;
        }
    }
    return false;
}


bool CClientPad::GetAnalogControlState ( const char * szName, CControllerState & cs, bool bOnFoot, float & fState )
{
    unsigned int uiIndex;
    if ( GetAnalogControlIndex ( szName, uiIndex ) )
    {       
        if ( bOnFoot )
        {
            switch ( uiIndex )
            {
            case 0: fState = cs.m_leftAxisX < 0 ? cs.m_leftAxisX/-128.0f : 0 ; return true;  //Left
            case 1: fState = cs.m_leftAxisX > 0 ? cs.m_leftAxisX/128.0f : 0 ; return true;  //Right
            case 2: fState = cs.m_leftAxisY < 0 ? cs.m_leftAxisY/-128.0f : 0 ; return true;  //Up
            case 3: fState = cs.m_leftAxisY > 0 ? cs.m_leftAxisY/128.0f : 0 ; return true;  //Down
            default: fState = 0; return true;
            }
            
        }
        else
        {
            switch ( uiIndex )
            { 
            case 4: fState = cs.m_leftAxisX  < 0 ? cs.m_leftAxisX/-128.0f : 0 ; return true;  //Left
            case 5: fState = cs.m_leftAxisX  > 0 ? cs.m_leftAxisX/128.0f : 0 ; return true;  //Right 
            case 6: fState = cs.m_leftAxisY  < 0 ? cs.m_leftAxisY/-128.0f : 0 ; return true;  //Up
            case 7: fState = cs.m_leftAxisY  > 0 ? cs.m_leftAxisY/128.0f : 0 ; return true;  //Down
            case 8: fState = cs.m_action3 > 0 ? cs.m_action3/255.0f : 0 ; return true;  //Accel
            case 9: fState = cs.m_action1 > 0 ? cs.m_action1/255.0f : 0 ; return true;  //Reverse
            case 10: fState = cs.m_rightAxisX < 0 ? cs.m_rightAxisX/-128.0f : 0 ; return true;  //Special Left
            case 11: fState = cs.m_rightAxisX > 0 ? cs.m_rightAxisX/128.0f : 0 ; return true;  //Special Right
            case 12: fState = cs.m_rightAxisY < 0 ? cs.m_rightAxisY/-128.0f : 0 ; return true;  //Special Up
            case 13: fState = cs.m_rightAxisY > 0 ? cs.m_rightAxisY/128.0f : 0 ; return true;  //Special Down
            default: fState = 0; return true;
            }
        }
    }
    return false;
}
