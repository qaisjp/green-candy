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
    lua_rawget( lua, LUA_ENVIRONINDEX );
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

static int luaconstructor_file( lua_State *lua )
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

static int lua_fileOpen( lua_State *lua )
{
    luaL_checktype( lua, 1, LUA_TSTRING );
    luaL_checktype( lua, 2, LUA_TSTRING );

    CFile *file = fileRoot->Open( lua_tostring( lua, 1 ), lua_tostring( lua, 2 ) );

    if ( !file )
    {
        cout << "invalid file access\n";

        lua_pushboolean( lua, false );
        return 1;
    }

    lua_pushlightuserdata( lua, file );
    lua_pushcclosure( lua, luaconstructor_file, 1 );
    lua_newclass( lua );
    return 1;
}

static void lua_findScanCallback( const filePath& path, void *ud )
{
    lua_pushlstring( (lua_State*)ud, path, path.size() );
    luaL_ref( (lua_State*)ud, -2 );
}

static int lua_fileScanDir( lua_State *lua )
{
    luaL_checktype( lua, 1, LUA_TSTRING );
    
    const char *path = lua_tostring( lua, 1 );
    const char *wildcard;
    bool recursive;

    int top = lua_gettop( lua );

    if ( top > 1 )
    {
        wildcard = lua_tostring( lua, 2 );

        if ( top > 2 )
            recursive = lua_toboolean( lua, 3 ) == 1;
        else
            recursive = false;
    }
    else
    {
        wildcard = "*";
        recursive = false;
    }

    lua_newtable( lua );

    fileRoot->ScanDirectory( path, wildcard, recursive, lua_findScanCallback, lua_findScanCallback, lua );
    return 1;
}

static int lua_getFiles( lua_State *lua )
{
    luaL_checktype( lua, 1, LUA_TSTRING );
    
    const char *path = lua_tostring( lua, 1 );
    const char *wildcard;
    bool recursive;

    int top = lua_gettop( lua );

    if ( top > 1 )
    {
        wildcard = lua_tostring( lua, 2 );

        if ( top > 2 )
            recursive = lua_toboolean( lua, 3 ) == 1;
        else
            recursive = false;
    }
    else
    {
        wildcard = "*";
        recursive = false;
    }

    lua_newtable( lua );

    fileRoot->ScanDirectory( path, wildcard, recursive, 0, lua_findScanCallback, lua );
    return 1;
}

static int lua_getDirs( lua_State *lua )
{
    luaL_checktype( lua, 1, LUA_TSTRING );
    
    const char *path = lua_tostring( lua, 1 );
    bool recursive;

    if ( lua_gettop( lua ) > 1 )
        recursive = lua_toboolean( lua, 2 ) == 1;
    else
        recursive = false;

    lua_newtable( lua );

    fileRoot->ScanDirectory( path, "*", recursive, lua_findScanCallback, 0, lua );
    return 1;
}

static const luaL_Reg fileLib[] =
{
    { "fileOpen", lua_fileOpen },
    { "fileScanDir", lua_fileScanDir },
    { "getFiles", lua_getFiles },
    { "getDirs", lua_getDirs },
    { NULL, NULL }
};

void luafile_open( lua_State *lua )
{
    lua_pushvalue( lua, LUA_GLOBALSINDEX );
    luaL_openlib( lua, NULL, fileLib, 0 );
}