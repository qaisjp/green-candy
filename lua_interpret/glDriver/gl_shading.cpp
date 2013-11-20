/*****************************************************************************
*
*  PROJECT:     Lua Interpreter
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        glDriver/gl_shading.cpp
*  PURPOSE:     OpenGL driver programmable shading implementation
*               ARB version; use this implementation for legacy compatibility.
*  DEVELOPERS:  Martin Turski <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#include <StdInc.h>
#include "gl_internal.h"

// Class for shader context switching.
struct glProgramContextARB
{
    inline glProgramContextARB( glDriver *driver, glProgramARB *program ) : m_driver( driver ), isSet( false )
    {
        glProgramARB *prev = driver->currentProgramARB;
            
        if ( prev != program )
        {
            m_prevProgram = prev;
            driver->currentProgramARB = program;

            driver->contextInfo->glUseProgramObjectARB( program->m_handle );

            driver->isProgramStackActive = true;
            isSet = true;
        }
    }

    inline ~glProgramContextARB( void )
    {
        if ( isSet )
        {
            glProgramARB *prev = m_prevProgram;

            m_driver->currentProgramARB = prev;

            if ( !prev )
            {
                m_driver->contextInfo->glUseProgramObjectARB( 0 );
                m_driver->isProgramStackActive = false;
            }
            else
                m_driver->contextInfo->glUseProgramObjectARB( prev->m_handle );
        }
    }

    bool isSet;
    glDriver *m_driver;
    glProgramARB *m_prevProgram;
};

/*
    OpenGL ARB objects was an attempt to bring OOP-style thinking into the OpenGL core.
    It was declined and the duty of an OOP-syntax put onto the developer who uses OpenGL.
    The shader OOP allows for neat functionality, such as per-object parameters and a
    shared destructor routine. Also, the handle can be per-object instead of per-hyperclass.

    OpenGL ARB objects are not constructible as hyperclass. They only exist as shader object
    or program object.
*/
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

        lua_pushcstring( L, "glslobject_arb" );
        lua_setfield( L, LUA_ENVIRONINDEX, "__type" );
        return 0;
    }
};

glObjectARB::glObjectARB( lua_State *L, glDriver *driver, GLhandleARB handle ) : glClass( L, driver )
{
    // Extend to basic object functionality
    Extend( L, gl_objectARB::constructor );

    // Store the basic parameters.
    m_handle = handle;
}

glObjectARB::~glObjectARB( void )
{
    // Destroy it from the driver.
    m_driver->contextInfo->glDeleteObjectARB( m_handle );
}

namespace gl_shaderARB
{
    static LUA_DECLARE( shaderSource )
    {
        int luaTop = lua_gettop( L );

        // Get an array of all strings.
        const GLcharARB **scriptArray = new const GLcharARB*[ luaTop ];
        GLsizei *scriptSizes = new GLsizei[ luaTop ];

        for ( int n = 0; n < luaTop; n++ )
        {
            size_t argStrLen;
            lua_String argStr = lua_tolstring( L, n + 1, &argStrLen );

            if ( argStr )
            {
                scriptArray[n] = (const GLcharARB*)argStr;
                scriptSizes[n] = (GLsizei)argStrLen;
            }
            else
            {
                scriptArray[n] = (const GLcharARB*)"";
                scriptSizes[n] = 0;
            }
        }

        // Replace the source code of the shader.
        glShaderARB *shader = (glShaderARB*)lua_getmethodtrans( L );

        glDriver *driver = shader->m_driver;
        glContextStack contextStack( driver );

        driver->contextInfo->glShaderSourceARB( shader->m_handle, (GLsizei)luaTop, scriptArray, scriptSizes );

        // Since we have out-cased the errors given by the implementation documentation,
        // we do not require a boolean return.
        LUA_SUCCESS;
    }

