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

#include "txdread.ps2gsman.hxx"

namespace rw
{

void verifyTexture( NativeTexturePS2::GSTexture& gsTex, eFormatEncodingType imageDecodeFormatType, ps2MipmapTransmissionData& transmissionOffset )
{
    // Debug register contents (only the important ones).
    uint32 regCount = gsTex.storedRegs.size();

    bool hasTRXPOS = false;
    bool hasTRXREG = false;
    bool hasTRXDIR = false;

    for ( uint32 n = 0; n < regCount; n++ )
    {
        const NativeTexturePS2::GSTexture::GSRegInfo& regInfo = gsTex.storedRegs[ n ];

        if ( regInfo.regID == 0x51 )
        {
            // TRXPOS
            const ps2GSRegisters::TRXPOS_REG& trxpos = (const ps2GSRegisters::TRXPOS_REG&)regInfo.content;

            assert(trxpos.ssax == 0);
            assert(trxpos.ssay == 0);
            assert(trxpos.dir == 0);

            // Give the transmission settings to the runtime.
            transmissionOffset.destX = trxpos.dsax;
            transmissionOffset.destY = trxpos.dsay;

            hasTRXPOS = true;
        }
        else if ( regInfo.regID == 0x52 )
        {
            // TRXREG
            const ps2GSRegisters::TRXREG_REG& trxreg = (const ps2GSRegisters::TRXREG_REG&)regInfo.content;

            // Convert this to swizzle width and height.
            uint32 storedSwizzleWidth = trxreg.transmissionAreaWidth;
            uint32 storedSwizzleHeight = trxreg.transmissionAreaHeight;

            if (gsTex.swizzleEncodingType == FORMAT_TEX32 && imageDecodeFormatType == FORMAT_IDTEX8_COMPRESSED)
            {
                storedSwizzleWidth /= 2;
            }

            assert(storedSwizzleWidth == gsTex.swizzleWidth);
            assert(storedSwizzleHeight == gsTex.swizzleHeight);

            hasTRXREG = true;
        }
        else if ( regInfo.regID == 0x53 )
        {
            // TRXDIR
            const ps2GSRegisters::TRXDIR_REG& trxdir = (const ps2GSRegisters::TRXDIR_REG&)regInfo.content;

            // Textures have to be transferred to the GS memory.
            assert(trxdir.xdir == 0);

            hasTRXDIR = true;
        }
    }

    // We kinda require all registers.
    assert(hasTRXPOS == true);
    assert(hasTRXREG == true);
    assert(hasTRXDIR == true);
}

uint32 NativeTexturePS2::GSTexture::readGIFPacket(std::istream& rw, bool hasHeaders)
{
    // See https://www.dropbox.com/s/onjaprt82y81sj7/EE_Users_Manual.pdf page 151

    uint32 readCount = 0;

	if (hasHeaders)
    {
        // A GSTexture always consists of a register list and the image data.
        {
            GIFtag regListTag;
            rw.read((char*)&regListTag, sizeof(regListTag));

            assert(rw.gcount() == sizeof(regListTag));

            readCount += sizeof(regListTag);

            // If we have a register list, parse it.
            if (regListTag.flg == 0)
            {
                assert(regListTag.eop == false);
                assert(regListTag.pre == false);
                assert(regListTag.prim == 0);
                assert(regListTag.nreg == 1);
                assert(regListTag.getRegisterID(0) == 0xE);

                uint32 numRegs = regListTag.nloop;

                // Preallocate the register space.
                this->storedRegs.resize( numRegs );

                for ( uint32 n = 0; n < numRegs; n++ )
                {
                    // Read the register content.
                    unsigned long long regContent;
                    rw.read((char*)&regContent, sizeof(regContent));

                    assert(rw.gcount() == sizeof(regContent));
                    
                    // Read the register ID.
                    struct regID_struct
                    {
                        unsigned long long regID : 8;
                        unsigned long long pad1 : 56;
                    };

                    regID_struct regID;
                    rw.read((char*)&regID, sizeof(regID_struct));

                    assert(rw.gcount() == sizeof(regID_struct));

                    // Put the register into the register storage.
                    GSRegInfo& regInfo = this->storedRegs[ n ];

                    regInfo.regID = regID.regID;
                    regInfo.content = regContent;
                }

                readCount += numRegs * ( sizeof(unsigned long long) * 2 );
            }
            else
            {
                assert(regListTag.flg == 3);
            }
        }

        // Read the image data GIFtag.
        {
            GIFtag imgDataTag;
            rw.read((char*)&imgDataTag, sizeof(imgDataTag));

            assert(rw.gcount() == sizeof(imgDataTag));

            readCount += sizeof(imgDataTag);

            // Verify that this is an image data tag.
            assert(imgDataTag.eop == false);
            assert(imgDataTag.pre == false);
            assert(imgDataTag.prim == 0);
            assert(imgDataTag.flg == 2);    // there has to be image data.
            assert(imgDataTag.nreg == 0);

            // Verify the image data size.
            assert(imgDataTag.nloop == (this->dataSize / (sizeof(uint64) * 2)));
        }
	}

    uint32 texDataSize = this->dataSize;

    uint8 *texelData = NULL;

    if ( texDataSize != 0 )
    {
        texelData = new uint8[texDataSize];

        rw.read(reinterpret_cast <char *> (texelData), texDataSize);

        assert(rw.gcount() == texDataSize);

        readCount += texDataSize;
    }

    this->texels = texelData;

    return readCount;
}

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

