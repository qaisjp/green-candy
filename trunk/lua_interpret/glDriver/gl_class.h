/*****************************************************************************
*
*  PROJECT:     Lua Interpreter
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        glDriver/gl_class.h
*  PURPOSE:     OpenGL driver class impl. for every OpenGL class
*  DEVELOPERS:  Martin Turski <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#ifndef _LUA_OPENGL_CLASS_
#define _LUA_OPENGL_CLASS_

#define LUACLASS_OPENGL_CLASS   57

// Every OpenGL struct/class which is exported to Lua should
// inherit this type. Every glClass must have a driver it belongs to.
// Operations from another driver on a glClass who is not owned by
// said driver are prohibited (undefined behaviour).
struct glClass : public LuaInstance
{
    glClass( lua_State *L, glDriver *driver );
    ~glClass( void );

    glDriver *m_driver;
};

#endif //_LUA_OPENGL_CLASS_