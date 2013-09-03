/*****************************************************************************
*
*  PROJECT:     Lua Interpreter
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        utils/Bitmap.cpp
*  PURPOSE:     MTA:Lua bitmap instance for image data traversal
*  DEVELOPERS:  Martin Turski <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#include <StdInc.h>

static LUA_DECLARE( getWidth )
{
    Bitmap *map = (Bitmap*)lua_getmethodtrans( L );

    lua_pushinteger( L, map->m_width );
    return 1;
}

static LUA_DECLARE( getHeight )
{
    Bitmap *map = (Bitmap*)lua_getmethodtrans( L );

    lua_pushinteger( L, map->m_height );
    return 1;
}

static LUA_DECLARE( getSize )
{
    Bitmap *map = (Bitmap*)lua_getmethodtrans( L );

    lua_pushinteger( L, map->m_dataSize );
    return 1;
}

enum eDrawOpType
{
    DRAWOP_MODULATE,
    DRAWOP_ADDITIVE,
    DRAWOP_WRITE
};

DECLARE_ENUM( eDrawOpType );

IMPLEMENT_ENUM_BEGIN( eDrawOpType )
    ADD_ENUM( DRAWOP_MODULATE, "modulate" )
    ADD_ENUM( DRAWOP_ADDITIVE, "additive" )
    ADD_ENUM( DRAWOP_WRITE, "write" )
IMPLEMENT_ENUM_END( "eDrawOpType" )

__forceinline void UnpackColor( const void *data, unsigned int pixelX, unsigned int pixelY, unsigned int width, Bitmap::eDataType dataType, double& red, double& green, double& blue, double& alpha )
{
    switch( dataType )
    {
    case Bitmap::BITMAP_RGB:
        {
            const BitmapRGB& color = *( (const BitmapRGB*)data + pixelX + pixelY * width );
            red = color.r / 255.0f;
            green = color.g / 255.0f;
            blue = color.b / 255.0f;
            alpha = 1.0f;
        }
        break;
    case Bitmap::BITMAP_RGBA:
        {
            const RwColor& color = *( (const RwColor*)data + pixelX + pixelY * width );
            red = color.r / 255.0f;
            green = color.g / 255.0f;
            blue = color.b / 255.0f;
            alpha = color.a / 255.0f;
        }
        break;
    case Bitmap::BITMAP_DXT1_RGBA:
        {
            const s3tc_dxt1_block& color = *( (const s3tc_dxt1_block*)data + ( ( pixelX / 4 ) + ( pixelY / 4 ) * ( width / 4 ) ) );
            red = (double)color.color1.red / 31.0f;
            green = (double)color.color1.green / 63.0f;
            blue = (double)color.color1.blue / 31.0f;
            alpha = 1.0f;
        }
        break;
    default:
        red = 0.0f;
        green = 0.0f;
        blue = 0.0f;
        alpha = 0.0f;
        break;
    }
}

__forceinline void PackColor( void *data, unsigned int pixelX, unsigned int pixelY, unsigned int width, Bitmap::eDataType dataType, double red, double green, double blue, double alpha )
{
    switch( dataType )
    {
    case Bitmap::BITMAP_RGB:
        {
            BitmapRGB& color = *( (BitmapRGB*)data + pixelX + pixelY * width );
            color.r = (unsigned char)( red * 255.0f );
            color.g = (unsigned char)( green * 255.0f );
            color.b = (unsigned char)( blue * 255.0f );
        }
        break;
    case Bitmap::BITMAP_RGBA:
        {
            RwColor& color = *( (RwColor*)data + pixelX + pixelY * width );
            color.r = (unsigned char)( red * 255.0f );
            color.g = (unsigned char)( green * 255.0f );
            color.b = (unsigned char)( blue * 255.0f );
            color.a = (unsigned char)( alpha * 255.0f );
        }
        break;
    case Bitmap::BITMAP_DXT1_RGBA:
        {
            s3tc_dxt1_block& color = *( (s3tc_dxt1_block*)data + ( ( pixelX / 4 ) + ( pixelY / 4 ) * ( width / 4 ) ) );
            color.color0.red = (unsigned short)( red * 31.0f );
            color.color0.green = (unsigned short)( green * 63.0f );
            color.color0.blue = (unsigned short)( blue * 31.0f );
        }
        break;
    }
}

static LUA_DECLARE( drawBitmap )
{
    Bitmap *src;
    double x, y;
    double width, height;
    eDrawOpType drawOp;

    LUA_ARGS_BEGIN;
    argStream.ReadClass( src, LUACLASS_BITMAP );
    argStream.ReadNumber( x );
    argStream.ReadNumber( y );
    argStream.ReadNumber( width, 0 );
    argStream.ReadNumber( height, 0 );
    argStream.ReadEnumString( drawOp, DRAWOP_MODULATE );
    LUA_ARGS_END;

    // Establish draw operation scales.
    double xScale, yScale;

    if ( width == 0 )
    {
        xScale = 1.0f;
        width = src->GetWidth();
    }
    else
        xScale = width / (float)src->GetWidth();

    if ( height == 0 )
    {
        yScale = 1.0f;
        height = src->GetHeight();
    }
    else
        yScale = height / (float)src->GetHeight();

    // For now; later we may support flipping of images!
    LUA_CHECK( width > 0 && height > 0 );

    Bitmap *map = (Bitmap*)lua_getmethodtrans( L );

    // Cache the bitmap types
    Bitmap::eDataType srcDataType = src->m_dataType;
    Bitmap::eDataType dstDataType = map->m_dataType;

    // We cannot draw us to ourselves.
    LUA_CHECK( map != src );

    unsigned int srcWidth = src->GetWidth();
    unsigned int srcHeight = src->GetHeight();
    unsigned int dstWidth = map->GetWidth();
    unsigned int dstHeight = map->GetHeight();

    // Make sure the draw operations target valid zones
    double endTargetX = x + width;
    double endTargetY = y + height;

    // Is the draw zone clipping out of the target zone?
    // Do not render then.
    LUA_CHECK( endTargetX > 0 && endTargetY > 0 &&
               x < dstWidth   && y < dstHeight );

    // Draw it!
    unsigned int ySrcPos = 0;

    for ( double yPos = y; yPos < endTargetY; yPos += yScale, ySrcPos++ )
    {
        unsigned int xSrcPos = 0;

        for ( double xPos = x; xPos < endTargetX; xPos += xScale, xSrcPos++ )
        {
            unsigned int pixelDstX = (unsigned int)xPos;
            unsigned int pixelDstY = (unsigned int)yPos;
            unsigned int pixelSrcX = (unsigned int)xSrcPos;
            unsigned int pixelSrcY = (unsigned int)ySrcPos;

            // Unpack the colors
            double srcRed;
            double srcGreen;
            double srcBlue;
            double srcAlpha;

            UnpackColor( src->m_data, pixelSrcX, pixelSrcY, srcWidth, srcDataType, srcRed, srcGreen, srcBlue, srcAlpha );

            double dstRed;
            double dstGreen;
            double dstBlue;
            double dstAlpha;

            UnpackColor( map->m_data, pixelDstX, pixelDstY, dstWidth, dstDataType, dstRed, dstGreen, dstBlue, dstAlpha );

            // TODO: allow blendfactor changing.
            // Currently it is SRC_ALPHA, ONE_MINUS_SRC_ALPHA

            double resRed;
            double resGreen;
            double resBlue;
            double resAlpha;

            switch( drawOp )
            {
            case DRAWOP_MODULATE:
                // Alpha blend the bitmaps together
                resRed = SharedUtil::Min <double> ( 1.0f, ( srcRed * srcAlpha ) + ( dstRed * (1 - srcAlpha) ) );
                resGreen = SharedUtil::Min <double> ( 1.0f, ( srcGreen * srcAlpha ) + ( dstGreen * (1 - srcAlpha) ) );
                resBlue = SharedUtil::Min <double> ( 1.0f, ( srcBlue * srcAlpha ) + ( dstBlue * (1 - srcAlpha) ) );
                resAlpha = SharedUtil::Min <double> ( 1.0f, ( srcAlpha * srcAlpha ) + ( dstAlpha * (1 - srcAlpha) ) );
                break;
            case DRAWOP_WRITE:
                // We just copy data from bitmap src to dst
                resRed = srcRed;
                resGreen = srcGreen;
                resBlue = srcBlue;
                resAlpha = srcAlpha;
                break;
            default:
                // Unknown operation.
                resRed = 1.0f;
                resGreen = 1.0f;
                resBlue = 1.0f;
                resAlpha = 1.0f;
                break;
            }

            // Pack the colors again.
            PackColor( map->m_data, pixelDstX, pixelDstY, dstWidth, dstDataType, resRed, resGreen, resBlue, resAlpha );
        }
    }

    lua_pushboolean( L, true );
    return 1;
}

const luaL_Reg Bitmap::_interface[] =
{
    LUA_METHOD( getWidth ),
    LUA_METHOD( getHeight ),
    LUA_METHOD( getSize ),
    LUA_METHOD( drawBitmap ),
    { NULL, NULL }
};

DECLARE_ENUM( Bitmap::eDataType )

IMPLEMENT_ENUM_BEGIN( Bitmap::eDataType )
    ADD_ENUM( Bitmap::BITMAP_RGB, "rgb" )
    ADD_ENUM( Bitmap::BITMAP_RGBA, "rgba" )
IMPLEMENT_ENUM_END( "Bitmap::eDataType" )

template <class blockType>
__forceinline blockType* AllocateImageData( unsigned int width, unsigned int height, size_t& dataSize, const blockType& clearColor )
{
    dataSize = width * height * sizeof(blockType);
    blockType *data = new blockType[width * height];

    for ( unsigned int y = 0; y < height; y++ )
    {
        for ( unsigned int x = 0; x < width; x++ )
        {
            data[x + y * width] = clearColor;
        }
    }

    return data;
}

static LUA_DECLARE( bmp_new )
{
    unsigned int width, height;
    Bitmap::eDataType dataType;

    LUA_ARGS_BEGIN;
    argStream.ReadNumber( width );
    argStream.ReadNumber( height );
    argStream.ReadEnumString( dataType, Bitmap::BITMAP_RGBA );
    LUA_ARGS_END;

    // Create new picture data
    void *data;

    size_t dataSize;

    switch( dataType )
    {
    case Bitmap::BITMAP_RGB:
        {
            BitmapRGB clearColor;

            if ( lua_type( L, 4 ) == LUA_TNUMBER )
            {
                RwColor color = (unsigned int)lua_tonumber( L, 4 );
                clearColor.r = color.r;
                clearColor.g = color.g;
                clearColor.b = color.b;
            }

            data = AllocateImageData <BitmapRGB> ( width, height, dataSize, clearColor );
        }
        break;
    case Bitmap::BITMAP_RGBA:
        {
            unsigned int clearColor;

            if ( lua_type( L, 4 ) == LUA_TNUMBER )
                clearColor = (unsigned int)lua_tonumber( L, 4 );
            else
                clearColor = 0x00000000;

            data = AllocateImageData <unsigned int> ( width, height, dataSize, clearColor );
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

static const luaL_Reg bitmap_library[] =
{
    { "new", bmp_new },
    { NULL, NULL }
};

void luabitmap_open( lua_State *L )
{
    luaL_openlib( L, "bitmap", bitmap_library, 0 );
    lua_pop( L, 1 );
}