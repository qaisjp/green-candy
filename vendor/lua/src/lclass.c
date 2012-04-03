/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        vendor/lua/src/lclass.c
*  PURPOSE:     Lua class system extension
*  DEVELOPERS:  The_GTA <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#include "lua.h"

#include "lapi.h"
#include "lgc.h"
#include "lmem.h"
#include "lstate.h"
#include "ltable.h"
#include "lclass.h"

static int methodenv_index( lua_State *L )
{
    lua_rawget( L, 1 );
    return 1;
}

static int methodenv_newindex( lua_State *L )
{
    return 0;
}

Class* luaJ_new( lua_State *L, int nargs )
{
    Class *c = luaM_new( L, Class );
    Table *meta = luaH_new( L, 0, 0 );
    luaC_link( L, obj2gco( c ), LUA_TCLASS );

    // Set up the method environment
    c->env = luaH_new( L, 0, 0 );
    c->env->metatable = meta;

    // Put the meta to stack
    sethvalue( L, L->top, meta );
    api_incr_top( L );

    // The upvalue has to be the class
    setjvalue( L, L->top, c );
    api_incr_top( L );
    lua_pushcclosure( L, methodenv_index, 1 );
    lua_setfield( L, -2, "__index" );

    // The upvalue has to be the class
    setjvalue( L, L->top, c );
    api_incr_top( L );
    lua_pushcclosure( L, methodenv_newindex, 1 );
    lua_setfield( L, -2, "__newindex" );

    lua_pushboolean( L, false );
    lua_setfield( L, -2, "__metatable" );

    sethvalue( L, L->top - 1, c->env );
    lua_setfenv( L, -nargs - 2 );

    lua_call( L, nargs, 0 );
    return c;
}

void luaJ_free( lua_State *L, Class *c )
{
    luaM_free( L, c );
}

void luaJ_construct( lua_State *L, int nargs )
{
    setjvalue( L, L->top - 1, luaJ_new( L, nargs ));
    api_incr_top( L );
}