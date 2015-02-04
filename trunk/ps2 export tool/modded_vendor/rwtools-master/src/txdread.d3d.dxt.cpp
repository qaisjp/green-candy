#include <StdInc.h>

#include "pixelformat.hxx"

#include "txdread.d3d.hxx"

#include <squish.h>

namespace rw
{

struct rgb565
{
    union
    {
        struct
        {
            unsigned short blue : 5;
            unsigned short green : 6;
            unsigned short red : 5;
        };
        unsigned short val;
    };
};

struct dxt1_block
{
    rgb565 col0;
    rgb565 col1;

    uint32 indexList;
};

struct dxt3_block
{
    uint64 alphaList;

    rgb565 col0;
    rgb565 col1;

    uint32 indexList;
};

struct dxt4_block
{
    uint8 alphaPreMult[2];
    uint8 alphaList[6];

    rgb565 col0;
    rgb565 col1;

    uint32 indexList;
};

struct squish_color
{
    uint8 blue;
    uint8 green;
    uint8 red;
    uint8 alpha;
};

inline bool decompressDXTBlock(
    eDXTCompressionMethod dxtMethod,
    const void *theTexels, uint32 blockIndex, uint32 dxtType,
    PixelFormat::pixeldata32bit colorsOut[4][4]
)
{
    bool hasDecompressed = false;

    bool useNative = false;

    if (dxtMethod == DXTRUNTIME_NATIVE || dxtType == 2 || dxtType == 4)
    {
        useNative = true;
    }
    
    if (useNative)
    {
        if (dxtType == 1)
        {
            const dxt1_block *block = (const dxt1_block*)theTexels + blockIndex;

		    /* calculate colors */
		    const rgb565 col0 = block->col0;
		    const rgb565 col1 = block->col1;
		    uint32 c[4][4];

		    c[0][0] = col0.red * 0xFF/0x1F;
		    c[0][1] = col0.green * 0xFF/0x3F;
		    c[0][2] = col0.blue * 0xFF/0x1F;
		    c[0][3] = 0xFF;

		    c[1][0] = col1.red * 0xFF/0x1F;
		    c[1][1] = col1.green * 0xFF/0x3F;
		    c[1][2] = col1.blue * 0xFF/0x1F;
            c[1][3] = 0xFF;

		    if (col0.val > col1.val)
            {
			    c[2][0] = (2*c[0][0] + 1*c[1][0])/3;
			    c[2][1] = (2*c[0][1] + 1*c[1][1])/3;
			    c[2][2] = (2*c[0][2] + 1*c[1][2])/3;
			    c[2][3] = 0xFF;

			    c[3][0] = (1*c[0][0] + 2*c[1][0])/3;
			    c[3][1] = (1*c[0][1] + 2*c[1][1])/3;
			    c[3][2] = (1*c[0][2] + 2*c[1][2])/3;
			    c[3][3] = 0xFF;
		    }
            else
            {
			    c[2][0] = (c[0][0] + c[1][0])/2;
			    c[2][1] = (c[0][1] + c[1][1])/2;
			    c[2][2] = (c[0][2] + c[1][2])/2;
			    c[2][3] = 0xFF;

			    c[3][0] = 0x00;
			    c[3][1] = 0x00;
			    c[3][2] = 0x00;
		        c[3][3] = 0x00;
		    }

		    /* make index list */
		    uint32 indicesint = block->indexList;
		    uint8 indices[16];
		    for (int32 k = 0; k < 16; k++)
            {
			    indices[k] = indicesint & 0x3;
			    indicesint >>= 2;
		    }

		    /* write bytes */
		    for (uint32 y_block = 0; y_block < 4; y_block++)
            {
			    for (uint32 x_block = 0; x_block < 4; x_block++)
                {
                    PixelFormat::pixeldata32bit& pixelOut = colorsOut[ y_block ][ x_block ];

                    uint32 coordIndex = PixelFormat::coord2index( x_block, y_block, 4 );

                    uint32 colorIndex = indices[ coordIndex ];

                    uint8 red       = c[ colorIndex ][0];
                    uint8 green     = c[ colorIndex ][1];
                    uint8 blue      = c[ colorIndex ][2];
                    uint8 alpha     = c[ colorIndex ][3];

                    pixelOut.red    = red;
                    pixelOut.green  = green;
                    pixelOut.blue   = blue;
                    pixelOut.alpha  = alpha;
			    }
            }

            hasDecompressed = true;
        }
        else if (dxtType == 3)
        {
            const dxt3_block *block = (const dxt3_block*)theTexels + blockIndex;

		    /* calculate colors */
		    const rgb565 col0 = block->col0;
		    const rgb565 col1 = block->col1;
		    uint32 c[4][4];

		    c[0][0] = col0.red * 0xFF/0x1F;
		    c[0][1] = col0.green * 0xFF/0x3F;
		    c[0][2] = col0.blue * 0xFF/0x1F;

		    c[1][0] = col1.red * 0xFF/0x1F;
		    c[1][1] = col1.green * 0xFF/0x3F;
		    c[1][2] = col1.blue * 0xFF/0x1F;

		    c[2][0] = (2*c[0][0] + 1*c[1][0])/3;
		    c[2][1] = (2*c[0][1] + 1*c[1][1])/3;
		    c[2][2] = (2*c[0][2] + 1*c[1][2])/3;

		    c[3][0] = (1*c[0][0] + 2*c[1][0])/3;
		    c[3][1] = (1*c[0][1] + 2*c[1][1])/3;
		    c[3][2] = (1*c[0][2] + 2*c[1][2])/3;

		    /* make index list */
		    uint32 indicesint = block->indexList;
		    uint8 indices[16];

		    for (int32 k = 0; k < 16; k++)
            {
			    indices[k] = indicesint & 0x3;
			    indicesint >>= 2;
		    }

		    uint64 alphasint = block->alphaList;
		    uint8 alphas[16];

		    for (int32 k = 0; k < 16; k++)
            {
			    alphas[k] = (alphasint & 0xF)*17;
			    alphasint >>= 4;
		    }

		    /* write bytes */
		    for (uint32 y_block = 0; y_block < 4; y_block++)
            {
			    for (uint32 x_block = 0; x_block < 4; x_block++)
                {
                    PixelFormat::pixeldata32bit& pixelOut = colorsOut[ y_block ][ x_block ];

                    uint32 coordIndex = PixelFormat::coord2index( x_block, y_block, 4 );

                    uint32 colorIndex = indices[ coordIndex ];

                    uint8 red       = c[ colorIndex ][0];
                    uint8 green     = c[ colorIndex ][1];
                    uint8 blue      = c[ colorIndex ][2];
                    uint8 alpha     = alphas[ coordIndex ];

                    pixelOut.red    = red;
                    pixelOut.green  = green;
                    pixelOut.blue   = blue;
                    pixelOut.alpha  = alpha;
			    }
            }

            hasDecompressed = true;
        }
        else if (dxtType == 4)
        {
            const dxt4_block *block = (const dxt4_block*)theTexels + blockIndex;

		    /* calculate colors */
		    const rgb565 col0 = block->col0;
		    const rgb565 col1 = block->col1;
		    uint32 c[4][4];

		    c[0][0] = col0.red * 0xFF/0x1F;
		    c[0][1] = col0.green * 0xFF/0x3F;
		    c[0][2] = col0.blue * 0xFF/0x1F;

		    c[1][0] = col1.red * 0xFF/0x1F;
		    c[1][1] = col1.green * 0xFF/0x3F;
		    c[1][2] = col1.blue * 0xFF/0x1F;

		    c[2][0] = (2*c[0][0] + 1*c[1][0])/3;
		    c[2][1] = (2*c[0][1] + 1*c[1][1])/3;
		    c[2][2] = (2*c[0][2] + 1*c[1][2])/3;

		    c[3][0] = (1*c[0][0] + 2*c[1][0])/3;
		    c[3][1] = (1*c[0][1] + 2*c[1][1])/3;
		    c[3][2] = (1*c[0][2] + 2*c[1][2])/3;

		    uint32 a[8];
		    a[0] = block->alphaPreMult[0];
		    a[1] = block->alphaPreMult[1];
		    if (a[0] > a[1])
            {
			    a[2] = (6*a[0] + 1*a[1])/7;
			    a[3] = (5*a[0] + 2*a[1])/7;
			    a[4] = (4*a[0] + 3*a[1])/7;
			    a[5] = (3*a[0] + 4*a[1])/7;
			    a[6] = (2*a[0] + 5*a[1])/7;
			    a[7] = (1*a[0] + 6*a[1])/7;
		    }
            else
            {
			    a[2] = (4*a[0] + 1*a[1])/5;
			    a[3] = (3*a[0] + 2*a[1])/5;
			    a[4] = (2*a[0] + 3*a[1])/5;
			    a[5] = (1*a[0] + 4*a[1])/5;
			    a[6] = 0;
			    a[7] = 0xFF;
		    }

		    /* make index list */
		    uint32 indicesint = block->indexList;
		    uint8 indices[16];
		    for (int32 k = 0; k < 16; k++)
            {
			    indices[k] = indicesint & 0x3;
			    indicesint >>= 2;
		    }
		    // actually 6 bytes
		    uint64 alphasint = *((uint64 *) &block->alphaList );
		    uint8 alphas[16];
		    for (int32 k = 0; k < 16; k++)
            {
			    alphas[k] = alphasint & 0x7;
			    alphasint >>= 3;
		    }

		    /* write bytes */
		    for (uint32 y_block = 0; y_block < 4; y_block++)
            {
			    for (uint32 x_block = 0; x_block < 4; x_block++)
                {
                    PixelFormat::pixeldata32bit& pixelOut = colorsOut[ y_block ][ x_block ];

                    uint32 coordIndex = PixelFormat::coord2index( x_block, y_block, 4 );

                    uint32 colorIndex = indices[ coordIndex ];

                    uint8 red       = c[ colorIndex ][0];
                    uint8 green     = c[ colorIndex ][1];
                    uint8 blue      = c[ colorIndex ][2];
                    uint8 alpha     = a[ alphas[ coordIndex ] ];

                    pixelOut.red    = red;
                    pixelOut.green  = green;
                    pixelOut.blue   = blue;
                    pixelOut.alpha  = alpha;
			    }
            }

            hasDecompressed = true;
        }
    }

    if ( !hasDecompressed )
    {
        // Get the decompression flags.
        int dxt_flags = 0;

        bool canDecompress = false;

        if (dxtType == 1)
        {
            dxt_flags |= squish::kDxt1;

            canDecompress = true;
        }
        else if (dxtType == 3)
        {
            dxt_flags |= squish::kDxt3;

            canDecompress = true;
        }
        else if (dxtType == 5)
        {
            dxt_flags |= squish::kDxt5;

            canDecompress = true;
        }

        if (canDecompress)
        {
            // If the native code did not decode the DXT block, try to do it with SQUISH.
            size_t dxtBlockSize = 0;

            if (dxtType == 1)
            {
                dxtBlockSize = sizeof(dxt1_block);
            }
            else if (dxtType == 2 || dxtType == 3)
            {
                dxtBlockSize = sizeof(dxt3_block);
            }
            else if (dxtType == 4 || dxtType == 5)
            {
                dxtBlockSize = sizeof(dxt4_block);
            }

            if ( dxtBlockSize != 0 )
            {
                const void *dxt_block = (const uint8*)theTexels + ( blockIndex * dxtBlockSize );

                squish::Decompress( (squish::u8*)colorsOut, dxt_block, dxt_flags );

                hasDecompressed = true;
            }
        }
    }

    return hasDecompressed;
}

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

