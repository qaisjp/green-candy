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

#include <StdInc.h>
#include "luafile.Utils.hxx"

static int rootIdx;

static int filesystem_open( lua_State *L )
{
    luaL_checktype( L, 1, LUA_TSTRING );
    luaL_checktype( L, 2, LUA_TSTRING );

    CFile *file = ((CFileTranslator*)lua_getmethodtrans( L ))->Open( lua_tostring( L, 1 ), lua_tostring( L, 2 ) );

    if ( !file )
    {
        lua_pushboolean( L, false );
        return 1;
    }

    lua_pushlightuserdata( L, file );
    lua_pushcclosure( L, luaconstructor_file, 1 );
#ifdef FU_CLASS
    lua_newclass( L, (unk*)file );
#else
    lua_newclass( L );

    // Register the file
    lua_getfield( L, 3, "setParent" );
    lua_pushvalue( L, lua_upvalueindex( 2 ) );
    lua_call( L, 1, 0 );
#endif
    return 1;
}

static int filesystem_exists( lua_State *L )
{
    luaL_checktype( L, 1, LUA_TSTRING );
    lua_pushboolean( L, ((CFileTranslator*)lua_getmethodtrans( L ))->Exists( lua_tostring( L, 1 ) ) );
    return 1;
}

static int filesystem_createDir( lua_State *L )
{
    luaL_checktype( L, 1, LUA_TSTRING );
    lua_pushboolean( L, ((CFileTranslator*)lua_getmethodtrans( L ))->CreateDir( lua_tostring( L, 1 ) ) );
    return 1;
}

static int filesystem_chdir( lua_State *L )
{
    luaL_checktype( L, 1, LUA_TSTRING );
    lua_pushboolean( L, ((CFileTranslator*)lua_getmethodtrans( L ))->ChangeDirectory( lua_tostring( L, 1 ) ) );
    return 1;
}

static int filesystem_delete( lua_State *L )
{
    luaL_checktype( L, 1, LUA_TSTRING );
    lua_pushboolean( L, ((CFileTranslator*)lua_getmethodtrans( L ))->Delete( lua_tostring( L, 1 ) ) );
    return 1;
}

static int filesystem_copy( lua_State *L )
{
    luaL_checktype( L, 1, LUA_TSTRING );
    luaL_checktype( L, 2, LUA_TSTRING );
    lua_pushboolean( L, ((CFileTranslator*)lua_getmethodtrans( L ))->Copy( lua_tostring( L, 1 ), lua_tostring( L, 2 ) ) );
    return 1;
}

static int filesystem_rename( lua_State *L )
{
    luaL_checktype( L, 1, LUA_TSTRING );
    luaL_checktype( L, 2, LUA_TSTRING );
    lua_pushboolean( L, ((CFileTranslator*)lua_getmethodtrans( L ))->Rename( lua_tostring( L, 1 ), lua_tostring( L, 2 ) ) );
    return 1;
}

static int filesystem_size( lua_State *L )
{
    luaL_checktype( L, 1, LUA_TSTRING );
    lua_pushnumber( L, ((CFileTranslator*)lua_getmethodtrans( L ))->Size( lua_tostring( L, 1 ) ) );
    return 1;
}

static int filesystem_stat( lua_State *L )
{
    luaL_checktype( L, 1, LUA_TSTRING );

    struct stat stats;

    if ( !((CFileTranslator*)lua_getmethodtrans( L ))->Stat( lua_tostring( L, 1 ), &stats ) )
    {
        lua_pushboolean( L, false );
        return 1;
    }

    luafile_pushStats( L, stats );
    return 1;
}

static int filesystem_relPath( lua_State *L )
{
    const char *src = lua_tostring( L, 1 );
    filePath path;

    if ( !src )
        src = "";

    if ( !((CFileTranslator*)lua_getmethodtrans( L ))->GetRelativePath( src, true, path ) )
    {
        lua_pushboolean( L, false );
        return 1;
    }

    lua_pushlstring( L, path.c_str(), path.size() );
    return 1;
}

static int filesystem_relPathRoot( lua_State *L )
{
    const char *src = lua_tostring( L, 1 );
    filePath path;

    if ( !src )
        src = "";

    if ( !((CFileTranslator*)lua_getmethodtrans( L ))->GetRelativePathFromRoot( src, true, path ) )
    {
        lua_pushboolean( L, false );
        return 1;
    }

    lua_pushlstring( L, path.c_str(), path.size() );
    return 1;
}

static int filesystem_absPath( lua_State *L )
{
    const char *src = lua_tostring( L, 1 );
    filePath path;

    if ( !src )
        src = "";

    if ( !((CFileTranslator*)lua_getmethodtrans( L ))->GetFullPath( src, true, path ) )
    {
        lua_pushboolean( L, false );
        return 1;
    }

    lua_pushlstring( L, path.c_str(), path.size() );
    return 1;
}

