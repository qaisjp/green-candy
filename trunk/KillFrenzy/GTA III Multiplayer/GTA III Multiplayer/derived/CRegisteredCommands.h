/*****************************************************************************
*
*  PROJECT:     KillFrenzy! 1.0
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        derived/CRegisteredCommands.h
*  PURPOSE:     Lua commands registry extension
*  DEVELOPERS:  The_GTA <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#ifndef _REGISTERED_COMMANDS_
#define _REGISTERED_COMMANDS_

class CCommand : public Command
{
public:
    CCommand( lua_State *L, class CRegisteredCommands& cmds );
    ~CCommand();
};

class CRegisteredCommands : public RegisteredCommands
{
public:
                                CRegisteredCommands( class CLuaManager& man );
                                ~CRegisteredCommands();

    bool                        Add( LuaMain& lua, const std::string& key, const LuaFunctionRef& ref, bool caseSensitive );
};

#endif //_REGISTERED_COMMANDS_