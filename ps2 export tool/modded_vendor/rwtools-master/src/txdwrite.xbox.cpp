#include <StdInc.h>

#include "txdread.xbox.hxx"

#include "txdread.d3d.hxx"

#include "streamutil.hxx"

namespace rw
{

uint32 NativeTexture::writeXbox(std::ostream& rw)
{
	HeaderInfo header;
    header.setVersion( rw::rwInterface.GetVersion() );
	uint32 writtenBytesReturn;

	if (platform != PLATFORM_XBOX)
		return 0;

    NativeTextureXBOX *platformTex = (NativeTextureXBOX*)this->platformData;

    // Debug some essentials.
    if ( platformTex->colorOrder != COLOR_BGRA )
    {
        throw RwException( "texture " + this->name + " has an invalid color ordering for writing (requires BGRA)" );
    }

    // Texture Native.
	SKIP_HEADER();

    // Write the struct.
    {
        SKIP_HEADER();

        // First comes the platform id.
        bytesWritten += writeUInt32(PLATFORM_XBOX, rw);

        // Write the header.
        uint32 mipmapCount = platformTex->mipmapCount;
        {
            textureMetaHeaderStructXbox metaInfo;

            // Write addressing information.
            metaInfo.formatInfo.filterMode = this->filterFlags;
            metaInfo.formatInfo.uAddressing = this->uAddressing;
            metaInfo.formatInfo.vAddressing = this->vAddressing;
            metaInfo.formatInfo.pad1 = 0;

            // Write texture names.
            // These need to be written securely.
            writeStringIntoBufferSafe( this->name, metaInfo.name, sizeof( metaInfo.name ), this->name, "name" );
            writeStringIntoBufferSafe( this->maskName, metaInfo.maskName, sizeof( metaInfo.maskName ), this->name, "mask name" );

            // Construct raster flags.
            uint32 rasterFlags = generateRasterFormatFlags(this->rasterFormat, platformTex->paletteType, mipmapCount > 1, platformTex->autoMipmaps);

            // Store the flags.
            metaInfo.rasterFormat = rasterFlags;

            metaInfo.hasAlpha = platformTex->hasAlpha;

            metaInfo.mipmapCount = platformTex->mipmapCount;

            metaInfo.rasterType = platformTex->rasterType;

            metaInfo.dxtCompression = platformTex->dxtCompression;

            // Write the dimensions.
            metaInfo.width = platformTex->width[ 0 ];
            metaInfo.height = platformTex->height[ 0 ];

            metaInfo.depth = platformTex->mipmapDepth[ 0 ];

            // Calculate the size of all the texture data combined.
            uint32 imageDataSectionSize = 0;

            for (uint32 n = 0; n < mipmapCount; n++)
            {
                imageDataSectionSize += platformTex->dataSizes[ n ];
            }

            metaInfo.imageDataSectionSize = imageDataSectionSize;

            // Write the generic header.
            rw.write((const char*)&metaInfo, sizeof( metaInfo ));

            bytesWritten += sizeof( metaInfo );
        }

        // Write palette data (if available).
        if (platformTex->paletteType != PALETTE_NONE)
        {
            // Make sure we write as much data as the system expects.
            uint32 reqPalCount = 0;

            if (platformTex->paletteType == PALETTE_4BIT)
            {
                reqPalCount = 32;
            }
            else if (platformTex->paletteType == PALETTE_8BIT)
            {
                reqPalCount = 256;
            }
            else
            {
                assert( 0 );
            }

            uint32 palItemCount = platformTex->paletteSize;

            // Get the real data size of the palette.
            uint32 palRasterDepth = Bitmap::getRasterFormatDepth(this->rasterFormat);

            uint32 paletteDataSize = getRasterDataSize( palItemCount, palRasterDepth );

            uint32 palByteWriteCount = writePartialBlockSafe(rw, platformTex->palette, paletteDataSize, getRasterDataSize(reqPalCount, palRasterDepth));
    
            assert( palByteWriteCount * 8 / palRasterDepth == reqPalCount );

            bytesWritten += palByteWriteCount;
        }

        // Write mipmap data.
        for ( uint32 n = 0; n < mipmapCount; n++ )
        {
			uint32 texDataSize = platformTex->dataSizes[ n ];

            uint8 *texelData = (uint8*)platformTex->texels[ n ];

			rw.write((const char*)texelData, texDataSize);
			bytesWritten += texDataSize;
        }

        WRITE_HEADER(CHUNK_STRUCT);
    }
    bytesWritten += writtenBytesReturn;

	// Extension
	{
		SKIP_HEADER();
		WRITE_HEADER(CHUNK_EXTENSION);
	}
	bytesWritten += writtenBytesReturn;

    WRITE_HEADER(CHUNK_TEXTURENATIVE);

	return bytesWritten;
}

};