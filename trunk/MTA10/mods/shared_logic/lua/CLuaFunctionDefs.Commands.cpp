/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.0
*               (Shared logic for modifications)
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        mods/shared_logic/lua/CLuaFunctionDefs.Commands.cpp
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
*
*****************************************************************************/

#include "StdInc.h"

namespace CLuaFunctionDefs
{
    LUA_DECLARE( addCommandHandler )
    {
    //  bool addCommandHandler ( string commandName, function handlerFunction, [bool caseSensitive = true] )
        SString strKey; LuaFunctionRef iLuaFunction; bool bCaseSensitive;

        CScriptArgReader argStream ( L );
        argStream.ReadString ( strKey );
        argStream.ReadFunction ( iLuaFunction );
        argStream.ReadBool ( bCaseSensitive, true );
        argStream.ReadFunctionComplete ();

        if ( !argStream.HasErrors () )
        {
            // Grab our VM
            CLuaMain* pLuaMain = m_pLuaManager->GetVirtualMachine ( L );
            if ( pLuaMain )
            {
                // Add them to our list over command handlers
                if ( m_pRegisteredCommands->AddCommand ( pLuaMain, strKey, iLuaFunction, bCaseSensitive ) )
                {
                    lua_pushboolean ( L, true );
                    return 1;
                }
            }
        }
        else
            m_pScriptDebugging->LogCustom( SString ( "Bad argument @ '%s' [%s]", "addCommandHandler", *argStream.GetErrorMessage () ) );

        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( removeCommandHandler )
    {
    //  bool removeCommandHandler ( string commandName )
        SString strKey;

        CScriptArgReader argStream ( L );
        argStream.ReadString ( strKey );

        if ( !argStream.HasErrors () )
        {
            // Grab our VM
            CLuaMain* pLuaMain = m_pLuaManager->GetVirtualMachine ( L );
            if ( pLuaMain )
            {
                // Remove it from our list
                if ( m_pRegisteredCommands->RemoveCommand ( pLuaMain, strKey ) )
                {
                    lua_pushboolean ( L, true );
                    return 1;
                }
            }
        }
        else
            m_pScriptDebugging->LogCustom( SString ( "Bad argument @ '%s' [%s]", "removeCommandHandler", *argStream.GetErrorMessage () ) );

        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( executeCommandHandler )
    {
    //  bool executeCommandHandler ( string commandName, [ string args ] )
        SString strKey; SString strArgs;

        CScriptArgReader argStream ( L );
        argStream.ReadString ( strKey );
        argStream.ReadString ( strArgs, "" );

        if ( !argStream.HasErrors () )
        {

            // Grab our VM
            CLuaMain* pLuaMain = m_pLuaManager->GetVirtualMachine ( L );
            if ( pLuaMain )
            {
                // Call it
                if ( m_pRegisteredCommands->ProcessCommand ( strKey, strArgs ) )
                {
                    lua_pushboolean ( L, true );
                    return 1;
                }
            }
        }
        else
            m_pScriptDebugging->LogCustom( SString ( "Bad argument @ '%s' [%s]", "executeCommandHandler", *argStream.GetErrorMessage () ) );

        lua_pushboolean ( L, false );
        return 1;
    }
}