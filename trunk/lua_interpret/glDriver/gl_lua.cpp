/*****************************************************************************
*
*  PROJECT:     Lua Interpreter
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        glDriver/gl_lua.cpp
*  PURPOSE:     OpenGL driver for MTA:Lua
*  DEVELOPERS:  Martin Turski <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#include <StdInc.h>

static LUA_DECLARE( create )
{
    Win32Dialog *dlg;

    LUA_ARGS_BEGIN;
    argStream.ReadClass( dlg, LUACLASS_WIN32DIALOG );
    LUA_ARGS_END;

    glDriver *driver = new glDriver( L, dlg );
    driver->PushStack( L );
    return 1;
}

static LUA_DECLARE( intToColor )
{
    unsigned int color;

    LUA_ARGS_BEGIN;
    argStream.ReadNumber( color );
    LUA_ARGS_END;

    RwColor rwcolor = color;

    lua_pushnumber( L, (double)rwcolor.r / 255.0f );
    lua_pushnumber( L, (double)rwcolor.g / 255.0f );
    lua_pushnumber( L, (double)rwcolor.b / 255.0f );
    lua_pushnumber( L, (double)rwcolor.a / 255.0f );
    return 4;
}

static const luaL_Reg gl_interface[] =
{
    LUA_METHOD( create ),
    LUA_METHOD( intToColor ),
    { NULL, NULL }
};

void luagl_open( lua_State *L )
{
    // Only initialize OpenGL if it is available on this system.
    if ( glMainLibrary )
    {
        luaL_openlib( L, "gl", gl_interface, 0 );
        lua_pop( L, 1 );
    }
}