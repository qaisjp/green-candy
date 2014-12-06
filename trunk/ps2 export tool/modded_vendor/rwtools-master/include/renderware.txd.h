/*
 * TXDs
 */

struct Texture
{
	uint32 filterFlags;
	std::string name;
	std::string maskName;

	/* Extensions */

	/* sky mipmap */
	bool hasSkyMipmap;

	/* functions */
	void read(std::istream &dff);
	uint32 write(std::ostream &dff);
	void readExtension(std::istream &dff);
	void dump(std::string ind = "");

	Texture(void);
};

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
    uint32 consts1[5];

    uint16 gpuSize1;
    uint16 gpuSize2;

    uint32 consts4[2];

    uint32 width;
    uint32 height;

    uint32 consts2[6];

    uint32 numberOfDoubleWords;

    uint32 consts3[3];
};

inline void writeImageDataHeader(std::ostream& rw, uint32 width, uint32 height, uint32 numberOfBytes, uint16 gpuSize1, uint16 gpuSize2)
{
    textureImageDataHeader imgHeader;

    imgHeader.consts1[0] = 0x00000003;
    imgHeader.consts1[1] = 0x10000000;
    imgHeader.consts1[2] = 0x0000000e;
    imgHeader.consts1[3] = 0x00000000;
    imgHeader.consts1[4] = 0x00000000;

    imgHeader.gpuSize1 = gpuSize1;
    imgHeader.gpuSize2 = gpuSize2;

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

    imgHeader.width = width;
    imgHeader.height = height;
    imgHeader.numberOfDoubleWords = ( numberOfBytes / sizeof(uint32) ) / 4;

    rw.write((char*)&imgHeader, sizeof(imgHeader));
}

inline bool readImageDataHeader(std::istream& rw, uint32& width, uint32& height, uint32& numberOfBytes, uint16& gpuSize1, uint16& gpuSize2)
{
    textureImageDataHeader imgHeader;
    rw.read((char*)&imgHeader, sizeof(imgHeader));

    // Assert some constants.
    bool properConstants =
        ( imgHeader.consts1[0] == 0x00000003 ) &&
        ( imgHeader.consts1[1] == 0x10000000 ) &&
        ( imgHeader.consts1[2] == 0x0000000e ) &&
        ( imgHeader.consts1[3] == 0x00000000 ) &&
        ( imgHeader.consts1[4] == 0x00000000 ) &&

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
        return false;

    width = imgHeader.width;
    height = imgHeader.height;
    numberOfBytes = ( imgHeader.numberOfDoubleWords * 4 ) * sizeof(uint32);

    uint32 gpuReg = *(uint32*)&imgHeader.gpuSize1;

    if ( gpuReg != 0 )
    {
        __asm nop
    }

    gpuSize1 = imgHeader.gpuSize1;
    gpuSize2 = imgHeader.gpuSize2;
    return true;
}

union texFormatInfo
{
    uint32 filterMode : 8;
    uint32 uAddressing : 4;
    uint32 vAddressing : 4;
};

#include "renderware.txd.pixelformat.h"

struct ps2MipmapUnknowns
{
    uint16 unk1, unk2;
};

// Texture container per platform for specialized color data.
struct PlatformTexture abstract
{
    virtual uint32 getWidth( void ) const = 0;
    virtual uint32 getHeight( void ) const = 0;

    virtual uint32 getDepth( void ) const = 0;

    virtual PlatformTexture* Clone( void ) const = 0;

    virtual void Delete( void ) = 0;
};

struct NativeTexture
{
	uint32 platform;
	std::string name;
	std::string maskName;
	uint32 filterFlags;

    uint8 uAddressing : 4;
    uint8 vAddressing : 4;

	uint32 rasterFormat;
	bool hasAlpha;

    // Platform texture type.
    // If it is NULL, then this texture has no platform representation.
    PlatformTexture *platformData;

	/* functions */
	void readD3d(std::istream &txd);
	void readPs2(std::istream &txd);
	void readXbox(std::istream &txd);
	uint32 writeD3d(std::ostream &txd);
    uint32 writePs2(std::ostream &txd);
    void writeTGA(const char *path);

	void convertFromPS2(uint32 aref);
    void convertToPS2(void);
	void convertFromXbox(void);
	void convertToFormat(uint32 format);
    void convertToPalette(uint32 paletteFormat);

	NativeTexture(void);
	NativeTexture(const NativeTexture &orig);
	NativeTexture &operator=(const NativeTexture &that);
	~NativeTexture(void);

    static void StartDebug( void );
    static void DebugParameters( void );
};

struct TextureDictionary
{
    inline TextureDictionary( void )
    {
        this->dataStatus = 0;
    }

    uint16 dataStatus;

	std::vector<NativeTexture> texList;

	/* functions */
	void read(std::istream &txd);
	uint32 write(std::ostream &txd);
	void clear(void);
	~TextureDictionary(void);
};