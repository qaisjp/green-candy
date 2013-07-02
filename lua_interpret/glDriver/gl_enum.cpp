/*****************************************************************************
*
*  PROJECT:     Lua Interpreter
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        glDriver/gl_enum.cpp
*  PURPOSE:     OpenGL driver enumerations and quick lookup routines
*  DEVELOPERS:  Martin Turski <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#include <StdInc.h>
#include "gl_internal.h"

__forceinline static const GLparamdesc* GetParamByHash( const GLparamdesc *list, size_t len, unsigned int hash )
{
    for ( unsigned int n = 0; n < len; n++ )
    {
        const GLparamdesc *item = list + n;

        if ( item->hash == hash )
            return item;
    }

    return NULL;
}

namespace gl_internalFormat
{
    static const GLparamdesc colorInfo[] =
    {
        { "GL_RGB", GL_RGB, 0x8248eabe },
        { "GL_RGBA", GL_RGBA, 0xbd9e6145 }
    };

    static const GLparamdesc colorInfo_1_2[] =
    {
        { "GL_BGRA", GL_BGRA, 0x38e3f2b0 },
        { "GL_BGR", GL_BGR, 0x8bf3e5ff }
    };

    static const GLparamdesc depthInfo[] =
    {
        { "GL_DEPTH_COMPONENT16", GL_DEPTH_COMPONENT16, 0x30ea4626 },
        { "GL_DEPTH_COMPONENT24", GL_DEPTH_COMPONENT24, 0x7a56dd0d },
        { "GL_DEPTH_COMPONENT32", GL_DEPTH_COMPONENT32, 0xd56cf18b },
        { "GL_DEPTH_COMPONENT32F", GL_DEPTH_COMPONENT32F, 0xf998f894 }
    };

    static const GLparamdesc stencilInfo[] =
    {
        { "GL_DEPTH24_STENCIL8", GL_DEPTH24_STENCIL8, 0x426bd723 },
        { "GL_DEPTH32F_STENCIL8", GL_DEPTH32F_STENCIL8, 0xe6eb5645 }
    };

    static const GLparamdesc stencilInfo_3_0[] =
    {
        { "GL_STENCIL_INDEX1", GL_STENCIL_INDEX1, 0xce58659 },
        { "GL_STENCIL_INDEX4", GL_STENCIL_INDEX4, 0x7fc81f06 },
        { "GL_STENCIL_INDEX8", GL_STENCIL_INDEX8, 0x77bff639 },
        { "GL_STENCIL_INDEX16", GL_STENCIL_INDEX16, 0xf9e96ef8 }
    };

    static const GLparamdesc compressed_1_3[] =
    {
        { "GL_COMPRESSED_RGB", GL_COMPRESSED_RGB, 0x2b55ea03 },
        { "GL_COMPRESSED_RGBA", GL_COMPRESSED_RGBA, 0x743cedbc },
        { "GL_COMPRESSED_ALPHA", GL_COMPRESSED_ALPHA, 0x50124115 },
        { "GL_COMPRESSED_LUMINANCE", GL_COMPRESSED_LUMINANCE, 0x81bd3e19 },
        { "GL_COMPRESSED_LUMINANCE_ALPHA", GL_COMPRESSED_LUMINANCE_ALPHA, 0x5622e212 },
        { "GL_COMPRESSED_INTENSITY", GL_COMPRESSED_INTENSITY, 0x57c419c8 }
    };

    static const GLparamdesc compressed_2_1[] =
    {
        { "GL_COMPRESSED_SRGB", GL_COMPRESSED_SRGB, 0x4396b365 },
        { "GL_COMPRESSED_SRGB_ALPHA", GL_COMPRESSED_SRGB_ALPHA, 0x496105f6 },
        { "GL_COMPRESSED_SLUMINANCE", GL_COMPRESSED_SLUMINANCE, 0x81e3a38f },
        { "GL_COMPRESSED_SLUMINANCE_ALPHA", GL_COMPRESSED_SLUMINANCE_ALPHA, 0xfb151109 }
    };

    static const GLparamdesc compressed_3_0[] =
    {
        { "GL_COMPRESSED_RED", GL_COMPRESSED_RED, 0xd5fc5b4a },
        { "GL_COMPRESSED_RG", GL_COMPRESSED_RG, 0xcd6e5656 },
        { "GL_COMPRESSED_RED_RGTC1", GL_COMPRESSED_RED_RGTC1, 0xc9e2f521 },
        { "GL_COMPRESSED_SIGNED_RED_RGTC1", GL_COMPRESSED_SIGNED_RED_RGTC1, 0x9d4ee8d7 },
        { "GL_COMPRESSED_RG_RGTC2", GL_COMPRESSED_RG_RGTC2, 0xc869ba8 },
        { "GL_COMPRESSED_SIGNED_RG_RGTC2", GL_COMPRESSED_SIGNED_RG_RGTC2, 0xb7978e77 }
    };

    static const GLparamdesc compressed_3dfx[] =
    {
        { "GL_COMPRESSED_RGB_FXT1_3DFX", GL_COMPRESSED_RGB_FXT1_3DFX, 0xa954af66 },
        { "GL_COMPRESSED_RGBA_FXT1_3DFX", GL_COMPRESSED_RGBA_FXT1_3DFX, 0xa6f4d5a2 }
    };

    static const GLparamdesc compressed_4_3[] =
    {
        { "GL_COMPRESSED_RGB8_ETC2", GL_COMPRESSED_RGB8_ETC2, 0x2afb7d21 },
        { "GL_COMPRESSED_SRGB8_ETC2", GL_COMPRESSED_SRGB8_ETC2, 0x80309e2d },
        { "GL_COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2", GL_COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2, 0x4106fbe5 },
        { "GL_COMPRESSED_SRGB8_PUNCHTHROUGH_ALPHA1_ETC2", GL_COMPRESSED_SRGB8_PUNCHTHROUGH_ALPHA1_ETC2, 0x86016f05 },
        { "GL_COMPRESSED_RGBA8_ETC2_EAC", GL_COMPRESSED_RGBA8_ETC2_EAC, 0x136e5539 },
        { "GL_COMPRESSED_SRGB8_ALPHA8_ETC2_EAC", GL_COMPRESSED_SRGB8_ALPHA8_ETC2_EAC, 0x72681d56 },
        { "GL_COMPRESSED_R11_EAC", GL_COMPRESSED_R11_EAC, 0x18a1043f },
        { "GL_COMPRESSED_SIGNED_R11_EAC", GL_COMPRESSED_SIGNED_R11_EAC, 0xfde19efc },
        { "GL_COMPRESSED_RG11_EAC", GL_COMPRESSED_RG11_EAC, 0x361b25a0 },
        { "GL_COMPRESSED_SIGNED_RG11_EAC", GL_COMPRESSED_SIGNED_RG11_EAC, 0x4523e46d }
    };

    static const GLparamdesc compressed_arb[] =
    {
        { "GL_COMPRESSED_ALPHA_ARB", GL_COMPRESSED_ALPHA_ARB, 0x58a02a0e },
        { "GL_COMPRESSED_LUMINANCE_ARB", GL_COMPRESSED_LUMINANCE_ARB, 0xda5b7c8c },
        { "GL_COMPRESSED_LUMINANCE_ALPHA_ARB", GL_COMPRESSED_LUMINANCE_ALPHA_ARB, 0x3f072d12 },
        { "GL_COMPRESSED_INTENSITY_ARB", GL_COMPRESSED_INTENSITY_ARB, 0xfa3931a7 },
        { "GL_COMPRESSED_RGB_ARB", GL_COMPRESSED_RGB_ARB, 0x92f0e4c7 },
        { "GL_COMPRESSED_RGBA_ARB", GL_COMPRESSED_RGBA_ARB, 0xed8b3e27 }
    };

    static const GLparamdesc compressed_arb_bptc[] =
    {
        { "GL_COMPRESSED_RGBA_BPTC_UNORM_ARB", GL_COMPRESSED_RGBA_BPTC_UNORM_ARB, 0x8d9b8c69 },
        { "GL_COMPRESSED_SRGB_ALPHA_BPTC_UNORM_ARB", GL_COMPRESSED_SRGB_ALPHA_BPTC_UNORM_ARB, 0x21c6fbb1 },
        { "GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT_ARB", GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT_ARB, 0xc667a424 },
        { "GL_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT_ARB", GL_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT_ARB, 0xe4b2fde7 }
    };

    static const GLparamdesc compressed_ext_latc[] =
    {
        { "GL_COMPRESSED_LUMINANCE_LATC1_EXT", GL_COMPRESSED_LUMINANCE_LATC1_EXT, 0x7f53f15d },
        { "GL_COMPRESSED_SIGNED_LUMINANCE_LATC1_EXT", GL_COMPRESSED_SIGNED_LUMINANCE_LATC1_EXT, 0x524881a6 },
        { "GL_COMPRESSED_LUMINANCE_ALPHA_LATC2_EXT", GL_COMPRESSED_LUMINANCE_ALPHA_LATC2_EXT, 0x40d60f4d },
        { "GL_COMPRESSED_SIGNED_LUMINANCE_ALPHA_LATC2_EXT", GL_COMPRESSED_SIGNED_LUMINANCE_ALPHA_LATC2_EXT, 0xe2996e55 }
    };

    static const GLparamdesc compressed_ext_rgtc[] =
    {
        { "GL_COMPRESSED_RED_RGTC1_EXT", GL_COMPRESSED_RED_RGTC1_EXT, 0xdb41c999 },
        { "GL_COMPRESSED_SIGNED_RED_RGTC1_EXT", GL_COMPRESSED_SIGNED_RED_RGTC1_EXT, 0xdc323c6c },
        { "GL_COMPRESSED_RED_GREEN_RGTC2_EXT", GL_COMPRESSED_RED_GREEN_RGTC2_EXT, 0x7f73c6e0 },
        { "GL_COMPRESSED_SIGNED_RED_GREEN_RGTC2_EXT", GL_COMPRESSED_SIGNED_RED_GREEN_RGTC2_EXT, 0x7aeae0f7 }
    };

    static const GLparamdesc compressed_ext_s3tc[] =
    {
        { "GL_COMPRESSED_RGB_S3TC_DXT1_EXT", GL_COMPRESSED_RGB_S3TC_DXT1_EXT, 0x30eb44b4 },
        { "GL_COMPRESSED_RGBA_S3TC_DXT1_EXT", GL_COMPRESSED_RGBA_S3TC_DXT1_EXT, 0xf5697e71 },
        { "GL_COMPRESSED_RGBA_S3TC_DXT3_EXT", GL_COMPRESSED_RGBA_S3TC_DXT3_EXT, 0x8e470e1d },
        { "GL_COMPRESSED_RGBA_S3TC_DXT5_EXT", GL_COMPRESSED_RGBA_S3TC_DXT5_EXT, 0xa111db1c }
    };

    static const GLparamdesc compressed_khr_astc_ldr[] =
    {
        { "GL_COMPRESSED_RGBA_ASTC_4x4_KHR", GL_COMPRESSED_RGBA_ASTC_4x4_KHR, 0x44a11406 },
        { "GL_COMPRESSED_RGBA_ASTC_5x4_KHR", GL_COMPRESSED_RGBA_ASTC_5x4_KHR, 0x8ee1a790 },
        { "GL_COMPRESSED_RGBA_ASTC_5x5_KHR", GL_COMPRESSED_RGBA_ASTC_5x5_KHR, 0xed4e9aea },
        { "GL_COMPRESSED_RGBA_ASTC_6x5_KHR", GL_COMPRESSED_RGBA_ASTC_6x5_KHR, 0x106d493d },
        { "GL_COMPRESSED_RGBA_ASTC_6x6_KHR", GL_COMPRESSED_RGBA_ASTC_6x6_KHR, 0x431e5b55 },
        { "GL_COMPRESSED_RGBA_ASTC_8x5_KHR", GL_COMPRESSED_RGBA_ASTC_8x5_KHR, 0x8c1c593b },
        { "GL_COMPRESSED_RGBA_ASTC_8x6_KHR", GL_COMPRESSED_RGBA_ASTC_8x6_KHR, 0xdf6f4b53 },
        { "GL_COMPRESSED_RGBA_ASTC_8x8_KHR", GL_COMPRESSED_RGBA_ASTC_8x8_KHR, 0xefb36441 },
        { "GL_COMPRESSED_RGBA_ASTC_10x5_KHR", GL_COMPRESSED_RGBA_ASTC_10x5_KHR, 0x17f368da },
        { "GL_COMPRESSED_RGBA_ASTC_10x6_KHR", GL_COMPRESSED_RGBA_ASTC_10x6_KHR, 0x263acab0 },
        { "GL_COMPRESSED_RGBA_ASTC_10x8_KHR", GL_COMPRESSED_RGBA_ASTC_10x8_KHR, 0x6dd1a09c },
        { "GL_COMPRESSED_RGBA_ASTC_10x10_KHR", GL_COMPRESSED_RGBA_ASTC_10x10_KHR, 0x93aad131 },
        { "GL_COMPRESSED_RGBA_ASTC_12x10_KHR", GL_COMPRESSED_RGBA_ASTC_12x10_KHR, 0xe6cf833d },
        { "GL_COMPRESSED_RGBA_ASTC_12x12_KHR", GL_COMPRESSED_RGBA_ASTC_12x12_KHR, 0xcf05b8b5 },
        { "GL_COMPRESSED_SRGB8_ALPHA8_ASTC_4x4_KHR", GL_COMPRESSED_SRGB8_ALPHA8_ASTC_4x4_KHR, 0x87a159dc },
        { "GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x4_KHR", GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x4_KHR, 0x6bf56221 },
        { "GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x5_KHR", GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x5_KHR, 0xa5d0eeff },
        { "GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x5_KHR", GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x5_KHR, 0x43f35255 },
        { "GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x6_KHR", GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x6_KHR, 0xed8c88b1 },
        { "GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x5_KHR", GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x5_KHR, 0x7adb100d },
        { "GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x6_KHR", GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x6_KHR, 0xd4a4cae9 },
        { "GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x8_KHR", GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x8_KHR, 0x45c8be6e },
        { "GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x5_KHR", GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x5_KHR, 0xe460c1b6 },
        { "GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x6_KHR", GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x6_KHR, 0x5e71c971 },
        { "GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x8_KHR", GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x8_KHR, 0x3345ce21 },
        { "GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x10_KHR", GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x10_KHR, 0xe0d1dbd7 },
        { "GL_COMPRESSED_SRGB8_ALPHA8_ASTC_12x10_KHR", GL_COMPRESSED_SRGB8_ALPHA8_ASTC_12x10_KHR, 0xccbc7043 },
        { "GL_COMPRESSED_SRGB8_ALPHA8_ASTC_12x12_KHR", GL_COMPRESSED_SRGB8_ALPHA8_ASTC_12x12_KHR, 0xed8827c6 }
    };

    static const GLparamdesc compressed_srgb[] =
    {
        { "GL_COMPRESSED_SRGB_EXT", GL_COMPRESSED_SRGB_EXT, 0x768ed43e },
        { "GL_COMPRESSED_SRGB_ALPHA_EXT", GL_COMPRESSED_SRGB_ALPHA_EXT, 0xf12c6b6d },
        { "GL_COMPRESSED_SLUMINANCE_EXT", GL_COMPRESSED_SLUMINANCE_EXT, 0x8f0efbda },
        { "GL_COMPRESSED_SLUMINANCE_ALPHA_EXT", GL_COMPRESSED_SLUMINANCE_ALPHA_EXT, 0x8081c7dc },
        { "GL_COMPRESSED_SRGB_S3TC_DXT1_EXT", GL_COMPRESSED_SRGB_S3TC_DXT1_EXT, 0x227e1784 },
        { "GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT", GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT, 0xb8d319bf },
        { "GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT", GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT, 0x69e64d29 },
        { "GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT", GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT, 0x93c46aa3 }
    };

#define PARAM_GET( hash, list ) \
    if ( const GLparamdesc *info = GetParamByHash( list, NUMELMS( list ), hash ) ) \
        return info

    __forceinline const GLparamdesc* GetColorFormatByHash( glDriver *driver, unsigned int hash )
    {
        PARAM_GET( hash, colorInfo );

        if ( driver->vMajor == 1 && driver->vMinor >= 2 || driver->vMajor > 1 )
        {
            PARAM_GET( hash, colorInfo_1_2 );
        }

        return NULL;
    }

    __forceinline const GLparamdesc* GetDepthFormatByHash( glDriver *driver, unsigned int hash )
    {
        PARAM_GET( hash, depthInfo );

        return NULL;
    }

    __forceinline const GLparamdesc* GetStencilFormatByHash( glDriver *driver, unsigned int hash )
    {
        PARAM_GET( hash, stencilInfo );

        if ( driver->vMajor >= 3 )
        {
            PARAM_GET( hash, stencilInfo_3_0 );
        }
        
        return NULL;
    }

    __forceinline const GLparamdesc* GetCompressedFormatByHash( glDriver *driver, unsigned int hash )
    {
        if ( driver->vMajor == 1 && driver->vMinor >= 3 || driver->vMajor > 1 )
        {
            PARAM_GET( hash, compressed_1_3 );
        }

        if ( driver->vMajor == 2 && driver->vMinor >= 1 || driver->vMajor > 2 )
        {
            PARAM_GET( hash, compressed_2_1 );
        }

        if ( driver->vMajor >= 3 )
        {
            PARAM_GET( hash, compressed_3_0 );
        }

        if ( driver->supportsFXT1 )
        {
            PARAM_GET( hash, compressed_3dfx );
        }

        if ( driver->vMajor == 4 && driver->vMinor >= 3 || driver->vMajor > 4 )
        {
            PARAM_GET( hash, compressed_4_3 );
        }

        if ( driver->supports_ARB_compression )
        {
            PARAM_GET( hash, compressed_arb );
        }

        if ( driver->supports_ARB_BPTC )
        {
            PARAM_GET( hash, compressed_arb_bptc );
        }

        if ( driver->supportsLATC )
        {
            PARAM_GET( hash, compressed_ext_latc );
        }

        if ( driver->supportsRGTC )
        {
            PARAM_GET( hash, compressed_ext_rgtc );
        }

        if ( driver->supportsS3TC )
        {
            PARAM_GET( hash, compressed_ext_s3tc );
        }

        if ( driver->supportsASTC )
        {
            PARAM_GET( hash, compressed_khr_astc_ldr );
        }

        if ( driver->supports_srgb_compression )
        {
            PARAM_GET( hash, compressed_srgb );
        }

        return NULL;
    }

    const GLparamdesc* GetParamByName( glDriver *driver, const char *name, size_t nameLen )
    {
        unsigned int hash = TumblerHash( name, nameLen );

        if ( const GLparamdesc *info = GetColorFormatByHash( driver, hash ) )
            return info;

        if ( const GLparamdesc *info = GetDepthFormatByHash( driver, hash ) )
            return info;

        if ( const GLparamdesc *info = GetStencilFormatByHash( driver, hash ) )
            return info;

        if ( const GLparamdesc *info = GetCompressedFormatByHash( driver, hash ) )
            return info;

        // TODO: add more formats.

        return NULL;
    }

    void PushAvailableParams( lua_State *L, glDriver *driver )
    {
        lua_newtable( L );

        unsigned int n = 1;

        // OpenGL basic color formats
        lua_newtable( L );
        lua_pushlstring( L, "GL_RGB", 6 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_RGBA", 7 );
        lua_rawseti( L, -2, n++ );

        // OpenGL basic stencil formats
        lua_pushlstring( L, "GL_DEPTH24_STENCIL8", 19 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_DEPTH32F_STENCIL8", 20 );
        lua_rawseti( L, -2, n++ );

        // OpenGL basic depth formats
        lua_pushlstring( L, "GL_DEPTH_COMPONENT16", 20 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_DEPTH_COMPONENT24", 20 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_DEPTH_COMPONENT32", 20 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_DEPTH_COMPONENT32F", 21 );
        lua_rawseti( L, -2, n++ );

        if ( driver->vMajor == 1 && driver->vMinor >= 2 || driver->vMajor > 1 )
        {
            // OpenGL 1.2 params
            lua_pushlstring( L, "GL_BGRA", 7 );
            lua_rawseti( L, -2, n++ );
            lua_pushlstring( L, "GL_BGR", 6 );
            lua_rawseti( L, -2, n++ );
        }

        if ( driver->vMajor >= 3 )
        {
            // OpenGL 3.0 stencil formats
            lua_pushlstring( L, "GL_STENCIL_INDEX1", 17 );
            lua_rawseti( L, -2, n++ );
            lua_pushlstring( L, "GL_STENCIL_INDEX4", 17 );
            lua_rawseti( L, -2, n++ );
            lua_pushlstring( L, "GL_STENCIL_INDEX8", 17 );
            lua_rawseti( L, -2, n++ );
            lua_pushlstring( L, "GL_STENCIL_INDEX16", 18 );
            lua_rawseti( L, -2, n++ );
        }

        if ( driver->vMajor == 1 && driver->vMinor >= 3 || driver->vMajor > 1 )
        {
            // OpenGL 1.3 compressed formats
            // WARNING: may be deprecated.
            lua_pushlstring( L, "GL_COMPRESSED_RGB", 17 );
            lua_rawseti( L, -2, n++ );
            lua_pushlstring( L, "GL_COMPRESSED_RGBA", 18 );
            lua_rawseti( L, -2, n++ );
            lua_pushlstring( L, "GL_COMPRESSED_ALPHA", 19 );
            lua_rawseti( L, -2, n++ );
            lua_pushlstring( L, "GL_COMPRESSED_LUMINANCE", 23 );
            lua_rawseti( L, -2, n++ );
            lua_pushlstring( L, "GL_COMPRESSED_LUMINANCE_ALPHA", 29 );
            lua_rawseti( L, -2, n++ );
            lua_pushlstring( L, "GL_COMPRESSED_INTENSITY", 23 );
            lua_rawseti( L, -2, n++ );
        }

        if ( driver->vMajor == 2 && driver->vMinor >= 1 || driver->vMajor > 2 )
        {
            // OpenGL 2.1 compressed formats
            lua_pushlstring( L, "GL_COMPRESSED_SRGB", 18 );
            lua_rawseti( L, -2, n++ );
            lua_pushlstring( L, "GL_COMPRESSED_SRGB_ALPHA", 24 );
            lua_rawseti( L, -2, n++ );
            lua_pushlstring( L, "GL_COMPRESSED_SLUMINANCE", 24 );
            lua_rawseti( L, -2, n++ );
            lua_pushlstring( L, "GL_COMPRESSED_SLUMINANCE_ALPHA", 30 );
            lua_rawseti( L, -2, n++ );
        }

        if ( driver->vMajor >= 3 )
        {
            // OpenGL 3.0 compressed formats
            lua_pushlstring( L, "GL_COMPRESSED_RED", 17 );
            lua_rawseti( L, -2, n++ );
            lua_pushlstring( L, "GL_COMPRESSED_RG", 16 );
            lua_rawseti( L, -2, n++ );
            lua_pushlstring( L, "GL_COMPRESSED_RED_RGTC1", 23 );
            lua_rawseti( L, -2, n++ );
            lua_pushlstring( L, "GL_COMPRESSED_SIGNED_RED_RGTC1", 30 );
            lua_rawseti( L, -2, n++ );
            lua_pushlstring( L, "GL_COMPRESSED_RG_RGTC2", 22 );
            lua_rawseti( L, -2, n++ );
            lua_pushlstring( L, "GL_COMPRESSED_SIGNED_RG_RGTC2", 29 );
            lua_rawseti( L, -2, n++ );
        }

        if ( driver->supportsFXT1 )
        {
            // OpenGL GL_3DFX_texture_compression_FXT1 extension
            lua_pushlstring( L, "GL_COMPRESSED_RGB_FXT1_3DFX", 27 );
            lua_rawseti( L, -2, n++ );
            lua_pushlstring( L, "GL_COMPRESSED_RGBA_FXT1_3DFX", 28 );
            lua_rawseti( L, -2, n++ );
        }

        if ( driver->vMajor == 4 && driver->vMinor >= 3 || driver->vMajor > 4 )
        {
            // OpenGL 4.3 compressed formats
            lua_pushlstring( L, "GL_COMPRESSED_RGB8_ETC2", 23 );
            lua_rawseti( L, -2, n++ );
            lua_pushlstring( L, "GL_COMPRESSED_SRGB8_ETC2", 24 );
            lua_rawseti( L, -2, n++ );
            lua_pushlstring( L, "GL_COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2", 43 );
            lua_rawseti( L, -2, n++ );
            lua_pushlstring( L, "GL_COMPRESSED_SRGB8_PUNCHTHROUGH_ALPHA1_ETC2", 44 );
            lua_rawseti( L, -2, n++ );
            lua_pushlstring( L, "GL_COMPRESSED_RGBA8_ETC2_EAC", 28 );
            lua_rawseti( L, -2, n++ );
            lua_pushlstring( L, "GL_COMPRESSED_SRGB8_ALPHA8_ETC2_EAC", 35 );
            lua_rawseti( L, -2, n++ );
            lua_pushlstring( L, "GL_COMPRESSED_R11_EAC", 21 );
            lua_rawseti( L, -2, n++ );
            lua_pushlstring( L, "GL_COMPRESSED_SIGNED_R11_EAC", 28 );
            lua_rawseti( L, -2, n++ );
            lua_pushlstring( L, "GL_COMPRESSED_RG11_EAC", 22 );
            lua_rawseti( L, -2, n++ );
            lua_pushlstring( L, "GL_COMPRESSED_SIGNED_RG11_EAC", 29 );
            lua_rawseti( L, -2, n++ );
        }

        if ( driver->supports_ARB_compression )
        {
            // OpenGL GL_ARB_texture_compression extension
            lua_pushlstring( L, "GL_COMPRESSED_ALPHA_ARB", 23 );
            lua_rawseti( L, -2, n++ );
            lua_pushlstring( L, "GL_COMPRESSED_LUMINANCE_ARB", 27 );
            lua_rawseti( L, -2, n++ );
            lua_pushlstring( L, "GL_COMPRESSED_LUMINANCE_ALPHA_ARB", 33 );
            lua_rawseti( L, -2, n++ );
            lua_pushlstring( L, "GL_COMPRESSED_INTENSITY_ARB", 27 );
            lua_rawseti( L, -2, n++ );
            lua_pushlstring( L, "GL_COMPRESSED_RGB_ARB", 21 );
            lua_rawseti( L, -2, n++ );
            lua_pushlstring( L, "GL_COMPRESSED_RGBA_ARB", 22 );
            lua_rawseti( L, -2, n++ );
        }

        if ( driver->supports_ARB_BPTC )
        {
            // OpenGL GL_ARB_texture_compression_bptc extension
            lua_pushlstring( L, "GL_COMPRESSED_RGBA_BPTC_UNORM_ARB", 33 );
            lua_rawseti( L, -2, n++ );
            lua_pushlstring( L, "GL_COMPRESSED_SRGB_ALPHA_BPTC_UNORM_ARB", 39 );
            lua_rawseti( L, -2, n++ );
            lua_pushlstring( L, "GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT_ARB", 39 );
            lua_rawseti( L, -2, n++ );
            lua_pushlstring( L, "GL_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT_ARB", 41 );
            lua_rawseti( L, -2, n++ );
        }

        if ( driver->supportsLATC )
        {
            // OpenGL GL_EXT_texture_compression_latc extension
            lua_pushlstring( L, "GL_COMPRESSED_LUMINANCE_LATC1_EXT", 33 );
            lua_rawseti( L, -2, n++ );
            lua_pushlstring( L, "GL_COMPRESSED_SIGNED_LUMINANCE_LATC1_EXT", 40 );
            lua_rawseti( L, -2, n++ );
            lua_pushlstring( L, "GL_COMPRESSED_LUMINANCE_ALPHA_LATC2_EXT", 39 );
            lua_rawseti( L, -2, n++ );
            lua_pushlstring( L, "GL_COMPRESSED_SIGNED_LUMINANCE_ALPHA_LATC2_EXT", 46 );
            lua_rawseti( L, -2, n++ );
        }

        if ( driver->supportsRGTC )
        {
            // OpenGL GL_EXT_texture_compression_rgtc extension
            lua_pushlstring( L, "GL_COMPRESSED_RED_RGTC1_EXT", 27 );
            lua_rawseti( L, -2, n++ );
            lua_pushlstring( L, "GL_COMPRESSED_SIGNED_RED_RGTC1_EXT", 34 );
            lua_rawseti( L, -2, n++ );
            lua_pushlstring( L, "GL_COMPRESSED_RED_GREEN_RGTC2_EXT", 33 );
            lua_rawseti( L, -2, n++ );
            lua_pushlstring( L, "GL_COMPRESSED_SIGNED_RED_GREEN_RGTC2_EXT", 40 );
            lua_rawseti( L, -2, n++ );
        }

        if ( driver->supportsS3TC )
        {
            // OpenGL GL_EXT_texture_compression_s3tc extension
            lua_pushlstring( L, "GL_COMPRESSED_RGB_S3TC_DXT1_EXT", 31 );
            lua_rawseti( L, -2, n++ );
            lua_pushlstring( L, "GL_COMPRESSED_RGBA_S3TC_DXT1_EXT", 32 );
            lua_rawseti( L, -2, n++ );
            lua_pushlstring( L, "GL_COMPRESSED_RGBA_S3TC_DXT3_EXT", 32 );
            lua_rawseti( L, -2, n++ );
            lua_pushlstring( L, "GL_COMPRESSED_RGBA_S3TC_DXT5_EXT", 32 );
            lua_rawseti( L, -2, n++ );
        }

        if ( driver->supportsASTC )
        {
            // OpenGL GL_KHR_texture_compression_astc_ldr extension
            lua_pushlstring( L, "GL_COMPRESSED_RGBA_ASTC_4x4_KHR", 31 );
            lua_rawseti( L, -2, n++ );
            lua_pushlstring( L, "GL_COMPRESSED_RGBA_ASTC_5x4_KHR", 31 );
            lua_rawseti( L, -2, n++ );
            lua_pushlstring( L, "GL_COMPRESSED_RGBA_ASTC_5x5_KHR", 31 );
            lua_rawseti( L, -2, n++ );
            lua_pushlstring( L, "GL_COMPRESSED_RGBA_ASTC_6x5_KHR", 31 );
            lua_rawseti( L, -2, n++ );
            lua_pushlstring( L, "GL_COMPRESSED_RGBA_ASTC_6x6_KHR", 31 );
            lua_rawseti( L, -2, n++ );
            lua_pushlstring( L, "GL_COMPRESSED_RGBA_ASTC_8x5_KHR", 31 );
            lua_rawseti( L, -2, n++ );
            lua_pushlstring( L, "GL_COMPRESSED_RGBA_ASTC_8x6_KHR", 31 );
            lua_rawseti( L, -2, n++ );
            lua_pushlstring( L, "GL_COMPRESSED_RGBA_ASTC_8x8_KHR", 31 );
            lua_rawseti( L, -2, n++ );
            lua_pushlstring( L, "GL_COMPRESSED_RGBA_ASTC_10x5_KHR", 32 );
            lua_rawseti( L, -2, n++ );
            lua_pushlstring( L, "GL_COMPRESSED_RGBA_ASTC_10x6_KHR", 32 );
            lua_rawseti( L, -2, n++ );
            lua_pushlstring( L, "GL_COMPRESSED_RGBA_ASTC_10x8_KHR", 32 );
            lua_rawseti( L, -2, n++ );
            lua_pushlstring( L, "GL_COMPRESSED_RGBA_ASTC_10x10_KHR", 33 );
            lua_rawseti( L, -2, n++ );
            lua_pushlstring( L, "GL_COMPRESSED_RGBA_ASTC_12x10_KHR", 33 );
            lua_rawseti( L, -2, n++ );
            lua_pushlstring( L, "GL_COMPRESSED_RGBA_ASTC_12x12_KHR", 33 );
            lua_rawseti( L, -2, n++ );
            lua_pushlstring( L, "GL_COMPRESSED_SRGB8_ALPHA8_ASTC_4x4_KHR", 39 );
            lua_rawseti( L, -2, n++ );
            lua_pushlstring( L, "GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x4_KHR", 39 );
            lua_rawseti( L, -2, n++ );
            lua_pushlstring( L, "GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x5_KHR", 39 );
            lua_rawseti( L, -2, n++ );
            lua_pushlstring( L, "GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x5_KHR", 39 );
            lua_rawseti( L, -2, n++ );
            lua_pushlstring( L, "GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x6_KHR", 39 );
            lua_rawseti( L, -2, n++ );
            lua_pushlstring( L, "GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x5_KHR", 39 );
            lua_rawseti( L, -2, n++ );
            lua_pushlstring( L, "GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x6_KHR", 39 );
            lua_rawseti( L, -2, n++ );
            lua_pushlstring( L, "GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x8_KHR", 39 );
            lua_rawseti( L, -2, n++ );
            lua_pushlstring( L, "GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x5_KHR", 40 );
            lua_rawseti( L, -2, n++ );
            lua_pushlstring( L, "GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x6_KHR", 40 );
            lua_rawseti( L, -2, n++ );
            lua_pushlstring( L, "GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x8_KHR", 40 );
            lua_rawseti( L, -2, n++ );
            lua_pushlstring( L, "GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x10_KHR", 41 );
            lua_rawseti( L, -2, n++ );
            lua_pushlstring( L, "GL_COMPRESSED_SRGB8_ALPHA8_ASTC_12x10_KHR", 41 );
            lua_rawseti( L, -2, n++ );
            lua_pushlstring( L, "GL_COMPRESSED_SRGB8_ALPHA8_ASTC_12x12_KHR", 41 );
            lua_rawseti( L, -2, n++ );
        }

        if ( driver->supports_srgb_compression )
        {
            // OpenGL GL_EXT_texture_sRGB extension
            lua_pushlstring( L, "GL_COMPRESSED_SRGB_EXT", 22 );
            lua_rawseti( L, -2, n++ );
            lua_pushlstring( L, "GL_COMPRESSED_SRGB_ALPHA_EXT", 28 );
            lua_rawseti( L, -2, n++ );
            lua_pushlstring( L, "GL_COMPRESSED_SLUMINANCE_EXT", 28 );
            lua_rawseti( L, -2, n++ );
            lua_pushlstring( L, "GL_COMPRESSED_SLUMINANCE_ALPHA_EXT", 34 );
            lua_rawseti( L, -2, n++ );
            lua_pushlstring( L, "GL_COMPRESSED_SRGB_S3TC_DXT1_EXT", 32 );
            lua_rawseti( L, -2, n++ );
            lua_pushlstring( L, "GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT", 38 );
            lua_rawseti( L, -2, n++ );
            lua_pushlstring( L, "GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT", 38 );
            lua_rawseti( L, -2, n++ );
            lua_pushlstring( L, "GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT", 38 );
            lua_rawseti( L, -2, n++ );
        }
    }
};

namespace gl_blendfunc
{
    static const GLparamdesc blendFunc[] =
    {
        { "GL_ZERO", GL_ZERO, 0xede10434 },
        { "GL_ONE", GL_ONE, 0xeda23afe },
        { "GL_SRC_COLOR", GL_SRC_COLOR, 0xb9df48a3 },
        { "GL_ONE_MINUS_SRC_COLOR", GL_ONE_MINUS_SRC_COLOR, 0xfb4007de },
        { "GL_SRC_ALPHA", GL_SRC_ALPHA, 0xe79b085f },
        { "GL_ONE_MINUS_SRC_ALPHA", GL_ONE_MINUS_SRC_ALPHA, 0x438c2a91 },
        { "GL_DST_ALPHA", GL_DST_ALPHA, 0xc2810613 },
        { "GL_ONE_MINUS_DST_ALPHA", GL_ONE_MINUS_DST_ALPHA, 0x68690ca0 },
        { "GL_DST_COLOR", GL_DST_COLOR, 0x9cc546ef },
        { "GL_ONE_MINUS_DST_COLOR", GL_ONE_MINUS_DST_COLOR, 0xd0a521ef },
        { "GL_SRC_ALPHA_SATURATE", GL_SRC_ALPHA_SATURATE, 0x6e1e27da }
    };

    static const GLparamdesc blendFunc_1_4[] =
    {
        { "GL_CONSTANT_COLOR", GL_CONSTANT_COLOR, 0x92ec5c1a },
        { "GL_ONE_MINUS_CONSTANT_COLOR", GL_ONE_MINUS_CONSTANT_COLOR, 0xaaf51d69 },
        { "GL_CONSTANT_ALPHA", GL_CONSTANT_ALPHA, 0xcfa54780 },
        { "GL_ONE_MINUS_CONSTANT_ALPHA", GL_ONE_MINUS_CONSTANT_ALPHA, 0xb3a7c576 }
    };

    const GLparamdesc* GetParamByName( const glDriver *driver, const char *name, size_t nameLen )
    {
        unsigned int hash = TumblerHash( name, nameLen );

        PARAM_GET( hash, blendFunc );

        if ( ( driver->vMajor == 1 && driver->vMinor >= 4 || driver->vMajor > 1 ) || driver->supports_ARB_imaging )
        {
            PARAM_GET( hash, blendFunc_1_4 );
        }

        return NULL;
    }
};