	uint32 depth = textureMeta.depth;

    // Deconstruct the rasterFormat.
    bool hasMipmaps = false;        // TODO: actually use this flag.

    readRasterFormatFlags( textureMeta.rasterFormat, this->rasterFormat, platformTex->paletteType, hasMipmaps, platformTex->autoMipmaps );

    platformTex->requiresHeaders = ( textureMeta.rasterFormat & 0x20000 ) != 0;
    platformTex->hasSwizzle = ( textureMeta.rasterFormat & 0x10000 ) != 0;

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
    bool hasSwizzle = platformTex->hasSwizzle;

	hasAlpha = false;

	if (maskName.size() != 0 || depth == 16)
    {
		hasAlpha = true;
    }

	READ_HEADER(CHUNK_STRUCT);

    // Decide about texture properties.
    eFormatEncodingType imageEncodingType = platformTex->getHardwareRequiredEncoding(header.version);

    // Get the format we should decode to.
    eFormatEncodingType actualEncodingType = getFormatEncodingFromRasterFormat(this->rasterFormat, platformTex->paletteType);
    
    assert(imageEncodingType != FORMAT_UNKNOWN);
    assert(actualEncodingType != FORMAT_UNKNOWN);

    // Absolute maximum of mipmaps.
    const size_t maxMipmaps = 7;

    // Reserve that much space for mipmaps.
    platformTex->mipmaps.reserve( maxMipmaps );

    ps2MipmapTransmissionData _origMipmapTransData[ maxMipmaps ];

	/* Pixels/Indices */
	long end = rw.tellg();
	end += (long)dataSize;
	uint32 i = 0;

    uint32 remainingImageData = dataSize;
    
    uint32 currentMipWidth = textureMeta.width;
    uint32 currentMipHeight = textureMeta.height;

	while (rw.tellg() < end)
    {
        if (i == maxMipmaps)
        {
            // We cannot have more than the maximum mipmaps.
            break;
        }

        if (i > 0 && !hasMipmaps)
        {
            break;
        }

	    // half dimensions if we have mipmaps
        if (i > 0)
        {
            currentMipWidth /= 2;
            currentMipHeight /= 2;
        }

        // Create a new mipmap.
        platformTex->mipmaps.resize( i + 1 );

        NativeTexturePS2::GSTexture& newMipmap = platformTex->mipmaps[i];

        newMipmap.width = currentMipWidth;
        newMipmap.height = currentMipHeight;

        // Calculate the encoded dimensions.
        {
            uint32 packedWidth, packedHeight;

            bool gotPackedDimms =
                ps2GSPixelEncodingFormats::getPackedFormatDimensions(
                    actualEncodingType, imageEncodingType,
                    newMipmap.width, newMipmap.height,
                    packedWidth, packedHeight
                );

            assert( gotPackedDimms == true );

		    newMipmap.swizzleWidth = packedWidth;
		    newMipmap.swizzleHeight = packedHeight;
        }

        // General properties.
        newMipmap.depth = depth;
        newMipmap.swizzleEncodingType = imageEncodingType;

        // Calculate the texture data size.
        {
            uint32 encodedTexItems = ( newMipmap.swizzleWidth * newMipmap.swizzleHeight );

            uint32 encodingDepth = getFormatEncodingDepth(imageEncodingType);

			newMipmap.dataSize = encodedTexItems * encodingDepth/8;
        }

        // Read the GIF packet data.
        uint32 readCount = newMipmap.readGIFPacket(rw, hasHeader);

        remainingImageData -= readCount;

        // Verify this mipmap.
        verifyTexture( newMipmap, actualEncodingType, _origMipmapTransData[i] );

	    i++;
	}

    // Assume we have at least one texture.
    assert( platformTex->mipmaps.size() > 0 );

    if ( remainingImageData != 0 )
    {
        // Make sure we are past the image data.
        rw.seekg( end, std::ios::beg );
    }

	/* Palette */
	// vc dyn_trash.txd is weird here
    ps2MipmapTransmissionData palTransData;

	if (platformTex->paletteType != PALETTE_NONE)
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

