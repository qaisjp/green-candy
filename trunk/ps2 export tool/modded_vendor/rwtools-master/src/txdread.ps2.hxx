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
            this->qword = 0L;
        }

        union
        {
            struct
            {
                ps2reg_t ssax : 11;
                ps2reg_t pad1 : 5;
                ps2reg_t ssay : 11;
                ps2reg_t pad2 : 5;
                ps2reg_t dsax : 11;
                ps2reg_t pad3 : 5;
                ps2reg_t dsay : 11;
                ps2reg_t dir : 2;
            };
            struct
            {
                ps2reg_t qword;
            };
        };
    };

    struct TRXREG_REG
    {
        inline TRXREG_REG( void )
        {
            this->qword = 0L;
        }

        union
        {
            struct
            {
                ps2reg_t transmissionAreaWidth : 12;
                ps2reg_t pad1 : 20;
                ps2reg_t transmissionAreaHeight : 12;
            };
            struct
            {
                ps2reg_t qword;
            };
        };
    };

    struct TRXDIR_REG
    {
        inline TRXDIR_REG( void )
        {
            this->qword = 0L;
        }

        union
        {
            struct
            {
                ps2reg_t xdir : 2;
            };
            struct
            {
                ps2reg_t qword;
            };
        };
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

    // constant (sky mipmap val)
    // see http://www.gtamodding.com/wiki/Sky_Mipmap_Val_%28RW_Section%29
    uint32 skyMipmapVal;
};

enum eGSRegister
{
    GIF_REG_PRIM,
    GIF_REG_RGBAQ,
    GIF_REG_ST,
    GIF_REG_UV,
    GIF_REG_XYZF2,
    GIF_REG_XYZ2,
    GIF_REG_TEX0_1,
    GIF_REG_TEX0_2,
    GIF_REG_CLAMP_1,
    GIF_REG_CLAMP_2,
    GIF_REG_FOG,
    GIF_REG_XYZF3 = 0x0C,
    GIF_REG_XYZ3,

    GIF_REG_TEX1_1 = 0x14,
    GIF_REG_TEX1_2,
    GIF_REG_TEX2_1,
    GIF_REG_TEX2_2,
    GIF_REG_XYOFFSET_1,
    GIF_REG_XYOFFSET_2,
    GIF_REG_PRMODECONT,
    GIF_REG_PRMODE,
    GIF_REG_TEXCLUT,

    GIF_REG_SCANMSK = 0x22,

    GIF_REG_MIPTBP1_1 = 0x34,
    GIF_REG_MIPTBP1_2,
    GIF_REG_MIPTBP2_1,
    GIF_REG_MIPTBP2_2,

    GIF_REG_TEXA = 0x3B,

    GIF_REG_FOGCOL = 0x3D,

    GIF_REG_TEXFLUSH = 0x3F,
    GIF_REG_SCISSOR_1,
    GIF_REG_SCISSOR_2,
    GIF_REG_ALPHA_1,
    GIF_REG_ALPHA_2,
    GIF_REG_DIMX,
    GIF_REG_DTHE,
    GIF_REG_COLCLAMP,
    GIF_REG_TEST_1,
    GIF_REG_TEST_2,
    GIF_REG_PABE,
    GIF_REG_FBA_1,
    GIF_REG_FBA_2,
    GIF_REG_FRAME_1,
    GIF_REG_FRAME_2,
    GIF_REG_ZBUF_1,
    GIF_REG_ZBUF_2,
    GIF_REG_BITBLTBUF,

    GIF_REG_TRXPOS,
    GIF_REG_TRXREG,
    GIF_REG_TRXDIR,

    GIF_REG_HWREG,

    GIF_REG_SIGNAL,
    GIF_REG_FINISH,
    GIF_REG_LABEL
};

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

enum eFormatEncodingType
{
    FORMAT_UNKNOWN,
    FORMAT_IDTEX4,
    FORMAT_IDTEX8,
    FORMAT_IDTEX8_COMPRESSED,
    FORMAT_TEX16,
    FORMAT_TEX32
};

inline static bool getMemoryLayoutFromTexelFormat(eFormatEncodingType encodingType, eMemoryLayoutType& memLayout)
{
    eMemoryLayoutType theLayout;

    if ( encodingType == FORMAT_IDTEX4 || encodingType == FORMAT_IDTEX8_COMPRESSED )
    {
        theLayout = PSMT4;
    }
    else if ( encodingType == FORMAT_IDTEX8 )
    {
        theLayout = PSMT8;
    }
    else if ( encodingType == FORMAT_TEX16 )
    {
        theLayout = PSMCT16S;
    }
    else if ( encodingType == FORMAT_TEX32 )
    {
        theLayout = PSMCT32;
    }
    else
    {
        return false;
    }

    memLayout = theLayout;

    return true;
}

