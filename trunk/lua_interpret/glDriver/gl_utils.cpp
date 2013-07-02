/*****************************************************************************
*
*  PROJECT:     Lua Interpreter
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        glDriver/gl_utils.cpp
*  PURPOSE:     OpenGL driver utilities
*  DEVELOPERS:  Martin Turski <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#include <StdInc.h>
#include "gl_internal.h"

enum eCullModeType : GLenum
{ };

DECLARE_ENUM( eCullModeType );

IMPLEMENT_ENUM_BEGIN( eCullModeType )
    ADD_ENUM( GL_FRONT, "front" )
    ADD_ENUM( GL_BACK, "back" )
IMPLEMENT_ENUM_END( "eCullModeType" )

static LUA_DECLARE( cullFace )
{
    eCullModeType cullType;

    LUA_ARGS_BEGIN;
    argStream.ReadEnumString( cullType );
    LUA_ARGS_END;

    glDriver *driver = (glDriver*)lua_getmethodtrans( L );
    glContextStack context( driver );

    _glCullFace( cullType );
    LUA_SUCCESS;
}

enum eShadeModelType : GLenum
{ };

DECLARE_ENUM( eShadeModelType );

IMPLEMENT_ENUM_BEGIN( eShadeModelType )
    ADD_ENUM( GL_FLAT, "flag" )
    ADD_ENUM( GL_SMOOTH, "smooth" )
IMPLEMENT_ENUM_END( "eShadeModelType" )

static LUA_DECLARE( shadeModel )
{
    eShadeModelType shadeModel;

    LUA_ARGS_BEGIN;
    argStream.ReadEnumString( shadeModel );
    LUA_ARGS_END;

    glDriver *driver = (glDriver*)lua_getmethodtrans( L );
    glContextStack context( driver );

    _glShadeModel( shadeModel );
    LUA_SUCCESS;
}

static LUA_DECLARE( blendFunc )
{
    size_t srcLen;
    lua_String src = lua_tolstring( L, 1, &srcLen );

    if ( !src )
        goto srcError;

    size_t dstLen;
    lua_String dst = lua_tolstring( L, 2, &dstLen );

    if ( !dst )
        goto dstError;

    glDriver *driver = (glDriver*)lua_getmethodtrans( L );

    const GLparamdesc *srcFactor = gl_blendfunc::GetParamByName( driver, src, srcLen );
    
    if ( !src )
        goto srcError;

    const GLparamdesc *dstFactor = gl_blendfunc::GetParamByName( driver, dst, dstLen );

    if ( !dst )
        goto dstError;

    // Run context operation.
    {
        glContextStack context( driver );

        _glBlendFunc( srcFactor->builtin, dstFactor->builtin );
    }

    LUA_SUCCESS;
    
srcError:
    throw lua_exception( L, LUA_ERRRUN, "invalid source factor" );

dstError:
    throw lua_exception( L, LUA_ERRRUN, "invalid dest factor" );
}

enum eAlphaRefFunc : GLenum
{ };

DECLARE_ENUM( eAlphaRefFunc );

IMPLEMENT_ENUM_BEGIN( eAlphaRefFunc )
    ADD_ENUM( GL_NEVER, "never" )
    ADD_ENUM( GL_LESS, "less" )
    ADD_ENUM( GL_EQUAL, "equal" )
    ADD_ENUM( GL_LEQUAL, "lequal" )
    ADD_ENUM( GL_GREATER, "greater" )
    ADD_ENUM( GL_NOTEQUAL, "notequal" )
    ADD_ENUM( GL_GEQUAL, "gequal" )
    ADD_ENUM( GL_ALWAYS, "always" )
IMPLEMENT_ENUM_END( "eAlphaRefFunc" )

static LUA_DECLARE( alphaFunc )
{
    eAlphaRefFunc func;
    GLclampf ref;

    LUA_ARGS_BEGIN;
    argStream.ReadEnumString( func );
    argStream.ReadNumber( ref );
    LUA_ARGS_END;

    glDriver *driver = (glDriver*)lua_getmethodtrans( L );
    glContextStack context( driver );

    _glAlphaFunc( func, ref );
    LUA_SUCCESS;
}

const luaL_Reg driver_util_interface[] =
{
    LUA_METHOD( cullFace ),
    LUA_METHOD( shadeModel ),
    LUA_METHOD( blendFunc ),
    LUA_METHOD( alphaFunc ),
    { NULL, NULL }
};