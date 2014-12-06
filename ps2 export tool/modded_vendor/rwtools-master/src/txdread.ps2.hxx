namespace rw
{

struct NativeTexturePS2 : public PlatformTexture
{
    NativeTexturePS2( void )
    {
        // Initialize the texture object.
        this->palette = NULL;
        this->paletteSize = 0;
        this->mipmapCount = 0;
        this->alphaDistribution = 0;
        this->skyMipMapVal = 4032;

        // Set default values for PS2 GS parameters.
        gsParams.maxMIPLevel = 7;
        gsParams.mtba = 0;
        gsParams.textureFunction = 0;       // MODULATE
        gsParams.lodCalculationModel = 0;   // LOD using formula
        gsParams.mmag = 0;                  // NEAREST
        gsParams.mmin = 0;                  // NEAREST
        gsParams.lodParamL = 0;
        gsParams.lodParamK = 0;

        gsParams.gsTEX1Unknown1 = 0;
        gsParams.gsTEX1Unknown2 = 0;
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

    PlatformTexture* Clone( void ) const
    {
        NativeTexturePS2 *newTex = new NativeTexturePS2();

        // Copy over attributes.
        newTex->width = this->width;
        newTex->height = this->height;
        newTex->mipmapDepth = this->mipmapDepth;
        newTex->dataSizes = this->dataSizes;
        
        // Copy palette information.
        {
	        if (this->palette)
            {
                size_t wholeDataSize = this->paletteSize*4*sizeof(uint8);

		        newTex->palette = new uint8[wholeDataSize];
		        memcpy(newTex->palette, this->palette, wholeDataSize);
	        }
            else
            {
		        newTex->palette = 0;
	        }

            newTex->paletteSize = this->paletteSize;
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
        
        // Copy PS2 data.
        newTex->isSwizzled = this->isSwizzled;
        newTex->swizzleWidth = this->swizzleWidth;
        newTex->swizzleHeight = this->swizzleHeight;
        newTex->mipmapUnknowns = this->mipmapUnknowns;
        newTex->alphaDistribution = this->alphaDistribution;
        newTex->skyMipMapVal = this->skyMipMapVal;
        newTex->gsParams = this->gsParams;

        return newTex;
    }

    // Backlink to original texture container.
    const NativeTexture *parent;

    uint32 mipmapCount;

	std::vector<uint32> width;	// store width & height
	std::vector<uint32> height;	// for each mipmap
    std::vector<uint32> mipmapDepth;
	std::vector<uint32> dataSizes;
	std::vector<void*> texels;	// holds either indices or color values
					// (also per mipmap)
	uint8 *palette;
	uint32 paletteSize;

    std::vector<bool> isSwizzled;
	std::vector<uint32> swizzleWidth;
	std::vector<uint32> swizzleHeight;
    std::vector<ps2MipmapUnknowns> mipmapUnknowns;
	// bit 0: alpha values above (or equal to) the threshold
	// bit 1: alpha values below the threshold
	// both 0: no info
	uint32 alphaDistribution;
    uint32 skyMipMapVal;

    ps2MipmapUnknowns palUnknowns;

    struct gsParams_t
    {
        // Unique PS2 configuration.
        uint8 maxMIPLevel;
        uint8 mtba;
        uint8 textureFunction;
        uint8 lodCalculationModel;
        uint8 mmag;
        uint8 mmin;
        uint8 lodParamL;
        uint8 lodParamK;
        
        // Some undocumented PS2 flags.
        uint8 gsTEX1Unknown1;
        uint8 gsTEX1Unknown2;
    };
    gsParams_t gsParams;

    bool swizzleEncryptPS2(uint32 mip);
	bool swizzleDecryptPS2(uint32 mip);
    uint32 calculateGPUDataSize(uint32 palDataSize) const;
    bool generatePS2GPUData(ps2GSRegisters& gpuData) const;

    void getOptimalGSParameters(gsParams_t& paramsOut) const;
};

}