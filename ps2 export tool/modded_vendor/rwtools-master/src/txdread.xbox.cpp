#include <StdInc.h>

#include "txdread.xbox.hxx"

#include "txdread.d3d.hxx"

namespace rw
{

void NativeTexture::readXbox(std::istream &rw)
{
	HeaderInfo header;

	READ_HEADER(CHUNK_STRUCT);

    long texNativeStructOff = rw.tellg();
    uint32 texNativeStructSize = header.getLength();

	uint32 platform = readUInt32(rw);

	// improve error handling
	if (platform != PLATFORM_XBOX)
		return;

    // Create the platform data container.
    NativeTextureXBOX *platformTex = new NativeTextureXBOX();

    // same as above.
    if ( !platformTex )
        return;

    // Set up the backlink.
    platformTex->parent = this;

    this->platformData = platformTex;

    int engineWarningLevel = rw::rwInterface.GetWarningLevel();

    // Attempt reading the data.
    try
    {
        textureMetaHeaderStructXbox metaInfo;
        rw.read((char*)&metaInfo, sizeof(metaInfo));

	    this->filterFlags = metaInfo.formatInfo.filterMode;
        this->uAddressing = metaInfo.formatInfo.uAddressing;
        this->vAddressing = metaInfo.formatInfo.vAddressing;

        // Read the texture names.
        {
            char tmpbuf[ sizeof( metaInfo.name ) + 1 ];

            // Make sure the name buffer is zero terminted.
            tmpbuf[ sizeof( metaInfo.name ) ] = '\0';

            // Move over the texture name.
            memcpy( tmpbuf, metaInfo.name, sizeof( metaInfo.name ) );

            this->name = tmpbuf;

            // Move over the texture mask name.
            memcpy( tmpbuf, metaInfo.maskName, sizeof( metaInfo.maskName ) );

            this->maskName = tmpbuf;
        }

        // Deconstruct the rasterFormat flags.
        bool hasMipmaps = false;        // TODO: actually use this flag.
        bool autoMipmaps = false;

        readRasterFormatFlags( metaInfo.rasterFormat, this->rasterFormat, platformTex->paletteType, hasMipmaps, autoMipmaps );

	    platformTex->hasAlpha = ( metaInfo.hasAlpha != 0 );

	    uint32 depth = metaInfo.depth;
	    uint32 maybeMipmapCount = metaInfo.mipmapCount;

        platformTex->rasterType = metaInfo.rasterType;

	    platformTex->dxtCompression = metaInfo.dxtCompression;

        platformTex->autoMipmaps = autoMipmaps;

        // TODO: for now I assume that XBOX textures always have BGRA color ordering.
        platformTex->colorOrder = COLOR_BGRA;

        // Verify the parameters.
        eRasterFormat rasterFormat = this->rasterFormat;
        ePaletteType paletteType = platformTex->paletteType;

        // - depth
        {
            bool isValidDepth = true;

            if (paletteType == PALETTE_4BIT)
            {
                if (depth != 4 && depth != 8)
                {
                    isValidDepth = false;
                }
            }
            else if (paletteType == PALETTE_8BIT)
            {
                if (depth != 8)
                {
                    isValidDepth = false;
                }
            }
            // TODO: find more ways of verification here.

            if (isValidDepth == false)
            {
                // We cannot repair a broken depth.
                throw RwException( "texture " + this->name + " has an invalid depth" );
            }
        }

        uint32 remainingImageSectionData = metaInfo.imageDataSectionSize;

	    if (paletteType != PALETTE_NONE)
        {
            uint32 palItemCount = 0;

            if (paletteType == PALETTE_4BIT)
            {
                palItemCount = 32;
            }
            else if (paletteType == PALETTE_8BIT)
            {
                palItemCount = 256;
            }
            else
            {
                assert( 0 );
            }

            uint32 palDepth = Bitmap::getRasterFormatDepth( rasterFormat );

            uint32 paletteDataSize = getRasterDataSize( palItemCount, palDepth );

		    void *palData = new uint8[ paletteDataSize ];
		    rw.read(reinterpret_cast <char *> (palData), paletteDataSize);

            // Write the parameters.
            platformTex->palette = palData;
		    platformTex->paletteSize = palItemCount;
	    }

        // Read all the textures.
        uint32 actualMipmapCount = 0;

        uint32 currentMipWidth = metaInfo.width;
        uint32 currentMipHeight = metaInfo.height;

        uint32 dxtCompression = platformTex->dxtCompression;

	    for (uint32 i = 0; i < maybeMipmapCount; i++)
        {
            if ( remainingImageSectionData == 0 )
            {
                break;
            }

		    if (i > 0)
            {
                currentMipWidth /= 2;
                currentMipHeight /= 2;
            }

            uint32 texWidth = currentMipWidth;
            uint32 texHeight = currentMipHeight;

            // Process dimensions.
            {
			    // DXT compression works on 4x4 blocks,
			    // align the dimensions.
			    if (dxtCompression != 0)
                {
				    texWidth = ALIGN_SIZE( texWidth, 4u );
                    texHeight = ALIGN_SIZE( texHeight, 4u );
			    }
            }

            // Calculate the data size of this mipmap.
            uint32 texUnitCount = ( texWidth * texHeight );
            uint32 texDataSize = 0;

            if (dxtCompression != 0)
            {
                uint32 dxtType = 0;

		        if (dxtCompression == 0xC)	// DXT1 (?)
                {
                    dxtType = 1;
                }
                else if (dxtCompression == 0xD)
                {
                    dxtType = 2;
                }
                else if (dxtCompression == 0xE)
                {
                    dxtType = 3;
                }
                else if (dxtCompression == 0xF)
                {
                    dxtType = 4;
                }
                else if (dxtCompression == 0x10)
                {
                    dxtType = 5;
                }
                else
                {
                    throw RwException( "texture " + this->name + " has an unknown compression type" );
                }
		        
                texDataSize = getDXTRasterDataSize(dxtType, texUnitCount);
            }
            else
            {
                // There should also be raw rasters supported.
                texDataSize = getRasterDataSize(texUnitCount, depth);
            }

            if ( remainingImageSectionData < texDataSize )
            {
                throw RwException( "texture " + this->name + " has an invalid image data section size parameter" );
            }

            // Decrement the overall remaining size.
            remainingImageSectionData -= texDataSize;

            // Store the texture size.
            platformTex->dataSizes.push_back( texDataSize );

            // Store mipmap properties.
            platformTex->width.push_back( texWidth );
            platformTex->height.push_back( texHeight );

            platformTex->mipmapDepth.push_back( depth );

            uint8 *texelData = new uint8[texDataSize];

		    platformTex->texels.push_back(texelData);
		    rw.read(reinterpret_cast <char *> (texelData), texDataSize);

            assert( rw.gcount() == texDataSize );

            // Increase mipmap count.
            actualMipmapCount++;
	    }

        if ( actualMipmapCount == 0 )
        {
            throw RwException( "texture " + this->name + " is empty" );
        }

        if ( remainingImageSectionData != 0 )
        {
            if ( engineWarningLevel >= 2 )
            {
                rw::rwInterface.PushWarning( "texture " + this->name + " appears to have image section meta-data" );
            }
        }

        // Store remaining mipmap info.
        platformTex->mipmapCount = actualMipmapCount;

        // Check whether we reached the end of the texture native struct.
        {
            uint32 curTexNativeOffset = rw.tellg() - texNativeStructOff;

            if ( curTexNativeOffset != texNativeStructSize )
            {
                __asm nop
            }
        }
    }
    catch( rw::RwException& )
    {
        // Destroy the platform texture again.
        platformTex->Delete();

        // Unlink the texture again.
        this->platformData = NULL;

        throw;
    }
}

// http://gtaforums.com/topic/213907-unswizzle-tool/
// TODO: make this function a template.
void unswizzleXboxBlock
(
    const uint8 *srcData, uint8 *outData,
    uint32& srcDataOffset, uint32 outDataOffset,
    uint32 imWidth, uint32 imHeight, uint32 imStride
)
{
	if (imWidth < 2 || imHeight < 2)
    {
        // This part of the algorithm is buggy. why?
            
		memcpy( outData + outDataOffset, srcData + srcDataOffset, imWidth*imHeight );

        // Wtf is this.
		srcDataOffset += imWidth*imHeight;
	}
    else if (imWidth == 2 && imHeight == 2)
    {
		*(outData+outDataOffset)                    = *(srcData+srcDataOffset);
		*(outData+outDataOffset+1)                  = *(srcData+srcDataOffset+1);
		*(outData+outDataOffset+imStride)           = *(srcData+srcDataOffset+2);
		*(outData+outDataOffset+imStride+1)         = *(srcData+srcDataOffset+3);

        // Go to the next block.
		srcDataOffset += 4;
	}
    else
    {
        // We split the picture into four parts, recursively.
        uint32 subImWidth = ( imWidth / 2 );
        uint32 subImHeight = ( imHeight / 2 );

        // Top left.
		unswizzleXboxBlock(
            srcData, outData, srcDataOffset,
            outDataOffset,
            subImWidth, subImHeight, imStride
        );

        // Top right.
		unswizzleXboxBlock(
            srcData, outData, srcDataOffset,
            outDataOffset + subImWidth,
            subImWidth, subImHeight, imStride
        );

        // Bottom left.
		unswizzleXboxBlock(
            srcData, outData, srcDataOffset,
            outDataOffset + subImHeight*imStride,
            subImWidth, subImHeight, imStride
        );

        // Bottom right.
		unswizzleXboxBlock(
            srcData, outData, srcDataOffset,
            outDataOffset + subImHeight*imStride + (imWidth/2),
            subImWidth, subImHeight, imStride
        );
	}
}

void NativeTexture::convertFromXbox(void)
{
    if ( platform != PLATFORM_XBOX )
        return;

    // Allocate our new texture container.
    NativeTextureD3D *d3dTex = new NativeTextureD3D();

    if ( !d3dTex )
        return;

    NativeTextureXBOX *platformTex = (NativeTextureXBOX*)this->platformData;

    // Copy common data.
    d3dTex->width = platformTex->width;
    d3dTex->height = platformTex->height;
    d3dTex->mipmapDepth = platformTex->mipmapDepth;

    // Move over palette information.
    d3dTex->palette = platformTex->palette;
    d3dTex->paletteSize = platformTex->paletteSize;
    d3dTex->paletteType = platformTex->paletteType;

    d3dTex->rasterType = platformTex->rasterType;

    // Decide about the D3DFORMAT.
    D3DFORMAT theD3DFormat;
    bool hasD3DFormat = false;

    uint32 targetCompression = 0;

    uint32 srcCompressionType = platformTex->dxtCompression;

    if (srcCompressionType != 0)
    {
        if (srcCompressionType == 0xc)
        {
            targetCompression = 1;

		    if (platformTex->hasAlpha)
            {
			    this->rasterFormat = RASTER_1555;
            }
		    else
            {
			    this->rasterFormat = RASTER_565;
            }

            theD3DFormat = D3DFMT_DXT1;

            hasD3DFormat = true;
	    }
        else if (srcCompressionType == 0xe)
        {
		    targetCompression = 3;

            this->rasterFormat = RASTER_4444;

            theD3DFormat = D3DFMT_DXT3;

            hasD3DFormat = true;
        }
        else if (srcCompressionType == 0xf)
        {
		    targetCompression = 4;

            theD3DFormat = D3DFMT_DXT4;

            hasD3DFormat = true;
	    }
        else
        {
            // TODO: DXT2, DXT5
            assert( 0 );
        }
    }
    else
    {
        // If there is no compression, the D3DFORMAT field is made up of the rasterFormat.
        hasD3DFormat = getD3DFormatFromRasterType(this->rasterFormat, platformTex->colorOrder, platformTex->mipmapDepth[ 0 ], theD3DFormat);

        if ( true )
        {
            // Unswizzle the mipmaps.
            for ( uint32 n = 0; n < platformTex->mipmapCount; n++ )
            {
                uint32 mipWidth = platformTex->width[ n ];
                uint32 mipHeight = platformTex->height[ n ];
                uint32 mipDepth = platformTex->mipmapDepth[ n ];

                if ( mipDepth == 8 )
                {
                    uint32 texelCount = ( mipWidth * mipHeight );

                    uint32 dataSize = getRasterDataSize(texelCount, mipDepth);
                    
                    // Let's try allocating a new array for the unswizzled texels.
                    void *newtexels = new uint8[ dataSize ];

                    void *srcTexels = platformTex->texels[ n ];

                    uint32 offOut = 0;

                    unswizzleXboxBlock( (const rw::uint8*)srcTexels, (rw::uint8*)newtexels, offOut, 0, mipWidth, mipHeight, mipWidth );

                    // Replace texels.
                    delete [] srcTexels;

                    platformTex->texels[ n ] = newtexels;
                }
                else
                {
                    __asm nop
                }
            }
        }
    }

    // Move our texel information.
    // Some texture types have to be unswizzled beforehand.
    

    d3dTex->texels = platformTex->texels;
    d3dTex->dataSizes = platformTex->dataSizes;

    d3dTex->mipmapCount = platformTex->mipmapCount;

    d3dTex->dxtCompression = targetCompression;

    d3dTex->hasAlpha = platformTex->hasAlpha;

    d3dTex->colorOrdering = platformTex->colorOrder;

    d3dTex->autoMipmaps = platformTex->autoMipmaps;

    // Calculate the raster format.
    if (hasD3DFormat)
    {
        d3dTex->d3dFormat = theD3DFormat;
    }
    d3dTex->hasD3DFormat = hasD3DFormat;

    // Backlink and put our texture as new platform data.
    d3dTex->parent = this;

    this->platformData = d3dTex;

    // Delete the old container.
    delete platformTex;

    platform = PLATFORM_D3D8;
}

void NativeTexture::convertToXbox(void)
{
    // Alright, lets convert.
    if ( platform != PLATFORM_D3D8 && platform != PLATFORM_D3D9 )
        return;

    NativeTextureD3D *platformTex = (NativeTextureD3D*)this->platformData;

    // Create a XBOX platform texture data container.
    NativeTextureXBOX *xboxTex = new NativeTextureXBOX();

    if ( !xboxTex )
        return;

    // Move over common data.
    xboxTex->width = platformTex->width;
    xboxTex->height = platformTex->height;
    xboxTex->mipmapDepth = platformTex->mipmapDepth;

    // Move over palettization information.
    xboxTex->palette = platformTex->palette;
    xboxTex->paletteSize = platformTex->paletteSize;
    xboxTex->paletteType = platformTex->paletteType;

    xboxTex->rasterType = platformTex->rasterType;

    // Move over the texture data.
    uint32 dxtCompression = platformTex->dxtCompression;

    uint32 xboxCompressionType = 0;

    uint32 mipmapCount = platformTex->mipmapCount;

    if ( dxtCompression != 0 )
    {
        if ( dxtCompression == 1 )
        {
            xboxCompressionType = 0xC;
        }
        else if ( dxtCompression == 2 )
        {
            xboxCompressionType = 0xD;
        }
        else if ( dxtCompression == 3 )
        {
            xboxCompressionType = 0xE;
        }
        else if ( dxtCompression == 4 )
        {
            xboxCompressionType = 0xF;
        }
        else if ( dxtCompression == 5 )
        {
            xboxCompressionType = 0x10;
        }
        else
        {
            assert( 0 );
        }

        // Compressed rasters are what they are.
        // They do not need swizzling.
    }
    else
    {
        // We are a raw raster; take care about swizzling.
        for ( uint32 n = 0; n < mipmapCount; n++ )
        {
            uint32 mipWidth = platformTex->width[ n ];
            uint32 mipHeight = platformTex->height[ n ];
            uint32 mipDepth = platformTex->mipmapDepth[ n ];
            void *srcTexels = platformTex->texels[ n ];

            
        }
    }

    // Store texel data.
    xboxTex->texels = platformTex->texels;
    xboxTex->dataSizes = platformTex->dataSizes;
    xboxTex->mipmapCount = mipmapCount;

    xboxTex->hasAlpha = platformTex->hasAlpha;

    xboxTex->colorOrder = platformTex->colorOrdering;

    // Store compression parameters.
    xboxTex->dxtCompression = xboxCompressionType;

    xboxTex->autoMipmaps = platformTex->autoMipmaps;

    // Backlink and set pointer to general data.
    xboxTex->parent = this;

    this->platformData = xboxTex;

    // Delete the old container.
    delete platformTex;

    platform = PLATFORM_XBOX;
}

};