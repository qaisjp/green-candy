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

// Useful quick data containers.
namespace gl_internalFormat
{
    const GLparamdesc*  GetColorFormatByHash( glDriver *driver, unsigned int hash );
    const GLparamdesc*  GetDepthFormatByHash( glDriver *driver, unsigned int hash );
    const GLparamdesc*  GetStencilFormatByHash( glDriver *driver, unsigned int hash );
    const GLparamdesc*  GetCompressedFormatByHash( glDriver *driver, unsigned int hash );

    const GLparamdesc*  GetParamByName( glDriver *driver, const char *name, size_t nameLen );

    void    PushAvailableParams( lua_State *L, glDriver *driver );
};

namespace gl_blendfunc
{
    const GLparamdesc*  GetParamByName( const glDriver *driver, const char *name, size_t nameLen );
};

#endif //_LUA_OPENGL_ENUMS_