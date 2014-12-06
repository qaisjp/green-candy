#include <StdInc.h>

#include "txdread.d3d.hxx"

#include "streamutil.hxx"

namespace rw
{

uint32 NativeTexture::writeD3d(std::ostream &rw)
{
	HeaderInfo header;
    header.version = rw::rwInterface.GetVersion();
	uint32 writtenBytesReturn;

	if (platform != PLATFORM_D3D8 &&
	    platform != PLATFORM_D3D9)
		return 0;

    NativeTextureD3D *platformTex = (NativeTextureD3D*)this->platformData;

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

        strncpy( metaHeader.name, this->name.c_str(), 32 );
        strncpy( metaHeader.maskName, this->maskName.c_str(), 32 );

        // Construct raster flags.
        metaHeader.rasterFormat = generateRasterFormatFlags( this->rasterFormat, platformTex->paletteType, platformTex->mipmapCount > 1, platformTex->autoMipmaps );

        rw.write((const char*)&metaHeader, sizeof(metaHeader));

        bytesWritten += sizeof(metaHeader);

		if (platform == PLATFORM_D3D8)
        {
			bytesWritten += writeUInt32(this->hasAlpha, rw);
		}
        else
        {
		    bytesWritten += writeUInt32((uint32)platformTex->d3dFormat, rw);
		}

        textureMetaHeaderStructDimInfo dimInfo;
        dimInfo.width = platformTex->width[0];
        dimInfo.height = platformTex->height[0];
        dimInfo.depth = platformTex->mipmapDepth[0];
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
            contentInfo.hasAlpha = this->hasAlpha;
            contentInfo.isCubeTexture = platformTex->isCubeTexture;
            contentInfo.autoMipMaps = platformTex->autoMipmaps;
            contentInfo.isCompressed = ( platformTex->dxtCompression != 0 );

            rw.write((const char*)&contentInfo, sizeof(contentInfo));

            bytesWritten += sizeof(contentInfo);
        }

		/* Palette */
		if (platformTex->paletteType != PALETTE_NONE)
        {
            uint32 paletteDataSize = platformTex->paletteSize * sizeof(uint32);

			rw.write(reinterpret_cast <char *> (platformTex->palette), paletteDataSize);
			bytesWritten += paletteDataSize;
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