/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*               (Shared logic for modifications)
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        mods/shared_logic/lua/CLuaFunctionDefs.Event.cpp
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
*               The_GTA <quiret@gmx.de>
*
*****************************************************************************/

#include "StdInc.h"

namespace CLuaFunctionDefs
{
    LUA_DECLARE( addEvent )
    {
    //  bool addEvent ( string eventName [, bool allowRemoteTrigger = false ] )
        SString strName; bool bAllowRemoteTrigger;

        CScriptArgReader argStream ( L );
        argStream.ReadString ( strName );
        argStream.ReadBool ( bAllowRemoteTrigger, false );

        if ( !argStream.HasErrors () )
        {
            // Grab our virtual machine
            CLuaMain* pLuaMain = lua_readcontext( L );
 
            lua_pushboolean( L, CStaticFunctionDefinitions::AddEvent ( *pLuaMain, strName, bAllowRemoteTrigger ) );
            return 1;
        }
        else
            m_pScriptDebugging->LogCustom( SString ( "Bad argument @ '%s' [%s]", "addEvent", *argStream.GetErrorMessage () ) );

        // Failed
        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( addEventHandler )
    {
    //  bool addEventHandler ( string eventName, element attachedTo, function handlerFunction, [bool getPropagated = true] )
        SString strName; CClientEntity* pEntity; LuaFunctionRef iLuaFunction; bool bPropagated;

        CScriptArgReader argStream ( L );
        argStream.ReadString ( strName );
        argStream.ReadClass( pEntity, LUACLASS_ENTITY );
        argStream.ReadFunction ( iLuaFunction );
        argStream.ReadBool ( bPropagated, true );
        argStream.ReadFunctionComplete ();

        if ( !argStream.HasErrors () )
        {
            // Grab our virtual machine
            CLuaMain* pLuaMain = lua_readcontext( L );
            CMapEvent *mapEvent = pEntity->GetEventManager()->Get( strName );

            // Check if the handle is in use
            if ( mapEvent && mapEvent->GetLuaFunction() == iLuaFunction )
            {
                m_pScriptDebugging->LogCustom( 255, 0, 0, "addEventHandler: '%s' with this function is already handled", *strName );
                lua_pushboolean( L, false );
                return 1;
            }

            // Do it
            lua_pushboolean( L, CStaticFunctionDefinitions::AddEventHandler ( *pLuaMain, strName, *pEntity, iLuaFunction, bPropagated ) );
            return 1;
        }
        else
            m_pScriptDebugging->LogCustom( SString ( "Bad argument @ '%s' [%s]", "addEventHandler", *argStream.GetErrorMessage () ) );

        // Failed
        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( removeEventHandler )
    {
    //  bool removeEventHandler ( string eventName, element attachedTo, function functionVar )
        SString strName; CClientEntity* pEntity; LuaFunctionRef iLuaFunction;

        CScriptArgReader argStream ( L );
        argStream.ReadString ( strName );
        argStream.ReadClass( pEntity, LUACLASS_ENTITY );
        argStream.ReadFunction ( iLuaFunction );
        argStream.ReadFunctionComplete ();

        if ( !argStream.HasErrors () )
        {
            CLuaMain* pLuaMain = lua_readcontext( L );

            lua_pushboolean( L, CStaticFunctionDefinitions::RemoveEventHandler ( *pLuaMain, strName, *pEntity, iLuaFunction ) );
            return 1;
        }
        else
            m_pScriptDebugging->LogCustom( SString ( "Bad argument @ '%s' [%s]", "removeEventHandler", *argStream.GetErrorMessage () ) );

        // Failed
        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( triggerEvent )
    {
    //  bool triggerEvent ( string eventName, element baseElement, [ var argument1, ... ] )
        SString strName; CClientEntity* pEntity;

        CScriptArgReader argStream( L );
        argStream.ReadString( strName );
        argStream.ReadClass( pEntity, LUACLASS_ENTITY );

        if ( !argStream.HasErrors() )
        {
            lua_pushboolean( L, pEntity->CallEvent( strName.c_str(), L, lua_gettop( L ) - 2, false ) );
            return 1;
        }
        else
            m_pScriptDebugging->LogCustom( SString( "Bad argument @ '" __FUNCTION__ "' [%s]", *argStream.GetErrorMessage() ) );

        // Error
        lua_pushnil ( L );
        return 1;
    }

    LUA_DECLARE( triggerServerEvent )
    {
    //  bool triggerServerEvent ( string event, element theElement, [arguments...] )
        SString strName; CClientEntity* pCallWithEntity; CLuaArguments Arguments;

        CScriptArgReader argStream ( L );
        argStream.ReadString ( strName );
        argStream.ReadClass( pCallWithEntity, LUACLASS_ENTITY );
        argStream.ReadLuaArguments ( Arguments );

        if ( !argStream.HasErrors () )
        {
            // Trigger it
            if ( CStaticFunctionDefinitions::TriggerServerEvent ( strName, *pCallWithEntity, Arguments ) )
            {
                lua_pushboolean ( L, true );
                return 1;
            }
        }
        else
            m_pScriptDebugging->LogCustom( SString ( "Bad argument @ '%s' [%s]", "triggerServerEvent", *argStream.GetErrorMessage () ) );

        // Failed
        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( cancelEvent )
    {
        // Cancel it
        if ( CStaticFunctionDefinitions::CancelEvent ( true ) )
        {
            lua_pushboolean ( L, true );
            return 1;
        }

        // Failed
        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( wasEventCancelled )
    {
        // Return whether the last event was cancelled or not
        lua_pushboolean ( L, CStaticFunctionDefinitions::WasEventCancelled () );
        return 1;
    }
}