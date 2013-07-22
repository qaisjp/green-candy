/*****************************************************************************
*
*  PROJECT:     Lua Interpreter
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        utils/dds/dds_main.cpp
*  PURPOSE:     MTA:Lua bitmap .DDS manager
*  DEVELOPERS:  Martin Turski <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#include <StdInc.h>
#include <d3d9.h>

struct DDS_PIXELFORMAT {
  DWORD dwSize;
  DWORD dwFlags;
  DWORD dwFourCC;
  DWORD dwRGBBitCount;
  DWORD dwRBitMask;
  DWORD dwGBitMask;
  DWORD dwBBitMask;
  DWORD dwABitMask;
};

#define DDSD_CAPS           0x00000001
#define DDSD_HEIGHT         0x00000002
#define DDSD_WIDTH          0x00000004
#define DDSD_PITCH          0x00000008
#define DDSD_PIXELFORMAT    0x00001000
#define DDSD_MIPMAPCOUNT    0x00020000
#define DDSD_LINEARSIZE     0x00080000
#define DDSD_DEPTH          0x00800000

#define DDSCAPS_COMPLEX     0x00000008
#define DDSCAPS_MIPMAP      0x00400000
#define DDSCAPS_TEXTURE     0x00004000

typedef struct {
  DWORD           dwSize;
  DWORD           dwFlags;
  DWORD           dwHeight;
  DWORD           dwWidth;
  DWORD           dwPitchOrLinearSize;
  DWORD           dwDepth;
  DWORD           dwMipMapCount;
  DWORD           dwReserved1[11];
  DDS_PIXELFORMAT ddspf;
  DWORD           dwCaps;
  DWORD           dwCaps2;
  DWORD           dwCaps3;
  DWORD           dwCaps4;
  DWORD           dwReserved2;
} DDS_HEADER;

template <class blockType>
__forceinline void AdjustDXTImage( void *dst, void *src, unsigned int& width, unsigned int& height )
{
    unsigned int _width = width;
    unsigned int _height = height;

    unsigned int pitch = max( 1, ((_width+3)/4) ) * sizeof( blockType );

    blockType *srcData = (blockType*)src;
    blockType *dstData = (blockType*)dst;

    unsigned int dxtWidth = _width / 4;
    unsigned int dxtHeight = _height / 4;

    for ( unsigned int y = 0; y < dxtHeight; y++ )
    {
        for ( unsigned int x = 0; x < dxtWidth; x++ )
        {
            unsigned int revX = dxtWidth - x - 1;
            unsigned int revY = dxtHeight - y - 1;

            dstData[y + x * dxtWidth] = *(blockType*)( (char*)( srcData + revX ) + revY * pitch );
        }
    }

    width = _height;
    height = _width;
}

static LUA_DECLARE( load )
{
    CFile *file;

    LUA_ARGS_BEGIN;
    argStream.ReadClass( file, LUACLASS_FILE );
    LUA_ARGS_END;

    // Check that we are really reading a DDS file.
    LUA_CHECK( file->ReadInt() == ' SDD' );

    DDS_HEADER header;

    LUA_CHECK( file->ReadStruct( header ) );
    LUA_CHECK( header.dwSize == sizeof(DDS_HEADER) );
    LUA_CHECK( header.dwFlags & ( DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT | DDSD_PIXELFORMAT ) );
    //LUA_CHECK( header.dwCaps & ( DDSCAPS_TEXTURE ) );

    size_t fileSize = file->GetSize();
    
    size_t sRead;
    Bitmap::eDataType dataType;

    switch( header.ddspf.dwFourCC )
    {
    case D3DFMT_DXT1:
        sRead = header.dwWidth * header.dwHeight / 2;
        dataType = Bitmap::BITMAP_DXT1_RGBA;
        break;
    case D3DFMT_DXT3:
        sRead = header.dwWidth * header.dwHeight;
        dataType = Bitmap::BITMAP_DXT3_RGBA;
        break;
    case D3DFMT_DXT5:
        sRead = header.dwWidth * header.dwHeight;
        dataType = Bitmap::BITMAP_DXT5_RGBA;
        break;
    default:
        lua_pushboolean( L, false );
        return 1;
    }

    void *data = new char[sRead];
    if ( file->Read( data, 1, sRead ) != sRead )
    {
        delete data;

        lua_pushboolean( L, false );
        return 1;
    }

    void *dst = new char[sRead];
    unsigned int _width = header.dwWidth;
    unsigned int _height = header.dwHeight;

    switch( dataType )
    {
    case Bitmap::BITMAP_DXT1_RGB:
    case Bitmap::BITMAP_DXT1_RGBA:
        AdjustDXTImage <long long> ( dst, data, _width, _height );
        break;
    case Bitmap::BITMAP_DXT3_RGBA:
        AdjustDXTImage <s3tc_dxt3_block> ( dst, data, _width, _height );
        break;
    case Bitmap::BITMAP_DXT5_RGBA:
        AdjustDXTImage <s3tc_dxt5_block> ( dst, data, _width, _height );
        break;
    }

    delete data;

    Bitmap *map = new BitmapCopy( L, dataType, dst, sRead, _width, _height );
    map->PushStack( L );
    map->DisableKeepAlive();
    return 1;
}

static const luaL_Reg dds_library[] =
{
    LUA_METHOD( load ),
    { NULL, NULL }
};

void luadds_open( lua_State *L )
{
    luaL_openlib( L, "dds", dds_library, 0 );
    lua_pop( L, 1 );
}