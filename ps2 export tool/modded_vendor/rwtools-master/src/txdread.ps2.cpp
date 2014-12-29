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

void verifyTexture( const NativeTexturePS2::GSTexture& gsTex, bool hasHeaders, eFormatEncodingType imageDecodeFormatType, ps2MipmapTransmissionData& transmissionOffset )
{
    // If the texture had the headers, then it should have come with the required registers.
    if ( hasHeaders )
    {
        // Debug register contents (only the important ones).
        uint32 regCount = gsTex.storedRegs.size();

        bool hasTRXPOS = false;
        bool hasTRXREG = false;
        bool hasTRXDIR = false;

        for ( uint32 n = 0; n < regCount; n++ )
        {
            const NativeTexturePS2::GSTexture::GSRegInfo& regInfo = gsTex.storedRegs[ n ];

            if ( regInfo.regID == GIF_REG_TRXPOS )
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
            else if ( regInfo.regID == GIF_REG_TRXREG )
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
            else if ( regInfo.regID == GIF_REG_TRXDIR )
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
}

uint32 NativeTexturePS2::GSTexture::readGIFPacket(std::istream& rw, bool hasHeaders, bool& corruptedHeaders_out)
{
    // See https://www.dropbox.com/s/onjaprt82y81sj7/EE_Users_Manual.pdf page 151

    uint32 readCount = 0;

	if (hasHeaders)
    {
        // A GSTexture always consists of a register list and the image data.
        struct invalid_gif_exception
        {
        };

        bool corruptedHeaders = false;

        uint32 streamOff_safe = rw.tellg();

        uint32 gif_readCount = 0;

        try
        {
            {
                GIFtag regListTag;
                rw.read((char*)&regListTag, sizeof(regListTag));

                assert(rw.gcount() == sizeof(regListTag));

                gif_readCount += sizeof(regListTag);

                // If we have a register list, parse it.
                if (regListTag.flg == 0)
                {
                    if (regListTag.eop != false ||
                        regListTag.pre != false ||
                        regListTag.prim != 0)
                    {
                        throw invalid_gif_exception();
                    }

                    // Only allow the register list descriptor.
                    if (regListTag.nreg != 1 ||
                        regListTag.getRegisterID(0) != 0xE)
                    {
                        throw invalid_gif_exception();
                    }

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

                        regInfo.regID = (eGSRegister)regID.regID;
                        regInfo.content = regContent;
                    }

                    gif_readCount += numRegs * ( sizeof(unsigned long long) * 2 );
                }
                else
                {
                    throw invalid_gif_exception();
                }
            }

            // Read the image data GIFtag.
            {
                GIFtag imgDataTag;
                rw.read((char*)&imgDataTag, sizeof(imgDataTag));

                assert(rw.gcount() == sizeof(imgDataTag));

                gif_readCount += sizeof(imgDataTag);

                // Verify that this is an image data tag.
                if (imgDataTag.eop != false ||
                    imgDataTag.pre != false ||
                    imgDataTag.prim != 0 ||
                    imgDataTag.flg != 2 ||
                    imgDataTag.nreg != 0)
                {
                    throw invalid_gif_exception();
                }

                // Verify the image data size.
                if (imgDataTag.nloop != (this->dataSize / (sizeof(uint64) * 2)))
                {
                    throw invalid_gif_exception();
                }
            }
        }
        catch( invalid_gif_exception& )
        {
            // We ignore the headers and try to read the image data.
            rw.seekg( streamOff_safe + 0x50, std::ios::beg );

            gif_readCount = 0x50;

            corruptedHeaders = true;
        }

        readCount += gif_readCount;

        corruptedHeaders_out = corruptedHeaders;
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

    // Store the raster type.
    platformTex->rasterType = ( textureMeta.rasterFormat & 0xFF );

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
    
    // If we are on the GTA III engine, we need to store the recommended buffer base pointer.
    if (header.version == rw::GTA3_1 || header.version == rw::GTA3_2 || header.version == rw::GTA3_3 || header.version == rw::GTA3_4)
    {
        platformTex->recommendedBufferBasePointer = textureMeta.tex0.textureBasePointer;
    }

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

    long remainingImageData = dataSize;
    
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

        NativeTexturePS2::GSMipmap& newMipmap = platformTex->mipmaps[i];

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
        newMipmap.dataSize = newMipmap.getDataSize();

        // Read the GIF packet data.
        bool hasCorruptedHeaders = false;

        uint32 readCount = newMipmap.readGIFPacket(rw, hasHeader, hasCorruptedHeaders);

        remainingImageData -= readCount;

        if ( !hasCorruptedHeaders )
        {
            // Verify this mipmap.
            verifyTexture( newMipmap, hasHeader, actualEncodingType, _origMipmapTransData[i] );
        }

	    i++;
	}

    // Assume we have at least one texture.
    assert( platformTex->mipmaps.size() > 0 );

    if ( remainingImageData > 0 )
    {
        // Make sure we are past the image data.
        rw.seekg( remainingImageData, std::ios::cur );
    }

	/* Palette */
	// vc dyn_trash.txd is weird here
    ps2MipmapTransmissionData palTransData;
    bool hasPalTransData = false;

    long remainingPaletteData = textureMeta.paletteDataSize;

	if (platformTex->paletteType != PALETTE_NONE)
    {
        // Craft the palette texture.
        NativeTexturePS2::GSTexture& palTex = platformTex->paletteTex;

        // The dimensions of this texture depend on game version.
        getPaletteTextureDimensions(platformTex->paletteType, header.version, palTex.swizzleWidth, palTex.swizzleHeight);

        // Decide about encoding type.
        // Only a limited amount of types are truly supported.
        palTex.swizzleEncodingType = getFormatEncodingFromRasterFormat(this->rasterFormat, PALETTE_NONE);

        assert(palTex.swizzleEncodingType == FORMAT_TEX32 || palTex.swizzleEncodingType == FORMAT_TEX16);

        // Calculate the texture data size.
        palTex.dataSize = palTex.getDataSize();

        // Read the GIF packet.
        bool hasCorruptedHeaders = false;

        uint32 readCount = palTex.readGIFPacket(rw, hasHeader, hasCorruptedHeaders);

        if ( !hasCorruptedHeaders )
        {
            // Also debug this texture.
            verifyTexture( palTex, hasHeader, palTex.swizzleEncodingType, palTransData );
        }

        remainingPaletteData -= readCount;

        if (hasHeader)
        {
            hasPalTransData = true;
        }
	}

    if ( remainingPaletteData > 0 )
    {
        // Make sure we are past the palette data.
        rw.seekg( remainingPaletteData, std::ios::cur );
    }

    // Verify that we are at the end of the texture native struct.
    {
        uint32 curStructOff = rw.tellg() - texStructPos;

        if ( curStructOff != texStructSize )
        {
            // Make sure we are at the end of the tex native struct.
            // vc "dyn_trash.txd" appears to be damaged.
            rw.seekg( texStructPos + texStructSize, std::ios::beg );
        }
    }

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
    if (platformTex->paletteType != PALETTE_NONE && hasPalTransData)
    {
        if ( clutTransData.destX != palTransData.destX ||
             clutTransData.destY != palTransData.destY )
        {
            __asm nop
        }
    }
}

/* convert from CLUT format used by the ps2 */
const static uint32 _clut_permute_psmct32[] =
{
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
    0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f
};

static bool clut(ePaletteType paletteType, NativeTexturePS2::GSTexture& clutTex)
{
    const uint32 *permuteData = NULL;
    uint32 permuteWidth, permuteHeight;
    uint32 itemDepth;

    if (paletteType == PALETTE_8BIT)
    {
        permuteData = _clut_permute_psmct32;

        permuteWidth = 16;
        permuteHeight = 2;

        itemDepth = getFormatEncodingDepth(clutTex.swizzleEncodingType);
    }

    if (permuteData != NULL)
    {
        // Create a new permutation destination.
        void *dstTexels = new uint8[ clutTex.dataSize ];
        void *srcTexels = clutTex.texels;

        uint32 clutWidth = clutTex.swizzleWidth;
        uint32 clutHeight = clutTex.swizzleHeight;

        uint32 alignedClutWidth = ALIGN_SIZE( clutWidth, permuteWidth );
        uint32 alignedClutHeight = ALIGN_SIZE( clutHeight, permuteHeight );

        uint32 colsWidth = ( alignedClutWidth / permuteWidth );
        uint32 colsHeight = ( alignedClutHeight / permuteHeight );

        // Perform the permutation.
        ps2GSPixelEncodingFormats::permuteArray(
            srcTexels, clutTex.swizzleWidth, clutTex.swizzleHeight, itemDepth, permuteWidth, permuteHeight,
            dstTexels, clutTex.swizzleWidth, clutTex.swizzleHeight, itemDepth, permuteWidth, permuteHeight,
            colsWidth, colsHeight,
            permuteData, permuteData, permuteWidth, permuteHeight, 1, 1,
            false
        );

        // Save the new texels.
        clutTex.texels = dstTexels;

        // Delete the old texels.
        delete [] srcTexels;
    }

    return true;
}

static bool unclut(ePaletteType paletteType, NativeTexturePS2::GSTexture& clutTex)
{
    // Since CLUT is a bijective algorithm, we can do that.
    return clut(paletteType, clutTex);
}

inline double clampcolor( double theColor )
{
    return std::min( 1.0, std::max( 0.0, theColor ) );
}

inline uint8 convertPCAlpha2PS2Alpha( uint8 pcAlpha )
{
    double pcAlphaDouble = clampcolor( (double)pcAlpha / 255.0 );

    double ps2AlphaDouble = pcAlphaDouble * 128.0;

    ps2AlphaDouble = floor( ps2AlphaDouble + 0.5 );

    uint8 ps2Alpha = (uint8)( ps2AlphaDouble );

    return ps2Alpha;
}

inline uint8 convertPS2Alpha2PCAlpha( uint8 ps2Alpha )
{
    double ps2AlphaDouble = clampcolor( (double)ps2Alpha / 128.0 );

    double pcAlphaDouble = ps2AlphaDouble * 255.0;

    pcAlphaDouble = floor( pcAlphaDouble + 0.495 );

    uint8 pcAlpha = (uint8)( pcAlphaDouble );

    return pcAlpha;
}

inline void getEffectivePaletteTextureDimensions(ePaletteType paletteType, uint32& palWidth_out, uint32& palHeight_out)
{
    uint32 palWidth = 0;
    uint32 palHeight = 0;

    if (paletteType == PALETTE_4BIT)
    {
        palWidth = 8;
        palHeight = 2;
    }
    else if (paletteType == PALETTE_8BIT)
    {
        palWidth = 16;
        palHeight = 16;
    }
    else
    {
        assert( 0 );
    }

    palWidth_out = palWidth;
    palHeight_out = palHeight;
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
    d3dtex->texels.resize( mipmapCount );

    d3dtex->mipmapCount = mipmapCount;

    d3dtex->autoMipmaps = platformTex->autoMipmaps;
    d3dtex->rasterType = platformTex->rasterType;

    // Copy mipmap data.
	for (uint32 j = 0; j < mipmapCount; j++)
    {
        NativeTexturePS2::GSMipmap& gsTex = platformTex->mipmaps[ j ];

        uint32 depth = gsTex.depth;

        // unswizzle the image data.
        {
		    bool opSuccess = platformTex->swizzleDecryptPS2(j);

            assert( opSuccess == true );
        }

        // Now that the texture is in linear format, we can prepare it.
		if (depth == 16)
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
                {
                    uint8 newAlpha = convertPS2Alpha2PCAlpha(alpha);

#ifdef DEBUG_ALPHA_LEVELS
                    assert(convertPCAlpha2PS2Alpha(newAlpha) == alpha);
#endif //DEBUG_ALPHA_LEVELS

                    alpha = newAlpha;
                }

			    // swap R and B
                texelData->setcolor(i, red, green, blue, alpha);
			}
		}

        // Move over the texture data to the D3D texture.
        d3dtex->width[ j ] = gsTex.width;
        d3dtex->height[ j ] = gsTex.height;
        d3dtex->mipmapDepth[ j ] = gsTex.depth;
        d3dtex->texels[ j ] = gsTex.texels;
        d3dtex->dataSizes[ j ] = gsTex.dataSize;

        // Zero out the GS texture.
        gsTex.texels = NULL;
        gsTex.dataSize = 0;
	}

    void *palTexels = NULL;
    uint32 palSize = 0;

	if (platformTex->paletteType != PALETTE_NONE)
    {
        // unCLUT the palette.
        bool unclutSuccess = unclut(platformTex->paletteType, platformTex->paletteTex);

        assert(unclutSuccess == true);

        // Prepare the palette colors.
        void *paletteTexelSource = platformTex->paletteTex.texels;
        {
            uint32 realSwizzleWidth, realSwizzleHeight;

            getEffectivePaletteTextureDimensions(platformTex->paletteType, realSwizzleWidth, realSwizzleHeight);
            
            uint32 palItems = ( realSwizzleWidth * realSwizzleHeight );

		    for (uint32 i = 0; i < palItems; i++)
            {
                uint8 red, green, blue, alpha;

                browsetexelcolor(paletteTexelSource, PALETTE_NONE, NULL, 0, i, this->rasterFormat, red, green, blue, alpha);
                {
	                uint8 newAlpha = convertPS2Alpha2PCAlpha(alpha);

#ifdef DEBUG_ALPHA_LEVELS
                    assert(convertPCAlpha2PS2Alpha(newAlpha) == alpha);
#endif //DEBUG_ALPHA_LEVELS

                    alpha = newAlpha;
                }
                puttexelcolor(paletteTexelSource, i, this->rasterFormat, red, green, blue, alpha);
		    }
        }

        // Give the palette texels to the new texture.
        palTexels = paletteTexelSource;
        palSize = ( platformTex->paletteTex.swizzleWidth * platformTex->paletteTex.swizzleHeight );

        // Unset palette texels.
        platformTex->paletteTex.texels = NULL;
        platformTex->paletteTex.dataSize = 0;
	}

    // Copy over more advanced attributes.
    d3dtex->palette = (uint8*)palTexels;
    d3dtex->paletteSize = palSize;
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
    if ( platform != PLATFORM_D3D8 && platform != PLATFORM_D3D9 )
        return;

    // Create the new PS2 platform texture.
    NativeTexturePS2 *ps2tex = new NativeTexturePS2();

    if ( !ps2tex )
        return;

    {
        NativeTextureD3D *platformTex = (NativeTextureD3D*)this->platformData;

        // Make sure the direct3D texture is decompressed.
        if ( platformTex->dxtCompression )
        {
            platformTex->decompressDxt();
        }

        // Prepare mipmap data.
        uint32 mipmapCount = platformTex->mipmapCount;

        ps2tex->mipmaps.resize( mipmapCount );

        for ( uint32 n = 0; n < mipmapCount; n++ )
        {
            // We create mipmap skeletons here.
            // The actual conversion to PS2 encoding happens later.
            NativeTexturePS2::GSMipmap& newMipmap = ps2tex->mipmaps[ n ];

            newMipmap.width = platformTex->width[ n ];
            newMipmap.height = platformTex->height[ n ];
            newMipmap.depth = platformTex->mipmapDepth[ n ];
            newMipmap.dataSize = platformTex->dataSizes[ n ];

            // We just move over the image data.
            newMipmap.texels = platformTex->texels[ n ];
        }

        // Copy over general attributes.
        ps2tex->autoMipmaps = platformTex->autoMipmaps;
        ps2tex->rasterType = platformTex->rasterType;

        // Move over the palette texels.
        if (platformTex->paletteType != PALETTE_NONE)
        {
            // Generate a palette texture.
            void *palTexelData = platformTex->palette;
            uint32 palSize = platformTex->paletteSize;

            void *newPalTexelData;
            uint32 newPalSize;

            uint32 palWidth, palHeight;

            genpalettetexeldata(
                rw::rwInterface.GetVersion(), palTexelData, this->rasterFormat, platformTex->paletteType, palSize,
                newPalTexelData, newPalSize, palWidth, palHeight
            );

            NativeTexturePS2::GSTexture& palTex = ps2tex->paletteTex;

            palTex.swizzleWidth = palWidth;
            palTex.swizzleHeight = palHeight;
            palTex.dataSize = newPalSize;
            palTex.swizzleEncodingType = getFormatEncodingFromRasterFormat(this->rasterFormat, PALETTE_NONE);

            palTex.texels = newPalTexelData;

            ps2tex->paletteType = platformTex->paletteType;

            // If we allocated a new palette array, we need to free the source one.
            if ( newPalTexelData != palTexelData )
            {
                delete [] palTexelData;
            }

            // Zero out the palette data at the source texture.
            platformTex->palette = NULL;
            platformTex->paletteSize = 0;
        }

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
        NativeTexturePS2::GSMipmap& gsTex = ps2tex->mipmaps[n];

        uint32 curWidth = gsTex.width;
        uint32 curHeight = gsTex.height;
        uint32 depth = gsTex.depth;

        // Prepare the pixels.
        if (depth == 16)
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
                {
                    uint8 newAlpha = convertPCAlpha2PS2Alpha(alpha);

#ifdef DEBUG_ALPHA_LEVELS
                    assert(convertPS2Alpha2PCAlpha(newAlpha) == alpha);
#endif //DEBUG_ALPHA_LEVELS

                    alpha = newAlpha;
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

        // We need to store dimensions into the texture of the current encoding.
        eFormatEncodingType requiredEncoding = ps2tex->getHardwareRequiredEncoding(rw::rwInterface.GetVersion());

        ps2GSPixelEncodingFormats::getPackedFormatDimensions(
            internalFormat, requiredEncoding,
            gsTex.width, gsTex.height,
            gsTex.swizzleWidth, gsTex.swizzleHeight
        );
        
        // Perform swizzling.
        {
            bool opSuccess = ps2tex->swizzleEncryptPS2(n);

            assert( opSuccess == true );
        }
    }

	if (ps2tex->paletteType != PALETTE_NONE)
    {
        // Prepare the palette texels.
        void *paletteTexelSource = ps2tex->paletteTex.texels;

        uint32 realSwizzleWidth, realSwizzleHeight;

        getEffectivePaletteTextureDimensions(ps2tex->paletteType, realSwizzleWidth, realSwizzleHeight);

        uint32 palItemCount = ( realSwizzleWidth * realSwizzleHeight );

		for (uint32 i = 0; i < palItemCount; i++)
        {
            uint8 red, green, blue, alpha;

            browsetexelcolor(paletteTexelSource, PALETTE_NONE, NULL, 0, i, this->rasterFormat, red, green, blue, alpha);
            {
	            uint8 newAlpha = convertPCAlpha2PS2Alpha(alpha);

#ifdef DEBUG_ALPHA_LEVELS
                if (convertPS2Alpha2PCAlpha(newAlpha) != alpha)
                {
                    __asm nop
                }
#endif //DEBUG_ALPHA_LEVELS

                alpha = newAlpha;
            }
            puttexelcolor(paletteTexelSource, i, this->rasterFormat, red, green, blue, alpha);
		}

        // Now CLUT the palette.
        clut(ps2tex->paletteType, ps2tex->paletteTex);
	}

    // Generate valid gsParams for this texture, as we lost our original ones.
    ps2tex->getOptimalGSParameters(ps2tex->gsParams);
}

};