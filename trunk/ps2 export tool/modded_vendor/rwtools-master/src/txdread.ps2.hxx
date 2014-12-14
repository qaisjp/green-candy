namespace rw
{

struct ps2GSRegisters
{
    typedef unsigned long long ps2reg_t;

    struct TEX0_REG
    {
        inline TEX0_REG( void )
        {
            *(ps2reg_t*)this = 0L;
        }

        ps2reg_t textureBasePointer : 14;
        ps2reg_t textureBufferWidth : 6;
        ps2reg_t pixelStorageFormat : 6;
        ps2reg_t textureWidthLog2 : 4;
        ps2reg_t textureHeightLog2 : 4;
        ps2reg_t texColorComponent : 1;
        ps2reg_t texFunction : 2;
        ps2reg_t clutBufferBase : 14;
        ps2reg_t clutStorageFmt : 4;
        ps2reg_t clutMode : 1;
        ps2reg_t clutEntryOffset : 5;
        ps2reg_t clutLoadControl : 3;

        inline bool operator ==( const TEX0_REG& right ) const
        {
            return ( *(ps2reg_t*)this == *(ps2reg_t*)&right );
        }

        inline bool operator !=( const TEX0_REG& right ) const
        {
            return !( *this == right );
        }
    };

    struct TEX1_REG
    {
        inline TEX1_REG( void )
        {
            *(ps2reg_t*)this = 0L;
        }

        ps2reg_t lodCalculationModel : 1;
        ps2reg_t unknown2 : 1;
        ps2reg_t maximumMIPLevel : 3;
        ps2reg_t mmag : 1;
        ps2reg_t mmin : 3;
        ps2reg_t mtba : 1;
        ps2reg_t unknown : 1;
        ps2reg_t unused2 : 8;
        ps2reg_t lodParamL : 2;
        ps2reg_t unused3 : 11;
        ps2reg_t lodParamK : 12;

        inline bool operator ==( const TEX1_REG& right ) const
        {
#if 0
            return
                this->lodCalculationModel == right.lodCalculationModel &&
                this->maximumMIPLevel == right.maximumMIPLevel &&
                this->mmag == right.mmag &&
                this->mmin == right.mmin &&
                this->mtba == right.mtba &&
                this->lodParamL == right.lodParamL &&
                this->lodParamK == right.lodParamK &&
                this->unknown == right.unknown &&
                this->unknown2 == right.unknown2;
#else
            return ( *(ps2reg_t*)this == *(ps2reg_t*)&right );
#endif
        }

        inline bool operator !=( const TEX1_REG& right ) const
        {
            return !( *this == right );
        }
    };

    struct MIPTBP1_REG
    {
        inline MIPTBP1_REG( void )
        {
            *(ps2reg_t*)this = 0L;
        }

        ps2reg_t textureBasePointer1 : 14;
        ps2reg_t textureBufferWidth1 : 6;
        ps2reg_t textureBasePointer2 : 14;
        ps2reg_t textureBufferWidth2 : 6;
        ps2reg_t textureBasePointer3 : 14;
        ps2reg_t textureBufferWidth3 : 6;

        inline bool operator ==( const MIPTBP1_REG& right ) const
        {
#if 0
            return
                this->textureBasePointer1 == right.textureBasePointer1 &&
                this->textureBufferWidth1 == right.textureBufferWidth1 &&
                this->textureBasePointer2 == right.textureBasePointer2 &&
                this->textureBufferWidth2 == right.textureBufferWidth2 &&
                this->textureBasePointer3 == right.textureBasePointer3 &&
                this->textureBufferWidth3 == right.textureBufferWidth3;
#else
            return ( *(ps2reg_t*)this == *(ps2reg_t*)&right );
#endif
        }

        inline bool operator !=( const MIPTBP1_REG& right ) const
        {
            return !( *this == right );
        }
    };

    struct MIPTBP2_REG
    {
        inline MIPTBP2_REG( void )
        {
            *(ps2reg_t*)this = 0L;
        }

        ps2reg_t textureBasePointer4 : 14;
        ps2reg_t textureBufferWidth4 : 6;
        ps2reg_t textureBasePointer5 : 14;
        ps2reg_t textureBufferWidth5 : 6;
        ps2reg_t textureBasePointer6 : 14;
        ps2reg_t textureBufferWidth6 : 6;

        inline bool operator ==( const MIPTBP2_REG& right ) const
        {
#if 0
            return
                this->textureBasePointer4 == right.textureBasePointer4 &&
                this->textureBufferWidth4 == right.textureBufferWidth4 &&
                this->textureBasePointer5 == right.textureBasePointer5 &&
                this->textureBufferWidth5 == right.textureBufferWidth5 &&
                this->textureBasePointer6 == right.textureBasePointer6 &&
                this->textureBufferWidth6 == right.textureBufferWidth6;
#else
            return ( *(ps2reg_t*)this == *(ps2reg_t*)&right );
#endif
        }

        inline bool operator !=( const MIPTBP2_REG& right ) const
        {
            return !( *this == right );
        }
    };

    struct TRXPOS_REG
    {
        inline TRXPOS_REG( void )
        {
            *(ps2reg_t*)this = 0L;
        }

        ps2reg_t ssax : 11;
        ps2reg_t pad1 : 5;
        ps2reg_t ssay : 11;
        ps2reg_t pad2 : 5;
        ps2reg_t dsax : 11;
        ps2reg_t pad3 : 5;
        ps2reg_t dsay : 11;
        ps2reg_t dir : 2;
    };

    struct TRXREG_REG
    {
        inline TRXREG_REG( void )
        {
            *(ps2reg_t*)this = 0L;
        }

        ps2reg_t transmissionAreaWidth : 12;
        ps2reg_t pad1 : 20;
        ps2reg_t transmissionAreaHeight : 12;
    };

    TEX0_REG tex0;
    TEX1_REG tex1;

    MIPTBP1_REG miptbp1;
    MIPTBP2_REG miptbp2;
};

struct textureMetaDataHeader
{
    uint32 width;
    uint32 height;
    uint32 depth;
    uint32 rasterFormat;

    ps2GSRegisters::TEX0_REG tex0;
    ps2GSRegisters::TEX1_REG tex1;

    ps2GSRegisters::MIPTBP1_REG miptbp1;
    ps2GSRegisters::MIPTBP2_REG miptbp2;

    uint32 dataSize;	// texels + header
    uint32 paletteDataSize; // palette + header + unknowns

    uint32 combinedGPUDataSize;
    uint32 skyMipmapVal;			// constant (sky mipmap val)
};

struct textureImageDataHeader
{
    uint32 consts1[4];

    ps2GSRegisters::TRXPOS_REG trxpos;

    uint32 consts4[2];

    ps2GSRegisters::TRXREG_REG trxreg;

    uint32 consts2[6];

    uint32 numberOfDoubleWords;

    uint32 consts3[3];
};

inline void writeImageDataHeader(std::ostream& rw, uint32 width, uint32 height, uint32 numberOfBytes, uint16 gpuTexOffX, uint16 gpuTexOffY)
{
    textureImageDataHeader imgHeader;

    imgHeader.consts1[0] = 0x00000003;
    imgHeader.consts1[1] = 0x10000000;
    imgHeader.consts1[2] = 0x0000000e;
    imgHeader.consts1[3] = 0x00000000;

    // Set TRXPOS register.
    imgHeader.trxpos.ssax = 0;
    imgHeader.trxpos.ssay = 0;
    imgHeader.trxpos.dsax = gpuTexOffX;
    imgHeader.trxpos.dsay = gpuTexOffY;
    imgHeader.trxpos.dir = 0;

    imgHeader.consts4[0] = 0x00000051;
    imgHeader.consts4[1] = 0x00000000;

    imgHeader.consts2[0] = 0x00000052;
    imgHeader.consts2[1] = 0x00000000;
    imgHeader.consts2[2] = 0x00000000;
    imgHeader.consts2[3] = 0x00000000;
    imgHeader.consts2[4] = 0x00000053;
    imgHeader.consts2[5] = 0x00000000;

    imgHeader.consts3[0] = 0x08000000;
    imgHeader.consts3[1] = 0x00000000;
    imgHeader.consts3[2] = 0x00000000;

    imgHeader.trxreg.transmissionAreaWidth = width;
    imgHeader.trxreg.transmissionAreaHeight = height;
    imgHeader.numberOfDoubleWords = ( numberOfBytes / sizeof(uint32) ) / 4;

    rw.write((const char*)&imgHeader, sizeof(imgHeader));
}

inline bool readImageDataHeader(std::istream& rw, uint32& width, uint32& height, uint32& numberOfBytes, uint16& gpuTexOffX, uint16& gpuTexOffY)
{
    textureImageDataHeader imgHeader;
    rw.read((char*)&imgHeader, sizeof(imgHeader));

    // Assert some constants.
    bool properConstants =
        ( imgHeader.consts1[0] == 0x00000003 ) &&
        ( imgHeader.consts1[1] == 0x10000000 ) &&
        ( imgHeader.consts1[2] == 0x0000000e ) &&
        ( imgHeader.consts1[3] == 0x00000000 ) &&

        ( imgHeader.trxpos.ssax == 0 ) &&
        ( imgHeader.trxpos.ssay == 0 ) &&
        ( imgHeader.trxpos.dir == 0 ) &&

        ( imgHeader.consts4[0] == 0x00000051 ) &&
        ( imgHeader.consts4[1] == 0x00000000 ) &&

        ( imgHeader.consts2[0] == 0x00000052 ) &&
        ( imgHeader.consts2[1] == 0x00000000 ) &&
        ( imgHeader.consts2[2] == 0x00000000 ) &&
        ( imgHeader.consts2[3] == 0x00000000 ) &&
        ( imgHeader.consts2[4] == 0x00000053 ) &&
        ( imgHeader.consts2[5] == 0x00000000 ) &&

        ( imgHeader.consts3[0] == 0x08000000 ) &&
        ( imgHeader.consts3[1] == 0x00000000 ) &&
        ( imgHeader.consts3[2] == 0x00000000 );

    if ( !properConstants )
    {
        // Check for a very special header.
        properConstants =
            ( imgHeader.consts1[0] == 0x03010000 );

        if ( !properConstants )
        {
            return false;
        }

        // We need to dispatch very special.
        imgHeader.trxreg.transmissionAreaWidth = 0x10;
        imgHeader.trxreg.transmissionAreaHeight = 0x10;
    }

    width = imgHeader.trxreg.transmissionAreaWidth;
    height = imgHeader.trxreg.transmissionAreaHeight;
    numberOfBytes = ( imgHeader.numberOfDoubleWords * 4 ) * sizeof(uint32);

#if 0
    uint64 unk1 = *(uint64*)&imgHeader;
    uint64 unk2 = *(uint64*)( (char*)&imgHeader + 8 );
    uint64 unk3 = *(uint64*)( (char*)&imgHeader + 16 );
    uint64 unk4 = *(uint64*)( (char*)&imgHeader + 24 );
    uint64 unk5 = *(uint64*)( (char*)&imgHeader + 32 );

    if ( unk3 != 0 )
    {
        __asm nop
    }
#endif

    gpuTexOffX = imgHeader.trxpos.dsax;
    gpuTexOffY = imgHeader.trxpos.dsay;
    return true;
}

enum eMemoryLayoutType
{
    PSMCT32 = 0,
    PSMCT24,
    PSMCT16,

    PSMCT16S = 10,

    PSMT8 = 19,
    PSMT4,

    PSMT8H = 27,
    PSMT4HL = 36,
    PSMT4HH = 44,

    PSMZ32 = 48,
    PSMZ24,
    PSMZ16,

    PSMZ16S = 58
};

struct NativeTexturePS2 : public PlatformTexture
{
    NativeTexturePS2( void )
    {
        // Initialize the texture object.
        this->palette = NULL;
        this->paletteSize = 0;
        this->paletteType = PALETTE_NONE;
        this->mipmapCount = 0;
        this->autoMipmaps = false;
        this->requiresHeaders = true;
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

        // Whatever those are.
        gsParams.gsTEX1Unknown1 = 0;
        gsParams.gsTEX1Unknown2 = 0;

        // And whatever that is.
        this->unknownFormatFlags = 0x04;
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

            newTex->palUnknowns = this->palUnknowns;
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
        newTex->autoMipmaps = this->autoMipmaps;
        newTex->requiresHeaders = this->requiresHeaders;
        newTex->alphaDistribution = this->alphaDistribution;
        newTex->skyMipMapVal = this->skyMipMapVal;
        newTex->gsParams = this->gsParams;

        newTex->unknownFormatFlags = this->unknownFormatFlags;

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

    ePaletteType paletteType;

    bool requiresHeaders;
    bool autoMipmaps;

    std::vector<bool> isSwizzled;
	std::vector<uint32> swizzleWidth;
	std::vector<uint32> swizzleHeight;
	// bit 0: alpha values above (or equal to) the threshold
	// bit 1: alpha values below the threshold
	// both 0: no info
	uint32 alphaDistribution;
    uint32 skyMipMapVal;

    uint8 unknownFormatFlags;

    ps2MipmapTransmissionData palUnknowns;

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

    bool allocateTextureMemory(
        uint32 mipmapBasePointer[], uint32 mipmapBufferWidth[], uint32 mipmapMemorySize[], ps2MipmapTransmissionData mipmapTransData[], uint32 maxMipmaps,
        eMemoryLayoutType& memLayoutType
    ) const;

    uint32 calculateGPUDataSize(
        const uint32 mipmapBasePointer[], const uint32 mipmapMemorySize[], uint32 maxMipmaps,
        eMemoryLayoutType memLayoutType,
        uint32 paletteDataSize
    ) const;

    bool generatePS2GPUData(
        ps2GSRegisters& gpuData,
        const uint32 mipmapBasePointer[], const uint32 mipmapBufferWidth[], const uint32 mipmapMemorySize[], uint32 maxMipmaps,
        eMemoryLayoutType memLayoutType
    ) const;

    void getOptimalGSParameters(gsParams_t& paramsOut) const;

    void PerformDebugChecks( void ) const;

    bool getDebugBitmap( Bitmap& bmpOut ) const;
};

}