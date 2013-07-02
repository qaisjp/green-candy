/*****************************************************************************
*
*  PROJECT:     Lua Interpreter
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        utils/s2g/s2g_main.cpp
*  PURPOSE:     MTA:Lua S2 Games texture cache format loader
*  DEVELOPERS:  Martin Turski <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#include <StdInc.h>

static LUA_DECLARE( load )
{
    return 0;
}

const static luaL_Reg s2g_library[] =
{
    LUA_METHOD( load ),
    { NULL, NULL }
};

void luas2g_open( lua_State *L )
{
    luaL_openlib( L, "s2g", s2g_library, 0 );
    lua_pop( L, 1 );
}