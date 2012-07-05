/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        Shared/logic/RegisteredCommands.h
*  PURPOSE:     Console command registry
*  DEVELOPERS:  The_GTA <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#ifndef _BASE_REGISTERED_COMMANDS_
#define _BASE_REGISTERED_COMMANDS_

class LuaMain;
class LuaManager;
class RegisteredCommands;

class Command : public LuaClass
{
public:
    Command( lua_State *L, RegisteredCommands& cmds, int ridx ) : manager( cmds ), LuaClass( L, ridx )
    {
    }

    virtual bool Execute( const std::vector <std::string>& args );

    RegisteredCommands& manager;
    LuaMain*        lua;
    std::string     key;
    LuaFunctionRef  ref;
    bool            caseSensitive;
};

class RegisteredCommands
{
    friend class Command;
public:
                                        RegisteredCommands( LuaManager& manager );
                                        ~RegisteredCommands();

    bool                                Remove( LuaMain *lua, const std::string& key );
    Command*                            Get( const char *key, LuaMain *lua = NULL );
    void                                Clear();
    void                                CleanUp( LuaMain *lua );

    bool                                Exists( const char *key, LuaMain *lua = NULL );

    typedef std::list <Command*> commandList_t;
    std::list <Command*>    m_commands;

    static int luaconstructor_command( lua_State *L );

protected:    
    LuaManager&     m_system;
};

#endif //_BASE_REGISTERED_COMMANDS_