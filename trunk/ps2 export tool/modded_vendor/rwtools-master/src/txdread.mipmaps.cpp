// Mipmap utilities for textures.
#include "StdInc.h"

#include "pixelformat.hxx"

#include "txdread.d3d.hxx"

#include "txdread.d3d.dxt.hxx"

#include "txdread.palette.hxx"

#include "txdread.common.hxx"

namespace rw
{

uint32 Raster::getMipmapCount( void ) const
{
    uint32 mipmapCount = 0;

    if ( PlatformTexture *platformTex = this->platformData )
    {
        texNativeTypeProvider *texProvider = GetNativeTextureTypeProvider( engineInterface, platformTex );

        if ( texProvider )
        {
            mipmapCount = GetNativeTextureMipmapCount( engineInterface, platformTex, texProvider );
        }
    }

    return mipmapCount;
}

void TextureBase::clearMipmaps( void )
{
    Raster *texRaster = this->GetRaster();

    if ( texRaster )
    {
        // We need a Direct3D texture.
        //if ( texRaster->platform != PLATFORM_D3D8 && texRaster->platform != PLATFORM_D3D9 )
        //    return;

        NativeTextureD3D *platformTex = (NativeTextureD3D*)texRaster->platformData;

        // Clear mipmaps. This is image data starting from level index 1.
        uint32 mipmapCount = platformTex->mipmaps.size();

        if ( mipmapCount > 1 )
        {
            for ( uint32 n = 1; n < mipmapCount; n++ )
            {
                // Delete the texels.
                const void *texels = platformTex->mipmaps[ n ].texels;

                delete [] texels;
            }

            // Set the mipmap count to 1.
            platformTex->mipmaps.resize( 1 );

            // Adjust the filtering.
            {
                eRasterStageFilterMode currentFilterMode = this->filterMode;

                if ( currentFilterMode == RWFILTER_POINT_POINT ||
                     currentFilterMode == RWFILTER_POINT_LINEAR )
                {
                    this->filterMode = RWFILTER_POINT;
                }
                else if ( currentFilterMode == RWFILTER_LINEAR_POINT ||
                          currentFilterMode == RWFILTER_LINEAR_LINEAR )
                {
                    this->filterMode = RWFILTER_LINEAR;
                }
            }
        }

        // Now we can have automatically generated mipmaps!
    }
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
    Bitmap& srcBitmap, uint32 srcPosX, uint32 srcPosY, uint32 mipProcessWidth, uint32 mipProcessHeight,
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

        for ( uint32 y = 0; y < mipProcessHeight; y++ )
        {
            for ( uint32 x = 0; x < mipProcessWidth; x++ )
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

void TextureBase::generateMipmaps( uint32 maxMipmapCount, eMipmapGenerationMode mipGenMode )
{
    Raster *texRaster = this->GetRaster();

    if ( texRaster )
    {
        // We need a Direct3D texture.
        //if ( texRaster->platform != PLATFORM_D3D8 && texRaster->platform != PLATFORM_D3D9 )
        //    return;

        NativeTextureD3D *platformTex = (NativeTextureD3D*)texRaster->platformData;

        // We cannot do anything if we do not have a first level (base texture).
        uint32 oldMipmapCount = platformTex->mipmaps.size();

        if ( oldMipmapCount == 0 )
            return;

        eRasterFormat srcRasterFormat = platformTex->rasterFormat;
        eColorOrdering srcColorOrder = platformTex->colorOrdering;
        uint32 srcItemDepth = platformTex->depth;

        uint32 dxtType = platformTex->dxtCompression;

        eRasterFormat dstRasterFormat = srcRasterFormat;
        eColorOrdering dstColorOrder = srcColorOrder;

        // Grab the bitmap of this texture, so we can generate mipmaps.
        Bitmap textureBitmap = texRaster->getBitmap();

        // Do the generation.
        // We process the image in 2x2 blocks for the level index 1, 4x4 for level index 2, ...
        uint32 firstLevelWidth, firstLevelHeight;
        textureBitmap.getSize( firstLevelWidth, firstLevelHeight );

        uint32 firstLevelDepth = textureBitmap.getDepth();

        eRasterFormat tmpRasterFormat = textureBitmap.getFormat();
        eColorOrdering tmpColorOrder = textureBitmap.getColorOrder();

        mipGenLevelGenerator mipLevelGen( firstLevelWidth, firstLevelHeight );

        if ( !mipLevelGen.isValidLevel() )
        {
            throw RwException( "invalid texture dimensions in mipmap generation (" + this->name + ")" );
        }

        uint32 curMipProcessWidth = 1;
        uint32 curMipProcessHeight = 1;

        uint32 curMipIndex = 0;

        uint32 actualNewMipmapCount = oldMipmapCount;

        while ( true )
        {
            bool canProcess = false;

            if ( !canProcess )
            {
                if ( curMipIndex >= oldMipmapCount )
                {
                    canProcess = true;
                }
            }

            if ( canProcess )
            {
                // Check whether the current gen index does not overshoot the max gen count.
                if ( curMipIndex >= maxMipmapCount )
                {
                    break;
                }

                // Get the processing dimensions.
                uint32 mipWidth = mipLevelGen.getLevelWidth();
                uint32 mipHeight = mipLevelGen.getLevelHeight();

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
                        uint32 mipProcessWidth = curMipProcessWidth;
                        uint32 mipProcessHeight = curMipProcessHeight;

                        performMipmapFiltering(
                            mipGenMode,
                            textureBitmap,
                            mip_x * mipProcessWidth, mip_y * mipProcessHeight,
                            mipProcessWidth, mipProcessHeight,
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
                uint32 realMipWidth = mipWidth;
                uint32 realMipHeight = mipHeight;

                if ( dxtType != 0 )
                {
                    compressTexelsUsingDXT(
                        engineInterface,
                        dxtType, newtexels, mipWidth, mipHeight,
                        tmpRasterFormat, NULL, PALETTE_NONE, 0, tmpColorOrder, firstLevelDepth,
                        actualTexelArray, actualDataSize,
                        realMipWidth, realMipHeight
                    );
                }
                else
                {
                    // Remap palette if required.
                    ePaletteType paletteType = platformTex->paletteType;

                    if ( paletteType != PALETTE_NONE )
                    {
                        RemapMipmapLayer(
                            engineInterface,
                            srcRasterFormat, srcColorOrder,
                            newtexels, texItemCount,
                            tmpRasterFormat, tmpColorOrder, firstLevelDepth, PALETTE_NONE, NULL, 0,
                            platformTex->palette, platformTex->paletteSize,
                            srcItemDepth, paletteType,
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
                NativeTextureD3D::mipmapLayer newLayer;
                newLayer.width = realMipWidth;
                newLayer.height = realMipHeight;
                newLayer.layerWidth = mipWidth;
                newLayer.layerHeight = mipHeight;
                
                newLayer.texels = actualTexelArray;
                newLayer.dataSize = actualDataSize;

                // Put the new layer.
                ensurePutIntoArray( newLayer, platformTex->mipmaps, curMipIndex );

                // We have a new mipmap.
                actualNewMipmapCount++;
            }

            // Increment mip index.
            curMipIndex++;

            // Process parameters.
            bool shouldContinue = mipLevelGen.incrementLevel();

            if ( shouldContinue )
            {
                if ( mipLevelGen.didIncrementWidth() )
                {
                    curMipProcessWidth *= 2;
                }

                if ( mipLevelGen.didIncrementHeight() )
                {
                    curMipProcessHeight *= 2;
                }
            }
            else
            {
                break;
            }
        }

        if ( srcRasterFormat != dstRasterFormat )
        {
            platformTex->rasterFormat = dstRasterFormat;
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

        // Adjust filtering mode.
        {
            eRasterStageFilterMode currentFilterMode = this->filterMode;

            if ( actualNewMipmapCount > 1 )
            {
                if ( currentFilterMode == RWFILTER_POINT )
                {
                    this->filterMode = RWFILTER_POINT_POINT;
                }
                else if ( currentFilterMode == RWFILTER_LINEAR )
                {
                    this->filterMode = RWFILTER_LINEAR_LINEAR;
                }
            }
            else
            {
                if ( currentFilterMode == RWFILTER_POINT_POINT ||
                     currentFilterMode == RWFILTER_POINT_LINEAR )
                {
                    this->filterMode = RWFILTER_POINT;
                }
                else if ( currentFilterMode == RWFILTER_LINEAR_POINT ||
                          currentFilterMode == RWFILTER_LINEAR_LINEAR )
                {
                    this->filterMode = RWFILTER_LINEAR;
                }
            }
        }
    }
}

}