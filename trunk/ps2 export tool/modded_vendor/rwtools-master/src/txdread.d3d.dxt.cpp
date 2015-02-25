#include <StdInc.h>

#include "pixelformat.hxx"

#include "txdread.d3d.hxx"

#include "txdread.d3d.dxt.hxx"

namespace rw
{

bool NativeTextureD3D::decompressDxtNative(uint32 dxtType)
{
    eDXTCompressionMethod dxtMethod = rw::rwInterface.GetDXTRuntime();

    bool doPackedDecompress = rw::rwInterface.GetDXTPackedDecompression();

    // Decide which raster format to use.
    eRasterFormat targetRasterFormat;

    if (this->hasAlpha)
    {
        eRasterFormat packedAlphaRasterFormat = RASTER_8888;

        if (dxtType == 1)
        {
            packedAlphaRasterFormat = RASTER_1555;
        }
        else if (dxtType == 2 || dxtType == 3)
        {
            packedAlphaRasterFormat = RASTER_4444;
        }

        targetRasterFormat = ( doPackedDecompress ) ? packedAlphaRasterFormat : RASTER_8888;
    }
    else
    {
        eRasterFormat packedRasterFormat = RASTER_565;

        targetRasterFormat = ( doPackedDecompress ) ? packedRasterFormat : RASTER_888;
    }

    uint32 rasterFormatDepth = Bitmap::getRasterFormatDepth(targetRasterFormat);

    eColorOrdering colorOrder = this->colorOrdering;

    bool conversionSuccessful = true;

	for (uint32 i = 0; i < mipmapCount; i++)
    {
        void *texelData = this->texels[i];

		uint32 x = 0, y = 0;

        // Allocate the new texel array.
        uint32 texWidth = this->width[i];
        uint32 texHeight = this->height[i];

        uint32 texelItemCount = ( texWidth * texHeight );

		uint32 dataSize = getRasterDataSize(texelItemCount, rasterFormatDepth);

		void *newtexels = new uint8[dataSize];

        // Get the compressed block count.
        uint32 compressedBlockCount = ( texWidth * texHeight ) / 16;

        bool successfullyDecompressed = true;

		for (uint32 n = 0; n < compressedBlockCount; n++)
        {
            PixelFormat::pixeldata32bit colors[4][4];

            bool couldDecompressBlock = decompressDXTBlock(dxtMethod, texelData, n, dxtType, colors);

            if (couldDecompressBlock == false)
            {
                // If even one block fails to decompress, abort.
                successfullyDecompressed = false;
                break;
            }

            // Write the colors.
            for (uint32 y_block = 0; y_block < 4; y_block++)
            {
                for (uint32 x_block = 0; x_block < 4; x_block++)
                {
                    uint32 target_x = ( x + x_block );
                    uint32 target_y = ( y + y_block );

                    const PixelFormat::pixeldata32bit& srcColor = colors[ y_block ][ x_block ];

                    uint8 red       = srcColor.red;
                    uint8 green     = srcColor.green;
                    uint8 blue      = srcColor.blue;
                    uint8 alpha     = srcColor.alpha;

                    uint32 dstColorIndex = PixelFormat::coord2index(target_x, target_y, texWidth);

                    puttexelcolor(newtexels, dstColorIndex, targetRasterFormat, colorOrder, rasterFormatDepth, red, green, blue, alpha);
                }
            }

			x += 4;

			if (x >= texWidth)
            {
				y += 4;
				x = 0;
			}

            if (y >= texHeight)
            {
                break;
            }
		}

        // If even one mipmap fails to decompress, abort.
        if ( !successfullyDecompressed )
        {
            assert( i == 0 );

            // Delete the new texel array again.
            delete [] newtexels;

            conversionSuccessful = false;
            break;
        }

        // Replace the texel data.
		delete [] texelData;

		this->texels[i] = newtexels;

		this->dataSizes[i] = dataSize;
	}

    if (conversionSuccessful)
    {
        // Update the depth.
        this->depth = rasterFormatDepth;

        // Update the raster format.
        parent->rasterFormat = targetRasterFormat;

        // We are not compressed anymore.
        this->dxtCompression = 0;

        // We need to update our D3DFORMAT field.
        this->updateD3DFormat();
    }

    return conversionSuccessful;
}

void NativeTextureD3D::decompressDxt(void)
{
    uint32 dxtType = this->dxtCompression;

	if (dxtType == 0)
		return;

    if (dxtType == 1 ||
        //dxtType == 2 ||
        dxtType == 3 ||
        dxtType == 4 ||
        dxtType == 5)
    {
        bool success = decompressDxtNative( dxtType );

        assert( success == true );
    }
    else
    {
        throw RwException( "unsupported DXT compression; cannot decompress" );
    }
}

void NativeTextureD3D::compressDxt(uint32 dxtType)
{
    if (this->dxtCompression == dxtType)
        return;

    if (dxtType != 1 && dxtType != 3 && dxtType != 5)
    {
        throw RwException( "cannot compress Direct3D textures using unsupported DXTn format" );
    }

    // Decompress if we are compressed.
    if (this->dxtCompression)
    {
        this->decompressDxt();
    }

    // Compress it now.
    uint32 mipmapCount = this->mipmapCount;

    uint32 itemDepth = this->depth;

    eRasterFormat rasterFormat = this->parent->rasterFormat;
    ePaletteType paletteType = this->paletteType;
    eColorOrdering colorOrder = this->colorOrdering;

    uint32 maxpalette = this->paletteSize;
    void *paletteData = this->palette;

    for (uint32 n = 0; n < mipmapCount; n++)
    {
        uint32 mipWidth = this->width[ n ];
        uint32 mipHeight = this->height[ n ];

        const void *texelSource = this->texels[ n ];

        void *dxtArray = NULL;
        size_t dxtDataSize = 0;

        // Create the new DXT array.
        uint32 realMipWidth, realMipHeight;

        compressTexelsUsingDXT(
            dxtType, texelSource, mipWidth, mipHeight,
            rasterFormat, paletteData, paletteType, maxpalette, colorOrder, itemDepth,
            dxtArray, dxtDataSize,
            realMipWidth, realMipHeight
        );

        // Delete the raw texels.
        delete [] texelSource;

        if ( mipWidth != realMipWidth )
        {
            this->width[ n ] = realMipWidth;
        }

        if ( mipHeight != realMipHeight )
        {
            this->height[ n ] = realMipHeight;
        }

        // Put in the new DXTn texels.
        this->texels[ n ] = dxtArray;

        // Update fields.
        this->dataSizes[ n ] = dxtDataSize;
    }

    // We are finished compressing.
    // If we were palettized, unset that.
    if ( paletteType != PALETTE_NONE )
    {
        // Free the palette colors.
        delete [] paletteData;

        // Reset the fields.
        this->paletteType = PALETTE_NONE;
        this->palette = NULL;
        this->paletteSize = 0;
    }

    // Set a virtual raster format.
    // This is what is done by the R* DXT output system.
    {
        uint32 newDepth = itemDepth;
        eRasterFormat virtualRasterFormat = RASTER_8888;

        if ( dxtType == 1 )
        {
            newDepth = 16;

            if ( this->hasAlpha )
            {
                virtualRasterFormat = RASTER_1555;
            }
            else
            {
                virtualRasterFormat = RASTER_565;
            }
        }
        else if ( dxtType == 2 || dxtType == 3 || dxtType == 4 || dxtType == 5 )
        {
            newDepth = 16;

            virtualRasterFormat = RASTER_4444;
        }

        if ( rasterFormat != virtualRasterFormat )
        {
            parent->rasterFormat = virtualRasterFormat;
        }

        if ( newDepth != itemDepth )
        {
            this->depth = newDepth;
        }
    }

    // Update the D3DFORMAT field.
    {
        D3DFORMAT newD3DFormat = D3DFMT_DXT1;

        if ( dxtType == 1 )
        {
            newD3DFormat = D3DFMT_DXT1;
        }
        else if ( dxtType == 2 )
        {
            newD3DFormat = D3DFMT_DXT2;
        }
        else if ( dxtType == 3 )
        {
            newD3DFormat = D3DFMT_DXT3;
        }
        else if ( dxtType == 4 )
        {
            newD3DFormat = D3DFMT_DXT4;
        }
        else if ( dxtType == 5 )
        {
            newD3DFormat = D3DFMT_DXT5;
        }

        this->d3dFormat = newD3DFormat;
        this->hasD3DFormat = true;
    }

    // We are now successfully compressed.
    this->dxtCompression = dxtType;
}

};