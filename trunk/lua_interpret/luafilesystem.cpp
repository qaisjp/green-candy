/*****************************************************************************
*
*  PROJECT:     Lua Interpreter
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        luafilesystem.cpp
*  PURPOSE:     Lua filesystem access
*  DEVELOPERS:  The_GTA <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#include "StdInc.h"

static int rootIdx;

static int filesystem_open( lua_State *lua )
{
    luaL_checktype( lua, 1, LUA_TSTRING );
    luaL_checktype( lua, 2, LUA_TSTRING );

    CFile *file = ((CFileTranslator*)lua_touserdata( lua, lua_upvalueindex( 1 ) ))->Open( lua_tostring( lua, 1 ), lua_tostring( lua, 2 ) );

    if ( !file )
    {
        std::cout << "invalid file access\n";

        lua_pushboolean( lua, false );
        return 1;
    }

    lua_pushlightuserdata( lua, file );
    lua_pushcclosure( lua, luaconstructor_file, 1 );
    lua_newclass( lua );
    return 1;
}

static int filesystem_exists( lua_State *L )
{
    luaL_checktype( L, 1, LUA_TSTRING );
    lua_pushboolean( L, ((CFileTranslator*)lua_touserdata( L, lua_upvalueindex( 1 ) ))->Exists( lua_tostring( L, 1 ) ) );
    return 1;
}

static int filesystem_createDir( lua_State *L )
{
    luaL_checktype( L, 1, LUA_TSTRING );
    lua_pushboolean( L, ((CFileTranslator*)lua_touserdata( L, lua_upvalueindex( 1 ) ))->CreateDir( lua_tostring( L, 1 ) ) );
    return 1;
}

static int filesystem_chdir( lua_State *L )
{
    luaL_checktype( L, 1, LUA_TSTRING );
    lua_pushboolean( L, ((CFileTranslator*)lua_touserdata( L, lua_upvalueindex( 1 ) ))->ChangeDirectory( lua_tostring( L, 1 ) ) );
    return 1;
}

static int filesystem_delete( lua_State *L )
{
    luaL_checktype( L, 1, LUA_TSTRING );
    lua_pushboolean( L, ((CFileTranslator*)lua_touserdata( L, lua_upvalueindex( 1 ) ))->Delete( lua_tostring( L, 1 ) ) );
    return 1;
}

static int filesystem_copy( lua_State *L )
{
    luaL_checktype( L, 1, LUA_TSTRING );
    luaL_checktype( L, 2, LUA_TSTRING );
    lua_pushboolean( L, ((CFileTranslator*)lua_touserdata( L, lua_upvalueindex( 1 ) ))->Copy( lua_tostring( L, 1 ), lua_tostring( L, 2 ) ) );
    return 1;
}

static int filesystem_rename( lua_State *L )
{
    luaL_checktype( L, 1, LUA_TSTRING );
    luaL_checktype( L, 2, LUA_TSTRING );
    lua_pushboolean( L, ((CFileTranslator*)lua_touserdata( L, lua_upvalueindex( 1 ) ))->Rename( lua_tostring( L, 1 ), lua_tostring( L, 2 ) ) );
    return 1;
}

static int filesystem_size( lua_State *L )
{
    luaL_checktype( L, 1, LUA_TSTRING );
    lua_pushnumber( L, ((CFileTranslator*)lua_touserdata( L, lua_upvalueindex( 1 ) ))->Size( lua_tostring( L, 1 ) ) );
    return 1;
}

static int filesystem_relPath( lua_State *L )
{
    luaL_checktype( L, 1, LUA_TSTRING );
    
    filePath path;

    if ( !((CFileTranslator*)lua_touserdata( L, lua_upvalueindex( 1 ) ))->GetRelativePath( lua_tostring( L, 1 ), true, path ) )
    {
        lua_pushboolean( L, false );
        return 1;
    }

    lua_pushlstring( L, path.c_str(), path.size() );
    return 1;
}

static int filesystem_absPath( lua_State *L )
{
    luaL_checktype( L, 1, LUA_TSTRING );
    
    filePath path;

    if ( !((CFileTranslator*)lua_touserdata( L, lua_upvalueindex( 1 ) ))->GetFullPath( lua_tostring( L, 1 ), true, path ) )
    {
        lua_pushboolean( L, false );
        return 1;
    }

    lua_pushlstring( L, path.c_str(), path.size() );
    return 1;
}

static void lua_findScanCallback( const filePath& path, void *ud )
{
    lua_pushlstring( (lua_State*)ud, path, path.size() );
    luaL_ref( (lua_State*)ud, -2 );
}

static int filesystem_scanDir( lua_State *lua )
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

    ((CFileTranslator*)lua_touserdata( lua, lua_upvalueindex( 1 ) ))->ScanDirectory( path, wildcard, recursive, lua_findScanCallback, lua_findScanCallback, lua );
    return 1;
}

static int filesystem_getFiles( lua_State *lua )
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

    ((CFileTranslator*)lua_touserdata( lua, lua_upvalueindex( 1 ) ))->ScanDirectory( path, wildcard, recursive, 0, lua_findScanCallback, lua );
    return 1;
}

static int filesystem_getDirs( lua_State *lua )
{
    luaL_checktype( lua, 1, LUA_TSTRING );
    
    const char *path = lua_tostring( lua, 1 );
    bool recursive;

    if ( lua_gettop( lua ) > 1 )
        recursive = lua_toboolean( lua, 2 ) == 1;
    else
        recursive = false;

    lua_newtable( lua );

    ((CFileTranslator*)lua_touserdata( lua, lua_upvalueindex( 1 ) ))->ScanDirectory( path, "*", recursive, lua_findScanCallback, 0, lua );
    return 1;
}

static int filesystem_destroy( lua_State *L )
{
    delete ((CFileTranslator*)lua_touserdata( L, lua_upvalueindex( 1 ) ));

    return 0;
}

static const luaL_Reg fsys_methods[] =
{
    { "open", filesystem_open },
    { "exists", filesystem_exists },
    { "createDir", filesystem_createDir },
    { "chdir", filesystem_chdir },
    { "delete", filesystem_delete },
    { "copy", filesystem_copy },
    { "rename", filesystem_rename },
    { "size", filesystem_size },
    { "relPath", filesystem_relPath },
    { "absPath", filesystem_absPath },
    { "getDirs", filesystem_getDirs },
    { "getFiles", filesystem_getFiles },
    { "scanDir", filesystem_scanDir },
    { "destroy", filesystem_destroy },
    { NULL, NULL }
};

int luafsys_constructor( lua_State *L )
{
    lua_pushvalue( L, LUA_ENVIRONINDEX );
    lua_pushvalue( L, lua_upvalueindex( 1 ) );
    luaL_openlib( L, NULL, fsys_methods, 1 );

    lua_pushlstring( L, "filesystem", 10 );
    lua_setfield( L, LUA_ENVIRONINDEX, "__type" );
    return 0;
}

void luafsys_pushroot( lua_State *L, CFileTranslator *root )
{
    lua_pushlightuserdata( L, root );
    lua_pushcclosure( L, luafsys_constructor, 1 );
    lua_newclass( L );
}

int luafsys_createTranslator( lua_State *L )
{
    luaL_checktype( L, 1, LUA_TSTRING );

    CFileTranslator *root = fileSystem->CreateTranslator( lua_tostring( L, 1 ) );

    if ( !root )
    {
        lua_pushboolean( L, false );
        return 1;
    }

    luafsys_pushroot( L, root );
    return 1;
}

int luafsys_getRoot( lua_State *L )
{
    lua_rawgeti( L, LUA_REGISTRYINDEX, rootIdx );
    return 1;
}

static const luaL_Reg fsysLib[] =
{
    { "createTranslator", luafsys_createTranslator },
    { "getRoot", luafsys_getRoot },
    { NULL, NULL }
};

int luafsys_init( lua_State *L )
{
    // Specify the root fileTranslator
    luafsys_pushroot( L, fileRoot );
    rootIdx = luaL_ref( L, LUA_REGISTRYINDEX );
    return 0;
}

void luafilesystem_open( lua_State *L )
{
    luaL_openlib( L, "file", fsysLib, 0 );

    lua_pushcclosure( L, luafsys_init, 0 );
    lua_call( L, 0, 0 );
}