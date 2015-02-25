#include <StdInc.h>

#include "txdread.d3d.hxx"

#include "pixelformat.hxx"

#include "txdread.common.hxx"

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

    textureMetaHeaderStructGeneric metaHeader;
    rw.read((char*)&metaHeader, sizeof(metaHeader));

	uint32 platform = metaHeader.platformDescriptor;

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
        // Read texture format.
        metaHeader.texFormat.parse( *this );

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
                ePaletteType paletteType = platformTex->paletteType;
                
                eColorOrdering probableColorOrder = COLOR_BGRA;

                if (paletteType != PALETTE_NONE)
                {
                    probableColorOrder = COLOR_RGBA;
                }

                bool hasFormat = getD3DFormatFromRasterType( paletteRasterType, paletteType, probableColorOrder, depth, d3dFormat );

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
            bool isRasterFormatRequired = true;

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

                isRasterFormatRequired = false;
            }
            else if (d3dFormat == D3DFMT_DXT2 || d3dFormat == D3DFMT_DXT3)
            {
                d3dRasterFormat = RASTER_4444;

                colorOrder = COLOR_BGRA;

                isValidFormat = true;

                isRasterFormatRequired = false;
            }
            else if (d3dFormat == D3DFMT_DXT4 || d3dFormat == D3DFMT_DXT5)
            {
                d3dRasterFormat = RASTER_4444;

                colorOrder = COLOR_BGRA;

                isValidFormat = true;

                isRasterFormatRequired = false;
            }
            else if (d3dFormat == D3DFMT_P8)
            {
                // We are a palette raster.
                // The raster format can be what it is.
                if (platformTex->paletteType != PALETTE_NONE)
                {
                    d3dRasterFormat = this->rasterFormat;

                    colorOrder = COLOR_RGBA;

                    isValidFormat = true;
                }
            }

            if ( isValidFormat == false )
            {
                throw RwException( "invalid D3DFORMAT in texture " + this->name );
            }

            eRasterFormat rasterFormat = this->rasterFormat;

            if ( rasterFormat != d3dRasterFormat )
            {
                if ( isRasterFormatRequired || !engineIgnoreSecureWarnings )
                {
                    if ( engineWarningLevel >= 3 )
                    {
                        rw::rwInterface.PushWarning( "texture " + this->name + " has an invalid raster format (ignoring)" );
                    }
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
        // - Verify auto mipmap
        {
            bool hasAutoMipmaps = platformTex->autoMipmaps;

            if ( hasAutoMipmaps )
            {
                bool canHaveAutoMipmaps = ( platformTex->mipmapCount == 1 );

                if ( !canHaveAutoMipmaps )
                {
                    rw::rwInterface.PushWarning( "texture " + this->name + " has an invalid auto-mipmap flag (fixing)" );

                    platformTex->autoMipmaps = false;
                }
            }
        }

	    if (platformTex->paletteType != PALETTE_NONE)
        {
            uint32 reqPalItemCount = getD3DPaletteCount( platformTex->paletteType );

            uint32 palDepth = Bitmap::getRasterFormatDepth( this->rasterFormat );

            assert( palDepth != 0 );

            size_t paletteDataSize = getRasterDataSize( reqPalItemCount, palDepth );

            void *palData = new uint8[paletteDataSize];

		    rw.read(reinterpret_cast <char *> (palData), paletteDataSize);

            // Store the palette.
            platformTex->palette = palData;
            platformTex->paletteSize = reqPalItemCount;
	    }

        mipGenLevelGenerator mipLevelGen( dimInfo.width, dimInfo.height );

        if ( !mipLevelGen.isValidLevel() )
        {
            throw RwException( "texture " + this->name + " has invalid dimensions" );
        }

        uint32 mipmapCount = 0;

        uint32 processedMipmapCount = 0;

        uint32 dxtCompression = platformTex->dxtCompression;

        bool hasDamagedMipmaps = false;

	    for (uint32 i = 0; i < maybeMipmapCount; i++)
        {
            bool couldEstablishLevel = true;

		    if (i > 0)
            {
                couldEstablishLevel = mipLevelGen.incrementLevel();
            }

            if (!couldEstablishLevel)
            {
                break;
            }

            uint32 texWidth = mipLevelGen.getLevelWidth();
            uint32 texHeight = mipLevelGen.getLevelHeight();

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

                    hasDamagedMipmaps = true;
                }

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

        // Fix filtering mode.
        fixFilteringMode( *this );

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

                bool hasColor = browsetexelcolor(palColorSource, PALETTE_NONE, NULL, 0, n, rasterFormat, colorOrder, palFormatDepth, r, g, b, a);

                if (hasColor && a != 255)
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

                bool hasColor = browsetexelcolor(texelSource, PALETTE_NONE, NULL, 0, n, rasterFormat, colorOrder, mipDepth, r, g, b, a);

                if (hasColor && a != 255)
                {
                    hasAlpha = true;
                    break;
                }
            }
        }
    }

    return hasAlpha;
}

