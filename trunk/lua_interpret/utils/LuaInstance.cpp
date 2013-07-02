/*****************************************************************************
*
*  PROJECT:     Lua Interpreter
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        utils/LuaInstance.cpp
*  PURPOSE:     MTA:Lua instance wrapper for less code repetition
*  DEVELOPERS:  Martin Turski <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#include <StdInc.h>

namespace lua_instance
{
    static LUA_DECLARE( isWeakChildLink )
    {
        LuaInstance *inst = (LuaInstance*)lua_touserdata( L, lua_upvalueindex( 1 ) );

        lua_pushboolean( L, !inst->keepAlive );
        return 1;
    }

    static LUA_DECLARE( destroy )
    {
        delete (LuaInstance*)lua_touserdata( L, lua_upvalueindex( 1 ) );

        return 0;
    }

    const luaL_Reg _interface[] =
    {
        LUA_METHOD( isWeakChildLink ),
        LUA_METHOD( destroy ),
        { NULL, NULL }
    };
};