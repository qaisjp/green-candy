/*****************************************************************************
*
*  PROJECT:     Lua Interpreter
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        glDriver/gl_texture.cpp
*  PURPOSE:     OpenGL driver texture management
*  DEVELOPERS:  Martin Turski <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#include <StdInc.h>
#include "gl_internal.h"

static LUA_DECLARE( bind )
{
    glTexture *tex = (glTexture*)lua_getmethodtrans( L );
    glDriver *sys = tex->m_driver;

    glContextStack stack( sys );

    _glBindTexture( GL_TEXTURE_2D, tex->m_texIndex );
    return 0;
};

static LUA_DECLARE( getSize )
{
    glTexture *tex = (glTexture*)lua_getmethodtrans( L );

    lua_pushnumber( L, tex->m_width );
    lua_pushnumber( L, tex->m_height );
    return 2;
}

static const luaL_Reg _interface[] =
{
    LUA_METHOD( bind ),
    LUA_METHOD( getSize ),
    { NULL, NULL }
};

static LUA_DECLARE( tex_constructor )
{
    glTexture *tex = (glTexture*)lua_touserdata( L, lua_upvalueindex( 1 ) );

    ILuaClass *j = lua_refclass( L, LUA_ENVIRONINDEX );
    j->SetTransmit( LUACLASS_GLTEXTURE, tex );

    j->RegisterInterfaceTrans( L, _interface, 0, LUACLASS_GLTEXTURE );

    lua_pushcstring( L, "gltexture" );
    lua_setfield( L, LUA_ENVIRONINDEX, "__type" );
    return 0;
}

glTexture::glTexture( lua_State *L, glDriver *gl, GLuint texIndex, unsigned int width, unsigned int height ) : LuaInstance( L ), m_texIndex( texIndex ), m_driver( gl ), m_width( width ), m_height( height )
{
    Extend( L, tex_constructor );
}

glTexture::~glTexture( void )
{
    glContextStack stack( m_driver );

    _glDeleteTextures( 1, &m_texIndex );
}

inline unsigned int makePow2( unsigned int val )
{
    unsigned int num = 1;

    while ( num < val )
        num = num * 2;

    return num;
}

enum eTextureWrapType : GLenum
{
};

DECLARE_ENUM( eTextureWrapType );

IMPLEMENT_ENUM_BEGIN( eTextureWrapType )
    ADD_ENUM( GL_REPEAT, "repeat" )
    ADD_ENUM( GL_CLAMP, "clamp" )
IMPLEMENT_ENUM_END( "eTextureWrapType" )

enum eInterpolationType : GLenum
{
};

DECLARE_ENUM( eInterpolationType );

IMPLEMENT_ENUM_BEGIN( eInterpolationType )
    ADD_ENUM( GL_LINEAR, "linear" )
    ADD_ENUM( GL_NEAREST, "nearest" )
IMPLEMENT_ENUM_END( "eInterpolationType" )

template <class blockType>
__forceinline void PadImageData( void *dst, const void *src, unsigned int width, unsigned int height, unsigned int _width, unsigned int _height, unsigned int spacingX, unsigned int spacingY )
{
    const blockType *srcData = (const blockType*)src;
    blockType *dstData = (blockType*)dst;

    unsigned int dxtWidth = width / spacingX;
    unsigned int dxtHeight = height / spacingY;
    unsigned int _dxtWidth = _width / spacingX;
    unsigned int _dxtHeight = _height / spacingY;

    // Copy main framework data
    for ( unsigned int y = 0; y < dxtHeight; y++ )
    {
        for ( unsigned int x = 0; x < dxtWidth; x++ )
        {
            dstData[x + y * _dxtWidth] = *( srcData + x + y * dxtWidth );
        }
    }

    for ( unsigned int y = dxtHeight; y < _dxtHeight; y++ )
    {
        for ( unsigned int x = 0; x < _dxtWidth; x++ )
        {
            dstData[x + y * _dxtWidth] = blockType();
        }
    }

    for ( unsigned int y = 0; y < dxtHeight; y++ )
    {
        for ( unsigned int x = dxtWidth; x < _dxtWidth; x++ )
        {
            dstData[x + y * _dxtWidth] = blockType();
        }
    }
}

static LUA_DECLARE( createTexture2D )
{
    Bitmap *map;
    bool mipmap;
    eTextureWrapType wrapType;
    eInterpolationType interpolateType;

    LUA_ARGS_BEGIN;
    argStream.ReadClass( map, LUACLASS_BITMAP );
    argStream.ReadBool( mipmap, true );
    argStream.ReadEnumString( wrapType, (eTextureWrapType)GL_CLAMP );
    argStream.ReadEnumString( interpolateType, (eInterpolationType)GL_LINEAR );
    LUA_ARGS_END;

    glDriver *driver = (glDriver*)lua_getmethodtrans( L );

    unsigned int width = map->m_width;
    unsigned int height = map->m_height;

    if ( width > (unsigned int)driver->maxTextureSize || height > (unsigned int)driver->maxTextureSize )
    {
        lua_pushboolean( L, false );
        lua_pushcstring( L, "bitmap too big for driver" );
        return 2;
    }

    glContextStack stack( driver );

    GLuint texIndex;
    _glGenTextures( 1, &texIndex );

    _glBindTexture( GL_TEXTURE_2D, texIndex );

    _glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE );

    int error;

    unsigned int _width;
    unsigned int _height;

    GLenum format;

    union
    {
        const void *data;
        unsigned int *uiData;
        unsigned char *ucData;
    };

    bool allocatedData = false;

    // Check the bitmap data type (and whether we support it!)
    Bitmap::eDataType bmpDataType = map->m_dataType;
    bool allowArbitrarySize = false;
    size_t dataSize = map->m_dataSize;

    switch( bmpDataType )
    {
    // we always support raw colors.
    case Bitmap::BITMAP_RGB:
        allowArbitrarySize = driver->allowTexturesOfArbitrarySize;

        _glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

        format = GL_RGB;
        break;
    case Bitmap::BITMAP_RGBA:
        allowArbitrarySize = driver->allowTexturesOfArbitrarySize;

        _glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

        format = GL_RGBA;
        break;
    // check whether compressed formats are available
    case Bitmap::BITMAP_DXT1_RGB:
        if ( !driver->supportsDXT1 )
            goto failSupportFormat;

        format = GL_COMPRESSED_RGB_S3TC_DXT1_EXT;
        break;
    case Bitmap::BITMAP_DXT1_RGBA:
        if ( !driver->supportsDXT1 )
            goto failSupportFormat;

        format = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
        break;
    case Bitmap::BITMAP_DXT3_RGBA:
        if ( !driver->supportsDXT3 )
            goto failSupportFormat;

        format = GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
        break;
    case Bitmap::BITMAP_DXT5_RGBA:
        if ( !driver->supportsDXT5 )
            goto failSupportFormat;

        format = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
        break;
    default:
failSupportFormat:
        // We do not support the type, so let's error out.
        _glDeleteTextures( 1, &texIndex );

        lua_pushboolean( L, false );
        return 1;
    }

    // Can we load any texture we want?
    if ( allowArbitrarySize )
    {
        // Textures can be of arbitrary size.

        // We just use the arbitrary size.
        _width = width;
        _height = height;

        data = map->m_data;
    }
    else
    {
        // We cannot, let's pad it.
        _width = makePow2( width );
        _height = makePow2( height );

        // Create padded color buffers
        switch( bmpDataType )
        {
        case Bitmap::BITMAP_RGB:
        case Bitmap::BITMAP_RGBA:
            // Scale the texture background if necessary
            if ( width != _width || height != _height )
            {
                // Recalculate the data size as it changed
                dataSize = sizeof(unsigned int) * _width * _height;

                uiData = new unsigned int[_width * _height];
                allocatedData = true;

                // Perform the padding
                if ( bmpDataType == Bitmap::BITMAP_RGB )
                    PadImageData <BitmapRGB> ( uiData, map->m_data, width, height, _width, _height, 1, 1 );
                else
                    PadImageData <unsigned int> ( uiData, map->m_data, width, height, _width, _height, 1, 1 );
            }
            else
            {
                // We can use the bitmap data directly.
                data = map->m_data;
            }

            break;
        case Bitmap::BITMAP_DXT1_RGB:
        case Bitmap::BITMAP_DXT1_RGBA:
            // We have to pad the texture
            if ( width != _width || height != _height )
            {
                dataSize = _width * _height / 2;

                ucData = new unsigned char[dataSize];
                allocatedData = true;

                PadImageData <long long> ( ucData, map->m_data, width, height, _width, _height, 4, 4 );
            }
            else
            {
                // Use the bitmap data
                data = map->m_data;
            }

            break;
        case Bitmap::BITMAP_DXT3_RGBA:
        case Bitmap::BITMAP_DXT5_RGBA:
            // We have to pad the texture
            if ( width != _width || height != _height )
            {
                dataSize = _width * _height;

                ucData = new unsigned char[dataSize];
                allocatedData = true;

                PadImageData <s3tc_dxt3_block> ( ucData, map->m_data, width, height, _width, _height, 4, 4 );
            }
            else
            {
                // Use the bitmap data
                data = map->m_data;
            }

            break;
        default:
            _glDeleteTextures( 1, &texIndex );

            lua_pushboolean( L, false );
            return 1;
        }
    }

    // Set the wrapping method
    _glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapType );
    _glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapType );

    // when texture area is large, bilinear filter the original
    _glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, interpolateType );

    switch( format )
    {
    case GL_RGB:
    case GL_RGBA:
        // Only enable mipmaps if the gluLibrary successfully loaded!
        if ( gluLibrary && mipmap )
        {
            // when texture area is small, bilinear filter the closest mipmap
            _glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST );

            // build our texture mipmaps
            error = _gluBuild2DMipmaps( GL_TEXTURE_2D, 3, _width, _height, format, GL_UNSIGNED_BYTE, data );
        }
        else
        {
            _glGetError();

            // Display default image at minimum
            _glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, interpolateType );

            // Load raw data without mipmaps
            _glTexImage2D( GL_TEXTURE_2D, 0, format, _width, _height, 0, format, GL_UNSIGNED_BYTE, data );

            error = _glGetError();
        }
        break;
    case GL_COMPRESSED_RGB_S3TC_DXT1_EXT:
    case GL_COMPRESSED_RGBA_S3TC_DXT1_EXT:
    case GL_COMPRESSED_RGBA_S3TC_DXT3_EXT:
    case GL_COMPRESSED_RGBA_S3TC_DXT5_EXT:
        if ( !driver->contextInfo->glCompressedTexImage2D )
        {
            error = -1;
            break;
        }

        _glGetError();

        _glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );

        driver->contextInfo->glCompressedTexImage2D( GL_TEXTURE_2D, 0, format, _width, _height, 0, dataSize, data );
        
        error = _glGetError();
        break;
    default:
        // Error out.
        error = -1;
        break;
    }

    if ( allocatedData )
        delete data;

    if ( error != 0 )
    {
        _glDeleteTextures( 1, &texIndex );

        lua_pushboolean( L, false );
        return 1;
    }

    // Create the native texture export
    glTexture *tex = new glTexture( L, driver, texIndex, _width, _height );
    tex->SetParent( driver );
    tex->PushStack( L );
    tex->Finalize( L );
    return 1;
}

const luaL_Reg tex_driver_interface[] =
{
    LUA_METHOD( createTexture2D ),
    { NULL, NULL }
};