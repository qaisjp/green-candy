#include <cstring>
#include <cstdlib>
#include <fstream>
#include <StdInc.h>
#include <assert.h>
#include <bitset>
#define _USE_MATH_DEFINES
#include <math.h>
#include <map>
#include <algorithm>
#include <cmath>

#include "pixelformat.hxx"

#include "txdread.d3d.hxx"

namespace rw
{

/*
 * Texture Dictionary
 */

void TextureDictionary::read(std::istream &rw)
{
	HeaderInfo header;

	header.read(rw);
	if (header.getType() != CHUNK_TEXDICTIONARY)
		return;

    uint32 texDictSize = header.getLength();

	READ_HEADER(CHUNK_STRUCT);

    // Read the header block depending on version.
    LibraryVersion libVer = header.getVersion();

    uint32 textureCount = 0;
    bool requiresRecommendedPlatform = true;

    if (libVer.rwLibMinor <= 5)
    {
        textureCount = readUInt32(rw);
    }
    else
    {
        textureCount = readUInt16(rw);
        uint16 recommendedPlatform = readUInt16(rw);

        // So if there is a recommended platform set, we will also give it one if we will write it.
        requiresRecommendedPlatform = ( recommendedPlatform != 0 );
    }
    this->hasRecommendedPlatform = requiresRecommendedPlatform;

    // Read the textures and store them.
	texList.resize(textureCount);

	for (uint32 i = 0; i < textureCount; i++)
    {
		READ_HEADER(CHUNK_TEXTURENATIVE);

        uint32 texNativeSize = header.getLength();

        rw.seekg(0x0c, std::ios::cur);
		texList[i].platform = readUInt32(rw);
		rw.seekg(-0x10, std::ios::cur);

		if (texList[i].platform == PLATFORM_XBOX)
        {
			texList[i].readXbox(rw);
		}
        else if (texList[i].platform == PLATFORM_D3D8 ||
		         texList[i].platform == PLATFORM_D3D9)
        {
			texList[i].readD3d(rw);
		}
        else if (texList[i].platform == PLATFORM_PS2FOURCC)
        {
			texList[i].platform = PLATFORM_PS2;
			texList[i].readPs2(rw);
		}
        else
        {
            throw RwException( "unknown platform" );
        }

		READ_HEADER(CHUNK_EXTENSION);
		long end = header.getLength();
		end += rw.tellg();
		while (rw.tellg() < end)
        {
			header.read(rw);
			switch (header.getType())
            {
			case CHUNK_SKYMIPMAP:
				rw.seekg(4, std::ios::cur);
				break;
			default:
				rw.seekg(header.getLength(), std::ios::cur);
				break;
			}
		}
	}

    // Read extensions.
    READ_HEADER(CHUNK_EXTENSION);
    long end = header.getLength();
    end += rw.tellg();
    
    while ( rw.tellg() < end )
    {
        header.read(rw);

        rw.seekg(header.getLength(), std::ios::cur);
    }
}

void TextureDictionary::clear(void)
{
	texList.clear();
}

TextureDictionary::~TextureDictionary(void)
{
	texList.clear();
}

/*
 * Native Texture
 */

void NativeTexture::convertToFormat(eRasterFormat newFormat)
{
    if (this->platform != PLATFORM_D3D8 && this->platform != PLATFORM_D3D9)
        return;

    NativeTextureD3D *platformTex = (NativeTextureD3D*)this->platformData;

    // If the texture is DXT compressed, uncompress it.
    if ( platformTex->dxtCompression )
    {
        platformTex->decompressDxt();
    }

    eRasterFormat rasterFormat = this->rasterFormat;
    ePaletteType paletteType = platformTex->paletteType;
    eColorOrdering colorOrder = platformTex->colorOrdering;
    uint32 mipmapCount = platformTex->mipmapCount;
    uint32 srcDepth = platformTex->depth;

    bool isPaletteRaster = ( paletteType != PALETTE_NONE );

    if ( isPaletteRaster || newFormat != rasterFormat )
    {
        void *paletteData = platformTex->palette;
        uint32 maxpalette = platformTex->paletteSize;

        uint32 newDepth = Bitmap::getRasterFormatDepth(newFormat);

	    for (uint32 j = 0; j < mipmapCount; j++)
        {
            uint32 dataSize = 0;

            // Decide about output data.
            uint32 colorIndiceCount = platformTex->width[j] * platformTex->height[j];

            dataSize = getRasterDataSize(colorIndiceCount, newDepth);

            {
	            void *newtexels = new uint8[dataSize];

                void *texelSource = platformTex->texels[j];
                uint32 srcWidth = platformTex->width[j];
                uint32 srcHeight = platformTex->height[j];

                for (uint32 y = 0; y < srcHeight; y++)
                {
                    for (uint32 x = 0; x < srcWidth; x++)
                    {
                        // Calculate the combined color texture index.
                        uint32 colorIndex = PixelFormat::coord2index(x, y, srcWidth);

                        // Grab the color values.
                        uint8 red, green, blue, alpha;
                        bool hasColor = browsetexelcolor(texelSource, paletteType, paletteData, maxpalette, colorIndex, rasterFormat, colorOrder, srcDepth, red, green, blue, alpha);

                        if ( !hasColor )
                        {
                            red = 0;
                            green = 0;
                            blue = 0;
                            alpha = 0;
                        }

                        // Write the color data.
                        puttexelcolor(newtexels, colorIndex, newFormat, colorOrder, newDepth, red, green, blue, alpha);
                    }
                }

                // Write the texture data.
	            delete[] texelSource;
	            platformTex->texels[j] = newtexels;
	            platformTex->dataSizes[j] = dataSize;
            }
	    }

        // Update the depth.
        platformTex->depth = newDepth;

        // Update the format.
        this->rasterFormat = newFormat;

        // Make sure we update the d3dFormat.
        {
            D3DFORMAT newD3DFormat;

            bool hasD3DFormat = getD3DFormatFromRasterType( newFormat, PALETTE_NONE, colorOrder, newDepth, newD3DFormat );

            if ( hasD3DFormat )
            {
                platformTex->d3dFormat = newD3DFormat;
            }

            platformTex->hasD3DFormat = hasD3DFormat;
        }

        // Delete unnecessary palette data.
	    if (isPaletteRaster)
        {
		    delete[] paletteData;
		    platformTex->palette = NULL;

            platformTex->paletteSize = 0;
		    
            platformTex->paletteType = PALETTE_NONE;
	    }

        // Since we most likely changed colors, recalculate the alpha flag.
        platformTex->hasAlpha = platformTex->doesHaveAlpha();
    }
}

Bitmap NativeTexture::getBitmap(void) const
{
    Bitmap resultBitmap;

    // If the texture is Direct3D, we can easily get the texels.
    if ( this->platform == PLATFORM_D3D8 || this->platform == PLATFORM_D3D9 )
    {
        NativeTextureD3D *platformTex = (NativeTextureD3D*)this->platformData;

        // If the texture is compressed, we must decompress it.
        if ( platformTex->dxtCompression )
        {
            platformTex->decompressDxt();
        }

        // If it has any mipmap at all.
        if ( platformTex->mipmapCount > 0 )
        {
            uint32 width = platformTex->width[ 0 ];
            uint32 height = platformTex->height[ 0 ];
            uint32 depth = platformTex->depth;
            uint32 dataSize = platformTex->dataSizes[ 0 ];
            eRasterFormat theFormat = this->rasterFormat;
            eColorOrdering theOrder = platformTex->colorOrdering;
            
            // Get the color data.
            bool hasAllocatedNewPixelData = false;
            void *pixelData = NULL;

            // If the image is palletized, it has to be converted to raw colors.
            if ( platformTex->paletteType != PALETTE_NONE )
            {
                hasAllocatedNewPixelData = true;

                uint32 rasterFormatDepth = Bitmap::getRasterFormatDepth(theFormat);

                uint32 texelCount = ( width * height );

                uint32 newDataSize = getRasterDataSize(texelCount, rasterFormatDepth);

                pixelData = new uint8[ newDataSize ];

                // Unpack the texels.
                void *texelSource = platformTex->texels[ 0 ];
                void *palData = platformTex->palette;
                uint32 palSize = platformTex->paletteSize;

                for ( uint32 n = 0; n < texelCount; n++ )
                {
                    uint8 r, g, b, a;
                    browsetexelcolor(texelSource, platformTex->paletteType, palData, palSize, n, theFormat, theOrder, depth, r, g, b, a);

                    puttexelcolor(pixelData, n, theFormat, theOrder, rasterFormatDepth, r, g, b, a);
                }

                // Update bitmap parameters.
                depth = rasterFormatDepth;
                dataSize = newDataSize;
            }
            else
            {
                pixelData = platformTex->texels[ 0 ];
            }

            if ( pixelData != NULL )
            {
                // Set the data into the bitmap.
                resultBitmap.setImageData(
                    pixelData, theFormat, theOrder, depth, width, height, dataSize
                );

                if ( hasAllocatedNewPixelData )
                {
                    delete [] pixelData;
                }
            }
        }
    }

    return resultBitmap;
}

void NativeTexture::setImageData(const Bitmap& srcImage)
{
    // If the texture is Direct3D, we can easily set the texels.
    if ( this->platform == PLATFORM_D3D8 || this->platform == PLATFORM_D3D9 )
    {
        NativeTextureD3D *platformTex = (NativeTextureD3D*)this->platformData;

        // Delete old image data.
        {
            uint32 mipmapCount = platformTex->mipmapCount;

            for ( uint32 n = 0; n < mipmapCount; n++ )
            {
                void *texels = platformTex->texels[ n ];

                delete [] texels;
            }

            // If we have palette data, delete it.
            if ( void *paletteTexels = platformTex->palette )
            {
                delete [] paletteTexels;

                platformTex->palette = NULL;
            }
            platformTex->paletteType = PALETTE_NONE;
            platformTex->paletteSize = 0;

            // Since we have no more mipmaps, we also dont have automatically generated mipmaps.
            platformTex->autoMipmaps = false;
        }

        // Resize mipmap containers.
        platformTex->width.resize( 1 );
        platformTex->height.resize( 1 );
        platformTex->texels.resize( 1 );
        platformTex->dataSizes.resize( 1 );

        platformTex->mipmapCount = 1;

        // Set the new texel data.
        uint32 newWidth, newHeight;
        uint32 newDepth = srcImage.getDepth();
        eRasterFormat newFormat = srcImage.getFormat();
        eColorOrdering newColorOrdering = srcImage.getColorOrder();
        uint32 newDataSize = srcImage.getDataSize();

        srcImage.getSize( newWidth, newHeight );

        platformTex->width[ 0 ] = newWidth;
        platformTex->height[ 0 ] = newHeight;
        platformTex->depth = newDepth;
        platformTex->texels[ 0 ] = srcImage.copyPixelData();
        platformTex->dataSizes[ 0 ] = newDataSize;

        platformTex->colorOrdering = newColorOrdering;

        // Update generics.
        {
            D3DFORMAT newD3DFormat;

            bool hasD3DFormat = getD3DFormatFromRasterType(newFormat, PALETTE_NONE, newColorOrdering, newDepth, newD3DFormat);

            if (hasD3DFormat)
            {
                platformTex->d3dFormat = newD3DFormat;
            }
            platformTex->hasD3DFormat = hasD3DFormat;
        }

        this->rasterFormat = newFormat;

        // We need to update the alpha flag.
        platformTex->hasAlpha = platformTex->doesHaveAlpha();
    }
}

void NativeTexture::resize(uint32 width, uint32 height)
{
    Bitmap mainBitmap = this->getBitmap();

    Bitmap targetBitmap( mainBitmap.getDepth(), mainBitmap.getFormat(), mainBitmap.getColorOrder() );

    targetBitmap.setSize(width, height);

    targetBitmap.drawBitmap(
        mainBitmap, 0, 0, width, height,
        Bitmap::SHADE_ZERO, Bitmap::SHADE_ONE, Bitmap::BLEND_ADDITIVE
    );

    this->setImageData( targetBitmap );
}

void NativeTexture::getSize(uint32& width, uint32& height) const
{
    PlatformTexture *platformTex = this->platformData;

    width = platformTex->getWidth();
    height = platformTex->getHeight();
}

void NativeTexture::newDirect3D(void)
{
    if ( this->platform != 0 )
        return;

    // Create new platform data.
    NativeTextureD3D *d3dtex = new NativeTextureD3D();

    // Create the backlink.
    d3dtex->parent = this;

    this->platformData = d3dtex;
    this->platform = PLATFORM_D3D9;
}

void NativeTexture::optimizeForLowEnd(float quality)
{
    if (this->platform != PLATFORM_D3D8 && this->platform != PLATFORM_D3D9)
        return;

    NativeTextureD3D *d3dtex = (NativeTextureD3D*)this->platformData;

    // Textures that should run on low end hardware should not be too HD.
    // This routine takes the PlayStation 2 as reference hardware.

    const uint32 maxTexWidth = 256;
    const uint32 maxTexHeight = 256;

    while ( true )
    {
        // Get properties of this texture first.
        uint32 texWidth, texHeight;

        texWidth = d3dtex->getWidth();
        texHeight = d3dtex->getHeight();

        // Optimize this texture.
        bool hasTakenStep = false;

        if ( !hasTakenStep && quality < 1.0f )
        {
            // We first must make sure that the texture is not unnecessaringly huge.
            if ( texWidth > maxTexWidth || texHeight > maxTexHeight )
            {
                // Half the texture size until we are at a suitable size.
                uint32 targetWidth = texWidth;
                uint32 targetHeight = texHeight;

                do
                {
                    targetWidth /= 2;
                    targetHeight /= 2;
                }
                while ( targetWidth > maxTexWidth || targetHeight > maxTexHeight );

                // The texture dimensions are too wide.
                // We half the texture in size.
                this->resize( targetWidth, targetHeight );

                hasTakenStep = true;
            }
        }
        
        if ( !hasTakenStep )
        {
            // We should decrease the texture size by palettization.
            if (d3dtex->paletteType == PALETTE_NONE)
            {
                ePaletteType targetPalette = ( quality > 0.0f ) ? ( PALETTE_8BIT ) : ( PALETTE_4BIT );

                if (d3dtex->hasAlpha == false)
                {
                    // 4bit palette is only feasible for non-alpha textures (at higher quality settings).
                    // Otherwise counting the colors is too complicated.
                    
                    // TODO: still allow 8bit textures.
                    targetPalette = PALETTE_4BIT;
                }

                // The texture should be palettized for good measure.
                this->convertToPalette( targetPalette );

                // TODO: decide whether 4bit or 8bit palette.

                hasTakenStep = true;
            }
        }

        if ( !hasTakenStep )
        {
            // The texture dimension is important for renderer performance. That is why we need to scale the texture according
            // to quality settings aswell.

        }
    
        if ( !hasTakenStep )
        {
            break;
        }
    }
}

bool NativeTexture::convertToDirect3D8(void)
{
    // Get the platform and convert it.
    uint32 thisPlatform = this->platform;

    bool canConvert = false;

    if (thisPlatform == PLATFORM_PS2)
    {
        this->convertFromPS2();

        canConvert = true;
    }
    else if (thisPlatform == PLATFORM_XBOX)
    {
        this->convertFromXbox();

        canConvert = true;
    }
    else if (thisPlatform == PLATFORM_D3D9)
    {
        // We need to downgrade this texture.
        canConvert = true;
    }
    else if (thisPlatform == PLATFORM_D3D8)
    {
        return true;
    }

    // Now that we are in Direct3D format, we need to target a specific version.
    bool successful = false;

    if (canConvert)
    {
        thisPlatform = this->platform;

        if (thisPlatform != PLATFORM_D3D8)
        {
            // Update it.
            this->platform = PLATFORM_D3D8;
        }

        successful = true;
    }

    return successful;
}

bool NativeTexture::convertToDirect3D9(void)
{
    // Get the platform and convert it.
    uint32 thisPlatform = this->platform;

    bool canConvert = false;

    if (thisPlatform == PLATFORM_PS2)
    {
        this->convertFromPS2();

        canConvert = true;
    }
    else if (thisPlatform == PLATFORM_XBOX)
    {
        this->convertFromXbox();

        canConvert = true;
    }
    else if (thisPlatform == PLATFORM_D3D8)
    {
        // We need to downgrade this texture.
        canConvert = true;
    }
    else if (thisPlatform == PLATFORM_D3D9)
    {
        return true;
    }

    // Now that we are in Direct3D format, we need to target a specific version.
    bool successful = false;

    if (canConvert)
    {
        thisPlatform = this->platform;

        if (thisPlatform != PLATFORM_D3D9)
        {
            // Update it.
            this->platform = PLATFORM_D3D9;
        }

        successful = true;
    }

    return successful;
}

bool NativeTexture::isDirect3DWritable(void) const
{
    bool isWritable = false;

    if (this->platform == PLATFORM_D3D8 || this->platform == PLATFORM_D3D9)
    {
        NativeTextureD3D *platformTex = (NativeTextureD3D*)this->platformData;

        // We can write the texture if it has a Direct3D format.
        if (platformTex->hasD3DFormat == true)
        {
            isWritable = true;
        }
    }

    return isWritable;
}

void NativeTexture::makeDirect3DCompatible(void)
{
    // Only works if the texture is in Direct3D platform already.

    if (this->platform == PLATFORM_D3D8 || this->platform == PLATFORM_D3D9)
    {
        NativeTextureD3D *platformTex = (NativeTextureD3D*)this->platformData;

        // Make sure this texture has a Direct3D format.
        if ( !platformTex->hasD3DFormat )
        {
            // TODO.
        }
    }
}

#pragma pack(1)
struct TgaHeader
{
    BYTE IDLength;        /* 00h  Size of Image ID field */
    BYTE ColorMapType;    /* 01h  Color map type */
    BYTE ImageType;       /* 02h  Image type code */
    WORD CMapStart;       /* 03h  Color map origin */
    WORD CMapLength;      /* 05h  Color map length */
    BYTE CMapDepth;       /* 07h  Depth of color map entries */
    WORD XOffset;         /* 08h  X origin of image */
    WORD YOffset;         /* 0Ah  Y origin of image */
    WORD Width;           /* 0Ch  Width of image */
    WORD Height;          /* 0Eh  Height of image */
    BYTE PixelDepth;      /* 10h  Image pixel size */

