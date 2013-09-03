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

/*
namespace gl_template
{
    const GLparamdesc* GetParamByName( const glDriver *driver, const char *name, size_t nameLen )
    {
        unsigned int hash = TumblerHash( name, nameLen );



        return NULL;
    }

    void PushAvailableParams( lua_State *L, const glDriver *driver )
    {
        unsigned int n = 1;


    }
};
*/

template <typename list_type>
__forceinline static const list_type* GetParamByHash( const list_type *list, size_t len, unsigned int hash )
{
    for ( unsigned int n = 0; n < len; n++ )
    {
        const list_type *item = list + n;

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

    void PushAvailableParams( lua_State *L, const glDriver *driver )
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

namespace gl_parameters
{
    static const GLparaminfo param_list[] =
    {
        { "GL_ACCUM_ALPHA_BITS", GL_ACCUM_ALPHA_BITS, 0x89c14d99, GLPARAM_INT, 1 },
        { "GL_ACCUM_BLUE_BITS", GL_ACCUM_BLUE_BITS, 0xe242e280, GLPARAM_INT, 1 },
        { "GL_ACCUM_CLEAR_VALUE", GL_ACCUM_CLEAR_VALUE, 0xb8f11f19, GLPARAM_DOUBLE, 4 },
        { "GL_ACCUM_GREEN_BITS", GL_ACCUM_GREEN_BITS, 0x49fb127f, GLPARAM_INT, 1 },
        { "GL_ACCUM_RED_BITS", GL_ACCUM_RED_BITS, 0xd8bca946, GLPARAM_INT, 1 },
        { "GL_ALPHA_BIAS", GL_ALPHA_BIAS, 0x48fadc31, GLPARAM_DOUBLE, 1 },
        { "GL_ALPHA_BITS", GL_ALPHA_BITS, 0x87b83b37, GLPARAM_INT, 1 },
        { "GL_ALPHA_SCALE", GL_ALPHA_SCALE, 0x728a545, GLPARAM_DOUBLE, 1 },
        { "GL_ALPHA_TEST", GL_ALPHA_TEST, 0xaae45f62, GLPARAM_BOOLEAN, 1 },
        { "GL_ALPHA_TEST_FUNC", GL_ALPHA_TEST_FUNC, 0x6692ead0, GLPARAM_INT, 1 },
        { "GL_ALPHA_TEST_REF", GL_ALPHA_TEST_REF, 0x36d25f32, GLPARAM_DOUBLE, 1 },
        { "GL_ATTRIB_STACK_DEPTH", GL_ATTRIB_STACK_DEPTH, 0xaf899468, GLPARAM_INT, 1 },
        { "GL_AUTO_NORMAL", GL_AUTO_NORMAL, 0x712edb6b, GLPARAM_BOOLEAN, 1 },
        { "GL_AUX_BUFFERS", GL_AUX_BUFFERS, 0x74082d22, GLPARAM_INT, 1 },
        { "GL_BLEND", GL_BLEND, 0xa40557e7, GLPARAM_BOOLEAN, 1 },
        { "GL_BLEND_DST", GL_BLEND_DST, 0x82aa14cc, GLPARAM_INT, 1 },
        { "GL_BLEND_SRC", GL_BLEND_SRC, 0xb7e64413, GLPARAM_INT, 1 },
        { "GL_BLUE_BIAS", GL_BLUE_BIAS, 0x5e25fd76, GLPARAM_DOUBLE, 1 },
        { "GL_BLUE_BITS", GL_BLUE_BITS, 0xa260dbd6, GLPARAM_INT, 1 },
        { "GL_BLUE_SCALE", GL_BLUE_SCALE, 0x285c90a7, GLPARAM_DOUBLE, 1 },
        { "GL_CLIENT_ATTRIB_STACK_DEPTH", GL_CLIENT_ATTRIB_STACK_DEPTH, 0x11c49981, GLPARAM_INT, 1 },
        { "GL_CLIP_PLANE0", GL_CLIP_PLANE0, 0x848c8631, GLPARAM_BOOLEAN, 1 },
        { "GL_CLIP_PLANE1", GL_CLIP_PLANE1, 0x6ccbd80b, GLPARAM_BOOLEAN, 1 },
        { "GL_CLIP_PLANE2", GL_CLIP_PLANE2, 0x259843b6, GLPARAM_BOOLEAN, 1 },
        { "GL_CLIP_PLANE3", GL_CLIP_PLANE3, 0x84c3f94b, GLPARAM_BOOLEAN, 1 },
        { "GL_CLIP_PLANE4", GL_CLIP_PLANE4, 0x8ddcb267, GLPARAM_BOOLEAN, 1 },
        { "GL_CLIP_PLANE5", GL_CLIP_PLANE5, 0xe40d4d43, GLPARAM_BOOLEAN, 1 },
        { "GL_COLOR_ARRAY", GL_COLOR_ARRAY, 0x63db86ea, GLPARAM_BOOLEAN, 1 },
        { "GL_COLOR_ARRAY_SIZE", GL_COLOR_ARRAY_SIZE, 0x2547efe, GLPARAM_INT, 1 },
        { "GL_COLOR_ARRAY_STRIDE", GL_COLOR_ARRAY_STRIDE, 0xd8087886, GLPARAM_INT, 1 },
        { "GL_COLOR_ARRAY_TYPE", GL_COLOR_ARRAY_TYPE, 0x5550b37d, GLPARAM_INT, 1 },
        { "GL_COLOR_CLEAR_VALUE", GL_COLOR_CLEAR_VALUE, 0x298a2e5c, GLPARAM_DOUBLE, 4 },
        { "GL_COLOR_LOGIC_OP", GL_COLOR_LOGIC_OP, 0x8b172a39, GLPARAM_BOOLEAN, 1 },
        { "GL_COLOR_MATERIAL", GL_COLOR_MATERIAL, 0xe442a42c, GLPARAM_BOOLEAN, 1 },
        { "GL_COLOR_MATERIAL_FACE", GL_COLOR_MATERIAL_FACE, 0x2ec87008, GLPARAM_INT, 1 },
        { "GL_COLOR_MATERIAL_PARAMETER", GL_COLOR_MATERIAL_PARAMETER, 0x4bf52092, GLPARAM_INT, 1 },
        { "GL_COLOR_WRITEMASK", GL_COLOR_WRITEMASK, 0x4cbef082, GLPARAM_BOOLEAN, 4 },
        { "GL_CULL_FACE", GL_CULL_FACE, 0x81bbd421, GLPARAM_BOOLEAN, 1 },
        { "GL_CULL_FACE_MODE", GL_CULL_FACE_MODE, 0xacf6457d, GLPARAM_INT, 1 },
        { "GL_CURRENT_COLOR", GL_CURRENT_COLOR, 0x4142f6c4, GLPARAM_DOUBLE, 4 },
        { "GL_CURRENT_INDEX", GL_CURRENT_INDEX, 0x36214fa6, GLPARAM_INT, 1 },
        { "GL_CURRENT_NORMAL", GL_CURRENT_NORMAL, 0x6fe02413, GLPARAM_DOUBLE, 3 },
        { "GL_CURRENT_RASTER_COLOR", GL_CURRENT_RASTER_COLOR, 0x6eeb92ba, GLPARAM_DOUBLE, 4 },
        { "GL_CURRENT_RASTER_DISTANCE", GL_CURRENT_RASTER_DISTANCE, 0xb89b9d4, GLPARAM_DOUBLE, 1 },
        { "GL_CURRENT_RASTER_INDEX", GL_CURRENT_RASTER_INDEX, 0x8b0a39c1, GLPARAM_INT, 1 },
        { "GL_CURRENT_RASTER_POSITION", GL_CURRENT_RASTER_POSITION, 0x8e81306f, GLPARAM_DOUBLE, 4 },
        { "GL_CURRENT_RASTER_POSITION_VALID", GL_CURRENT_RASTER_POSITION_VALID, 0xf898f29f, GLPARAM_BOOLEAN, 1 },
        { "GL_CURRENT_RASTER_TEXTURE_COORDS", GL_CURRENT_RASTER_TEXTURE_COORDS, 0xbcee654a, GLPARAM_DOUBLE, 4 },
        { "GL_CURRENT_TEXTURE_COORDS", GL_CURRENT_TEXTURE_COORDS, 0x4d83f9d7, GLPARAM_DOUBLE, 4 },
        { "GL_DEPTH_BIAS", GL_DEPTH_BIAS, 0xdbb44bb7, GLPARAM_DOUBLE, 1 },
        { "GL_DEPTH_BITS", GL_DEPTH_BITS, 0x14f6acb1, GLPARAM_INT, 1 },
        { "GL_DEPTH_CLEAR_VALUE", GL_DEPTH_CLEAR_VALUE, 0x1808fd23, GLPARAM_DOUBLE, 1 },
        { "GL_DEPTH_FUNC", GL_DEPTH_FUNC, 0x57ac404a, GLPARAM_INT, 1 },
        { "GL_DEPTH_RANGE", GL_DEPTH_RANGE, 0xc98da9b3, GLPARAM_DOUBLE, 2 },
        { "GL_DEPTH_SCALE", GL_DEPTH_SCALE, 0xa1b465bb, GLPARAM_DOUBLE, 1 },
        { "GL_DEPTH_TEST", GL_DEPTH_TEST, 0x39aac8e4, GLPARAM_BOOLEAN, 1 },
        { "GL_DEPTH_WRITEMASK", GL_DEPTH_WRITEMASK, 0xd8ff35ec, GLPARAM_BOOLEAN, 1 },
        { "GL_DITHER", GL_DITHER, 0x128ad5d0, GLPARAM_BOOLEAN, 1 },
        { "GL_DOUBLEBUFFER", GL_DOUBLEBUFFER, 0x4d6d04de, GLPARAM_BOOLEAN, 1 },
        { "GL_DRAW_BUFFER", GL_DRAW_BUFFER, 0xda9a3bff, GLPARAM_INT, 1 },
        { "GL_EDGE_FLAG", GL_EDGE_FLAG, 0x38d4116d, GLPARAM_BOOLEAN, 1 },
        { "GL_EDGE_FLAG_ARRAY", GL_EDGE_FLAG_ARRAY, 0x49521ab9, GLPARAM_BOOLEAN, 1 },
        { "GL_EDGE_FLAG_ARRAY_STRIDE", GL_EDGE_FLAG_ARRAY_STRIDE, 0x69d56c52, GLPARAM_INT, 1 },
        { "GL_FOG", GL_FOG, 0x8ed3f4e8, GLPARAM_BOOLEAN, 1 },
        { "GL_FOG_COLOR", GL_FOG_COLOR, 0x8f34178b, GLPARAM_DOUBLE, 4 },
        { "GL_FOG_DENSITY", GL_FOG_DENSITY, 0x448e4245, GLPARAM_DOUBLE, 1 },
        { "GL_FOG_END", GL_FOG_END, 0x4fcbcea4, GLPARAM_DOUBLE, 1 },
        { "GL_FOG_HINT", GL_FOG_HINT, 0xbcc5dff5, GLPARAM_INT, 1 },
        { "GL_FOG_INDEX", GL_FOG_INDEX, 0x7667f006, GLPARAM_INT, 1 },
        { "GL_FOG_MODE", GL_FOG_MODE, 0x3894ebad, GLPARAM_INT, 1 },
        { "GL_FOG_START", GL_FOG_START, 0xe503c09f, GLPARAM_DOUBLE, 1 },
        { "GL_FRONT_FACE", GL_FRONT_FACE, 0xed1ec541, GLPARAM_INT, 1 },
        { "GL_GREEN_BIAS", GL_GREEN_BIAS, 0xf5d701b6, GLPARAM_DOUBLE, 1 },
        { "GL_GREEN_BITS", GL_GREEN_BITS, 0x3a95e6b0, GLPARAM_INT, 1 },
        { "GL_GREEN_SCALE", GL_GREEN_SCALE, 0xab7c594, GLPARAM_DOUBLE, 1 },
        { "GL_INDEX_ARRAY", GL_INDEX_ARRAY, 0x79cff321, GLPARAM_BOOLEAN, 1 },
        { "GL_INDEX_ARRAY_STRIDE", GL_INDEX_ARRAY_STRIDE, 0xce9c6c10, GLPARAM_INT, 1 },
        { "GL_INDEX_ARRAY_TYPE", GL_INDEX_ARRAY_TYPE, 0x41263a30, GLPARAM_INT, 1 },
        { "GL_INDEX_BITS", GL_INDEX_BITS, 0x4dfbf162, GLPARAM_INT, 1 },
        { "GL_INDEX_CLEAR_VALUE", GL_INDEX_CLEAR_VALUE, 0x40a66a49, GLPARAM_DOUBLE, 1 },
        { "GL_INDEX_LOGIC_OP", GL_INDEX_LOGIC_OP, 0x9bd85422, GLPARAM_BOOLEAN, 1 },
        { "GL_INDEX_MODE", GL_INDEX_MODE, 0xc1a16288, GLPARAM_BOOLEAN, 1 },
        { "GL_INDEX_OFFSET", GL_INDEX_OFFSET, 0x421ff2ca, GLPARAM_INT, 1 },
        { "GL_INDEX_SHIFT", GL_INDEX_SHIFT, 0x3bc532f2, GLPARAM_INT, 1 },
        { "GL_INDEX_WRITEMASK", GL_INDEX_WRITEMASK, 0x9be7458b, GLPARAM_INT, 1 },
        { "GL_LIGHT0", GL_LIGHT0, 0x5bda68ee, GLPARAM_BOOLEAN, 1 },
        { "GL_LIGHT1", GL_LIGHT1, 0x1b886cd2, GLPARAM_BOOLEAN, 1 },
        { "GL_LIGHT2", GL_LIGHT2, 0x7ecb116a, GLPARAM_BOOLEAN, 1 },
        { "GL_LIGHT3", GL_LIGHT3, 0xfaabaa6f, GLPARAM_BOOLEAN, 1 },
        { "GL_LIGHT4", GL_LIGHT4, 0xbb9130b9, GLPARAM_BOOLEAN, 1 },
        { "GL_LIGHT5", GL_LIGHT5, 0x532a2f97, GLPARAM_BOOLEAN, 1 },
        { "GL_LIGHT6", GL_LIGHT6, 0x1f8afeb3, GLPARAM_BOOLEAN, 1 },
        { "GL_LIGHT7", GL_LIGHT7, 0x146d8bd2, GLPARAM_BOOLEAN, 1 },
        { "GL_LIGHTING", GL_LIGHTING, 0x59667ac0, GLPARAM_BOOLEAN, 1 },
        { "GL_LIGHT_MODEL_AMBIENT", GL_LIGHT_MODEL_AMBIENT, 0x2e1ea6d5, GLPARAM_DOUBLE, 4 },
        { "GL_LIGHT_MODEL_LOCAL_VIEWER", GL_LIGHT_MODEL_LOCAL_VIEWER, 0x30df85bb, GLPARAM_BOOLEAN, 1 },
        { "GL_LIGHT_MODEL_TWO_SIDE", GL_LIGHT_MODEL_TWO_SIDE, 0x9949a130, GLPARAM_BOOLEAN, 1 },
        { "GL_LINE_SMOOTH", GL_LINE_SMOOTH, 0xb50bf1e5, GLPARAM_BOOLEAN, 1 },
        { "GL_LINE_SMOOTH_HINT", GL_LINE_SMOOTH_HINT, 0x3220d2a9, GLPARAM_INT, 1 },
        { "GL_LINE_STIPPLE", GL_LINE_STIPPLE, 0x7c77b2ea, GLPARAM_BOOLEAN, 1 },
        { "GL_LINE_STIPPLE_PATTERN", GL_LINE_STIPPLE_PATTERN, 0x26f5a1de, GLPARAM_INT, 1 },
        { "GL_LINE_STIPPLE_REPEAT", GL_LINE_STIPPLE_REPEAT, 0x45972ff1, GLPARAM_DOUBLE, 1 },
        { "GL_LINE_WIDTH", GL_LINE_WIDTH, 0x4ecaa0ff, GLPARAM_DOUBLE, 1 },
        { "GL_LINE_WIDTH_GRANULARITY", GL_LINE_WIDTH_GRANULARITY, 0xe50132d9, GLPARAM_DOUBLE, 1 },
        { "GL_LINE_WIDTH_RANGE", GL_LINE_WIDTH_RANGE, 0x70a1427b, GLPARAM_DOUBLE, 2 },
        { "GL_LIST_BASE", GL_LIST_BASE, 0x82fc8aab, GLPARAM_INT, 1 },
        { "GL_LIST_INDEX", GL_LIST_INDEX, 0xdecfce14, GLPARAM_INT, 1 },
        { "GL_LIST_MODE", GL_LIST_MODE, 0x8ccb0bf7, GLPARAM_INT, 1 },
        { "GL_LOGIC_OP", GL_LOGIC_OP, 0xaae3bb9a, GLPARAM_BOOLEAN, 1 },
        { "GL_LOGIC_OP_MODE", GL_LOGIC_OP_MODE, 0x2af52693, GLPARAM_INT, 1 },
        { "GL_MAP1_COLOR_4", GL_MAP1_COLOR_4, 0x384eb930, GLPARAM_BOOLEAN, 1 },
        { "GL_MAP1_GRID_DOMAIN", GL_MAP1_GRID_DOMAIN, 0xe79010ad, GLPARAM_DOUBLE, 2 },
        { "GL_MAP1_GRID_SEGMENTS", GL_MAP1_GRID_SEGMENTS, 0x7de39797, GLPARAM_INT, 1 },
        { "GL_MAP1_INDEX", GL_MAP1_INDEX, 0xa09ff53f, GLPARAM_INT, 1 },
        { "GL_MAP1_NORMAL", GL_MAP1_NORMAL, 0xa6ab1a46, GLPARAM_BOOLEAN, 1 },
        { "GL_MAP1_TEXTURE_COORD_1", GL_MAP1_TEXTURE_COORD_1, 0x151d1a8b, GLPARAM_BOOLEAN, 1 },
        { "GL_MAP1_TEXTURE_COORD_2", GL_MAP1_TEXTURE_COORD_2, 0x413aac21, GLPARAM_BOOLEAN, 1 },
        { "GL_MAP1_TEXTURE_COORD_3", GL_MAP1_TEXTURE_COORD_3, 0x913d7ca6, GLPARAM_BOOLEAN, 1 },
        { "GL_MAP1_TEXTURE_COORD_4", GL_MAP1_TEXTURE_COORD_4, 0xc40342ac, GLPARAM_BOOLEAN, 1 },
        { "GL_MAP1_VERTEX_3", GL_MAP1_VERTEX_3, 0x55c3f9a5, GLPARAM_BOOLEAN, 1 },
        { "GL_MAP1_VERTEX_4", GL_MAP1_VERTEX_4, 0x5f8980f3, GLPARAM_BOOLEAN, 1 },
        { "GL_MAP2_COLOR_4", GL_MAP2_COLOR_4, 0xb288454, GLPARAM_BOOLEAN, 1 },
        { "GL_MAP2_GRID_DOMAIN", GL_MAP2_GRID_DOMAIN, 0xbcd336fa, GLPARAM_DOUBLE, 2 },
        { "GL_MAP2_GRID_SEGMENTS", GL_MAP2_GRID_SEGMENTS, 0x479a7c52, GLPARAM_INT, 1 },
        { "GL_MAP2_INDEX", GL_MAP2_INDEX, 0x8ff86285, GLPARAM_INT, 1 },
        { "GL_MAP2_NORMAL", GL_MAP2_NORMAL, 0x51fcdb15, GLPARAM_BOOLEAN, 1 },
        { "GL_MAP2_TEXTURE_COORD_1", GL_MAP2_TEXTURE_COORD_1, 0x1b2e6191, GLPARAM_BOOLEAN, 1 },
        { "GL_MAP2_TEXTURE_COORD_2", GL_MAP2_TEXTURE_COORD_2, 0x4f09d73b, GLPARAM_BOOLEAN, 1 },
        { "GL_MAP2_TEXTURE_COORD_3", GL_MAP2_TEXTURE_COORD_3, 0x9f0e07bc, GLPARAM_BOOLEAN, 1 },
        { "GL_MAP2_TEXTURE_COORD_4", GL_MAP2_TEXTURE_COORD_4, 0xca3039b6, GLPARAM_BOOLEAN, 1 },
        { "GL_MAP2_VERTEX_3", GL_MAP2_VERTEX_3, 0x87e0f262, GLPARAM_BOOLEAN, 1 },
        { "GL_MAP2_VERTEX_4", GL_MAP2_VERTEX_4, 0x8daa8b34, GLPARAM_BOOLEAN, 1 },
        { "GL_MAP_COLOR", GL_MAP_COLOR, 0xf7f71c1c, GLPARAM_BOOLEAN, 1 },
        { "GL_MAP_STENCIL", GL_MAP_STENCIL, 0x150d4500, GLPARAM_BOOLEAN, 1 },
        { "GL_MATRIX_MODE", GL_MATRIX_MODE, 0x5fca7fa2, GLPARAM_INT, 1 },
        { "GL_MAX_CLIENT_ATTRIB_STACK_DEPTH", GL_MAX_CLIENT_ATTRIB_STACK_DEPTH, 0xc1318221, GLPARAM_INT, 1 },
        { "GL_MAX_ATTRIB_STACK_DEPTH", GL_MAX_ATTRIB_STACK_DEPTH, 0x2d2a8235, GLPARAM_INT, 1 },
        { "GL_MAX_CLIP_PLANES", GL_MAX_CLIP_PLANES, 0x32524b18, GLPARAM_INT, 1 },
        { "GL_MAX_EVAL_ORDER", GL_MAX_EVAL_ORDER, 0x5558be77, GLPARAM_INT, 1 },
        { "GL_MAX_LIGHTS", GL_MAX_LIGHTS, 0x31fc482c, GLPARAM_INT, 1 },
        { "GL_MAX_LIST_NESTING", GL_MAX_LIST_NESTING, 0xbae8bec1, GLPARAM_INT, 1 },
        { "GL_MAX_MODELVIEW_STACK_DEPTH", GL_MAX_MODELVIEW_STACK_DEPTH, 0xbaca15ce, GLPARAM_INT, 1 },
        { "GL_MAX_NAME_STACK_DEPTH", GL_MAX_NAME_STACK_DEPTH, 0x885a40e9, GLPARAM_INT, 1 },
        { "GL_MAX_PIXEL_MAP_TABLE", GL_MAX_PIXEL_MAP_TABLE, 0x2713c350, GLPARAM_INT, 1 },
        { "GL_MAX_PROJECTION_STACK_DEPTH", GL_MAX_PROJECTION_STACK_DEPTH, 0xb6ebc69b, GLPARAM_INT, 1 },
        { "GL_MAX_TEXTURE_SIZE", GL_MAX_TEXTURE_SIZE, 0x4da60713, GLPARAM_INT, 1 },
        { "GL_MAX_TEXTURE_STACK_DEPTH", GL_MAX_TEXTURE_STACK_DEPTH, 0x22a22646, GLPARAM_INT, 1 },
        { "GL_MAX_VIEWPORT_DIMS", GL_MAX_VIEWPORT_DIMS, 0x1fd44300, GLPARAM_INT, 2 },
        { "GL_MODELVIEW_MATRIX", GL_MODELVIEW_MATRIX, 0x28ae2c7, GLPARAM_DOUBLE, 16 },
        { "GL_MODELVIEW_STACK_DEPTH", GL_MODELVIEW_STACK_DEPTH, 0xe8ebedfc, GLPARAM_INT, 1 },
        { "GL_NAME_STACK_DEPTH", GL_NAME_STACK_DEPTH, 0x696ab135, GLPARAM_INT, 1 },
        { "GL_NORMAL_ARRAY", GL_NORMAL_ARRAY, 0xca3c3867, GLPARAM_BOOLEAN, 1 },
        { "GL_NORMAL_ARRAY_STRIDE", GL_NORMAL_ARRAY_STRIDE, 0xd501e62, GLPARAM_INT, 1 },
        { "GL_NORMAL_ARRAY_TYPE", GL_NORMAL_ARRAY_TYPE, 0x18b5baa7, GLPARAM_INT, 1 },
        { "GL_NORMALIZE", GL_NORMALIZE, 0xea0eb318, GLPARAM_BOOLEAN, 1 },
        { "GL_PACK_ALIGNMENT", GL_PACK_ALIGNMENT, 0xcc1a0925, GLPARAM_INT, 1 },
        { "GL_PACK_LSB_FIRST", GL_PACK_LSB_FIRST, 0x19c6b7c, GLPARAM_BOOLEAN, 1 },
        { "GL_PACK_ROW_LENGTH", GL_PACK_ROW_LENGTH, 0x9f7e0890, GLPARAM_INT, 1 },
        { "GL_PACK_SKIP_PIXELS", GL_PACK_SKIP_PIXELS, 0x1610580, GLPARAM_INT, 1 },
        { "GL_PACK_SKIP_ROWS", GL_PACK_SKIP_ROWS, 0xd946eac8, GLPARAM_INT, 1 },
        { "GL_PACK_SWAP_BYTES", GL_PACK_SWAP_BYTES, 0xa6bec190, GLPARAM_BOOLEAN, 1 },
        { "GL_PERSPECTIVE_CORRECTION_HINT", GL_PERSPECTIVE_CORRECTION_HINT, 0x8d6f0477, GLPARAM_INT, 1 },
        { "GL_PIXEL_MAP_A_TO_A_SIZE", GL_PIXEL_MAP_A_TO_A_SIZE, 0x6b2c0067, GLPARAM_INT, 1 },
        { "GL_PIXEL_MAP_B_TO_B_SIZE", GL_PIXEL_MAP_B_TO_B_SIZE, 0x25405857, GLPARAM_INT, 1 },
        { "GL_PIXEL_MAP_G_TO_G_SIZE", GL_PIXEL_MAP_G_TO_G_SIZE, 0x1bd44b7, GLPARAM_INT, 1 },
        { "GL_PIXEL_MAP_I_TO_A_SIZE", GL_PIXEL_MAP_I_TO_A_SIZE, 0x7c9da7c5, GLPARAM_INT, 1 },
        { "GL_PIXEL_MAP_I_TO_B_SIZE", GL_PIXEL_MAP_I_TO_B_SIZE, 0x122042f3, GLPARAM_INT, 1 },
        { "GL_PIXEL_MAP_I_TO_G_SIZE", GL_PIXEL_MAP_I_TO_G_SIZE, 0x6be2a12c, GLPARAM_INT, 1 },
        { "GL_PIXEL_MAP_I_TO_I_SIZE", GL_PIXEL_MAP_I_TO_I_SIZE, 0x6f9bb79c, GLPARAM_INT, 1 },
        { "GL_PIXEL_MAP_I_TO_R_SIZE", GL_PIXEL_MAP_I_TO_R_SIZE, 0x56af6269, GLPARAM_INT, 1 },
        { "GL_PIXEL_MAP_R_TO_R_SIZE", GL_PIXEL_MAP_R_TO_R_SIZE, 0x5d8ac0e5, GLPARAM_INT, 1 },
        { "GL_PIXEL_MAP_S_TO_S_SIZE", GL_PIXEL_MAP_S_TO_S_SIZE, 0xa5ac84d2, GLPARAM_INT, 1 },
        { "GL_POINT_SIZE", GL_POINT_SIZE, 0xa1ad1a8d, GLPARAM_DOUBLE, 1 },
        { "GL_POINT_SIZE_GRANULARITY", GL_POINT_SIZE_GRANULARITY, 0x1a1cf7f1, GLPARAM_DOUBLE, 1 },
        { "GL_POINT_SIZE_RANGE", GL_POINT_SIZE_RANGE, 0xf341db2a, GLPARAM_DOUBLE, 2 },
        { "GL_POINT_SMOOTH", GL_POINT_SMOOTH, 0xacb67eed, GLPARAM_BOOLEAN, 1 },
        { "GL_POINT_SMOOTH_HINT", GL_POINT_SMOOTH_HINT, 0x7fddac08, GLPARAM_INT, 1 },
        { "GL_POLYGON_MODE", GL_POLYGON_MODE, 0x5c82008e, GLPARAM_INT, 2 },
        { "GL_POLYGON_OFFSET_FACTOR", GL_POLYGON_OFFSET_FACTOR, 0x24a68003, GLPARAM_DOUBLE, 1 },
        { "GL_POLYGON_OFFSET_UNITS", GL_POLYGON_OFFSET_UNITS, 0xe964a9e, GLPARAM_DOUBLE, 1 },
        { "GL_POLYGON_OFFSET_FILL", GL_POLYGON_OFFSET_FILL, 0x33db9633, GLPARAM_BOOLEAN, 1 },
        { "GL_POLYGON_OFFSET_LINE", GL_POLYGON_OFFSET_LINE, 0x1d0bf477, GLPARAM_BOOLEAN, 1 },
        { "GL_POLYGON_OFFSET_POINT", GL_POLYGON_OFFSET_POINT, 0x7026f3, GLPARAM_BOOLEAN, 1 },
        { "GL_POLYGON_SMOOTH", GL_POLYGON_SMOOTH, 0xaa85c7a3, GLPARAM_BOOLEAN, 1 },
        { "GL_POLYGON_SMOOTH_HINT", GL_POLYGON_SMOOTH_HINT, 0x3ed766d5, GLPARAM_INT, 1 },
        { "GL_POLYGON_STIPPLE", GL_POLYGON_STIPPLE, 0x9d2afe05, GLPARAM_BOOLEAN, 1 },
        { "GL_PROJECTION_MATRIX", GL_PROJECTION_MATRIX, 0x77ffe1af, GLPARAM_DOUBLE, 16 },
        { "GL_PROJECTION_STACK_DEPTH", GL_PROJECTION_STACK_DEPTH, 0x8c0a4ff3, GLPARAM_INT, 1 },
        { "GL_READ_BUFFER", GL_READ_BUFFER, 0xa98284f0, GLPARAM_INT, 1 },
        { "GL_RED_BIAS", GL_RED_BIAS, 0x24f78267, GLPARAM_DOUBLE, 1 },
        { "GL_RED_BITS", GL_RED_BITS, 0xeda26dd1, GLPARAM_INT, 1 },
        { "GL_RED_SCALE", GL_RED_SCALE, 0x9427392b, GLPARAM_DOUBLE, 1 },
        { "GL_RENDER_MODE", GL_RENDER_MODE, 0x7fb9f74d, GLPARAM_INT, 1 },
        { "GL_RGBA_MODE", GL_RGBA_MODE, 0x1aea7482, GLPARAM_BOOLEAN, 1 },
        { "GL_SCISSOR_BOX", GL_SCISSOR_BOX, 0x1aae8169, GLPARAM_DOUBLE, 4 },
        { "GL_SCISSOR_TEST", GL_SCISSOR_TEST, 0xd765600a, GLPARAM_BOOLEAN, 1 },
        { "GL_SHADE_MODEL", GL_SHADE_MODEL, 0x54775558, GLPARAM_INT, 1 },
        { "GL_STENCIL_BITS", GL_STENCIL_BITS, 0xbb4ccd09, GLPARAM_INT, 1 },
        { "GL_STENCIL_CLEAR_VALUE", GL_STENCIL_CLEAR_VALUE, 0x9aa933a2, GLPARAM_DOUBLE, 1 },
        { "GL_STENCIL_FAIL", GL_STENCIL_FAIL, 0x9d669020, GLPARAM_INT, 1 },
        { "GL_STENCIL_FUNC", GL_STENCIL_FUNC, 0x5f738047, GLPARAM_INT, 1 },
        { "GL_STENCIL_PASS_DEPTH_FAIL", GL_STENCIL_PASS_DEPTH_FAIL, 0xfc785121, GLPARAM_INT, 1 },
        { "GL_STENCIL_PASS_DEPTH_PASS", GL_STENCIL_PASS_DEPTH_PASS, 0x5152491f, GLPARAM_INT, 1 },
        { "GL_STENCIL_REF", GL_STENCIL_REF, 0xfdcbf484, GLPARAM_DOUBLE, 1 },
        { "GL_STENCIL_TEST", GL_STENCIL_TEST, 0x245817c1, GLPARAM_BOOLEAN, 1 },
        { "GL_STENCIL_VALUE_MASK", GL_STENCIL_VALUE_MASK, 0x96bb7f38, GLPARAM_INT, 1 },
        { "GL_STENCIL_WRITEMASK", GL_STENCIL_WRITEMASK, 0xee3b40b1, GLPARAM_INT, 1 },
        { "GL_STEREO", GL_STEREO, 0x9eb7b4b, GLPARAM_BOOLEAN, 1 },
        { "GL_SUBPIXEL_BITS", GL_SUBPIXEL_BITS, 0x5cc0fc2a, GLPARAM_INT, 1 },
        { "GL_TEXTURE_1D", GL_TEXTURE_1D, 0x5a769edd, GLPARAM_BOOLEAN, 1 },
        { "GL_TEXTURE_2D", GL_TEXTURE_2D, 0x5b2970f1, GLPARAM_BOOLEAN, 1 },
        { "GL_TEXTURE_COORD_ARRAY", GL_TEXTURE_COORD_ARRAY, 0xe5c63ac3, GLPARAM_BOOLEAN, 1 },
        { "GL_TEXTURE_COORD_ARRAY_SIZE", GL_TEXTURE_COORD_ARRAY_SIZE, 0xc6369648, GLPARAM_INT, 1 },
        { "GL_TEXTURE_COORD_ARRAY_STRIDE", GL_TEXTURE_COORD_ARRAY_STRIDE, 0x3cb26375, GLPARAM_INT, 1 },
        { "GL_TEXTURE_COORD_ARRAY_TYPE", GL_TEXTURE_COORD_ARRAY_TYPE, 0x52caaa00, GLPARAM_INT, 1 },
        { "GL_TEXTURE_ENV_COLOR", GL_TEXTURE_ENV_COLOR, 0xaff39fa2, GLPARAM_DOUBLE, 4 },
        { "GL_TEXTURE_ENV_MODE", GL_TEXTURE_ENV_MODE, 0x3be892d6, GLPARAM_INT, 1 },
        { "GL_TEXTURE_GEN_Q", GL_TEXTURE_GEN_Q, 0xf9a6254b, GLPARAM_BOOLEAN, 1 },
        { "GL_TEXTURE_GEN_R", GL_TEXTURE_GEN_R, 0x80732541, GLPARAM_BOOLEAN, 1 },
        { "GL_TEXTURE_GEN_S", GL_TEXTURE_GEN_S, 0xdeb50068, GLPARAM_BOOLEAN, 1 },
        { "GL_TEXTURE_GEN_T", GL_TEXTURE_GEN_T, 0x55aa013b, GLPARAM_BOOLEAN, 1 },
        { "GL_TEXTURE_MATRIX", GL_TEXTURE_MATRIX, 0xe2ee2471, GLPARAM_DOUBLE, 16 },
        { "GL_TEXTURE_STACK_DEPTH", GL_TEXTURE_STACK_DEPTH, 0x220504de, GLPARAM_INT, 1 },
        { "GL_UNPACK_ALIGNMENT", GL_UNPACK_ALIGNMENT, 0xcc46b1ae, GLPARAM_INT, 1 },
        { "GL_UNPACK_ROW_LENGTH", GL_UNPACK_ROW_LENGTH, 0xc2ebc06c, GLPARAM_INT, 1 },
        { "GL_UNPACK_SKIP_PIXELS", GL_UNPACK_SKIP_PIXELS, 0x1204b192, GLPARAM_INT, 1 },
        { "GL_UNPACK_SWAP_BYTES", GL_UNPACK_SWAP_BYTES, 0x33d05a10, GLPARAM_BOOLEAN, 1 },
        { "GL_VERTEX_ARRAY", GL_VERTEX_ARRAY, 0xd04f30e5, GLPARAM_BOOLEAN, 1 },
        { "GL_VERTEX_ARRAY_SIZE", GL_VERTEX_ARRAY_SIZE, 0xe8ca882b, GLPARAM_INT, 1 },
        { "GL_VERTEX_ARRAY_STRIDE", GL_VERTEX_ARRAY_STRIDE, 0xdaa7f1a, GLPARAM_INT, 1 },
        { "GL_VERTEX_ARRAY_TYPE", GL_VERTEX_ARRAY_TYPE, 0xe5dc63d3, GLPARAM_INT, 1 },
        { "GL_VIEWPORT", GL_VIEWPORT, 0x49fe7a9e, GLPARAM_INT, 4 },
        { "GL_ZOOM_X", GL_ZOOM_X, 0xe1e6eed7, GLPARAM_DOUBLE, 1 },
        { "GL_ZOOM_Y", GL_ZOOM_Y, 0xa95ee43, GLPARAM_DOUBLE, 1 }
    };

    static const GLparaminfo arb_vertex_shader[] =
    {
        { "GL_MAX_VERTEX_UNIFORM_COMPONENTS_ARB", GL_MAX_VERTEX_UNIFORM_COMPONENTS_ARB, 0x99c443b, GLPARAM_INT, 1 },
        { "GL_MAX_VARYING_FLOATS_ARB", GL_MAX_VARYING_FLOATS_ARB, 0x71d691dd, GLPARAM_INT, 1 },
        { "GL_MAX_VERTEX_ATTRIBS_ARB", GL_MAX_VERTEX_ATTRIBS_ARB, 0x3404d44a, GLPARAM_INT, 1 },
        { "GL_MAX_TEXTURE_IMAGE_UNITS_ARB", GL_MAX_TEXTURE_IMAGE_UNITS_ARB, 0x888fafcf, GLPARAM_INT, 1 },
        { "GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS_ARB", GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS_ARB, 0x1acf3d1e, GLPARAM_INT, 1 },
        { "GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS_ARB", GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS_ARB, 0x40336bd, GLPARAM_INT, 1 },
        { "GL_MAX_TEXTURE_COORDS_ARB", GL_MAX_TEXTURE_COORDS_ARB, 0xfaff7fca, GLPARAM_INT, 1 }
    };

    static const GLparaminfo arb_vertex_program[] =
    {
        { "GL_PROGRAM_ERROR_POSITION_ARB", GL_PROGRAM_ERROR_POSITION_ARB, 0xf9e8b4c5, GLPARAM_INT, 1 },
        { "GL_CURRENT_MATRIX_ARB", GL_CURRENT_MATRIX_ARB, 0x98c9fa15, GLPARAM_DOUBLE, 16 },
        { "GL_TRANSPOSE_CURRENT_MATRIX_ARB", GL_TRANSPOSE_CURRENT_MATRIX_ARB, 0x604b8152, GLPARAM_DOUBLE, 16 },
        { "GL_CURRENT_MATRIX_STACK_DEPTH_ARB", GL_CURRENT_MATRIX_STACK_DEPTH_ARB, 0x8bf9d277, GLPARAM_INT, 1 },
        { "GL_MAX_VERTEX_ATTRIBS_ARB", GL_MAX_VERTEX_ATTRIBS_ARB, 0x3404d44a, GLPARAM_INT, 1 },
        { "GL_MAX_PROGRAM_MATRICES_ARB", GL_MAX_PROGRAM_MATRICES_ARB, 0xff15e0c4, GLPARAM_INT, 1 },
        { "GL_MAX_PROGRAM_MATRIX_STACK_DEPTH_ARB", GL_MAX_PROGRAM_MATRIX_STACK_DEPTH_ARB, 0x9d133e62, GLPARAM_INT, 1 },
        { "GL_VERTEX_PROGRAM_POINT_SIZE_ARB", GL_VERTEX_PROGRAM_POINT_SIZE_ARB, 0xb98513c8, GLPARAM_BOOLEAN, 1 },
        { "GL_VERTEX_PROGRAM_TWO_SIDE_ARB", GL_VERTEX_PROGRAM_TWO_SIDE_ARB, 0x8f3e58, GLPARAM_BOOLEAN, 1 },
        { "GL_COLOR_SUM_ARB", GL_COLOR_SUM_ARB, 0x5966e0fa, GLPARAM_BOOLEAN, 1 }
    };

#define PARAMINFO_GET( hash, list ) \
    if ( const GLparaminfo *info = GetParamByHash( list, NUMELMS( list ), hash ) ) \
        return info

    const GLparaminfo* GetParamByName( const glDriver *driver, const char *name, size_t nameLen )
    {
        unsigned int hash = TumblerHash( name, nameLen );

        PARAMINFO_GET( hash, param_list );

        if ( driver->supports_ARB_vertex_shader )
        {
            PARAMINFO_GET( hash, arb_vertex_shader );
        }

        if ( driver->supports_ARB_vertex_program )
        {
            PARAMINFO_GET( hash, arb_vertex_program );
        }

        return NULL;
    }

    void PushAvailableParams( lua_State *L, const glDriver *driver )
    {
        unsigned int n = 1;

        lua_createtable( L, 234, 0 );
        lua_pushlstring( L, "GL_ACCUM_ALPHA_BITS", 19 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_ACCUM_BLUE_BITS", 18 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_ACCUM_CLEAR_VALUE", 20 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_ACCUM_GREEN_BITS", 19 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_ACCUM_RED_BITS", 17 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_ALPHA_BIAS", 13 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_ALPHA_BITS", 13 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_ALPHA_SCALE", 14 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_ALPHA_TEST", 13 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_ALPHA_TEST_FUNC", 18 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_ALPHA_TEST_REF", 17 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_ATTRIB_STACK_DEPTH", 21 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_AUTO_NORMAL", 14 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_AUX_BUFFERS", 14 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_BLEND", 8 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_BLEND_DST", 12 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_BLEND_SRC", 12 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_BLUE_BIAS", 12 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_BLUE_BITS", 12 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_BLUE_SCALE", 13 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_CLIENT_ATTRIB_STACK_DEPTH", 28 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_CLIP_PLANE0", 14 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_CLIP_PLANE1", 14 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_CLIP_PLANE2", 14 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_CLIP_PLANE3", 14 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_CLIP_PLANE4", 14 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_CLIP_PLANE5", 14 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_COLOR_ARRAY", 14 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_COLOR_ARRAY_SIZE", 19 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_COLOR_ARRAY_STRIDE", 21 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_COLOR_ARRAY_TYPE", 19 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_COLOR_CLEAR_VALUE", 20 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_COLOR_LOGIC_OP", 17 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_COLOR_MATERIAL", 17 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_COLOR_MATERIAL_FACE", 22 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_COLOR_MATERIAL_PARAMETER", 27 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_COLOR_WRITEMASK", 18 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_CULL_FACE", 12 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_CULL_FACE_MODE", 17 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_CURRENT_COLOR", 16 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_CURRENT_INDEX", 16 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_CURRENT_NORMAL", 17 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_CURRENT_RASTER_COLOR", 23 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_CURRENT_RASTER_DISTANCE", 26 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_CURRENT_RASTER_INDEX", 23 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_CURRENT_RASTER_POSITION", 26 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_CURRENT_RASTER_POSITION_VALID", 32 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_CURRENT_RASTER_TEXTURE_COORDS", 32 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_CURRENT_TEXTURE_COORDS", 25 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_DEPTH_BIAS", 13 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_DEPTH_BITS", 13 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_DEPTH_CLEAR_VALUE", 20 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_DEPTH_FUNC", 13 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_DEPTH_RANGE", 14 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_DEPTH_SCALE", 14 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_DEPTH_TEST", 13 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_DEPTH_WRITEMASK", 18 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_DITHER", 9 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_DOUBLEBUFFER", 15 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_DRAW_BUFFER", 14 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_EDGE_FLAG", 12 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_EDGE_FLAG_ARRAY", 18 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_EDGE_FLAG_ARRAY_STRIDE", 25 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_FOG", 6 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_FOG_COLOR", 12 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_FOG_DENSITY", 14 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_FOG_END", 10 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_FOG_HINT", 11 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_FOG_INDEX", 12 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_FOG_MODE", 11 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_FOG_START", 12 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_FRONT_FACE", 13 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_GREEN_BIAS", 13 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_GREEN_BITS", 13 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_GREEN_SCALE", 14 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_INDEX_ARRAY", 14 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_INDEX_ARRAY_STRIDE", 21 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_INDEX_ARRAY_TYPE", 19 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_INDEX_BITS", 13 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_INDEX_CLEAR_VALUE", 20 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_INDEX_LOGIC_OP", 17 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_INDEX_MODE", 13 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_INDEX_OFFSET", 15 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_INDEX_SHIFT", 14 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_INDEX_WRITEMASK", 18 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_LIGHT0", 9 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_LIGHT1", 9 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_LIGHT2", 9 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_LIGHT3", 9 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_LIGHT4", 9 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_LIGHT5", 9 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_LIGHT6", 9 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_LIGHT7", 9 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_LIGHTING", 11 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_LIGHT_MODEL_AMBIENT", 22 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_LIGHT_MODEL_LOCAL_VIEWER", 27 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_LIGHT_MODEL_TWO_SIDE", 23 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_LINE_SMOOTH", 14 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_LINE_SMOOTH_HINT", 19 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_LINE_STIPPLE", 15 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_LINE_STIPPLE_PATTERN", 23 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_LINE_STIPPLE_REPEAT", 22 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_LINE_WIDTH", 13 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_LINE_WIDTH_GRANULARITY", 25 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_LINE_WIDTH_RANGE", 19 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_LIST_BASE", 12 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_LIST_INDEX", 13 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_LIST_MODE", 12 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_LOGIC_OP", 11 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_LOGIC_OP_MODE", 16 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_MAP1_COLOR_4", 15 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_MAP1_GRID_DOMAIN", 19 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_MAP1_GRID_SEGMENTS", 21 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_MAP1_INDEX", 13 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_MAP1_NORMAL", 14 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_MAP1_TEXTURE_COORD_1", 23 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_MAP1_TEXTURE_COORD_2", 23 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_MAP1_TEXTURE_COORD_3", 23 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_MAP1_TEXTURE_COORD_4", 23 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_MAP1_VERTEX_3", 16 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_MAP1_VERTEX_4", 16 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_MAP2_COLOR_4", 15 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_MAP2_GRID_DOMAIN", 19 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_MAP2_GRID_SEGMENTS", 21 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_MAP2_INDEX", 13 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_MAP2_NORMAL", 14 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_MAP2_TEXTURE_COORD_1", 23 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_MAP2_TEXTURE_COORD_2", 23 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_MAP2_TEXTURE_COORD_3", 23 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_MAP2_TEXTURE_COORD_4", 23 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_MAP2_VERTEX_3", 16 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_MAP2_VERTEX_4", 16 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_MAP_COLOR", 12 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_MAP_STENCIL", 14 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_MATRIX_MODE", 14 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_MAX_CLIENT_ATTRIB_STACK_DEPTH", 32 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_MAX_ATTRIB_STACK_DEPTH", 25 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_MAX_CLIP_PLANES", 18 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_MAX_EVAL_ORDER", 17 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_MAX_LIGHTS", 13 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_MAX_LIST_NESTING", 19 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_MAX_MODELVIEW_STACK_DEPTH", 28 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_MAX_NAME_STACK_DEPTH", 23 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_MAX_PIXEL_MAP_TABLE", 22 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_MAX_PROJECTION_STACK_DEPTH", 29 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_MAX_TEXTURE_SIZE", 19 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_MAX_TEXTURE_STACK_DEPTH", 26 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_MAX_VIEWPORT_DIMS", 20 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_MODELVIEW_MATRIX", 19 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_MODELVIEW_STACK_DEPTH", 24 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_NAME_STACK_DEPTH", 19 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_NORMAL_ARRAY", 15 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_NORMAL_ARRAY_STRIDE", 22 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_NORMAL_ARRAY_TYPE", 20 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_NORMALIZE", 12 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_PACK_ALIGNMENT", 17 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_PACK_LSB_FIRST", 17 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_PACK_ROW_LENGTH", 18 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_PACK_SKIP_PIXELS", 19 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_PACK_SKIP_ROWS", 17 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_PACK_SWAP_BYTES", 18 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_PERSPECTIVE_CORRECTION_HINT", 30 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_PIXEL_MAP_A_TO_A_SIZE", 24 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_PIXEL_MAP_B_TO_B_SIZE", 24 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_PIXEL_MAP_G_TO_G_SIZE", 24 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_PIXEL_MAP_I_TO_A_SIZE", 24 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_PIXEL_MAP_I_TO_B_SIZE", 24 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_PIXEL_MAP_I_TO_G_SIZE", 24 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_PIXEL_MAP_I_TO_I_SIZE", 24 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_PIXEL_MAP_I_TO_R_SIZE", 24 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_PIXEL_MAP_R_TO_R_SIZE", 24 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_PIXEL_MAP_S_TO_S_SIZE", 24 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_POINT_SIZE", 13 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_POINT_SIZE_GRANULARITY", 25 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_POINT_SIZE_RANGE", 19 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_POINT_SMOOTH", 15 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_POINT_SMOOTH_HINT", 20 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_POLYGON_MODE", 15 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_POLYGON_OFFSET_FACTOR", 24 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_POLYGON_OFFSET_UNITS", 23 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_POLYGON_OFFSET_FILL", 22 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_POLYGON_OFFSET_LINE", 22 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_POLYGON_OFFSET_POINT", 23 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_POLYGON_SMOOTH", 17 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_POLYGON_SMOOTH_HINT", 22 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_POLYGON_STIPPLE", 18 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_PROJECTION_MATRIX", 20 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_PROJECTION_STACK_DEPTH", 25 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_READ_BUFFER", 14 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_RED_BIAS", 11 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_RED_BITS", 11 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_RED_SCALE", 12 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_RENDER_MODE", 14 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_RGBA_MODE", 12 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_SCISSOR_BOX", 14 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_SCISSOR_TEST", 15 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_SHADE_MODEL", 14 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_STENCIL_BITS", 15 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_STENCIL_CLEAR_VALUE", 22 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_STENCIL_FAIL", 15 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_STENCIL_FUNC", 15 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_STENCIL_PASS_DEPTH_FAIL", 26 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_STENCIL_PASS_DEPTH_PASS", 26 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_STENCIL_REF", 14 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_STENCIL_TEST", 15 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_STENCIL_VALUE_MASK", 21 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_STENCIL_WRITEMASK", 20 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_STEREO", 9 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_SUBPIXEL_BITS", 16 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_TEXTURE_1D", 13 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_TEXTURE_2D", 13 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_TEXTURE_COORD_ARRAY", 22 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_TEXTURE_COORD_ARRAY_SIZE", 27 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_TEXTURE_COORD_ARRAY_STRIDE", 29 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_TEXTURE_COORD_ARRAY_TYPE", 27 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_TEXTURE_ENV_COLOR", 20 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_TEXTURE_ENV_MODE", 19 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_TEXTURE_GEN_Q", 16 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_TEXTURE_GEN_R", 16 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_TEXTURE_GEN_S", 16 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_TEXTURE_GEN_T", 16 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_TEXTURE_MATRIX", 17 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_TEXTURE_STACK_DEPTH", 22 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_UNPACK_ALIGNMENT", 19 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_UNPACK_ROW_LENGTH", 20 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_UNPACK_SKIP_PIXELS", 21 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_UNPACK_SWAP_BYTES", 20 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_VERTEX_ARRAY", 15 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_VERTEX_ARRAY_SIZE", 20 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_VERTEX_ARRAY_STRIDE", 22 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_VERTEX_ARRAY_TYPE", 20 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_VIEWPORT", 11 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_ZOOM_X", 9 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_ZOOM_Y", 9 );
        lua_rawseti( L, -2, n++ );
        
        if ( driver->supports_ARB_vertex_shader )
        {
            lua_pushlstring( L, "GL_MAX_VERTEX_UNIFORM_COMPONENTS_ARB", 36 );
            lua_rawseti( L, -2, n++ );
            lua_pushlstring( L, "GL_MAX_VARYING_FLOATS_ARB", 25 );
            lua_rawseti( L, -2, n++ );
            lua_pushlstring( L, "GL_MAX_VERTEX_ATTRIBS_ARB", 25 );
            lua_rawseti( L, -2, n++ );
            lua_pushlstring( L, "GL_MAX_TEXTURE_IMAGE_UNITS_ARB", 30 );
            lua_rawseti( L, -2, n++ );
            lua_pushlstring( L, "GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS_ARB", 37 );
            lua_rawseti( L, -2, n++ );
            lua_pushlstring( L, "GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS_ARB", 39 );
            lua_rawseti( L, -2, n++ );
            lua_pushlstring( L, "GL_MAX_TEXTURE_COORDS_ARB", 25 );
            lua_rawseti( L, -2, n++ );
        }

        if ( driver->supports_ARB_vertex_program )
        {
            lua_pushlstring( L, "GL_PROGRAM_ERROR_POSITION_ARB", 29 );
            lua_rawseti( L, -2, n++ );
            lua_pushlstring( L, "GL_CURRENT_MATRIX_ARB", 21 );
            lua_rawseti( L, -2, n++ );
            lua_pushlstring( L, "GL_TRANSPOSE_CURRENT_MATRIX_ARB", 31 );
            lua_rawseti( L, -2, n++ );
            lua_pushlstring( L, "GL_CURRENT_MATRIX_STACK_DEPTH_ARB", 33 );
            lua_rawseti( L, -2, n++ );
            lua_pushlstring( L, "GL_MAX_VERTEX_ATTRIBS_ARB", 25 );
            lua_rawseti( L, -2, n++ );
            lua_pushlstring( L, "GL_MAX_PROGRAM_MATRICES_ARB", 27 );
            lua_rawseti( L, -2, n++ );
            lua_pushlstring( L, "GL_MAX_PROGRAM_MATRIX_STACK_DEPTH_ARB", 37 );
            lua_rawseti( L, -2, n++ );
            lua_pushlstring( L, "GL_VERTEX_PROGRAM_POINT_SIZE_ARB", 32 );
            lua_rawseti( L, -2, n++ );
            lua_pushlstring( L, "GL_VERTEX_PROGRAM_TWO_SIDE_ARB", 30 );
            lua_rawseti( L, -2, n++ );
            lua_pushlstring( L, "GL_COLOR_SUM_ARB", 16 );
            lua_rawseti( L, -2, n++ );
        }
    }
};

namespace gl_capabilities
{
    static const GLparamdesc caps[] =
    {
        { "GL_ALPHA_TEST", GL_ALPHA_TEST, 0xaae45f62 },
        { "GL_AUTO_NORMAL", GL_AUTO_NORMAL, 0x712edb6b },
        { "GL_BLEND", GL_BLEND, 0xa40557e7 },
        { "GL_CLIP_PLANE0", GL_CLIP_PLANE0, 0x848c8631 },
        { "GL_CLIP_PLANE1", GL_CLIP_PLANE1, 0x6ccbd80b },
        { "GL_CLIP_PLANE2", GL_CLIP_PLANE2, 0x259843b6 },
        { "GL_CLIP_PLANE3", GL_CLIP_PLANE3, 0x84c3f94b },
        { "GL_CLIP_PLANE4", GL_CLIP_PLANE4, 0x8ddcb267 },
        { "GL_CLIP_PLANE5", GL_CLIP_PLANE5, 0xe40d4d43 },
        { "GL_COLOR_LOGIC_OP", GL_COLOR_LOGIC_OP, 0x8b172a39 },
        { "GL_COLOR_MATERIAL", GL_COLOR_MATERIAL, 0xe442a42c },
        { "GL_CULL_FACE", GL_CULL_FACE, 0x81bbd421 },
        { "GL_DEPTH_TEST", GL_DEPTH_TEST, 0x39aac8e4 },
        { "GL_DITHER", GL_DITHER, 0x128ad5d0 },
        { "GL_FOG", GL_FOG, 0x8ed3f4e8 },
        { "GL_INDEX_LOGIC_OP", GL_INDEX_LOGIC_OP, 0x9bd85422 },
        { "GL_LIGHT0", GL_LIGHT0, 0x5bda68ee },
        { "GL_LIGHT1", GL_LIGHT1, 0x1b886cd2 },
        { "GL_LIGHT2", GL_LIGHT2, 0x7ecb116a },
        { "GL_LIGHT3", GL_LIGHT3, 0xfaabaa6f },
        { "GL_LIGHT4", GL_LIGHT4, 0xbb9130b9 },
        { "GL_LIGHT5", GL_LIGHT5, 0x532a2f97 },
        { "GL_LIGHT6", GL_LIGHT6, 0x1f8afeb3 },
        { "GL_LIGHT7", GL_LIGHT7, 0x146d8bd2 },
        { "GL_LINE_STIPPLE", GL_LINE_STIPPLE, 0x7c77b2ea },
        { "GL_LOGIC_OP", GL_LOGIC_OP, 0xaae3bb9a },
        { "GL_MAP1_COLOR_4", GL_MAP1_COLOR_4, 0x384eb930 },
        { "GL_MAP1_INDEX", GL_MAP1_INDEX, 0xa09ff53f },
        { "GL_MAP1_NORMAL", GL_MAP1_NORMAL, 0xa6ab1a46 },
        { "GL_MAP1_TEXTURE_COORD_1", GL_MAP1_TEXTURE_COORD_1, 0x151d1a8b },
        { "GL_MAP1_TEXTURE_COORD_2", GL_MAP1_TEXTURE_COORD_2, 0x413aac21 },
        { "GL_MAP1_TEXTURE_COORD_3", GL_MAP1_TEXTURE_COORD_3, 0x913d7ca6 },
        { "GL_MAP1_TEXTURE_COORD_4", GL_MAP1_TEXTURE_COORD_4, 0xc40342ac },
        { "GL_MAP1_VERTEX_3", GL_MAP1_VERTEX_3, 0x55c3f9a5 },
        { "GL_MAP1_VERTEX_4", GL_MAP1_VERTEX_4, 0x5f8980f3 },
        { "GL_MAP2_COLOR_4", GL_MAP2_COLOR_4, 0xb288454 },
        { "GL_MAP2_INDEX", GL_MAP2_INDEX, 0x8ff86285 },
        { "GL_MAP2_NORMAL", GL_MAP2_NORMAL, 0x51fcdb15 },
        { "GL_MAP2_TEXTURE_COORD_1", GL_MAP2_TEXTURE_COORD_1, 0x1b2e6191 },
        { "GL_MAP2_TEXTURE_COORD_2", GL_MAP2_TEXTURE_COORD_2, 0x4f09d73b },
        { "GL_MAP2_TEXTURE_COORD_3", GL_MAP2_TEXTURE_COORD_3, 0x9f0e07bc },
        { "GL_MAP2_TEXTURE_COORD_4", GL_MAP2_TEXTURE_COORD_4, 0xca3039b6 },
        { "GL_MAP2_VERTEX_3", GL_MAP2_VERTEX_3, 0x87e0f262 },
        { "GL_MAP2_VERTEX_4", GL_MAP2_VERTEX_4, 0x8daa8b34 },
        { "GL_NORMALIZE", GL_NORMALIZE, 0xea0eb318 },
        { "GL_POINT_SMOOTH", GL_POINT_SMOOTH, 0xacb67eed },
        { "GL_POLYGON_OFFSET_FILL", GL_POLYGON_OFFSET_FILL, 0x33db9633 },
        { "GL_POLYGON_OFFSET_LINE", GL_POLYGON_OFFSET_LINE, 0x1d0bf477 },
        { "GL_POLYGON_OFFSET_POINT", GL_POLYGON_OFFSET_POINT, 0x7026f3 },
        { "GL_POLYGON_SMOOTH", GL_POLYGON_SMOOTH, 0xaa85c7a3 },
        { "GL_POLYGON_STIPPLE", GL_POLYGON_STIPPLE, 0x9d2afe05 },
        { "GL_SCISSOR_TEST", GL_SCISSOR_TEST, 0xd765600a },
        { "GL_STENCIL_TEST", GL_STENCIL_TEST, 0x245817c1 },
        { "GL_TEXTURE_1D", GL_TEXTURE_1D, 0x5a769edd },
        { "GL_TEXTURE_2D", GL_TEXTURE_2D, 0x5b2970f1 },
        { "GL_TEXTURE_GEN_Q", GL_TEXTURE_GEN_Q, 0xf9a6254b },
        { "GL_TEXTURE_GEN_R", GL_TEXTURE_GEN_R, 0x80732541 },
        { "GL_TEXTURE_GEN_S", GL_TEXTURE_GEN_S, 0xdeb50068 },
        { "GL_TEXTURE_GEN_T", GL_TEXTURE_GEN_T, 0x55aa013b },
    };

    static const GLparamdesc arb_vertex_program[] =
    {
        { "GL_VERTEX_PROGRAM_POINT_SIZE_ARB", GL_VERTEX_PROGRAM_POINT_SIZE_ARB, 0xb98513c8 },
        { "GL_VERTEX_PROGRAM_TWO_SIDE_ARB", GL_VERTEX_PROGRAM_TWO_SIDE_ARB, 0x8f3e58 },
        { "GL_COLOR_SUM_ARB", GL_COLOR_SUM_ARB, 0x5966e0fa },
        { "GL_VERTEX_PROGRAM_ARB", GL_VERTEX_PROGRAM_ARB, 0x771b3557 }
    };

    const GLparamdesc* GetParamByName( const glDriver *driver, const char *name, size_t nameLen )
    {
        unsigned int hash = TumblerHash( name, nameLen );

        PARAM_GET( hash, caps );

        if ( driver->supports_ARB_vertex_program )
        {
            PARAM_GET( hash, arb_vertex_program );
        }

        return NULL;
    }

    void PushAvailableParams( lua_State *L, const glDriver *driver )
    {
        unsigned int n = 1;

        lua_createtable( L, 60, 0 );
        lua_pushlstring( L, "GL_ALPHA_TEST", 13 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_AUTO_NORMAL", 14 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_BLEND", 8 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_CLIP_PLANE0", 14 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_CLIP_PLANE1", 14 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_CLIP_PLANE2", 14 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_CLIP_PLANE3", 14 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_CLIP_PLANE4", 14 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_CLIP_PLANE5", 14 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_COLOR_LOGIC_OP", 17 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_COLOR_MATERIAL", 17 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_CULL_FACE", 12 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_DEPTH_TEST", 13 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_DITHER", 9 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_FOG", 6 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_INDEX_LOGIC_OP", 17 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_LIGHT0", 9 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_LIGHT1", 9 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_LIGHT2", 9 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_LIGHT3", 9 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_LIGHT4", 9 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_LIGHT5", 9 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_LIGHT6", 9 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_LIGHT7", 9 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_LINE_STIPPLE", 15 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_LOGIC_OP", 11 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_MAP1_COLOR_4", 15 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_MAP1_INDEX", 13 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_MAP1_NORMAL", 14 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_MAP1_TEXTURE_COORD_1", 23 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_MAP1_TEXTURE_COORD_2", 23 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_MAP1_TEXTURE_COORD_3", 23 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_MAP1_TEXTURE_COORD_4", 23 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_MAP1_VERTEX_3", 16 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_MAP1_VERTEX_4", 16 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_MAP2_COLOR_4", 15 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_MAP2_INDEX", 13 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_MAP2_NORMAL", 14 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_MAP2_TEXTURE_COORD_1", 23 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_MAP2_TEXTURE_COORD_2", 23 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_MAP2_TEXTURE_COORD_3", 23 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_MAP2_TEXTURE_COORD_4", 23 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_MAP2_VERTEX_3", 16 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_MAP2_VERTEX_4", 16 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_NORMALIZE", 12 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_POINT_SMOOTH", 15 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_POLYGON_OFFSET_FILL", 22 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_POLYGON_OFFSET_LINE", 22 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_POLYGON_OFFSET_POINT", 23 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_POLYGON_SMOOTH", 17 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_POLYGON_STIPPLE", 18 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_SCISSOR_TEST", 15 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_STENCIL_TEST", 15 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_TEXTURE_1D", 13 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_TEXTURE_2D", 13 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_TEXTURE_GEN_Q", 16 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_TEXTURE_GEN_R", 16 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_TEXTURE_GEN_S", 16 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_TEXTURE_GEN_T", 16 );
        lua_rawseti( L, -2, n++ );

        if ( driver->supports_ARB_vertex_program )
        {
            lua_pushlstring( L, "GL_VERTEX_PROGRAM_POINT_SIZE_ARB", 32 );
            lua_rawseti( L, -2, n++ );
            lua_pushlstring( L, "GL_VERTEX_PROGRAM_TWO_SIDE_ARB", 30 );
            lua_rawseti( L, -2, n++ );
            lua_pushlstring( L, "GL_COLOR_SUM_ARB", 16 );
            lua_rawseti( L, -2, n++ );
            lua_pushlstring( L, "GL_VERTEX_PROGRAM_ARB", 21 );
            lua_rawseti( L, -2, n++ );
        }
    }
};

namespace gl_stringParams
{
    const GLparamdesc* GetParamByName( const glDriver *driver, const char *name, size_t nameLen )
    {
        unsigned int hash = TumblerHash( name, nameLen );



        return NULL;
    }

    void PushAvailableParams( lua_State *L, const glDriver *driver )
    {
        unsigned int n = 1;


    }
};

namespace gl_objectARB
{
    static const GLparaminfo param_list[] =
    {
        { "GL_OBJECT_TYPE_ARB", GL_OBJECT_TYPE_ARB, 0x1fb4a2a0, GLPARAM_INT, 1 },
        { "GL_OBJECT_SUBTYPE_ARB", GL_OBJECT_SUBTYPE_ARB, 0x79429d6c, GLPARAM_INT, 1 },
        { "GL_OBJECT_DELETE_STATUS_ARB", GL_OBJECT_DELETE_STATUS_ARB, 0xd354e5da, GLPARAM_BOOLEAN, 1 },
        { "GL_OBJECT_COMPILE_STATUS_ARB", GL_OBJECT_COMPILE_STATUS_ARB, 0x198d9796, GLPARAM_BOOLEAN, 1 },
        { "GL_OBJECT_LINK_STATUS_ARB", GL_OBJECT_LINK_STATUS_ARB, 0x9d874ec2, GLPARAM_BOOLEAN, 1 },
        { "GL_OBJECT_VALIDATE_STATUS_ARB", GL_OBJECT_VALIDATE_STATUS_ARB, 0x8aaba4a5, GLPARAM_BOOLEAN, 1 },
        { "GL_OBJECT_INFO_LOG_LENGTH_ARB", GL_OBJECT_INFO_LOG_LENGTH_ARB, 0xe244001c, GLPARAM_INT, 1 },
        { "GL_OBJECT_ATTACHED_OBJECTS_ARB", GL_OBJECT_ATTACHED_OBJECTS_ARB, 0xd8b97a6a, GLPARAM_INT, 1 },
        { "GL_OBJECT_ACTIVE_UNIFORMS_ARB", GL_OBJECT_ACTIVE_UNIFORMS_ARB, 0x85e65e2f, GLPARAM_INT, 1 },
        { "GL_OBJECT_ACTIVE_UNIFORM_MAX_LENGTH_ARB", GL_OBJECT_ACTIVE_UNIFORM_MAX_LENGTH_ARB, 0x3f490479, GLPARAM_INT, 1 },
        { "GL_OBJECT_SHADER_SOURCE_LENGTH_ARB", GL_OBJECT_SHADER_SOURCE_LENGTH_ARB, 0xd4c3134d, GLPARAM_INT, 1 }
    };

    static const GLparaminfo arb_vertex_program[] =
    {
        { "GL_OBJECT_ACTIVE_ATTRIBUTES_ARB", GL_OBJECT_ACTIVE_ATTRIBUTES_ARB, 0x735a219f, GLPARAM_INT, 1 },
        { "GL_OBJECT_ACTIVE_ATTRIBUTE_MAX_LENGTH_ARB", GL_OBJECT_ACTIVE_ATTRIBUTE_MAX_LENGTH_ARB, 0xdb005c1f, GLPARAM_INT, 1 }
    };

    const GLparaminfo* GetParamByName( const glDriver *driver, const char *name, size_t nameLen )
    {
        unsigned int hash = TumblerHash( name, nameLen );

        if ( driver->supports_ARB_shader_objects )
        {
            PARAMINFO_GET( hash, param_list );
        }

        if ( driver->supports_ARB_vertex_program )
        {
            PARAMINFO_GET( hash, arb_vertex_program );
        }

        return NULL;
    }

    void PushAvailableParams( lua_State *L, const glDriver *driver )
    {
        unsigned int n = 1;

        lua_createtable( L, 11, 0 );
        lua_pushlstring( L, "GL_OBJECT_TYPE_ARB", 18 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_OBJECT_SUBTYPE_ARB", 21 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_OBJECT_DELETE_STATUS_ARB", 27 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_OBJECT_COMPILE_STATUS_ARB", 28 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_OBJECT_LINK_STATUS_ARB", 25 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_OBJECT_VALIDATE_STATUS_ARB", 29 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_OBJECT_INFO_LOG_LENGTH_ARB", 29 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_OBJECT_ATTACHED_OBJECTS_ARB", 30 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_OBJECT_ACTIVE_UNIFORMS_ARB", 29 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_OBJECT_ACTIVE_UNIFORM_MAX_LENGTH_ARB", 39 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_OBJECT_SHADER_SOURCE_LENGTH_ARB", 34 );
        lua_rawseti( L, -2, n++ );

        if ( driver->supports_ARB_vertex_program )
        {
            lua_pushlstring( L, "GL_OBJECT_ACTIVE_ATTRIBUTES_ARB", 31 );
            lua_rawseti( L, -2, n++ );
            lua_pushlstring( L, "GL_OBJECT_ACTIVE_ATTRIBUTE_MAX_LENGTH_ARB", 41 );
            lua_rawseti( L, -2, n++ );
        }
    }
};

namespace gl_programARB
{
    static const GLparaminfo param_list[] =
    {
        { "GL_PROGRAM_LENGTH_ARB", GL_PROGRAM_LENGTH_ARB, 0x32689463, GLPARAM_INT, 1 },
        { "GL_PROGRAM_FORMAT_ARB", GL_PROGRAM_FORMAT_ARB, 0x9cea6c4e, GLPARAM_INT, 1 },
        { "GL_PROGRAM_BINDING_ARB", GL_PROGRAM_BINDING_ARB, 0x9e5cd3d0, GLPARAM_INT, 1 },
        { "GL_PROGRAM_INSTRUCTIONS_ARB", GL_PROGRAM_INSTRUCTIONS_ARB, 0xed49b69e, GLPARAM_INT, 1 },
        { "GL_MAX_PROGRAM_INSTRUCTIONS_ARB", GL_MAX_PROGRAM_INSTRUCTIONS_ARB, 0xcca96e77, GLPARAM_INT, 1 },
        { "GL_PROGRAM_NATIVE_INSTRUCTIONS_ARB", GL_PROGRAM_NATIVE_INSTRUCTIONS_ARB, 0x46322a9e, GLPARAM_INT, 1 },
        { "GL_MAX_PROGRAM_NATIVE_INSTRUCTIONS_ARB", GL_MAX_PROGRAM_NATIVE_INSTRUCTIONS_ARB, 0x2d3ba598, GLPARAM_INT, 1 },
        { "GL_PROGRAM_TEMPORARIES_ARB", GL_PROGRAM_TEMPORARIES_ARB, 0xf5932482, GLPARAM_INT, 1 },
        { "GL_MAX_PROGRAM_TEMPORARIES_ARB", GL_MAX_PROGRAM_TEMPORARIES_ARB, 0xf1c024c9, GLPARAM_INT, 1 },
        { "GL_PROGRAM_NATIVE_TEMPORARIES_ARB", GL_PROGRAM_NATIVE_TEMPORARIES_ARB, 0xb1840957, GLPARAM_INT, 1 },
        { "GL_MAX_PROGRAM_NATIVE_TEMPORARIES_ARB", GL_MAX_PROGRAM_NATIVE_TEMPORARIES_ARB, 0x77fcab70, GLPARAM_INT, 1 },
        { "GL_PROGRAM_PARAMETERS_ARB", GL_PROGRAM_PARAMETERS_ARB, 0xc5550a85, GLPARAM_INT, 1 },
        { "GL_MAX_PROGRAM_PARAMETERS_ARB", GL_MAX_PROGRAM_PARAMETERS_ARB, 0xf8b11f66, GLPARAM_INT, 1 },
        { "GL_PROGRAM_NATIVE_PARAMETERS_ARB", GL_PROGRAM_NATIVE_PARAMETERS_ARB, 0xfadaf980, GLPARAM_INT, 1 },
        { "GL_MAX_PROGRAM_NATIVE_PARAMETERS_ARB", GL_MAX_PROGRAM_NATIVE_PARAMETERS_ARB, 0x60d494b5, GLPARAM_INT, 1 },
        { "GL_PROGRAM_ATTRIBS_ARB", GL_PROGRAM_ATTRIBS_ARB, 0xdfdf3b4e, GLPARAM_INT, 1 },
        { "GL_MAX_PROGRAM_ATTRIBS_ARB", GL_MAX_PROGRAM_ATTRIBS_ARB, 0xc06561d1, GLPARAM_INT, 1 },
        { "GL_PROGRAM_NATIVE_ATTRIBS_ARB", GL_PROGRAM_NATIVE_ATTRIBS_ARB, 0xc896e124, GLPARAM_INT, 1 },
        { "GL_MAX_PROGRAM_NATIVE_ATTRIBS_ARB", GL_MAX_PROGRAM_NATIVE_ATTRIBS_ARB, 0xaafa1138, GLPARAM_INT, 1 },
        { "GL_PROGRAM_ADDRESS_REGISTERS_ARB", GL_PROGRAM_ADDRESS_REGISTERS_ARB, 0x670d388, GLPARAM_INT, 1 },
        { "GL_MAX_PROGRAM_ADDRESS_REGISTERS_ARB", GL_MAX_PROGRAM_ADDRESS_REGISTERS_ARB, 0x6904f4ff, GLPARAM_INT, 1 },
        { "GL_PROGRAM_NATIVE_ADDRESS_REGISTERS_ARB", GL_PROGRAM_NATIVE_ADDRESS_REGISTERS_ARB, 0xb668bf51, GLPARAM_INT, 1 },
        { "GL_MAX_PROGRAM_NATIVE_ADDRESS_REGISTERS_ARB", GL_MAX_PROGRAM_NATIVE_ADDRESS_REGISTERS_ARB, 0xd03f1649, GLPARAM_INT, 1 },
        { "GL_MAX_PROGRAM_LOCAL_PARAMETERS_ARB", GL_MAX_PROGRAM_LOCAL_PARAMETERS_ARB, 0x7237edd5, GLPARAM_INT, 1 },
        { "GL_MAX_PROGRAM_ENV_PARAMETERS_ARB", GL_MAX_PROGRAM_ENV_PARAMETERS_ARB, 0xd63fd131, GLPARAM_INT, 1 },
        { "GL_PROGRAM_UNDER_NATIVE_LIMITS_ARB", GL_PROGRAM_UNDER_NATIVE_LIMITS_ARB, 0x62d496c5, GLPARAM_BOOLEAN, 1 }
    };

    const GLparaminfo* GetParamByName( const glDriver *driver, const char *name, size_t nameLen )
    {
        unsigned int hash = TumblerHash( name, nameLen );

        if ( driver->supports_ARB_vertex_program )
        {
            PARAMINFO_GET( hash, param_list );
        }

        return NULL;
    }

    void PushAvailableParams( lua_State *L, const glDriver *driver )
    {
        unsigned int n = 1;

        lua_createtable( L, 26, 0 );
        lua_pushlstring( L, "GL_PROGRAM_LENGTH_ARB", 21 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_PROGRAM_FORMAT_ARB", 21 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_PROGRAM_BINDING_ARB", 22 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_PROGRAM_INSTRUCTIONS_ARB", 27 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_MAX_PROGRAM_INSTRUCTIONS_ARB", 31 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_PROGRAM_NATIVE_INSTRUCTIONS_ARB", 34 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_MAX_PROGRAM_NATIVE_INSTRUCTIONS_ARB", 38 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_PROGRAM_TEMPORARIES_ARB", 26 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_MAX_PROGRAM_TEMPORARIES_ARB", 30 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_PROGRAM_NATIVE_TEMPORARIES_ARB", 33 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_MAX_PROGRAM_NATIVE_TEMPORARIES_ARB", 37 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_PROGRAM_PARAMETERS_ARB", 25 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_MAX_PROGRAM_PARAMETERS_ARB", 29 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_PROGRAM_NATIVE_PARAMETERS_ARB", 32 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_MAX_PROGRAM_NATIVE_PARAMETERS_ARB", 36 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_PROGRAM_ATTRIBS_ARB", 22 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_MAX_PROGRAM_ATTRIBS_ARB", 26 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_PROGRAM_NATIVE_ATTRIBS_ARB", 29 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_MAX_PROGRAM_NATIVE_ATTRIBS_ARB", 33 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_PROGRAM_ADDRESS_REGISTERS_ARB", 32 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_MAX_PROGRAM_ADDRESS_REGISTERS_ARB", 36 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_PROGRAM_NATIVE_ADDRESS_REGISTERS_ARB", 39 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_MAX_PROGRAM_NATIVE_ADDRESS_REGISTERS_ARB", 43 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_MAX_PROGRAM_LOCAL_PARAMETERS_ARB", 35 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_MAX_PROGRAM_ENV_PARAMETERS_ARB", 33 );
        lua_rawseti( L, -2, n++ );
        lua_pushlstring( L, "GL_PROGRAM_UNDER_NATIVE_LIMITS_ARB", 34 );
        lua_rawseti( L, -2, n++ );
    }
};

namespace gl_shaderARB
{
    const GLparaminfo* GetParamByName( const glDriver *driver, const char *name, size_t nameLen )
    {
        unsigned int hash = TumblerHash( name, nameLen );

        

        return NULL;
    }

    void PushAvailableParams( lua_State *L, const glDriver *driver )
    {
        unsigned int n = 1;

        
    }
};