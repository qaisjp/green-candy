/*****************************************************************************
*
*  PROJECT:     Lua Interpreter
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        glDriver/gl_main.h
*  PURPOSE:     OpenGL driver for MTA:Lua
*  DEVELOPERS:  Martin Turski <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#ifndef _OPENGL_MAIN_
#define _OPENGL_MAIN_

#include "gl_lua.h"

#define LUACLASS_GLDRIVER   41

struct glDriver : public LuaClass
{
    glDriver( lua_State *L, Win32Dialog *wnd );
    ~glDriver( void );

    Win32Dialog *m_window;
    HDC deviceContext;
    HGLRC glContext;

    RwListEntry <glDriver> rootNode;
};

void luagl_initDrivers( void );
void luagl_shutdownDrivers( void );
void luagl_pulseDrivers( lua_State *L );

// Global OpenGL exports
extern HMODULE glMainLibrary;

#endif //_OPENGL_MAIN_