#include <StdInc.h>

#include "txdread.d3d.hxx"

#include "streamutil.hxx"

namespace rw
{

uint32 NativeTexture::writeD3d(std::ostream &rw)
{
	HeaderInfo header;
    header.setVersion( rw::rwInterface.GetVersion() );
	uint32 writtenBytesReturn;

	if (platform != PLATFORM_D3D8 && platform != PLATFORM_D3D9)
		return 0;

    NativeTextureD3D *platformTex = (NativeTextureD3D*)this->platformData;

    // Make sure the texture has some qualities before it can even be written.
    if ( !platformTex->hasD3DFormat )
    {
        throw RwException( "texture " + this->name + " has no representation in Direct3D" );
    }

    // Texture Native.
	SKIP_HEADER();

	// Struct
	{
		SKIP_HEADER();
		bytesWritten += writeUInt32(platform, rw);

        textureMetaHeaderStructGeneric metaHeader;
        metaHeader.texFormat.filterMode = this->filterFlags;
        metaHeader.texFormat.uAddressing = this->uAddressing;
        metaHeader.texFormat.vAddressing = this->vAddressing;

        // Correctly write the name strings (for safety).
        // Even though we can read those name fields with zero-termination safety,
        // the engines are not guarranteed to do so.
        // Also, print a warning if the name is changed this way.
        writeStringIntoBufferSafe( this->name, metaHeader.name, sizeof( metaHeader.name ), this->name, "name" );
        writeStringIntoBufferSafe( this->maskName, metaHeader.maskName, sizeof( metaHeader.maskName ), this->name, "mask name" );

        // Construct raster flags.
        metaHeader.rasterFormat = generateRasterFormatFlags( this->rasterFormat, platformTex->paletteType, platformTex->mipmapCount > 1, platformTex->autoMipmaps );

        rw.write((const char*)&metaHeader, sizeof(metaHeader));

        bytesWritten += sizeof(metaHeader);

		if (platform == PLATFORM_D3D8)
        {
			bytesWritten += writeUInt32(platformTex->hasAlpha, rw);
		}
        else
        {
		    bytesWritten += writeUInt32((uint32)platformTex->d3dFormat, rw);
		}

        textureMetaHeaderStructDimInfo dimInfo;
        dimInfo.width = platformTex->width[0];
        dimInfo.height = platformTex->height[0];
        dimInfo.depth = platformTex->depth;
        dimInfo.mipmapCount = platformTex->mipmapCount;
        dimInfo.rasterType = platformTex->rasterType;

        rw.write((const char*)&dimInfo, sizeof(dimInfo));

        bytesWritten += sizeof(dimInfo);

		if (platform == PLATFORM_D3D8)
        {
			bytesWritten += writeUInt8( platformTex->dxtCompression, rw );
        }
		else
        {
            textureContentInfoStruct contentInfo;
            contentInfo.hasAlpha = platformTex->hasAlpha;
            contentInfo.isCubeTexture = platformTex->isCubeTexture;
            contentInfo.autoMipMaps = platformTex->autoMipmaps;
            contentInfo.isCompressed = ( platformTex->dxtCompression != 0 );

            rw.write((const char*)&contentInfo, sizeof(contentInfo));

            bytesWritten += sizeof(contentInfo);
        }

		/* Palette */
		if (platformTex->paletteType != PALETTE_NONE)
        {
            // Make sure we write as much data as the system expects.
            uint32 reqPalCount = getPaletteItemCount(platformTex->paletteType);

            uint32 palItemCount = platformTex->paletteSize;

            // Get the real data size of the palette.
            uint32 palRasterDepth = Bitmap::getRasterFormatDepth(this->rasterFormat);

            uint32 paletteDataSize = getRasterDataSize( palItemCount, palRasterDepth );

            uint32 palByteWriteCount = writePartialBlockSafe(rw, platformTex->palette, paletteDataSize, getRasterDataSize(reqPalCount, palRasterDepth));
    
            assert( palByteWriteCount * 8 / palRasterDepth == reqPalCount );

            bytesWritten += palByteWriteCount;
		}

		/* Texels */
		for (uint32 i = 0; i < platformTex->mipmapCount; i++)
        {
			uint32 texDataSize = platformTex->dataSizes[i];

			bytesWritten += writeUInt32(texDataSize, rw);

            uint8 *texelData = (uint8*)platformTex->texels[i];

			rw.write(reinterpret_cast <char *> (texelData), texDataSize);
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

	return bytesWritten;
}

}