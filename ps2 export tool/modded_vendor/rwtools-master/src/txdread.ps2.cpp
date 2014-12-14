#include <StdInc.h>

#include <bitset>
#define _USE_MATH_DEFINES
#include <math.h>
#include <map>
#include <algorithm>
#include <cmath>

#include "txdread.ps2.hxx"
#include "txdread.d3d.hxx"

#include "pixelformat.hxx"

namespace rw
{

void NativeTexture::readPs2(std::istream &rw)
{
	HeaderInfo header;

	READ_HEADER(CHUNK_STRUCT);
	uint32 platform = readUInt32(rw);
	// improve error handling
	if (platform != PLATFORM_PS2FOURCC)
		return;

    // Allocate a platform type texture.
    NativeTexturePS2 *platformTex = new NativeTexturePS2();

    // same as above.
    if ( platformTex == NULL )
        return;

    // Store the backlink.
    platformTex->parent = this;

    // Give the texture link to our native data.
    this->platformData = platformTex;

	platformTex->paletteSize = 0;

    texFormatInfo formatInfo;
    rw.read((char*)&formatInfo, sizeof(formatInfo));

    this->filterFlags = formatInfo.filterMode;
    this->uAddressing = formatInfo.uAddressing;
    this->vAddressing = formatInfo.vAddressing;
	
	READ_HEADER(CHUNK_STRING);
	char *buffer = new char[header.length+1];
	rw.read(buffer, header.length);
    buffer[header.length] = '\0';
	this->name = buffer;
	delete[] buffer;

	READ_HEADER(CHUNK_STRING);
	buffer = new char[header.length+1];
	rw.read(buffer, header.length);
    buffer[header.length] = '\0';
	this->maskName = buffer;
	delete[] buffer;

    // Texture Struct.
	READ_HEADER(CHUNK_STRUCT);

    uint32 texStructSize = header.length;
    long texStructPos = rw.tellg();

	READ_HEADER(CHUNK_STRUCT);

    assert( header.length == sizeof( textureMetaDataHeader ) );
    
    textureMetaDataHeader textureMeta;
    rw.read((char*)&textureMeta, sizeof(textureMetaDataHeader));

	platformTex->width.push_back(textureMeta.width);
	platformTex->height.push_back(textureMeta.height);
	uint32 depth = textureMeta.depth;

    // Deconstruct the rasterFormat.
    bool hasMipmaps = false;        // TODO: actually use this flag.

    readRasterFormatFlags( textureMeta.rasterFormat, this->rasterFormat, platformTex->paletteType, hasMipmaps, platformTex->autoMipmaps );

    platformTex->requiresHeaders = ( textureMeta.rasterFormat & 0x20000 ) != 0;

    // Some really unknown format flags. We should debug this!
    platformTex->unknownFormatFlags = ( textureMeta.rasterFormat & 0xFF );

    // Store unique parameters from the texture registers.
    platformTex->gsParams.maxMIPLevel = textureMeta.tex1.maximumMIPLevel;
    platformTex->gsParams.mtba = textureMeta.tex1.mtba;
    platformTex->gsParams.textureFunction = textureMeta.tex0.texFunction;
    platformTex->gsParams.lodCalculationModel = textureMeta.tex1.lodCalculationModel;
    platformTex->gsParams.mmag = textureMeta.tex1.mmag;
    platformTex->gsParams.mmin = textureMeta.tex1.mmin;
    platformTex->gsParams.lodParamL = textureMeta.tex1.lodParamL;
    platformTex->gsParams.lodParamK = textureMeta.tex1.lodParamK;

    platformTex->gsParams.gsTEX1Unknown1 = textureMeta.tex1.unknown;
    platformTex->gsParams.gsTEX1Unknown2 = textureMeta.tex1.unknown2;

	uint32 dataSize = textureMeta.dataSize;
	uint32 paletteDataSize = textureMeta.paletteDataSize;

    platformTex->skyMipMapVal = textureMeta.skyMipmapVal;

	// 0x00000 means the texture is not swizzled and has no headers
	// 0x10000 means the texture is swizzled and has no headers
	// 0x20000 means swizzling information is contained in the header
	// the rest is the same as the generic raster format
	bool hasHeader = platformTex->requiresHeaders;
    bool hasSwizzle = (textureMeta.rasterFormat & 0x10000) != 0;

/*
	// only these are ever used (so alpha for all textures :/ )
	if ((rasterFormat == RASTER_1555) ||
	    (rasterFormat == RASTER_4444) ||
	    (rasterFormat == RASTER_8888))
		hasAlpha = true;
	else
		hasAlpha = false;
*/
	hasAlpha = false;

//	cout << " " << maskName;
	if (maskName.size() != 0 || depth == 16)
    {
		hasAlpha = true;
    }
//	cout << " " << hasAlpha;

	READ_HEADER(CHUNK_STRUCT);

    // Decide about texture properties.
    bool isPalettized = (platformTex->paletteType != PALETTE_NONE);
    bool isSwizzled = false;

    if (hasHeader)
    {
        if (isPalettized)
        {
            isSwizzled = true;

            assert(depth == 4 || depth == 8);
        }
    }
    else
    {
		if (hasSwizzle)
        {
            isSwizzled = true;
		}
    }

    // Absolute maximum of mipmaps.
    const size_t maxMipmaps = 7;

    ps2MipmapTransmissionData _origMipmapTransData[ maxMipmaps ];

	/* Pixels/Indices */
	long end = rw.tellg();
	end += (long)dataSize;
	uint32 i = 0;
	while (rw.tellg() < end)
    {
        uint32 texWidth = 0;
        uint32 texHeight = 0;

	    // half dimensions if we have mipmaps
	    if (i > 0)
        {
		    texWidth = platformTex->width[i-1] / 2;
		    texHeight = platformTex->height[i-1] / 2;
	    }
        else
        {
            texWidth = textureMeta.width;
            texHeight = textureMeta.height;
        }

        uint32 texDataSize = 0;

        uint32 mipOffX = 0;
        uint32 mipOffY = 0;

		if (hasHeader)
        {
            uint32 vImgWidth, vImgHeight, vImgDataSize;
            uint16 vImgSize1, vImgSize2;
    
            bool validTex = readImageDataHeader(rw, vImgWidth, vImgHeight, vImgDataSize, vImgSize1, vImgSize2);

            if ( validTex )
            {
                bool requiresDoubling = false;

                if (header.version == rw::GTA3_1 || header.version == rw::GTA3_2 ||
                    header.version == rw::GTA3_3 || header.version == rw::GTA3_4)
                {
                    if (isSwizzled && textureMeta.depth == 8)
                    {
                        requiresDoubling = true;
                    }
                }
                else
                {
                    if (isSwizzled)
                    {
                        requiresDoubling = true;
                    }
                }

                if (requiresDoubling)
                {
                    vImgWidth *= 2;
                    vImgHeight *= 2;
                }

			    platformTex->swizzleWidth.push_back(vImgWidth);
			    platformTex->swizzleHeight.push_back(vImgHeight);
			    texDataSize = vImgDataSize;

                mipOffX = vImgSize1;
                mipOffY = vImgSize2;
            }

            // TODO: there is seemingly invalid data in some texture archives; figure out what to do with it...
		}
        else
        {
            uint32 texItems = ( texWidth * texHeight );

			platformTex->swizzleWidth.push_back(texWidth);
			platformTex->swizzleHeight.push_back(texHeight);

			texDataSize = texItems*depth/8;
		}

        if ( texDataSize == 0 )
        {
            rw.seekg(end, std::ios::beg);
            break;
        }

        platformTex->mipmapDepth.push_back( depth );
        platformTex->isSwizzled.push_back(isSwizzled);

        // Store mipmap offsets (for debugging).
        if ( i < maxMipmaps )
        {
            ps2MipmapTransmissionData& transData = _origMipmapTransData[ i ];

            transData.destX = mipOffX;
            transData.destY = mipOffY;
        }

		platformTex->dataSizes.push_back(texDataSize);

        uint8 *texelData = NULL;

        if ( texDataSize != 0 )
        {
            texelData = new uint8[texDataSize];

		    rw.read(reinterpret_cast <char *> (texelData), texDataSize*sizeof(uint8));
        }

	    if (i > 0)
        {
            platformTex->width.push_back(texWidth);
            platformTex->height.push_back(texHeight);
        }

        platformTex->texels.push_back(texelData);

	    i++;
	}
	platformTex->mipmapCount = i;

    // Assume we have at least one texture.
    assert( platformTex->mipmapCount > 0 );

	/* Palette */
	// vc dyn_trash.txd is weird here
	if (isPalettized)
    {
        uint32 paletteColorComponents = 0;
        uint32 palTexDataSize = 0;
        uint16 gpuSize1 = 0;
        uint16 gpuSize2 = 0;

		if (hasHeader)
        {
            // Basically, the palette is a texture, too!
            uint32 palTexWidth, palTexHeight, _dataSize;

            bool validTex = readImageDataHeader(rw, palTexWidth, palTexHeight, _dataSize, gpuSize1, gpuSize2);

            if ( validTex )
            {
                if ( textureMeta.depth == 4 )
                {
                    if ( gpuSize2 != 0 || gpuSize1 != 0 )
                    {
                        __asm nop
                    }
                }
                else if ( textureMeta.depth == 8 )
                {
                    if ( gpuSize2 != 0 && gpuSize2 * 2 != textureMeta.height )
                    {
                        __asm nop
                    }

                    if ( gpuSize1 == 0 && gpuSize2 == 0 )
                    {
                        __asm nop
                    }
                    else
                    {
                        __asm nop
                    }
                }

                // Give parameters to the runtime to load the palette.
			    paletteColorComponents = ( palTexWidth * palTexHeight );

                palTexDataSize = _dataSize;
            }
		}
        else
        {
            paletteColorComponents = getPaletteItemCount( platformTex->paletteType );

            palTexDataSize = textureMeta.paletteDataSize;
        }

        if ( paletteColorComponents != 0 )
        {
            size_t paletteDataSize = palTexDataSize;

            platformTex->palUnknowns.destX = gpuSize1;
            platformTex->palUnknowns.destY = gpuSize2;

            platformTex->paletteSize = ( paletteColorComponents );
		    platformTex->palette = new uint8[ paletteDataSize ];
		    rw.read(reinterpret_cast <char *> (platformTex->palette), paletteDataSize);
        }
	}

    // Make sure we are at the end of the tex native struct.
    // vc "dyn_trash.txd" appears to be damaged.
    rw.seekg( texStructPos + texStructSize, std::ios::beg );

    // Allocate texture memory.
    uint32 mipmapBasePointer[ maxMipmaps ];
    uint32 mipmapMemorySize[ maxMipmaps ];
    uint32 mipmapBufferWidth[ maxMipmaps ];

    ps2MipmapTransmissionData mipmapTransData[ maxMipmaps ];

    eMemoryLayoutType memLayoutType;

    bool hasAllocatedMemory = platformTex->allocateTextureMemory(mipmapBasePointer, mipmapBufferWidth, mipmapMemorySize, mipmapTransData, maxMipmaps, memLayoutType);

    // Could fail if no memory left.
    if ( !hasAllocatedMemory )
    {
        assert( 0 );
    }

    // Verify that our memory calculation routine is correct.
    uint32 gpuMinMemory = platformTex->calculateGPUDataSize(mipmapBasePointer, mipmapMemorySize, maxMipmaps, memLayoutType, platformTex->paletteSize);

    if ( textureMeta.combinedGPUDataSize > gpuMinMemory )
    {
        // If this assertion is triggered, then a adjust the gpu size calculation algorithm
        // so it outputs a big enough number.
        assert( 0 );
    }

    if ( textureMeta.combinedGPUDataSize != gpuMinMemory )
    {
        // It would be perfect if this condition were never triggered for official R* games textures.
        __asm nop
    }

    // Verify that our GPU data calculation routine is correct.
    ps2GSRegisters gpuData;
    
    bool isValidTexture = platformTex->generatePS2GPUData(gpuData, mipmapBasePointer, mipmapBufferWidth, mipmapMemorySize, maxMipmaps, memLayoutType);

    // If any of those assertions fail then either our is routine incomplete
    // or the input texture is invalid (created by wrong tool probably.)
    assert( isValidTexture == true );
    if ( gpuData.tex0 != textureMeta.tex0 )
    {
        __asm nop
    }
    if ( gpuData.tex1 != textureMeta.tex1 )
    {
        __asm nop
    }
    if ( gpuData.miptbp1 != textureMeta.miptbp1 )
    {
        __asm nop
    }
    if ( gpuData.miptbp2 != textureMeta.miptbp2 )
    {
        __asm nop
    }

    if ( textureMeta.depth == 8 )
    {
        if ( platformTex->mipmapCount > 1 )
        {
            __asm nop
        }
    }

    // Verify transmission rectangle same-ness.
    for ( uint32 n = 0; n < platformTex->mipmapCount; n++ )
    {
        const ps2MipmapTransmissionData& srcTransData = _origMipmapTransData[ n ];
        const ps2MipmapTransmissionData& dstTransData = mipmapTransData[ n ];

        if ( srcTransData.destX != srcTransData.destX ||
             srcTransData.destY != srcTransData.destY )
        {
            __asm nop
        }
    }

    // Weird debugging shit.
    platformTex->PerformDebugChecks();
}

/* convert from CLUT format used by the ps2 */
static void unclut(uint8 *texels, uint32 width, uint32 height)
{
	for (uint32 i = 0; i < width*height; i++)
    {
        uint8 clutIndex = texels[i];

        uint32 middleBits = ( clutIndex & 0x18 ) >> 3;

        uint32 lowerBits = ( clutIndex & ~0x18 );

        uint32 newMiddleBits = 0;

        if ( middleBits == 0 )
        {
            newMiddleBits = 0;
        }
        else if ( middleBits == 1 )
        {
            newMiddleBits = 16;
        }
        else if ( middleBits == 2 )
        {
            newMiddleBits = 8;
        }
        else if ( middleBits == 3 )
        {
            newMiddleBits = 24;
        }

        assert( ( newMiddleBits & lowerBits ) == 0 );

		texels[i] = ( lowerBits | newMiddleBits );
    }
}

// convert to CLUT format
static void clut(uint8 *texels, uint32 width, uint32 height)
{
	// actually exactly the same as unclut.
    unclut( texels, width, height );
}

void NativeTexture::convertFromPS2(uint32 aref)
{
	if (platform != PLATFORM_PS2)
		return;

    // Create a new Direct3D platform texture.
    NativeTextureD3D *d3dtex = new NativeTextureD3D();

    if ( !d3dtex )
        return;

    NativeTexturePS2 *platformTex = (NativeTexturePS2*)this->platformData;

    // Copy over general attributes.
    d3dtex->width = platformTex->width;
    d3dtex->height = platformTex->height;
    d3dtex->mipmapCount = platformTex->mipmapCount;
    d3dtex->mipmapDepth = platformTex->mipmapDepth;

    d3dtex->autoMipmaps = platformTex->autoMipmaps;

	for (uint32 j = 0; j < platformTex->mipmapCount; j++)
    {
		bool swizzled = (platformTex->isSwizzled[j]);

        uint32 depth = platformTex->mipmapDepth[j];

		if (depth == 4)
        {
			if (swizzled)
            {
				platformTex->swizzleDecryptPS2(j);
            }
		}
        else if (depth == 8)
        {
			if (swizzled)
            {
				platformTex->swizzleDecryptPS2(j);
            }

            uint8 *texels = (uint8*)platformTex->texels[j];
			unclut(texels, platformTex->width[j], platformTex->height[j]);
        }
        else if (depth == 32)
        {
            typedef PixelFormat::texeltemplate <PixelFormat::pixeldata32bit> pixel32_t;

            pixel32_t *texelData = (pixel32_t*)platformTex->texels[j];

			for (uint32 i = 0; i < platformTex->height[j] * platformTex->width[j]; i++)
            {
                uint8 red, green, blue, alpha;
                texelData->getcolor(i, red, green, blue, alpha);

			    // swap R and B
			    // fix alpha
                texelData->setcolor(i, blue, green, red, (uint32)alpha * 0xFF / 0x80);
			}
		}
        else
        {
            assert( 0 );
        }
	}

	if (platformTex->paletteType != PALETTE_NONE)
    {
        void *paletteTexelSource = platformTex->palette;

		for (uint32 i = 0; i < platformTex->paletteSize; i++)
        {
            uint8 red, green, blue, alpha;

            browsetexelcolor(paletteTexelSource, PALETTE_NONE, NULL, 0, i, this->rasterFormat, red, green, blue, alpha);
            {
			    if ((platformTex->alphaDistribution & 0x1) == 0 && alpha >= aref)
                {
				    platformTex->alphaDistribution |= 0x1;
                }
			    else if ((platformTex->alphaDistribution & 0x2) == 0 && alpha < aref)
                {
				    platformTex->alphaDistribution |= 0x2;
                }

			    uint32 newalpha = alpha * 0xff;

			    newalpha /= 0x80;

			    alpha = newalpha;
            }
            puttexelcolor(paletteTexelSource, i, this->rasterFormat, blue, green, red, alpha);
		}
	}

    // Copy over more advanced attributes.
    d3dtex->texels = platformTex->texels;
    d3dtex->dataSizes = platformTex->dataSizes;
    d3dtex->palette = platformTex->palette;
    d3dtex->paletteSize = platformTex->paletteSize;
    d3dtex->paletteType = platformTex->paletteType;

    // Set the backlink.
    d3dtex->parent = this;

    this->platformData = d3dtex;

	this->platform = PLATFORM_D3D8;

    // Delete the old platform texture.
    delete platformTex;
}

void NativeTexture::convertToPS2( void )
{
    if ( platform != PLATFORM_D3D8 )
        return;

    // Create the new PS2 platform texture.
    NativeTexturePS2 *ps2tex = new NativeTexturePS2();

    if ( !ps2tex )
        return;

    {
        NativeTextureD3D *platformTex = (NativeTextureD3D*)this->platformData;

        // Copy over general attributes.
        ps2tex->width = platformTex->width;
        ps2tex->height = platformTex->height;
        ps2tex->mipmapCount = platformTex->mipmapCount;
        ps2tex->mipmapDepth = platformTex->mipmapDepth;

        ps2tex->autoMipmaps = platformTex->autoMipmaps;

        // Copy over more complex data.
        ps2tex->dataSizes = platformTex->dataSizes;
        ps2tex->texels = platformTex->texels;
        ps2tex->palette = platformTex->palette;
        ps2tex->paletteSize = platformTex->paletteSize;
        ps2tex->paletteType = platformTex->paletteType;

        // Store the backlink.
        ps2tex->parent = this;

        this->platformData = ps2tex;

        this->platform = PLATFORM_PS2;

        // Delete the direct3D container.
        delete platformTex;
    }

    // Crypt the texel data.
    for ( uint32 n = 0; n < ps2tex->mipmapCount; n++ )
    {
        bool shouldSwizzle = ( true );  // todo: decide whether swizzling should be a thing.

        uint32 curWidth = ps2tex->width[n];
        uint32 curHeight = ps2tex->height[n];
        uint32 depth = ps2tex->mipmapDepth[n];

        // Initialize the swizzling state.
        ps2tex->isSwizzled.push_back( false );
        ps2tex->swizzleWidth.push_back( 0 );
        ps2tex->swizzleHeight.push_back( 0 );

        if ( depth == 4 )
        {
            if ( shouldSwizzle )
            {
                ps2tex->swizzleEncryptPS2(n);
            }
        }
        else if ( depth == 8 )
        {
            clut( (uint8*)ps2tex->texels[n], curWidth, curHeight );

            if ( shouldSwizzle )
            {
                ps2tex->swizzleEncryptPS2(n);
            }
        }
        else if ( depth == 32 )
        {
            typedef PixelFormat::texeltemplate <PixelFormat::pixeldata32bit> pixel32_t;

            pixel32_t *texelData = (pixel32_t*)ps2tex->texels[n];

			for (uint32 i = 0; i < curWidth * curHeight; i++)
            {
                uint8 red, green, blue, alpha;
                texelData->getcolor(i, red, green, blue, alpha);

			    // swap R and B
			    // fix alpha
                texelData->setcolor(i, blue, green, red, (uint32)alpha * 0x80 / 0xFF);
			}
        }
        else
        {
            assert( 0 );
        }
    }

	if (ps2tex->paletteType != PALETTE_NONE)
    {
        void *paletteTexelSource = ps2tex->palette;

		for (uint32 i = 0; i < ps2tex->paletteSize; i++)
        {
            uint8 red, green, blue, alpha;

            browsetexelcolor(paletteTexelSource, PALETTE_NONE, NULL, 0, i, this->rasterFormat, red, green, blue, alpha);
            {
			    uint32 newalpha = alpha * 0x80;

			    newalpha /= 0xff;

			    alpha = newalpha;
            }
            puttexelcolor(paletteTexelSource, i, this->rasterFormat, blue, green, red, alpha);
		}

        // Initialize palette unknowns.
        ps2MipmapTransmissionData unkData;
        unkData.destX = 0;
        unkData.destY = 0;

        ps2tex->palUnknowns = unkData;
	}
}

};