    struct
    {
        unsigned char numAttrBits : 4;
        unsigned char imageOrdering : 2;
        unsigned char reserved : 2;
    } ImageDescriptor; /* 11h  Image descriptor byte */
};
#pragma pack()

static void writeTGAPixels(
    const void *texelSource, uint32 colorUnitCount,
    eRasterFormat srcRasterFormat, uint32 srcItemDepth, ePaletteType srcPaletteType, const void *srcPaletteData, uint32 srcMaxPalette,
    eRasterFormat dstRasterFormat, uint32 dstItemDepth,
    eColorOrdering srcColorOrder,
    std::ostream& tga
)
{
    // Check whether we need to create a TGA compatible color array.
    if ( srcRasterFormat != dstRasterFormat || srcItemDepth != dstItemDepth || srcPaletteType != PALETTE_NONE || srcColorOrder != COLOR_BGRA )
    {
        // Get the data size.
        uint32 texelDataSize = getRasterDataSize(colorUnitCount, dstItemDepth);

        // Allocate a buffer for the fixed pixel data.
        void *tgaColors = new uint8[ texelDataSize ];

        for ( uint32 n = 0; n < colorUnitCount; n++ )
        {
            // Grab the color.
            uint8 red, green, blue, alpha;

            browsetexelcolor(texelSource, srcPaletteType, srcPaletteData, srcMaxPalette, n, srcRasterFormat, srcColorOrder, srcItemDepth, red, green, blue, alpha);

            // Write it with correct ordering.
            puttexelcolor(tgaColors, n, dstRasterFormat, COLOR_BGRA, dstItemDepth, red, green, blue, alpha);
        }

        tga.write((const char*)tgaColors, texelDataSize);

        // Free memory.
        delete [] tgaColors;
    }
    else
    {
        // Simply write the color source.
        uint32 texelDataSize = getRasterDataSize(colorUnitCount, srcItemDepth);

        tga.write((const char*)texelSource, texelDataSize);
    }
}

void NativeTexture::writeTGA(const char *path, bool optimized)
{
    // If optimized == true, then this routine will output files in a commonly unsupported format
    // that is much smaller than in regular mode.
    // Problem with TGA is that it is poorly supported by most of the applications out there.

    if ( this->platform != PLATFORM_D3D8 && this->platform != PLATFORM_D3D9 )
        return;

    NativeTextureD3D *platformTex = (NativeTextureD3D*)this->platformData;

    // If the texture is compressed, decompress it.
    if ( platformTex->dxtCompression )
    {
        platformTex->decompressDxt();
    }

	std::ofstream tga(path, std::ios::binary);
	if (tga.fail())
    {
        rw::rwInterface.PushWarning( "not writing file: " + std::string( path ) );
		return;
	}

    // Decide how to write the raster.
    eRasterFormat srcRasterFormat = this->rasterFormat;
    ePaletteType srcPaletteType = platformTex->paletteType;
    uint32 srcItemDepth = platformTex->depth;

    eRasterFormat dstRasterFormat;
    uint32 dstColorDepth;
    uint32 dstAlphaBits;
    bool hasDstRasterFormat = false;

    ePaletteType dstPaletteType;

    if ( !optimized )
    {
        // We output in a format that every parser should understand.
        dstRasterFormat = RASTER_8888;
        dstColorDepth = 32;
        dstAlphaBits = 8;

        dstPaletteType = PALETTE_NONE;

        hasDstRasterFormat = true;
    }
    else
    {
        if ( srcRasterFormat == RASTER_1555 )
        {
            dstRasterFormat = RASTER_1555;
            dstColorDepth = 16;
            dstAlphaBits = 1;

            hasDstRasterFormat = true;
        }
        else if ( srcRasterFormat == RASTER_565 )
        {
            dstRasterFormat = RASTER_565;
            dstColorDepth = 16;
            dstAlphaBits = 0;

            hasDstRasterFormat = true;
        }
        else if ( srcRasterFormat == RASTER_4444 )
        {
            dstRasterFormat = RASTER_4444;
            dstColorDepth = 16;
            dstAlphaBits = 4;

            hasDstRasterFormat = true;
        }
        else if ( srcRasterFormat == RASTER_8888 ||
                  srcRasterFormat == RASTER_888 )
        {
            dstRasterFormat = RASTER_8888;
            dstColorDepth = 32;
            dstAlphaBits = 8;

            hasDstRasterFormat = true;
        }
        else if ( srcRasterFormat == RASTER_555 )
        {
            dstRasterFormat = RASTER_555;
            dstColorDepth = 15;
            dstAlphaBits = 0;

            hasDstRasterFormat = true;
        }

        // We palettize if present.
        dstPaletteType = srcPaletteType;
    }

    if ( !hasDstRasterFormat )
    {
        rw::rwInterface.PushWarning( "could not find a raster format to write TGA image with" );
        return;
    }

    // Prepare the TGA header.
    TgaHeader header;

    uint32 maxpalette = platformTex->paletteSize;

    bool isPalette = (dstPaletteType != PALETTE_NONE);

    header.IDLength = 0;
    header.ColorMapType = ( isPalette ? 1 : 0 );
    header.ImageType = ( isPalette ? 1 : 2 );

    // The pixel depth is the number of bits a color entry is going to take (real RGBA color).
    uint32 pixelDepth = 0;

    if (isPalette)
    {
        pixelDepth = Bitmap::getRasterFormatDepth(dstRasterFormat);
    }
    else
    {
        pixelDepth = dstColorDepth;
    }
    
    header.CMapStart = 0;
    header.CMapLength = ( isPalette ? maxpalette : 0 );
    header.CMapDepth = ( isPalette ? pixelDepth : 0 );

    header.XOffset = 0;
    header.YOffset = 0;

    uint32 width = platformTex->width[0];
    uint32 height = platformTex->height[0];

    header.Width = width;
    header.Height = height;
    header.PixelDepth = ( isPalette ? srcItemDepth : dstColorDepth );

    header.ImageDescriptor.numAttrBits = dstAlphaBits;
    header.ImageDescriptor.imageOrdering = 2;
    header.ImageDescriptor.reserved = 0;

    // Write the header.
    tga.write((const char*)&header, sizeof(header));

    void *texelSource = platformTex->texels[0];
    void *paletteData = platformTex->palette;
    eColorOrdering colorOrder = platformTex->colorOrdering;

    // Write the palette if we require.
    if (isPalette)
    {
        writeTGAPixels(
            paletteData, maxpalette,
            srcRasterFormat, pixelDepth, PALETTE_NONE, NULL, 0,
            dstRasterFormat, pixelDepth,
            colorOrder, tga
        );
    }

    // Now write image information.
    // If we are a palette, we simply write the color indice.
    uint32 colorUnitCount = ( width * height );

    if (isPalette)
    {
        assert( srcPaletteType != PALETTE_NONE );

        // Write a fixed version of the palette indice.
        uint32 texelDataSize = platformTex->dataSizes[ 0 ]; // for palette items its the same size.

        void *fixedPalItems = new uint8[ texelDataSize ];

        for (uint32 n = 0; n < colorUnitCount; n++)
        {
            uint8 palIndex;

            bool couldGetPalIndex = getpaletteindex(texelSource, srcPaletteType, maxpalette, srcItemDepth, n, palIndex);

            if ( !couldGetPalIndex )
            {
                palIndex = 0;
            }

            // Put the palette index.
            if (srcItemDepth == 4)
            {
                ( (PixelFormat::palette4bit*)fixedPalItems )->setvalue(n, palIndex);
            }
            else if (srcItemDepth == 8)
            {
                ( (PixelFormat::palette8bit*)fixedPalItems )->setvalue(n, palIndex);
            }
            else
            {
                assert( 0 );
            }
        }

        tga.write((const char*)fixedPalItems, texelDataSize);

        // Clean up memory.
        delete [] fixedPalItems;
    }
    else
    {
        writeTGAPixels(
            texelSource, colorUnitCount,
            srcRasterFormat, srcItemDepth, srcPaletteType, paletteData, maxpalette,
            dstRasterFormat, dstColorDepth,
            colorOrder, tga
        );
    }

	tga.close();
}

NativeTexture::NativeTexture(void)
: platform(0), name(""), maskName(""), filterFlags(0), uAddressing(1), vAddressing(1), rasterFormat(rw::RASTER_DEFAULT)
{
    this->platformData = NULL;
}

NativeTexture::NativeTexture(const NativeTexture &orig)
: platform(orig.platform),
  name(orig.name),
  maskName(orig.maskName),
  filterFlags(orig.filterFlags),
  uAddressing(orig.uAddressing),
  vAddressing(orig.vAddressing),
  rasterFormat(orig.rasterFormat)
{
	PlatformTexture *platformTex = NULL;

    if ( orig.platformData )
    {
        platformTex = orig.platformData->Clone();
    }

    this->platformData = platformTex;
}

NativeTexture &NativeTexture::operator=(const NativeTexture &that)
{
	if (this != &that)
    {
		platform = that.platform;
		name = that.name;
		maskName = that.maskName;
		filterFlags = that.filterFlags;
        uAddressing = that.uAddressing;
        vAddressing = that.vAddressing;
		rasterFormat = that.rasterFormat;

	    PlatformTexture *platformTex = NULL;

        if ( that.platformData )
        {
            platformTex = that.platformData->Clone();
        }

        this->platformData = platformTex;
	}
	return *this;
}

NativeTexture::~NativeTexture(void)
{
    if ( this->platformData )
    {
        this->platformData->Delete();

        this->platformData = NULL;
    }
}

}
