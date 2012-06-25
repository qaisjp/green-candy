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

class Command : public LuaClass
{
public:
    Command( lua_State *L, int ridx ) : LuaClass( L, ridx )
    {
    }

    LuaMain*        lua;
    std::string     key;
    LuaFunctionRef  ref;
    bool            caseSensitive;
};

class RegisteredCommands
{
public:
                                        RegisteredCommands( LuaManager& manager );
                                        ~RegisteredCommands();

    bool                                Add( LuaMain *lua, const std::string& key, const LuaFunctionRef& ref, bool caseSensitive );
    bool                                Remove( LuaMain *lua, const std::string& key );
    void                                Clear();
    void                                CleanUp( LuaMain *lua );

    bool                                Exists( const char *key, LuaMain *lua = NULL );

    typedef std::list <Command*> commandList_t;
    std::list <Command*>    m_commands;

protected:
    Command*                            Get( const char *key, LuaMain *lua = NULL );
    
    unsigned int    m_refTable;

    int luaconstructor_command( lua_State *L );

    LuaManager&     m_system;
};

#endif //_BASE_REGISTERED_COMMANDS_