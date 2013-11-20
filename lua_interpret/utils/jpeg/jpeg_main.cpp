/*****************************************************************************
*
*  PROJECT:     Lua Interpreter
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        utils/jpeg/jpeg_main.cpp
*  PURPOSE:     MTA:Lua bitmap .jpg manager
*  DEVELOPERS:  Martin Turski <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#include <StdInc.h>
#include <jpeglib.h>
#include <jerror.h>

struct LuaJPEGErrorManager : public jpeg_error_mgr
{
    lua_State *state;
};

struct LuaJPEGError
{
    LuaJPEGError( const char *err ) : msg( err )
    { }

    const char *msg;
};

static void lua_jpeg_error_handler( j_common_ptr cinfo )
{
    LuaJPEGErrorManager *info = (LuaJPEGErrorManager*)cinfo->err;

    throw LuaJPEGError( "jpeg processing error" );
}

static JOCTET FakeEOI[] = { 0xFF, JPEG_EOI };

struct LuaJPEGSourceManager : public jpeg_source_mgr
{
    static void lua_jpeg_init_source( j_decompress_ptr info )
    {
        LuaJPEGSourceManager& man = *(LuaJPEGSourceManager*)info->src;

        man.bytes_in_buffer = 0;
        man.next_input_byte = NULL;
    }

    static boolean lua_jpeg_fill_input_buffer( j_decompress_ptr info )
    {
        LuaJPEGSourceManager& src = *(LuaJPEGSourceManager*)info->src;

        size_t numRead = src.m_file->Read( src.temp_buf, 1, sizeof(src.temp_buf) );

        if ( numRead == 0 )
        {
            // Did the stream truly end?
            if ( !src.m_file->IsEOF() )
                return false;

            WARNMS( info, JWRN_JPEG_EOF );
            
            src.temp_buf[0] = 0xFF;
            src.temp_buf[1] = JPEG_EOI;
            numRead = 2;
        }

        src.next_input_byte = src.temp_buf;
        src.bytes_in_buffer = numRead;
        return true;
    }

    static void lua_jpeg_skip_input_data( j_decompress_ptr info, long num_bytes )
    {
        LuaJPEGSourceManager& src = *(LuaJPEGSourceManager*)info->src;

        if ( src.bytes_in_buffer > 0 )
        {
            if ( src.bytes_in_buffer < (size_t)num_bytes )
            {
                num_bytes -= src.bytes_in_buffer;
                src.bytes_in_buffer = 0;
            }
            else
            {
                src.bytes_in_buffer -= num_bytes;

                if ( src.bytes_in_buffer == 0 )
                    src.next_input_byte = NULL;
                else
                    src.next_input_byte += num_bytes;

                num_bytes = 0;
            }

            if ( src.bytes_in_buffer == 0 )
                src.next_input_byte = NULL;
        }

        if ( num_bytes != 0 )
            src.m_file->Seek( num_bytes, SEEK_CUR );
    }

    static boolean lua_jpeg_resync_to_restart( j_decompress_ptr info, int desired )
    {
        return jpeg_resync_to_restart( info, desired );
    }

    static void lua_jpeg_term_source( j_decompress_ptr info )
    {
        return;
    }

    LuaJPEGSourceManager( CFile *src )
    {
        bytes_in_buffer = 0;
        next_input_byte = NULL;

        init_source = lua_jpeg_init_source;
        fill_input_buffer = lua_jpeg_fill_input_buffer;
        skip_input_data = lua_jpeg_skip_input_data;
        resync_to_restart = lua_jpeg_resync_to_restart;
        term_source = lua_jpeg_term_source;

        m_file = src;
    }

    ~LuaJPEGSourceManager( void )
    {
    }

    CFile *m_file;
    unsigned char temp_buf[4096];
};

static LUA_DECLARE( load )
{
    CFile *file;

    LUA_ARGS_BEGIN;
    argStream.ReadClass( file, LUACLASS_FILE );
    LUA_ARGS_END;

    jpeg_decompress_struct readStruct;

    LuaJPEGErrorManager error_mgr;
    error_mgr.state = L;

    // Set up default error manager but overwrite error out handler
    readStruct.err = jpeg_std_error( &error_mgr );
    error_mgr.error_exit = lua_jpeg_error_handler;

    unsigned char *data = NULL;
    size_t dataSize = 0;

    try
    {
        // Allocate decompression information
        jpeg_create_decompress( &readStruct );
    }
    catch( LuaJPEGError& )
    {
        lua_pushboolean( L, false );
        lua_pushcstring( L, "failed to create decompress struct" );
        return 2;
    }

    try
    {
        // Set up custom file source manager
        LuaJPEGSourceManager srcman( file );
        readStruct.src = &srcman;

        jpeg_read_header( &readStruct, true );

        jpeg_start_decompress( &readStruct );

        // JPEG files have to be RGB; alpha is not supported by the standard
        if ( readStruct.output_components != 3 )
        {
            jpeg_destroy_decompress( &readStruct );

            lua_pushboolean( L, false );
            lua_pushcstring( L, "invalid number of jpeg color components" );
            return 2;
        }

        // Depth of three
        unsigned int row_stride = readStruct.output_width * 3;
        unsigned int minHeight = readStruct.rec_outbuf_height;
        unsigned char **buffer = new unsigned char*[minHeight];

        // Allocated JPEG rows
        for ( unsigned int n = 0; n < minHeight; n++ )
            buffer[n] = new unsigned char[row_stride * sizeof(JSAMPLE)];

        // Create the image data array
        dataSize = 3 * readStruct.output_width * readStruct.output_height;
        data = new unsigned char[dataSize];

        while ( readStruct.output_scanline < readStruct.output_height )
        {
            jpeg_read_scanlines( &readStruct, buffer, minHeight );

            for ( unsigned int n = 0; n < minHeight; n++ )
                memcpy( data + ( ( readStruct.output_scanline - 1 ) * row_stride ) + n * row_stride, buffer[n], row_stride );
        }

        // Free allocated rows
        for ( unsigned int n = 0; n < minHeight; n++ )
            delete [] buffer[n];

        delete [] buffer;

        // Finish the decompression unit
        jpeg_finish_decompress( &readStruct );
    }
    catch( LuaJPEGError& e )
    {
        // Catch any runtime error, such as invalid jpeg file
        lua_pushboolean( L, false );
        lua_pushstring( L, e.msg );

        jpeg_destroy_decompress( &readStruct );
        return 2;
    }
    catch( ... )
    {
        // Other errors have to be passed along, but destroy the decompression runtime
        jpeg_destroy_decompress( &readStruct );
        throw;
    }

    // Create our bitmap and return it, since everything went well
    Bitmap *map = new BitmapCopy( L, Bitmap::BITMAP_RGB, data, dataSize, readStruct.output_width, readStruct.output_height );
    map->PushStack( L );
    map->DisableKeepAlive();

    // Deallocate decompression information.
    jpeg_destroy_decompress( &readStruct );

    return 1;
}

const static luaL_Reg jpeg_library[] =
{
    LUA_METHOD( load ),
    { NULL, NULL }
};

void luajpeg_open( lua_State *L )
{
    luaL_openlib( L, "jpeg", jpeg_library, 0 );
    lua_pop( L, 1 );
}