#include <StdInc.h>

#include "txdread.xbox.hxx"
#include "txdread.d3d.hxx"

namespace rw
{

#pragma pack(1)
struct textureMetaHeaderStruct
{
    rw::texFormatInfo formatInfo;

    char name[32];
    char maskName[32];

    uint32 rasterFormat;
    uint32 hasAlpha;
    uint16 width, height;

    uint8 depth;
    uint8 mipmapCount;
    uint8 rasterType;
    uint8 dxtCompression;

    uint32 unknownSize;
};
#pragma pack()

void NativeTexture::readXbox(std::istream &rw)
{
	HeaderInfo header;

	READ_HEADER(CHUNK_STRUCT);
	uint32 end = rw.tellg();
	end += header.length;

	uint32 platform = readUInt32(rw);

	// improve error handling
	if (platform != PLATFORM_XBOX)
		return;

    // Create the platform data container.
    NativeTextureXBOX *platformTex = new NativeTextureXBOX();

    // same as above.
    if ( !platformTex )
        return;

    textureMetaHeaderStruct metaInfo;
    rw.read((char*)&metaInfo, sizeof(metaInfo));

	this->filterFlags = metaInfo.formatInfo.filterMode;
    this->uAddressing = metaInfo.formatInfo.uAddressing;
    this->vAddressing = metaInfo.formatInfo.vAddressing;

    // TODO: fix this.
	this->name = metaInfo.name;
    this->maskName = metaInfo.maskName;

    // Deconstruct the rasterFormat flags.
    bool hasMipmaps = false;        // TODO: actually use this flag.
    bool autoMipmaps = false;

    readRasterFormatFlags( metaInfo.rasterFormat, this->rasterFormat, platformTex->paletteType, hasMipmaps, autoMipmaps );

	this->hasAlpha = ( metaInfo.hasAlpha != 0 );

	uint32 depth = metaInfo.depth;
	platformTex->mipmapCount = metaInfo.mipmapCount;

    assert( metaInfo.rasterType == 4 );

	platformTex->dxtCompression = metaInfo.dxtCompression;

    // TODO: debug the size parameter.

	if (platformTex->paletteType != PALETTE_NONE)
    {
		platformTex->paletteSize = getPaletteItemCount( platformTex->paletteType );

        uint32 paletteDataSize = platformTex->paletteSize * sizeof(uint32);

		platformTex->palette = new uint8[paletteDataSize];
		rw.read(reinterpret_cast <char *> (platformTex->palette), paletteDataSize);
	}

	for (uint32 i = 0; i < platformTex->mipmapCount; i++)
    {
        uint32 texWidth, texHeight;

		if (i > 0)
        {
            texWidth = platformTex->width[i-1] / 2;
            texHeight = platformTex->height[i-1] / 2;
        }
        else
        {
            texWidth = metaInfo.width;
            texHeight = metaInfo.height;
        }

        // Process dimensions.
        {
			platformTex->width.push_back( texWidth );
			platformTex->height.push_back( texHeight );

			// DXT compression works on 4x4 blocks,
			// no smaller values allowed
			if (platformTex->dxtCompression)
            {
				if (texWidth < 4 && texWidth != 0)
                {
					platformTex->width[i] = 4;
                }
				if (texHeight < 4 && texHeight != 0)
                {
					platformTex->height[i] = 4;
                }
			}
        }

        uint32 texUnitCount = ( texWidth * texHeight );
        uint32 texDataSize = 0;

		if (metaInfo.dxtCompression == 0)
        {
            texDataSize = texUnitCount * (depth/8);
        }
		else if (metaInfo.dxtCompression == 0xC)	// DXT1 (?)
        {
            texDataSize = texUnitCount / 2;
        }
        else
        {
            // Not so sure; should debug this.
            texDataSize = texUnitCount;
        }
		// else (0xe, 0xf) DXT3 (?)

        // Store the texture size.
        platformTex->dataSizes.push_back( texDataSize );

        platformTex->mipmapDepth.push_back( depth );

        uint8 *texelData = new uint8[texDataSize];

		platformTex->texels.push_back(texelData);
		rw.read(reinterpret_cast <char *> (texelData), texDataSize*sizeof(uint8));
	}
}

void unswizzleXboxBlock(uint8 *out, uint8 *in, uint32 &outOff, uint32 inOff,
                        uint32 width, uint32 height, uint32 stride)
{
	if (width < 2 || height < 2)
    {
		memcpy(out+outOff, in+inOff, width*height);

		inOff += width*height;
	}
    else if (width == 2 && height == 2)
    {
		*(out+outOff) = *(in+inOff);
		*(out+outOff+1) = *(in+inOff+1);
		*(out+outOff+stride) = *(in+inOff+2);
		*(out+outOff+stride+1) = *(in+inOff+3);

		inOff += 4;
	}
    else
    {
		unswizzleXboxBlock(
            out, in, inOff,
            outOff, width/2, height/2, stride
        );

		unswizzleXboxBlock(
            out, in, inOff,
            outOff + (width/2), width/2,
            height/2, stride
        );

		unswizzleXboxBlock(
            out, in, inOff,
            outOff + (height/2)*stride,width/2,
            height/2, stride
        );

		unswizzleXboxBlock(
            out, in, inOff,
            outOff + (height/2)*stride + (width/2),
            width/2, height/2, stride
        );
	}
}

void NativeTexture::convertFromXbox(void)
{
    if ( platform != PLATFORM_XBOX )
        return;

    // Allocate our new texture container.
    NativeTextureD3D *d3dTex = new NativeTextureD3D();

    if ( !d3dTex )
        return;

    NativeTextureXBOX *platformTex = (NativeTextureXBOX*)this->platformData;

    // Copy common data.
    d3dTex->width = platformTex->width;
    d3dTex->height = platformTex->height;
    d3dTex->mipmapDepth = platformTex->mipmapDepth;

    // Move over palette information.
    d3dTex->palette = platformTex->palette;
    d3dTex->paletteSize = platformTex->paletteSize;
    d3dTex->paletteType = platformTex->paletteType;

    uint32 targetCompression = 0;

	if (platformTex->dxtCompression == 0xc)
    {
		targetCompression = 1;

		if (hasAlpha)
        {
			this->rasterFormat = RASTER_1555;
        }
		else
        {
			this->rasterFormat = RASTER_565;
        }
	}
    else if (platformTex->dxtCompression == 0xe)
    {
		targetCompression = 3;

		this->rasterFormat = RASTER_4444;
	}
    else if (platformTex->dxtCompression == 0xf)
    {
		targetCompression = 4;
	}

    // Move our texel information.
    d3dTex->texels = platformTex->texels;
    d3dTex->dataSizes = platformTex->dataSizes;

    d3dTex->dxtCompression = targetCompression;

    // Backlink and put our texture as new platform data.
    d3dTex->parent = this;

    this->platformData = d3dTex;

    // Delete the old container.
    delete platformTex;

	platform = PLATFORM_D3D8;
}

};