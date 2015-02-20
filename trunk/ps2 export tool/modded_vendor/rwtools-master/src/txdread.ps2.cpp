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

                if (rw.gcount() != sizeof(regListTag))
                {
                    throw invalid_gif_exception();
                }

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

                        if (rw.gcount() != sizeof(regContent))
                        {
                            throw invalid_gif_exception();
                        }
                        
                        // Read the register ID.
                        struct regID_struct
                        {
                            unsigned long long regID : 8;
                            unsigned long long pad1 : 56;
                        };

                        regID_struct regID;
                        rw.read((char*)&regID, sizeof(regID_struct));

                        if (rw.gcount() != sizeof(regID_struct))
                        {
                            throw invalid_gif_exception();
                        }

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

                if (rw.gcount() != sizeof(imgDataTag))
                {
                    throw invalid_gif_exception();
                }

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

        if (rw.gcount() != texDataSize)
        {
            delete [] texelData;

            throw RwException( "corrupted texture image section" );
        }

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

	if (platform != PLATFORM_PS2FOURCC)
    {
        throw RwException( "invalid platform for PS2 texture reading" );
    }

    // Allocate a platform type texture.
    NativeTexturePS2 *platformTex = new NativeTexturePS2();

    if ( platformTex == NULL )
    {
        throw RwException( "failed to allocate memory for PS2 texture creation" );
    }

    // Store the backlink.
    platformTex->parent = this;

    // Give the texture link to our native data.
    this->platformData = platformTex;

    uint32 engineWarningLevel = rw::rwInterface.GetWarningLevel();

    // Attempt to read the texture.
    try
    {
        texFormatInfo formatInfo;
        rw.read((char*)&formatInfo, sizeof(formatInfo));

        this->filterFlags = formatInfo.filterMode;
        this->uAddressing = formatInfo.uAddressing;
        this->vAddressing = formatInfo.vAddressing;
    	
        // Read the name chunk section.
        {
	        READ_HEADER(CHUNK_STRING);
       
            uint32 strLen = header.getLength();

	        char *buffer = new char[strLen+1];
	        rw.read(buffer, strLen);

            buffer[strLen] = '\0';

	        this->name = buffer;

	        delete[] buffer;
        }

        // Read the mask name chunk section.
        {
	        READ_HEADER(CHUNK_STRING);

            uint32 strLen = header.getLength();

	        char *buffer = new char[strLen+1];
	        rw.read(buffer, strLen);

            buffer[strLen] = '\0';

	        this->maskName = buffer;

	        delete[] buffer;
        }

        // Texture Struct.
	    READ_HEADER(CHUNK_STRUCT);

        uint32 texStructSize = header.getLength();
        long texStructPos = rw.tellg();

	    READ_HEADER(CHUNK_STRUCT);

        if ( header.getLength() != sizeof( textureMetaDataHeader ) )
        {
            throw RwException( "invalid native header length" );
        }
        
        textureMetaDataHeader textureMeta;
        rw.read((char*)&textureMeta, sizeof(textureMetaDataHeader));

	    uint32 depth = textureMeta.depth;

        // Deconstruct the rasterFormat.
        bool hasMipmaps = false;        // TODO: actually use this flag.

        readRasterFormatFlags( textureMeta.rasterFormat, this->rasterFormat, platformTex->paletteType, hasMipmaps, platformTex->autoMipmaps );

        // Verify the raster format.
        eRasterFormat rasterFormat = this->rasterFormat;

        if ( !isValidRasterFormat( rasterFormat ) )
        {
            throw RwException( "invalid raster format in PS2 texture" );
        }

        // Verify the texture depth.
        {
            uint32 texDepth = 0;

            if (platformTex->paletteType == PALETTE_4BIT)
            {
                texDepth = 4;
            }
            else if (platformTex->paletteType == PALETTE_8BIT)
            {
                texDepth = 8;
            }
            else
            {
                texDepth = Bitmap::getRasterFormatDepth(rasterFormat);
            }

            if (texDepth != depth)
            {
                throw RwException( "texture " + this->name + " has an invalid depth" );
            }
        }

        platformTex->requiresHeaders = ( textureMeta.rasterFormat & 0x20000 ) != 0;
        platformTex->hasSwizzle = ( textureMeta.rasterFormat & 0x10000 ) != 0;

        // Store the raster type.
        platformTex->rasterType = ( textureMeta.rasterFormat & 0xFF );

        platformTex->depth = depth;

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
        LibraryVersion libVer = header.getVersion();

        if (libVer.rwLibMinor <= 3)
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

	    platformTex->hasAlpha = false;

	    if (maskName.size() != 0 || depth == 16)
        {
		    platformTex->hasAlpha = true;
        }

	    READ_HEADER(CHUNK_STRUCT);

        // Decide about texture properties.
        eFormatEncodingType imageEncodingType = platformTex->getHardwareRequiredEncoding(header.getVersion());

        // Get the format we should decode to.
        eFormatEncodingType actualEncodingType = getFormatEncodingFromRasterFormat(rasterFormat, platformTex->paletteType);
        
        if (imageEncodingType == FORMAT_UNKNOWN)
        {
            throw RwException( "unknown image decoding format" );
        }
        if (actualEncodingType == FORMAT_UNKNOWN)
        {
            throw RwException( "unknown image encoding format" );
        }

        // Absolute maximum of mipmaps.
        const size_t maxMipmaps = 7;

        // Reserve that much space for mipmaps.
        platformTex->mipmaps.reserve( maxMipmaps );

        ps2MipmapTransmissionData _origMipmapTransData[ maxMipmaps ];

        // TODO: are PS2 rasters always RGBA?
        // If not, adjust the color order parameter!

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

                if ( gotPackedDimms == false )
                {
                    throw RwException( "failed to get encoded dimensions for mipmap" );
                }

		        newMipmap.swizzleWidth = packedWidth;
		        newMipmap.swizzleHeight = packedHeight;
            }

            // General properties.
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
            else
            {
                rw::rwInterface.PushWarning( "texture " + this->name + " has corrupted image GIF packets" );
            }

	        i++;
	    }

        // Assume we have at least one texture.
        if ( platformTex->mipmaps.size() == 0 )
        {
            throw RwException( "empty texture" );
        }

        if ( remainingImageData > 0 )
        {
            rw::rwInterface.PushWarning( "texture " + this->name + " has image meta data" );

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
            getPaletteTextureDimensions(platformTex->paletteType, header.getVersion(), palTex.swizzleWidth, palTex.swizzleHeight);

            // Decide about encoding type.
            // Only a limited amount of types are truly supported.
            palTex.swizzleEncodingType = getFormatEncodingFromRasterFormat(rasterFormat, PALETTE_NONE);

            if (palTex.swizzleEncodingType != FORMAT_TEX32 && palTex.swizzleEncodingType != FORMAT_TEX16)
            {
                throw RwException( "invalid palette raster format" );
            }

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
            else
            {
                rw::rwInterface.PushWarning( "texture " + this->name + " has corrupted palette GIF packets" );
            }

            remainingPaletteData -= readCount;

            if (hasHeader)
            {
                hasPalTransData = true;
            }
	    }

        if ( remainingPaletteData > 0 )
        {
            rw::rwInterface.PushWarning( "texture " + this->name + " has palette meta data" );

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
            throw RwException( "failed to allocate texture memory" );
        }

        // Verify that our memory calculation routine is correct.
        uint32 gpuMinMemory = platformTex->calculateGPUDataSize(mipmapBasePointer, mipmapMemorySize, maxMipmaps, decodedMemLayoutType, clutBasePointer, clutMemSize);

        if ( textureMeta.combinedGPUDataSize > gpuMinMemory )
        {
            // If this assertion is triggered, then adjust the gpu size calculation algorithm
            // so it outputs a big enough number.
            rw::rwInterface.PushWarning( "too small GPU data size for texture " + this->name );

            // TODO: handle this as error?
        }
        else if ( textureMeta.combinedGPUDataSize != gpuMinMemory )
        {
            // It would be perfect if this condition were never triggered for official R* games textures.
            rw::rwInterface.PushWarning( "invalid GPU data size for texture " + this->name );
        }

        // Verify that our GPU data calculation routine is correct.
        ps2GSRegisters gpuData;
        
        bool isValidTexture = platformTex->generatePS2GPUData(header.getVersion(), gpuData, mipmapBasePointer, mipmapBufferWidth, mipmapMemorySize, maxMipmaps, decodedMemLayoutType, clutBasePointer);

        // If any of those assertions fail then either our is routine incomplete
        // or the input texture is invalid (created by wrong tool probably.)
        if ( !isValidTexture )
        {
            throw RwException( "invalid texture format" );
        }

        if ( gpuData.tex0 != textureMeta.tex0 )
        {
            if ( engineWarningLevel >= 3 )
            {
                rw::rwInterface.PushWarning( "texture " + this->name + " has invalid TEX0 register" );
            }
        }
        if ( gpuData.tex1 != textureMeta.tex1 )
        {
            if ( engineWarningLevel >= 2 )
            {
                rw::rwInterface.PushWarning( "texture " + this->name + " has invalid TEX1 register" );
            }
        }
        if ( gpuData.miptbp1 != textureMeta.miptbp1 )
        {
            if ( engineWarningLevel >= 1 )
            {
                rw::rwInterface.PushWarning( "texture " + this->name + " has invalid MIPTBP1 register" );
            }
        }
        if ( gpuData.miptbp2 != textureMeta.miptbp2 )
        {
            if ( engineWarningLevel >= 1 )
            {
                rw::rwInterface.PushWarning( "texture " + this->name + " has invalid MIPTBP2 register" );
            }
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
                rw::rwInterface.PushWarning( "texture " + this->name + " has invalid mipmap transmission offsets" );
            }
        }

        // Verify palette transmission rectangle.
        if (platformTex->paletteType != PALETTE_NONE && hasPalTransData)
        {
            if ( clutTransData.destX != palTransData.destX ||
                 clutTransData.destY != palTransData.destY )
            {
                rw::rwInterface.PushWarning( "texture " + this->name + " has invalid CLUT transmission offset" );
            }
        }
    }
    catch( ... )
    {
        // Delete the raster.
        platformTex->Delete();

        this->platformData = NULL;

        throw;
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

inline void convertTexelsFromPS2(
    void *texelSource, uint32 texelCount, eRasterFormat rasterFormat, uint32 itemDepth,
    eColorOrdering ps2ColorOrder, eColorOrdering d3dColorOrder, bool fixAlpha
)
{
    if ( fixAlpha || ps2ColorOrder != d3dColorOrder )
    {
	    for (uint32 i = 0; i < texelCount; i++)
        {
            uint8 red, green, blue, alpha;
            browsetexelcolor(texelSource, PALETTE_NONE, NULL, 0, i, rasterFormat, ps2ColorOrder, itemDepth, red, green, blue, alpha);

	        // fix alpha
            if (fixAlpha)
            {
                uint8 newAlpha = convertPS2Alpha2PCAlpha(alpha);

#ifdef DEBUG_ALPHA_LEVELS
                assert(convertPCAlpha2PS2Alpha(newAlpha) == alpha);
#endif //DEBUG_ALPHA_LEVELS

                alpha = newAlpha;
            }

            puttexelcolor(texelSource, i, rasterFormat, d3dColorOrder, itemDepth, red, green, blue, alpha);
	    }
    }
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

    uint32 mipmapCount = platformTex->mipmaps.size();

    eRasterFormat rasterFormat = this->rasterFormat;
    ePaletteType paletteType = platformTex->paletteType;

    try
    {
        // Do unsafe preparations.
        // These operations can be performed by itself, but since they may fail, they should not be connected
        // with code that must not fail.

        // Unswizzle all mipmaps.
        for ( uint32 n = 0; n < mipmapCount; n++ )
        {
            // unswizzle the image data.
            {
	            bool opSuccess = platformTex->swizzleDecryptPS2(n);

                if ( opSuccess == false )
                {
                    throw RwException( "failed to unswizzle texture" );
                }
            }
        }

        // Unswizzle CLUT, if present.
        if ( paletteType != PALETTE_NONE )
        {
            // unCLUT the palette.
            bool unclutSuccess = unclut(paletteType, platformTex->paletteTex);

            if ( unclutSuccess == false )
            {
                throw RwException( "failed to unswizzle the CLUT" );
            }
        }
    }
    catch( RwException& )
    {
        // Delete the new texture again.
        delete d3dtex;

        // We failed.
        throw;
    }

    // Copy over general attributes.
    d3dtex->width.resize( mipmapCount );
    d3dtex->height.resize( mipmapCount );
    d3dtex->dataSizes.resize( mipmapCount );
    d3dtex->texels.resize( mipmapCount );

    d3dtex->mipmapCount = mipmapCount;

    uint32 depth = platformTex->depth;

    d3dtex->depth = depth;

    // Fix wrong auto mipmap property.
    bool hasAutoMipmaps = false;

    if ( mipmapCount == 1 )
    {
        // Direct3D textures can only have mipmaps if they dont come with mipmaps.
        hasAutoMipmaps = platformTex->autoMipmaps;
    }

    d3dtex->autoMipmaps = hasAutoMipmaps;
    d3dtex->rasterType = platformTex->rasterType;

    d3dtex->hasAlpha = platformTex->hasAlpha;

    // Copy mipmap data.
    eColorOrdering ps2ColorOrder = platformTex->colorOrdering;
    eColorOrdering d3dColorOrder = d3dtex->colorOrdering;

    for (uint32 j = 0; j < mipmapCount; j++)
    {
        NativeTexturePS2::GSMipmap& gsTex = platformTex->mipmaps[ j ];

        // Now that the texture is in linear format, we can prepare it.
        bool fixAlpha = false;

        // TODO: do we have to fix alpha for 16bit raster depths?

	    if (rasterFormat == RASTER_8888)
        {
            fixAlpha = true;
	    }

        // Prepare colors.
        uint32 mipWidth = gsTex.width;
        uint32 mipHeight = gsTex.height;

        void *texelData = gsTex.texels;

        if (paletteType == PALETTE_NONE)
        {
            convertTexelsFromPS2(texelData, mipWidth * mipHeight, rasterFormat, depth, ps2ColorOrder, d3dColorOrder, fixAlpha);
        }

        // Move over the texture data to the D3D texture.
        d3dtex->width[ j ] = mipWidth;
        d3dtex->height[ j ] = mipHeight;
        d3dtex->texels[ j ] = texelData;
        d3dtex->dataSizes[ j ] = gsTex.dataSize;

        // Unset mipmap texels.
        gsTex.texels = NULL;
        gsTex.dataSize = 0;
    }

    void *palTexels = NULL;
    uint32 palSize = 0;

    if (paletteType != PALETTE_NONE)
    {
        // Prepare the palette colors.
        void *paletteTexelSource = platformTex->paletteTex.texels;
        {
            uint32 realSwizzleWidth, realSwizzleHeight;

            getEffectivePaletteTextureDimensions(paletteType, realSwizzleWidth, realSwizzleHeight);

            uint32 palFormatDepth = Bitmap::getRasterFormatDepth(rasterFormat);
            
            convertTexelsFromPS2(paletteTexelSource, realSwizzleWidth * realSwizzleHeight, rasterFormat, palFormatDepth, ps2ColorOrder, d3dColorOrder, true);
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
    d3dtex->paletteType = paletteType;

    // Set the backlink.
    d3dtex->parent = this;

    this->platformData = d3dtex;

	this->platform = PLATFORM_D3D8;

    // Delete the old platform texture.
    delete platformTex;

    // Execute managed methods now...

    // Update the D3DFORMAT field.
    d3dtex->updateD3DFormat();

    // Actually, since there is no alpha flag in PS2 textures, we should recalculate the alpha flag here.
    d3dtex->hasAlpha = d3dtex->doesHaveAlpha();
}

inline void convertTexelsToPS2(
    const void *srcTexelData, void *dstTexelData, uint32 texelCount, eRasterFormat srcRasterFormat, eRasterFormat dstRasterFormat, uint32 srcItemDepth, uint32 dstItemDepth,
    eColorOrdering d3dColorOrder, eColorOrdering ps2ColorOrder,
    bool fixAlpha
)
{
    if ( fixAlpha || d3dColorOrder != ps2ColorOrder || srcRasterFormat != dstRasterFormat || srcItemDepth != dstItemDepth )
    {
		for (uint32 i = 0; i < texelCount; i++)
        {
            uint8 red, green, blue, alpha;
            browsetexelcolor(srcTexelData, PALETTE_NONE, NULL, 0, i, srcRasterFormat, d3dColorOrder, srcItemDepth, red, green, blue, alpha);

		    // fix alpha
            {
                uint8 newAlpha = convertPCAlpha2PS2Alpha(alpha);

#ifdef DEBUG_ALPHA_LEVELS
                assert(convertPS2Alpha2PCAlpha(newAlpha) == alpha);
#endif //DEBUG_ALPHA_LEVELS

                alpha = newAlpha;
            }

            puttexelcolor(dstTexelData, i, dstRasterFormat, ps2ColorOrder, dstItemDepth, red, green, blue, alpha);
		}
    }
}

void NativeTexture::convertToPS2( void )
{
    if ( platform != PLATFORM_D3D8 && platform != PLATFORM_D3D9 )
        return;

    // Create the new PS2 platform texture.
    NativeTexturePS2 *ps2tex = new NativeTexturePS2();

    if ( !ps2tex )
        return;

    // The maximum of mipmaps supported by PS2 textures.
    const uint32 maxMipmaps = 7;

    {
        NativeTextureD3D *platformTex = (NativeTextureD3D*)this->platformData;

        // Make sure the direct3D texture is decompressed.
        if ( platformTex->dxtCompression )
        {
            platformTex->decompressDxt();
        }

        // The PlayStation 2 does NOT support all raster formats.
        // We need to avoid giving it raster formats that are prone to crashes, like RASTER_888.
        eRasterFormat srcRasterFormat = this->rasterFormat;
        uint32 srcItemDepth = platformTex->depth;

        eRasterFormat targetRasterFormat = srcRasterFormat;
        uint32 dstItemDepth = srcItemDepth;

        ePaletteType paletteType = platformTex->paletteType;

        // Only fix if the user wants us to.
        if (rw::rwInterface.GetFixIncompatibleRasters())
        {
            if (targetRasterFormat == RASTER_888)
            {
                // Since this raster takes the same memory space as RASTER_8888, we can silently convert it.
                targetRasterFormat = RASTER_8888;
            }

            if (paletteType != PALETTE_NONE)
            {
                // The architecture does not support 8bit PALETTE_4BIT rasters.
                // Fix that.
                if (paletteType == PALETTE_4BIT)
                {
                    dstItemDepth = 4;
                }
                else if (paletteType == PALETTE_8BIT)
                {
                    dstItemDepth = 8;
                }
            }
        }

        uint32 targetRasterDepth = Bitmap::getRasterFormatDepth(targetRasterFormat);

        if (paletteType == PALETTE_NONE)
        {
            dstItemDepth = targetRasterDepth;
        }

        // Prepare mipmap data.
        eColorOrdering d3dColorOrder = platformTex->colorOrdering;
        eColorOrdering ps2ColorOrder = ps2tex->colorOrdering;

        uint32 mipmapCount = platformTex->mipmapCount;
        {
            uint32 mipProcessCount = std::min( maxMipmaps, mipmapCount );

            ps2tex->mipmaps.resize( mipProcessCount );

            for ( uint32 n = 0; n < mipProcessCount; n++ )
            {
                // We create mipmap skeletons here.
                // The actual conversion to PS2 encoding happens later.
                NativeTexturePS2::GSMipmap& newMipmap = ps2tex->mipmaps[ n ];

                uint32 mipWidth = platformTex->width[ n ];
                uint32 mipHeight = platformTex->height[ n ];
                uint32 srcDataSize = platformTex->dataSizes[ n ];

                void *srcTexelData = platformTex->texels[ n ];

                // We need to convert the texels before storing them in the PS2 texture.
                bool fixAlpha = false;

                // TODO: do we have to fix alpha for 16bit rasters?

                if (targetRasterFormat == RASTER_8888)
                {
                    fixAlpha = true;
                }

                // Determine whether we need to allocate new texel data.
                void *dstTexelData = srcTexelData;
                uint32 dstDataSize = srcDataSize;

                // Convert the texels.
                if (paletteType == PALETTE_NONE)
                {
                    uint32 itemCount = mipWidth * mipHeight;

                    if (srcItemDepth != dstItemDepth)
                    {
                        dstDataSize = getRasterDataSize(itemCount, dstItemDepth);

                        dstTexelData = new uint8[ dstDataSize ];
                    }

                    convertTexelsToPS2(
                        srcTexelData, dstTexelData, itemCount,
                        srcRasterFormat, targetRasterFormat,
                        srcItemDepth, dstItemDepth,
                        d3dColorOrder, ps2ColorOrder,
                        fixAlpha
                    );
                }
                else
                {
                    // Maybe we need to fix the indice (if the texture comes from PC or XBOX architecture).
                    if (dstItemDepth != srcItemDepth)
                    {
                        uint32 itemCount = mipWidth * mipHeight;

                        uint32 newDataSize = 0;

                        if (dstItemDepth == 4)
                        {
                            newDataSize = PixelFormat::palette4bit::sizeitems( itemCount );
                        }
                        else if (dstItemDepth == 8)
                        {
                            newDataSize = PixelFormat::palette8bit::sizeitems( itemCount );
                        }

                        if (newDataSize != 0)
                        {
                            dstTexelData = new uint8[ newDataSize ];
                            
                            dstDataSize = newDataSize;

                            // Update the indice.
                            uint32 palSize = platformTex->paletteSize;

                            for ( uint32 n = 0; n < itemCount; n++ )
                            {
                                uint8 palIndex;

                                bool gotPalIndex = getpaletteindex(srcTexelData, paletteType, palSize, srcItemDepth, n, palIndex);

                                if ( !gotPalIndex )
                                {
                                    palIndex = 0;
                                }

                                // Write it again.
                                if (dstItemDepth == 4)
                                {
                                    ( (PixelFormat::palette4bit*)dstTexelData )->setvalue(n, palIndex);
                                }
                                else if (dstItemDepth == 8)
                                {
                                    ( (PixelFormat::palette8bit*)dstTexelData )->setvalue(n, palIndex);
                                }
                                else
                                {
                                    assert( 0 );
                                }
                            }
                        }
                    }
                }

                if (srcTexelData != dstTexelData)
                {
                    delete [] srcTexelData;
                }

                newMipmap.width = mipWidth;
                newMipmap.height = mipHeight;
                newMipmap.dataSize = dstDataSize;

                newMipmap.texels = dstTexelData;
            }

            // For all the texels that we dont require, delete them.
            for ( uint32 n = maxMipmaps; n < mipmapCount; n++ )
            {
                delete platformTex->texels[ n ];
            }
        }

        // Copy over general attributes.
        ps2tex->depth = dstItemDepth;

        // Make sure we apply autoMipmap property just like the R* converter.
        bool hasAutoMipmaps = platformTex->autoMipmaps;

        if ( mipmapCount > 1 )
        {
            hasAutoMipmaps = true;
        }

        ps2tex->autoMipmaps = hasAutoMipmaps;
        ps2tex->rasterType = platformTex->rasterType;

        ps2tex->hasAlpha = platformTex->hasAlpha;

        // Move over the palette texels.
        if (paletteType != PALETTE_NONE)
        {
            // Prepare the palette texels.
            void *srcPalTexelData = platformTex->palette;
            uint32 palSize = platformTex->paletteSize;

            uint32 srcPalFormatDepth = Bitmap::getRasterFormatDepth( srcRasterFormat );
            uint32 targetPalFormatDepth = targetRasterDepth;

            void *dstPalTexelData = srcPalTexelData;

            if (targetPalFormatDepth != srcPalFormatDepth)
            {
                uint32 palDataSize = getRasterDataSize(palSize, targetPalFormatDepth);

                dstPalTexelData = new uint32[ palDataSize ];
            }

            convertTexelsToPS2(
                srcPalTexelData, dstPalTexelData, palSize,
                srcRasterFormat, targetRasterFormat,
                srcPalFormatDepth, targetPalFormatDepth,
                d3dColorOrder, ps2ColorOrder,
                true
            );

            if (srcPalTexelData != dstPalTexelData)
            {
                delete [] srcPalTexelData;
            }

            // Generate a palette texture.
            void *newPalTexelData;
            uint32 newPalSize;

            uint32 palWidth, palHeight;

            genpalettetexeldata(
                rw::rwInterface.GetVersion(), dstPalTexelData, targetRasterFormat, paletteType, palSize,
                newPalTexelData, newPalSize, palWidth, palHeight
            );

            NativeTexturePS2::GSTexture& palTex = ps2tex->paletteTex;

            palTex.swizzleWidth = palWidth;
            palTex.swizzleHeight = palHeight;
            palTex.dataSize = newPalSize;
            palTex.swizzleEncodingType = getFormatEncodingFromRasterFormat(targetRasterFormat, PALETTE_NONE);

            palTex.texels = newPalTexelData;

            // If we allocated a new palette array, we need to free the source one.
            if ( newPalTexelData != dstPalTexelData )
            {
                delete [] dstPalTexelData;
            }

            // Zero out the palette data at the source texture.
            platformTex->palette = NULL;
            platformTex->paletteSize = 0;
        }

        // Set the palette type.
        ps2tex->paletteType = paletteType;

        // Finally, update the raster format.
        if (srcRasterFormat != targetRasterFormat)
        {
            this->rasterFormat = targetRasterFormat;
        }

        // Store the backlink.
        ps2tex->parent = this;

        this->platformData = ps2tex;

        this->platform = PLATFORM_PS2;

        // Delete the direct3D container.
        delete platformTex;
    }

    // Any failing operation from now on does not have to result in an exception.
    // In case we do, though, we need to figure out how to properly handle the error...
    // For now, we just ignore it (TODO).
    try
    {
        // Encode the image data.
        uint32 mipmapCount = ps2tex->mipmaps.size();

        for ( uint32 n = 0; n < mipmapCount; n++ )
        {
            NativeTexturePS2::GSMipmap& gsTex = ps2tex->mipmaps[n];

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

                if ( opSuccess == false )
                {
                    // The probability of this failing is medium.
                    throw RwException( "failed to swizzle texture" );
                }
            }
        }

	    if (ps2tex->paletteType != PALETTE_NONE)
        {
            // Now CLUT the palette.
            bool clutSuccess = clut(ps2tex->paletteType, ps2tex->paletteTex);

            if ( clutSuccess == false )
            {
                // The probability of this failing is slim like a straw of hair.
                throw RwException( "failed to swizzle the CLUT" );
            }
	    }
    }
    catch( RwException& )
    {
        // TODO: properly handle the exception.

        // Pass the exception on.
        throw;
    }

    // Generate valid gsParams for this texture, as we lost our original ones.
    ps2tex->getOptimalGSParameters(ps2tex->gsParams);
}

};