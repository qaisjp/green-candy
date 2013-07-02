/*****************************************************************************
*
*  PROJECT:     Lua Interpreter
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        utils/png/png_main.cpp
*  PURPOSE:     MTA:Lua bitmap .png manager
*  DEVELOPERS:  Martin Turski <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#include <StdInc.h>
#include <libpng/png.h>

static void PNGAPI png_user_read_data( png_structp png, png_bytep buf, png_size_t length )
{
    ((CFile*)png_get_io_ptr( png ))->Read( buf, 1, length );
}

static void PNGAPI png_user_write_data( png_structp png, png_bytep buf, png_size_t length )
{
    ((CFile*)png_get_io_ptr( png ))->Write( buf, 1, length );
}

static void PNGAPI png_user_flush_data( png_structp png )
{
    ((CFile*)png_get_io_ptr( png ))->Flush();
}

struct png_exception
{
    png_exception( png_const_charp err )
    {
        png_err = err;
    }

    png_const_charp png_err;
};

static void PNGAPI png_user_error( png_structp png, png_const_charp err )
{
    throw png_exception( err );
}

static void PNGAPI png_user_warning( png_structp png, png_const_charp warn )
{
    return;
}

static LUA_DECLARE( load )
{
    CFile *file;

    LUA_ARGS_BEGIN;
    argStream.ReadClass( file, LUACLASS_FILE );
    LUA_ARGS_END;

    char sigbuf[8];
    file->Read( sigbuf, 1, sizeof(sigbuf) );

    if ( png_sig_cmp( (png_bytep)sigbuf, 0, sizeof(sigbuf) ) != 0 )
    {
        lua_pushboolean( L, false );
        lua_pushstring( L, "not a .png file" );
        return 2;
    }

    png_structp png_struct = NULL;
    png_infop png_info = NULL;
    png_infop png_end_info = NULL;

    try
    {
        png_struct = png_create_read_struct( PNG_LIBPNG_VER_STRING, NULL, png_user_error, png_user_warning );

        if ( !png_struct )
            throw lua_exception( L, LUA_ERRRUN, "failed to create libpng read struct" );

        png_info = png_create_info_struct( png_struct );

        if ( !png_info )
            throw lua_exception( L, LUA_ERRRUN, "failed to create libpng info struct" );

        png_set_sig_bytes( png_struct, sizeof(sigbuf) );
        png_set_read_fn( png_struct, file, png_user_read_data );
        png_read_png( png_struct, png_info, PNG_TRANSFORM_STRIP_16 | PNG_TRANSFORM_PACKING | PNG_TRANSFORM_EXPAND, NULL );

        png_uint_32 width, height;
        int bit_depth, color_type, interlace_method, compression_method, filter_method;

        // Grab .png picture information.
        png_get_IHDR( png_struct, png_info, &width, &height, &bit_depth, &color_type, &interlace_method, &compression_method, &filter_method );

        // looked at http://blog.nobel-joergensen.com/2010/11/07/loading-a-png-as-texture-in-opengl-using-libpng/
        Bitmap::eDataType dataType;

        switch( color_type )
        {
        case PNG_COLOR_TYPE_RGBA:       dataType = Bitmap::BITMAP_RGBA; break;
        case PNG_COLOR_TYPE_RGB:        dataType = Bitmap::BITMAP_RGB; break;
        default:
            throw png_exception( "invalid color type" );
        }

        png_bytepp rowPointers = png_get_rows( png_struct, png_info );

        unsigned int rowBytes = png_get_rowbytes( png_struct, png_info );
        size_t dataSize = rowBytes * height;
        char *image_data = new char[dataSize];

        for ( unsigned int i = 0; i < height; i++ )
        {
            // note that png is ordered top to
            // bottom, but OpenGL expects it bottom to top
            // so the order is swapped
            memcpy( image_data + ( rowBytes * i ), rowPointers[i], rowBytes );
        }

        Bitmap *map = new Bitmap( L, dataType, image_data, dataSize, width, height );
        map->PushStack( L );
        map->DisableKeepAlive();

        // Clean up information.
        png_destroy_read_struct( &png_struct, &png_info, &png_end_info );

        return 1;
    }
    catch( png_exception& e )
    {
        lua_pushboolean( L, false );
        lua_pushstring( L, e.png_err );

        png_destroy_read_struct( &png_struct, &png_info, &png_end_info );
        return 2;
    }
    catch( ... )
    {
        png_destroy_read_struct( &png_struct, &png_info, &png_end_info );
        throw;
    }

    return 0;
}

const static luaL_Reg png_library[] =
{
    LUA_METHOD( load ),
    { NULL, NULL }
};

void luapng_open( lua_State *L )
{
    luaL_openlib( L, "png", png_library, 0 );
    lua_pop( L, 1 );
}