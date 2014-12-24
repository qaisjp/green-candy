#include <StdInc.h>
#include <cstring>
#include <assert.h>
#include <math.h>

#include "streamutil.hxx"

#include "txdread.ps2.hxx"

namespace rw
{

inline void genpalettetexeldata(const void *paletteData, eRasterFormat rasterFormat, ePaletteType paletteType, uint32 itemCount, void*& texelData, uint32& texelDataSize, uint32& palWidth, uint32& palHeight)
{
    // Select a compatible texture size for our palette data.
    uint32 texelWidth = 0;
    uint32 texelHeight = 0;

    if ( paletteType == PALETTE_4BIT )
    {
        texelWidth = 8;
        texelHeight = 3;
    }
    else if ( paletteType == PALETTE_8BIT )
    {
        texelWidth = 16;
        texelHeight = 16;
    }
    else
    {
        assert( 0 );
    }

    // Allocate texture memory.
    uint32 texelItemCount = ( texelWidth * texelHeight );

    size_t srcDataSize = ( itemCount * sizeof(PixelFormat::pixeldata32bit) );
    size_t dstDataSize = ( texelItemCount * sizeof(PixelFormat::pixeldata32bit) );

    void *newTexelData = new uint8[ dstDataSize ];

    // Write the new memory.
    memcpy(newTexelData, paletteData, srcDataSize);
    
    // Zero out the rest.
    memset((char*)newTexelData + srcDataSize, 0, (dstDataSize - srcDataSize));

    // Give parameters to the runtime.
    texelData = newTexelData;
    texelDataSize = dstDataSize;
    
    palWidth = texelWidth;
    palHeight = texelHeight;
}

inline size_t writeStringSection( std::ostream& rw, const char *string, size_t strLen )
{
    HeaderInfo header;
    header.version = rw::rwInterface.GetVersion();

    uint32 writtenBytesReturn = 0;

    SKIP_HEADER();

    rw.write(string, strLen);
    bytesWritten += strLen;

    // Pad to multiples of four.
    // This will automatically zero-terminate the string.
    size_t remainder = 4 - (strLen % 4);

    // Write zeroes.
    for ( size_t n = 0; n < remainder; n++ )
    {
        unsigned char zeroTerminator = 0;

        rw.write((char*)&zeroTerminator, sizeof(zeroTerminator));
    }
    bytesWritten += remainder;

    WRITE_HEADER(CHUNK_STRING);

    return writtenBytesReturn;
}

template <typename numType>
inline bool IsNumberInRange( numType val, numType lower, numType upper )
{
    return ( lower <= val ) && ( val <= upper );
}

template <typename numType>
inline bool BijectiveInclusion( numType val1, numType val2, numType inclusionReq, numType lower, numType upper )
{
    return
        val1 == inclusionReq && IsNumberInRange( val2, lower, upper ) ||
        val2 == inclusionReq && IsNumberInRange( val1, lower, upper );
}

void NativeTexturePS2::getOptimalGSParameters(gsParams_t& paramsOut) const
{
    // Calculate according to texture properties.
    uint32 width = this->width[0];
    uint32 height = this->height[0];
    uint32 depth = this->mipmapDepth[0];

    bool hasMipmaps = this->autoMipmaps;

    gsParams_t params;

    // Default the parameters.
    params.maxMIPLevel = 0;
    params.mtba = 0;
    params.textureFunction = 0;         // MODULATE
    params.lodCalculationModel = 0;     // LOD using formula
    params.mmag = 0;                    // NEAREST
    params.mmin = 0;                    // NEAREST
    params.lodParamL = 0;
    params.lodParamK = 0;

    if ( depth == 4 || depth == 8 )
    {
        if ( !hasMipmaps )
        {
            // TODO: equivalent in auto-mipmap version.
            if ( depth == 4 && (
                    width < 0x80 || height < 0x80
                 ) && (
                    width + height >= 0x10 ||
                    width == height
                 ) )
            {
                params.maxMIPLevel = 7;
            }

            if ( depth == 8 && (
                 BijectiveInclusion( width, height, 0x10u, 0x10u, 0x80u ) ||
                 BijectiveInclusion( width, height, 0x20u, 0x10u, 0x80u ) ||
                 BijectiveInclusion( width, height, 0x40u, 0x10u, 0x40u ) ||
                 BijectiveInclusion( width, height, 0x80u, 0x10u, 0x20u ) ||
                 width == 0x40 && height == 0x80 ||
                 width == 0x40 && height == 0x100 ||
                 width == 0x100 && height == 0x20 ||
                 width == 0x08 && height == 0x08 ||
                 width == 0x10 && height == 0x04 ) )
            {
                params.maxMIPLevel = 7;
            }

            if ( depth == 4 && (
                 IsNumberInRange( width, 0x10u, 0x40u ) && IsNumberInRange( height, 0x100u, 0x200u ) ||
                 IsNumberInRange( width, 0x100u, 0x200u ) && IsNumberInRange( height, 0x10u, 0x40u ) ||
                 width == 0x80 && height == 0x80
                 ) )
            {
                params.mmag = 1;      // LINEAR
            }

            if ( depth == 8 && (
                 width == 0x80 && height == 0x40 ||
                 width == 0x40 && height == 0x80 ||
                 width == 0x100 && height == 0x20 ||
                 width == 0x10 && height == 0x80 ||
                 width == 0x20 && height == 0x80 ||
                 width == 0x40 && height == 0x100
                 ) )
            {
                params.mmag = 1;      // LINEAR
            }

            uint32 mminMode = 0;    // NEAREST

            if ( depth == 4 && (
                 BijectiveInclusion( width, height, 0x100u, 0x80u, 0x80u ) ||
                 BijectiveInclusion( width, height, 0x200u, 0x20u, 0x40u ) ) )
            {
                mminMode = 1;   // LINEAR
            }

            if ( depth == 8 && (
                 width == 0x80 && height == 0x80 ||
                 BijectiveInclusion( width, height, 0x40u, 0x100u, 0x100u ) ) )
            {
                mminMode = 1;   // NEAREST
            }

            if ( depth == 4 && (
                 width == 0x100 && height == 0x100 ) )
            {
                mminMode |= 0x02;   // NEAREST_MIPMAP_x
            }

            if ( depth == 8 && (
                 BijectiveInclusion( width, height, 0x80u, 0x100u, 0x100u ) ) )
            {
                mminMode |= 0x02;   // NEAREST_MIPMAP_x
            }

            if ( depth == 4 && (
                 width == 0x200 && height == 0x100 ) )
            {
                mminMode |= 0x04;   // LINEAR_MIPMAP_x
            }

            if ( depth == 8 && (
                 width == 0x100 && height == 0x100 ) )
            {
                mminMode |= 0x04;   // LINEAR_MIPMAP_x
            }

            params.mmin = mminMode;

            if ( depth == 4 && (
                 width == 0x200 && height == 0x200 ) )
            {
                params.mtba = 1;  // automatically generate mipmap buffer offsets and sizes.
            }

            //

            // 

            if ( depth == 8 && (
                 width == 0x200 && height == 0x200 ) )
            {
                params.gsTEX1Unknown1 = 1;
            }
        }
        else
        {
            if ( depth == 4 )
            {
                params.lodCalculationModel = 1;
                params.gsTEX1Unknown2 = 1;
            }

            if ( depth == 4 && (
                 width == 0x10 && height == 0x10 ||
                 width == 0x40 && height == 0x08 ||
                 BijectiveInclusion( width, height, 0x20u, 0x20u, 0x80u ) ||
                 BijectiveInclusion( width, height, 0x40u, 0x20u, 0x100u ) ||
                 BijectiveInclusion( width, height, 0x80u, 0x20u, 0x100u ) ||
                 BijectiveInclusion( width, height, 0x100u, 0x40u, 0x100u ) ) )
            {
                params.maxMIPLevel = 7;
            }

            if ( depth == 8 && (
                 width == 0x20 && height == 0x20 ||
                 BijectiveInclusion( width, height, 0x40u, 0x40u, 0x100u ) ||
                 BijectiveInclusion( width, height, 0x80u, 0x40u, 0x100u ) ||
                 BijectiveInclusion( width, height, 0x100u, 0x40u, 0x100u ) ) )
            {
                params.maxMIPLevel = 7;
            }

            if ( depth == 4 && (
                 width == 0x80 && height == 0x80 ||
                 width == 0x100 && height == 0x100 ||
                 BijectiveInclusion( width, height, 0x40u, 0x100u, 0x100u ) ) )
            {
                params.mmag = 1;  // LINEAR
            }

            if ( depth == 8 && (
                 BijectiveInclusion( width, height, 0x40u, 0x80u, 0x80u ) ||
                 BijectiveInclusion( width, height, 0x80u, 0x100u, 0x100u ) ) )
            {
                params.mmag = 1;  // LINEAR
            }

            uint32 mminMode = 0;    // NEAREST

            if ( depth == 4 && (
                 BijectiveInclusion( width, height, 0x80u, 0x100u, 0x100u ) ) )
            {
                mminMode = 1;   // LINEAR
            }

            if ( depth == 8 && (
                 width == 0x80 && height == 0x80 ||
                 width == 0x100 && height == 0x100 ||
                 width == 0x100 && height == 0x40 ) )
            {
                mminMode = 1;   // LINEAR
            }

            if ( depth == 4 && (
                 width == 0x100 && height == 0x100 ) )
            {
                mminMode |= 0x02;   // NEAREST_MIPMAP_x
            }

            if ( depth == 8 && (
                 width == 0x100 && height == 0x80 ||
                 width == 0x80 && height == 0x100 ) )
            {
                mminMode |= 0x02;   // NEAREST_MIPMAP_x
            }

            if ( depth == 4 && (
                 width == 0x200 && height == 0x100 ) )
            {
                mminMode |= 0x04;   // LINEAR_MIPMAP_x
            }

            if ( depth == 8 && (
                 width == 0x100 && height == 0x100 ) )
            {
                mminMode |= 0x04;   // LINEAR_MIPMAP_x
            }

            params.mmin = mminMode;

            if ( depth == 4 && (
                 width == 0x200 && height == 0x200 ) )
            {
                params.mtba = 1;  // detect automatically
            }

            //

            //

            if ( depth == 8 && (
                 width == 0x200 && height == 0x200 ) )
            {
                params.gsTEX1Unknown1 = 1;
            }
        }
    }

    if ( hasMipmaps )
    {
        if ( width == 0x100 && height == 0x100 ||
             width == 0x80 && height == 0x40 ||
             width == 0x10 && height == 0x10 ||
             BijectiveInclusion( width, height, 0x40u, 0x40u, 0x100u ) )
        {
            params.lodParamK |= 4;
        }

        if ( BijectiveInclusion( width, height, 0x20u, 0x20u, 0x80u ) ||
             BijectiveInclusion( width, height, 0x40u, 0x40u, 0x100u ) ||
             BijectiveInclusion( width, height, 0x80u, 0x20u, 0x40u ) )
        {
            params.lodParamK |= 0x08;
        }

        if ( BijectiveInclusion( width, height, 0x80u, 0x80u, 0x100u ) ||
             BijectiveInclusion( width, height, 0x100u, 0x80u, 0x100u ) )
        {
            params.lodParamK |= 0x10;
        }
    }

    // Give the parameters to the runtime.
    paramsOut = params;
}

bool NativeTexturePS2::generatePS2GPUData(
    uint32 gameVersion,
    ps2GSRegisters& gpuData,
    const uint32 mipmapBasePointer[], const uint32 mipmapBufferWidth[], const uint32 mipmapMemorySize[], uint32 maxMipmaps, eMemoryLayoutType memLayoutType,
    uint32 clutBasePointer
) const
{
    // This algorithm is guarranteed to produce correct values on identity-transformed PS2 GTA:SA textures.
    // There is no guarrantee that this works for modified textures!
    uint32 width = this->width[0];
    uint32 height = this->height[0];
    uint32 depth = this->mipmapDepth[0];

    // Reconstruct GPU flags, kinda.
    rw::ps2GSRegisters::TEX0_REG tex0;

    eRasterFormat pixelFormatRaster = parent->rasterFormat;

    // The base pointers are stored differently depending on game version.
    uint32 finalTexBasePointer = 0;
    uint32 finalClutBasePointer = 0;

    if (gameVersion == rw::GTA3_1 || gameVersion == rw::GTA3_2 || gameVersion == rw::GTA3_3 || gameVersion == rw::GTA3_4)
    {
        // We actually preallocate the textures on the game engine GS memory.
        // TODO: find out the TXD allocation order and fix this!
        uint32 totalMemOffset = 0;

        finalTexBasePointer = mipmapBasePointer[ 0 ] + totalMemOffset;
        finalClutBasePointer = clutBasePointer + totalMemOffset;
    }
    else
    {
        finalTexBasePointer = mipmapBasePointer[ 0 ];
    }

    tex0.textureBasePointer = finalTexBasePointer;
    tex0.textureBufferWidth = mipmapBufferWidth[ 0 ];
    tex0.pixelStorageFormat = memLayoutType;

    // Store texture dimensions.
    {
        if ( width == 0 || height == 0 )
            return false;
        
        double log2val = log(2.0);

        double expWidth = ( log((double)width) / log2val );
        double expHeight = ( log((double)height) / log2val );

        // Check that dimensions are power-of-two.
        if ( expWidth != floor(expWidth) || expHeight != floor(expHeight) )
            return false;

        // Check that dimensions are not too big.
        if ( expWidth > 10 || expHeight > 10 )
            return false;

        tex0.textureWidthLog2 = (unsigned int)expWidth;
        tex0.textureHeightLog2 = (unsigned int)expHeight;
    }

    tex0.texColorComponent = 1;     // with alpha
    tex0.texFunction = this->gsParams.textureFunction;
    tex0.clutBufferBase = finalClutBasePointer;

    // Decide about clut pixel storage format.
    {
        uint32 gsPixelFormat = 0;

        if ( pixelFormatRaster == rw::RASTER_8888 )
        {
            gsPixelFormat = 0;  // PSMCT32
        }
        else if ( pixelFormatRaster == rw::RASTER_1555 )
        {
            gsPixelFormat = 10; // PSMCT16S
        }
        else
        {
            // Incompatible CLUT pixel format.
            return false;
        }

        tex0.clutStorageFmt = gsPixelFormat;
    }

    tex0.clutMode = 0;  // CSM1
    tex0.clutEntryOffset = 0;

    if ( this->paletteType != PALETTE_NONE )
    {
        tex0.clutLoadControl = 1;
    }
    else
    {
        tex0.clutLoadControl = 0;
    }

    // Calculate TEX1 register.
    rw::ps2GSRegisters::TEX1_REG tex1;

    tex1.lodCalculationModel = this->gsParams.lodCalculationModel;
    tex1.maximumMIPLevel = this->gsParams.maxMIPLevel;
    tex1.mmag = this->gsParams.mmag;
    tex1.mmin = this->gsParams.mmin;
    tex1.mtba = this->gsParams.mtba;
    tex1.lodParamL = this->gsParams.lodParamL;
    tex1.lodParamK = this->gsParams.lodParamK;

    // Unknown registers.
    tex1.unknown = this->gsParams.gsTEX1Unknown1;
    tex1.unknown2 = this->gsParams.gsTEX1Unknown2;


    eRasterFormat rasterFormat = parent->rasterFormat;
    ePaletteType paletteType = this->paletteType;

    bool hasMipmaps = this->autoMipmaps;

    // Store mipmap data.
    ps2GSRegisters::MIPTBP1_REG miptbp1;
    ps2GSRegisters::MIPTBP2_REG miptbp2;

    // We want to fill out the entire fields of the registers.
    // This is why we assume the max mipmaps should meet that standard.
    assert(maxMipmaps >= 7);

    // Store the sizes and widths in the registers.
    miptbp1.textureBasePointer1 = mipmapBasePointer[1];
    miptbp1.textureBufferWidth1 = mipmapBufferWidth[1];
    miptbp1.textureBasePointer2 = mipmapBasePointer[2];
    miptbp1.textureBufferWidth2 = mipmapBufferWidth[2];
    miptbp1.textureBasePointer3 = mipmapBasePointer[3];
    miptbp1.textureBufferWidth3 = mipmapBufferWidth[3];

    miptbp2.textureBasePointer4 = mipmapBasePointer[4];
    miptbp2.textureBufferWidth4 = mipmapBufferWidth[4];
    miptbp2.textureBasePointer5 = mipmapBasePointer[5];
    miptbp2.textureBufferWidth5 = mipmapBufferWidth[5];
    miptbp2.textureBasePointer6 = mipmapBasePointer[6];
    miptbp2.textureBufferWidth6 = mipmapBufferWidth[6];

    // Give the data to the runtime.
    gpuData.tex0 = tex0;
    gpuData.tex1 = tex1;

    gpuData.miptbp1 = miptbp1;
    gpuData.miptbp2 = miptbp2;

    return true;
}

uint32 NativeTexture::writePs2(std::ostream& rw)
{
	HeaderInfo header;
    header.version = rw::rwInterface.GetVersion();
	uint32 writtenBytesReturn;

	if (platform != PLATFORM_PS2)
		return 0;

    NativeTexturePS2 *platformTex = (NativeTexturePS2*)this->platformData;

    // Texture Native.
    SKIP_HEADER();

    // Write the master header.
    {
        SKIP_HEADER();
        bytesWritten += writeUInt32(PLATFORM_PS2FOURCC, rw);

        texFormatInfo formatInfo;
        formatInfo.filterMode = this->filterFlags;
        formatInfo.uAddressing = this->uAddressing;
        formatInfo.vAddressing = this->vAddressing;

        rw.write((char*)&formatInfo, sizeof(formatInfo));

        bytesWritten += sizeof(formatInfo);

        WRITE_HEADER(CHUNK_STRUCT);
    }
    bytesWritten += writtenBytesReturn;

    // Write texture name.
    bytesWritten += writeStringSection(rw, this->name.c_str(), this->name.size());

    // Write mask name.
    bytesWritten += writeStringSection(rw, this->maskName.c_str(), this->maskName.size());

    // Prepare the image data (if not already prepared).
     uint32 mipmapCount = platformTex->mipmapCount;

    for ( uint32 n = 0; n < mipmapCount; n++ )
    {
        // Put the image data into the required format.
        bool hasEncrypted = platformTex->swizzleEncryptPS2( n );

        assert( hasEncrypted == true );
    }

    // Write the texture data.
    {
        // Write the master header.
        SKIP_HEADER();

        bool requiresHeaders = platformTex->requiresHeaders;

        // Prepare palette data.
        uint32 palTexWidth, palTexHeight;
        uint32 palDataSize = 0;
        void *paletteTexelMemory = NULL;
        bool hasAllocatedPaletteTexelMemory = false;

        if ( platformTex->paletteType != PALETTE_NONE )
        {
            if ( requiresHeaders )
            {
                genpalettetexeldata(platformTex->palette, this->rasterFormat, platformTex->paletteType, platformTex->paletteSize, paletteTexelMemory, palDataSize, palTexWidth, palTexHeight);

                hasAllocatedPaletteTexelMemory = true;
            }
            else
            {
                // We do not have to create a texture; just dump palette colors.
                palDataSize = ( platformTex->paletteSize * sizeof(PixelFormat::pixeldata32bit) );
                paletteTexelMemory = platformTex->palette;
            }
        }

        // Calculate block sizes.
        size_t justTextureSize = 0;
        size_t justPaletteSize = 0;
        {
            for ( uint32 n = 0; n < mipmapCount; n++ )
            {
                if ( requiresHeaders )
                {
                    justTextureSize += sizeof( textureImageDataHeader );
                }

                size_t dataSize = platformTex->dataSizes[ n ];

                justTextureSize += dataSize;
            }

            if ( platformTex->paletteType != PALETTE_NONE )
            {
                if ( requiresHeaders )
                {
                    justPaletteSize += sizeof( textureImageDataHeader );
                }

                justPaletteSize += palDataSize;
            }
        }

        // Write the texture meta information.
        const size_t maxMipmaps = 7;

        // Make sure all textures are in the required encoding format.
        eFormatEncodingType requiredFormat = platformTex->getHardwareRequiredEncoding(header.version);

        // Get the format we should decode to.
        eFormatEncodingType actualEncodingType = getFormatEncodingFromRasterFormat(this->rasterFormat, platformTex->paletteType);

        assert( requiredFormat != FORMAT_UNKNOWN );
        assert( actualEncodingType != FORMAT_UNKNOWN );

        ps2MipmapTransmissionData mipmapTransData[ maxMipmaps ];
        ps2MipmapTransmissionData clutTransData;
        {
            SKIP_HEADER();

            // Allocate textures.
            uint32 mipmapBasePointer[ maxMipmaps ];
            uint32 mipmapBufferWidth[ maxMipmaps ];
            uint32 mipmapMemorySize[ maxMipmaps ];

            uint32 clutBasePointer;
            uint32 clutMemSize;

            eMemoryLayoutType decodedMemLayoutType;

            bool couldAllocate = platformTex->allocateTextureMemory(mipmapBasePointer, mipmapBufferWidth, mipmapMemorySize, mipmapTransData, maxMipmaps, decodedMemLayoutType, clutBasePointer, clutMemSize, clutTransData);

            ps2GSRegisters gpuData;
            bool isCompatible = platformTex->generatePS2GPUData(header.version, gpuData, mipmapBasePointer, mipmapBufferWidth, mipmapMemorySize, maxMipmaps, decodedMemLayoutType, clutBasePointer);

            assert( isCompatible == true );

            // Create raster format flags.
            uint32 formatFlags = generateRasterFormatFlags( this->rasterFormat, platformTex->paletteType, platformTex->mipmapCount > 1, platformTex->autoMipmaps );

            // Apply special flags.
            if ( platformTex->requiresHeaders )
            {
                formatFlags |= 0x20000;
            }
            else if ( platformTex->hasSwizzle )
            {
                formatFlags |= 0x10000;
            }

            // Apply unknown stuff.
            formatFlags |= platformTex->unknownFormatFlags;

            textureMetaDataHeader metaHeader;
            metaHeader.miptbp1 = gpuData.miptbp1;
            metaHeader.miptbp2 = gpuData.miptbp2;
            metaHeader.width = platformTex->width[0];
            metaHeader.height = platformTex->height[0];
            metaHeader.depth = platformTex->mipmapDepth[0];
            metaHeader.tex0 = gpuData.tex0;
            metaHeader.tex1 = gpuData.tex1;
            metaHeader.rasterFormat = formatFlags;
            metaHeader.dataSize = justTextureSize;
            metaHeader.paletteDataSize = justPaletteSize;
            metaHeader.combinedGPUDataSize = platformTex->calculateGPUDataSize(mipmapBasePointer, mipmapMemorySize, maxMipmaps, decodedMemLayoutType, clutBasePointer, clutMemSize);
            metaHeader.skyMipmapVal = platformTex->skyMipMapVal;

            rw.write((const char*)&metaHeader, sizeof(metaHeader));

            bytesWritten += sizeof( textureMetaDataHeader );

            WRITE_HEADER(CHUNK_STRUCT);
        }
        bytesWritten += writtenBytesReturn;

        // Write the texture data (with palette info).
        {
            SKIP_HEADER();

            for ( uint32 n = 0; n < mipmapCount; n++ )
            {
                // TODO: swizzle the image data (if required)
                assert(platformTex->swizzleEncodingType[n] == requiredFormat);

                uint32 curDataSize = platformTex->dataSizes[n];

                if ( requiresHeaders )
                {
                    uint32 texWidth = platformTex->swizzleWidth[ n ];
                    uint32 texHeight = platformTex->swizzleHeight[ n ];

                    if (requiredFormat == FORMAT_TEX32 && actualEncodingType == FORMAT_IDTEX8_COMPRESSED)
                    {
                        texWidth *= 2;
                    }

                    // Save the important stuff.
                    ps2MipmapTransmissionData& mipTransData = mipmapTransData[ n ];

                    writeImageDataHeader(
                        rw,
                        texWidth,
                        texHeight,
                        curDataSize,
                        mipTransData.destX, mipTransData.destY
                    );

                    bytesWritten += sizeof(textureImageDataHeader);
                }

                rw.write((const char*)platformTex->texels[n], curDataSize);

                bytesWritten += curDataSize;
            }

            // Write palette information.
            if ( platformTex->paletteType != PALETTE_NONE )
            {
                if ( requiresHeaders )
                {
                    writeImageDataHeader(
                        rw,
                        palTexWidth, palTexHeight,
                        palDataSize,
                        clutTransData.destX, clutTransData.destY
                    );

                    bytesWritten += sizeof(textureImageDataHeader);
                }

                rw.write((const char*)paletteTexelMemory, palDataSize);

                bytesWritten += palDataSize;
            }

            if ( hasAllocatedPaletteTexelMemory )
            {
                delete [] paletteTexelMemory;
            }

            WRITE_HEADER(CHUNK_STRUCT);
        }

        bytesWritten += writtenBytesReturn;

        WRITE_HEADER(CHUNK_STRUCT);
    }
    bytesWritten += writtenBytesReturn;

	// Extension
	{
		SKIP_HEADER();

        // Write the sky mipmap extension.
        {
            SKIP_HEADER();
            bytesWritten += writeUInt32(platformTex->skyMipMapVal, rw);
            WRITE_HEADER(CHUNK_SKYMIPMAP);
        }
        bytesWritten += writtenBytesReturn;

		WRITE_HEADER(CHUNK_EXTENSION);
	}
	bytesWritten += writtenBytesReturn;

    WRITE_HEADER(CHUNK_TEXTURENATIVE);

    return writtenBytesReturn;
}

};