struct GIFtag
{
    unsigned long long nloop : 15;
    unsigned long long eop : 1;
    unsigned long long pad1 : 30;
    unsigned long long pre : 1;
    unsigned long long prim : 11;
    unsigned long long flg : 2;
    unsigned long long nreg : 4;
    
    unsigned long long regs;

    uint32 getRegisterID(uint32 i) const
    {
        assert(i < 16);

        unsigned long long shiftPos = i * 4;

        return ( this->regs & ( 0xF << shiftPos ) ) >> shiftPos;
    }

    void setRegisterID(uint32 i, uint32 regContent)
    {
        assert(i < 16);

        unsigned long long shiftPos = i * 4;

        this->regs &= ~( 0xF << shiftPos );

        this->regs |= regContent >> shiftPos;
    }
};

inline uint32 getFormatEncodingDepth(eFormatEncodingType encodingType)
{
    uint32 depth = 0;

    if (encodingType == FORMAT_IDTEX4 || encodingType == FORMAT_IDTEX8_COMPRESSED)
    {
        depth = 4;
    }
    else if (encodingType == FORMAT_IDTEX8)
    {
        depth = 8;
    }
    else if (encodingType == FORMAT_TEX16)
    {
        depth = 16;
    }
    else if (encodingType == FORMAT_TEX32)
    {
        depth = 32;
    }

    return depth;
}

struct NativeTexturePS2 : public PlatformTexture
{
    NativeTexturePS2( void )
    {
        // Initialize the texture object.
        this->paletteType = PALETTE_NONE;
        this->autoMipmaps = false;
        this->requiresHeaders = true;
        this->hasSwizzle = false;
        this->skyMipMapVal = 4032;
        this->recommendedBufferBasePointer = 0;

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

        // Texture raster by default.
        this->rasterType = 4;

        this->hasAlpha = true;
    }

    void Delete( void )
    {
        delete this;
    }

    uint32 getWidth( void ) const
    {
        return this->mipmaps[0].width;
    }

    uint32 getHeight( void ) const
    {
        return this->mipmaps[0].height;
    }

    uint32 getDepth( void ) const
    {
        return this->mipmaps[0].depth;
    }

    uint32 getMipmapCount( void ) const
    {
        return this->mipmaps.size();
    }

    ePaletteType getPaletteType( void ) const
    {
        return this->paletteType;
    }

    PlatformTexture* Clone( void ) const
    {
        NativeTexturePS2 *newTex = new NativeTexturePS2();
        
        // Copy palette information.
        newTex->paletteTex = this->paletteTex;
        newTex->paletteType = this->paletteType;

        // Copy image texel information.
        newTex->mipmaps = this->mipmaps;
        
        // Copy PS2 data.
        newTex->autoMipmaps = this->autoMipmaps;
        newTex->requiresHeaders = this->requiresHeaders;
        newTex->hasSwizzle = this->hasSwizzle;
        newTex->skyMipMapVal = this->skyMipMapVal;
        newTex->gsParams = this->gsParams;
        newTex->recommendedBufferBasePointer = this->recommendedBufferBasePointer;

        newTex->rasterType = this->rasterType;

        newTex->hasAlpha = this->hasAlpha;

        return newTex;
    }

    // Backlink to original texture container.
    const NativeTexture *parent;

    struct GSTexture
    {
        inline GSTexture( void )
        {
            this->dataSize = 0;
            this->texels = NULL;

            this->swizzleEncodingType = FORMAT_UNKNOWN;
            this->swizzleWidth = 0;
            this->swizzleHeight = 0;
        }

        inline ~GSTexture( void )
        {
            if ( void *texels = this->texels )
            {
                delete texels;

                this->texels = NULL;
            }
        }

        inline GSTexture( const GSTexture& right )
        {
            *this = right;
        }

        inline void operator = ( const GSTexture& right )
        {
            // Copy over image data.
            void *newTexels = NULL;

            uint32 dataSize = right.dataSize;

            if ( dataSize != 0 )
            {
                const void *srcTexels = right.texels;

                newTexels = new uint8[ dataSize ];

                memcpy( newTexels, srcTexels, dataSize );
            }
            this->texels = newTexels;
            this->dataSize = dataSize;

            // Copy over encoding properties.
            this->swizzleEncodingType = right.swizzleEncodingType;
            this->swizzleWidth = right.swizzleWidth;
            this->swizzleHeight = right.swizzleHeight;
        }

