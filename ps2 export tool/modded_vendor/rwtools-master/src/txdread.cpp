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

		if (texList[i].platform == PLATFORM_XBOX){
			texList[i].readXbox(rw);
		} else if (texList[i].platform == PLATFORM_D3D8 ||
		           texList[i].platform == PLATFORM_D3D9) {
			texList[i].readD3d(rw);
		} else if (texList[i].platform == PLATFORM_PS2FOURCC) {
			texList[i].platform = PLATFORM_PS2;
			texList[i].readPs2(rw);
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
                        bool hasColor = browsetexelcolor(texelSource, paletteType, paletteData, maxpalette, colorIndex, rasterFormat, red, green, blue, alpha);

                        if ( !hasColor )
                        {
                            red = 0;
                            green = 0;
                            blue = 0;
                            alpha = 0;
                        }

                        // Write the color data.
                        puttexelcolor(newtexels, colorIndex, newFormat, red, green, blue, alpha);
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
        platformTex->d3dFormat = getD3DFormatFromRasterType( newFormat );

        // Delete unnecessary palette data.
	    if (isPaletteRaster)
        {
		    delete[] paletteData;
		    platformTex->palette = NULL;

            platformTex->paletteSize = 0;
		    
            platformTex->paletteType = PALETTE_NONE;
	    }
    }
}

Bitmap NativeTexture::getBitmap(void) const
{
    Bitmap resultBitmap;

    // If the texture is Direct3D, we can easily get the texels.
    if ( this->platform == PLATFORM_D3D8 || this->platform == PLATFORM_D3D9 )
    {
        NativeTextureD3D *platformTex = (NativeTextureD3D*)this->platformData;

        // If it has any mipmap at all.
        if ( platformTex->mipmapCount > 0 )
        {
            uint32 width = platformTex->width[ 0 ];
            uint32 height = platformTex->height[ 0 ];
            uint32 depth = platformTex->mipmapDepth[ 0 ];
            uint32 dataSize = platformTex->dataSizes[ 0 ];
            eRasterFormat theFormat = this->rasterFormat;
            
            // Get the color data.
            bool hasAllocatedNewPixelData = false;
            void *pixelData = NULL;

            // If the image is palletized, it has to be converted to raw colors.
            if ( platformTex->paletteType != PALETTE_NONE )
            {
                hasAllocatedNewPixelData = true;

                
            }
            else
            {
                pixelData = platformTex->texels[ 0 ];
            }

            if ( pixelData != NULL )
            {
                // Set the data into the bitmap.
                resultBitmap.setImageData(
                    pixelData, theFormat, depth, width, height, dataSize
                );

                if ( hasAllocatedNewPixelData )
                {
                    delete pixelData;
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
        uint32 newDataSize = srcImage.getDataSize();

        srcImage.getSize( newWidth, newHeight );

        platformTex->width[ 0 ] = newWidth;
        platformTex->height[ 0 ] = newHeight;
        platformTex->mipmapDepth[ 0 ] = newDepth;
        platformTex->texels[ 0 ] = srcImage.copyPixelData();
        platformTex->dataSizes[ 0 ] = newDataSize;

        // Update generics.
        platformTex->d3dFormat = getD3DFormatFromRasterType(newFormat);

        this->rasterFormat = newFormat;
    }
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

	for (uint32 j = 0; j < width*height; j++)
    {
        uint8 red, green, blue, alpha;
        browsetexelcolor(texelSource, paletteType, paletteData, maxpalette, j, rasterFormat, red, green, blue, alpha);

		writeUInt8(blue, tga);
		writeUInt8(green, tga);
		writeUInt8(red, tga);
		writeUInt8(alpha, tga);
	}
	tga.close();
}

NativeTexture::NativeTexture(void)
: platform(0), name(""), maskName(""), filterFlags(0), uAddressing(1), vAddressing(1), rasterFormat(rw::RASTER_DEFAULT), hasAlpha(false)
{
    this->platformData = NULL;

    this->convertAlpha = true;
}

NativeTexture::NativeTexture(const NativeTexture &orig)
: platform(orig.platform),
  name(orig.name),
  maskName(orig.maskName),
  filterFlags(orig.filterFlags),
  uAddressing(orig.uAddressing),
  vAddressing(orig.vAddressing),
  rasterFormat(orig.rasterFormat),
  hasAlpha(orig.hasAlpha)
{
	PlatformTexture *platformTex = NULL;

    if ( orig.platformData )
    {
        platformTex = orig.platformData->Clone();
    }

    this->platformData = platformTex;

    this->convertAlpha = orig.convertAlpha;
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

		hasAlpha = that.hasAlpha;

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
