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

using namespace std;

namespace rw {

/*
 * Texture Dictionary
 */

void TextureDictionary::read(istream &rw)
{
	HeaderInfo header;

	header.read(rw);
	if (header.type != CHUNK_TEXDICTIONARY)
		return;

    uint32 texDictSize = header.length;

	READ_HEADER(CHUNK_STRUCT);
	uint32 textureCount = readUInt16(rw);
	this->dataStatus = readUInt16(rw);
	texList.resize(textureCount);

	for (uint32 i = 0; i < textureCount; i++)
    {
		READ_HEADER(CHUNK_TEXTURENATIVE);

        uint32 texNativeSize = header.length;

		rw.seekg(0x0c, ios::cur);
		texList[i].platform = readUInt32(rw);
		rw.seekg(-0x10, ios::cur);

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
		uint32 end = header.length;
		end += rw.tellg();
		while (rw.tellg() < end) {
			header.read(rw);
			switch (header.type) {
			case CHUNK_SKYMIPMAP:
				rw.seekg(4, ios::cur);
				break;
			default:
				rw.seekg(header.length, ios::cur);
				break;
			}
		}
	}

    // Read extensions.
    READ_HEADER(CHUNK_EXTENSION);
    uint32 end = header.length;
    end += rw.tellg();
    
    while ( rw.tellg() < end )
    {
        header.read(rw);

        rw.seekg(header.length, ios::cur);
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
    if (this->platform != PLATFORM_D3D8)
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

    bool isPaletteRaster = ( paletteType != PALETTE_NONE );

    if ( isPaletteRaster || newFormat != rasterFormat )
    {
        void *paletteData = platformTex->palette;
        uint32 maxpalette = platformTex->paletteSize;

	    for (uint32 j = 0; j < mipmapCount; j++)
        {
            uint32 dataSize = 0;
            uint32 newDepth = 0;

            // Decide about output data.
            uint32 colorIndiceCount = platformTex->width[j] * platformTex->height[j];

            bool validFormat = getrastermeta(newFormat, colorIndiceCount, dataSize, newDepth);

            if (validFormat)
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
                        bool hasColor = browsetexelcolor(texelSource, paletteType, paletteData, maxpalette, colorIndex, rasterFormat, colorOrder, red, green, blue, alpha);

                        if ( !hasColor )
                        {
                            red = 0;
                            green = 0;
                            blue = 0;
                            alpha = 0;
                        }

                        // Write the color data.
                        puttexelcolor(newtexels, colorIndex, newFormat, colorOrder, red, green, blue, alpha);
                    }
                }

                // Write the texture data.
	            delete[] texelSource;
	            platformTex->texels[j] = newtexels;
	            platformTex->dataSizes[j] = dataSize;
                platformTex->mipmapDepth[j] = newDepth;
            }
	    }

        // Update the format.
        this->rasterFormat = newFormat;

        // Make sure we update the d3dFormat.
        {
            D3DFORMAT newD3DFormat;

            bool hasD3DFormat = getD3DFormatFromRasterType( newFormat, colorOrder, newD3DFormat );

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
            uint32 depth = platformTex->mipmapDepth[ 0 ];
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
                    browsetexelcolor(texelSource, platformTex->paletteType, palData, palSize, n, theFormat, theOrder, r, g, b, a);

                    puttexelcolor(pixelData, n, theFormat, theOrder, r, g, b, a);
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
        platformTex->mipmapDepth.resize( 1 );

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
        platformTex->mipmapDepth[ 0 ] = newDepth;
        platformTex->texels[ 0 ] = srcImage.copyPixelData();
        platformTex->dataSizes[ 0 ] = newDataSize;

        platformTex->colorOrdering = newColorOrdering;

        // Update generics.
        {
            D3DFORMAT newD3DFormat;

            bool hasD3DFormat = getD3DFormatFromRasterType(newFormat, newColorOrdering, newD3DFormat);

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

void NativeTexture::writeTGA(const char *path)
{
    if ( this->platform != PLATFORM_D3D8 && this->platform != PLATFORM_D3D9 )
        return;

    NativeTextureD3D *platformTex = (NativeTextureD3D*)this->platformData;

    // If the texture is compressed, decompress it.
    if ( platformTex->dxtCompression )
    {
        platformTex->decompressDxt();
    }

	ofstream tga(path, ios::binary);
	if (tga.fail()) {
		cout << "not writing file: " << path << endl;
		return;
	}
	writeUInt8(0, tga);
	writeUInt8(0, tga);
	writeUInt8(2, tga);
	writeUInt16(0, tga);
	writeUInt16(0, tga);
	writeUInt8(0, tga);
	writeUInt16(0, tga);
	writeUInt16(0, tga);

    uint32 width = platformTex->width[0];
    uint32 height = platformTex->height[0];

	writeUInt16(width, tga);
	writeUInt16(height, tga);
	writeUInt8(0x20, tga);
	writeUInt8(0x28, tga);

    void *texelSource = platformTex->texels[0];
    void *paletteData = platformTex->palette;
    uint32 maxpalette = platformTex->paletteSize;
    eRasterFormat rasterFormat = this->rasterFormat;
    ePaletteType paletteType = platformTex->paletteType;
    eColorOrdering colorOrder = platformTex->colorOrdering;

	for (uint32 j = 0; j < width*height; j++)
    {
        uint8 red, green, blue, alpha;
        browsetexelcolor(texelSource, paletteType, paletteData, maxpalette, j, rasterFormat, colorOrder, red, green, blue, alpha);

		writeUInt8(blue, tga);
		writeUInt8(green, tga);
		writeUInt8(red, tga);
		writeUInt8(alpha, tga);
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
