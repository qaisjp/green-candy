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

    Command& cmd = *new Command( *lua, *this );
    cmd.lua = &lua;
    cmd.key = key;
    cmd.ref = ref;
    cmd.caseSensitive = caseSensitive;

    m_commands.push_back( &cmd );
    return true;
}