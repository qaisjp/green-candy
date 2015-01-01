#include <d3d9.h>

namespace rw
{

inline D3DFORMAT getD3DFormatFromRasterType(eRasterFormat paletteRasterType)
{
    D3DFORMAT d3dFormat = D3DFMT_A8R8G8B8;

    if ( paletteRasterType == RASTER_1555 )
    {
        d3dFormat = D3DFMT_A1R5G5B5;
    }
    else if ( paletteRasterType == RASTER_565 )
    {
        d3dFormat = D3DFMT_R5G6B5;
    }
    else if ( paletteRasterType == RASTER_4444 )
    {
        d3dFormat = D3DFMT_A4R4G4B4;
    }
    else if ( paletteRasterType == RASTER_LUM8 )
    {
        d3dFormat = D3DFMT_L8;
    }
    else if ( paletteRasterType == RASTER_8888 )
    {
        d3dFormat = D3DFMT_A8R8G8B8;
    }
    else if ( paletteRasterType == RASTER_888 )
    {
        d3dFormat = D3DFMT_X8R8G8B8;
    }
    else if ( paletteRasterType == RASTER_16 )
    {
        // not sure.
        d3dFormat = D3DFMT_L16;
    }
    else if ( paletteRasterType == RASTER_24 )
    {
        // not sure.
        d3dFormat = D3DFMT_R8G8B8;
    }
    else if ( paletteRasterType == RASTER_32 )
    {
        // not sure.
        d3dFormat = D3DFMT_X8R8G8B8;
    }
    else if ( paletteRasterType == RASTER_555 )
    {
        d3dFormat = D3DFMT_X1R5G5B5;
    }
    else
    {
        // unknown format.
        assert( 0 );
    }

    return d3dFormat;
}

struct NativeTextureD3D : public PlatformTexture
{
    NativeTextureD3D( void )
    {
        // Initialize the texture object.
        this->palette = NULL;
        this->paletteSize = 0;
        this->paletteType = PALETTE_NONE;
        this->mipmapCount = 0;
        this->isCubeTexture = false;
        this->autoMipmaps = false;
        this->d3dFormat = D3DFMT_A8R8G8B8;
        this->dxtCompression = 0;
        this->rasterType = 4;
        this->hasAlpha = true;
        this->colorOrdering = COLOR_BGRA;
    }

    void Delete( void )
    {
        if ( this->palette )
        {
	        delete[] palette;

	        palette = NULL;
        }
	    for (uint32 i = 0; i < texels.size(); i++)
        {
		    delete[] texels[i];
		    texels[i] = 0;
	    }

        delete this;
    }

    uint32 getWidth( void ) const
    {
        return this->width[0];
    }

    uint32 getHeight( void ) const
    {
        return this->height[0];
    }

    uint32 getDepth( void ) const
    {
        return this->mipmapDepth[0];
    }

    uint32 getMipmapCount( void ) const
    {
        return this->mipmapCount;
    }

    ePaletteType getPaletteType( void ) const
    {
        return this->paletteType;
    }

    PlatformTexture* Clone( void ) const
    {
        NativeTextureD3D *newTex = new NativeTextureD3D();

        // Copy palette information.
        {
	        if (this->palette)
            {
                size_t wholeDataSize = this->paletteSize * sizeof(uint32);

		        newTex->palette = new uint8[wholeDataSize];
		        memcpy(newTex->palette, this->palette, wholeDataSize);
	        }
            else
            {
		        newTex->palette = 0;
	        }

            newTex->paletteSize = this->paletteSize;
            newTex->paletteType = this->paletteType;
        }

        // Copy image texel information.
        {
            size_t numTexels = this->texels.size();

	        for (uint32 i = 0; i < numTexels; i++)
            {
		        uint32 dataSize = this->dataSizes[i];
		        uint8 *newtexels = new uint8[dataSize];

                const uint8 *srctexels = (const uint8*)this->texels[i];

		        memcpy(newtexels, srctexels, dataSize);

		        newTex->texels.push_back(newtexels);
	        }
        }

        newTex->mipmapCount = this->mipmapCount;
        newTex->isCubeTexture = this->isCubeTexture;
        newTex->autoMipmaps = this->autoMipmaps;
        newTex->d3dFormat = this->d3dFormat;
        newTex->dxtCompression = this->dxtCompression;
        newTex->rasterType = this->rasterType;
        newTex->hasAlpha = this->hasAlpha;
        newTex->colorOrdering = this->colorOrdering;

        return newTex;
    }

    // Backlink to original texture container.
    NativeTexture *parent;

	uint32 mipmapCount;

	std::vector<uint32> width;	// store width & height
	std::vector<uint32> height;	// for each mipmap
    std::vector<uint32> mipmapDepth;
	std::vector<uint32> dataSizes;
	std::vector<void*> texels;	// holds either indices or color values
					// (also per mipmap)
	void *palette;
	uint32 paletteSize;

    ePaletteType paletteType;

	// PC/XBOX
    bool isCubeTexture;
    bool autoMipmaps;

    D3DFORMAT d3dFormat;
    uint32 dxtCompression;
    uint32 rasterType;

    bool hasAlpha;

    eColorOrdering colorOrdering;

	void decompressDxt(void);
	void decompressDxt1(void);
	void decompressDxt3(void);
	void decompressDxt4(void);

    // Check whether this texture has alpha.
    // Use this to update/calculate the alpha flag when required.
    bool doesHaveAlpha(void) const;
};

#pragma pack(1)
struct textureMetaHeaderStructGeneric
{
    rw::texFormatInfo texFormat;
    
    char name[32];
    char maskName[32];

    uint32 rasterFormat;
};

struct textureMetaHeaderStructDimInfo
{
    uint16 width;
    uint16 height;
    uint8 depth;
    uint8 mipmapCount;
    uint8 rasterType;
};

struct textureContentInfoStruct
{
    uint8 hasAlpha : 1;
    uint8 isCubeTexture : 1;
    uint8 autoMipMaps : 1;
    uint8 isCompressed : 1;
};
#pragma pack()

};