static int filesystem_absPathRoot( lua_State *L )
{
    const char *src = lua_tostring( L, 1 );
    filePath path;

    if ( !src )
        src = "";

    if ( !((CFileTranslator*)lua_getmethodtrans( L ))->GetFullPathFromRoot( src, true, path ) )
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
            recursive = lua_toboolean( lua, 3 );
        else
            recursive = false;
    }
    else
    {
        wildcard = "*";
        recursive = false;
    }

    lua_newtable( lua );

    ((CFileTranslator*)lua_getmethodtrans( lua ))->ScanDirectory( path, wildcard, recursive, lua_findScanCallback, lua_findScanCallback, lua );
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
            recursive = lua_toboolean( lua, 3 );
        else
            recursive = false;
    }
    else
    {
        wildcard = "*";
        recursive = false;
    }

    lua_newtable( lua );

    ((CFileTranslator*)lua_getmethodtrans( lua ))->ScanDirectory( path, wildcard, recursive, 0, lua_findScanCallback, lua );
    return 1;
}

static int filesystem_getDirs( lua_State *lua )
{
    luaL_checktype( lua, 1, LUA_TSTRING );
    
    const char *path = lua_tostring( lua, 1 );
    bool recursive;

    if ( lua_gettop( lua ) > 1 )
        recursive = lua_toboolean( lua, 2 );
    else
        recursive = false;

    lua_newtable( lua );

    ((CFileTranslator*)lua_getmethodtrans( lua ))->ScanDirectory( path, "*", recursive, lua_findScanCallback, 0, lua );
    return 1;
}

static void filesystem_exfilecb( const filePath& path, void *ud )
{
    lua_State *L = (lua_State*)ud;
    lua_pushvalue( L, 4 );
    lua_pushlstring( L, path.c_str(), path.size() );
    lua_call( L, 1, 0 );
}

static void filesystem_exdircb( const filePath& path, void *ud )
{
    lua_State *L = (lua_State*)ud;
    lua_pushvalue( L, 3 );
    lua_pushlstring( L, path.c_str(), path.size() );
    lua_call( L, 1, 0 );
}

static int filesystem_scanDirEx( lua_State *lua )
{
    luaL_checktype( lua, 1, LUA_TSTRING );
    luaL_checktype( lua, 2, LUA_TSTRING );

    ((CFileTranslator*)lua_getmethodtrans( lua ))->ScanDirectory( 
        lua_tostring( lua, 1 ), 
        lua_tostring( lua, 2 ), 
        lua_toboolean( lua, 5 ), 
        lua_type( lua, 3 ) == LUA_TFUNCTION ? filesystem_exdircb : NULL, 
        lua_type( lua, 4 ) == LUA_TFUNCTION ? filesystem_exfilecb : NULL, lua );

    return 0;
}

static int filesystem_destroy( lua_State *L )
{
    delete (CFileTranslator*)lua_touserdata( L, lua_upvalueindex( 1 ) );

    return 0;
}

static const luaL_Reg fsys_methods[] =
{
    { "destroy", filesystem_destroy },
    { NULL, NULL }
};

static const luaL_Reg fsys_methods_trans[] =
{
    { "open", filesystem_open },
    { "exists", filesystem_exists },
    { "createDir", filesystem_createDir },
    { "chdir", filesystem_chdir },
    { "delete", filesystem_delete },
    { "copy", filesystem_copy },
    { "rename", filesystem_rename },
    { "size", filesystem_size },
    { "stat", filesystem_stat },
    { "relPath", filesystem_relPath },
    { "relPathRoot", filesystem_relPathRoot },
    { "absPath", filesystem_absPath },
    { "absPathRoot", filesystem_absPathRoot },
    { "scanDir", filesystem_scanDir },
    { "scanDirEx", filesystem_scanDirEx },
    { "getDirs", filesystem_getDirs },
    { "getFiles", filesystem_getFiles },
    { NULL, NULL }
};

int luafsys_constructor( lua_State *L )
{
#ifndef FU_CLASS
    CFileTranslator *trans = (CFileTranslator*)lua_touserdata( L, lua_upvalueindex( 1 ) );

    ILuaClass *j = lua_refclass( L, 1 );
    j->SetTransmit( LUACLASS_FILETRANSLATOR, trans );

    j->RegisterInterfaceTrans( L, fsys_methods_trans, 0, LUACLASS_FILETRANSLATOR );
#endif

    lua_pushvalue( L, LUA_ENVIRONINDEX );
    lua_pushvalue( L, lua_upvalueindex( 1 ) );
    lua_getfield( L, LUA_ENVIRONINDEX, "this" );
    luaL_openlib( L, NULL, fsys_methods, 2 );

    lua_pushlstring( L, "filesystem", 10 );
    lua_setfield( L, LUA_ENVIRONINDEX, "__type" );
    return 0;
}

