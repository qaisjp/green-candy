#include <StdInc.h>

#include "txdread.d3d.hxx"

namespace rw
{

void NativeTexture::readD3d(std::istream &rw)
{
	HeaderInfo header;

	READ_HEADER(CHUNK_STRUCT);
	
    long texNativeStructOff = rw.tellg();

    uint32 texNativeStructSize = header.length;

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

    // Deconstruct the format flags.
    bool hasMipmaps = false;    // TODO: actually use this flag.

	readRasterFormatFlags( metaHeader.rasterFormat, this->rasterFormat, platformTex->paletteType, hasMipmaps, platformTex->autoMipmaps );

	hasAlpha = false;

	if ( platform == PLATFORM_D3D9 )
    {
        D3DFORMAT d3dFormat;

		rw.read((char*)&d3dFormat, sizeof(d3dFormat));

        // Alpha is not decided here.
        platformTex->d3dFormat = d3dFormat;
    }
	else
    {
		hasAlpha = ( readUInt32(rw) != 0 );

        // Set d3dFormat later.
    }

    textureMetaHeaderStructDimInfo dimInfo;
    rw.read((char*)&dimInfo, sizeof(dimInfo));

	uint32 depth = dimInfo.depth;
	uint32 maybeMipmapCount = dimInfo.mipmapCount;

    assert( dimInfo.rasterType == 4 );  // TODO

    platformTex->rasterType = dimInfo.rasterType;

	if ( platform == PLATFORM_D3D9 )
    {
        // Here we decide about alpha.
        textureContentInfoStruct contentInfo;
        rw.read((char*)&contentInfo, sizeof(contentInfo));

		this->hasAlpha = contentInfo.hasAlpha;
        platformTex->isCubeTexture = contentInfo.isCubeTexture;
        platformTex->autoMipmaps = contentInfo.autoMipMaps;

		if ( contentInfo.isCompressed )
        {
			// Detect FOUR-CC versions for compression method.
            if ( platformTex->d3dFormat == D3DFMT_DXT1 )
            {
                platformTex->dxtCompression = 1;
            }
            else if ( platformTex->d3dFormat == D3DFMT_DXT2 )
            {
                platformTex->dxtCompression = 2;
            }
            else if ( platformTex->d3dFormat == D3DFMT_DXT3 )
            {
                platformTex->dxtCompression = 3;
            }
            else if ( platformTex->d3dFormat == D3DFMT_DXT4 )
            {
                platformTex->dxtCompression = 4;
            }
            else if ( platformTex->d3dFormat == D3DFMT_DXT5 )
            {
                platformTex->dxtCompression = 5;
            }
            else
            {
                assert( 0 );
            }
        }
		else
        {
			platformTex->dxtCompression = 0;
        }
    }
    else
    {
        uint32 dxtInfo = readUInt8(rw);

        platformTex->dxtCompression = dxtInfo;

        // Auto-decide the Direct3D format.
        D3DFORMAT d3dFormat = D3DFMT_A8R8G8B8;

        if ( dxtInfo != 0 )
        {
            if ( dxtInfo == 1 )
            {
                d3dFormat = D3DFMT_DXT1;
            }
            else if ( dxtInfo == 2 )
            {
                d3dFormat = D3DFMT_DXT2;
            }
            else if ( dxtInfo == 3 )
            {
                d3dFormat = D3DFMT_DXT3;
            }
            else if ( dxtInfo == 4 )
            {
                d3dFormat = D3DFMT_DXT4;
            }
            else if ( dxtInfo == 5 )
            {
                d3dFormat = D3DFMT_DXT5;
            }
            else
            {
                assert( 0 );
            }
        }
        else
        {
            eRasterFormat paletteRasterType = this->rasterFormat;

            d3dFormat = getD3DFormatFromRasterType( paletteRasterType );
        }

        platformTex->d3dFormat = d3dFormat;
    }

	if (platformTex->paletteType != PALETTE_NONE)
    {
		platformTex->paletteSize = getPaletteItemCount( platformTex->paletteType );

        size_t paletteDataSize = platformTex->paletteSize * sizeof(uint32);

		platformTex->palette = new uint8[paletteDataSize];
		rw.read(reinterpret_cast <char *> (platformTex->palette), paletteDataSize);
	}

    uint32 currentMipWidth = dimInfo.width;
    uint32 currentMipHeight = dimInfo.height;

    uint32 mipmapCount = 0;

	for (uint32 i = 0; i < maybeMipmapCount; i++)
    {
		if (i > 0)
        {
            currentMipWidth /= 2;
            currentMipHeight /= 2;
        }

        if (currentMipWidth == 0 || currentMipHeight == 0)
        {
            break;
        }

        uint32 texWidth = currentMipWidth;
        uint32 texHeight = currentMipHeight;

        // Process dimensions.
        {
			// DXT compression works on 4x4 blocks,
			// no smaller values allowed
			if (platformTex->dxtCompression)
            {
				if (texWidth < 4 && texWidth != 0)
                {
					texWidth = 4;
                }
				if (texHeight < 4 && texHeight != 0)
                {
					texHeight = 4;
                }
			}
        }

		uint32 texDataSize = readUInt32(rw);

        if ( texDataSize == 0 )
        {
            break;
        }

        uint8 *texelData = new uint8[texDataSize];

		rw.read(reinterpret_cast <char *> (texelData), texDataSize*sizeof(uint8));

        // Store mipmap properties.
		platformTex->width.push_back( texWidth );
		platformTex->height.push_back( texHeight );

        platformTex->mipmapDepth.push_back( depth );

		platformTex->dataSizes.push_back(texDataSize);

        // Store the image data pointer.
		platformTex->texels.push_back(texelData);

        mipmapCount++;
    }
    assert( mipmapCount != 0 );

    platformTex->mipmapCount = mipmapCount;

    // Make sure we go to the end of the texture native struct.
    {
        uint32 curStructOffset = rw.tellg() - texNativeStructOff;

        if ( curStructOffset != texNativeStructSize )
        {
            rw.seekg( texNativeStructOff + texNativeStructSize, std::ios::beg );
        }
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
    this->d3dFormat = D3DFMT_A8R8G8B8;
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
	parent->rasterFormat = RASTER_8888;
    this->d3dFormat = D3DFMT_A8R8G8B8;
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
				if (parent->rasterFormat == RASTER_565)
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
    
    if ( parent->rasterFormat == RASTER_1555 )
    {
        parent->rasterFormat = RASTER_8888;

        this->d3dFormat = D3DFMT_A8R8G8B8;
    }
    else if ( parent->rasterFormat == RASTER_565 )
    {
        parent->rasterFormat = RASTER_888;

        this->d3dFormat = D3DFMT_X8R8G8B8;
    }
    else
    {
        assert( 0 );
    }

	dxtCompression = 0;
}

void NativeTextureD3D::decompressDxt(void)
{
	if (dxtCompression == 0)
		return;

	if (dxtCompression == 1)
    {
		decompressDxt1();
    }
	else if (dxtCompression == 3)
    {
		decompressDxt3();
    }
	else if (dxtCompression == 4)
    {
		decompressDxt4();
	}
    else
    {
        std::cout << "dxt" << dxtCompression << " not supported\n";
    }
}

};