        // Make sure the texture dimensions are aligned by 4.
        uint32 alignedMipWidth = ALIGN_SIZE( mipWidth, 4u );
        uint32 alignedMipHeight = ALIGN_SIZE( mipHeight, 4u );

        uint32 texUnitCount = ( alignedMipWidth * alignedMipHeight );

        uint32 dxtDataSize = getDXTRasterDataSize(dxtType, texUnitCount);

        void *dxtArray = new uint8[ dxtDataSize ];

        // Loop across the image.
        uint32 compressedBlockCount = 0;

        uint32 widthBlocks = alignedMipWidth / 4;
        uint32 heightBlocks = alignedMipHeight / 4;

        uint32 y = 0;

        for ( uint32 y_block = 0; y_block < heightBlocks; y_block++, y += 4 )
        {
            uint32 x = 0;

            for ( uint32 x_block = 0; x_block < widthBlocks; x_block++, x += 4 )
            {
                // Compress a 4x4 color block.
                PixelFormat::pixeldata32bit colors[4][4];

                for ( uint32 y_iter = 0; y_iter != 4; y_iter++ )
                {
                    for ( uint32 x_iter = 0; x_iter != 4; x_iter++ )
                    {
                        PixelFormat::pixeldata32bit& inColor = colors[ y_iter ][ x_iter ];

                        uint8 r = 0;
                        uint8 g = 0;
                        uint8 b = 0;
                        uint8 a = 0;

                        uint32 targetX = ( x + x_iter );
                        uint32 targetY = ( y + y_iter );

                        if ( targetX < mipWidth && targetY < mipHeight )
                        {
                            uint32 colorIndex = PixelFormat::coord2index( targetX, targetY, mipWidth );

                            browsetexelcolor(
                                texelSource, paletteType, paletteData, maxpalette,
                                colorIndex, rasterFormat, colorOrder, itemDepth,
                                r, g, b, a
                            );
                        }

                        inColor.red = r;
                        inColor.green = g;
                        inColor.blue = b;
                        inColor.alpha = a;
                    }
                }

                // Compress it using SQUISH.
                int squishFlags = 0;
                bool canCompress = false;

                size_t compressBlockSize = 0;
                void *blockPointer = NULL;

                union
                {
                    dxt1_block _dxt1_block;
                    dxt3_block _dxt3_block;
                    dxt4_block _dxt5_block;
                };

                if ( dxtType == 1 )
                {
                    squishFlags |= squish::kDxt1;

                    canCompress = true;

                    compressBlockSize = sizeof( _dxt1_block );
                    blockPointer = &_dxt1_block;
                }
                else if ( dxtType == 3 )
                {
                    squishFlags |= squish::kDxt3;

                    canCompress = true;

                    compressBlockSize = sizeof( _dxt3_block );
                    blockPointer = &_dxt3_block;
                }
                else if ( dxtType == 5 )
                {
                    squishFlags |= squish::kDxt5;

                    canCompress = true;

                    compressBlockSize = sizeof( _dxt5_block );
                    blockPointer = &_dxt5_block;
                }

                if ( canCompress )
                {
                    squish::Compress( (const squish::u8*)colors, blockPointer, squishFlags );
                }

                // Put the block into the texture.
                memcpy( (char*)dxtArray + compressedBlockCount * compressBlockSize, blockPointer, compressBlockSize );

                // Increment the block count.
                compressedBlockCount++;
            }
        }

        // Delete the raw texels.
        delete [] texelSource;

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