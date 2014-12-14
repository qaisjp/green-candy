namespace rw
{

static inline bool browsetexelcolor(
    const void *texelSource, ePaletteType paletteType, const void *paletteData, uint32 maxpalette,
    uint32 colorIndex, eRasterFormat rasterFormat,
    uint8& red, uint8& green, uint8& blue, uint8& alpha)
{
    typedef PixelFormat::texeltemplate <PixelFormat::pixeldata32bit> pixel32_t;

    bool hasColor = false;

    const void *realTexelSource = NULL;
    uint32 realColorIndex = 0;

    if (paletteType == PALETTE_4BIT || paletteType == PALETTE_8BIT)
    {
        realTexelSource = paletteData;

        // Get the color lookup index from the texel.
        uint8 paletteIndex;

        if (paletteType == PALETTE_4BIT)
        {
            PixelFormat::palette4bit *srcData = (PixelFormat::palette4bit*)texelSource;

            srcData->getvalue(colorIndex, paletteIndex);
        }
        else if (paletteType == PALETTE_8BIT)
        {
            PixelFormat::palette8bit *srcData = (PixelFormat::palette8bit*)texelSource;

            srcData->getvalue(colorIndex, paletteIndex);
        }

        realColorIndex = paletteIndex;
    }
    else
    {
        realTexelSource = texelSource;
        realColorIndex = colorIndex;
    }

    if (rasterFormat == RASTER_1555)
    {
        struct pixel_t
        {
            uint16 red : 5;
            uint16 green : 5;
            uint16 blue : 5;
            uint16 alpha : 1;
        };

        typedef PixelFormat::texeltemplate <pixel_t> pixel1555_t;

        pixel1555_t *srcData = (pixel1555_t*)realTexelSource;

        srcData->getcolor(realColorIndex, red, green, blue, alpha);

        // Scale the color values.
        red *= 0xFF / 0x1F;
        green *= 0xFF / 0x1F;
        blue *= 0xFF / 0x1F;
        alpha *= 0xFF;

        hasColor = true;
    }
    else if (rasterFormat == RASTER_565)
    {
        struct pixel_t
        {
            uint16 red : 5;
            uint16 green : 6;
            uint16 blue : 5;
        };

        pixel_t *srcData = ( (pixel_t*)realTexelSource + realColorIndex );

        // Scale the color values.
        red = srcData->red * 0xFF / 0x1F;
        green = srcData->green * 0xFF / 0x3F;
        blue = srcData->blue * 0xFF / 0x1F;
        alpha = 0xFF;

        hasColor = true;
    }
    else if (rasterFormat == RASTER_4444)
    {
        struct pixel_t
        {
            uint16 red : 4;
            uint16 green : 4;
            uint16 blue : 4;
            uint16 alpha : 4;
        };

        typedef PixelFormat::texeltemplate <pixel_t> pixel4444_t;

        pixel4444_t *srcData = (pixel4444_t*)realTexelSource;

        srcData->getcolor(realColorIndex, red, green, blue, alpha);

        // Scale the color values.
        red *= 0xFF / 0xF;
        green *= 0xFF / 0xF;
        blue *= 0xFF / 0xF;
        alpha = 0xFF / 0xF;

        hasColor = true;
    }
    else if (rasterFormat == RASTER_8888)
    {
        pixel32_t *srcData = (pixel32_t*)realTexelSource;

        srcData->getcolor(realColorIndex, red, green, blue, alpha);

        hasColor = true;
    }
    else if (rasterFormat == RASTER_888)
    {
        struct pixel_t
        {
            uint8 red;
            uint8 green;
            uint8 blue;
            uint8 unused;
        };

        pixel_t *srcData = ( (pixel_t*)realTexelSource + realColorIndex );

        // Get the color values.
        red = srcData->red;
        green = srcData->green;
        blue = srcData->blue;
        alpha = 0xFF;

        hasColor = true;
    }

    return hasColor;
}

static inline uint8 scalecolor(uint8 color, uint32 curMax, uint32 newMax)
{
    return (uint8)( (double)color / (double)curMax * (double)newMax );
}

static inline bool puttexelcolor(void *texelDest, uint32 colorIndex, eRasterFormat rasterFormat, uint8 red, uint8 green, uint8 blue, uint8 alpha)
{
    typedef PixelFormat::texeltemplate <PixelFormat::pixeldata32bit> pixel32_t;

    bool setColor = false;

    if (rasterFormat == RASTER_1555)
    {
        struct pixel_t
        {
            uint16 red : 5;
            uint16 green : 5;
            uint16 blue : 5;
            uint16 alpha : 1;
        };

        typedef PixelFormat::texeltemplate <pixel_t> pixel1555_t;

        pixel1555_t *dstData = (pixel1555_t*)texelDest;

        // Scale the color values.
        uint8 redScaled =       scalecolor(red, 255, 31);
        uint8 greenScaled =     scalecolor(green, 255, 31);
        uint8 blueScaled =      scalecolor(blue, 255, 31);
        uint8 alphaScaled =     ( alpha != 0 ) ? ( 1 ) : ( 0 );

        dstData->setcolor(colorIndex, redScaled, greenScaled, blueScaled, alphaScaled);

        setColor = true;
    }
    else if (rasterFormat == RASTER_565)
    {
        struct pixel_t
        {
            uint16 red : 5;
            uint16 green : 6;
            uint16 blue : 5;
        };

        pixel_t *dstData = ( (pixel_t*)texelDest + colorIndex );

        // Scale the color values.
        uint8 redScaled =       scalecolor(red, 255, 31);
        uint8 greenScaled =     scalecolor(green, 255, 63);
        uint8 blueScaled =      scalecolor(blue, 255, 31);

        dstData->red = redScaled;
        dstData->green = greenScaled;
        dstData->blue = blueScaled;

        setColor = true;
    }
    else if (rasterFormat == RASTER_4444)
    {
        struct pixel_t
        {
            uint16 red : 4;
            uint16 green : 4;
            uint16 blue : 4;
            uint16 alpha : 4;
        };

        typedef PixelFormat::texeltemplate <pixel_t> pixel4444_t;

        pixel4444_t *dstData = (pixel4444_t*)texelDest;

        // Scale the color values.
        uint8 redScaled =       scalecolor(red, 255, 15);
        uint8 greenScaled =     scalecolor(green, 255, 15);
        uint8 blueScaled =      scalecolor(blue, 255, 15);
        uint8 alphaScaled =     scalecolor(alpha, 255, 15);

        dstData->setcolor(colorIndex, redScaled, greenScaled, blueScaled, alphaScaled);

        setColor = true;
    }
    else if (rasterFormat == RASTER_8888)
    {
        pixel32_t *dstData = (pixel32_t*)texelDest;

        dstData->setcolor(colorIndex, red, green, blue, alpha);

        setColor = true;
    }
    else if (rasterFormat == RASTER_888)
    {
        struct pixel_t
        {
            uint8 red;
            uint8 green;
            uint8 blue;
            uint8 unused;
        };

        pixel_t *dstData = ( (pixel_t*)texelDest + colorIndex );

        // Put the color values.
        dstData->red = red;
        dstData->green = green;
        dstData->blue = blue;

        setColor = true;
    }

    return setColor;
}

static inline bool getrastermeta(uint32 rasterFormat, uint32 colorItemCount, uint32& dataSize, uint32& colorDepth)
{
    bool validFormat = false;

    if (rasterFormat == RASTER_1555 ||
        rasterFormat == RASTER_565 ||
        rasterFormat == RASTER_4444)
    {
        dataSize = colorItemCount*sizeof(uint16);
        colorDepth = 16;

        validFormat = true;
    }
    else if (rasterFormat == RASTER_888)
    {
        dataSize = colorItemCount*3;
        colorDepth = 24;

        validFormat = true;
    }
    else if (rasterFormat == RASTER_8888)
    {
        dataSize = colorItemCount*sizeof(uint32);
        colorDepth = 32;

        validFormat = true;
    }

    return validFormat;
}

};