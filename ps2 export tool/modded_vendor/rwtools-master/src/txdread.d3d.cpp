#include <StdInc.h>

#include "txdread.d3d.hxx"

namespace rw
{

void NativeTexture::readD3d(std::istream &rw)
{
	HeaderInfo header;

	READ_HEADER(CHUNK_STRUCT);
	uint32 end = rw.tellg();
	end += header.length;

	uint32 platform = readUInt32(rw);
	// improve error handling
	if (platform != PLATFORM_D3D8 && platform != PLATFORM_D3D9)
		return;

    // Create the Direct3D texture container.
    NativeTextureD3D *platformTex = new NativeTextureD3D();

    //same as above.
    if ( !platformTex )
        return;

    // Store the backlink.
    platformTex->parent = this;

    this->platformData = platformTex;

    textureMetaHeaderStructGeneric metaHeader;
    rw.read((char*)&metaHeader, sizeof(metaHeader));

	this->filterFlags = metaHeader.texFormat.filterMode;
    this->uAddressing = metaHeader.texFormat.uAddressing;
    this->vAddressing = metaHeader.texFormat.vAddressing;

    // Read the texture names.
    this->name = metaHeader.name;
    this->maskName = metaHeader.maskName;

	this->rasterFormat = metaHeader.rasterFormat;

	hasAlpha = false;

	char fourcc[5];
	fourcc[4] = 0;

	if ( platform == PLATFORM_D3D9 )
    {
		rw.read(fourcc, 4*sizeof(char));
    }
	else
    {
		hasAlpha = ( readUInt32(rw) != 0 );
    }

    textureMetaHeaderStructDimInfo dimInfo;
    rw.read((char*)&dimInfo, sizeof(dimInfo));

	uint32 depth = dimInfo.depth;
	platformTex->mipmapCount = dimInfo.mipmapCount;

    assert( dimInfo.rasterType == 4 );  // TODO

    platformTex->rasterType = dimInfo.rasterType;

	uint32 dxtInfo = dimInfo.dxtCompression;

	if ( platform == PLATFORM_D3D9 )
    {
		hasAlpha = dxtInfo & 0x1;

		if (dxtInfo & 0x8)
        {
			platformTex->dxtCompression = fourcc[3] - '0';
        }
		else
        {
			platformTex->dxtCompression = 0;
        }
    }
    else
    {
        platformTex->dxtCompression = dxtInfo;
    }

	if (rasterFormat & RASTER_PAL8 || rasterFormat & RASTER_PAL4)
    {
		platformTex->paletteSize = (rasterFormat & RASTER_PAL8) ? 0x100 : 0x10;

		platformTex->palette = new uint8[platformTex->paletteSize*4 *sizeof(uint8)];
		rw.read(reinterpret_cast <char *> (platformTex->palette), platformTex->paletteSize*4*sizeof(uint8));
	}

    uint32 mipmapCount = platformTex->mipmapCount;

	for (uint32 i = 0; i < mipmapCount; i++)
    {
        uint32 texWidth, texHeight;

		if (i > 0)
        {
            texWidth = platformTex->width[i-1];
            texHeight = platformTex->height[i-1];
        }
        else
        {
            texWidth = dimInfo.width;
            texHeight = dimInfo.height;
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

        platformTex->mipmapDepth.push_back( depth );

		uint32 texDataSize = readUInt32(rw);

		// There is no way to predict, when the size is going to be zero
		if (texDataSize == 0)
        {
			platformTex->width[i] = platformTex->height[i] = 0;
        }

		platformTex->dataSizes.push_back(texDataSize);

        uint8 *texelData = new uint8[texDataSize];

		platformTex->texels.push_back(texelData);
		rw.read(reinterpret_cast <char *> (texelData), texDataSize*sizeof(uint8));
    }
}

void NativeTextureD3D::decompressDxt4(void)
{
    uint32 mipmapCount = this->mipmapCount;

	for (uint32 i = 0; i < mipmapCount; i++)
    {
        uint8 *texelData = (uint8*)this->texels[i];

		/* j loops through old texels
		 * x and y loop through new texels */
		uint32 x = 0, y = 0;
		uint32 dataSize = width[i]*height[i]*4;
		uint8 *newtexels = new uint8[dataSize];

		for (uint32 j = 0; j < width[i]*height[i]; j += 16)
        {
			/* calculate colors */
			uint32 col0 = *((uint16 *) &texelData[j+8]);
			uint32 col1 = *((uint16 *) &texelData[j+10]);
			uint32 c[4][4];
			// swap r and b
			c[0][0] = (col0 & 0x1F)*0xFF/0x1F;
			c[0][1] = ((col0 & 0x7E0) >> 5)*0xFF/0x3F;
			c[0][2] = ((col0 & 0xF800) >> 11)*0xFF/0x1F;

			c[1][0] = (col1 & 0x1F)*0xFF/0x1F;
			c[1][1] = ((col1 & 0x7E0) >> 5)*0xFF/0x3F;
			c[1][2] = ((col1 & 0xF800) >> 11)*0xFF/0x1F;

			c[2][0] = (2*c[0][0] + 1*c[1][0])/3;
			c[2][1] = (2*c[0][1] + 1*c[1][1])/3;
			c[2][2] = (2*c[0][2] + 1*c[1][2])/3;

			c[3][0] = (1*c[0][0] + 2*c[1][0])/3;
			c[3][1] = (1*c[0][1] + 2*c[1][1])/3;
			c[3][2] = (1*c[0][2] + 2*c[1][2])/3;

			uint32 a[8];
			a[0] = texelData[j+0];
			a[1] = texelData[j+1];
			if (a[0] > a[1])
            {
				a[2] = (6*a[0] + 1*a[1])/7;
				a[3] = (5*a[0] + 2*a[1])/7;
				a[4] = (4*a[0] + 3*a[1])/7;
				a[5] = (3*a[0] + 4*a[1])/7;
				a[6] = (2*a[0] + 5*a[1])/7;
				a[7] = (1*a[0] + 6*a[1])/7;
			}
            else
            {
				a[2] = (4*a[0] + 1*a[1])/5;
				a[3] = (3*a[0] + 2*a[1])/5;
				a[4] = (2*a[0] + 3*a[1])/5;
				a[5] = (1*a[0] + 4*a[1])/5;
				a[6] = 0;
				a[7] = 0xFF;
			}

			/* make index list */
			uint32 indicesint = *((uint32 *) &texelData[j+12]);
			uint8 indices[16];
			for (int32 k = 0; k < 16; k++)
            {
				indices[k] = indicesint & 0x3;
				indicesint >>= 2;
			}
			// actually 6 bytes
			uint64 alphasint = *((uint64 *) &texelData[j+2]);
			uint8 alphas[16];
			for (int32 k = 0; k < 16; k++)
            {
				alphas[k] = alphasint & 0x7;
				alphasint >>= 3;
			}

			/* write bytes */
			for (uint32 k = 0; k < 4; k++)
            {
				for (uint32 l = 0; l < 4; l++) {
	// wtf?
	newtexels[(y+l)*width[i]*4 + (x+k)*4 + 0] = c[indices[l*4+k]][0];
	newtexels[(y+l)*width[i]*4 + (x+k)*4 + 1] = c[indices[l*4+k]][1];
	newtexels[(y+l)*width[i]*4 + (x+k)*4 + 2] = c[indices[l*4+k]][2];
	newtexels[(y+l)*width[i]*4 + (x+k)*4 + 3] = a[alphas[l*4+k]];
				}
            }
			x += 4;
			if (x >= width[i])
            {
				y += 4;
				x = 0;
			}
		}
		delete[] texels[i];
		texels[i] = newtexels;
		dataSizes[i] = dataSize;
        this->mipmapDepth[i] = 0x20;
	}
	parent->rasterFormat = RASTER_8888;
	dxtCompression = 0;
}

void NativeTextureD3D::decompressDxt3(void)
{
	for (uint32 i = 0; i < mipmapCount; i++)
    {
        uint8 *texelData = (uint8*)this->texels[i];

		/* j loops through old texels
		 * x and y loop through new texels */
		uint32 x = 0, y = 0;
		uint32 dataSize = width[i]*height[i]*4;
		uint8 *newtexels = new uint8[dataSize];
		for (uint32 j = 0; j < width[i]*height[i]; j += 16)
        {
			/* calculate colors */
			uint32 col0 = *((uint16 *) &texelData[j+8]);
			uint32 col1 = *((uint16 *) &texelData[j+10]);
			uint32 c[4][4];
			// swap r and b
			c[0][0] = (col0 & 0x1F)*0xFF/0x1F;
			c[0][1] = ((col0 & 0x7E0) >> 5)*0xFF/0x3F;
			c[0][2] = ((col0 & 0xF800) >> 11)*0xFF/0x1F;

			c[1][0] = (col1 & 0x1F)*0xFF/0x1F;
			c[1][1] = ((col1 & 0x7E0) >> 5)*0xFF/0x3F;
			c[1][2] = ((col1 & 0xF800) >> 11)*0xFF/0x1F;

			c[2][0] = (2*c[0][0] + 1*c[1][0])/3;
			c[2][1] = (2*c[0][1] + 1*c[1][1])/3;
			c[2][2] = (2*c[0][2] + 1*c[1][2])/3;

			c[3][0] = (1*c[0][0] + 2*c[1][0])/3;
			c[3][1] = (1*c[0][1] + 2*c[1][1])/3;
			c[3][2] = (1*c[0][2] + 2*c[1][2])/3;

			/* make index list */
			uint32 indicesint = *((uint32 *) &texelData[j+12]);
			uint8 indices[16];

			for (int32 k = 0; k < 16; k++)
            {
				indices[k] = indicesint & 0x3;
				indicesint >>= 2;
			}

			uint64 alphasint = *((uint64 *) &texelData[j+0]);
			uint8 alphas[16];

			for (int32 k = 0; k < 16; k++)
            {
				alphas[k] = (alphasint & 0xF)*17;
				alphasint >>= 4;
			}

			/* write bytes */
			for (uint32 k = 0; k < 4; k++)
            {
				for (uint32 l = 0; l < 4; l++) {
	// wtf?
	newtexels[(y+l)*width[i]*4 + (x+k)*4 + 0] = c[indices[l*4+k]][0];
	newtexels[(y+l)*width[i]*4 + (x+k)*4 + 1] = c[indices[l*4+k]][1];
	newtexels[(y+l)*width[i]*4 + (x+k)*4 + 2] = c[indices[l*4+k]][2];
	newtexels[(y+l)*width[i]*4 + (x+k)*4 + 3] = alphas[l*4+k];
				}
            }
			x += 4;
			if (x >= width[i])
            {
				y += 4;
				x = 0;
			}
		}
		delete[] texels[i];
		texels[i] = newtexels;
		dataSizes[i] = dataSize;
        this->mipmapDepth[i] = 0x20;
	}
	parent->rasterFormat += 0x0200;
	dxtCompression = 0;
}

void NativeTextureD3D::decompressDxt1(void)
{
	for (uint32 i = 0; i < mipmapCount; i++)
    {
        uint8 *texelData = (uint8*)this->texels[i];

		/* j loops through old texels
		 * x and y loop through new texels */
		uint32 x = 0, y = 0;
		uint32 dataSize = width[i]*height[i]*4;
		uint8 *newtexels = new uint8[dataSize];
		for (uint32 j = 0; j < width[i]*height[i]/2; j += 8)
        {
			/* calculate colors */
			uint32 col0 = *((uint16 *) &texelData[j+0]);
			uint32 col1 = *((uint16 *) &texelData[j+2]);
			uint32 c[4][4];
			// swap r and b
			c[0][0] = (col0 & 0x1F)*0xFF/0x1F;
			c[0][1] = ((col0 & 0x7E0) >> 5)*0xFF/0x3F;
			c[0][2] = ((col0 & 0xF800) >> 11)*0xFF/0x1F;
			c[0][3] = 0xFF;

			c[1][0] = (col1 & 0x1F)*0xFF/0x1F;
			c[1][1] = ((col1 & 0x7E0) >> 5)*0xFF/0x3F;
			c[1][2] = ((col1 & 0xF800) >> 11)*0xFF/0x1F;
			c[1][3] = 0xFF;
			if (col0 > col1)
            {
				c[2][0] = (2*c[0][0] + 1*c[1][0])/3;
				c[2][1] = (2*c[0][1] + 1*c[1][1])/3;
				c[2][2] = (2*c[0][2] + 1*c[1][2])/3;
				c[2][3] = 0xFF;

				c[3][0] = (1*c[0][0] + 2*c[1][0])/3;
				c[3][1] = (1*c[0][1] + 2*c[1][1])/3;
				c[3][2] = (1*c[0][2] + 2*c[1][2])/3;
				c[3][3] = 0xFF;
			}
            else
            {
				c[2][0] = (c[0][0] + c[1][0])/2;
				c[2][1] = (c[0][1] + c[1][1])/2;
				c[2][2] = (c[0][2] + c[1][2])/2;
				c[2][3] = 0xFF;

				c[3][0] = 0x00;
				c[3][1] = 0x00;
				c[3][2] = 0x00;
				if (parent->rasterFormat & 0x0200)
                {
					c[3][3] = 0xFF;
                }
				else // 0x0100
                {
					c[3][3] = 0x00;
                }
			}

			/* make index list */
			uint32 indicesint = *((uint32 *) &texelData[j+4]);
			uint8 indices[16];
			for (int32 k = 0; k < 16; k++)
            {
				indices[k] = indicesint & 0x3;
				indicesint >>= 2;
			}

			/* write bytes */
			for (uint32 k = 0; k < 4; k++)
            {
				for (uint32 l = 0; l < 4; l++)
                {
	// wtf?
	newtexels[(y+l)*width[i]*4 + (x+k)*4 + 0] = c[indices[l*4+k]][0];
	newtexels[(y+l)*width[i]*4 + (x+k)*4 + 1] = c[indices[l*4+k]][1];
	newtexels[(y+l)*width[i]*4 + (x+k)*4 + 2] = c[indices[l*4+k]][2];
	newtexels[(y+l)*width[i]*4 + (x+k)*4 + 3] = c[indices[l*4+k]][3];
				}
            }
			x += 4;
			if (x >= width[i]) {
				y += 4;
				x = 0;
			}
		}
		delete[] texels[i];
		texels[i] = newtexels;
		dataSizes[i] = dataSize;
        this->mipmapDepth[i] = 0x20;
	}
	parent->rasterFormat += 0x0400;
	dxtCompression = 0;
}

void NativeTextureD3D::decompressDxt(void)
{
	if (dxtCompression == 0)
		return;

	if (dxtCompression == 1)
		decompressDxt1();
	else if (dxtCompression == 3)
		decompressDxt3();
	else if (dxtCompression == 4) {
		decompressDxt4();
	} else
        std::cout << "dxt" << dxtCompression << " not supported\n";
}

};