            palTransData.destX = gpuSize1;
            palTransData.destY = gpuSize2;

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

    uint32 clutBasePointer;
    uint32 clutMemSize;
    ps2MipmapTransmissionData clutTransData;

    eMemoryLayoutType decodedMemLayoutType;

    bool hasAllocatedMemory =
        platformTex->allocateTextureMemory(mipmapBasePointer, mipmapBufferWidth, mipmapMemorySize, mipmapTransData, maxMipmaps, decodedMemLayoutType, clutBasePointer, clutMemSize, clutTransData);

    // Could fail if no memory left.
    if ( !hasAllocatedMemory )
    {
        assert( 0 );
    }

    // Verify that our memory calculation routine is correct.
    uint32 gpuMinMemory = platformTex->calculateGPUDataSize(mipmapBasePointer, mipmapMemorySize, maxMipmaps, decodedMemLayoutType, clutBasePointer, clutMemSize);

    if ( textureMeta.combinedGPUDataSize > gpuMinMemory )
    {
        // If this assertion is triggered, then a adjust the gpu size calculation algorithm
        // so it outputs a big enough number.
        __asm nop
    }

    if ( textureMeta.combinedGPUDataSize != gpuMinMemory )
    {
        // It would be perfect if this condition were never triggered for official R* games textures.
        __asm nop
    }

    // Verify that our GPU data calculation routine is correct.
    ps2GSRegisters gpuData;
    
    bool isValidTexture = platformTex->generatePS2GPUData(header.version, gpuData, mipmapBasePointer, mipmapBufferWidth, mipmapMemorySize, maxMipmaps, decodedMemLayoutType, clutBasePointer);

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

    // Verify transmission rectangle same-ness.
    if (hasHeader)
    {
        bool hasValidTransmissionRects = true;

        uint32 mipmapCount = platformTex->mipmaps.size();

        for ( uint32 n = 0; n < mipmapCount; n++ )
        {
            const ps2MipmapTransmissionData& srcTransData = _origMipmapTransData[ n ];
            const ps2MipmapTransmissionData& dstTransData = mipmapTransData[ n ];

            if ( srcTransData.destX != dstTransData.destX ||
                 srcTransData.destY != dstTransData.destY )
            {
                hasValidTransmissionRects = false;
                break;
            }
        }

        if ( hasValidTransmissionRects == false )
        {
            __asm nop
        }
    }

    // Verify palette transmission rectangle.
    if (platformTex->paletteType != PALETTE_NONE)
    {
        if ( clutTransData.destX != palTransData.destX ||
             clutTransData.destY != palTransData.destY )
        {
            __asm nop
        }
    }

    // Just for some visual debugging.
    platformTex->palUnknowns = palTransData;
    platformTex->hasPalUnknowns = true;

