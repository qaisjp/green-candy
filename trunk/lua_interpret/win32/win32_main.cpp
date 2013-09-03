/*****************************************************************************
*
*  PROJECT:     Lua Interpreter
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        win32/win32_main.cpp
*  PURPOSE:     Win32 MTA:Lua main logic
*  DEVELOPERS:  Martin Turski <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#include <StdInc.h>

static LUA_DECLARE( setCursorPos )
{
    int x, y;

    LUA_ARGS_BEGIN;
    argStream.ReadNumber( x );
    argStream.ReadNumber( y );
    LUA_ARGS_END;

    lua_pushboolean( L, SetCursorPos( x, y ) != FALSE );
    return 1;
}

static LUA_DECLARE( getCursorPos )
{
    POINT mousePos;
    LUA_CHECK( GetCursorPos( &mousePos ) );

    lua_pushnumber( L, mousePos.x );
    lua_pushnumber( L, mousePos.y );
    return 2;
}

static LUA_DECLARE( getWindowRect )
{
    RECT winRect;
    winRect.top = 0;
    winRect.left = 0;

    LUA_ARGS_BEGIN;
    argStream.ReadNumber( winRect.right );
    argStream.ReadNumber( winRect.bottom );
    LUA_ARGS_END;

    AdjustWindowRect( &winRect, WS_SYSMENU, false );

    lua_pushnumber( L, winRect.right );
    lua_pushnumber( L, winRect.bottom );
    return 2;
}

static const luaL_Reg win32_interface[] =
{
    LUA_METHOD( setCursorPos ),
    LUA_METHOD( getCursorPos ),
    LUA_METHOD( getWindowRect ),
    { NULL, NULL }
};

void luawin32_open( lua_State *L )
{
    lua_newtable( L );
    luaL_openlib( L, NULL, win32_interface, 0 );
    luawin32_extendDialogs( L );
    lua_setfield( L, LUA_ENVIRONINDEX, "win32" );
}

void luawin32_pulse()
{
    luawin32_updateWindows();
}