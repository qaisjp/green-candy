/*****************************************************************************
*
*  PROJECT:     Lua Interpreter
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        luaclass.cpp
*  PURPOSE:     Lua filesystem access
*  DEVELOPERS:  The_GTA <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#include "StdInc.h"

struct _class_internal
{
    unk *inter;
    bool destroyed;
};

static int lua_iunkcollect( lua_State *L )
{
    _class_internal *inter = (_class_internal*)lua_touserdata( L, 1 );

    if ( !inter->destroyed )
        delete (unk*)inter->inter;

    return 0;
}

static int lua_destrPre( lua_State *L )
{
    _class_internal *inter = (_class_internal*)lua_touserdata( L, lua_upvalueindex( 2 ) );
    
    if ( inter->destroyed )
        return 0;

    lua_pushvalue( L, lua_upvalueindex( 1 ) );
    lua_call( L, 0, 0 );

    inter->destroyed = true;
    return 0;
}

static int lua_grk( lua_State *L )
{
    if ( strcmp( lua_tostring( L, 2 ), "destroy" ) == 0 )
    {
        lua_rawget( L, lua_upvalueindex( 1 ) );
        lua_pushvalue( L, lua_upvalueindex( 2 ) );
        lua_pushcclosure( L, lua_destrPre, 2 );
        return 1;
    }

    lua_rawget( L, lua_upvalueindex( 1 ) );
    return 1;
}

void lua_newclass( lua_State *L, unk *inter )
{
    lua_newtable( L );
    lua_pushvalue( L, -1 );
    lua_newtable( L );

    lua_pushvalue( L, LUA_ENVIRONINDEX );
    lua_setfield( L, -2, "__index" );

    lua_pushboolean( L, false );
    lua_setfield( L, -2, "__metatable" );

    lua_setmetatable( L, -2 );

    lua_setfenv( L, -3 );

    lua_insert( L, -2 );

    lua_call( L, 0, 0 );

    _class_internal *it = (_class_internal*)lua_newuserdata( L, sizeof(_class_internal) );
    it->inter = inter;
    it->destroyed = false;
    lua_insert( L, -2 );

    lua_newtable( L );
    lua_insert( L, -2 );
    lua_pushvalue( L, -1 );
    lua_pushvalue( L, -4 );

    lua_pushcclosure( L, lua_grk, 2 );
    lua_setfield( L, -3, "__index" );

    lua_pushcclosure( L, lua_iunkcollect, 1 );
    lua_setfield( L, -2, "__gc" );

    lua_pushboolean( L, false );
    lua_setfield( L, -2, "__metatable" );

    lua_setmetatable( L, -2 );
}

void* lua_getmethodtrans( lua_State *L )
{
    return lua_touserdata( L, lua_upvalueindex( 1 ) );
}