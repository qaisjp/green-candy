/*****************************************************************************
*
*  PROJECT:     KillFrenzy! 1.0
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        lua/luaroot.cpp
*  PURPOSE:     Game Lua root
*  DEVELOPERS:  The_GTA <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#include <StdInc.h>

static int root_destroy( lua_State *L )
{
    delete (LuaRoot*)lua_touserdata( L, lua_upvalueindex( 1 ) );

    return 0;
}

static const luaL_Reg root_interface[] =
{
    { "destroy", root_destroy },
    { NULL, NULL }
};

static int luaconstructor_root( lua_State *L )
{
    LuaRoot *root = (LuaRoot*)lua_touserdata( L, lua_upvalueindex( 1 ) );

    ILuaClass& j = *lua_refclass( L, 1 );
    j.SetTransmit( LUACLASS_ROOT, root );

    lua_pushvalue( L, LUA_ENVIRONINDEX );
    lua_pushvalue( L, lua_upvalueindex( 1 ) );
    luaL_openlib( L, NULL, root_interface, 1 );

    lua_basicprotect( L );

    lua_pushlstring( L, "root", 4 );
    lua_setfield( L, LUA_ENVIRONINDEX, "__type" );
    return 0;
}

static inline int _trefget( lua_State *L, LuaRoot *root )
{
    lua_pushlightuserdata( L, root );
    lua_pushcclosure( L, luaconstructor_root, 1 );
    lua_newclass( L );
    return luaL_ref( L, LUA_REGISTRYINDEX );
}

LuaRoot::LuaRoot( lua_State *L ) : LuaClass( L, _trefget( L, this ) )
{
}

LuaRoot::~LuaRoot()
{
}