void luafsys_pushroot( lua_State *L, CFileTranslator *root )
{
    lua_pushlightuserdata( L, root );
    lua_pushcclosure( L, luafsys_constructor, 1 );
#ifdef FU_CLASS
    lua_newclass( L, (unk*)root );
#else
    lua_newclass( L );
#endif
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

#ifndef FU_CLASS
static int archive_save( lua_State *L )
{
    lua_getfield( L, lua_upvalueindex( 1 ), "ioptr" );

    if ( !((CFile*)lua_touserdata( L, lua_gettop( L ) ))->IsWriteable() )
    {
        lua_pushboolean( L, false );
        return 1;
    }

    ((CArchiveTranslator*)lua_touserdata( L, lua_upvalueindex( 2 ) ))->Save();
    lua_pushboolean( L, true );
    return 1;
}

static int archive_destroy( lua_State *L )
{
    // Decrement the file reference count
    ILuaClass *j = lua_refclass( L, lua_upvalueindex( 1 ) );
    j->DecrementMethodStack( L );

    return 0;
}

static const luaL_Reg archiveLib[] =
{
    { "save", archive_save },
    { "destroy", archive_destroy },
    { NULL, NULL }
};

static int archive_constructor( lua_State *L )
{
    lua_pushvalue( L, LUA_ENVIRONINDEX );
    lua_pushvalue( L, lua_upvalueindex( 1 ) );
    lua_pushvalue( L, lua_upvalueindex( 2 ) );
    luaL_openlib( L, NULL, archiveLib, 2 );
    return 0;
}

int luafsys_createArchiveTranslator( lua_State *L )
{
    luaL_checktype( L, 1, LUA_TCLASS );

    ILuaClass *j = lua_refclass( L, 1 );

    if ( !j->IsTransmit( LUACLASS_FILE ) )
        throw lua_exception( L, LUA_ERRRUN, "expected file at archive creation" );

    // Keep the file alive during archive business
    j->IncrementMethodStack( L );

    // Grab the file interface
    lua_getfield( L, 1, "ioptr" );

    CArchiveTranslator *root = fileSystem->OpenArchive( *(CFile*)lua_touserdata( L, 2 ) );

    if ( !root )
    {
        lua_pushboolean( L, false );
        return 1;
    }

    luafsys_pushroot( L, root );

    // Extend the fileTranslator class
    lua_pushvalue( L, 1 );
    lua_pushlightuserdata( L, root );
    lua_pushcclosure( L, archive_constructor, 2 );
    luaJ_extend( L, 3, 0 );
    return 1;
}

int luafsys_createZIPArchive( lua_State *L )
{
    luaL_checktype( L, 1, LUA_TCLASS );

    ILuaClass *j = lua_refclass( L, 1 );

    if ( !j->IsTransmit( LUACLASS_FILE ) )
        throw lua_exception( L, LUA_ERRRUN, "expected file at archive creation" );

    // Keep the file alive during archive business
    j->IncrementMethodStack( L );

    // Grab the file interface
    lua_getfield( L, 1, "ioptr" );

    CArchiveTranslator *root = fileSystem->CreateZIPArchive( *(CFile*)lua_touserdata( L, 2 ) );

    if ( !root )
    {
        lua_pushboolean( L, false );
        return 1;
    }

    luafsys_pushroot( L, root );

    // Extend the fileTranslator class
    lua_pushvalue( L, 1 );
    lua_pushlightuserdata( L, root );
    lua_pushcclosure( L, archive_constructor, 2 );
    luaJ_extend( L, 3, 0 );
    return 1;
}
#endif //FU_CLASS

int luafsys_getRoot( lua_State *L )
{
    lua_pushvalue( L, lua_upvalueindex( 1 ) );
    return 1;
}

static const luaL_Reg fsysLib[] =
{
    { "createTranslator", luafsys_createTranslator },
#ifndef FU_CLASS
    { "createArchiveTranslator", luafsys_createArchiveTranslator },
    { "createZIPArchive", luafsys_createZIPArchive },
#endif //FU_CLASS
    { NULL, NULL }
};

int luafsys_init( lua_State *L )
{
    // Specify the root fileTranslator
    luafsys_pushroot( L, fileSystem->CreateTranslator( "/" ) );
    return 1;
}

void luafilesystem_open( lua_State *L )
{
    lua_newtable( L );
    luaL_openlib( L, NULL, fsysLib, 0 );

    lua_pushlstring( L, "getRoot", 7 );

    lua_pushcclosure( L, luafsys_init, 0 );
    lua_call( L, 0, 1 );

    lua_pushlstring( L, "root", 4 );

    lua_pushvalue( L, -2 );
    lua_rawset( L, -5 );

    lua_pushcclosure( L, luafsys_getRoot, 1 );
    lua_rawset( L, -3 );
}