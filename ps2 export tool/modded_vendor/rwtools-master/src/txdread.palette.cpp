#include "StdInc.h"

#include <bitset>
#include <map>
#include <cmath>

#include "pixelformat.hxx"

#include "txdread.d3d.hxx"

#include "txdread.palette.hxx"

namespace rw
{

struct _fetch_texel_libquant_traverse
{
    NativeTextureD3D *platformTex;

    uint32 mipIndex;
};

static void _fetch_image_data_libquant(liq_color row_out[], int row_index, int width, void *user_info)
{
    _fetch_texel_libquant_traverse *info = (_fetch_texel_libquant_traverse*)user_info;

    NativeTextureD3D *platformTex = info->platformTex;

    // Fetch the color row.
    void *texelSource = platformTex->texels[ info->mipIndex ];

    eRasterFormat rasterFormat = platformTex->parent->rasterFormat;
    ePaletteType paletteType = platformTex->paletteType;

    uint32 itemDepth = platformTex->depth;

    eColorOrdering colorOrder = platformTex->colorOrdering;

    void *palColors = platformTex->palette;
    uint32 palColorCount = platformTex->paletteSize;

    for (int n = 0; n < width; n++)
    {
        uint32 colorIndex = PixelFormat::coord2index(n, row_index, width);

        // Fetch the color.
        uint8 r, g, b, a;

        browsetexelcolor(texelSource, paletteType, palColors, palColorCount, colorIndex, rasterFormat, colorOrder, itemDepth, r, g, b, a);

        // Store the color.
        liq_color& outColor = row_out[ n ];
        outColor.r = r;
        outColor.g = g;
        outColor.b = b;
        outColor.a = a;
    }
}

// Custom algorithm for palettizing image data.
void NativeTexture::convertToPalette(ePaletteType convPaletteFormat)
{
    if ( this->platform != PLATFORM_D3D8 && this->platform != PLATFORM_D3D9 )
        return;

    NativeTextureD3D *platformTex = (NativeTextureD3D*)this->platformData;

    if (convPaletteFormat != PALETTE_8BIT && convPaletteFormat != PALETTE_4BIT)
        return;

    // If the texture is DXT compressed, decompress it.
    if ( platformTex->dxtCompression )
    {
        platformTex->decompressDxt();
    }

    eRasterFormat rasterFormat = this->rasterFormat;
    ePaletteType paletteType = platformTex->paletteType;

    if (paletteType == convPaletteFormat)
        return;

    eColorOrdering srcColorOrder = platformTex->colorOrdering;
    eColorOrdering dstColorOrder = srcColorOrder;

    // We prefer COLOR_RGBA, since all architectures seem to use it.
    if ( dstColorOrder != COLOR_RGBA )
    {
        dstColorOrder = COLOR_RGBA;
    }

    void *srcPaletteData = platformTex->palette;
    uint32 srcPaletteCount = platformTex->paletteSize;

    // Get palette maximums.
    uint32 maxPaletteEntries = 0;
    uint8 newDepth = 0;

    if (convPaletteFormat == PALETTE_8BIT)
    {
        maxPaletteEntries = 256;
        newDepth = 8;
    }
    else if (convPaletteFormat == PALETTE_4BIT)
    {
        maxPaletteEntries = 16;
        newDepth = 4;
    }

    // Do the palettization.
    {
        uint32 mipmapCount = platformTex->mipmapCount;

        // Decide what palette system to use.
        ePaletteRuntimeType useRuntime = rw::rwInterface.GetPaletteRuntime();

        if (useRuntime == PALRUNTIME_NATIVE)
        {
            palettizer conv;

            // Linear eliminate unique texels.
            // Use only the first texture.
            if ( mipmapCount > 0 )
            {
                uint32 srcWidth = platformTex->width[0];
                uint32 srcHeight = platformTex->height[0];
                void *texelSource = platformTex->texels[0];
                uint32 srcDepth = platformTex->depth;

#if 0
                // First define properties to use for linear elimination.
                for (uint32 y = 0; y < srcHeight; y++)
                {
                    for (uint32 x = 0; x < srcWidth; x++)
                    {
                        uint32 colorIndex = PixelFormat::coord2index(x, y, srcWidth);

                        uint8 red, green, blue, alpha;
                        bool hasColor = browsetexelcolor(texelSource, paletteType, paletteData, maxpalette, colorIndex, rasterFormat, red, green, blue, alpha);

                        if ( hasColor )
                        {
                            conv.characterize(red, green, blue, alpha);
                        }
                    }
                }

                // Prepare the linear elimination.
                conv.after_characterize();
#endif

                // Linear eliminate.
                for (uint32 y = 0; y < srcHeight; y++)
                {
                    for (uint32 x = 0; x < srcWidth; x++)
                    {
                        uint32 colorIndex = PixelFormat::coord2index(x, y, srcWidth);

                        uint8 red, green, blue, alpha;
                        bool hasColor = browsetexelcolor(texelSource, paletteType, srcPaletteData, srcPaletteCount, colorIndex, rasterFormat, srcColorOrder, srcDepth, red, green, blue, alpha);

                        if ( hasColor )
                        {
                            conv.feedcolor(red, green, blue, alpha);
                        }
                    }
                }
            }

            // Construct a palette out of the remaining colors.
            conv.constructpalette(maxPaletteEntries);

            // Point each color from the original texture to the palette.
            uint32 itemDepth = platformTex->depth;

            for (uint32 n = 0; n < mipmapCount; n++)
            {
                // Create palette index memory for each mipmap.
                uint32 srcWidth = platformTex->width[n];
                uint32 srcHeight = platformTex->height[n];
                void *texelSource = platformTex->texels[n];

                uint32 itemCount = ( srcWidth * srcHeight );
                
                uint32 dataSize = 0;
                void *newTexelData = NULL;

                // Remap the texels.
                nativePaletteRemap(
                    conv, convPaletteFormat, newDepth,
                    texelSource, itemCount,
                    paletteType, srcPaletteData, srcPaletteCount, rasterFormat, srcColorOrder, itemDepth,
                    newTexelData, dataSize
                );

                // Replace texture data.
                if ( texelSource )
                {
                    delete [] texelSource;
                }

                platformTex->texels[n] = newTexelData;

                platformTex->dataSizes[n] = dataSize;
            }

            // Set the new depth of the texture.
            platformTex->depth = newDepth;

            // Delete the old palette data (if available).
            if (srcPaletteData != NULL)
            {
                delete [] srcPaletteData;
            }

            // Store the new palette texels.
            platformTex->palette = conv.makepalette(rasterFormat, dstColorOrder);
            platformTex->paletteSize = conv.texelElimData.size();
        }
        else if (useRuntime == PALRUNTIME_PNGQUANT)
        {
            liq_attr *quant_attr = liq_attr_create();

            assert( quant_attr != NULL );

            liq_set_max_colors(quant_attr, maxPaletteEntries);

            _fetch_texel_libquant_traverse main_traverse;

            main_traverse.platformTex = platformTex;
            main_traverse.mipIndex = 0;

            liq_image *quant_image = liq_image_create_custom(
                quant_attr, _fetch_image_data_libquant, &main_traverse,
                platformTex->width[0], platformTex->height[0],
                1.0
            );

            assert( quant_image != NULL );

            // Quant it!
            liq_result *quant_result = liq_quantize_image(quant_attr, quant_image);

            if (quant_result != NULL)
            {
                // Get the palette and remap all mipmaps.
                for (uint32 n = 0; n < mipmapCount; n++)
                {
                    uint32 mipWidth = platformTex->width[n];
                    uint32 mipHeight = platformTex->height[n];

                    uint32 palItemCount = ( mipWidth * mipHeight );

                    unsigned char *newPalItems = new unsigned char[ palItemCount ];

                    assert( newPalItems != NULL );

                    liq_image *srcImage = NULL;
                    bool newImage = false;

                    _fetch_texel_libquant_traverse thisTraverse;

                    if ( n == 0 )
                    {
                        srcImage = quant_image;
                    }
                    else
                    {
                        // Create a new image.
                        thisTraverse.platformTex = platformTex;
                        thisTraverse.mipIndex = n;

                        srcImage = liq_image_create_custom(
                            quant_attr, _fetch_image_data_libquant, &thisTraverse,
                            mipWidth, mipHeight,
                            1.0
                        );

                        newImage = true;
                    }

                    // Map it.
                    liq_write_remapped_image( quant_result, srcImage, newPalItems, palItemCount );

                    // Delete image (if newly allocated)
                    if (newImage)
                    {
                        liq_image_destroy( srcImage );
                    }

                    // Update the texels.
                    delete [] platformTex->texels[ n ];

                    bool hasUsedArray = false;

                    {
                        uint32 dataSize = 0;
                        void *newTexelArray = NULL;

                        if (convPaletteFormat == PALETTE_4BIT)
                        {
                            dataSize = PixelFormat::palette4bit::sizeitems( palItemCount );
                        }
                        else if (convPaletteFormat == PALETTE_8BIT)
                        {
                            dataSize = palItemCount;

                            newTexelArray = newPalItems;

                            hasUsedArray = true;
                        }

                        if ( !hasUsedArray )
                        {
                            newTexelArray = new uint8[ dataSize ];

                            // Copy over the items.
                            for ( uint32 n = 0; n < palItemCount; n++ )
                            {
                                uint32 resVal = newPalItems[ n ];

                                if (convPaletteFormat == PALETTE_4BIT)
                                {
                                    ( (PixelFormat::palette4bit*)newTexelArray )->setvalue(n, resVal);
                                }
                                else if (convPaletteFormat == PALETTE_8BIT)
                                {
                                    ( (PixelFormat::palette8bit*)newTexelArray)->setvalue(n, resVal);
                                }
                            }
                        }

                        // Set the texels.
                        platformTex->texels[ n ] = newTexelArray;
                        platformTex->dataSizes[ n ] = dataSize;
                    }

                    if ( !hasUsedArray )
                    {
                        delete [] newPalItems;
                    }
                }

                // Delete the old palette data.
                if (void *srcPalData = platformTex->palette)
                {
                    delete [] srcPalData;
                }

                // Update the texture depth.
                platformTex->depth = newDepth;

                // Update the texture palette data.
                {
                    const liq_palette *palData = liq_get_palette(quant_result);

                    uint32 newPalItemCount = palData->count;

                    uint32 palDepth = Bitmap::getRasterFormatDepth(rasterFormat);

                    uint32 palDataSize = getRasterDataSize( newPalItemCount, palDepth );

                    void *newPalArray = new uint8[ palDataSize ];

                    for ( unsigned int n = 0; n < newPalItemCount; n++ )
                    {
                        const liq_color& srcColor = palData->entries[ n ];

                        puttexelcolor(newPalArray, n, rasterFormat, dstColorOrder, palDepth, srcColor.r, srcColor.g, srcColor.b, srcColor.a);
                    }

                    // Update texture properties.
                    platformTex->palette = newPalArray;
                    platformTex->paletteSize = newPalItemCount;
                }
            }
            else
            {
                assert( 0 );
            }

            // Release resources.
            liq_image_destroy( quant_image );

            liq_attr_destroy( quant_attr );

            liq_result_destroy( quant_result );
        }
        else
        {
            assert( 0 );
        }
    }

    // Update our color order.
    platformTex->colorOrdering = dstColorOrder;

    // Notify the raster about its new format.
    platformTex->paletteType = convPaletteFormat;

    // Update the D3DFORMAT field.
    platformTex->updateD3DFormat();

    // Since we changed the colors, update the alpha flag.
    platformTex->hasAlpha = platformTex->doesHaveAlpha();
}

};