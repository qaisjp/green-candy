/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*               (Shared logic for modifications)
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        mods/shared_logic/lua/CLuaFunctionDefs.Input.cpp
*  PURPOSE:     Lua function definitions class
*  DEVELOPERS:  Ed Lyons <eai@opencoding.net>
*               Jax <>
*               Cecill Etheredge <ijsf@gmx.net>
*               Kevin Whiteside <kevuwk@gmail.com>
*               Chris McArthur <>
*               Derek Abdine <>
*               Christian Myhre Lundheim <>
*               Stanislav Bobrov <lil_toady@hotmail.com>
*               Alberto Alonso <rydencillo@gmail.com>
*               Florian Busse <flobu@gmx.net>
*               The_GTA <quiret@gmx.de>
*
*****************************************************************************/

#include "StdInc.h"

namespace CLuaFunctionDefs
{
    LUA_DECLARE( getCursorPosition )
    {
        CVector2D vecCursor;
        CVector vecWorld;
        if ( CStaticFunctionDefinitions::GetCursorPosition ( vecCursor, vecWorld ) )
        {
            lua_pushnumber ( L, ( double ) vecCursor.fX );
            lua_pushnumber ( L, ( double ) vecCursor.fY );
            lua_pushnumber ( L, ( double ) vecWorld.fX );
            lua_pushnumber ( L, ( double ) vecWorld.fY );
            lua_pushnumber ( L, ( double ) vecWorld.fZ );
            return 5;
        }
        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( setCursorPosition )
    {
        float x, y;

        if ( lua_isnumber( L, 1 ) )
        {
            if ( lua_isnumber( L, 2 ) )
            {
                HWND hookedWindow = g_pCore->GetHookedWindow ();

                tagPOINT windowPos = { 0 };
                ClientToScreen( hookedWindow, &windowPos );

                CVector2D vecResolution = g_pCore->GetGUI ()->GetResolution ();

                float fX = static_cast < float > ( lua_tonumber ( L, 1 ) );
                float fY = static_cast < float > ( lua_tonumber ( L, 2 ) );

                if ( fX < 0 )
                    fX = 0.0f;
                else if ( fX > vecResolution.fX )
                    fX = vecResolution.fX;
                if ( fY < 0 )
                    fY = 0.0f;
                else if ( fY > vecResolution.fY )
                    fY = vecResolution.fY;

                g_pCore->CallSetCursorPos ( ( int ) fX + ( int ) windowPos.x, ( int ) fY + ( int ) windowPos.y );
                lua_pushboolean ( L, true );
                return 1;
            }
            else
                m_pScriptDebugging->LogBadPointer( __FUNCTION__, "float", 2 );
        }
        else
            m_pScriptDebugging->LogBadPointer( __FUNCTION__, "float", 1 );

        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( isCursorShowing )
    {
        bool bShowing = false;
        if ( CStaticFunctionDefinitions::IsCursorShowing ( bShowing ) )
        {
            lua_pushboolean ( L, bShowing );
            return 1;
        }
        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( showCursor )
    {
        // Get the VM
        CLuaMain* pLuaMain = lua_readcontext( L );

        // Boolean type passed?
        if ( lua_type ( L, 1 ) == LUA_TBOOLEAN )
        {
            // Grab the argument
            bool bShow = lua_toboolean ( L, 1 ) ?true:false;
            bool bToggleControls = true;
            if ( lua_type ( L, 2 ) == LUA_TBOOLEAN )
                bToggleControls = ( lua_toboolean ( L, 2 ) ) ? true:false;

            // Grab the resource belonging to this VM
            CResource* pResource = pLuaMain->GetResource ();

            // Show/hide it inside that resource
            pResource->ShowCursor ( bShow, bToggleControls );
            lua_pushboolean ( L, true );
            return 1;
        }
        else
            m_pScriptDebugging->LogBadType( "showCursor" );

        // Fail
        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( bindKey )
    {
        CLuaMain* pLuaMain = lua_readcontext( L );

        if ( lua_type ( L, 1 ) == LUA_TSTRING && lua_type ( L, 2 ) == LUA_TSTRING )
        {
            const char* szKey = lua_tostring ( L, 1 );
            const char* szHitState = lua_tostring ( L, 2 );

            if ( lua_type ( L, 3 ) == LUA_TSTRING )
            {
                const char* szResource = pLuaMain->GetResource()->GetName();
                const char* szCommand = lua_tostring ( L, 3 );
                const char* szArguments = "";
                if  ( lua_type ( L, 4 ) == LUA_TSTRING )
                    szArguments = lua_tostring ( L, 4 );
                if ( CStaticFunctionDefinitions::BindKey ( szKey, szHitState, szCommand, szArguments, szResource ) )
                {
                    lua_pushboolean ( L, true );
                    return 1;
                }  
            }
            else
            { 
                CLuaArguments Arguments;
                Arguments.ReadArguments ( L, 4 );
                LuaFunctionRef iLuaFunction = pLuaMain->CreateReference( 3 );            

                if ( VERIFY_FUNCTION ( iLuaFunction ) )
                {
                    if ( CStaticFunctionDefinitions::BindKey ( szKey, szHitState, pLuaMain, iLuaFunction, Arguments ) )
                    {
                        lua_pushboolean ( L, true );
                        return 1;
                    }
                }
                else
                    m_pScriptDebugging->LogBadPointer( "bindKey", "function", 3 );
            }
        }

        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( unbindKey )
    {
        CLuaMain* pLuaMain = lua_readcontext( L );

        if ( lua_type ( L, 1 ) == LUA_TSTRING )
        {
            const char* szKey = lua_tostring ( L, 1 );
            const char* szHitState = NULL;

            if ( lua_type ( L, 2 ) )
                szHitState = lua_tostring ( L, 2 );

            if ( lua_type ( L, 3 ) == LUA_TSTRING )
            {
                if ( CStaticFunctionDefinitions::UnbindKey ( szKey, szHitState, pLuaMain->GetResource()->GetName(), lua_tostring( L, 3 ) ) )
                {
                    lua_pushboolean ( L, true );
                    return 1;
                }
            }
            else
            {   
                LuaFunctionRef iLuaFunction;
                if ( lua_type ( L, 3 ) == LUA_TFUNCTION )
                    iLuaFunction = pLuaMain->CreateReference( 3 );

                if ( IS_REFNIL ( iLuaFunction ) || VERIFY_FUNCTION ( iLuaFunction ) )
                {
                    if ( CStaticFunctionDefinitions::UnbindKey ( szKey, pLuaMain, szHitState, iLuaFunction ) )
                    {
                        lua_pushboolean ( L, true );
                        return 1;
                    }
                }
                else
                {
                    m_pScriptDebugging->LogBadType( "unbindKey" );
                }
            }
        }
        else
        {
            m_pScriptDebugging->LogBadType( "unbindKey" );
        }

        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( getKeyState )
    {
        if ( lua_istype ( L, 1, LUA_TSTRING ) )
        {
            const char* szKey = lua_tostring ( L, 1 );
            bool bState;
            if ( CStaticFunctionDefinitions::GetKeyState ( szKey, bState ) )
            {
                lua_pushboolean ( L, bState );
                return 1;
            }
        }
        else
            m_pScriptDebugging->LogBadType( "getKeyType" );

        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( getControlState )
    {
        if ( lua_istype ( L, 1, LUA_TSTRING ) )
        {
            const char* szControl = lua_tostring ( L, 1 );
            bool bState;
            if ( CStaticFunctionDefinitions::GetControlState ( szControl , bState ) )
            {
                lua_pushboolean ( L, bState );
                return 1;
            }
        }
        else
            m_pScriptDebugging->LogBadType( "getControlType" );

        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( getAnalogControlState )
    {
        if ( lua_istype ( L, 1, LUA_TSTRING ) )
        {
            const char* szControl = lua_tostring ( L, 1 );
            float fState;
            if ( CStaticFunctionDefinitions::GetAnalogControlState ( szControl , fState ) )
            {
                lua_pushnumber ( L, fState );
                return 1;
            }
        }
        else
            m_pScriptDebugging->LogBadType( "getAnalogControlState" );

        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( isControlEnabled )
    {
        if ( lua_istype ( L, 1, LUA_TSTRING ) )
        {
            const char* szControl = lua_tostring ( L, 1 );
            bool bEnabled;
            if ( CStaticFunctionDefinitions::IsControlEnabled ( szControl, bEnabled ) )
            {
                lua_pushboolean ( L, bEnabled );
                return 1;
            }
        }
        else
            m_pScriptDebugging->LogBadType( "isControlEnabled" );

        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( getBoundKeys )
    {
        if ( lua_istype ( L, 1, LUA_TSTRING ) )
        {
            const char * szControl = lua_tostring ( L, 1 );
            SBindableGTAControl * pControl = g_pCore->GetKeyBinds ()->GetBindableFromControl ( szControl );
            // Did we find a control matching the string given?
            if ( pControl )
            {
                list < CGTAControlBind * > controlBinds;
                g_pCore->GetKeyBinds ()->GetBoundControls ( pControl, controlBinds );
                if ( !controlBinds.empty () )
                {
                    lua_newtable ( L );
                    list < CGTAControlBind * > ::iterator iter = controlBinds.begin ();
                    for ( ; iter != controlBinds.end () ; iter++ )
                    {
                        lua_pushstring ( L, (*iter)->boundKey->szKey );
                        lua_pushstring ( L, "down" );
                        lua_settable ( L, -3 );
                    }
                }
                else
                    lua_pushboolean ( L, false );
                return 1;
            }
            // If not, assume its a command
            else
            {
                list < CCommandBind * > commandBinds;
                g_pCore->GetKeyBinds ()->GetBoundCommands ( szControl, commandBinds );
                if ( !commandBinds.empty () )
                {
                    lua_newtable ( L );
                    list < CCommandBind * > ::iterator iter = commandBinds.begin ();
                    for ( ; iter != commandBinds.end () ; iter++ )
                    {
                        lua_pushstring ( L, (*iter)->boundKey->szKey );
                        lua_pushstring ( L, (*iter)->bHitState ? "down" : "up" );
                        lua_settable ( L, -3 );
                    }
                }
                else
                    lua_pushboolean ( L, false );
                return 1;
            }
        }
        else
            m_pScriptDebugging->LogBadType( "getBoundKeys" );

        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( getFunctionsBoundToKey )
    {
        CLuaMain* pLuaMain = lua_readcontext( L );

        if ( lua_type ( L, 1 ) == LUA_TSTRING )
        {
            const char* szKey = lua_tostring ( L, 1 );
            const char* szHitState = NULL;
            if ( lua_type ( L, 2 ) )
            {
                szHitState = lua_tostring ( L, 2 );
            }

            bool bCheckHitState = false, bHitState = true;
            if ( szHitState )
            {
                if ( stricmp ( szHitState, "down" ) == 0 )
                    bCheckHitState = true, bHitState = true;
                else if ( stricmp ( szHitState, "up" ) == 0 )
                    bCheckHitState = true, bHitState = false;
            }

            // Create a new table
            lua_newtable ( L );

            // Add all the bound functions to it
            unsigned int uiIndex = 0;
            list < CScriptKeyBind* > ::const_iterator iter = m_pClientGame->GetScriptKeyBinds ()->IterBegin ();
            for ( ; iter != m_pClientGame->GetScriptKeyBinds ()->IterEnd (); iter++ )
            {
                CScriptKeyBind* pScriptKeyBind = *iter;
                if ( !pScriptKeyBind->IsBeingDeleted () )
                {
                    switch ( pScriptKeyBind->GetType () )
                    {
                        case SCRIPT_KEY_BIND_FUNCTION:
                        {
                            CScriptKeyFunctionBind* pBind = static_cast < CScriptKeyFunctionBind* > ( pScriptKeyBind );
                            if ( !bCheckHitState || pBind->bHitState == bHitState )
                            {
                                if ( strcmp ( szKey, pBind->boundKey->szKey ) == 0 )
                                {
                                    lua_pushnumber ( L, ++uiIndex );
                                    lua_rawgeti ( L, LUA_REGISTRYINDEX, pBind->m_iLuaFunction.ToInt () );
                                    lua_settable ( L, -3 );
                                }
                            }
                            break;
                        }
                        case SCRIPT_KEY_BIND_CONTROL_FUNCTION:
                        {
                            CScriptControlFunctionBind* pBind = static_cast < CScriptControlFunctionBind* > ( pScriptKeyBind );
                            if ( !bCheckHitState || pBind->bHitState == bHitState )
                            {
                                if ( strcmp ( szKey, pBind->boundControl->szControl ) == 0 )
                                {
                                    lua_pushnumber ( L, ++uiIndex );
                                    lua_rawgeti ( L, LUA_REGISTRYINDEX, pBind->m_iLuaFunction.ToInt () );
                                    lua_settable ( L, -3 );
                                }
                            }
                            break;
                        }
                        default:
                            break;
                    }
                }
            }
            return 1;
        }
        else
            m_pScriptDebugging->LogBadType( "getFunctionsBoundToKey" );

        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( getKeyBoundToFunction )
    {
        CLuaMain* pLuaMain = lua_readcontext( L );

        if ( lua_type ( L, 1 ) == LUA_TFUNCTION || lua_type ( L, 1 ) == LUA_TSTRING )
        {
            LuaFunctionRef iLuaFunction = pLuaMain->CreateReference( 1 );
            // get the key
            list < CScriptKeyBind* > ::const_iterator iter =  m_pClientGame->GetScriptKeyBinds ()->IterBegin ();
            for ( ; iter !=  m_pClientGame->GetScriptKeyBinds ()->IterEnd (); iter++ )
            {
                CScriptKeyBind* pScriptKeyBind = *iter;
                if ( !pScriptKeyBind->IsBeingDeleted () )
                {
                    switch ( pScriptKeyBind->GetType () )
                    {
                        case SCRIPT_KEY_BIND_FUNCTION:
                        {
                            CScriptKeyFunctionBind* pBind = static_cast < CScriptKeyFunctionBind* > ( pScriptKeyBind );
                            // ACHTUNG: DOES IT FIND THE CORRECT LUA REF HERE?
                            if ( iLuaFunction == pBind->m_iLuaFunction )
                            {
                                lua_pushstring ( L, pBind->boundKey->szKey );
                                return 1;
                            }
                            break;
                        }
                        case SCRIPT_KEY_BIND_CONTROL_FUNCTION:
                        {
                            CScriptControlFunctionBind* pBind = static_cast < CScriptControlFunctionBind* > ( pScriptKeyBind );
                            // ACHTUNG: DOES IT FIND THE CORRECT LUA REF HERE?
                            if ( iLuaFunction == pBind->m_iLuaFunction )
                            {
                                lua_pushstring ( L, pBind->boundKey->szKey );
                                return 1;
                            }
                            break;
                        }
                        default:
                            break;
                    }
                }
            }
            lua_pushboolean ( L, false );
            return 1;
        }
        else
            m_pScriptDebugging->LogBadType( "getKeyBoundToFunction" );

        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( getCommandsBoundToKey )
    {
        CLuaMain* pLuaMain = lua_readcontext( L );

        if ( lua_type ( L, 1 ) == LUA_TSTRING )
        {
            const char* szKey = lua_tostring ( L, 1 );
            const char* szHitState = NULL;
            if ( lua_type ( L, 2 ) == LUA_TSTRING )
            {
                szHitState = lua_tostring ( L, 2 );
            }

            bool bCheckHitState = false, bHitState = true;
            if ( szHitState )
            {
                if ( stricmp ( szHitState, "down" ) == 0 )
                    bCheckHitState = true, bHitState = true;
                else if ( stricmp ( szHitState, "up" ) == 0 )
                    bCheckHitState = true, bHitState = false;
            }

            // Create a new table
            lua_newtable ( L );

            // Add all the bound functions to it
            unsigned int uiIndex = 0;
            list < CKeyBind* > ::const_iterator iter = g_pCore->GetKeyBinds ()->IterBegin ();
            for ( ; iter != g_pCore->GetKeyBinds ()->IterEnd (); iter++ )
            {
                CKeyBind* pKeyBind = *iter;
                if ( !pKeyBind->IsBeingDeleted () && pKeyBind->GetType () == KEY_BIND_COMMAND )
                {
                    CCommandBind* pBind = static_cast < CCommandBind* > ( pKeyBind );
                    if ( !bCheckHitState || pBind->bHitState == bHitState )
                    {
                        if ( strcmp ( szKey, pBind->boundKey->szKey ) == 0 )
                        {
                            lua_pushstring ( L, pBind->szCommand );
                            lua_pushstring ( L, pBind->szArguments );
                            lua_settable ( L, -3 );
                        }
                    }
                }
            }
            return 1;
        }
        else
            m_pScriptDebugging->LogBadType( "getFunctionsBoundToKey" );

        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( getKeyBoundToCommand )
    {
        CLuaMain* pLuaMain = lua_readcontext( L );

        if ( lua_type ( L, 1 ) == LUA_TSTRING )
        {
            const char* szCommand = lua_tostring ( L, 1 );
            // get the key
            list < CKeyBind* > ::const_iterator iter = g_pCore->GetKeyBinds ()->IterBegin ();
            for ( ; iter != g_pCore->GetKeyBinds ()->IterEnd (); iter++ )
            {
                CKeyBind* pKeyBind = *iter;
                if ( !pKeyBind->IsBeingDeleted () )
                {
                    if ( pKeyBind->GetType () == KEY_BIND_COMMAND )
                    {
                        CCommandBind* pBind = static_cast < CCommandBind* > ( pKeyBind );
                        if ( strcmp ( szCommand, pBind->szCommand ) == 0 )
                        {
                            lua_pushstring ( L, pBind->boundKey->szKey );
                            return 1;
                        }
                    }
                }
            }
            lua_pushboolean ( L, false );
            return 1;
        }
        else
            m_pScriptDebugging->LogBadType( "getKeyBoundToFunction" );

        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( setControlState )
    {
        if ( lua_istype ( L, 1, LUA_TSTRING ) && lua_istype ( L, 2, LUA_TBOOLEAN ) )
        {
            const char* szControl = lua_tostring ( L, 1 );
            bool bState = ( lua_toboolean ( L, 2 ) ) ? true:false;
            if ( CStaticFunctionDefinitions::SetControlState ( szControl, bState ) )
            {
                lua_pushboolean ( L, true );
                return 1;
            }
        }
        else
            m_pScriptDebugging->LogBadType( "setControlState" );

        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( toggleControl )
    {
        if ( lua_istype ( L, 1, LUA_TSTRING ) && lua_istype ( L, 2, LUA_TBOOLEAN ) )
        {
            const char* szControl = lua_tostring ( L, 1 );
            bool bEnabled = ( lua_toboolean ( L, 2 ) ) ? true:false;
            if ( CStaticFunctionDefinitions::ToggleControl ( szControl, bEnabled ) )
            {
                lua_pushboolean ( L, true );
                return 1;
            }
        }
        else
            m_pScriptDebugging->LogBadType( "toggleControl" );

        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( toggleAllControls )
    {
        if ( lua_type ( L, 1 ) == LUA_TBOOLEAN )
        {
            bool bEnabled = ( lua_toboolean ( L, 1 ) ) ? true:false;
            bool bGTAControls = true, bMTAControls = true;

            if ( lua_type ( L, 2 ) == LUA_TBOOLEAN )
                bGTAControls = ( lua_toboolean ( L, 2 ) ) ? true:false;

            if ( lua_type ( L, 3 ) == LUA_TBOOLEAN )
                bMTAControls = ( lua_toboolean ( L, 3 ) ) ? true:false;

            if ( CStaticFunctionDefinitions::ToggleAllControls ( bGTAControls, bMTAControls, bEnabled ) )
            {
                lua_pushboolean ( L, true );
                return 1;
            }
        }
        else
            m_pScriptDebugging->LogBadType( "toggleAllControls" );

        lua_pushboolean ( L, false );
        return 1;
    }
}