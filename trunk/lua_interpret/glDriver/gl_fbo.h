/*****************************************************************************
*
*  PROJECT:     Lua Interpreter
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        glDriver/gl_fbo.h
*  PURPOSE:     OpenGL driver render buffer extension
*  DEVELOPERS:  Martin Turski <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#ifndef _OPENGL_RENDERBUFFER_
#define _OPENGL_RENDERBUFFER_

#define LUACLASS_RENDERBUFFER       52
#define LUACLASS_FRAMEBUFFER        53

// OpenGL renderbuffer objects
enum eRenderBufferType : unsigned int
{
    RENDERBUFFER_EXT,
    RENDERBUFFER_CORE
};

struct glRenderBufferBase abstract : public glClass
{
    glRenderBufferBase( lua_State *L, glDriver *driver, eRenderBufferType bufType, unsigned int width, unsigned int height );
    ~glRenderBufferBase( void );

    unsigned int m_width, m_height;
    eRenderBufferType m_type;
};

struct glRenderBufferEXT : public glRenderBufferBase
{
    glRenderBufferEXT( lua_State *L, glDriver *driver, GLenum bufIndex, unsigned int width, unsigned int height );
    ~glRenderBufferEXT( void );

    GLenum m_bufIndex;
};

struct glRenderBuffer : public glRenderBufferBase
{
    glRenderBuffer( lua_State *L, glDriver *driver, GLenum bufIndex, unsigned int width, unsigned int height );
    ~glRenderBuffer( void );

    GLenum m_bufIndex;
};

// OpenGL framebuffer objects
enum eFrameBufferType : unsigned int
{
    FRAMEBUFFER_EXT,
    FRAMEBUFFER_CORE
};

struct glFrameBufferBase abstract : public glClass
{
    glFrameBufferBase( lua_State *L, glDriver *driver, eFrameBufferType bufType );
    ~glFrameBufferBase( void );

    eFrameBufferType m_type;

    LuaClass **colorAttachments;     // array of color attached objects
    LuaClass *depthAttachment;      // depth buffer attachment
    LuaClass *stencilAttachment;    // stencil buffer attachment
};

struct glFrameBufferEXT : public glFrameBufferBase
{
    glFrameBufferEXT( lua_State *L, glDriver *driver, GLuint bufIndex );
    ~glFrameBufferEXT( void );

    GLuint m_bufIndex;
};

struct glFrameBuffer : public glFrameBufferBase
{
    glFrameBuffer( lua_State *L, glDriver *driver, GLuint bufIndex );
    ~glFrameBuffer( void );

    GLuint m_bufIndex;
};

extern const luaL_Reg fbo_driver_interface[];

// framebuffer context stack - draw
struct glFrameBufferDrawContext
{
    __forceinline glFrameBufferDrawContext( glDriver *driver, glFrameBufferBase *fbo )
    {
        if ( driver->fboDrawStack != fbo )
        {
            prev = driver->fboDrawStack;
            driver->fboDrawStack = fbo;

            if ( fbo->m_type == FRAMEBUFFER_EXT )
                driver->contextInfo->glBindFramebufferEXT( GL_DRAW_FRAMEBUFFER_EXT, ((glFrameBufferEXT*)fbo)->m_bufIndex );
            else
                driver->contextInfo->glBindFramebuffer( GL_DRAW_FRAMEBUFFER, ((glFrameBuffer*)fbo)->m_bufIndex );

            this->driver = driver;

            applied = true;
        }
        else
            applied = false;
    }

    __forceinline ~glFrameBufferDrawContext( void )
    {
        if ( applied )
        {
            if ( prev )
            {
                glFrameBufferBase *fbo = prev;

                if ( fbo->m_type == FRAMEBUFFER_EXT )
                    driver->contextInfo->glBindFramebufferEXT( GL_DRAW_FRAMEBUFFER_EXT, ((glFrameBufferEXT*)fbo)->m_bufIndex );
                else
                    driver->contextInfo->glBindFramebuffer( GL_DRAW_FRAMEBUFFER, ((glFrameBuffer*)fbo)->m_bufIndex );
            }
            else
            {
                glFrameBufferBase *fbo = driver->fboDrawStack;

                if ( fbo->m_type == FRAMEBUFFER_EXT )
                    driver->contextInfo->glBindFramebufferEXT( GL_DRAW_FRAMEBUFFER_EXT, 0 );
                else
                    driver->contextInfo->glBindFramebuffer( GL_DRAW_FRAMEBUFFER, 0 );
            }

            driver->fboDrawStack = prev;
        }
    }

    bool applied;
    glDriver *driver;
    glFrameBufferBase *prev;
};

// framebuffer context stack - read
struct glFrameBufferReadContext
{
    __forceinline glFrameBufferReadContext( glDriver *driver, glFrameBufferBase *fbo )
    {
        if ( driver->fboReadStack != fbo )
        {
            prev = driver->fboReadStack;
            driver->fboReadStack = fbo;

            if ( fbo->m_type == FRAMEBUFFER_EXT )
                driver->contextInfo->glBindFramebufferEXT( GL_READ_FRAMEBUFFER_EXT, ((glFrameBufferEXT*)fbo)->m_bufIndex );
            else
                driver->contextInfo->glBindFramebuffer( GL_READ_FRAMEBUFFER, ((glFrameBuffer*)fbo)->m_bufIndex );

            this->driver = driver;

            applied = true;
        }
        else
            applied = false;
    }

    __forceinline ~glFrameBufferReadContext( void )
    {
        if ( applied )
        {
            if ( prev )
            {
                glFrameBufferBase *fbo = prev;

                if ( fbo->m_type == FRAMEBUFFER_EXT )
                    driver->contextInfo->glBindFramebufferEXT( GL_READ_FRAMEBUFFER_EXT, ((glFrameBufferEXT*)fbo)->m_bufIndex );
                else
                    driver->contextInfo->glBindFramebuffer( GL_READ_FRAMEBUFFER, ((glFrameBuffer*)fbo)->m_bufIndex );
            }
            else
            {
                glFrameBufferBase *fbo = driver->fboReadStack;

                if ( fbo->m_type == FRAMEBUFFER_EXT )
                    driver->contextInfo->glBindFramebufferEXT( GL_READ_FRAMEBUFFER_EXT, 0 );
                else
                    driver->contextInfo->glBindFramebuffer( GL_READ_FRAMEBUFFER, 0 );
            }

            driver->fboReadStack = prev;
        }
    }

    bool applied;
    glDriver *driver;
    glFrameBufferBase *prev;
};

// General framebuffer context stack item
struct glFrameBufferContext
{
    __forceinline glFrameBufferContext( glDriver *driver, glFrameBufferBase *fbo, GLenum bindOp ) : drawContext( NULL ), readContext( NULL )
    {
        if ( fbo->m_type == FRAMEBUFFER_EXT )
        {
            switch( bindOp )
            {
            case GL_DRAW_FRAMEBUFFER_EXT: drawContext = new glFrameBufferDrawContext( driver, fbo ); break;
            case GL_READ_FRAMEBUFFER_EXT: readContext = new glFrameBufferReadContext( driver, fbo ); break;
            default:
                drawContext = new glFrameBufferDrawContext( driver, fbo );
                readContext = new glFrameBufferReadContext( driver, fbo );
                break;
            }
        }
        else
        {
            switch( bindOp )
            {
            case GL_DRAW_FRAMEBUFFER: drawContext = new glFrameBufferDrawContext( driver, fbo ); break;
            case GL_READ_FRAMEBUFFER: readContext = new glFrameBufferReadContext( driver, fbo ); break;
            default:
                drawContext = new glFrameBufferDrawContext( driver, fbo );
                readContext = new glFrameBufferReadContext( driver, fbo );
                break;
            }
        }
    }

    __forceinline ~glFrameBufferContext( void )
    {
        if ( drawContext )
            delete drawContext;

        if ( readContext )
            delete readContext;
    }

    glFrameBufferDrawContext *drawContext;
    glFrameBufferReadContext *readContext;
};

#endif //_OPENGL_RENDERBUFFFER_