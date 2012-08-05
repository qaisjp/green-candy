/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*               (Shared logic for modifications)
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        mods/shared_logic/CClientTexture.cpp
*  PURPOSE:     Core texture management interface for deathmatch
*  DEVELOPERS:  The_GTA <quiret@gmx.de>
*
*****************************************************************************/

#include <StdInc.h>

static const luaL_Reg texture_interface[] =
{
    { NULL, NULL }
};

static int luaconstructor_texture( lua_State *L )
{
    CClientTexture *tex = (CClientTexture*)lua_touserdata( L, lua_upvalueindex( 1 ) );

    ILuaClass& j = *lua_refclass( L, 1 );
    j.SetTransmit( LUACLASS_CORETEXTURE, tex );

    lua_pushvalue( L, LUA_ENVIRONINDEX );
    lua_pushvalue( L, lua_upvalueindex( 1 ) );
    luaL_openlib( L, NULL, texture_interface, 1 );

    lua_pushlstring( L, "core-texture", 8 );
    lua_setfield( L, LUA_ENVIRONINDEX, "__type" );
    return 0;
}

CClientTexture::CClientTexture( CClientManager* pManager, ElementID ID, LuaClass& root, CTextureItem* pTextureItem ) : CClientMaterial( pManager, ID, root )
{
    // Lua instancing
    lua_State *L = root.GetVM();

    PushStack( L );
    lua_pushlightuserdata( L, this );
    lua_pushcclosure( L, luaconstructor_texture, 1 );
    luaJ_extend( L, -2, 0 );
    lua_pop( L, 1 );

    SetTypeName( "texture" );
}

static const luaL_Reg target_interface[] =
{
    { NULL, NULL }
};

static int luaconstructor_target( lua_State *L )
{
    CClientRenderTarget *tex = (CClientRenderTarget*)lua_touserdata( L, lua_upvalueindex( 1 ) );

    ILuaClass& j = *lua_refclass( L, 1 );
    j.SetTransmit( LUACLASS_CORERENDERTARGET, tex );

    lua_pushvalue( L, LUA_ENVIRONINDEX );
    lua_pushvalue( L, lua_upvalueindex( 1 ) );
    luaL_openlib( L, NULL, target_interface, 1 );

    lua_pushlstring( L, "rendertarget", 8 );
    lua_setfield( L, LUA_ENVIRONINDEX, "__type" );
    return 0;
}

CClientRenderTarget::CClientRenderTarget( CClientManager* pManager, ElementID ID, LuaClass& root, CRenderTargetItem* pRenderTargetItem ) : CClientTexture( pManager, ID, root, pRenderTargetItem )
{
    // Lua instancing
    lua_State *L = root.GetVM();

    PushStack( L );
    lua_pushlightuserdata( L, this );
    lua_pushcclosure( L, luaconstructor_target, 1 );
    luaJ_extend( L, -2, 0 );
    lua_pop( L, 1 );

    SetTypeName( "rendertarget" );
}

static const luaL_Reg screensource_interface[] =
{
    { NULL, NULL }
};

static int luaconstructor_screensource( lua_State *L )
{
    CClientScreenSource *tex = (CClientScreenSource*)lua_touserdata( L, lua_upvalueindex( 1 ) );

    ILuaClass& j = *lua_refclass( L, 1 );
    j.SetTransmit( LUACLASS_CORESCREENSOURCE, tex );

    lua_pushvalue( L, LUA_ENVIRONINDEX );
    lua_pushvalue( L, lua_upvalueindex( 1 ) );
    luaL_openlib( L, NULL, screensource_interface, 1 );

    lua_pushlstring( L, "screensource", 12 );
    lua_setfield( L, LUA_ENVIRONINDEX, "__type" );
    return 0;
}

CClientScreenSource::CClientScreenSource( CClientManager* pManager, ElementID ID, LuaClass& root, CScreenSourceItem* pScreenSourceItem ) : CClientTexture( pManager, ID, root, pScreenSourceItem )
{
    // Lua instancing
    lua_State *L = root.GetVM();

    PushStack( L );
    lua_pushlightuserdata( L, this );
    lua_pushcclosure( L, luaconstructor_screensource, 1 );
    luaJ_extend( L, -2, 0 );
    lua_pop( L, 1 );

    SetTypeName( "screensource" );
}