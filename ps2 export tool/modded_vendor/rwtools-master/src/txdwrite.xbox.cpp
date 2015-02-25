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
    ePaletteType paletteType = platformTex->paletteType;

    uint32 compressionType = platformTex->dxtCompression;

    // If we are not compressed, then the color order matters.
    if ( compressionType == 0 )
    {
        eColorOrdering requiredColorOrder = COLOR_BGRA;

        if ( paletteType != PALETTE_NONE )
        {
            requiredColorOrder = COLOR_RGBA;
        }

        if ( platformTex->colorOrder != requiredColorOrder )
        {
            throw RwException( "texture " + this->name + " has an invalid color ordering for writing" );
        }
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
            metaInfo.formatInfo.set( *this );

            // Write texture names.
            // These need to be written securely.
            writeStringIntoBufferSafe( this->name, metaInfo.name, sizeof( metaInfo.name ), this->name, "name" );
            writeStringIntoBufferSafe( this->maskName, metaInfo.maskName, sizeof( metaInfo.maskName ), this->name, "mask name" );

            // Construct raster flags.
            uint32 rasterFlags = generateRasterFormatFlags(this->rasterFormat, paletteType, mipmapCount > 1, platformTex->autoMipmaps);

            // Store the flags.
            metaInfo.rasterFormat = rasterFlags;

            metaInfo.hasAlpha = platformTex->hasAlpha;

            metaInfo.mipmapCount = platformTex->mipmapCount;

            metaInfo.rasterType = platformTex->rasterType;

            metaInfo.dxtCompression = compressionType;

            // Write the dimensions.
            metaInfo.width = platformTex->width[ 0 ];
            metaInfo.height = platformTex->height[ 0 ];

            metaInfo.depth = platformTex->depth;

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
        if (paletteType != PALETTE_NONE)
        {
            // Make sure we write as much data as the system expects.
            uint32 reqPalCount = getD3DPaletteCount(paletteType);

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

	return writtenBytesReturn;
}

};