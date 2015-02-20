#include <d3d9.h>

namespace rw
{

inline bool getD3DFormatFromRasterType(eRasterFormat paletteRasterType, ePaletteType paletteType, eColorOrdering colorOrder, uint32 itemDepth, D3DFORMAT& d3dFormat)
{
    bool hasFormat = false;

    if ( paletteType != PALETTE_NONE )
    {
        if ( itemDepth == 8 )
        {
            if (colorOrder == COLOR_RGBA)
            {
                d3dFormat = D3DFMT_P8;

                hasFormat = true;
            }
        }
    }
    else
    {
        if ( paletteRasterType == RASTER_1555 )
        {
            if ( itemDepth == 16 )
            {
                if (colorOrder == COLOR_BGRA)
                {
                    d3dFormat = D3DFMT_A1R5G5B5;

                    hasFormat = true;
                }
            }
        }
        else if ( paletteRasterType == RASTER_565 )
        {
            if ( itemDepth == 16 )
            {
                if (colorOrder == COLOR_BGRA)
                {
                    d3dFormat = D3DFMT_R5G6B5;

                    hasFormat = true;
                }
            }
        }
        else if ( paletteRasterType == RASTER_4444 )
        {
            if ( itemDepth == 16 )
            {
                if (colorOrder == COLOR_BGRA)
                {
                    d3dFormat = D3DFMT_A4R4G4B4;

                    hasFormat = true;
                }
            }
        }
        else if ( paletteRasterType == RASTER_LUM8 )
        {
            if ( itemDepth == 8 )
            {
                d3dFormat = D3DFMT_L8;

                hasFormat = true;
            }
        }
        else if ( paletteRasterType == RASTER_8888 )
        {
            if ( itemDepth == 32 )
            {
                if (colorOrder == COLOR_BGRA)
                {
                    d3dFormat = D3DFMT_A8R8G8B8;

                    hasFormat = true;
                }
                else if (colorOrder == COLOR_RGBA)
                {
                    d3dFormat = D3DFMT_A8B8G8R8;

                    hasFormat = true;
                }
            }
        }
        else if ( paletteRasterType == RASTER_888 )
        {
            if (colorOrder == COLOR_BGRA)
            {
                if ( itemDepth == 32 )
                {
                    d3dFormat = D3DFMT_X8R8G8B8;

                    hasFormat = true;
                }
                else if ( itemDepth == 24 )
                {
                    d3dFormat = D3DFMT_R8G8B8;

                    hasFormat = true;
                }
            }
            else if (colorOrder == COLOR_RGBA)
            {
                if ( itemDepth == 32 )
                {
                    d3dFormat = D3DFMT_X8B8G8R8;

                    hasFormat = true;
                }
            }
        }
        else if ( paletteRasterType == RASTER_555 )
        {
            if ( itemDepth == 16 )
            {
                if (colorOrder == COLOR_BGRA)
                {
                    d3dFormat = D3DFMT_X1R5G5B5;

                    hasFormat = true;
                }
            }
        }
    }

    return hasFormat;
}

// This data can be returned by the NativeTextureD3D texture.
// It will contain newly allocated texels to be used in a bitmap.
struct rawBitmapFetchResult
{
    void *texelData;
    uint32 dataSize;
    uint32 width, height;
    bool isNewlyAllocated;
    uint32 depth;
    eRasterFormat rasterFormat;
    eColorOrdering colorOrder;
    const void *paletteData;
    uint32 paletteSize;
    ePaletteType paletteType;
};

struct NativeTextureD3D : public PlatformTexture
{
    NativeTextureD3D( void )
    {
        // Initialize the texture object.
        this->palette = NULL;
        this->paletteSize = 0;
        this->paletteType = PALETTE_NONE;
        this->mipmapCount = 0;
        this->depth = 0;
        this->isCubeTexture = false;
        this->autoMipmaps = false;
        this->d3dFormat = D3DFMT_A8R8G8B8;
        this->hasD3DFormat = true;
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
        return this->depth;
    }

    uint32 getMipmapCount( void ) const
    {
        return this->mipmapCount;
    }

    ePaletteType getPaletteType( void ) const
    {
        return this->paletteType;
    }

    bool isCompressed( void ) const
    {
        return ( this->dxtCompression != 0 );
    }

