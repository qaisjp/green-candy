/*****************************************************************************
*
*  PROJECT:     Lua Interpreter
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        lfreetype/lft_main.cpp
*  PURPOSE:     Freetype management library for MTA:Lua
*  DEVELOPERS:  Martin Turski <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#include <StdInc.h>
#include FT_GLYPH_H
#include FT_BITMAP_H

const static wchar_t unicode_ascii_table[] =
{
    L'\x00', L'\x01', L'\x02', L'\x03', L'\x04', L'\x05', L'\x06', L'\x07', L'\x08', L'\x09', L'\x0a', L'\x0b', L'\x0c', L'\x0d', L'\x0e', L'\x0f', L'\x10', L'\x11', L'\x12', L'\x13', L'\x14', L'\x15', L'\x16', L'\x17', L'\x18', L'\x19', L'\x1a', L'\x1b', L'\x1c', L'\x1d', L'\x1e', 
    L'\x1f', L'\x20', L'\x21', L'\x22', L'\x23', L'\x24', L'\x25', L'\x26', L'\x27', L'\x28', L'\x29', L'\x2a', L'\x2b', L'\x2c', L'\x2d', L'\x2e', L'\x2f', L'\x30', L'\x31', L'\x32', L'\x33', L'\x34', L'\x35', L'\x36', L'\x37', L'\x38', L'\x39', L'\x3a', L'\x3b', L'\x3c', L'\x3d', 
    L'\x3e', L'\x3f', L'\x40', L'\x41', L'\x42', L'\x43', L'\x44', L'\x45', L'\x46', L'\x47', L'\x48', L'\x49', L'\x4a', L'\x4b', L'\x4c', L'\x4d', L'\x4e', L'\x4f', L'\x50', L'\x51', L'\x52', L'\x53', L'\x54', L'\x55', L'\x56', L'\x57', L'\x58', L'\x59', L'\x5a', L'\x5b', L'\x5c', 
    L'\x5d', L'\x5e', L'\x5f', L'\x60', L'\x61', L'\x62', L'\x63', L'\x64', L'\x65', L'\x66', L'\x67', L'\x68', L'\x69', L'\x6a', L'\x6b', L'\x6c', L'\x6d', L'\x6e', L'\x6f', L'\x70', L'\x71', L'\x72', L'\x73', L'\x74', L'\x75', L'\x76', L'\x77', L'\x78', L'\x79', L'\x7a', L'\x7b', 
    L'\x7c', L'\x7d', L'\x7e', L'\x7f', L'\x80', L'\x81', L'\x82', L'\x83', L'\x84', L'\x85', L'\x86', L'\x87', L'\x88', L'\x89', L'\x8a', L'\x8b', L'\x8c', L'\x8d', L'\x8e', L'\x8f', L'\x90', L'\x91', L'\x92', L'\x93', L'\x94', L'\x95', L'\x96', L'\x97', L'\x98', L'\x99', L'\x9a', 
    L'\x9b', L'\x9c', L'\x9d', L'\x9e', L'\x9f', L'\xa0', L'\xa1', L'\xa2', L'\xa3', L'\xa4', L'\xa5', L'\xa6', L'\xa7', L'\xa8', L'\xa9', L'\xaa', L'\xab', L'\xac', L'\xad', L'\xae', L'\xaf', L'\xb0', L'\xb1', L'\xb2', L'\xb3', L'\xb4', L'\xb5', L'\xb6', L'\xb7', L'\xb8', L'\xb9', 
    L'\xba', L'\xbb', L'\xbc', L'\xbd', L'\xbe', L'\xbf', L'\xc0', L'\xc1', L'\xc2', L'\xc3', L'\xc4', L'\xc5', L'\xc6', L'\xc7', L'\xc8', L'\xc9', L'\xca', L'\xcb', L'\xcc', L'\xcd', L'\xce', L'\xcf', L'\xd0', L'\xd1', L'\xd2', L'\xd3', L'\xd4', L'\xd5', L'\xd6', L'\xd7', L'\xd8', 
    L'\xd9', L'\xda', L'\xdb', L'\xdc', L'\xdd', L'\xde', L'\xdf', L'\xe0', L'\xe1', L'\xe2', L'\xe3', L'\xe4', L'\xe5', L'\xe6', L'\xe7', L'\xe8', L'\xe9', L'\xea', L'\xeb', L'\xec', L'\xed', L'\xee', L'\xef', L'\xf0', L'\xf1', L'\xf2', L'\xf3', L'\xf4', L'\xf5', L'\xf6', L'\xf7', 
    L'\xf8', L'\xf9', L'\xfa', L'\xfb', L'\xfc', L'\xfd', L'\xfe', L'\xff'
};

namespace lfreetypeface
{
    static LUA_DECLARE( __index )
    {
        LFreeTypeFace *face = (LFreeTypeFace*)lua_getmethodtrans( L );

        lua_String name;
        size_t len;
        
        name = lua_tolstring( L, 2, &len );

        if ( !name )
            return 0;

        unsigned int hash = TumblerHash( name, len );

        FT_Face handle = face->handle;

        // General fields
        if ( hash == 0x34b9627d )       lua_pushstring( L, handle->family_name );
        else if ( hash == 0xceea541d )  lua_pushstring( L, handle->style_name );
        else if ( hash == 0xa2bf54fd )  lua_pushnumber( L, handle->num_glyphs );
        else if ( hash == 0x2ad5189c )  lua_pushnumber( L, handle->num_faces );
        else if ( hash == 0xb5f80039 )  lua_pushnumber( L, handle->face_index );
        else if ( hash == 0x4fe29cc8 )  lua_pushnumber( L, handle->face_flags );
        else if ( hash == 0xba86492f )  lua_pushnumber( L, handle->style_flags );
        else if ( hash == 0x949c671d )  lua_pushinteger( L, handle->num_fixed_sizes );
        else if ( hash == 0x3cdf26ab )  lua_pushinteger( L, handle->num_charmaps );
        else if ( hash == 0x66cea338 )  lua_pushinteger( L, handle->units_per_EM );
        else if ( hash == 0xc3901a32 )  lua_pushinteger( L, handle->ascender );
        else if ( hash == 0x9aada984 )  lua_pushinteger( L, handle->descender );
        else if ( hash == 0xa5ab598b )  lua_pushinteger( L, handle->height );
        else if ( hash == 0xfe90175b )  lua_pushinteger( L, handle->max_advance_width );
        else if ( hash == 0x14b254be )  lua_pushinteger( L, handle->max_advance_height );
        else if ( hash == 0x09bd2d4d )  lua_pushinteger( L, handle->underline_position );
        else if ( hash == 0x8bae3638 )  lua_pushinteger( L, handle->underline_thickness );
        // Advanced fields
        else if ( hash == 0xb3faf66b )
        {
            lua_createtable( L, handle->num_fixed_sizes, 0 );

            for ( int n = 0; n < handle->num_fixed_sizes; n++ )
            {
                const FT_Bitmap_Size& size = handle->available_sizes[n];

                lua_newtable( L );
                lua_pushinteger( L, size.width ); lua_setfield( L, -2, "width" );
                lua_pushinteger( L, size.height ); lua_setfield( L, -2, "height" );
                lua_pushnumber( L, size.size ); lua_setfield( L, -2, "size" );
                lua_pushnumber( L, size.x_ppem ); lua_setfield( L, -2, "x_ppem" );
                lua_pushnumber( L, size.y_ppem ); lua_setfield( L, -2, "y_ppem" );
                lua_rawseti( L, -2, n + 1 );
            }
        }
        else goto skip;

        return 1;

skip:
        face->GetClass()->PushOuterEnvironment( L );
        lua_pushvalue( L, 2 );
        lua_gettable( L, -2 );
        return 1;
    }

    static LUA_DECLARE( setCharSize )
    {
        FT_F26Dot6 char_width, char_height;
        unsigned int horz_resolution, vert_resolution;

        LUA_ARGS_BEGIN;
        argStream.ReadNumber( char_width ); argStream.ReadNumber( char_height );
        argStream.ReadNumber( horz_resolution ); argStream.ReadNumber( vert_resolution );
        LUA_ARGS_END;

        LFreeTypeFace *face = (LFreeTypeFace*)lua_getmethodtrans( L );

        lua_pushinteger( L, FT_Set_Char_Size( face->handle, char_width, char_height, horz_resolution, vert_resolution ) );
        return 1;
    }

    static LUA_DECLARE( setPixelSizes )
    {
        unsigned int pixel_width, pixel_height;

        LUA_ARGS_BEGIN;
        argStream.ReadNumber( pixel_width );
        argStream.ReadNumber( pixel_height );
        LUA_ARGS_END;

        LFreeTypeFace *face = (LFreeTypeFace*)lua_getmethodtrans( L );

        lua_pushinteger( L, FT_Set_Pixel_Sizes( face->handle, pixel_width, pixel_height ) );
        return 1;
    }

    static LUA_DECLARE( getCharIndexASCII )
    {
        unsigned char asciiChar;
        int luaType = lua_type( L, 1 );

        if ( luaType == LUA_TSTRING )
        {
            size_t len;
            const char *string = lua_tolstring( L, 1, &len );

            LUA_CHECK( string );

            unsigned int index;

            if ( lua_type( L, 2 ) == LUA_TNUMBER )
                index = lua_tointeger( L, 2 );
            else
                index = 0;

            LUA_CHECK( index < len );

            asciiChar = (unsigned char)string[index];
        }
        else if ( luaType == LUA_TNUMBER )
            asciiChar = lua_tointeger( L, 1 );
        else
        {
            lua_pushboolean( L, false );
            return 1;
        }

        wchar_t charCode = unicode_ascii_table[asciiChar];

        LFreeTypeFace *face = (LFreeTypeFace*)lua_getmethodtrans( L );

        lua_pushnumber( L, FT_Get_Char_Index( face->handle, charCode ) );
        return 1;
    }

    static LUA_DECLARE( getCharIndex )
    {
        FT_ULong charIndex;

        LUA_ARGS_BEGIN;
        argStream.ReadNumber( charIndex );
        LUA_ARGS_END;

        LFreeTypeFace *face = (LFreeTypeFace*)lua_getmethodtrans( L );

        lua_pushnumber( L, FT_Get_Char_Index( face->handle, charIndex ) );
        return 1;
    }

    static LUA_DECLARE( loadGlyph )
    {
        FT_UInt charIndex;
        FT_Int32 flags;

        LUA_ARGS_BEGIN;
        argStream.ReadNumber( charIndex );
        argStream.ReadNumber( flags, 0 );
        LUA_ARGS_END;

        LFreeTypeFace *face = (LFreeTypeFace*)lua_getmethodtrans( L );

        lua_pushinteger( L, FT_Load_Glyph( face->handle, charIndex, flags ) );
        return 1;
    }

    static LUA_DECLARE( getGlyphBitmap )
    {
        LFreeTypeFace *face = (LFreeTypeFace*)lua_getmethodtrans( L );
        FT_GlyphSlot glyph = face->handle->glyph;

        if ( glyph->format != FT_GLYPH_FORMAT_BITMAP )
            FT_Render_Glyph( glyph, FT_RENDER_MODE_NORMAL );

        LFreeType *sys = face->parent;
        const FT_Bitmap& bmp = glyph->bitmap;

        unsigned int width = (unsigned int)bmp.width;
        unsigned int height = (unsigned int)bmp.rows;

        union
        {
            void *data;
            unsigned int *uiData;
        };

        Bitmap::eDataType dataType;
        size_t dataSize;

        // Convert the freetype bitmap to a general-purpose RGBA bitmap
        switch( bmp.pixel_mode )
        {
        case FT_PIXEL_MODE_GRAY:
            uiData = new unsigned int[width * height];
            dataSize = width * height * sizeof(unsigned int);
            dataType = Bitmap::BITMAP_RGBA;

            for ( unsigned int y = 0; y < height; y++ )
            {
                for ( unsigned int x = 0; x < width; x++ )
                {
                    uiData[x + y * width] =
                        (unsigned int)( (double)bmp.buffer[x + y * width] / 255 * 0xFFFFFFFF );
                }
            }
            break;
        default:
            lua_pushboolean( L, false );
            return 1;
        }

        BitmapCopy *map = new BitmapCopy( L, dataType, data, dataSize, width, height );
        map->PushStack( L );
        map->DisableKeepAlive();
        return 1;
    }

    static LUA_DECLARE( getGlyphBitmapOffset )
    {
        LFreeTypeFace *face = (LFreeTypeFace*)lua_getmethodtrans( L );
        FT_GlyphSlot glyph = face->handle->glyph;

        lua_pushinteger( L, glyph->bitmap_left );
        lua_pushinteger( L, glyph->bitmap_top );
        return 2;
    }

    static LUA_DECLARE( getGlyphAdvance )
    {
        LFreeTypeFace *face = (LFreeTypeFace*)lua_getmethodtrans( L );
        FT_GlyphSlot glyph = face->handle->glyph;

        lua_pushinteger( L, glyph->advance.x );
        lua_pushinteger( L, glyph->advance.y );
        return 2;
    }

    static LUA_DECLARE( getGlyphMetricsHoriBearing )
    {
        LFreeTypeFace *face = (LFreeTypeFace*)lua_getmethodtrans( L );
        FT_GlyphSlot glyph = face->handle->glyph;

        lua_pushinteger( L, glyph->metrics.horiBearingX );
        lua_pushinteger( L, glyph->metrics.horiBearingY );
        return 2;
    }

    static LUA_DECLARE( getGlyphBoundingBox )
    {
        LFreeTypeFace *face = (LFreeTypeFace*)lua_getmethodtrans( L );
        FT_GlyphSlot slot = face->handle->glyph;
        FT_Glyph glyph;
        FT_BBox box;

        FT_Get_Glyph( slot, &glyph );

        FT_Glyph_Get_CBox( glyph, 0, &box );

        FT_Done_Glyph( glyph );

        lua_pushnumber( L, box.xMin );
        lua_pushnumber( L, box.yMin );
        lua_pushnumber( L, box.xMax );
        lua_pushnumber( L, box.yMax );
        return 4;
    }

    static LUA_DECLARE( getSizeMetrics )
    {
        LFreeTypeFace *face = (LFreeTypeFace*)lua_getmethodtrans( L );
        const FT_Size_Metrics& metrics = face->handle->size->metrics;

        lua_newtable( L );
        lua_pushinteger( L, metrics.x_ppem );
        lua_setfield( L, -2, "x_ppem" );
        lua_pushinteger( L, metrics.y_ppem );
        lua_setfield( L, -2, "y_ppem" );
        lua_pushnumber( L, metrics.x_scale );
        lua_setfield( L, -2, "x_scale" );
        lua_pushnumber( L, metrics.y_scale );
        lua_setfield( L, -2, "y_scale" );
        lua_pushnumber( L, metrics.ascender );
        lua_setfield( L, -2, "ascender" );
        lua_pushnumber( L, metrics.descender );
        lua_setfield( L, -2, "descender" );
        lua_pushnumber( L, metrics.height );
        lua_setfield( L, -2, "height" );
        lua_pushnumber( L, metrics.max_advance );
        lua_setfield( L, -2, "max_advance" );
        return 1;
    }

    static const luaL_Reg face_interface[] =
    {
        LUA_METHOD( __index ),
        LUA_METHOD( setCharSize ),
        LUA_METHOD( setPixelSizes ),
        LUA_METHOD( getCharIndexASCII ),
        LUA_METHOD( getCharIndex ),
        LUA_METHOD( loadGlyph ),
        LUA_METHOD( getGlyphBitmap ),
        LUA_METHOD( getGlyphBitmapOffset ),
        LUA_METHOD( getGlyphAdvance ),
        LUA_METHOD( getGlyphMetricsHoriBearing ),
        LUA_METHOD( getGlyphBoundingBox ),
        LUA_METHOD( getSizeMetrics ),
        { NULL, NULL }
    };

    static LUA_DECLARE( constructor )
    {
        LFreeTypeFace *face = (LFreeTypeFace*)lua_touserdata( L, lua_upvalueindex( 1 ) );

        ILuaClass *j = lua_refclass( L, LUA_ENVIRONINDEX );
        j->SetTransmit( LUACLASS_FREETYPEFACE, face );

        j->RegisterInterfaceTrans( L, face_interface, 0, LUACLASS_FREETYPEFACE );

        lua_pushcstring( L, "freetype_face" );
        lua_setfield( L, LUA_ENVIRONINDEX, "__type" );
        return 0;
    }
};

LFreeTypeFace::LFreeTypeFace( lua_State *L, FT_Face face, ILuaClass *file, FT_Stream stream, LFreeType *sys ) : LuaInstance( L )
{
    file->IncrementMethodStack( L );

    Extend( L, lfreetypeface::constructor );

    parent = sys;
    handle = face;
    fileClass = file;
    source = stream;
}

LFreeTypeFace::~LFreeTypeFace( void )
{
    FT_Done_Face( handle );

    fileClass->DecrementMethodStack( m_lua );

    // Delete stream descriptor
    delete source;
}

namespace lfreetype
{
    static unsigned long LFreeType_StreamRead( FT_Stream stream, unsigned long offset, unsigned char *buf, unsigned long count )
    {
        if ( count == 0 )
        {
            ((CFile*)stream->descriptor.pointer)->Seek( (long)offset, SEEK_SET );
            return 0;
        }

        return ((CFile*)stream->descriptor.pointer)->Read( buf, 1, count );
    }

    static LUA_DECLARE( createFace )
    {
        CFile *src;
        FT_Long index;

        LUA_ARGS_BEGIN;
        argStream.ReadClass( src, LUACLASS_FILE );
        argStream.ReadNumber( index, 0 );
        LUA_ARGS_END;

        LUA_CHECK( index >= 0 );

        LFreeType *sys = (LFreeType*)lua_getmethodtrans( L );
        FT_Face face;

        // Make a freetype stream
        FT_StreamRec *stream = new FT_StreamRec;
        memset( stream, 0, sizeof(*stream) );
        stream->descriptor.pointer = src;
        stream->read = LFreeType_StreamRead;
        stream->pos = 0;
        stream->size = src->GetSize();

        FT_Open_Args args;
        args.flags = FT_OPEN_STREAM;
        args.stream = stream;

        int error = FT_Open_Face( sys->handle, &args, index, &face );

        if ( error != 0 )
        {
            delete stream;

            lua_pushboolean( L, false );
            return 1;
        }

        LFreeTypeFace *obj = new LFreeTypeFace( L, face, lua_refclass( L, 1 ), stream, sys );
        obj->SetParent( sys );
        obj->PushStack( L );
        obj->Finalize( L );
        return 1;
    }

    static const luaL_Reg freetype_interface[] =
    {
        LUA_METHOD( createFace ),
        { NULL, NULL }
    };

    static LUA_DECLARE( constructor )
    {
        LFreeType *ft = (LFreeType*)lua_touserdata( L, lua_upvalueindex( 1 ) );

        ILuaClass *j = lua_refclass( L, LUA_ENVIRONINDEX );
        j->SetTransmit( LUACLASS_FREETYPE, ft );

        j->RegisterInterfaceTrans( L, freetype_interface, 0, LUACLASS_FREETYPE );

        lua_pushcstring( L, "freetype" );
        lua_setfield( L, LUA_ENVIRONINDEX, "__type" );
        return 0;
    }
}

LFreeType::LFreeType( lua_State *L ) : LuaInstance( L )
{
    // Instance MTA:Lua class
    Extend( L, lfreetype::constructor );

    FT_Init_FreeType( &handle );
}

LFreeType::~LFreeType( void )
{
    FT_Done_FreeType( handle );
}

static LUA_DECLARE( create )
{
    LFreeType *ft = new LFreeType( L );
    ft->PushStack( L );
    ft->DisableKeepAlive();
    return 1;
}

const static luaL_Reg freetype_library[] =
{
    LUA_METHOD( create ),
    { NULL, NULL }
};

void luafreetype_open( lua_State *L )
{
    luaL_openlib( L, "ft", freetype_library, 0 );
    lua_pop( L, 1 );
}