        inline void setGSRegister(eGSRegister regID, unsigned long long regContent)
        {
            // Try to find an existing entry with this register.
            uint32 numRegs = this->storedRegs.size();

            bool hasReplacedReg = false;

            for ( uint32 n = 0; n < numRegs; n++ )
            {
                GSRegInfo& regInfo = this->storedRegs[ n ];

                if ( regInfo.regID == regID )
                {
                    regInfo.content = regContent;
                    
                    hasReplacedReg = true;
                    break;
                }
            }

            if ( !hasReplacedReg )
            {
                GSRegInfo newRegInfo;
                newRegInfo.regID = regID;
                newRegInfo.content = regContent;

                this->storedRegs.push_back( newRegInfo );
            }
        }

        uint32 getDataSize( void ) const
        {
            uint32 encodedTexItems = ( this->swizzleWidth * this->swizzleHeight );

            uint32 encodingDepth = getFormatEncodingDepth(this->swizzleEncodingType);

			return ( encodedTexItems * encodingDepth/8 );
        }

        uint32 getStreamSize( bool requiresHeaders ) const
        {
            uint32 streamSize = 0;

            if ( requiresHeaders )
            {
                streamSize += (
                    sizeof(GIFtag) +
                    this->storedRegs.size() * ( sizeof(unsigned long long) * 2 ) +
                    sizeof(GIFtag)
                );
            }

            streamSize += this->dataSize;

            return streamSize;
        }

        uint32 readGIFPacket(std::istream& rw, bool hasHeaders, bool& corruptedHeaders_out);
        uint32 writeGIFPacket(std::ostream& rw, bool requiresHeaders) const;

        // Members.
        uint32 dataSize;
        void *texels;           // holds either indices or color values

        eFormatEncodingType swizzleEncodingType;
        uint32 swizzleWidth, swizzleHeight;

        struct GSRegInfo
        {
            eGSRegister regID;
            unsigned long long content;
        };

        typedef std::vector <GSRegInfo> regInfoList_t;

        regInfoList_t storedRegs;
    };

    struct GSMipmap : public GSTexture
    {
        inline GSMipmap( void )
        {
            this->width = 0;
            this->height = 0;
            this->depth = 0;
        }

        inline ~GSMipmap( void )
        {
            return;
        }

        inline GSMipmap( const GSMipmap& right )
        {
            *this = right;
        }

        inline void operator = ( const GSMipmap& right )
        {
            // Copy general attributes.
            this->width = right.width;
            this->height = right.height;
            this->depth = right.depth;

            // Copy texture stuff.
            GSTexture::operator = ( right );
        }

        uint32 width, height;   // store width & height for each mipmap
        uint32 depth;
    };

    // mipmaps are GSTextures.
    std::vector <GSMipmap> mipmaps;

    GSTexture paletteTex;

    ePaletteType paletteType;

    uint32 recommendedBufferBasePointer;

    bool requiresHeaders;
    bool hasSwizzle;
    bool autoMipmaps;

    bool hasAlpha;

    uint32 skyMipMapVal;

    uint8 rasterType;

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

    eFormatEncodingType getHardwareRequiredEncoding(uint32 version) const;

    bool swizzleEncryptPS2(uint32 mip);
	bool swizzleDecryptPS2(uint32 mip);

private:
    bool allocateTextureMemoryNative(
        uint32 mipmapBasePointer[], uint32 mipmapBufferWidth[], uint32 mipmapMemorySize[], ps2MipmapTransmissionData mipmapTransData[], uint32 maxMipmaps,
        eMemoryLayoutType& pixelMemLayoutTypeOut,
        uint32& clutBasePointer, uint32& clutMemSize, ps2MipmapTransmissionData& clutTransData,
        uint32& maxBuffHeight
    ) const;

public:
    bool allocateTextureMemory(
        uint32 mipmapBasePointer[], uint32 mipmapBufferWidth[], uint32 mipmapMemorySize[], ps2MipmapTransmissionData mipmapTransData[], uint32 maxMipmaps,
        eMemoryLayoutType& pixelMemLayoutTypeOut,
        uint32& clutBasePointer, uint32& clutMemSize, ps2MipmapTransmissionData& clutTransData
    ) const;

    uint32 calculateGPUDataSize(
        const uint32 mipmapBasePointer[], const uint32 mipmapMemorySize[], uint32 maxMipmaps,
        eMemoryLayoutType memLayoutType,
        uint32 clutBasePointer, uint32 clutMemSize
    ) const;

