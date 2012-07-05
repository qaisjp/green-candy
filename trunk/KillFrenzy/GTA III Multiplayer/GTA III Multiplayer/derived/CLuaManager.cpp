/*****************************************************************************
*
*  PROJECT:     KillFrenzy! 1.0
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        derived/CLuaManager.cpp
*  PURPOSE:     Lua hyperstructure management extension
*  DEVELOPERS:  The_GTA <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#include <StdInc.h>

CLuaManager::CLuaManager( CEvents& events ) : m_debug( *this ), LuaManager( events, m_debug ),
    m_commands( *this )
{
}

CLuaManager::~CLuaManager()
{
}

static void LoadCFunctions( lua_State *L )
{
    using namespace CLuaFunctionDefs;

    LUA_REGISTER( L, addCommandHandler );
    LUA_REGISTER( L, removeCommandHandler );

    LUA_REGISTER( L, playSoundFrontEnd );

    LUA_REGISTER( L, callSCM );

    LUA_REGISTER( L, createVehicle );
}

CLuaMain* CLuaManager::Create( const std::string& name, CFileTranslator& fileRoot )
{
    CLuaMain& main = *new CLuaMain( *this, fileRoot );
    main.m_name = name;

    Init( &main );
    LoadCFunctions( *main );

    // Add special globals
    g_player->PushStack( *main );
    lua_setfield( *main, LUA_GLOBALSINDEX, "localPlayer" );

    m_structures.push_back( &main );
    return &main;
}

bool CLuaManager::Remove( LuaMain *lua )
{
    if ( !LuaManager::Remove( lua ) )
        return false;

    m_commands.CleanUp( lua );
    return true;
}