#include <StdInc.h>

#include "pixelformat.hxx"

#include "txdread.d3d.hxx"

#include "txdread.d3d.dxt.hxx"

namespace rw
{

bool NativeTextureD3D::getRawBitmap( uint32 mipLayer, bool allowPalette, rawBitmapFetchResult& bitmapOut ) const
{
    bool couldFetchBitmap = false;

    uint32 mipmapCount = this->mipmapCount;

    if ( mipLayer < mipmapCount )
    {
        // Parameters to pass to the raw bitmap result.
        void *texelsOut = NULL;
        uint32 dataSizeOut = 0;
        uint32 widthOut, heightOut;
        bool isNewlyAllocated = false;
        uint32 depthOut = 0;
        eRasterFormat rasterFormatOut = RASTER_8888;
        eColorOrdering colorOrderOut = COLOR_RGBA;
        const void *paletteDataOut = NULL;
        uint32 paletteSizeOut = 0;
        ePaletteType paletteTypeOut = PALETTE_NONE;

        // Get raw colors from this texture.
        {
            uint32 compressionType = this->dxtCompression;

            void *theTexels = this->texels[ mipLayer ];
            uint32 dataSize = this->dataSizes[ mipLayer ];
            uint32 mipWidth = this->width[ mipLayer ];
            uint32 mipHeight = this->height[ mipLayer ];
            uint32 depth = this->depth;
            eRasterFormat rasterFormat = parent->rasterFormat;
            eColorOrdering colorOrder = this->colorOrdering;

            if ( compressionType != 0 )
            {
                // Decompress the texels.
                uint32 texItemCount = ( mipWidth * mipHeight );

                uint32 compressedBlockCount = ( texItemCount / 16 );

                uint32 y = 0;
                uint32 x = 0;

                // Allocate a new texture array.
                rasterFormatOut = RASTER_8888;
                colorOrderOut = COLOR_RGBA;
                widthOut = mipWidth;
                heightOut = mipHeight;
                depthOut = 32;
                
                uint32 texDataSize = getRasterDataSize( texItemCount, depthOut );

                texelsOut = new uint8[ texDataSize ];

                dataSizeOut = texDataSize;

                isNewlyAllocated = true;

                // Get decompression properties.
                eDXTCompressionMethod compressionMethod = rwInterface.GetDXTRuntime();

                for ( uint32 n = 0; n < compressedBlockCount; n++ )
                {
                    PixelFormat::pixeldata32bit colors[4][4];

                    // Decompress the block.
                    bool couldDecompress = decompressDXTBlock(
                        compressionMethod, theTexels,
                        n,
                        compressionType,
                        colors
                    );

                    if ( couldDecompress )
                    {
                        // Put the colors into the image.
                        for ( uint32 y_block = 0; y_block < 4; y_block++ )
                        {
                            for ( uint32 x_block = 0; x_block < 4; x_block++ )
                            {
                                // Get the color that we want to write.
                                const PixelFormat::pixeldata32bit& theColor = colors[ y_block ][ x_block ];

                                // Calculate the destination position.
                                uint32 dest_x = ( x + x_block );
                                uint32 dest_y = ( y + y_block );

                                // Put the color.
                                uint32 colorIndex = PixelFormat::coord2index( dest_x, dest_y, mipWidth );

                                puttexelcolor(
                                    texelsOut, colorIndex, rasterFormatOut, colorOrderOut, depthOut,
                                    theColor.red, theColor.green, theColor.blue, theColor.alpha
                                );
                            }
                        }
                    }

                    // Increment the reader position.
                    x += 4;

                    if ( x >= mipWidth )
                    {
                        y += 4;
                        x = 0;
                    }

                    if ( y >= mipHeight )
                    {
                        break;
                    }
                }

                couldFetchBitmap = true;
            }
            else
            {
                const void *paletteData = this->palette;
                uint32 paletteSize = this->paletteSize;

                ePaletteType paletteType = this->paletteType;

                if ( allowPalette == false && paletteType != PALETTE_NONE )
                {
                    // Unpalettize the texels.
                    uint32 texItemCount = ( mipWidth * mipHeight );

                    uint32 palRasterFormatDepth = Bitmap::getRasterFormatDepth(rasterFormat);

                    // Set output properties.
                    rasterFormatOut = rasterFormat;
                    colorOrderOut = colorOrder;
                    widthOut = mipWidth;
                    heightOut = mipHeight;
                    depthOut = palRasterFormatDepth;

                    uint32 texDataSize = getRasterDataSize( texItemCount, depthOut );

                    texelsOut = new uint8[ texDataSize ];

                    dataSizeOut = texDataSize;

                    isNewlyAllocated = true;

                    // Put the raw colors.
                    for ( uint32 y = 0; y < mipHeight; y++ )
                    {
                        for ( uint32 x = 0; x < mipWidth; x++ )
                        {
                            // Get the color index.
                            uint32 colorIndex = PixelFormat::coord2index( x, y, mipWidth );

                            // Transfer the color.
                            uint8 r, g, b, a;

                            bool hasColor = browsetexelcolor(
                                theTexels, paletteType, paletteData, paletteSize, colorIndex, rasterFormat, colorOrder, depth,
                                r, g, b, a
                            );

                            if ( !hasColor )
                            {
                                r = 0;
                                g = 0;
                                b = 0;
                                a = 0;
                            }

                            puttexelcolor(
                                texelsOut, colorIndex, rasterFormatOut, colorOrderOut, depthOut,
                                r, g, b, a
                            );
                        }
                    }

                    couldFetchBitmap = true;
                }
                else
                {
                    // Just return the raw texels.
                    texelsOut = theTexels;
                    dataSizeOut = dataSize;
                    widthOut = mipWidth;
                    heightOut = mipHeight;
                    isNewlyAllocated = false;
                    depthOut = depth;
                    rasterFormatOut = rasterFormat;
                    colorOrderOut = colorOrder;
                    paletteDataOut = paletteData;
                    paletteSizeOut = paletteSize;
                    paletteTypeOut = paletteType;

                    couldFetchBitmap = true;
                }
            }
        }

        // Pass the parameters to the runtime, if successful.
        if ( couldFetchBitmap )
        {
            bitmapOut.texelData = texelsOut;
            bitmapOut.dataSize = dataSizeOut;
            bitmapOut.width = widthOut;
            bitmapOut.height = heightOut;
            bitmapOut.isNewlyAllocated = isNewlyAllocated;
            bitmapOut.depth = depthOut;
            bitmapOut.rasterFormat = rasterFormatOut;
            bitmapOut.colorOrder = colorOrderOut;
            bitmapOut.paletteData = paletteDataOut;
            bitmapOut.paletteSize = paletteSizeOut;
            bitmapOut.paletteType = paletteTypeOut;

            // TODO: maybe allow allocation of palette data.
        }
    }

    return couldFetchBitmap;
}

}