    // Weird debugging shit.
    //platformTex->PerformDebugChecks(textureMeta);
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

void NativeTexture::convertFromPS2(void)
{
	if (platform != PLATFORM_PS2)
		return;

    // Create a new Direct3D platform texture.
    NativeTextureD3D *d3dtex = new NativeTextureD3D();

    if ( !d3dtex )
        return;

    NativeTexturePS2 *platformTex = (NativeTexturePS2*)this->platformData;

    // Copy over general attributes.
    uint32 mipmapCount = platformTex->mipmaps.size();

    d3dtex->width.resize( mipmapCount );
    d3dtex->height.resize( mipmapCount );
    d3dtex->mipmapDepth.resize( mipmapCount );
    d3dtex->dataSizes.resize( mipmapCount );

    d3dtex->mipmapCount = mipmapCount;

    d3dtex->autoMipmaps = platformTex->autoMipmaps;

    // Copy mipmap data.
	for (uint32 j = 0; j < mipmapCount; j++)
    {
        NativeTexturePS2::GSTexture& gsTex = platformTex->mipmaps[ j ];

        uint32 depth = gsTex.depth;

        // unswizzle the image data.
        {
		    bool opSuccess = platformTex->swizzleDecryptPS2(j);

            assert( opSuccess == true );
        }

        // Now that the texture is in linear format, we can prepare it.
		if (platformTex->paletteType == PALETTE_8BIT)
        {
            uint8 *texels = (uint8*)gsTex.texels;
			unclut(texels, gsTex.width, gsTex.height);
        }
        else if (depth == 16)
        {
            // TODO: do we have to do anything?
        }
        else if (depth == 32)
        {
            typedef PixelFormat::texeltemplate <PixelFormat::pixeldata32bit> pixel32_t;

            pixel32_t *texelData = (pixel32_t*)gsTex.texels;

			for (uint32 i = 0; i < gsTex.width * gsTex.height; i++)
            {
                uint8 red, green, blue, alpha;
                texelData->getcolor(i, red, green, blue, alpha);

			    // fix alpha
                if (this->convertAlpha)
                {
                    alpha = (uint32)alpha * 0xFF / 0x80;
                }

			    // swap R and B
                texelData->setcolor(i, red, green, blue, alpha);
			}
		}

        // Move over the texture data to the D3D texture.
        d3dtex->width[ j ] = gsTex.width;
        d3dtex->height[ j ] = gsTex.height;
        d3dtex->texels[ j ] = gsTex.texels;
        d3dtex->dataSizes[ j ] = gsTex.dataSize;

        // Zero out the GS texture.
        gsTex.texels = NULL;
        gsTex.dataSize = 0;
	}

	if (platformTex->paletteType != PALETTE_NONE)
    {
        void *paletteTexelSource = platformTex->palette;

		for (uint32 i = 0; i < platformTex->paletteSize; i++)
        {
            uint8 red, green, blue, alpha;

            if (this->convertAlpha)
            {
                browsetexelcolor(paletteTexelSource, PALETTE_NONE, NULL, 0, i, this->rasterFormat, red, green, blue, alpha);
                {
                    if (this->convertAlpha)
                    {
			            uint32 newalpha = alpha * 0xff;

			            newalpha /= 0x80;

			            alpha = newalpha;
                    }
                }
                puttexelcolor(paletteTexelSource, i, this->rasterFormat, red, green, blue, alpha);
            }
		}
	}

    // Copy over more advanced attributes.
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

        // Prepare mipmap data.
        uint32 mipmapCount = platformTex->mipmapCount;

        ps2tex->mipmaps.resize( mipmapCount );

        for ( uint32 n = 0; n < mipmapCount; n++ )
        {
            // We create mipmap skeletons here.
            // The actual conversion to PS2 encoding happens later.
            NativeTexturePS2::GSTexture newMipmap;

            newMipmap.width = platformTex->width[ n ];
            newMipmap.height = platformTex->height[ n ];
            newMipmap.depth = platformTex->mipmapDepth[ n ];
            newMipmap.dataSize = platformTex->dataSizes[ n ];

            // We just move over the image data.
            newMipmap.texels = platformTex->texels[ n ];
        }

        // Copy over general attributes.
        ps2tex->autoMipmaps = platformTex->autoMipmaps;

        // Copy over more complex data.
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

    // Encode the image data.
    uint32 mipmapCount = ps2tex->mipmaps.size();

    for ( uint32 n = 0; n < mipmapCount; n++ )
    {
        NativeTexturePS2::GSTexture& gsTex = ps2tex->mipmaps[n];

        uint32 curWidth = gsTex.width;
        uint32 curHeight = gsTex.height;
        uint32 depth = gsTex.depth;

        // Prepare the pixels.
        if (ps2tex->paletteType == PALETTE_8BIT)
        {
            clut( (uint8*)gsTex.texels, curWidth, curHeight );
        }
        else if (depth == 16)
        {
            // TODO: do we have to do anything?
        }
        else if (depth == 32)
        {
            typedef PixelFormat::texeltemplate <PixelFormat::pixeldata32bit> pixel32_t;

            pixel32_t *texelData = (pixel32_t*)gsTex.texels;

			for (uint32 i = 0; i < curWidth * curHeight; i++)
            {
                uint8 red, green, blue, alpha;
                texelData->getcolor(i, red, green, blue, alpha);

			    // fix alpha
                if (this->convertAlpha)
                {
                    alpha = (uint32)alpha * 0x80 / 0xFF;
                }

			    // swap R and B
                texelData->setcolor(i, red, green, blue, alpha);
			}
        }

        // Initialize the swizzling state.
        eFormatEncodingType internalFormat = 
            getFormatEncodingFromRasterFormat(this->rasterFormat, ps2tex->paletteType);

        gsTex.swizzleEncodingType = internalFormat;

        assert(internalFormat != FORMAT_UNKNOWN);

        // Perform swizzling.
        {
            bool opSuccess = ps2tex->swizzleEncryptPS2(n);

            assert( opSuccess == true );
        }
    }

	if (ps2tex->paletteType != PALETTE_NONE)
    {
        void *paletteTexelSource = ps2tex->palette;

		for (uint32 i = 0; i < ps2tex->paletteSize; i++)
        {
            uint8 red, green, blue, alpha;

            if (this->convertAlpha)
            {
                browsetexelcolor(paletteTexelSource, PALETTE_NONE, NULL, 0, i, this->rasterFormat, red, green, blue, alpha);
                {
                    if (this->convertAlpha)
                    {
			            uint32 newalpha = alpha * 0x80;

			            newalpha /= 0xff;

			            alpha = newalpha;
                    }
                }
                puttexelcolor(paletteTexelSource, i, this->rasterFormat, red, green, blue, alpha);
            }
		}
	}
}

};