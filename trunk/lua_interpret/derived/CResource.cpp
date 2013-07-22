/*****************************************************************************
*
*  PROJECT:     Lua Interpreter
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        derived/CResource.cpp
*  PURPOSE:     Lua resource class
*  DEVELOPERS:  Martin Turski <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#include <StdInc.h>

static LUA_DECLARE( loadScript )
{
    lua_String path;

    LUA_ARGS_BEGIN;
    argStream.ReadString( path );
    LUA_ARGS_END;

    CResource *res = (CResource*)lua_getmethodtrans( L );
    std::vector <char> buf;

    CFileTranslator *root = res->GetFileRoot();
    LUA_CHECK( root->ReadToBuffer( path, buf ) );

    // Always zero terminate for security!
    buf.push_back( 0 );

    // Try to compile the code.
    if ( luaL_loadstring( L, &buf[0] ) != 0 )
    {
        lua_pushboolean( L, false );
        lua_pushvalue( L, -2 );
        return 2;
    }

    // Move the global environment to our state
    lua_State *remoteState = *res->GetVM();
    lua_pushvalue( remoteState, LUA_GLOBALSINDEX );
    lua_xmove( remoteState, L, 1 );

    // Set the created function closure environment to the global environment
    // of the resource
    lua_setfenv( L, -2 );

    // Run the closure.
    if ( lua_pcall( L, 0, 0, 0 ) != 0 )
    {
        lua_pushboolean( L, false );
        lua_pushvalue( L, -2 );
        return 2;
    }

    // Success!
    lua_pushboolean( L, true );
    return 1;
}

static LUA_DECLARE( getEnv )
{
    CResource *res = (CResource*)lua_getmethodtrans( L );

    lua_State *remoteState = *res->GetVM();
    lua_pushvalue( remoteState, LUA_GLOBALSINDEX );
    lua_xmove( remoteState, L, 1 );
    return 1;
}

static const luaL_Reg resource_interface[] =
{
    LUA_METHOD( loadScript ),
    LUA_METHOD( getEnv ),
    { NULL, NULL }
};

static LUA_DECLARE( constructor )
{
    CResource *res = (CResource*)lua_touserdata( L, lua_upvalueindex( 1 ) );

    ILuaClass *j = lua_refclass( L, LUA_ENVIRONINDEX );
    j->SetTransmit( LUACLASS_RESOURCE_DERIVE, res );

    lua_basicextend( L );

    j->RegisterInterfaceTrans( L, resource_interface, 0, LUACLASS_RESOURCE_DERIVE );
    return 0;
}

CResource::CResource( CLuaMain& main, unsigned short id, const filePath& name, CFileTranslator& root ) : Resource( main, id, name, root )
{
    lua_State *L = *main;

    PushStack( L );
    lua_pushlightuserdata( L, this );
    lua_pushcclosure( L, constructor, 1 );
    luaJ_extend( L, -2, 0 );
    lua_pop( L, 1 );
}

CResource::~CResource()
{
}