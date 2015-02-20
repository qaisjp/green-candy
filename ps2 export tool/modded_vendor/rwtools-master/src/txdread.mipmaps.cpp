// Mipmap utilities for textures.
#include "StdInc.h"

#include "pixelformat.hxx"

#include "txdread.d3d.hxx"

#include "txdread.d3d.dxt.hxx"

#include "txdread.palette.hxx"

namespace rw
{

uint32 NativeTexture::getMipmapCount( void ) const
{
    uint32 mipmapCount = 0;

    if ( PlatformTexture *platformTex = this->platformData )
    {
        mipmapCount = platformTex->getMipmapCount();
    }

    return mipmapCount;
}

void NativeTexture::clearMipmaps( void )
{
    // We need a Direct3D texture.
    if ( this->platform != PLATFORM_D3D8 && this->platform != PLATFORM_D3D9 )
        return;

    NativeTextureD3D *platformTex = (NativeTextureD3D*)this->platformData;

    // Clear mipmaps. This is image data starting from level index 1.
    uint32 mipmapCount = platformTex->mipmapCount;

    if ( mipmapCount > 1 )
    {
        for ( uint32 n = 1; n < mipmapCount; n++ )
        {
            // Delete the texels.
            void *texels = platformTex->texels[ n ];

            delete [] texels;
        }

        // Set the mipmap count to 1.
        platformTex->mipmapCount = 1;

        // Trim the other arrays.
        platformTex->texels.resize( 1 );
        platformTex->width.resize( 1 );
        platformTex->height.resize( 1 );
        platformTex->dataSizes.resize( 1 );
    }

    // Now we can have automatically generated mipmaps!
}

inline uint32 pow2( uint32 val )
{
    uint32 n = 1;

    while ( val )
    {
        n *= 2;

        val--;
    }

    return n;
}

inline void performMipmapFiltering(
    eMipmapGenerationMode mipGenMode,
    Bitmap& srcBitmap, uint32 srcPosX, uint32 srcPosY, uint32 mipProcessDimm,
    uint8& redOut, uint8& greenOut, uint8& blueOut, uint8& alphaOut
)
{
    uint8 red = 0;
    uint8 green = 0;
    uint8 blue = 0;
    uint8 alpha = 0;

    if ( mipGenMode == MIPMAPGEN_DEFAULT )
    {
        uint32 redSumm = 0;
        uint32 greenSumm = 0;
        uint32 blueSumm = 0;
        uint32 alphaSumm = 0;

        // Loop through the texels and calculate a blur.
        uint32 addCount = 0;

        for ( uint32 y = 0; y < mipProcessDimm; y++ )
        {
            for ( uint32 x = 0; x < mipProcessDimm; x++ )
            {
                uint8 r, g, b, a;

                bool hasColor = srcBitmap.browsecolor(
                    x + srcPosX, y + srcPosY,
                    r, g, b, a
                );

                if ( hasColor )
                {
                    // Add colors together.
                    redSumm += r;
                    greenSumm += g;
                    blueSumm += b;
                    alphaSumm += a;

                    addCount++;
                }
            }
        }

        if ( addCount != 0 )
        {
            // Calculate the real color.
            red = std::min( redSumm / addCount, 255u );
            green = std::min( greenSumm / addCount, 255u );
            blue = std::min( blueSumm / addCount, 255u );
            alpha = std::min( alphaSumm / addCount, 255u );
        }
    }
    else
    {
        //assert( 0 );
    }

    redOut = red;
    greenOut = green;
    blueOut = blue;
    alphaOut = alpha;
}

template <typename dataType, typename containerType>
inline void ensurePutIntoArray( dataType dataToPut, containerType& container, uint32 putIndex )
{
    if ( container.size() <= putIndex )
    {
        container.resize( putIndex + 1 );
    }

    container[ putIndex ] = dataToPut;
}

void NativeTexture::generateMipmaps( uint32 maxMipmapCount, eMipmapGenerationMode mipGenMode, bool safeMipmaps )
{
    // We need a Direct3D texture.
    if ( this->platform != PLATFORM_D3D8 && this->platform != PLATFORM_D3D9 )
        return;

    NativeTextureD3D *platformTex = (NativeTextureD3D*)this->platformData;

    // We cannot do anything if we do not have a first level.
    if ( platformTex->mipmapCount == 0 )
        return;

    eRasterFormat srcRasterFormat = this->rasterFormat;
    eColorOrdering srcColorOrder = platformTex->colorOrdering;
    uint32 srcItemDepth = platformTex->depth;

    uint32 dxtType = platformTex->dxtCompression;

    eRasterFormat dstRasterFormat = srcRasterFormat;
    eColorOrdering dstColorOrder = srcColorOrder;

    // Grab the bitmap of this texture, so we can generate mipmaps.
    Bitmap textureBitmap = this->getBitmap();

    // Do the generation.
    // We process the image in 2x2 blocks for the level index 1, 4x4 for level index 2, ...
    uint32 oldMipmapCount = platformTex->mipmapCount;

    uint32 firstLevelWidth, firstLevelHeight;
    textureBitmap.getSize( firstLevelWidth, firstLevelHeight );

    uint32 firstLevelDepth = textureBitmap.getDepth();

    eRasterFormat tmpRasterFormat = textureBitmap.getFormat();
    eColorOrdering tmpColorOrder = textureBitmap.getColorOrder();

    double doubleFirstLevelWidth = (double)firstLevelWidth;
    double doubleFirstLevelHeight = (double)firstLevelHeight;

    uint32 curMipIndex = oldMipmapCount;

    uint32 actualNewMipmapCount = oldMipmapCount;

    while ( true )
    {
        // Check whether the current gen index does not overshoot the max gen count.
        if ( curMipIndex >= maxMipmapCount )
        {
            break;
        }

        // Get the processing dimensions.
        uint32 mipProcessDimm = pow2( curMipIndex );
        double doubleMipProcessDimm = (double)mipProcessDimm;

        double doubleMipWidth = doubleFirstLevelWidth / doubleMipProcessDimm;
        double doubleMipHeight = doubleFirstLevelHeight / doubleMipProcessDimm;

        // If we want safe mipmaps, then only generate mipmaps until we reach width or height == 4, in case we use compression.
        if ( safeMipmaps && dxtType != 0 )
        {
            if ( doubleMipWidth < 4.0 || doubleMipHeight < 4.0 )
            {
                break;
            }
        }

        // Terminate if any dimension is smaller than one.
        if ( doubleMipWidth < 1.0 || doubleMipHeight < 1.0 )
        {
            break;
        }

        uint32 mipWidth = (uint32)ceil( doubleMipWidth );
        uint32 mipHeight = (uint32)ceil( doubleMipHeight );

        // Allocate the new bitmap.
        uint32 texItemCount = ( mipWidth * mipHeight );

        uint32 texDataSize = getRasterDataSize( texItemCount, firstLevelDepth );

        void *newtexels = new uint8[ texDataSize ];

        // Process the pixels.
        for ( uint32 mip_y = 0; mip_y < mipHeight; mip_y++ )
        {
            for ( uint32 mip_x = 0; mip_x < mipWidth; mip_x++ )
            {
                // Get the color for this pixel.
                uint8 r, g, b, a;

                r = 0;
                g = 0;
                b = 0;
                a = 0;

                // Perform a filter operation on the currently selected texture block.
                performMipmapFiltering(
                    mipGenMode,
                    textureBitmap,
                    mip_x * mipProcessDimm, mip_y * mipProcessDimm, mipProcessDimm,
                    r, g, b, a
                );

                // Put the color.
                uint32 colorIndex = PixelFormat::coord2index( mip_x, mip_y, mipWidth );

                puttexelcolor(
                    newtexels, colorIndex, tmpRasterFormat, tmpColorOrder, firstLevelDepth,
                    r, g, b, a
                );
            }
        }

        // Pack the pixels into the required format.
        // That means compress if the texture is compressed and palettize if the texture is palettized.
        void *actualTexelArray = NULL;
        uint32 actualDataSize = 0;

        // Compress if required.
        if ( dxtType != 0 )
        {
            compressTexelsUsingDXT(
                dxtType, newtexels, mipWidth, mipHeight,
                tmpRasterFormat, NULL, PALETTE_NONE, 0, tmpColorOrder, firstLevelDepth,
                actualTexelArray, actualDataSize
            );
        }
        else
        {
            // Palettize if required.
            ePaletteType paletteType = platformTex->paletteType;

            if ( paletteType != PALETTE_NONE )
            {
                const void *paletteData = platformTex->palette;
                uint32 paletteSize = platformTex->paletteSize;

                // Do some complex remapping.
                // Since libimagequant does not support just remapping, we need to map it with the native algorithm.
                palettizer remapper;

                // Create an array with all the palette colors.
                palettizer::texelContainer_t paletteContainer;

                paletteContainer.resize( paletteSize );

                for ( uint32 n = 0; n < paletteSize; n++ )
                {
                    uint8 r, g, b, a;

                    bool hasColor = browsetexelcolor(
                        paletteData, PALETTE_NONE, NULL, 0, n, srcRasterFormat, srcColorOrder, srcItemDepth,
                        r, g, b, a
                    );

                    if ( !hasColor )
                    {
                        r = 0;
                        g = 0;
                        b = 0;
                        a = 0;
                    }

                    palettizer::texel_t inTexel;
                    inTexel.red = r;
                    inTexel.green = g;
                    inTexel.blue = b;
                    inTexel.alpha = a;

                    paletteContainer[ n ] = inTexel;
                }

                // Put the palette texels into the remapper.
                remapper.texelElimData = paletteContainer;

                // Do the remap.
                nativePaletteRemap(
                    remapper, paletteType, srcItemDepth,
                    newtexels, texItemCount, PALETTE_NONE, NULL, 0,
                    srcRasterFormat, srcColorOrder, srcItemDepth,
                    actualTexelArray, actualDataSize
                );
            }
            else
            {
                // Do a raw copy.
                actualTexelArray = newtexels;
                actualDataSize = texDataSize;
            }
        }

        // Delete old or temporary texels.
        if ( actualTexelArray != newtexels )
        {
            delete [] newtexels;
        }

        // Store the texels.
        ensurePutIntoArray( actualTexelArray, platformTex->texels, curMipIndex );
        ensurePutIntoArray( actualDataSize, platformTex->dataSizes, curMipIndex );
        ensurePutIntoArray( mipWidth, platformTex->width, curMipIndex );
        ensurePutIntoArray( mipHeight, platformTex->height, curMipIndex );

        // Increment the mip index.
        curMipIndex++;

        // We have a new mipmap.
        actualNewMipmapCount++;
    }

    if ( oldMipmapCount != actualNewMipmapCount )
    {
        // Update texture properties.
        platformTex->mipmapCount = actualNewMipmapCount;
    }

    if ( srcRasterFormat != dstRasterFormat )
    {
        this->rasterFormat = dstRasterFormat;
    }

    if ( srcColorOrder != dstColorOrder )
    {
        platformTex->colorOrdering = dstColorOrder;
    }

    // Make sure that we have no auto mipmaps when we have mipmaps already.
    if ( actualNewMipmapCount > 1 )
    {
        platformTex->autoMipmaps = false;
    }
}

}