    static LUA_DECLARE( compile )
    {
        glShaderARB *shader = (glShaderARB*)lua_getmethodtrans( L );

        glDriver *driver = shader->m_driver;
        glContextStack contextStack( driver );

        glContextDescriptor *context = driver->contextInfo;

        // We know that according to the documentation, no viable error-out may happen.
        context->glCompileShaderARB( shader->m_handle );

        // Return whether compilation succeeded.
        GLint compileStatus;
        context->glGetObjectParameterivARB( shader->m_handle, GL_OBJECT_COMPILE_STATUS_ARB, &compileStatus );

        lua_pushboolean( L, compileStatus != 0 );
        return 1;
    }

    static const luaL_Reg _interface[] =
    {
        LUA_METHOD( shaderSource ),
        LUA_METHOD( compile ),
        { NULL, NULL }
    };

    static LUA_DECLARE( constructor )
    {
        glShaderARB *shader = (glShaderARB*)lua_touserdata( L, lua_upvalueindex( 1 ) );

        ILuaClass *j = lua_refclass( L, LUA_ENVIRONINDEX );
        j->SetTransmit( LUACLASS_GLSLSHADER_ARB, shader );

        j->RegisterInterfaceTrans( L, _interface, 0, LUACLASS_GLSLSHADER_ARB );

        lua_pushcstring( L, "glslshader_arb" );
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

static LUA_DECLARE( createShader )
{
    GLeShaderType shaderType;

    LUA_ARGS_BEGIN;
    argStream.ReadEnumString( shaderType );
    LUA_ARGS_END;

    glDriver *driver = (glDriver*)lua_getmethodtrans( L );

    LUA_CHECK( driver->supports_ARB_shader_objects );

    glContextStack contextStack( driver );
    glContextDescriptor *context = driver->contextInfo;

    GLhandleARB shaderHandle = context->glCreateShaderObjectARB( shaderType );

    LUA_CHECK( shaderHandle );

    // Create the Lua Class representing the OpenGL shader object.
    glShaderARB *shader = new glShaderARB( L, driver, shaderHandle, shaderType );
    shader->SetParent( driver );
    shader->PushStack( L );
    shader->Finalize( L );
    return 1;
}

// Utility function to directly compile a shader at creation.
// Only supports one shader source string as opposed to shader.shaderSource(...)
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
    context->glCompileShaderARB( shaderHandle );

    GLint compileStatus;
    context->glGetObjectParameterivARB( shaderHandle, GL_OBJECT_COMPILE_STATUS_ARB, &compileStatus );

    if ( compileStatus == 0 )
    {
        context->glDeleteObjectARB( shaderHandle );

        lua_pushboolean( L, false );
        return 1;
    }

    // Create the Lua Class representing the OpenGL shader object.
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

        shader->IncrementMethodStack();

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

        shader->DecrementMethodStack();

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

    static LUA_DECLARE( getUniformLocation )
    {
        lua_String uniformName;

        LUA_ARGS_BEGIN;
        argStream.ReadString( uniformName );
        LUA_ARGS_END;

        glProgramARB *program = (glProgramARB*)lua_getmethodtrans( L );

        glDriver *driver = program->m_driver;
        glContextStack contextStack( driver );

        // Errors may happen. Let us clear any already present error.
        _glGetError();

        GLint iLocation = driver->contextInfo->glGetUniformLocationARB( program->m_handle, (const GLcharARB*)uniformName );

        // Make sure the program was successfully linked.
        LUA_CHECK( _glGetError() == 0 );

        lua_pushinteger( L, iLocation );
        return 1;
    }

    // This function requires an active OpenGL context.
    static AINLINE bool _GetUniformLocation( lua_State *L, int iStackId, glProgramARB *program, GLint& uniLoc )
    {
        int iType = lua_type( L, iStackId );

        // We must get a location.
        switch( iType )
        {
        case LUA_TSTRING:
            // The string describes a uniform name.
            {
                // Checks for errors during the runtime.
                // i.e. locations not available.
                _glGetError();

                GLint iLocation = program->m_driver->contextInfo->glGetUniformLocationARB( program->m_handle, (GLcharARB*)lua_tostring( L, iStackId ) );

                if ( _glGetError() != 0 )
                    return false;

                uniLoc = iLocation;
                return true;
            }
        case LUA_TNUMBER:
            uniLoc = (GLint)lua_tointeger( L, iStackId );
            return true;
        }

        return false;
    }

    static LUA_DECLARE( getActiveUniform )
    {
        glProgramARB *program = (glProgramARB*)lua_getmethodtrans( L );

        // We must get a context from the get-go.
        glDriver *driver = program->m_driver;
        glContextStack contextStack( driver );

        // Retrive a possible uniform location.
        GLint iLocation = -1;

        LUA_CHECK( _GetUniformLocation( L, 1, program, iLocation ) && iLocation > -1 );

        glContextDescriptor *context = driver->contextInfo;

        // Post data about the uniform.
        GLsizei uniLength;

        // Shield against errors.
        _glGetError();

        // First get the length of the uniform name.
        // Pssst. We are using an undocumented feature!
        context->glGetActiveUniformARB( program->m_handle, (GLuint)iLocation, 0, &uniLength, NULL, NULL, NULL );

        LUA_CHECK( _glGetError() == 0 );

        // Now get all the data besides the name length.
        GLcharARB *name = NULL;

        if ( uniLength > 0 )
            name = new GLcharARB[ uniLength ];

        GLint iDataSize;
        GLenum iDataType;

        // Shield against errors.
        _glGetError();

        context->glGetActiveUniformARB( program->m_handle, (GLuint)iLocation, uniLength, NULL, &iDataSize, &iDataType, name );

        LUA_CHECK( _glGetError() == 0 );

        // Create a Lua table with all information.
        lua_newtable( L );
        lua_pushinteger( L, iDataSize );
        lua_setfield( L, -2, "size" );
        
        if ( name )
            lua_pushlstring( L, name, uniLength );
        else
            lua_pushboolean( L, false );

        lua_setfield( L, -2, "name" );

        // Dispatch the type into a name string.
        switch( iDataType )
        {
        case GL_FLOAT:                          lua_pushcstring( L, "float" ); break;
        case GL_FLOAT_VEC2_ARB:                 lua_pushcstring( L, "float_vec2" ); break;
        case GL_FLOAT_VEC3_ARB:                 lua_pushcstring( L, "float_vec3" ); break;
        case GL_FLOAT_VEC4_ARB:                 lua_pushcstring( L, "float_vec4" ); break;
        case GL_INT:                            lua_pushcstring( L, "int" ); break;
        case GL_INT_VEC2_ARB:                   lua_pushcstring( L, "int_vec2" ); break;
        case GL_INT_VEC3_ARB:                   lua_pushcstring( L, "int_vec3" ); break;
        case GL_INT_VEC4_ARB:                   lua_pushcstring( L, "int_vec4" ); break;
        case GL_BOOL_ARB:                       lua_pushcstring( L, "bool" ); break;
        case GL_BOOL_VEC2_ARB:                  lua_pushcstring( L, "bool_vec2" ); break;
        case GL_BOOL_VEC3_ARB:                  lua_pushcstring( L, "bool_vec3" ); break;
        case GL_BOOL_VEC4_ARB:                  lua_pushcstring( L, "bool_vec4" ); break;
        case GL_FLOAT_MAT2_ARB:                 lua_pushcstring( L, "float_mat2" ); break;
        case GL_FLOAT_MAT3_ARB:                 lua_pushcstring( L, "float_mat3" ); break;
        case GL_FLOAT_MAT4_ARB:                 lua_pushcstring( L, "float_mat4" ); break;
        case GL_SAMPLER_1D_ARB:                 lua_pushcstring( L, "sampler_1d" ); break;
        case GL_SAMPLER_2D_ARB:                 lua_pushcstring( L, "sampler_2d" ); break;
        case GL_SAMPLER_3D_ARB:                 lua_pushcstring( L, "sampler_3d" ); break;
        case GL_SAMPLER_CUBE_ARB:               lua_pushcstring( L, "sampler_cube" ); break;
        case GL_SAMPLER_1D_SHADOW_ARB:          lua_pushcstring( L, "sampler_1d_shadow" ); break;
        case GL_SAMPLER_2D_SHADOW_ARB:          lua_pushcstring( L, "sampler_2d_shadow" ); break;
        case GL_SAMPLER_2D_RECT_ARB:            lua_pushcstring( L, "sampler_2d_rect" ); break;
        case GL_SAMPLER_2D_RECT_SHADOW_ARB:     lua_pushcstring( L, "sampler_2d_rect_shadow" ); break;
        default:
            lua_pushcstring( L, "unknown" );
            break;
        }

        lua_setfield( L, -2, "type" );
        return 1;   // return the info table.
    }

    // You can only set uniforms, never get their values.
    static LUA_DECLARE( setUniform )
    {
        glProgramARB *program = (glProgramARB*)lua_getmethodtrans( L );

        // We must get a context from the get-go.
        glDriver *driver = program->m_driver;
        glContextStack contextStack( driver );

        // Retrive a possible uniform location.
        GLint iLocation = -1;

        LUA_CHECK( _GetUniformLocation( L, 1, program, iLocation ) && iLocation > -1 );

        glContextDescriptor *context = driver->contextInfo;

        // Get the data size and data type to know how to apply values to it.
        GLint iDataSize;
        GLenum iDataType;

        // Shield against errors.
        _glGetError();

        context->glGetActiveUniformARB( program->m_handle, (GLuint)iLocation, 0, NULL, &iDataSize, &iDataType, NULL );

        LUA_CHECK( _glGetError() == 0 );

        {
            // Temporarily switch program contexts
            glProgramContextARB progContext( driver, program );

            // Protect against Lua exceptions.
            try
            {
                // Write data according to the data type.
                // Note: invalid id accesses in MTA:Lua are not errors; hence they result into nil
                switch( iDataType )
                {
                case GL_FLOAT:
                    context->glUniform1fARB( (GLuint)iLocation,
                        (GLfloat)lua_tonumber( L, 2 )
                    );
                    break;
                case GL_FLOAT_VEC2_ARB:
                    context->glUniform2fARB( (GLuint)iLocation,
                        (GLfloat)lua_tonumber( L, 2 ),
                        (GLfloat)lua_tonumber( L, 3 )
                    );
                    break;
                case GL_FLOAT_VEC3_ARB:
                    context->glUniform3fARB( (GLuint)iLocation,
                        (GLfloat)lua_tonumber( L, 2 ),
                        (GLfloat)lua_tonumber( L, 3 ),
                        (GLfloat)lua_tonumber( L, 4 )
                    );
                    break;
                case GL_FLOAT_VEC4_ARB:
                    context->glUniform4fARB( (GLuint)iLocation,
                        (GLfloat)lua_tonumber( L, 2 ),
                        (GLfloat)lua_tonumber( L, 3 ),
                        (GLfloat)lua_tonumber( L, 4 ),
                        (GLfloat)lua_tonumber( L, 5 )
                    );
                    break;
                case GL_INT:
                    context->glUniform1iARB( (GLuint)iLocation,
                        (GLint)lua_tointeger( L, 2 )
                    );
                    break;
                case GL_INT_VEC2_ARB:
                    context->glUniform2iARB( (GLuint)iLocation,
                        (GLint)lua_tointeger( L, 2 ),
                        (GLint)lua_tointeger( L, 3 )
                    );
                    break;
                case GL_INT_VEC3_ARB:
                    context->glUniform3iARB( (GLuint)iLocation,
                        (GLint)lua_tointeger( L, 2 ),
                        (GLint)lua_tointeger( L, 3 ),
                        (GLint)lua_tointeger( L, 4 )
                    );
                    break;
                case GL_INT_VEC4_ARB:
                    context->glUniform4iARB( (GLuint)iLocation,
                        (GLint)lua_tointeger( L, 2 ),
                        (GLint)lua_tointeger( L, 3 ),
                        (GLint)lua_tointeger( L, 4 ),
                        (GLint)lua_tointeger( L, 5 )
                    );
                    break;
                case GL_BOOL_ARB:
                    context->glUniform1iARB( (GLuint)iLocation,
                        (GLint)lua_toboolean( L, 2 )
                    );
                    break;
                case GL_BOOL_VEC2_ARB:
                    context->glUniform2iARB( (GLuint)iLocation,
                        (GLint)lua_toboolean( L, 2 ),
                        (GLint)lua_toboolean( L, 3 )
                    );
                    break;
                case GL_BOOL_VEC3_ARB:
                    context->glUniform3iARB( (GLuint)iLocation,
                        (GLint)lua_toboolean( L, 2 ),
                        (GLint)lua_toboolean( L, 3 ),
                        (GLint)lua_toboolean( L, 4 )
                    );
                    break;
                case GL_BOOL_VEC4_ARB:
                    context->glUniform4iARB( (GLuint)iLocation,
                        (GLint)lua_toboolean( L, 2 ),
                        (GLint)lua_toboolean( L, 3 ),
                        (GLint)lua_toboolean( L, 4 ),
                        (GLint)lua_toboolean( L, 5 )
                    );
                    break;
                case GL_FLOAT_MAT2_ARB:
                case GL_FLOAT_MAT3_ARB:
                case GL_FLOAT_MAT4_ARB:
                    // TODO
                    lua_pushboolean( L, false );
                    return 1;
                case GL_SAMPLER_1D_ARB:
                case GL_SAMPLER_2D_ARB:
                case GL_SAMPLER_3D_ARB:
                case GL_SAMPLER_CUBE_ARB:
                case GL_SAMPLER_1D_SHADOW_ARB:
                case GL_SAMPLER_2D_SHADOW_ARB:
                case GL_SAMPLER_2D_RECT_ARB:
                case GL_SAMPLER_2D_RECT_SHADOW_ARB:
                    context->glUniform1iARB( (GLuint)iLocation,
                        (GLint)lua_tointeger( L, 2 )
                    );
                    break;
                default:
                    // We do not know how to handle this type.
                    lua_pushboolean( L, false );
                    return 1;
                }
            }
            catch( lua_exception& )
            {
                lua_pushboolean( L, false );
                return 1;
            }
        }

        // We succeeded writing the type.
        lua_pushboolean( L, true );
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
        LUA_METHOD( getUniformLocation ),
        LUA_METHOD( getActiveUniform ),
        LUA_METHOD( setUniform ),
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

        lua_pushcstring( L, "glslprogram_arb" );
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
    // Detach all shaders.
    // This is easily done by dereferencing them.
    for ( shaderList_t::const_iterator iter = m_attachedShaders.begin(); iter != m_attachedShaders.end(); iter++ )
    {
        (*iter)->DecrementMethodStack();
    }

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

    // We may not have a context stack running
    LUA_CHECK( !driver->isProgramStackActive );

    // The program to-be-set has to differ from the current program.
    LUA_CHECK( ( !program || program->m_driver == driver ) && driver->currentProgramARB != program );

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

static LUA_DECLARE( getRunningProgram )
{
    glDriver *driver = (glDriver*)lua_getmethodtrans( L );

    if ( glProgramARB *program = driver->currentProgramARB )
        program->PushStack( L );
    else
        lua_pushboolean( L, false );

    return 1;
}

const luaL_Reg shader_driver_interface[] =
{
    LUA_METHOD( createShader ),
    LUA_METHOD( compileShader ),
    LUA_METHOD( createProgram ),
    LUA_METHOD( useProgram ),
    LUA_METHOD( getRunningProgram ),
    { NULL, NULL }
};

void luaGL_registerShaderExtension( lua_State *L, glDriver *driver )
{
    // TODO.
    return;
}