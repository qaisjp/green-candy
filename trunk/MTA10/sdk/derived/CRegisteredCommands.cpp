/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        derived/CRegisteredCommands.cpp
*  PURPOSE:     Extended Console command registry
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

CCommand::CCommand( lua_State *L, CRegisteredCommands& cmds ) : Command( L, cmds, _trefget( L, *this ) )
{
}

CCommand::~CCommand()
{
}

bool CCommand::Execute( const std::vector <std::string>& args )
{
    lua_pushlstring( **lua, key.c_str(), key.size() );
    return Command::Execute( args );
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

    Command& cmd = *new CCommand( *lua, *this );
    cmd.lua = &lua;
    cmd.key = key;
    cmd.ref = ref;
    cmd.caseSensitive = caseSensitive;

    m_commands.push_back( &cmd );
    return true;
}