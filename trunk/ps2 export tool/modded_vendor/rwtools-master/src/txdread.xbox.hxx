namespace rw
{

struct NativeTextureXBOX : public PlatformTexture
{
    NativeTextureXBOX( void )
    {
        // Initialize the texture object.
        this->palette = NULL;
        this->paletteSize = 0;
        this->paletteType = PALETTE_NONE;
        this->mipmapCount = 0;
        this->dxtCompression = 0;
        this->hasAlpha = false;
        this->colorOrder = COLOR_BGRA;
        this->rasterType = 4;   // by default it is a texture raster.
        this->autoMipmaps = false;
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

    bool isCompressed( void ) const
    {
        return ( this->dxtCompression != 0 );
    }

    PlatformTexture* Clone( void ) const
    {
        NativeTextureXBOX *newTex = new NativeTextureXBOX();

        // Copy over attributes.
        newTex->width = this->width;
        newTex->height = this->height;
        newTex->mipmapDepth = this->mipmapDepth;
        newTex->dataSizes = this->dataSizes;
        
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
        newTex->dxtCompression = this->dxtCompression;
        newTex->hasAlpha = this->hasAlpha;

        newTex->colorOrder = this->colorOrder;
        newTex->rasterType = this->rasterType;
        newTex->autoMipmaps = this->autoMipmaps;

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

    uint8 rasterType;

    eColorOrdering colorOrder;

    bool hasAlpha;

    bool autoMipmaps;

	// PC/XBOX
	uint32 dxtCompression;
};

#pragma pack(1)
struct textureMetaHeaderStructXbox
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

    uint32 imageDataSectionSize;
};
#pragma pack()

};