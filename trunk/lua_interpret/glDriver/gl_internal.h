/*****************************************************************************
*
*  PROJECT:     Lua Interpreter
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        glDriver/gl_internal.h
*  PURPOSE:     OpenGL driver internal/private header file
*  DEVELOPERS:  Martin Turski <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#ifndef _LUA_OPENGL_PRIVATE_
#define _LUA_OPENGL_PRIVATE_

#include <gl.h>
#include <gl/glext.h>
#include "gl_func.h"
#include "gl_texture.h"
#include "gl_context.h"
#include "gl_fbo.h"
#include "gl_enum.h"
#include "gl_utils.h"

// Quick context switching object (to stack context acquisitions safely)
struct glContextStack
{
    // Last driver used for context stack
    static glDriver *lastStackDriver;

    __forceinline glContextStack( glDriver *driver )
    {
        if ( lastStackDriver != driver )
        {
            // Save previous context
            dc = _wglGetCurrentDC();
            glrc = _wglGetCurrentContext();

            // Change current context
            _wglMakeCurrent( driver->deviceContext, driver->glContext );

            prevStackDriver = lastStackDriver;
            lastStackDriver = driver;

            contextSwapped = true;
        }
        else
            contextSwapped = false;
    }

    __forceinline ~glContextStack( void )
    {
        if ( contextSwapped )
        {
            lastStackDriver = prevStackDriver;

            _wglMakeCurrent( dc, glrc );
        }
    }

    bool contextSwapped;
    HDC dc;
    HGLRC glrc;
    glDriver *prevStackDriver;
};

#endif //_LUA_OPENGL_PRIVATE_