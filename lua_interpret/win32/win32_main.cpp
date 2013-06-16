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

static const luaL_Reg win32_interface[] =
{
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