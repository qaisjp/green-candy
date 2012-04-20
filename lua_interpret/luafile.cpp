/*****************************************************************************
*
*  PROJECT:     Lua Interpreter
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        luafile.cpp
*  PURPOSE:     Test environment for the filesystem
*  DEVELOPERS:  The_GTA <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#include "StdInc.h"

using namespace std;

static int luafile_onIndex( lua_State *lua )
{
    lua_pushvalue( lua, 2 );
    lua_gettable( lua, lua_upvalueindex( 1 ) );

    if ( lua_type( lua, 3 ) == LUA_TBOOLEAN && lua_toboolean( lua, 3 ) == false )
        return 0;

    lua_pop( lua, 1 );
    lua_gettable( lua, LUA_ENVIRONINDEX );
    return 1;
}

static int luafile_onNewindex( lua_State *lua )
{
    // We do not allow any modification from outside
    return 0;
}

static int luafile_read( lua_State *lua )
{
    luaL_checktype( lua, 1, LUA_TNUMBER );

    CFile *file = (CFile*)lua_touserdata( lua, lua_upvalueindex( 1 ) );
    std::vector <char> buf( (size_t)lua_tonumber( lua, 1 ) );

    buf.resize( file->Read( &buf[0], 1, buf.size() ) );

    lua_pushlstring( lua, &buf[0], buf.size() );
    return 1;
}

static int luafile_readInt( lua_State *lua )
{
    lua_pushnumber( lua, ((CFile*)lua_touserdata( lua, lua_upvalueindex( 1 ) ))->ReadInt() );
    return 1;
}

static int luafile_readFloat( lua_State *lua )
{
    lua_pushnumber( lua, ((CFile*)lua_touserdata( lua, lua_upvalueindex( 1 ) ))->ReadFloat() );
    return 1;
}

static int luafile_destroy( lua_State *lua )
{
    delete (CFile*)lua_touserdata( lua, lua_upvalueindex( 1 ) );

    return 0;
}

static const luaL_Reg fileInterface[] =
{
    { "__newindex", luafile_onNewindex },
    { "read", luafile_read },
    { "readInt", luafile_readInt },
    { "readFloat", luafile_readFloat },
    { "destroy", luafile_destroy },
    { NULL, NULL }
};

int luaconstructor_file( lua_State *lua )
{
    lua_pushvalue( lua, LUA_ENVIRONINDEX );
    lua_pushvalue( lua, lua_upvalueindex( 1 ) );
    luaL_openlib( lua, NULL, fileInterface, 1 );

    // Create the illegal access table
    lua_newtable( lua );
    lua_pushboolean( lua, false );
    lua_setfield( lua, 2, "__index" );
    lua_pushboolean( lua, false );
    lua_setfield( lua, 2, "__newindex" );

    lua_pushcclosure( lua, luafile_onIndex, 1 );
    lua_setfield( lua, LUA_ENVIRONINDEX, "__index" );

    lua_pushlstring( lua, "file", 4 );
    lua_setfield( lua, LUA_ENVIRONINDEX, "__type" );
    return 0;
}

void luafile_open( lua_State *lua )
{
}