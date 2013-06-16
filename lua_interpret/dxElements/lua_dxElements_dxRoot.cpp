/*****************************************************************************
*
*  PROJECT:     Lua Interpreter
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        dxElements/lua_dxElements_dxRoot.cpp
*  PURPOSE:     Lua dxElements GUI development environment
*  DEVELOPERS:  Martin Turski <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#include <StdInc.h>

static LUA_DECLARE( getRenderCapture )
{
    //todo
    return 0;
}

static LUA_DECLARE( getScreenSize )
{
    //todo
    return 0;
}

static LUA_DECLARE( getScreenPosition )
{
    lua_pushnumber( L, 0 );
    lua_pushnumber( L, 0 );
    return 2;
}

static LUA_DECLARE( getAbsolutePosition )
{
    lua_pushnumber( L, 0 );
    lua_pushnumber( L, 0 );
    return 2;
}

static LUA_DECLARE( getAbsoluteMousePosition )
{
    //todo
    return 0;
}

static LUA_DECLARE( getMousePosition )
{
    //todo
    return 0;
}

static LUA_DECLARE( getPosition )
{
    lua_pushnumber( L, 0 );
    lua_pushnumber( L, 0 );
    return 2;
}

static LUA_DECLARE( getSize )
{
    //todo
    return 0;
}

static LUA_DECLARE( isInLocalArea )
{
    lua_pushboolean( L, true );
    return 1;
}

static LUA_DECLARE( test )
{
    dxElement *element = (dxElement*)lua_getmethodtrans( L );

    element->PushMethod( L, "isHit" );
    lua_call( L, 0, 1 );
    return 1;
}

static LUA_DECLARE( getRenderTarget )
{
    //todo
    return 0;
}

static LUA_DECLARE( destroyRenderTarget )
{
    //todo
    return 0;
}

static LUA_DECLARE( ready )
{
    lua_pushboolean( L, true );
    return 1;
}

static LUA_DECLARE( preRender )
{
    //todo
    return 0;
}

static LUA_DECLARE( resetRenderTarget )
{
    //todo
    return 0;
}

static luaL_Reg root_interface[] =
{
    LUA_METHOD( getRenderCapture ),
    LUA_METHOD( getScreenSize ),
    LUA_METHOD( getScreenPosition ),
    LUA_METHOD( getAbsolutePosition ),
    LUA_METHOD( getAbsoluteMousePosition ),
    LUA_METHOD( getMousePosition ),
    LUA_METHOD( getPosition ),
    LUA_METHOD( getSize ),
    LUA_METHOD( isInLocalArea ),
    LUA_METHOD( test ),
    LUA_METHOD( getRenderTarget ),
    LUA_METHOD( destroyRenderTarget ),
    LUA_METHOD( ready ),
    LUA_METHOD( preRender ),
    LUA_METHOD( resetRenderTarget ),
    { NULL, NULL }
};

void lua_dxElement_extendRoot( lua_State *L, dxElement *element )
{
    ILuaClass *j = element->GetClass();

    j->RegisterInterfaceTrans( L, root_interface, 0, LUACLASS_DXELEMENT );

    element->markEnv = false;
}