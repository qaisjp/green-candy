/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        Shared/logic/lua/LuaManager.cpp
*  PURPOSE:     Management of Lua hyperstructures
*       Every lua hyperstructure is a table in a global lua environment
*       upkept by the LuaManager entity. LuaManager is supposed to secure
*       the connections between hyperstructures and provide the basic
*       framework for their runtime.
*  DEVELOPERS:  Ed Lyons <>
*               Chris McArthur <>
*               Jax <>
*               Cecill Etheredge <>
*               Christian Myhre Lundheim <>
*               Oliver Brown <>
*               Kevin Whiteside <>
*               lil_Toady <>
*               The_GTA <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#include <StdInc.h>

LuaManager::LuaManager( RegisteredCommands& commands, Events& events )
{
    m_commands = commands;
    m_events = events;

    // Load our C Functions into LUA and hook callback
    LuaCFunctions::InitializeHashMaps();
    LoadCFunctions();

    lua_registerPreCallHook( CLuaDefs::CanUseFunction );
}

LuaManager::~LuaManager()
{
    LuaCFunctions::RemoveAllFunctions();

    std::list <LuaMain*>::iterator iter;

    for ( iter = m_structures.begin(); iter != m_structures.end(); iter++ )
        delete *iter;
}

bool LuaManager::Remove( LuaMain *lua )
{
    if ( !lua )
        return false;

    // Remove all events registered by it and all commands added
    m_events.RemoveAllEvents( lua );
    m_commands.CleanUpForVM( lua );

    // Delete it unless it is already
    if ( !vm->BeingDeleted () )
        delete vm;

    // Remove it from our list
    if ( !m_virtualMachines.empty() )
        m_virtualMachines.remove ( vm );

    return true;
}


void LuaManager::DoPulse()
{
    std::list <CLuaMain *>::iterator iter;
    for ( iter = m_virtualMachines.begin(); iter != m_virtualMachines.end(); iter++ )

        (*iter)->DoPulse();
    }
    m_pLuaModuleManager->_DoPulse ();
}

CLuaMain* LuaManager::GetVirtualMachine( lua_State *lua )
{
    // Grab the main virtual state because the one we've got passed might be a coroutine state
    // and only the main state is in our list.
    lua_State* main = lua_getmainstate ( luaVM );
    if ( main )
    {
        luaVM = main;
    }

    // Find a matching VM in our list
    list < CLuaMain* >::const_iterator iter = m_virtualMachines.begin ();
    for ( ; iter != m_virtualMachines.end (); iter++ )
    {
        if ( luaVM == (*iter)->GetVirtualMachine () )
        {
            return *iter;
        }
    }

    // Doesn't exist
    return NULL;
}

void LuaManager::LoadCFunctions()
{
    // JSON funcs
    lua_register( m_lua, "toJSON", LuaFunctionDefinitions::toJSON );
    lua_register( m_lua, "fromJSON", LuaFunctionDefinitions::fromJSON );

    // Utility
    lua_register( m_lua, "getDistanceBetweenPoints2D", LuaFunctionDefinitions::GetDistanceBetweenPoints2D );
    lua_register( m_lua, "getDistanceBetweenPoints3D", LuaFunctionDefinitions::GetDistanceBetweenPoints3D );
    lua_register( m_lua, "getEasingValue", LuaFunctionDefinitions::GetEasingValue );
    lua_register( m_lua, "interpolateBetween", LuaFunctionDefinitions::InterpolateBetween );

    lua_register( m_lua, "getTickCount", LuaFunctionDefinitions::GetTickCount_ );
    lua_register( m_lua, "getRealTime", LuaFunctionDefinitions::GetCTime );
    lua_register( m_lua, "split", LuaFunctionDefinitions::Split );
    lua_register( m_lua, "gettok", LuaFunctionDefinitions::GetTok );
    lua_register( m_lua, "setTimer", LuaFunctionDefinitions::SetTimer );
    lua_register( m_lua, "killTimer", LuaFunctionDefinitions::KillTimer );
    lua_register( m_lua, "resetTimer", LuaFunctionDefinitions::ResetTimer );
    lua_register( m_lua, "getTimers", LuaFunctionDefinitions::GetTimers );
    lua_register( m_lua, "isTimer", LuaFunctionDefinitions::IsTimer );
    lua_register( m_lua, "getTimerDetails", LuaFunctionDefinitions::GetTimerDetails );
    lua_register( m_lua, "getColorFromString", LuaFunctionDefinitions::GetColorFromString );

    // UTF functions
    lua_register( m_lua, "utfLen", LuaFunctionDefinitions::UtfLen );
    lua_register( m_lua, "utfSeek", LuaFunctionDefinitions::UtfSeek );
    lua_register( m_lua, "utfSub", LuaFunctionDefinitions::UtfSub );
    lua_register( m_lua, "utfChar", LuaFunctionDefinitions::UtfChar );
    lua_register( m_lua, "utfCode", LuaFunctionDefinitions::UtfCode );

    lua_register( m_lua, "md5", LuaFunctionDefinitions::Md5 );
    lua_register( m_lua, "getVersion", LuaFunctionDefinitions::GetVersion );
}
