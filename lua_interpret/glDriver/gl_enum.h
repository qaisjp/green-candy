/*****************************************************************************
*
*  PROJECT:     Lua Interpreter
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        glDriver/gl_enum.h
*  PURPOSE:     OpenGL driver enumerations and quick lookup routines
*  DEVELOPERS:  Martin Turski <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#ifndef _LUA_OPENGL_ENUMS_
#define _LUA_OPENGL_ENUMS_

// Used to provide parameters based on context version.
struct glDriver;

struct GLparamdesc
{
    const char *name;
    GLenum builtin;
    unsigned int hash;
};

enum GLeParamType : GLenum
{
    GLPARAM_BOOLEAN,
    GLPARAM_FLOAT,
    GLPARAM_INT,
    GLPARAM_DOUBLE
};

// Structure used to describe parameters used in glGet* functions.
struct GLparaminfo
{
    const std::string name;
    GLenum builtin;
    unsigned int hash;
    GLeParamType type;
    unsigned int numArgs;
};

// Useful quick data containers.
namespace gl_internalFormat
{
    const GLparamdesc*  GetColorFormatByHash( glDriver *driver, unsigned int hash );
    const GLparamdesc*  GetDepthFormatByHash( glDriver *driver, unsigned int hash );
    const GLparamdesc*  GetStencilFormatByHash( glDriver *driver, unsigned int hash );
    const GLparamdesc*  GetCompressedFormatByHash( glDriver *driver, unsigned int hash );

    const GLparamdesc*  GetParamByName( glDriver *driver, const char *name, size_t nameLen );

    void                PushAvailableParams( lua_State *L, const glDriver *driver );
};

namespace gl_blendfunc
{
    const GLparamdesc*  GetParamByName( const glDriver *driver, const char *name, size_t nameLen );
};

namespace gl_parameters
{
    const GLparaminfo*  GetParamByName( const glDriver *driver, const char *name, size_t nameLen );
    void                PushAvailableParams( lua_State *L, const glDriver *driver );
};

namespace gl_capabilities
{
    const GLparamdesc*  GetParamByName( const glDriver *driver, const char *name, size_t nameLen );
    void                PushAvailableParams( lua_State *L, const glDriver *driver );
};

namespace gl_stringParams
{
    const GLparamdesc*  GetParamByName( const glDriver *driver, const char *name, size_t nameLen );
    void                PushAvailableParams( lua_State *L, const glDriver *driver );
};

namespace gl_objectARB
{
    const GLparaminfo*  GetParamByName( const glDriver *driver, const char *name, size_t nameLen );
    void                PushAvailableParams( lua_State *L, const glDriver *driver );
};

namespace gl_programARB
{
    const GLparaminfo*  GetParamByName( const glDriver *driver, const char *name, size_t nameLen );
    void                PushAvailableParams( lua_State *L, const glDriver *driver );
};

namespace gl_shaderARB
{
    const GLparaminfo*  GetParamByName( const glDriver *driver, const char *name, size_t nameLen );
    void                PushAvailableParams( lua_State *L, const glDriver *driver );
};

#endif //_LUA_OPENGL_ENUMS_