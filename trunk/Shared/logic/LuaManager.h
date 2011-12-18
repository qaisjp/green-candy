/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        Shared/logic/lua/LuaManager.h
*  PURPOSE:     Management of Lua hyperstructures
*       Every lua hyperstructure is a table in a global lua environment
*       upkept by the LuaManager entity. LuaManager is supposed to secure
*       the connections between hyperstructures and provide the basic
*       framework for their runtime.
*  DEVELOPERS:  Ed Lyons <>
*               Cecill Etheredge <>
*               Christian Myhre Lundheim <>
*               Oliver Brown <>
*               The_GTA <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#ifndef _BASE_LUA_MANAGER_
#define _BASE_LUA_MANAGER_

#include "LuaCFunctions.h"

class LuaManager
{
    friend class LuaMain;
public:
                                    LuaManager( RegisteredCommands& commands, Events& events );
                                    ~LuaManager();

    // Provide your own creation function
    bool                            Remove( LuaMain *lua );
    LuaMain*                        Get( lua_State *lua );

    inline std::list <LuaMain*>::const_iterator IterBegin()     { return m_structures.begin (); };
    inline std::list <LuaMain*>::const_iterator IterEnd()       { return m_structures.end (); };

    void                            DoPulse();

protected:
    void                            LoadCFunctions();

    lua_State*                      m_lua;

    class RegisteredCommands&       m_commands;
    class Events&                   m_events;
    list <LuaMain*>                 m_structures;
};

#endif //_BASE_LUA_MANAGER_