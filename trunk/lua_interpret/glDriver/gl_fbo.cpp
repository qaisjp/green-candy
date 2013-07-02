/*****************************************************************************
*
*  PROJECT:     Lua Interpreter
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        glDriver/gl_fbo.cpp
*  PURPOSE:     OpenGL driver render buffer extension
*  DEVELOPERS:  Martin Turski <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#include <StdInc.h>
#include "gl_internal.h"

namespace renderbuffer
{
    static LUA_DECLARE( getSize )
    {
        glRenderBufferBase *buf = (glRenderBufferBase*)lua_getmethodtrans( L );

        lua_pushnumber( L, buf->m_width );
        lua_pushnumber( L, buf->m_height );
        return 2;
    }

    static const luaL_Reg renderbufferbase_interface[] =
    {
        LUA_METHOD( getSize ),
        { NULL, NULL }
    };

    static LUA_DECLARE( base_constructor )
    {
        glRenderBufferBase *buf = (glRenderBufferBase*)lua_touserdata( L, lua_upvalueindex( 1 ) );

        ILuaClass *j = lua_refclass( L, LUA_ENVIRONINDEX );
        j->SetTransmit( LUACLASS_RENDERBUFFER, buf );

        j->RegisterInterfaceTrans( L, renderbufferbase_interface, 0, LUACLASS_RENDERBUFFER );

        lua_pushcstring( L, "glrenderbuffer" );
        lua_setfield( L, LUA_ENVIRONINDEX, "__type" );
        return 0;
    }

    namespace ext
    {
        enum eBindOperation : GLenum
        { };

        DECLARE_ENUM( eBindOperation );

        IMPLEMENT_ENUM_BEGIN( eBindOperation )
            ADD_ENUM( GL_DRAW_FRAMEBUFFER_EXT, "draw" )
            ADD_ENUM( GL_READ_FRAMEBUFFER_EXT, "read" )
        IMPLEMENT_ENUM_END( "eBindOperation" )

        static LUA_DECLARE( bindColor )
        {
            glFrameBufferBase *fbo;
            eBindOperation bindOp;
            unsigned int attachIndex;

            LUA_ARGS_BEGIN;
            argStream.ReadClass( fbo, LUACLASS_FRAMEBUFFER );
            argStream.ReadEnumString( bindOp );
            argStream.ReadNumber( attachIndex );
            LUA_ARGS_END;

            glRenderBufferEXT *buf = (glRenderBufferEXT*)lua_getmethodtrans( L );

            glDriver *driver = buf->m_driver;

            LUA_CHECK( driver == fbo->m_driver && fbo->m_type == FRAMEBUFFER_EXT && attachIndex < (unsigned int)driver->maxFBOColorAttachmentsEXT );

            // Check that we ain't re-attaching redundantly.
            LuaClass *prevAttach = fbo->colorAttachments[attachIndex];

            LUA_CHECK( prevAttach != buf );

            // Execute context operation.
            GLenum error;
            {
                glContextStack context( driver );
                glContextDescriptor *contextInfo = driver->contextInfo;

                glFrameBufferContext fbo_context( driver, fbo, bindOp );

                _glGetError();

                contextInfo->glFramebufferRenderbufferEXT( bindOp, GL_COLOR_ATTACHMENT0_EXT + attachIndex, GL_RENDERBUFFER_EXT, buf->m_bufIndex );

                error = _glGetError();
            }

            if ( error != 0 )
            {
                lua_pushboolean( L, false );
                lua_pushnumber( L, error );
                return 2;
            }

            // Exchange attachments.
            if ( prevAttach )
                prevAttach->DecrementMethodStack();

            fbo->colorAttachments[attachIndex] = buf;

            buf->IncrementMethodStack();

            lua_pushboolean( L, true );
            return 1;
        }

        template <GLenum attachType>
        __forceinline LUA_DECLARE( single_attach_func )
        {
            glFrameBufferBase *fbo;
            eBindOperation bindOp;

            LUA_ARGS_BEGIN;
            argStream.ReadClass( fbo, LUACLASS_FRAMEBUFFER );
            argStream.ReadEnumString( bindOp );
            LUA_ARGS_END;

            glRenderBufferEXT *buf = (glRenderBufferEXT*)lua_getmethodtrans( L );

            glDriver *driver = buf->m_driver;

            LUA_CHECK( driver == fbo->m_driver && fbo->m_type == FRAMEBUFFER_EXT );

            // Check that we are not re-attaching already attached objects
            LuaClass *prevAttach = NULL;

            if ( attachType == GL_DEPTH_ATTACHMENT_EXT )
                prevAttach = fbo->depthAttachment;
            else if ( attachType == GL_STENCIL_ATTACHMENT_EXT )
                prevAttach = fbo->stencilAttachment;

            LUA_CHECK( prevAttach != buf );

            // Run context operation.
            GLenum error;
            {
                glContextStack context( driver );
                glContextDescriptor *contextInfo = driver->contextInfo;

                glFrameBufferContext fbo_context( driver, fbo, bindOp );

                _glGetError();

                contextInfo->glFramebufferRenderbufferEXT( bindOp, attachType, GL_RENDERBUFFER_EXT, buf->m_bufIndex );

                error = _glGetError();
            }

            if ( error != 0 )
            {
                lua_pushboolean( L, false );
                lua_pushnumber( L, error );
                return 2;
            }

            // Exchange the attachments
            if ( prevAttach )
                prevAttach->DecrementMethodStack();

            if ( attachType == GL_DEPTH_ATTACHMENT_EXT )
                fbo->depthAttachment = buf;
            else if ( attachType == GL_STENCIL_ATTACHMENT_EXT )
                fbo->stencilAttachment = buf;

            buf->IncrementMethodStack();

            lua_pushboolean( L, true );
            return 1;
        }

        static LUA_DECLARE( bindDepth )
        {
            return single_attach_func <GL_DEPTH_ATTACHMENT_EXT> ( L );
        }

        static LUA_DECLARE( bindStencil )
        {
            return single_attach_func <GL_STENCIL_ATTACHMENT_EXT> ( L );
        }

        static const luaL_Reg renderbufferEXT_interface[] =
        {
            LUA_METHOD( bindColor ),
            LUA_METHOD( bindDepth ),
            LUA_METHOD( bindStencil ),
            { NULL, NULL }
        };

        static LUA_DECLARE( constructor )
        {
            glRenderBufferEXT *buf = (glRenderBufferEXT*)lua_touserdata( L, lua_upvalueindex( 1 ) );

            ILuaClass *j = lua_refclass( L, LUA_ENVIRONINDEX );
            
            j->RegisterInterfaceTrans( L, renderbufferEXT_interface, 0, LUACLASS_RENDERBUFFER );
            return 0;
        }
    };

    namespace core
    {
        enum eBindOperation : GLenum
        { };

        DECLARE_ENUM( eBindOperation );

        IMPLEMENT_ENUM_BEGIN( eBindOperation )
            ADD_ENUM( GL_DRAW_FRAMEBUFFER, "draw" )
            ADD_ENUM( GL_READ_FRAMEBUFFER, "read" )
        IMPLEMENT_ENUM_END( "eBindOperation" )

        static LUA_DECLARE( bindColor )
        {
            glFrameBufferBase *fbo;
            eBindOperation bindOp;
            unsigned int attachIndex;

            LUA_ARGS_BEGIN;
            argStream.ReadClass( fbo, LUACLASS_FRAMEBUFFER );
            argStream.ReadEnumString( bindOp );
            argStream.ReadNumber( attachIndex );
            LUA_ARGS_END;

            glRenderBuffer *buf = (glRenderBuffer*)lua_getmethodtrans( L );

            glDriver *driver = buf->m_driver;

            LUA_CHECK( driver == fbo->m_driver && fbo->m_type == FRAMEBUFFER_CORE && attachIndex < (unsigned int)driver->maxFBOColorAttachments );

            // Check that we are not trying to re-attached already attached objects
            LuaClass *prevAttach = fbo->colorAttachments[attachIndex];

            LUA_CHECK( prevAttach != buf );

            // Run context operation.
            GLenum error;
            {
                glContextStack context( driver );
                glContextDescriptor *contextInfo = driver->contextInfo;

                glFrameBufferContext fbo_context( driver, fbo, bindOp );

                _glGetError();

                contextInfo->glFramebufferRenderbuffer( bindOp, GL_COLOR_ATTACHMENT0 + attachIndex, GL_RENDERBUFFER, buf->m_bufIndex );

                error = _glGetError();
            }

            if ( error != 0 )
            {
                lua_pushboolean( L, false );
                lua_pushnumber( L, error );
                return 2;
            }

            // Exchange the attachments
            if ( prevAttach )
                prevAttach->DecrementMethodStack();

            fbo->colorAttachments[attachIndex] = buf;

            buf->IncrementMethodStack();

            lua_pushboolean( L, true );
            return 1;
        }

        template <GLenum attachType>
        __forceinline LUA_DECLARE( single_attach_func )
        {
            glFrameBufferBase *fbo;
            eBindOperation bindOp;

            LUA_ARGS_BEGIN;
            argStream.ReadClass( fbo, LUACLASS_FRAMEBUFFER );
            argStream.ReadEnumString( bindOp );
            LUA_ARGS_END;

            glRenderBuffer *buf = (glRenderBuffer*)lua_getmethodtrans( L );

            glDriver *driver = buf->m_driver;

            LUA_CHECK( driver == fbo->m_driver && fbo->m_type == FRAMEBUFFER_CORE );

            // Check that we are not re-attaching the same object
            LuaClass *prevAttach = NULL;
            LuaClass *prevAttach2 = NULL;

            if ( attachType == GL_DEPTH_ATTACHMENT )
            {
                prevAttach = fbo->depthAttachment;

                LUA_CHECK( prevAttach != buf );
            }
            else if ( attachType == GL_STENCIL_ATTACHMENT )
            {
                prevAttach = fbo->stencilAttachment;

                LUA_CHECK( prevAttach != buf );
            }
            else if ( attachType == GL_DEPTH_STENCIL_ATTACHMENT )
            {
                prevAttach = fbo->depthAttachment;
                prevAttach2 = fbo->stencilAttachment;

                LUA_CHECK( prevAttach != buf || prevAttach2 != buf );
            }

            // Perform context operation.
            GLenum error;
            {
                glContextStack context( driver );
                glContextDescriptor *contextInfo = driver->contextInfo;

                glFrameBufferContext fbo_context( driver, fbo, bindOp );

                _glGetError();

                contextInfo->glFramebufferRenderbuffer( bindOp, attachType, GL_RENDERBUFFER, buf->m_bufIndex );

                error = _glGetError();
            }

            if ( error != 0 )
            {
                lua_pushboolean( L, false );
                lua_pushnumber( L, error );
                return 2;
            }

            // Exchange the attachments
            if ( attachType == GL_DEPTH_ATTACHMENT )
                fbo->depthAttachment = buf;
            else if ( attachType == GL_STENCIL_ATTACHMENT )
                fbo->stencilAttachment = buf;
            else if ( attachType == GL_DEPTH_STENCIL_ATTACHMENT )
            {
                fbo->depthAttachment = buf;
                fbo->stencilAttachment = buf;

                buf->IncrementMethodStack();
            }

            buf->IncrementMethodStack();

            // We do not require the old attachments anymore.
            if ( prevAttach )
                prevAttach->DecrementMethodStack();

            if ( prevAttach2 )
                prevAttach2->DecrementMethodStack();
            
            lua_pushboolean( L, true );
            return 1;
        }

        static LUA_DECLARE( bindDepth )
        {
            return single_attach_func <GL_DEPTH_ATTACHMENT> ( L );
        }

        static LUA_DECLARE( bindStencil )
        {
            return single_attach_func <GL_STENCIL_ATTACHMENT> ( L );
        }

        static LUA_DECLARE( bindDepthStencil )
        {
            return single_attach_func <GL_DEPTH_STENCIL_ATTACHMENT> ( L );
        }

        static const luaL_Reg renderbuffer_interface[] =
        {
            LUA_METHOD( bindColor ),
            LUA_METHOD( bindDepth ),
            LUA_METHOD( bindStencil ),
            LUA_METHOD( bindDepthStencil ),
            { NULL, NULL }
        };

        static LUA_DECLARE( constructor )
        {
            glRenderBuffer *buf = (glRenderBuffer*)lua_touserdata( L, lua_upvalueindex( 1 ) );

            ILuaClass *j = lua_refclass( L, LUA_ENVIRONINDEX );
            
            j->RegisterInterfaceTrans( L, renderbuffer_interface, 0, LUACLASS_RENDERBUFFER );
            return 0;
        }
    };
};

// Shared constructor for all render buffers
glRenderBufferBase::glRenderBufferBase( lua_State *L, glDriver *driver, eRenderBufferType bufType, unsigned int width, unsigned int height ) : LuaInstance( L ), m_driver( driver ), m_width( width ), m_height( height ), m_type( bufType )
{
    Extend( L, renderbuffer::base_constructor );
}

glRenderBufferBase::~glRenderBufferBase( void )
{
}

// Constructor for the EXT renderbuffer extension
glRenderBufferEXT::glRenderBufferEXT( lua_State *L, glDriver *driver, GLenum bufIndex, unsigned int width, unsigned int height ) : glRenderBufferBase( L, driver, RENDERBUFFER_EXT, width, height ), m_bufIndex( bufIndex )
{
    Extend( L, renderbuffer::ext::constructor );
}

glRenderBufferEXT::~glRenderBufferEXT( void )
{
    glContextStack context( m_driver );
    
    m_driver->contextInfo->glDeleteRenderbuffersEXT( 1, &m_bufIndex );
}

// Constructor for the 3.0 core module
glRenderBuffer::glRenderBuffer( lua_State *L, glDriver *driver, GLenum bufIndex, unsigned int width, unsigned int height ) : glRenderBufferBase( L, driver, RENDERBUFFER_CORE, width, height ), m_bufIndex( bufIndex )
{
    Extend( L, renderbuffer::core::constructor );
}

glRenderBuffer::~glRenderBuffer( void )
{
    glContextStack context( m_driver );
    
    m_driver->contextInfo->glDeleteRenderbuffers( 1, &m_bufIndex );
}

__forceinline const GLparamdesc* GetRenderTargetParam( glDriver *driver, const char *name, size_t len )
{
    unsigned int hash = TumblerHash( name, len );

    // Either color-renderable, depth-renderable or stencil-renderable
    if ( const GLparamdesc *info = gl_internalFormat::GetColorFormatByHash( driver, hash ) )
        return info;

    if ( const GLparamdesc *info = gl_internalFormat::GetDepthFormatByHash( driver, hash ) )
        return info;

    if ( const GLparamdesc *info = gl_internalFormat::GetStencilFormatByHash( driver, hash ) )
        return info;

    return NULL;
}

static LUA_DECLARE( createRenderBuffer )
{
    unsigned int width, height;
    std::string format;

    LUA_ARGS_BEGIN;
    argStream.ReadNumber( width );
    argStream.ReadNumber( height );
    argStream.ReadString( format );
    LUA_ARGS_END;

    glDriver *driver = (glDriver*)lua_getmethodtrans( L );

    const GLparamdesc *info = GetRenderTargetParam( driver, format.c_str(), format.size() );

    if ( !info )
    {
        lua_pushboolean( L, false );
        lua_pushcstring( L, "invalid format descriptor" );
        return 2;
    }

    if ( !driver->supports_FBO_EXT && !driver->supports3_0 )
    {
        lua_pushboolean( L, false );
        lua_pushcstring( L, "driver does not support FBO" );
        return 2;
    }

    glContextStack context( driver );
    glContextDescriptor *contextInfo = driver->contextInfo;

    // Reset the error so we can see if the creation failed.
    _glGetError();

    glRenderBufferBase *buf;

    if ( driver->supports3_0 )
    {
        // Do core logic here.
        GLuint bufIndex;
        contextInfo->glGenRenderbuffers( 1, &bufIndex );

        contextInfo->glBindRenderbuffer( GL_RENDERBUFFER, bufIndex );

        contextInfo->glRenderbufferStorage( GL_RENDERBUFFER, info->builtin, width, height );
        
        contextInfo->glBindRenderbuffer( GL_RENDERBUFFER, 0 );

        GLenum error = _glGetError();

        if ( error != 0 )
        {
            contextInfo->glDeleteRenderbuffers( 1, &bufIndex );

            lua_pushboolean( L, false );
            lua_pushnumber( L, error );
            return 2;
        }

        buf = new glRenderBuffer( L, driver, bufIndex, width, height );
    }
    else
    {
        // GL_EXT_framebuffer_object
        GLuint bufIndex;
        contextInfo->glGenRenderbuffersEXT( 1, &bufIndex );

        contextInfo->glBindRenderbufferEXT( GL_RENDERBUFFER_EXT, bufIndex );

        contextInfo->glRenderbufferStorageEXT( GL_RENDERBUFFER_EXT, info->builtin, width, height );

        contextInfo->glBindRenderbufferEXT( GL_RENDERBUFFER_EXT, 0 );

        GLenum error = _glGetError();

        if ( error != 0 )
        {
            contextInfo->glDeleteRenderbuffersEXT( 1, &bufIndex );

            lua_pushboolean( L, false );
            lua_pushnumber( L, error );
            return 2;
        }

        buf = new glRenderBufferEXT( L, driver, bufIndex, width, height );
    }

    buf->SetParent( driver );
    buf->PushStack( L );
    buf->Finalize( L );
    return 1;
}

namespace framebuffer
{
    static LUA_DECLARE( getDepthAttachment )
    {
        glFrameBufferBase *fbo = (glFrameBufferBase*)lua_getmethodtrans( L );
        LuaClass *depthAttach = fbo->depthAttachment;

        if ( !depthAttach )
            return 0;

        depthAttach->PushStack( L );
        return 1;
    }

    static LUA_DECLARE( getStencilAttachment )
    {
        glFrameBufferBase *fbo = (glFrameBufferBase*)lua_getmethodtrans( L );
        LuaClass *stencilAttach = fbo->stencilAttachment;

        if ( !stencilAttach )
            return 0;

        stencilAttach->PushStack( L );
        return 1;
    }

    static const luaL_Reg framebufferbase_interface[] =
    {
        LUA_METHOD( getDepthAttachment ),
        LUA_METHOD( getStencilAttachment ),
        { NULL, NULL }
    };

    static LUA_DECLARE( framebufferbase_constructor )
    {
        glFrameBufferBase *fbo = (glFrameBufferBase*)lua_touserdata( L, lua_upvalueindex( 1 ) );

        ILuaClass *j = lua_refclass( L, LUA_ENVIRONINDEX );
        j->SetTransmit( LUACLASS_FRAMEBUFFER, fbo );

        j->RegisterInterfaceTrans( L, framebufferbase_interface, 0, LUACLASS_FRAMEBUFFER );

        lua_pushcstring( L, "glframebuffer" );
        lua_setfield( L, LUA_ENVIRONINDEX, "__type" );
        return 0;
    }

    namespace ext
    {
        static LUA_DECLARE( getColorAttachment )
        {
            unsigned int attachIndex;

            LUA_ARGS_BEGIN;
            argStream.ReadNumber( attachIndex, 0 );
            LUA_ARGS_END;

            glFrameBufferEXT *fbo = (glFrameBufferEXT*)lua_getmethodtrans( L );

            LUA_CHECK( attachIndex < (unsigned int)fbo->m_driver->maxFBOColorAttachmentsEXT );

            LuaClass *colorAttach = fbo->colorAttachments[attachIndex];

            LUA_CHECK( colorAttach );

            colorAttach->PushStack( L );
            return 1;
        }

        __forceinline void DetachFBOObject( glDriver *driver, glFrameBufferEXT *fbo, GLenum slot, LuaClass *obj )
        {
            glContextStack context( driver );
            glFrameBufferContext( driver, fbo, GL_DRAW_FRAMEBUFFER_EXT );

            // Detach anything that was attached before.
            if ( obj->IsTransmit( LUACLASS_GLTEXTURE ) )
                driver->contextInfo->glFramebufferTexture2DEXT( GL_DRAW_FRAMEBUFFER_EXT, slot, GL_TEXTURE_2D, 0, 0 );
            else
                driver->contextInfo->glFramebufferRenderbufferEXT( GL_DRAW_FRAMEBUFFER_EXT, slot, GL_RENDERBUFFER_EXT, 0 );
        }

        static LUA_DECLARE( unbindColor )
        {
            unsigned int attachIndex;

            LUA_ARGS_BEGIN;
            argStream.ReadNumber( attachIndex );
            LUA_ARGS_END;

            glFrameBufferEXT *fbo = (glFrameBufferEXT*)lua_getmethodtrans( L );
            glDriver *driver = fbo->m_driver;

            LUA_CHECK( attachIndex < (unsigned int)driver->maxFBOColorAttachmentsEXT );

            LuaClass *colorAttach = fbo->colorAttachments[attachIndex];

            LUA_CHECK( colorAttach );

            // Run context operation.
            DetachFBOObject( driver, fbo, GL_COLOR_ATTACHMENT0_EXT + attachIndex, colorAttach );

            fbo->colorAttachments[attachIndex] = NULL;
            colorAttach->DecrementMethodStack();

            lua_pushboolean( L, true );
            return 1;
        }

        template <GLenum attachType>
        __forceinline LUA_DECLARE( single_detach_perform )
        {
            glFrameBufferEXT *fbo = (glFrameBufferEXT*)lua_getmethodtrans( L );
            glDriver *driver = fbo->m_driver;

            LuaClass *attachObj = NULL;

            if ( attachType == GL_DEPTH_ATTACHMENT_EXT )
                attachObj = fbo->depthAttachment;
            else if ( attachType == GL_STENCIL_ATTACHMENT_EXT )
                attachObj = fbo->stencilAttachment;

            LUA_CHECK( attachObj );

            // Run context operation.
            DetachFBOObject( driver, fbo, attachType, attachObj );

            if ( attachType == GL_DEPTH_ATTACHMENT_EXT )
                fbo->depthAttachment = NULL;
            else if ( attachType == GL_STENCIL_ATTACHMENT_EXT )
                fbo->stencilAttachment = NULL;

            attachObj->DecrementMethodStack();

            lua_pushboolean( L, true );
            return 1;
        }

        static LUA_DECLARE( unbindDepth )
        {
            return single_detach_perform <GL_DEPTH_ATTACHMENT_EXT> ( L );
        }

        static LUA_DECLARE( unbindStencil )
        {
            return single_detach_perform <GL_STENCIL_ATTACHMENT_EXT> ( L );
        }

        enum eBindOperation : GLenum
        { };

        DECLARE_ENUM( eBindOperation );

        IMPLEMENT_ENUM_BEGIN( eBindOperation )
            ADD_ENUM( GL_DRAW_FRAMEBUFFER_EXT, "draw" )
            ADD_ENUM( GL_READ_FRAMEBUFFER_EXT, "read" )
        IMPLEMENT_ENUM_END( "eBindOperation" )

        static LUA_DECLARE( checkStatus )
        {
            eBindOperation bindOp;

            LUA_ARGS_BEGIN;
            argStream.ReadEnumString( bindOp );
            LUA_ARGS_END;

            glFrameBufferEXT *fbo = (glFrameBufferEXT*)lua_getmethodtrans( L );
            glDriver *driver = fbo->m_driver;

            glContextStack context( driver );
            glFrameBufferContext fboContext( driver, fbo, bindOp );

            GLenum status = driver->contextInfo->glCheckFramebufferStatusEXT( bindOp );

            switch( status )
            {
            case GL_FRAMEBUFFER_COMPLETE_EXT:
                lua_pushboolean( L, true );
                return 1;
            case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT_EXT:
                lua_pushboolean( L, false );
                lua_pushcstring( L, "incomplete attachment" );
                return 2;
            case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT_EXT:
                lua_pushboolean( L, false );
                lua_pushcstring( L, "incomplete missing attachment" );
                return 2;
            case GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT:
                lua_pushboolean( L, false );
                lua_pushcstring( L, "incomplete dimensions" );
                return 2;
            case GL_FRAMEBUFFER_INCOMPLETE_FORMATS_EXT:
                lua_pushboolean( L, false );
                lua_pushcstring( L, "incomplete formats" );
                return 2;
            case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER_EXT:
                lua_pushboolean( L, false );
                lua_pushcstring( L, "incomplete draw buffer" );
                return 2;
            case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER_EXT:
                lua_pushboolean( L, false );
                lua_pushcstring( L, "incomplete read buffer" );
                return 2;
            case GL_FRAMEBUFFER_UNSUPPORTED_EXT:
                lua_pushboolean( L, false );
                lua_pushcstring( L, "framebuffer unsupported" );
                return 2;
            }

            lua_pushboolean( L, false );
            lua_pushcstring( L, "unknown exception" );
            return 2;
        }

        static LUA_DECLARE( bindColorTexture )
        {
            glTexture *tex;
            eBindOperation bindOp;
            unsigned int attachIndex;
            GLint mipmapLevel;

            LUA_ARGS_BEGIN;
            argStream.ReadClass( tex, LUACLASS_GLTEXTURE );
            argStream.ReadEnumString( bindOp, (eBindOperation)GL_DRAW_FRAMEBUFFER_EXT );
            argStream.ReadNumber( attachIndex, 0 );
            argStream.ReadNumber( mipmapLevel, 0 );
            LUA_ARGS_END;

            glFrameBufferEXT *fbo = (glFrameBufferEXT*)lua_getmethodtrans( L );
            glDriver *driver = fbo->m_driver;

            LUA_CHECK( driver == tex->m_driver && attachIndex < (unsigned int)driver->maxFBOColorAttachmentsEXT );

            // Check that we are not reattaching things
            LuaClass *prevAttach = fbo->colorAttachments[attachIndex];

            LUA_CHECK( prevAttach != tex );

            // Perform context operation.
            GLenum error;
            {
                glContextStack context( driver );
                glContextDescriptor *contextInfo = driver->contextInfo;

                glFrameBufferContext fbo_context( driver, fbo, bindOp );

                _glGetError();

                contextInfo->glFramebufferTexture2DEXT( bindOp, GL_COLOR_ATTACHMENT0_EXT + attachIndex, GL_TEXTURE_2D, tex->m_texIndex, mipmapLevel );

                error = _glGetError();
            }

            if ( error != 0 )
            {
                lua_pushboolean( L, false );
                lua_pushnumber( L, error );
                return 2;
            }

            // Exchange attachments
            if ( prevAttach )
                prevAttach->DecrementMethodStack();

            fbo->colorAttachments[attachIndex] = tex;

            tex->IncrementMethodStack();
            
            lua_pushboolean( L, true );
            return 1;
        }

        template <GLenum bindType>
        __forceinline LUA_DECLARE( single_attach_func )
        {
            glTexture *tex;
            eBindOperation bindOp;
            GLint mipmapLevel;

            LUA_ARGS_BEGIN;
            argStream.ReadClass( tex, LUACLASS_GLTEXTURE );
            argStream.ReadEnumString( bindOp, (eBindOperation)GL_DRAW_FRAMEBUFFER_EXT );
            argStream.ReadNumber( mipmapLevel, 0 );
            LUA_ARGS_END;

            glFrameBufferEXT *fbo = (glFrameBufferEXT*)lua_getmethodtrans( L );
            glDriver *driver = fbo->m_driver;

            LUA_CHECK( driver == tex->m_driver );

            // Check that we do not re-attach the same object
            LuaClass *prevAttach = NULL;

            if ( bindType == GL_DEPTH_ATTACHMENT_EXT )
                prevAttach = fbo->depthAttachment;
            else if ( bindType == GL_STENCIL_ATTACHMENT_EXT )
                prevAttach = fbo->stencilAttachment;

            LUA_CHECK( tex != prevAttach );

            // Perform context operation.
            GLenum error;
            {
                glContextStack context( driver );
                glContextDescriptor *contextInfo = driver->contextInfo;

                glFrameBufferContext fbo_context( driver, fbo, bindOp );

                _glGetError();

                contextInfo->glFramebufferTexture2DEXT( bindOp, bindType, GL_TEXTURE_2D, tex->m_texIndex, mipmapLevel );

                error = _glGetError();
            }

            if ( error != 0 )
            {
                lua_pushboolean( L, false );
                lua_pushnumber( L, error );
                return 2;
            }

            // Exchange attachments
            if ( prevAttach )
                prevAttach->DecrementMethodStack();

            if ( bindType == GL_DEPTH_ATTACHMENT_EXT )
                fbo->depthAttachment = tex;
            else if ( bindType == GL_STENCIL_ATTACHMENT_EXT )
                fbo->stencilAttachment = tex;

            tex->IncrementMethodStack();

            lua_pushboolean( L, true );
            return 1;
        }

        static LUA_DECLARE( bindDepthTexture )
        {
            return single_attach_func <GL_DEPTH_ATTACHMENT_EXT> ( L );
        }

        static LUA_DECLARE( bindStencilTexture )
        {
            return single_attach_func <GL_STENCIL_ATTACHMENT_EXT> ( L );
        }

        static LUA_DECLARE( runContext )
        {
            luaL_checktype( L, 2, LUA_TFUNCTION );
            
            lua_String bindOpDesc = lua_tostring( L, 1 );
            eBindOperation bindOp;

            if ( !bindOpDesc || !StringToEnum( bindOpDesc, bindOp ) )
                throw lua_exception( L, LUA_ERRRUN, "invalid context binding (either draw or read)" );

            glFrameBufferEXT *fbo = (glFrameBufferEXT*)lua_getmethodtrans( L );
            glDriver *driver = fbo->m_driver;
            glContextStack context( driver );
            glFrameBufferContext fboContext( driver, fbo, bindOp );
            
            lua_settop( L, 2 );
            lua_call( L, 0, 0 );
            return 0;
        }

        static const luaL_Reg framebufferEXT_interface[] =
        {
            LUA_METHOD( getColorAttachment ),
#if 0
            LUA_METHOD( unbindColor ),
            LUA_METHOD( unbindDepth ),
            LUA_METHOD( unbindStencil ),
#endif
            LUA_METHOD( checkStatus ),
            LUA_METHOD( bindColorTexture ),
            LUA_METHOD( bindDepthTexture ),
            LUA_METHOD( bindStencilTexture ),
            LUA_METHOD( runContext ),
            { NULL, NULL }
        };

        static LUA_DECLARE( constructor )
        {
            glFrameBufferEXT *fbo = (glFrameBufferEXT*)lua_touserdata( L, lua_upvalueindex( 1 ) );

            ILuaClass *j = lua_refclass( L, LUA_ENVIRONINDEX );

            j->RegisterInterfaceTrans( L, framebufferEXT_interface, 0, LUACLASS_FRAMEBUFFER );
            return 0;
        }
    };

    namespace core
    {
        static LUA_DECLARE( getColorAttachment )
        {
            unsigned int attachIndex;

            LUA_ARGS_BEGIN;
            argStream.ReadNumber( attachIndex, 0 );
            LUA_ARGS_END;

            glFrameBuffer *fbo = (glFrameBuffer*)lua_getmethodtrans( L );

            LUA_CHECK( attachIndex < (unsigned int)fbo->m_driver->maxFBOColorAttachments );

            LuaClass *colorAttach = fbo->colorAttachments[attachIndex];

            LUA_CHECK( colorAttach );

            colorAttach->PushStack( L );
            return 1;
        }

        __forceinline void DetachFBOObject( glDriver *driver, glFrameBuffer *fbo, GLenum slot, LuaClass *obj )
        {
            glContextStack context( driver );
            glFrameBufferContext( driver, fbo, GL_DRAW_FRAMEBUFFER );

            // Detach anything that was attached before.
            if ( obj->IsTransmit( LUACLASS_GLTEXTURE ) )
                driver->contextInfo->glFramebufferTexture2D( GL_DRAW_FRAMEBUFFER, slot, GL_TEXTURE_2D, 0, 0 );
            else
                driver->contextInfo->glFramebufferRenderbuffer( GL_DRAW_FRAMEBUFFER, slot, GL_RENDERBUFFER, 0 );
        }

        static LUA_DECLARE( unbindColor )
        {
            unsigned int attachIndex;

            LUA_ARGS_BEGIN;
            argStream.ReadNumber( attachIndex );
            LUA_ARGS_END;

            glFrameBuffer *fbo = (glFrameBuffer*)lua_getmethodtrans( L );
            glDriver *driver = fbo->m_driver;

            LUA_CHECK( attachIndex < (unsigned int)driver->maxFBOColorAttachments );

            LuaClass *colorAttach = fbo->colorAttachments[attachIndex];

            LUA_CHECK( colorAttach );

            // Run context operation.
            DetachFBOObject( driver, fbo, GL_COLOR_ATTACHMENT0 + attachIndex, colorAttach );

            fbo->colorAttachments[attachIndex] = NULL;
            colorAttach->DecrementMethodStack();

            lua_pushboolean( L, true );
            return 1;
        }

        template <GLenum attachType>
        __forceinline LUA_DECLARE( single_detach_perform )
        {
            glFrameBuffer *fbo = (glFrameBuffer*)lua_getmethodtrans( L );
            glDriver *driver = fbo->m_driver;

            LuaClass *attachObj = NULL;

            if ( attachType == GL_DEPTH_ATTACHMENT )
                attachObj = fbo->depthAttachment;
            else if ( attachType == GL_STENCIL_ATTACHMENT )
                attachObj = fbo->stencilAttachment;

            LUA_CHECK( attachObj );

            // Run context operation.
            DetachFBOObject( driver, fbo, attachType, attachObj );

            if ( attachType == GL_DEPTH_ATTACHMENT )
                fbo->depthAttachment = NULL;
            else if ( attachType == GL_STENCIL_ATTACHMENT )
                fbo->stencilAttachment = NULL;

            attachObj->DecrementMethodStack();

            lua_pushboolean( L, true );
            return 1;
        }

        static LUA_DECLARE( unbindDepth )
        {
            return single_detach_perform <GL_DEPTH_ATTACHMENT> ( L );
        }

        static LUA_DECLARE( unbindStencil )
        {
            return single_detach_perform <GL_STENCIL_ATTACHMENT> ( L );
        }

        enum eBindOperation : GLenum
        { };

        DECLARE_ENUM( eBindOperation );

        IMPLEMENT_ENUM_BEGIN( eBindOperation )
            ADD_ENUM( GL_DRAW_FRAMEBUFFER, "draw" )
            ADD_ENUM( GL_READ_FRAMEBUFFER, "read" )
        IMPLEMENT_ENUM_END( "eBindOperation" )

        static LUA_DECLARE( checkStatus )
        {
            eBindOperation bindOp;

            LUA_ARGS_BEGIN;
            argStream.ReadEnumString( bindOp );
            LUA_ARGS_END;

            glFrameBuffer *fbo = (glFrameBuffer*)lua_getmethodtrans( L );
            glDriver *driver = fbo->m_driver;

            glContextStack context( driver );
            glFrameBufferContext fboContext( driver, fbo, bindOp );

            GLenum status = driver->contextInfo->glCheckFramebufferStatus( bindOp );

            switch( status )
            {
            case GL_FRAMEBUFFER_COMPLETE:
                lua_pushboolean( L, true );
                return 1;
            case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
                lua_pushboolean( L, false );
                lua_pushcstring( L, "incomplete attachment" );
                return 2;
            case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
                lua_pushboolean( L, false );
                lua_pushcstring( L, "incomplete missing attachment" );
                return 2;
            case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
                lua_pushboolean( L, false );
                lua_pushcstring( L, "incomplete draw buffer" );
                return 2;
            case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
                lua_pushboolean( L, false );
                lua_pushcstring( L, "incomplete read buffer" );
                return 2;
            case GL_FRAMEBUFFER_UNSUPPORTED:
                lua_pushboolean( L, false );
                lua_pushcstring( L, "framebuffer unsupported" );
                return 2;
            }

            lua_pushboolean( L, false );
            lua_pushcstring( L, "unknown exception" );
            return 2;
        }

        static LUA_DECLARE( bindColorTexture )
        {
            glTexture *tex;
            eBindOperation bindOp;
            unsigned int attachIndex;
            GLint mipmapLevel;

            LUA_ARGS_BEGIN;
            argStream.ReadClass( tex, LUACLASS_GLTEXTURE );
            argStream.ReadEnumString( bindOp, (eBindOperation)GL_DRAW_FRAMEBUFFER );
            argStream.ReadNumber( attachIndex, 0 );
            argStream.ReadNumber( mipmapLevel, 0 );
            LUA_ARGS_END;

            glFrameBuffer *fbo = (glFrameBuffer*)lua_getmethodtrans( L );
            glDriver *driver = fbo->m_driver;

            LUA_CHECK( driver == tex->m_driver && attachIndex < (unsigned int)driver->maxFBOColorAttachments );

            // Make sure we attach a new object (re-attachment prevented)
            LuaClass *prevAttach = fbo->colorAttachments[attachIndex];

            LUA_CHECK( prevAttach != tex );

            // Perform context operation.
            GLenum error;
            {
                glContextStack context( driver );
                glContextDescriptor *contextInfo = driver->contextInfo;

                glFrameBufferContext fbo_context( driver, fbo, bindOp );

                _glGetError();

                contextInfo->glFramebufferTexture2D( bindOp, GL_COLOR_ATTACHMENT0 + attachIndex, GL_TEXTURE_2D, tex->m_texIndex, mipmapLevel );
                
                error = _glGetError();
            }

            if ( error != 0 )
            {
                lua_pushboolean( L, false );
                lua_pushnumber( L, error );
                return 2;
            }

            // Exchange the attached objects.
            if ( prevAttach )
                prevAttach->DecrementMethodStack();

            fbo->colorAttachments[attachIndex] = tex;
           
            tex->IncrementMethodStack();
            
            lua_pushboolean( L, true );
            return 1;
        }

        template <GLenum bindType>
        __forceinline LUA_DECLARE( single_attach_func )
        {
            glTexture *tex;
            eBindOperation bindOp;
            GLint mipmapLevel;

            LUA_ARGS_BEGIN;
            argStream.ReadClass( tex, LUACLASS_GLTEXTURE );
            argStream.ReadEnumString( bindOp, (eBindOperation)GL_DRAW_FRAMEBUFFER );
            argStream.ReadNumber( mipmapLevel, 0 );
            LUA_ARGS_END;

            glFrameBuffer *fbo = (glFrameBuffer*)lua_getmethodtrans( L );
            glDriver *driver = fbo->m_driver;

            LUA_CHECK( driver == tex->m_driver );

            // Make sure we do not re-attach objects
            LuaClass *prevAttach = NULL;
            LuaClass *prevAttach2 = NULL;

            if ( bindType == GL_DEPTH_ATTACHMENT )
            {
                prevAttach = fbo->depthAttachment;

                LUA_CHECK( prevAttach != tex );
            }
            else if ( bindType == GL_STENCIL_ATTACHMENT )
            {
                prevAttach = fbo->stencilAttachment;

                LUA_CHECK( prevAttach != tex );
            }
            else if ( bindType == GL_DEPTH_STENCIL_ATTACHMENT )
            {
                prevAttach = fbo->depthAttachment;
                prevAttach2 = fbo->stencilAttachment;

                LUA_CHECK( prevAttach != tex || prevAttach2 != tex );
            }

            // Perform context operation.
            GLenum error;
            {
                glContextStack context( driver );
                glContextDescriptor *contextInfo = driver->contextInfo;

                glFrameBufferContext fbo_context( driver, fbo, bindOp );

                _glGetError();

                contextInfo->glFramebufferTexture2D( bindOp, bindType, GL_TEXTURE_2D, tex->m_texIndex, mipmapLevel );

                error = _glGetError();
            }

            if ( error != 0 )
            {
                lua_pushboolean( L, false );
                lua_pushnumber( L, error );
                return 2;
            }

            // Exchange the attachments
            if ( bindType == GL_DEPTH_ATTACHMENT )
                fbo->depthAttachment = tex;
            else if ( bindType == GL_STENCIL_ATTACHMENT )
                fbo->stencilAttachment = tex;
            else if ( bindType == GL_DEPTH_STENCIL_ATTACHMENT )
            {
                fbo->depthAttachment = tex;
                fbo->stencilAttachment = tex;

                tex->IncrementMethodStack();
            }

            tex->IncrementMethodStack();

            // We do not require the previous attachment anymore.
            if ( prevAttach )
                prevAttach->DecrementMethodStack();

            if ( prevAttach2 )
                prevAttach2->DecrementMethodStack();

            lua_pushboolean( L, true );
            return 1;
        }

        static LUA_DECLARE( bindDepthTexture )
        {
            return single_attach_func <GL_DEPTH_ATTACHMENT> ( L );
        }

        static LUA_DECLARE( bindStencilTexture )
        {
            return single_attach_func <GL_STENCIL_ATTACHMENT> ( L );
        }

        static LUA_DECLARE( bindDepthStencilTexture )
        {
            return single_attach_func <GL_DEPTH_STENCIL_ATTACHMENT> ( L );
        }

        static LUA_DECLARE( runContext )
        {
            luaL_checktype( L, 2, LUA_TFUNCTION );
            
            lua_String bindOpDesc = lua_tostring( L, 1 );
            eBindOperation bindOp;

            if ( !bindOpDesc || !StringToEnum( bindOpDesc, bindOp ) )
                throw lua_exception( L, LUA_ERRRUN, "invalid context binding (either draw or read)" );

            glFrameBuffer *fbo = (glFrameBuffer*)lua_getmethodtrans( L );
            glDriver *driver = fbo->m_driver;
            glContextStack context( driver );
            glFrameBufferContext fboContext( driver, fbo, bindOp );
            
            lua_settop( L, 2 );
            lua_call( L, 0, 0 );
            return 0;
        }

        static const luaL_Reg framebuffer_interface[] =
        {
            LUA_METHOD( getColorAttachment ),
#if 0
            LUA_METHOD( unbindColor ),
            LUA_METHOD( unbindDepth ),
            LUA_METHOD( unbindStencil ),
#endif
            LUA_METHOD( checkStatus ),
            LUA_METHOD( bindColorTexture ),
            LUA_METHOD( bindDepthTexture ),
            LUA_METHOD( bindStencilTexture ),
            LUA_METHOD( bindDepthStencilTexture ),
            LUA_METHOD( runContext ),
            { NULL, NULL }
        };

        static LUA_DECLARE( constructor )
        {
            glFrameBuffer *fbo = (glFrameBuffer*)lua_touserdata( L, lua_upvalueindex( 1 ) );

            ILuaClass *j = lua_refclass( L, LUA_ENVIRONINDEX );

            j->RegisterInterfaceTrans( L, framebuffer_interface, 0, LUACLASS_FRAMEBUFFER );
            return 0;
        }
    };
};

// OpenGL framebuffer shared constructor
glFrameBufferBase::glFrameBufferBase( lua_State *L, glDriver *driver, eFrameBufferType bufType ) : LuaInstance( L ), m_driver( driver ), m_type( bufType )
{
    Extend( L, framebuffer::framebufferbase_constructor );

    colorAttachments = NULL;
    depthAttachment = NULL;
    stencilAttachment = NULL;
}

glFrameBufferBase::~glFrameBufferBase( void )
{
    if ( depthAttachment )
    {
        depthAttachment->DecrementMethodStack();
        depthAttachment = NULL;
    }

    if ( stencilAttachment )
    {
        stencilAttachment->DecrementMethodStack();
        stencilAttachment = NULL;
    }

    if ( colorAttachments )
        delete colorAttachments;
}

// OpenGL GL_EXT_framebuffer_object extension
glFrameBufferEXT::glFrameBufferEXT( lua_State *L, glDriver *driver, GLuint bufIndex ) : glFrameBufferBase( L, driver, FRAMEBUFFER_EXT ), m_bufIndex( bufIndex )
{
    Extend( L, framebuffer::ext::constructor );

    // Set up the attachments array.
    colorAttachments = new LuaClass*[driver->maxFBOColorAttachmentsEXT];
    memset( colorAttachments, 0, sizeof(LuaClass*) * driver->maxFBOColorAttachmentsEXT );
}

glFrameBufferEXT::~glFrameBufferEXT( void )
{
    // Delete the framebuffer in a quick context
    {
        glContextStack context( m_driver );

        m_driver->contextInfo->glDeleteFramebuffersEXT( 1, &m_bufIndex );
    }

    // Unlink all color attachments.
    for ( unsigned int n = 0; n < (unsigned int)m_driver->maxFBOColorAttachmentsEXT; n++ )
    {
        if ( LuaClass *obj = colorAttachments[n] )
        {
            obj->DecrementMethodStack();
            colorAttachments[n] = NULL;
        }
    }
}

// OpenGL 3.0 framebuffer core module
glFrameBuffer::glFrameBuffer( lua_State *L, glDriver *driver, GLuint bufIndex ) : glFrameBufferBase( L, driver, FRAMEBUFFER_CORE ), m_bufIndex( bufIndex )
{
    Extend( L, framebuffer::core::constructor );

    // Set up the attachments array.
    colorAttachments = new LuaClass*[driver->maxFBOColorAttachments];
    memset( colorAttachments, 0, sizeof(LuaClass*) * driver->maxFBOColorAttachments );
}

glFrameBuffer::~glFrameBuffer( void )
{
    // Delete the framebuffer in a quick context
    {
        glContextStack context( m_driver );

        m_driver->contextInfo->glDeleteFramebuffers( 1, &m_bufIndex );
    }

    // Unlink all color attachments.
    for ( unsigned int n = 0; n < (unsigned int)m_driver->maxFBOColorAttachments; n++ )
    {
        if ( LuaClass *obj = colorAttachments[n] )
        {
            obj->DecrementMethodStack();
            colorAttachments[n] = NULL;
        }
    }
}

static LUA_DECLARE( createFrameBuffer )
{
    glDriver *driver = (glDriver*)lua_getmethodtrans( L );

    if ( !driver->supports3_0 && !driver->supports_FBO_EXT )
    {
        lua_pushboolean( L, false );
        lua_pushcstring( L, "driver does not support framebuffers" );
        return 2;
    }
  
    glContextStack context( driver );
    glContextDescriptor *contextInfo = driver->contextInfo;

    glFrameBufferBase *fbo;

    if ( driver->supports3_0 )
    {
        GLuint bufIndex;
        contextInfo->glGenFramebuffers( 1, &bufIndex );

        fbo = new glFrameBuffer( L, driver, bufIndex );
    }
    else if ( driver->supports_FBO_EXT )
    {
        GLuint bufIndex;
        contextInfo->glGenFramebuffersEXT( 1, &bufIndex );

        fbo = new glFrameBufferEXT( L, driver, bufIndex );
    }

    fbo->SetParent( driver );
    fbo->PushStack( L );
    fbo->Finalize( L );
    return 1;
}

const luaL_Reg fbo_driver_interface[] =
{
    LUA_METHOD( createRenderBuffer ),
    LUA_METHOD( createFrameBuffer ),
    { NULL, NULL }
};