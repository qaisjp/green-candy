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

    ePaletteType getPaletteType( void ) const
    {
        return this->paletteType;
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
	uint8 *palette;
	uint32 paletteSize;

    ePaletteType paletteType;

	// PC/XBOX
	uint32 dxtCompression;
};

};