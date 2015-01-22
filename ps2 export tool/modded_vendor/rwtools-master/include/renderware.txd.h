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

// I do not know why I did this a union!
// I must have been drunk or something.
struct texFormatInfo
{
    uint32 filterMode : 8;
    uint32 uAddressing : 4;
    uint32 vAddressing : 4;
    uint32 pad1 : 16;
};

#include "renderware.txd.pixelformat.h"

struct ps2MipmapTransmissionData
{
    uint16 destX, destY;
};

enum eRasterFormat
{
    RASTER_DEFAULT,
    RASTER_1555,
    RASTER_565,
    RASTER_4444,
    RASTER_LUM8,
    RASTER_8888,
    RASTER_888,
    RASTER_16,
    RASTER_24,
    RASTER_32,
    RASTER_555
};

inline bool isValidRasterFormat(eRasterFormat rasterFormat)
{
    bool isValidRaster = false;

    if (rasterFormat == RASTER_1555 ||
        rasterFormat == RASTER_565 ||
        rasterFormat == RASTER_4444 ||
        rasterFormat == RASTER_LUM8 ||
        rasterFormat == RASTER_8888 ||
        rasterFormat == RASTER_888 ||
        rasterFormat == RASTER_555)
    {
        isValidRaster = true;
    }

    return isValidRaster;
}

enum ePaletteType
{
    PALETTE_NONE,
    PALETTE_4BIT,
    PALETTE_8BIT
};

enum eColorOrdering
{
    COLOR_RGBA,
    COLOR_BGRA
};

// utility to calculate palette item count.
inline uint32 getPaletteItemCount( ePaletteType paletteType )
{
    uint32 count = 0;

    if ( paletteType == PALETTE_4BIT )
    {
        count = 16;
    }
    else if ( paletteType == PALETTE_8BIT )
    {
        count = 256;
    }
    else if ( paletteType == PALETTE_NONE )
    {
        count = 0;
    }
    else
    {
        assert( 0 );
    }

    return count;
}

#include "renderware.bmp.h"

// Texture container per platform for specialized color data.
struct PlatformTexture abstract
{
    virtual uint32 getWidth( void ) const = 0;
    virtual uint32 getHeight( void ) const = 0;

    virtual uint32 getDepth( void ) const = 0;

    virtual uint32 getMipmapCount( void ) const = 0;

    virtual ePaletteType getPaletteType( void ) const = 0;

    virtual bool isCompressed( void ) const = 0;

    virtual PlatformTexture* Clone( void ) const = 0;

    // Virtual function that can be overwritten so that the
    // runtime can obtain a debug image that displays internals.
    virtual bool getDebugBitmap( Bitmap& bmpOut ) const
    {
        return false;
    }

    virtual void Delete( void ) = 0;
};

// Useful routine to generate generic raster format flags.
inline uint32 generateRasterFormatFlags( eRasterFormat rasterFormat, ePaletteType paletteType, bool hasMipmaps, bool autoMipmaps )
{
    uint32 rasterFlags = 0;

    rasterFlags |= ( (uint32)rasterFormat << 8 );

    if ( paletteType == PALETTE_4BIT )
    {
        rasterFlags |= RASTER_PAL4;
    }
    else if ( paletteType == PALETTE_8BIT )
    {
        rasterFlags |= RASTER_PAL8;
    }

    if ( hasMipmaps )
    {
        rasterFlags |= RASTER_MIPMAP;
    }

    if ( autoMipmaps )
    {
        rasterFlags |= RASTER_AUTOMIPMAP;
    }

    return rasterFlags;
}

// Useful routine to read generic raster format flags.
inline void readRasterFormatFlags( uint32 rasterFormatFlags, eRasterFormat& rasterFormat, ePaletteType& paletteType, bool& hasMipmaps, bool& autoMipmaps )
{
    rasterFormat = (eRasterFormat)( ( rasterFormatFlags & 0xF00 ) >> 8 );
    
    if ( ( rasterFormatFlags & RASTER_PAL4 ) != 0 )
    {
        paletteType = PALETTE_4BIT;
    }
    else if ( ( rasterFormatFlags & RASTER_PAL8 ) != 0 )
    {
        paletteType = PALETTE_8BIT;
    }
    else
    {
        paletteType = PALETTE_NONE;
    }

    hasMipmaps = ( rasterFormatFlags & RASTER_MIPMAP ) != 0;
    autoMipmaps = ( rasterFormatFlags & RASTER_AUTOMIPMAP ) != 0;
}

struct NativeTexture
{
	uint32 platform;
	std::string name;
	std::string maskName;
	uint32 filterFlags;

    uint8 uAddressing : 4;
    uint8 vAddressing : 4;

    eRasterFormat rasterFormat;

    // Platform texture type.
    // If it is NULL, then this texture has no platform representation.
    PlatformTexture *platformData;

	/* functions */
	void readD3d(std::istream &txd);
	void readPs2(std::istream &txd);
	void readXbox(std::istream &txd);
	uint32 writeD3d(std::ostream &txd);
    uint32 writePs2(std::ostream &txd);
    uint32 writeXbox(std::ostream &txd);
    void writeTGA(const char *path);

    Bitmap getBitmap(void) const;
    void setImageData(const Bitmap& srcImage);

    void resize(uint32 width, uint32 height);

    void getSize(uint32& width, uint32& height) const;

    void newDirect3D(void);

	void convertFromPS2(void);
    void convertToPS2(void);
	void convertFromXbox(void);
    void convertToXbox(void);
	void convertToFormat(eRasterFormat format);
    void convertToPalette(ePaletteType paletteFormat);

    // Optimization routines.
    void optimizeForLowEnd(float quality);

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
        this->hasRecommendedPlatform = false;
    }

    bool hasRecommendedPlatform;

	std::vector<NativeTexture> texList;

	/* functions */
	void read(std::istream &txd);
	uint32 write(std::ostream &txd);
	void clear(void);
	~TextureDictionary(void);
};