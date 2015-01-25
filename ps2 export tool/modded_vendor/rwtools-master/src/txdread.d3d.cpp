#include <StdInc.h>

#include "txdread.d3d.hxx"

#include "pixelformat.hxx"

namespace rw
{

inline uint32 getCompressionFromD3DFormat( D3DFORMAT d3dFormat )
{
    uint32 compressionIndex = 0;

    if ( d3dFormat == D3DFMT_DXT1 )
    {
        compressionIndex = 1;
    }
    else if ( d3dFormat == D3DFMT_DXT2 )
    {
        compressionIndex = 2;
    }
    else if ( d3dFormat == D3DFMT_DXT3 )
    {
        compressionIndex = 3;
    }
    else if ( d3dFormat == D3DFMT_DXT4 )
    {
        compressionIndex = 4;
    }
    else if ( d3dFormat == D3DFMT_DXT5 )
    {
        compressionIndex = 5;
    }

    return compressionIndex;
}

void NativeTexture::readD3d(std::istream &rw)
{
	HeaderInfo header;

	READ_HEADER(CHUNK_STRUCT);
	
    long texNativeStructOff = rw.tellg();

    uint32 texNativeStructSize = header.getLength();

	uint32 platform = readUInt32(rw);

	if (platform != PLATFORM_D3D8 && platform != PLATFORM_D3D9)
    {
        throw RwException( "invalid platform type in Direct3D texture reading" );
    }

    // Create the Direct3D texture container.
    NativeTextureD3D *platformTex = new NativeTextureD3D();

    if ( !platformTex )
    {
        throw RwException( "failed to allocate memory for Direct3D platform texture reading" );
    }

    // Store the backlink.
    platformTex->parent = this;

    this->platformData = platformTex;

    int engineWarningLevel = rw::rwInterface.GetWarningLevel();

    bool engineIgnoreSecureWarnings = rw::rwInterface.GetIgnoreSecureWarnings();

    // Attempt to read the texture.
    try
    {
        textureMetaHeaderStructGeneric metaHeader;
        rw.read((char*)&metaHeader, sizeof(metaHeader));

	    this->filterFlags = metaHeader.texFormat.filterMode;
        this->uAddressing = metaHeader.texFormat.uAddressing;
        this->vAddressing = metaHeader.texFormat.vAddressing;

        // Read the texture names.
        {
            char tmpbuf[ sizeof( metaHeader.name ) + 1 ];

            // Make sure the name buffer is zero terminted.
            tmpbuf[ sizeof( metaHeader.name ) ] = '\0';

            // Move over the texture name.
            memcpy( tmpbuf, metaHeader.name, sizeof( metaHeader.name ) );

            this->name = tmpbuf;

            // Move over the texture mask name.
            memcpy( tmpbuf, metaHeader.maskName, sizeof( metaHeader.maskName ) );

            this->maskName = tmpbuf;
        }

        // Deconstruct the format flags.
        bool hasMipmaps = false;    // TODO: actually use this flag.

	    readRasterFormatFlags( metaHeader.rasterFormat, this->rasterFormat, platformTex->paletteType, hasMipmaps, platformTex->autoMipmaps );

	    platformTex->hasAlpha = false;

	    if ( platform == PLATFORM_D3D9 )
        {
            D3DFORMAT d3dFormat;

		    rw.read((char*)&d3dFormat, sizeof(d3dFormat));

            // Alpha is not decided here.
            platformTex->d3dFormat = d3dFormat;
        }
	    else
        {
		    platformTex->hasAlpha = ( readUInt32(rw) != 0 );

            // Set d3dFormat later.
        }

        textureMetaHeaderStructDimInfo dimInfo;
        rw.read((char*)&dimInfo, sizeof(dimInfo));

	    uint32 depth = dimInfo.depth;
	    uint32 maybeMipmapCount = dimInfo.mipmapCount;

        platformTex->depth = depth;

        assert( dimInfo.rasterType == 4 );  // TODO

        platformTex->rasterType = dimInfo.rasterType;

	    if ( platform == PLATFORM_D3D9 )
        {
            // Here we decide about alpha.
            textureContentInfoStruct contentInfo;
            rw.read((char*)&contentInfo, sizeof(contentInfo));

		    platformTex->hasAlpha = contentInfo.hasAlpha;
            platformTex->isCubeTexture = contentInfo.isCubeTexture;
            platformTex->autoMipmaps = contentInfo.autoMipMaps;

		    if ( contentInfo.isCompressed )
            {
			    // Detect FOUR-CC versions for compression method.
                uint32 dxtCompression = getCompressionFromD3DFormat(platformTex->d3dFormat);

                if ( dxtCompression == 0 )
                {
                    throw RwException( "invalid Direct3D texture compression format" );
                }

                platformTex->dxtCompression = dxtCompression;
            }
		    else
            {
			    platformTex->dxtCompression = 0;
            }
        }
        else
        {
            uint32 dxtInfo = readUInt8(rw);

            platformTex->dxtCompression = dxtInfo;

            // Auto-decide the Direct3D format.
            D3DFORMAT d3dFormat = D3DFMT_A8R8G8B8;

            if ( dxtInfo != 0 )
            {
                if ( dxtInfo == 1 )
                {
                    d3dFormat = D3DFMT_DXT1;
                }
                else if ( dxtInfo == 2 )
                {
                    d3dFormat = D3DFMT_DXT2;
                }
                else if ( dxtInfo == 3 )
                {
                    d3dFormat = D3DFMT_DXT3;
                }
                else if ( dxtInfo == 4 )
                {
                    d3dFormat = D3DFMT_DXT4;
                }
                else if ( dxtInfo == 5 )
                {
                    d3dFormat = D3DFMT_DXT5;
                }
                else
                {
                    throw RwException( "invalid Direct3D texture compression format" );
                }
            }
            else
            {
                eRasterFormat paletteRasterType = this->rasterFormat;

                bool hasFormat = getD3DFormatFromRasterType( paletteRasterType, COLOR_BGRA, depth, d3dFormat );

                if ( !hasFormat )
                {
                    throw RwException( "could not determine D3DFORMAT for texture " + this->name );
                }
            }

            platformTex->d3dFormat = d3dFormat;
        }

        // Verify raster properties and attempt to fix broken textures.
        // Broken textures travel with mods like San Andreas Retextured.
        // - Verify compression.
        D3DFORMAT d3dFormat = platformTex->d3dFormat;
        {
            uint32 actualCompression = getCompressionFromD3DFormat( d3dFormat );

            if (actualCompression != platformTex->dxtCompression)
            {
                rw::rwInterface.PushWarning( "texture " + this->name + " has invalid compression parameters (ignoring)" );

                platformTex->dxtCompression = actualCompression;
            }
        }
        // - Verify raster format.
        {
            bool isValidFormat = false;

            eColorOrdering colorOrder;
            eRasterFormat d3dRasterFormat;

            if (d3dFormat == D3DFMT_A8R8G8B8)
            {
                d3dRasterFormat = RASTER_8888;

                colorOrder = COLOR_BGRA;

                isValidFormat = true;
            }
            else if (d3dFormat == D3DFMT_X8R8G8B8)
            {
                d3dRasterFormat = RASTER_888;

                colorOrder = COLOR_BGRA;

                isValidFormat = true;
            }
            else if (d3dFormat == D3DFMT_R8G8B8)
            {
                d3dRasterFormat = RASTER_888;

                colorOrder = COLOR_BGRA;

                isValidFormat = true;
            }
            else if (d3dFormat == D3DFMT_R5G6B5)
            {
                d3dRasterFormat = RASTER_565;

                colorOrder = COLOR_BGRA;

                isValidFormat = true;
            }
            else if (d3dFormat == D3DFMT_X1R5G5B5)
            {
                d3dRasterFormat = RASTER_555;

                colorOrder = COLOR_BGRA;

                isValidFormat = true;
            }
            else if (d3dFormat == D3DFMT_A1R5G5B5)
            {
                d3dRasterFormat = RASTER_1555;

                colorOrder = COLOR_BGRA;

                isValidFormat = true;
            }
            else if (d3dFormat == D3DFMT_A4R4G4B4)
            {
                d3dRasterFormat = RASTER_4444;

                colorOrder = COLOR_BGRA;

                isValidFormat = true;
            }
            else if (d3dFormat == D3DFMT_A8B8G8R8)
            {
                d3dRasterFormat = RASTER_8888;

                colorOrder = COLOR_RGBA;

                isValidFormat = true;
            }
            else if (d3dFormat == D3DFMT_X8B8G8R8)
            {
                d3dRasterFormat = RASTER_888;

                colorOrder = COLOR_RGBA;

                isValidFormat = true;
            }
            else if (d3dFormat == D3DFMT_DXT1)
            {
                if (platformTex->hasAlpha)
                {
                    d3dRasterFormat = RASTER_1555;
                }
                else
                {
                    d3dRasterFormat = RASTER_565;
                }

                colorOrder = COLOR_BGRA;

                isValidFormat = true;
            }
            else if (d3dFormat == D3DFMT_DXT2 || d3dFormat == D3DFMT_DXT3)
            {
                d3dRasterFormat = RASTER_4444;

                colorOrder = COLOR_BGRA;

                isValidFormat = true;
            }
            else if (d3dFormat == D3DFMT_DXT4 || d3dFormat == D3DFMT_DXT5)
            {
                d3dRasterFormat = RASTER_4444;

                colorOrder = COLOR_BGRA;

                isValidFormat = true;
            }

            if ( isValidFormat == false )
            {
                throw RwException( "invalid D3DFORMAT in texture " + this->name );
            }

            eRasterFormat rasterFormat = this->rasterFormat;

            if ( rasterFormat != d3dRasterFormat )
            {
                if ( engineWarningLevel >= 3 )
                {
                    rw::rwInterface.PushWarning( "texture " + this->name + " has an invalid raster format (ignoring)" );
                }

                // Fix it.
                this->rasterFormat = d3dRasterFormat;
            }

            // Store the color ordering.
            platformTex->colorOrdering = colorOrder;

            // When reading a texture native, we must have a D3DFORMAT.
            platformTex->hasD3DFormat = true;
        }
        // - Verify depth.
        {
            bool hasInvalidDepth = false;

            if (platformTex->paletteType == PALETTE_4BIT)
            {
                if (depth != 4 && depth != 8)
                {
                    hasInvalidDepth = true;
                }
            }
            else if (platformTex->paletteType == PALETTE_8BIT)
            {
                if (depth != 8)
                {
                    hasInvalidDepth = true;
                }
            }

            if (hasInvalidDepth == true)
            {
                throw RwException( "texture " + this->name + " has an invalid depth" );

                // We cannot fix an invalid depth.
            }
        }

	    if (platformTex->paletteType != PALETTE_NONE)
        {
            uint32 reqPalItemCount = getPaletteItemCount( platformTex->paletteType );

            uint32 palDepth = Bitmap::getRasterFormatDepth( this->rasterFormat );

            assert( palDepth != 0 );

            size_t paletteDataSize = getRasterDataSize( reqPalItemCount, palDepth );

            void *palData = new uint8[paletteDataSize];

		    rw.read(reinterpret_cast <char *> (palData), paletteDataSize);

            // Store the palette.
            platformTex->palette = (uint8*)palData;
            platformTex->paletteSize = reqPalItemCount;
	    }

        uint32 currentMipWidth = dimInfo.width;
        uint32 currentMipHeight = dimInfo.height;

        uint32 mipmapCount = 0;

        uint32 processedMipmapCount = 0;

        uint32 dxtCompression = platformTex->dxtCompression;

        bool hasDamagedMipmaps = false;

	    for (uint32 i = 0; i < maybeMipmapCount; i++)
        {
		    if (i > 0)
            {
                currentMipWidth /= 2;
                currentMipHeight /= 2;
            }

            if (currentMipWidth == 0 || currentMipHeight == 0)
            {
                break;
            }

            uint32 texWidth = currentMipWidth;
            uint32 texHeight = currentMipHeight;

            // Process dimensions.
            {
			    // DXT compression works on 4x4 blocks,
			    // no smaller values allowed
			    if (dxtCompression != 0)
                {
				    texWidth = ALIGN_SIZE( texWidth, 4u );
                    texHeight = ALIGN_SIZE( texHeight, 4u );
			    }
            }

		    uint32 texDataSize = readUInt32(rw);

            // We started processing this mipmap.
            processedMipmapCount++;

            // Verify the data size.
            bool isValidMipmap = true;
            {
                uint32 texItemCount = ( texWidth * texHeight );

                uint32 actualDataSize = 0;

                if (dxtCompression != 0)
                {
                    actualDataSize = getDXTRasterDataSize(dxtCompression, texItemCount);
                }
                else
                {
                    actualDataSize = getRasterDataSize(texItemCount, depth);
                }

                if (actualDataSize != texDataSize)
                {
                    isValidMipmap = false;
                }
            }

            if ( !isValidMipmap )
            {
                // Even the Rockstar games texture generator appeared to have problems with mipmap generation.
                // This is why textures appear to have the size of zero.

                if (texDataSize != 0)
                {
                    if ( !engineIgnoreSecureWarnings )
                    {
                        rw::rwInterface.PushWarning( "texture " + this->name + " has damaged mipmaps (ignoring)" );
                    }
                }

                hasDamagedMipmaps = true;

                // Skip the damaged bytes.
                if (texDataSize != 0)
                {
                    rw.seekg( texDataSize, std::ios::cur );
                }
                break;
            }
            
            uint8 *texelData = new uint8[texDataSize];

		    rw.read(reinterpret_cast <char *> (texelData), texDataSize);

            assert(rw.gcount() == texDataSize);

            // Store mipmap properties.
		    platformTex->width.push_back( texWidth );
		    platformTex->height.push_back( texHeight );

		    platformTex->dataSizes.push_back(texDataSize);

            // Store the image data pointer.
		    platformTex->texels.push_back(texelData);

            mipmapCount++;
        }
        
        if ( mipmapCount == 0 )
        {
            throw RwException( "texture " + this->name + " is empty" );
        }

        // mipmapCount can only be smaller than maybeMipmapCount.
        // This is logically true and would be absurd to assert here.

        if ( processedMipmapCount < maybeMipmapCount )
        {
            // Skip the remaining mipmaps (most likely zero-sized).
            bool hasSkippedNonZeroSized = false;

            for ( uint32 n = processedMipmapCount; n < maybeMipmapCount; n++ )
            {
                uint32 mipSize = readUInt32(rw);

                if ( mipSize != 0 )
                {
                    hasSkippedNonZeroSized = true;

                    // Skip the section.
                    rw.seekg( mipSize, std::ios::cur );
                }
            }

            if ( !engineIgnoreSecureWarnings && !hasDamagedMipmaps )
            {
                // Print the debug message.
                if ( !hasSkippedNonZeroSized )
                {
                    rw::rwInterface.PushWarning( "texture " + this->name + " has zero sized mipmaps" );
                }
                else
                {
                    rw::rwInterface.PushWarning( "texture " + this->name + " violates mipmap rules" );
                }
            }
        }

        platformTex->mipmapCount = mipmapCount;

        // Make sure we go to the end of the texture native struct.
        {
            uint32 curStructOffset = rw.tellg() - texNativeStructOff;

            if ( curStructOffset != texNativeStructSize )
            {
                rw::rwInterface.PushWarning( "texture " + this->name + " has native struct meta data" );

                rw.seekg( texNativeStructOff + texNativeStructSize, std::ios::beg );
            }
        }
    }
    catch( ... )
    {
        // Destroy the temporary data.
        platformTex->Delete();

        // Unlink.
        this->platformData = NULL;

        throw;
    }
}

bool NativeTextureD3D::doesHaveAlpha(void) const
{
    bool hasAlpha = false;

    // Decide whether we even can have alpha.
    // Otherwise there is no point in going through the pixels.
    eRasterFormat rasterFormat = parent->rasterFormat;
    ePaletteType paletteType = this->paletteType;
    eColorOrdering colorOrder = this->colorOrdering;

    if (rasterFormat == RASTER_1555 || rasterFormat == RASTER_4444 || rasterFormat == RASTER_8888)
    {
        // Alright, the raster can have alpha.
        // If we are palettized, we can just check the palette colors.
        if (paletteType != PALETTE_NONE)
        {
            uint32 palItemCount = this->paletteSize;

            void *palColorSource = this->palette;

            uint32 palFormatDepth = Bitmap::getRasterFormatDepth(rasterFormat);

            for (uint32 n = 0; n < palItemCount; n++)
            {
                uint8 r, g, b, a;

                browsetexelcolor(palColorSource, PALETTE_NONE, NULL, 0, n, rasterFormat, colorOrder, palFormatDepth, r, g, b, a);

                if (a != 255)
                {
                    hasAlpha = true;
                    break;
                }
            }
        }
        else
        {
            // We have to process the entire image. Oh boy.
            // For that, we decide based on the main raster only.
            void *texelSource = this->texels[ 0 ];

            uint32 mipWidth = this->width[ 0 ];
            uint32 mipHeight = this->height[ 0 ];

            uint32 mipDepth = this->depth;

            uint32 imageItemCount = ( mipWidth * mipHeight );

            for (uint32 n = 0; n < imageItemCount; n++)
            {
                uint8 r, g, b, a;

                browsetexelcolor(texelSource, PALETTE_NONE, NULL, 0, n, rasterFormat, colorOrder, mipDepth, r, g, b, a);

                if (a != 255)
                {
                    hasAlpha = true;
                    break;
                }
            }
        }
    }

    return hasAlpha;
}

};