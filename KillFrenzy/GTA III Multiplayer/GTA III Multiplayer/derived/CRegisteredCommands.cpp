/*****************************************************************************
*
*  PROJECT:     KillFrenzy! 1.0
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        derived/CRegisteredCommands.cpp
*  PURPOSE:     Lua commands registry extension
*  DEVELOPERS:  The_GTA <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#include <StdInc.h>

static inline int _trefget( lua_State *L, CCommand& cmd )
{
    lua_pushlightuserdata( L, &cmd );
    lua_pushcclosure( L, RegisteredCommands::luaconstructor_command, 1 );
    lua_newclass( L );
    return luaL_ref( L, LUA_REGISTRYINDEX );
}

CCommand::CCommand( lua_State *L, CRegisteredCommands& cmds ) : Command( cmds, L )
{
}

CCommand::~CCommand()
{
}

CRegisteredCommands::CRegisteredCommands( CLuaManager& manager ) : RegisteredCommands( manager )
{
}

CRegisteredCommands::~CRegisteredCommands()
{
}

bool CRegisteredCommands::Add( LuaMain& lua, const std::string& key, const LuaFunctionRef& ref, bool caseSensitive )
{
    if ( key.size() == 0 )
        return false;

    CCommand& cmd = *new CCommand( *lua, *this );
    cmd.lua = &lua;
    cmd.key = key;
    cmd.ref = ref;
    cmd.caseSensitive = caseSensitive;

    m_commands.push_back( &cmd );
    return true;
}