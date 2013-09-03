/*****************************************************************************
*
*  PROJECT:     Lua Interpreter
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        glDriver/gl_shading.h
*  PURPOSE:     OpenGL driver programmable shading implementation
*  DEVELOPERS:  Martin Turski <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#ifndef _OPENGL_SHADING_IMPLEMENTATION_
#define _OPENGL_SHADING_IMPLEMENTATION_

#define LUACLASS_GLSLPROGRAM_ARB    54
#define LUACLASS_GLSLSHADER_ARB     55
#define LUACLASS_GLSLOBJECT_ARB     56

struct glObjectARB : public LuaInstance
{
    glObjectARB( lua_State *L, glDriver *driver, GLhandleARB handle );
    ~glObjectARB( void );

    glDriver *m_driver;
    GLhandleARB m_handle;
};

struct glShaderARB : public glObjectARB
{
    glShaderARB( lua_State *L, glDriver *driver, GLhandleARB handle, GLenum type );
    ~glShaderARB( void );

    GLenum m_type;
};

struct glProgramARB : public glObjectARB
{
    glProgramARB( lua_State *L, glDriver *driver, GLhandleARB handle );
    ~glProgramARB( void );

    typedef std::list <glShaderARB*> shaderList_t;
    shaderList_t m_attachedShaders;

    unsigned int m_numAttachedShaders;
};

extern const luaL_Reg shader_driver_interface[];

#endif //_OPENGL_SHADING_IMPLEMENTATION_