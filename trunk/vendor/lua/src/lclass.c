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
    
    return 1;
}

static int methodenv_newindex( lua_State *L )
{
    return 0;
}

Class* luaJ_new( lua_State *L, Class *super )
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

    lua_pushvalue( L, -2 );
    lua_pushcclosure( L, methodenv_index, 1 );
    lua_setfield( L, -2, "__index" );

    lua_pushvalue( L, -2 );
    lua_pushcclosure( L, methodenv_newindex, 1 );
    lua_setfield( L, -2, "__newindex" );

    lua_pushboolean( L, false );
    lua_setfield( L, -2, "__metatable" );

    lua_pop( L, 1 );

    c->super = super;
    return c;
}

void luaJ_free( lua_State *L, Class *c )
{
    luaM_free( L, c );
}

static int classcm_index( lua_State *L )
{
    return 1;
}

static int classcm_newindex( lua_State *L )
{
    return 0;
}

void luaJ_construct( lua_State *L, int nargs )
{
    int top = lua_gettop( L );

    lua_newtable( L );
    lua_newtable( L );

    lua_pushvalue( L, top );
    lua_pushcclosure( L, classcm_index, 1 );
    lua_setfield( L, top + 1, "__index" );

    lua_pushvalue( L, top );
    lua_pushcclosure( L, classcm_newindex, 1 );
    lua_setfield( L, top + 1, "__newindex" );

    lua_pushboolean( L, false );
    lua_setfield( L, top + 1, "__metatable" );

    lua_setmetatable( L, top );
    lua_pop( L, 1 );

    lua_setfenv( L, -nargs - 1 );

    lua_call( L, top - 1, 0 );
}