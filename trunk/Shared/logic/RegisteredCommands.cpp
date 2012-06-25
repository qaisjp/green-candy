/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        Shared/logic/RegisteredCommands.cpp
*  PURPOSE:     Console command registry
*  DEVELOPERS:  The_GTA <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#include <StdInc.h>

static LUA_DECLARE( luacmd_destroy )
{
    ((RegisteredCommands*)lua_touserdata( L, lua_upvalueindex( 1 ) ) )->m_commands.remove( (Command*)lua_touserdata( L, lua_upvalueindex( 2 ) ) );
    return 0;
}

static const luaL_Reg cmdInterface[] =
{
    { "destroy", luacmd_destroy },
    { NULL, NULL }
};

int RegisteredCommands::luaconstructor_command( lua_State *L )
{
    lua_pushvalue( L, lua_upvalueindex( 1 ) );
    lua_pushvalue( L, lua_upvalueindex( 2 ) );
    luaL_openlib( L, NULL, cmdInterface, 2 );
    return 0;
}

RegisteredCommands::RegisteredCommands( LuaManager& manager ) : m_system( manager )
{
    lua_newtable( manager.m_lua );
    m_refTable = luaL_ref( manager.m_lua, LUA_REGISTRYINDEX );
}

RegisteredCommands::~RegisteredCommands()
{
    luaL_unref( m_system.m_lua, LUA_REGISTRYINDEX, m_refTable );
}

bool RegisteredCommands::Remove( LuaMain *lua, const std::string& key )
{
    Command *cmd = Get( key.c_str(), lua );

    if ( !cmd )
        return false;

    // Delete it and remove it from our list
    cmd->Destroy();
    return true;
}

void RegisteredCommands::Clear()
{
    commandList_t::iterator iter = m_commands.begin();
    luaRefs refs;

    for ( ; iter != m_commands.end(); iter++ )
    {
        (*iter)->Reference( refs );
        (*iter)->Destroy();
    }
}

void RegisteredCommands::CleanUp( LuaMain *lua )
{
    commandList_t::iterator iter = m_commands.begin();
    luaRefs refs;

    for ( ; iter != m_commands.end(); iter++ )
    {
        Command& cmd = **iter;

        if ( cmd.lua != lua )
            continue;

        cmd.Reference( refs );
        cmd.Destroy();
    }
}

bool RegisteredCommands::Exists( const char *key, LuaMain *lua )
{
    return Get( key, lua ) != NULL;
}

Command* RegisteredCommands::Get( const char *key, LuaMain *lua )
{
    commandList_t::iterator iter = m_commands.begin ();
    bool exact;

    for ( ; iter != m_commands.end(); iter++ )
    {
        Command& cmd = **iter;

        if ( cmd.caseSensitive )
            exact = ( cmd.key == key );
        else
            exact = stricmp( cmd.key.c_str(), key ) == 0;

        // Matching vm's and names?
        if ( lua && cmd.lua != lua || !exact )
            continue;

        return &cmd;
    }

    return NULL;
}