bool NativeTextureD3D::getDebugBitmap( Bitmap& bmpOut ) const
{
    // Return a debug bitmap which contains all mipmap layers.
    uint32 requiredBitmapWidth = 0;
    uint32 requiredBitmapHeight = 0;

    uint32 mipmapCount = this->mipmapCount;

    if ( mipmapCount == 0 )
        return false;

    for ( uint32 n = 0; n < mipmapCount; n++ )
    {
        uint32 mipWidth = this->width[ n ];
        uint32 mipHeight = this->height[ n ];

        // We allocate all mipmaps from top left to top right.
        requiredBitmapWidth += mipWidth;

        if ( requiredBitmapHeight < mipHeight )
        {
            requiredBitmapHeight = mipHeight;
        }
    }

    if ( requiredBitmapWidth == 0 || requiredBitmapHeight == 0 )
        return false;

    // Allocate bitmap space.
    bmpOut.setSize( requiredBitmapWidth, requiredBitmapHeight );

    // Cursor for the drawing operation.
    uint32 cursor_x = 0;
    uint32 cursor_y = 0;

    // Draw them.
    for ( uint32 n = 0; n < mipmapCount; n++ )
    {
        rawBitmapFetchResult rawBmp;

        bool gotRawBitmap = this->getRawBitmap( n, true, rawBmp );

        if ( gotRawBitmap )
        {
            // Fetch colors from this mipmap layer.
            struct mipmapColorSourcePipeline : public Bitmap::sourceColorPipeline
            {
                uint32 mipWidth, mipHeight;
                uint32 depth;
                const void *texelSource;
                eRasterFormat rasterFormat;
                eColorOrdering colorOrder;
                const void *paletteData;
                uint32 paletteSize;
                ePaletteType paletteType;

                inline mipmapColorSourcePipeline(
                    uint32 mipWidth, uint32 mipHeight, uint32 depth,
                    const void *texelSource,
                    eRasterFormat rasterFormat, eColorOrdering colorOrder,
                    const void *paletteData, uint32 paletteSize, ePaletteType paletteType
                )
                {
                    this->mipWidth = mipWidth;
                    this->mipHeight = mipHeight;
                    this->depth = depth;
                    this->texelSource = texelSource;
                    this->rasterFormat = rasterFormat;
                    this->colorOrder = colorOrder;
                    this->paletteData = paletteData;
                    this->paletteSize = paletteSize;
                    this->paletteType = paletteType;
                }

                uint32 getWidth( void ) const
                {
                    return this->mipWidth;
                }

                uint32 getHeight( void ) const
                {
                    return this->mipHeight;
                }

                void fetchcolor( uint32 colorIndex, double& red, double& green, double& blue, double& alpha )
                {
                    uint8 r, g, b, a;

                    bool hasColor = browsetexelcolor(
                        this->texelSource, this->paletteType, this->paletteData, this->paletteSize,
                        colorIndex, this->rasterFormat, this->colorOrder, this->depth,
                        r, g, b, a
                    );

                    if ( !hasColor )
                    {
                        r = 0;
                        g = 0;
                        b = 0;
                        a = 0;
                    }

                    red = unpackcolor( r );
                    green = unpackcolor( g );
                    blue = unpackcolor( b );
                    alpha = unpackcolor( a );
                }
            };

            mipmapColorSourcePipeline colorPipe(
                rawBmp.width, rawBmp.height, rawBmp.depth,
                rawBmp.texelData,
                rawBmp.rasterFormat, rawBmp.colorOrder,
                rawBmp.paletteData, rawBmp.paletteSize, rawBmp.paletteType
            );

            // Draw it at its position.
            bmpOut.draw(
                colorPipe, cursor_x, cursor_y,
                rawBmp.width, rawBmp.height,
                Bitmap::SHADE_ZERO, Bitmap::SHADE_ONE, Bitmap::BLEND_ADDITIVE
            );

            // Delete if necessary.
            if ( rawBmp.isNewlyAllocated )
            {
                delete [] rawBmp.texelData;
            }

            // Increase cursor.
            cursor_x += rawBmp.width;
        }
    }

    return true;
}

};