    void compress( float quality )
    {
        // Compress it with DXT.
        // If the compression quality is above or equal to 1.0, we want to allow DXT5 compression when alpha is there.
        uint32 dxtType = 1;

        if ( this->hasAlpha )
        {
            if ( quality >= 1.0f )
            {
                dxtType = 5;
            }
            else
            {
                dxtType = 3;
            }
        }

        // This may not be the optimal routine for compressing to DXT.
        // A different method should be used if more accuracy is required.
        compressDxt( dxtType );
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

            newTex->width = this->width;
            newTex->height = this->height;
            newTex->dataSizes = this->dataSizes;

            newTex->depth = this->depth;
        }

        newTex->mipmapCount = this->mipmapCount;
        newTex->isCubeTexture = this->isCubeTexture;
        newTex->autoMipmaps = this->autoMipmaps;
        newTex->d3dFormat = this->d3dFormat;
        newTex->hasD3DFormat = this->hasD3DFormat;
        newTex->dxtCompression = this->dxtCompression;
        newTex->rasterType = this->rasterType;
        newTex->hasAlpha = this->hasAlpha;
        newTex->colorOrdering = this->colorOrdering;

        return newTex;
    }

    // Function to return a raw bitmap from this texture.
    bool getRawBitmap( uint32 mipLayer, bool allowPalette, rawBitmapFetchResult& bitmapOut ) const;

    // Backlink to original texture container.
    NativeTexture *parent;

	uint32 mipmapCount;

    uint32 depth;

	std::vector<uint32> width;	// store width & height
	std::vector<uint32> height;	// for each mipmap
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

    bool hasD3DFormat;

    inline void updateD3DFormat( void )
    {
        // Execute it whenever the rasterFormat, the palette type, the color order or depth may change.
        D3DFORMAT newD3DFormat;

        bool hasD3DFormat = getD3DFormatFromRasterType( parent->rasterFormat, this->paletteType, this->colorOrdering, this->depth, newD3DFormat );

        if ( hasD3DFormat )
        {
            this->d3dFormat = newD3DFormat;
        }
        this->hasD3DFormat = hasD3DFormat;
    }

    bool hasAlpha;

    eColorOrdering colorOrdering;

    void compressDxt(uint32 dxtType);
	void decompressDxt(void);
	bool decompressDxtNative(uint32 dxtType);

    // Check whether this texture has alpha.
    // Use this to update/calculate the alpha flag when required.
    bool doesHaveAlpha(void) const;

    // Debug stuff.
    bool getDebugBitmap( Bitmap& bmpOut ) const;
};

inline uint32 getDXTRasterDataSize(uint32 dxtType, uint32 texUnitCount)
{
    uint32 texBlockCount = texUnitCount / 16;

    uint32 blockSize = 0;

    if (dxtType == 1)
    {
        blockSize = 8;
    }
    else if (dxtType == 2 || dxtType == 3 ||
             dxtType == 4 || dxtType == 5)
    {
        blockSize = 16;
    }

    return ( texBlockCount * blockSize );
}

inline uint32 getD3DPaletteCount(ePaletteType paletteType)
{
    uint32 reqPalCount = 0;

    if (paletteType == PALETTE_4BIT)
    {
        reqPalCount = 32;
    }
    else if (paletteType == PALETTE_8BIT)
    {
        reqPalCount = 256;
    }
    else
    {
        assert( 0 );
    }

    return reqPalCount;
}

#pragma pack(1)
struct textureMetaHeaderStructGeneric
{
    uint32 platformDescriptor;

    rw::texFormatInfo texFormat;
    
    char name[32];
    char maskName[32];

    uint32 rasterFormat;
};

struct textureMetaHeaderStructDimInfo
{
    uint16 width;               // 0
    uint16 height;              // 2
    uint8 depth;                // 4
    uint8 mipmapCount;          // 5
    uint8 rasterType : 3;       // 6
    uint8 pad1 : 5;
};

struct textureContentInfoStruct
{
    uint8 hasAlpha : 1;
    uint8 isCubeTexture : 1;
    uint8 autoMipMaps : 1;
    uint8 isCompressed : 1;
    uint8 pad : 4;
};
#pragma pack()

};