    bool generatePS2GPUData(
        uint32 gameVersion,
        ps2GSRegisters& gpuData,
        const uint32 mipmapBasePointer[], const uint32 mipmapBufferWidth[], const uint32 mipmapMemorySize[], uint32 maxMipmaps,
        eMemoryLayoutType memLayoutType,
        uint32 clutBasePointer
    ) const;

    void getOptimalGSParameters(gsParams_t& paramsOut) const;

    void PerformDebugChecks(const textureMetaDataHeader& textureMeta) const;

    bool getDebugBitmap( Bitmap& bmpOut ) const;
};

inline eFormatEncodingType getFormatEncodingFromRasterFormat(eRasterFormat rasterFormat, ePaletteType paletteType)
{
    eFormatEncodingType encodingFormat = FORMAT_UNKNOWN;

    if (paletteType == PALETTE_4BIT)
    {
        encodingFormat = FORMAT_IDTEX8_COMPRESSED;
    }
    else if (paletteType == PALETTE_8BIT)
    {
        encodingFormat = FORMAT_IDTEX8;
    }
    else if (rasterFormat == RASTER_LUM8)
    {
        encodingFormat = FORMAT_IDTEX8;
    }
    else if (rasterFormat == RASTER_1555 || rasterFormat == RASTER_565 || rasterFormat == RASTER_4444 ||
             rasterFormat == RASTER_16 || rasterFormat == RASTER_555)
    {
        encodingFormat = FORMAT_TEX16;
    }
    else if (rasterFormat == RASTER_8888 || rasterFormat == RASTER_888 || rasterFormat == RASTER_32)
    {
        encodingFormat = FORMAT_TEX32;
    }

    return encodingFormat;
}

inline eFormatEncodingType getFormatEncodingFromMemoryLayout(eMemoryLayoutType memLayout)
{
    eFormatEncodingType encodingFormat = FORMAT_UNKNOWN;

    if (memLayout == PSMT4)
    {
        encodingFormat = FORMAT_IDTEX8_COMPRESSED;
    }
    else if (memLayout == PSMT8)
    {
        encodingFormat = FORMAT_IDTEX8;
    }
    else if (memLayout == PSMCT16 || memLayout == PSMCT16S)
    {
        encodingFormat = FORMAT_TEX16;
    }
    else if (memLayout == PSMCT32)
    {
        encodingFormat = FORMAT_TEX32;
    }

    return encodingFormat;
}

inline void getPaletteTextureDimensions(ePaletteType paletteType, uint32 version, uint32& width, uint32& height)
{
    if (paletteType == PALETTE_4BIT)
    {
        if (version == rw::GTA3_1 || version == rw::GTA3_2 || version == rw::GTA3_3 || version == rw::GTA3_4)
        {
            width = 8;
            height = 2;
        }
        else
        {
            width = 8;
            height = 3;
        }
    }
    else if (paletteType == PALETTE_8BIT)
    {
        width = 16;
        height = 16;
    }
    else
    {
        assert( 0 );
    }
}

inline void genpalettetexeldata(
    uint32 gameVersion,
    void *paletteData, eRasterFormat rasterFormat, ePaletteType paletteType, uint32 itemCount,
    void*& texelData, uint32& texelDataSize, uint32& palWidth, uint32& palHeight
)
{
    // Select a compatible texture size for our palette data.
    uint32 texelWidth = 0;
    uint32 texelHeight = 0;

    getPaletteTextureDimensions(paletteType, gameVersion, texelWidth, texelHeight);

    // Allocate texture memory.
    uint32 texelItemCount = ( texelWidth * texelHeight );

    // Calculate the data size.
    uint32 palDepth = Bitmap::getRasterFormatDepth(rasterFormat);

    uint32 srcDataSize = itemCount * palDepth / 8;
    uint32 dstDataSize = texelItemCount * palDepth / 8;

    assert( srcDataSize != 0 );
    assert( dstDataSize != 0 );

    void *newTexelData = NULL;

    if ( srcDataSize != dstDataSize )
    {
        newTexelData = new uint8[ dstDataSize ];

        // Write the new memory.
        memcpy(newTexelData, paletteData, srcDataSize);
        
        if (dstDataSize > srcDataSize)
        {
            // Zero out the rest.
            memset((char*)newTexelData + srcDataSize, 0, (dstDataSize - srcDataSize));
        }
    }
    else
    {
        newTexelData = paletteData;
    }

    // Give parameters to the runtime.
    texelData = newTexelData;
    texelDataSize = dstDataSize;
    
    palWidth = texelWidth;
    palHeight = texelHeight;
}

}