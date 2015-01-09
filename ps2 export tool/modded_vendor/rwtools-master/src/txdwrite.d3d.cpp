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
        {
            size_t nameLen = this->name.size();

            if (nameLen >= sizeof(metaHeader.name))
            {
                rw::rwInterface.PushWarning( "texture " + this->name + " has been written using truncated name" );

                nameLen = sizeof(metaHeader.name) - 1;
            }

            memcpy( metaHeader.name, this->name.c_str(), nameLen );

            // Pad with zeroes (which will also zero-terminate).
            memset( metaHeader.name + nameLen, 0, sizeof(metaHeader.name) - nameLen );
        }
        {
            size_t maskNameLen = this->maskName.size();

            if (maskNameLen >= sizeof(metaHeader.maskName))
            {
                rw::rwInterface.PushWarning( "texture " + this->maskName + " has been written using truncated mask name" );

                maskNameLen = sizeof(metaHeader.maskName) - 1;
            }

            memcpy( metaHeader.maskName, this->maskName.c_str(), maskNameLen );

            // Pad with zeroes.
            memset( metaHeader.maskName + maskNameLen, 0, sizeof(metaHeader.maskName) - maskNameLen );
        }

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

            uint32 actualPalItemWriteCount = std::min( palItemCount, reqPalCount );

            // Get the real data size of the palette.
            uint32 palRasterDepth = Bitmap::getRasterFormatDepth(this->rasterFormat);

            uint32 paletteDataSize = getRasterDataSize( actualPalItemWriteCount, palRasterDepth );

			rw.write(reinterpret_cast <char *> (platformTex->palette), paletteDataSize);

            uint32 palByteWriteCount = paletteDataSize;
    
            // Write the remainder, if required.
            if (actualPalItemWriteCount < reqPalCount)
            {
                uint32 leftCount = ( reqPalCount - actualPalItemWriteCount );

                uint32 leftDataSize = getRasterDataSize( leftCount, palRasterDepth );

                for ( uint32 n = 0; n < leftDataSize; n++ )
                {
                    writeUInt8(0, rw);
                }

                palByteWriteCount += leftDataSize;
            }

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