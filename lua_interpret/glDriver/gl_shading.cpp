/*****************************************************************************
*
*  PROJECT:     Lua Interpreter
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        glDriver/gl_shading.cpp
*  PURPOSE:     OpenGL driver programmable shading implementation
*  DEVELOPERS:  Martin Turski <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#include <StdInc.h>
#include "gl_internal.h"

namespace gl_objectARB
{
    static LUA_DECLARE( get )
    {
        size_t paramLen;
        lua_String paramName = lua_tolstring( L, 1, &paramLen );

        LUA_CHECK( paramName );

        glDriver *driver = (glDriver*)lua_getmethodtrans( L );

        const GLparaminfo *info = gl_objectARB::GetParamByName( driver, paramName, paramLen );

        LUA_CHECK( info );

        glObjectARB *obj = (glObjectARB*)lua_getmethodtrans( L );

        glContextStack contextStack( driver );
        glContextDescriptor *context = driver->contextInfo;

        // Arguments.
        union
        {
            GLint integers[16];
            GLfloat floats[16];
        };

        switch( info->type )
        {
        case GLPARAM_BOOLEAN:
            context->glGetObjectParameterivARB( obj->m_handle, info->builtin, integers );

            for ( unsigned int n = 0; n < info->numArgs; n++ )
                lua_pushboolean( L, integers[n] != 0 );

            break;
        case GLPARAM_FLOAT:
            context->glGetObjectParameterfvARB( obj->m_handle, info->builtin, floats );

            for ( unsigned int n = 0; n < info->numArgs; n++ )
                lua_pushnumber( L, floats[n] );

            break;
        default:
            context->glGetObjectParameterivARB( obj->m_handle, info->builtin, integers );

            for ( unsigned int n = 0; n < info->numArgs; n++ )
                lua_pushinteger( L, integers[n] );

            break;
        }

        return info->numArgs;
    }

    static LUA_DECLARE( getInfoLog )
    {
        glObjectARB *obj = (glObjectARB*)lua_getmethodtrans( L );

        glDriver *driver = obj->m_driver;
        glContextStack contextStack( driver );

        glContextDescriptor *context = driver->contextInfo;
    
        // First get the length of the info log.
        // If it is zero, we should return false.
        GLint infoLogLength = 0;
        context->glGetObjectParameterivARB( obj->m_handle, GL_OBJECT_INFO_LOG_LENGTH_ARB, &infoLogLength );

        LUA_CHECK( infoLogLength != 0 );

        // Clear error state.
        _glGetError();

        // Attempt to get the info log of our object.
        GLcharARB *infoLog = new GLcharARB[ infoLogLength ];

        context->glGetInfoLogARB( obj->m_handle, infoLogLength, NULL, infoLog );

        // If an error occured while trying to get the info log,
        // free the memory and terminate.
        if ( _glGetError() != 0 )
        {
            delete infoLog;

            lua_pushboolean( L, false );
            return 1;
        }

        // Return the info log to the LuaVM.
        // We assume it is a C-string.
        // The OpenGL implementation dictates that it is automatically zero-terminated.
        lua_pushlstring( L, (const char*)infoLog, infoLogLength );

        delete infoLog;

        return 1;
    }

    static const luaL_Reg _interface[] =
    {
        LUA_METHOD( get ),
        LUA_METHOD( getInfoLog ),
        { NULL, NULL }
    };

    static LUA_DECLARE( constructor )
    {
        glObjectARB *obj = (glObjectARB*)lua_touserdata( L, lua_upvalueindex( 1 ) );

        ILuaClass *j = lua_refclass( L, LUA_ENVIRONINDEX );
        j->SetTransmit( LUACLASS_GLSLOBJECT_ARB, obj );

        j->RegisterInterfaceTrans( L, _interface, 0, LUACLASS_GLSLOBJECT_ARB );

        lua_pushcstring( L, "glslobject" );
        lua_setfield( L, LUA_ENVIRONINDEX, "__type" );
        return 0;
    }
};

glObjectARB::glObjectARB( lua_State *L, glDriver *driver, GLhandleARB handle ) : LuaInstance( L )
{
    // Extend to basic object functionality
    Extend( L, gl_objectARB::constructor );

    // Store the basic parameters.
    m_driver = driver;
    m_handle = handle;
}

glObjectARB::~glObjectARB( void )
{
    // Destroy it from the driver.
    m_driver->contextInfo->glDeleteObjectARB( m_handle );
}

namespace gl_shaderARB
{
    static const luaL_Reg _interface[] =
    {
        { NULL, NULL }
    };

    static LUA_DECLARE( constructor )
    {
        glShaderARB *shader = (glShaderARB*)lua_touserdata( L, lua_upvalueindex( 1 ) );

        ILuaClass *j = lua_refclass( L, LUA_ENVIRONINDEX );
        j->SetTransmit( LUACLASS_GLSLSHADER_ARB, shader );

        j->RegisterInterfaceTrans( L, _interface, 0, LUACLASS_GLSLSHADER_ARB );

        lua_pushcstring( L, "glslshader" );
        lua_setfield( L, LUA_ENVIRONINDEX, "__type" );
        return 0;
    }
}

glShaderARB::glShaderARB( lua_State *L, glDriver *driver, GLhandleARB handle, GLenum type ) : glObjectARB( L, driver, handle )
{
    // Extend the class
    Extend( L, gl_shaderARB::constructor );

    m_type = type;
}

glShaderARB::~glShaderARB( void )
{
    // Destruction is handled by glObjectARB.
    return;
}

enum GLeShaderType : GLenum
{};

DECLARE_ENUM( GLeShaderType );

IMPLEMENT_ENUM_BEGIN( GLeShaderType )
    ADD_ENUM( GL_VERTEX_SHADER_ARB, "vertex" )
    ADD_ENUM( GL_FRAGMENT_SHADER_ARB, "fragment" )
IMPLEMENT_ENUM_END( "GLeShaderType" )

static LUA_DECLARE( compileShader )
{
    GLeShaderType shaderType;

    LUA_ARGS_BEGIN;
    argStream.ReadEnumString( shaderType );
    LUA_ARGS_END;

    size_t len;
    lua_String progString;

    LUA_CHECK( progString = lua_tolstring( L, 2, &len ) );

    glDriver *driver = (glDriver*)lua_getmethodtrans( L );

    LUA_CHECK( driver->supports_ARB_shader_objects );

    glContextStack contextStack( driver );
    glContextDescriptor *context = driver->contextInfo;

    GLhandleARB shaderHandle = context->glCreateShaderObjectARB( shaderType );

    LUA_CHECK( shaderHandle );

    const GLcharARB *srcString = (const GLcharARB*)progString;
    GLint srcLen = len;

    context->glShaderSourceARB( shaderHandle, 1, &srcString, &srcLen );

    GLint compileStatus;
    context->glGetObjectParameterivARB( shaderHandle, GL_OBJECT_COMPILE_STATUS_ARB, &compileStatus );

    if ( compileStatus == 0 )
    {
        context->glDeleteObjectARB( shaderHandle );

        lua_pushboolean( L, false );
        return 1;
    }

    glShaderARB *shader = new glShaderARB( L, driver, shaderHandle, shaderType );
    shader->SetParent( driver );
    shader->PushStack( L );
    shader->Finalize( L );
    return 1;
}

namespace gl_programARB
{
    static LUA_DECLARE( attachShader )
    {
        glShaderARB *shader;

        LUA_ARGS_BEGIN;
        argStream.ReadClass( shader, LUACLASS_GLSLSHADER_ARB );
        LUA_ARGS_END;

        glProgramARB *program = (glProgramARB*)lua_getmethodtrans( L );

        glDriver *driver = program->m_driver;

        // Check that the OpenGL drivers match.
        LUA_CHECK( shader->m_driver == program->m_driver );

        glContextStack contextStack( driver );

        // Clear the error info.
        _glGetError();

        driver->contextInfo->glAttachObjectARB( program->m_handle, shader->m_handle );

        // Only proceed if the attachment succeeded.
        // This avoids double attachment.
        LUA_CHECK( _glGetError() == 0 );

        // Add the shader to the program's shader list and reference it
        program->m_attachedShaders.push_back( shader );

        // We have to return true as we used LUA_CHECK in this Lua routine.
        lua_pushboolean( L, true );
        return 1;
    }

    static LUA_DECLARE( detachShader )
    {
        glShaderARB *shader;

        LUA_ARGS_BEGIN;
        argStream.ReadClass( shader, LUACLASS_GLSLSHADER_ARB );
        LUA_ARGS_END;

        glProgramARB *program = (glProgramARB*)lua_getmethodtrans( L );

        glDriver *driver = program->m_driver;

        // Check that the OpenGL drivers match.
        LUA_CHECK( shader->m_driver == program->m_driver );

        glContextStack contextStack( driver );

        // Clear the error info.
        _glGetError();

        driver->contextInfo->glDetachObjectARB( program->m_handle, shader->m_handle );

        // Only proceed if the detachment succeeded.
        // This ensures that the object was already attached.
        LUA_CHECK( _glGetError() == 0 );

        // Remove the shader from the list.
        // It must be inside of the list.
        program->m_attachedShaders.remove( shader );

        // We have to return true as we used LUA_CHECK in this Lua routine.
        lua_pushboolean( L, true );
        return 1;
    }

    static LUA_DECLARE( getAttachedShaders )
    {
        glProgramARB *program = (glProgramARB*)lua_getmethodtrans( L );

        // Return a table of all shaders attached to this program object.
        unsigned int count = program->m_numAttachedShaders;

        lua_createtable( L, count, 0 );

        unsigned int n = 1;

        for ( glProgramARB::shaderList_t::const_iterator iter = program->m_attachedShaders.begin();
              iter != program->m_attachedShaders.end(); iter++, n++ )
        {
            (*iter)->PushStack( L );
            lua_rawseti( L, -2, n );
        }

        return 1;
    }

    static LUA_DECLARE( get )
    {
        size_t paramLen;
        lua_String paramName = lua_tolstring( L, 1, &paramLen );

        LUA_CHECK( paramName );

        glDriver *driver = (glDriver*)lua_getmethodtrans( L );

        const GLparaminfo *info = gl_programARB::GetParamByName( driver, paramName, paramLen );

        if ( !info )
        {
            lua_pushmethodsuper( L );
            lua_pushvalue( L, 1 );
            lua_call( L, 1, 1 );
            return 1;
        }

        glProgramARB *program = (glProgramARB*)lua_getmethodtrans( L );

        glContextStack contextStack( driver );
        glContextDescriptor *context = driver->contextInfo;

        // Arguments.
        GLint integers[16];

        switch( info->type )
        {
        case GLPARAM_BOOLEAN:
            context->glGetProgramiv( program->m_handle, info->builtin, integers );

            for ( unsigned int n = 0; n < info->numArgs; n++ )
                lua_pushboolean( L, integers[n] != 0 );

            break;
        default:
            // Default type. There is no float.
            context->glGetProgramiv( program->m_handle, info->builtin, integers );

            for ( unsigned int n = 0; n < info->numArgs; n++ )
                lua_pushinteger( L, integers[n] );

            break;
        }

        return info->numArgs;
    }

    static LUA_DECLARE( bindAttribLocation )
    {
        GLuint index;
        lua_String attribName;

        LUA_ARGS_BEGIN;
        argStream.ReadNumber( index );
        argStream.ReadString( attribName );
        LUA_ARGS_END;

        glProgramARB *program = (glProgramARB*)lua_getmethodtrans( L );

        glDriver *driver = program->m_driver;
        glContextStack contextStack( driver );
        glContextDescriptor *context = driver->contextInfo;

        // Clear OpenGL error info.
        _glGetError();

        context->glBindAttribLocationARB( program->m_handle, index, (const GLcharARB*)attribName );

        // Return whether it succeeded.
        lua_pushboolean( L, _glGetError() == 0 );
        return 1;
    }
    
    static LUA_DECLARE( validate )
    {
        glProgramARB *program = (glProgramARB*)lua_getmethodtrans( L );

        glDriver *driver = program->m_driver;
        glContextStack contextStack( driver );

        glContextDescriptor *context = driver->contextInfo;

        context->glValidateProgramARB( program->m_handle );

        // Return the validation status.
        GLint valStatus;
        context->glGetObjectParameterivARB( program->m_handle, GL_OBJECT_VALIDATE_STATUS_ARB, &valStatus );

        lua_pushboolean( L, valStatus != 0 );
        return 1;
    }

    static LUA_DECLARE( link )
    {
        glProgramARB *program = (glProgramARB*)lua_getmethodtrans( L );
    
        glDriver *driver = program->m_driver;
        glContextStack contextStack( driver );
        
        glContextDescriptor *context = driver->contextInfo;

        context->glLinkProgramARB( program->m_handle );

        // Return whether linking was successful.
        GLint linkStatus;
        context->glGetObjectParameterivARB( program->m_handle, GL_OBJECT_LINK_STATUS_ARB, &linkStatus );

        lua_pushboolean( L, linkStatus != 0 );
        return 1;
    }

    static const luaL_Reg _interface[] =
    {
        LUA_METHOD( attachShader ),
        LUA_METHOD( detachShader ),
        LUA_METHOD( getAttachedShaders ),
        LUA_METHOD( get ),
        LUA_METHOD( bindAttribLocation ),
        LUA_METHOD( validate ),
        LUA_METHOD( link ),
        { NULL, NULL }
    };

    static LUA_DECLARE( constructor )
    {
        glProgramARB *program = (glProgramARB*)lua_touserdata( L, lua_upvalueindex( 1 ) );

        ILuaClass *j = lua_refclass( L, LUA_ENVIRONINDEX );
        j->SetTransmit( LUACLASS_GLSLPROGRAM_ARB, program );

        j->RegisterInterfaceTrans( L, _interface, 0, LUACLASS_GLSLPROGRAM_ARB );

        lua_pushcstring( L, "glslprogram" );
        lua_setfield( L, LUA_ENVIRONINDEX, "__type" );
        return 0;
    }
};

glProgramARB::glProgramARB( lua_State *L, glDriver *driver, GLhandleARB handle ) : glObjectARB( L, driver, handle )
{
    // Extend the class
    Extend( L, gl_programARB::constructor );

    // Standardize the values.
    m_numAttachedShaders = 0;
}

glProgramARB::~glProgramARB( void )
{
    // Destruction is handled by glObjectARB.
    return;
}

static LUA_DECLARE( createProgram )
{
    glDriver *driver = (glDriver*)lua_getmethodtrans( L );
    glContextStack contextStack( driver );
    glContextDescriptor *context = driver->contextInfo;

    LUA_CHECK( context->glCreateProgramObjectARB );

    GLhandleARB programHandle = context->glCreateProgramObjectARB();

    LUA_CHECK( programHandle != 0 );

    glProgramARB *program = new glProgramARB( L, driver, programHandle );
    program->SetParent( driver );
    program->PushStack( L );
    program->Finalize( L );
    return 1;
}

static LUA_DECLARE( useProgram )
{
    glProgramARB *program;

    LUA_ARGS_BEGIN;
    argStream.ReadClass( program, LUACLASS_GLSLPROGRAM_ARB, NULL );
    LUA_ARGS_END;

    glDriver *driver = (glDriver*)lua_getmethodtrans( L );

    // The program to-be-set has to differ from the current program.
    LUA_CHECK( driver->currentProgramARB != program );

    // Clear current OpenGL error.
    _glGetError();

    driver->contextInfo->glUseProgramObjectARB( program ? program->m_handle : 0 );

    // Only proceed if we could change the current program.
    LUA_CHECK( _glGetError() == 0 );

    // Change the currently bound program.
    driver->currentProgramARB = program;

    // Return true as we used LUA_CHECK.
    lua_pushboolean( L, true );
    return 1;
}

const luaL_Reg shader_driver_interface[] =
{
    LUA_METHOD( compileShader ),
    LUA_METHOD( createProgram ),
    LUA_METHOD( useProgram ),
    { NULL, NULL }
};

void luaGL_registerShaderExtension( lua_State *L, glDriver *driver )
{
    // TODO.
    return;
}