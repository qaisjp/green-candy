#include "txdread.d3d.genmip.hxx"

namespace rw
{

struct NativeTextureXBOX : public PlatformTexture
{
    Interface *engineInterface;

    inline NativeTextureXBOX( Interface *engineInterface )
    {
        // Initialize the texture object.
        this->engineInterface = engineInterface;
        this->palette = NULL;
        this->paletteSize = 0;
        this->paletteType = PALETTE_NONE;
        this->rasterFormat = RASTER_DEFAULT;
        this->depth = 0;
        this->dxtCompression = 0;
        this->hasAlpha = false;
        this->colorOrder = COLOR_BGRA;
        this->rasterType = 4;   // by default it is a texture raster.
        this->autoMipmaps = false;
    }

    inline NativeTextureXBOX( const NativeTextureXBOX& right )
    {
        Interface *engineInterface = right.engineInterface;

        this->engineInterface = right.engineInterface;

        // Copy palette information.
        {
	        if (right.palette)
            {
                uint32 palRasterDepth = Bitmap::getRasterFormatDepth(right.rasterFormat);

                size_t wholeDataSize = getRasterDataSize( right.paletteSize, palRasterDepth );

		        this->palette = engineInterface->PixelAllocate( wholeDataSize );

		        memcpy(this->palette, right.palette, wholeDataSize);
	        }
            else
            {
		        this->palette = NULL;
	        }

            this->paletteSize = right.paletteSize;
            this->paletteType = right.paletteType;
        }

        // Copy image texel information.
        {
            copyMipmapLayers( engineInterface, right.mipmaps, this->mipmaps );

            // Copy over attributes.
            this->rasterFormat = right.rasterFormat;
            this->depth = right.depth;
        }

        this->dxtCompression = right.dxtCompression;
        this->hasAlpha = right.hasAlpha;

        this->colorOrder = right.colorOrder;
        this->rasterType = right.rasterType;
        this->autoMipmaps = right.autoMipmaps;
    }

    inline void clearTexelData( void )
    {
        if ( this->palette )
        {
	        this->engineInterface->PixelFree( palette );

	        palette = NULL;
        }

	    deleteMipmapLayers( this->engineInterface, this->mipmaps );

        this->mipmaps.clear();
    }

    inline ~NativeTextureXBOX( void )
    {
        this->clearTexelData();
    }

    uint32 getWidth( void ) const
    {
        return this->mipmaps[ 0 ].layerWidth;
    }

    uint32 getHeight( void ) const
    {
        return this->mipmaps[ 0 ].layerHeight;
    }

    uint32 getDepth( void ) const
    {
        return this->depth;
    }

    uint32 getMipmapCount( void ) const
    {
        return this->mipmaps.size();
    }

    ePaletteType getPaletteType( void ) const
    {
        return this->paletteType;
    }

    bool isCompressed( void ) const
    {
        return ( this->dxtCompression != 0 );
    }

    void compress( float quality )
    {
        // nothing to do here.
        // you have to convert to D3D first to be able to compress (to DXT).
    }

    eRasterFormat rasterFormat;

    uint32 depth;

    typedef genmip::mipmapLayer mipmapLayer;

    std::vector <mipmapLayer> mipmaps;

	void *palette;
	uint32 paletteSize;

    ePaletteType paletteType;

    uint8 rasterType;

    eColorOrdering colorOrder;

    bool hasAlpha;

    bool autoMipmaps;

	// PC/XBOX
	uint32 dxtCompression;

    struct swizzleMipmapTraversal
    {
        // Input.
        uint32 mipWidth, mipHeight;
        uint32 depth;
        void *texels;
        uint32 dataSize;

        // Output.
        uint32 newWidth, newHeight;
        void *newtexels;
        uint32 newDataSize;
    };

    static void swizzleMipmap( Interface *engineInterface, swizzleMipmapTraversal& pixelData );
    static void unswizzleMipmap( Interface *engineInterface, swizzleMipmapTraversal& pixelData );
};

struct xboxNativeTextureTypeProvider : public texNativeTypeProvider
{
    void ConstructTexture( Interface *engineInterface, void *objMem, size_t memSize )
    {
        new (objMem) NativeTextureXBOX( engineInterface );
    }

    void CopyConstructTexture( Interface *engineInterface, void *objMem, const void *srcObjMem, size_t memSize )
    {
        new (objMem) NativeTextureXBOX( *(const NativeTextureXBOX*)srcObjMem );
    }
    
    void DestroyTexture( Interface *engineInterface, void *objMem, size_t memSize )
    {
        ( *(NativeTextureXBOX*)objMem ).~NativeTextureXBOX();
    }

    eTexNativeCompatibility IsCompatibleTextureBlock( BlockProvider& inputProvider ) const;

    void SerializeTexture( TextureBase *theTexture, PlatformTexture *nativeTex, BlockProvider& outputProvider ) const;
    void DeserializeTexture( TextureBase *theTexture, PlatformTexture *nativeTex, BlockProvider& inputProvider ) const;

    void GetPixelCapabilities( pixelCapabilities& capsOut ) const
    {
        capsOut.supportsDXT1 = true;
        capsOut.supportsDXT2 = true;
        capsOut.supportsDXT3 = true;
        capsOut.supportsDXT4 = true;
        capsOut.supportsDXT5 = true;
        capsOut.supportsPalette = true;
    }

    void GetPixelDataFromTexture( Interface *engineInterface, void *objMem, pixelDataTraversal& pixelsOut );
    void SetPixelDataToTexture( Interface *engineInterface, void *objMem, const pixelDataTraversal& pixelsIn, acquireFeedback_t& feedbackOut );
    void UnsetPixelDataFromTexture( Interface *engineInterface, void *objMem, bool deallocate );

    uint32 GetDriverIdentifier( void *objMem ) const
    {
        // Always the generic XBOX driver.
        return 8;
    }

    inline void Initialize( Interface *engineInterface )
    {

    }

    inline void Shutdown( Interface *engineInterface )
    {

    }
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