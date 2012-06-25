/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        Shared/logic/LuaFunctionDefs.cpp
*  PURPOSE:     Shared Lua function definitions
*  DEVELOPERS:  The_GTA <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#include <StdInc.h>

static ScriptDebugging *debug;
static ResourceManager *resManager;

static inline const char* lua_getclassdesc( int ctype )
{
    switch( ctype )
    {
    case LUACLASS_FILE:
        return "file";
    case LUACLASS_FILETRANSLATOR:
        return "fileTranslator";
    }

    return "class";
}

static inline LuaMain* lua_readcontext( lua_State *L )
{
    return lua_readuserdata <LuaMain, LUA_REGISTRYINDEX, 2> ( L );
}

static inline Resource* lua_readresource( lua_State *L )
{
    return lua_readuserdata <Resource, LUA_REGISTRYINDEX, 3> ( L );
}

static inline ILuaClass& lua_classobtain( lua_State *L, int idx, int ctype )
{
    luaL_checktype( L, idx, LUA_TCLASS );

    ILuaClass *j = lua_refclass( L, idx );

    if ( !j->IsTransmit( ctype ) )
        throw lua_exception( L, LUA_ERRRUN, SString( "expected class type '%s'", lua_getclassdesc( ctype ) ) );

    return *j;
}

namespace LuaFunctionDefs
{
    LUA_DECLARE( DisabledFunction )
    {
        debug->LogError( "Unsafe function was called." );

        lua_pushboolean( L, false );
        return 1;
    }

    LUA_DECLARE( fileCreate )
    {
        luaL_checktype( L, 1, LUA_TSTRING );

        CFile *file = resManager->OpenStream( lua_readresource( L ), lua_tostring( L, 1 ), "wb" );

        if ( !file )
        {
            lua_pushboolean( L, false );
            return 1;
        }

        lua_pushlightuserdata( L, file );
        lua_pushcclosure( L, luaconstructor_file, 1 );
        lua_newclass( L );
        return 1;
    }

    LUA_DECLARE( fileExists )
    {
        luaL_checktype( L, 1, LUA_TSTRING );

        lua_pushboolean( L, resManager->FileExists( lua_readresource( L ), lua_tostring( L, 1 ) ) );
        return 1;
    }

    LUA_DECLARE( fileOpen )
    {
        luaL_checktype( L, 1, LUA_TSTRING );

        CFile *file = resManager->OpenStream( lua_readresource( L ), lua_tostring( L, 1 ), "rb+" );

        if ( !file )
        {
            lua_pushboolean( L, false );
            return 1;
        }

        lua_pushlightuserdata( L, file );
        lua_pushcclosure( L, luaconstructor_file, 1 );
        lua_newclass( L );
        return 1;
    }

    LUA_DECLARE( fileIsEOF )
    {
        lua_classobtain( L, 1, LUACLASS_FILE );

        lua_getfield( L, 1, "eof" );
        lua_call( L, 0, 1 );
        return 1;
    }

    LUA_DECLARE( fileGetPos )
    {
        lua_classobtain( L, 1, LUACLASS_FILE );

        lua_getfield( L, 1, "tell" );
        lua_call( L, 0, 1 );
        return 1;
    }

    LUA_DECLARE( fileSetPos )
    {
        lua_classobtain( L, 1, LUACLASS_FILE );
        luaL_checktype( L, 2, LUA_TNUMBER );

        lua_getfield( L, 1, "seek" );
        lua_pushvalue( L, 2 );
        lua_pushlstring( L, "set", 3 );
        lua_call( L, 2, 1 );
        return 1;
    }

    LUA_DECLARE( fileGetSize )
    {
        lua_classobtain( L, 1, LUACLASS_FILE );
        
        lua_getfield( L, 1, "size" );
        lua_call( L, 0, 1 );
        return 1;
    }

    LUA_DECLARE( fileRead )
    {
        lua_classobtain( L, 1, LUACLASS_FILE );
        luaL_checktype( L, 2, LUA_TNUMBER );

        lua_getfield( L, 1, "read" );
        lua_pushvalue( L, 2 );
        lua_call( L, 1, 1 );
        return 1;
    }

    LUA_DECLARE( fileWrite )
    {
        lua_classobtain( L, 1, LUACLASS_FILE );
        luaL_checktype( L, 2, LUA_TSTRING );

        lua_getfield( L, 1, "write" );
        lua_pushvalue( L, 2 );
        lua_call( L, 1, 1 );
        return 1;
    }

    LUA_DECLARE( fileFlush )
    {
        lua_classobtain( L, 1, LUACLASS_FILE );
        
        lua_getfield( L, 1, "flush" );
        lua_call( L, 0, 1 );
        return 1;
    }

    LUA_DECLARE( fileClose )
    {
        lua_classobtain( L, 1, LUACLASS_FILE );

        lua_getfield( L, 1, "destroy" );
        lua_call( L, 0, 0 );

        lua_pushboolean( L, true );
        return 1;
    }

    LUA_DECLARE( fileDelete )
    {
        luaL_checktype( L, 1, LUA_TSTRING );

        lua_pushboolean( L, resManager->FileDelete( lua_readresource( L ), lua_tostring( L, 1 ) ) );
        return 1;
    }

    LUA_DECLARE( fileRename )
    {
        luaL_checktype( L, 1, LUA_TSTRING );
        luaL_checktype( L, 2, LUA_TSTRING );

        lua_pushboolean( L, resManager->FileRename( lua_readresource( L ), lua_tostring( L, 1 ), lua_tostring( L, 2 ) ) );
        return 1;
    }

    LUA_DECLARE( fileCopy )
    {
        luaL_checktype( L, 1, LUA_TSTRING );
        luaL_checktype( L, 2, LUA_TSTRING );

        lua_pushboolean( L, resManager->FileCopy( lua_readresource( L ), lua_tostring( L, 1 ), lua_tostring( L, 2 ) ) );
        return 1;
    }
};