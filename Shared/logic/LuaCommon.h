/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        Shared/logic/LuaCommon.h
*  PURPOSE:     Master lua header
*  DEVELOPERS:  Christian Myhre Lundheim <>
*               Jax <>
*               The_GTA <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#ifndef _BASE_LUA_COMMON_
#define _BASE_LUA_COMMON_

#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"

#define TO_ELEMENTID(x) ((ElementID) reinterpret_cast < unsigned long > (x) )

template <class type, int t, int idx>
static inline type* lua_readuserdata( lua_State *L )
{
    int top = lua_gettop( L );

    lua_rawgeti( L, t, idx );
    
    type *ud = (type*)lua_touserdata( L, top + 1 );

    lua_settop( L, top );
    return ud;
}

template <class type, int t, int idx>
static inline type* lua_readuserdata_assert( lua_State *L )
{
    type *data = lua_readuserdata <type, t, idx> ( L );

    if ( !data )
        throw lua_exception( L, LUA_ERRRUN, "attempted to browse a destroyed resource environment" );

    return data;
}

static inline std::string lua_getstring( lua_State *L, int idx )
{
    size_t len;
    const char *msg = lua_tolstring( L, idx, &len );

    return std::string( msg, len );
}

static inline void lua_unpack( lua_State *L )
{
    int idx = lua_gettop( L );

    lua_pushnil( L );

    while ( lua_next( L, idx ) )
        lua_insert( L, -2 );

    lua_remove( L, idx );
}

static inline void lua_stack2table( lua_State *L, int tidx, int n )
{
    while ( n )
        lua_rawseti( L, tidx, n-- + 1 );
}

#endif //_BASE_LUA_COMMON