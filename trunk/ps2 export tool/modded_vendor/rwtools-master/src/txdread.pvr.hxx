#include <PVRTextureUtilities.h>

#include "txdread.d3d.genmip.hxx"

namespace rw
{

enum ePVRInternalFormat
{
    GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG = 0x8C00,
    GL_COMPRESSED_RGB_PVRTC_2BPPV1_IMG = 0x8C01,
    GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG = 0x8C02,
    GL_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG = 0x8C03
};

inline uint32 getDepthByPVRFormat( ePVRInternalFormat theFormat )
{
    uint32 formatDepth = 0;

    if ( theFormat == GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG ||
         theFormat == GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG )
    {
        formatDepth = 4;
    }
    else if ( theFormat == GL_COMPRESSED_RGB_PVRTC_2BPPV1_IMG ||
              theFormat == GL_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG )
    {
        formatDepth = 2;
    }
    else
    {
        assert( 0 );
    }

    return formatDepth;
}

struct NativeTexturePVR : public PlatformTexture
{
    Interface *engineInterface;

    inline NativeTexturePVR( Interface *engineInterface )
    {
        this->engineInterface = engineInterface;

        this->internalFormat = GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG;
        this->hasAlpha = false;

        this->unk1 = 0;
        this->unk8 = 0;
    }

    inline NativeTexturePVR( const NativeTexturePVR& right )
    {
        // Copy parameters.
        this->engineInterface = right.engineInterface;
        this->internalFormat = right.internalFormat;
        this->hasAlpha = right.hasAlpha;
        this->unk1 = right.unk1;
        this->unk8 = right.unk8;

        // Copy mipmaps.
        copyMipmapLayers( this->engineInterface, right.mipmaps, this->mipmaps );
    }

    inline void clearImageData( void )
    {
        // Delete mipmap layers.
        deleteMipmapLayers( this->engineInterface, this->mipmaps );
    }

    inline ~NativeTexturePVR( void )
    {
        this->clearImageData();
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
        return getDepthByPVRFormat( this->internalFormat );
    }

    uint32 getMipmapCount( void ) const
    {
        return this->mipmaps.size();
    }

    ePaletteType getPaletteType( void ) const
    {
        // PVR textures are never palettized.
        return PALETTE_NONE;
    }

    bool isCompressed( void ) const
    {
        // PVR textures are always compressed.
        return true;
    }

    void compress( float quality )
    {
        // Since we are always compressed, there is nothing to do here.
        return;
    }

    typedef genmip::mipmapLayer mipmapLayer;

    std::vector <mipmapLayer> mipmaps;

    ePVRInternalFormat internalFormat;

    bool hasAlpha;

    // Unknowns.
    uint8 unk1;
    uint32 unk8;
};

struct pvrNativeTextureTypeProvider : public texNativeTypeProvider
{
    void ConstructTexture( Interface *engineInterface, void *objMem, size_t memSize )
    {
        new (objMem) NativeTexturePVR( engineInterface );
    }

    void CopyConstructTexture( Interface *engineInterface, void *objMem, const void *srcObjMem, size_t memSize )
    {
        new (objMem) NativeTexturePVR( *(const NativeTexturePVR*)srcObjMem );
    }
    
    void DestroyTexture( Interface *engineInterface, void *objMem, size_t memSize )
    {
        ( *(NativeTexturePVR*)objMem ).~NativeTexturePVR();
    }

    eTexNativeCompatibility IsCompatibleTextureBlock( BlockProvider& inputProvider ) const;

    void SerializeTexture( TextureBase *theTexture, PlatformTexture *nativeTex, BlockProvider& outputProvider ) const;
    void DeserializeTexture( TextureBase *theTexture, PlatformTexture *nativeTex, BlockProvider& inputProvider ) const;

    void GetPixelCapabilities( pixelCapabilities& capsOut ) const
    {
        capsOut.supportsDXT1 = false;
        capsOut.supportsDXT2 = false;
        capsOut.supportsDXT3 = false;
        capsOut.supportsDXT4 = false;
        capsOut.supportsDXT5 = false;
        capsOut.supportsPalette = true;
    }

    void GetPixelDataFromTexture( Interface *engineInterface, void *objMem, pixelDataTraversal& pixelsOut );
    void SetPixelDataToTexture( Interface *engineInterface, void *objMem, const pixelDataTraversal& pixelsIn, acquireFeedback_t& feedbackOut );
    void UnsetPixelDataFromTexture( Interface *engineInterface, void *objMem, bool deallocate );

    uint32 GetDriverIdentifier( void *objMem ) const
    {
        // Has not been officially defined.
        return 0;
    }

    inline void Initialize( Interface *engineInterface )
    {

    }

    inline void Shutdown( Interface *engineInterface )
    {

    }
};

namespace pvr
{

#pragma pack(1)
struct textureMetaHeaderGeneric
{
    uint32 platformDescriptor;

    uint32 filterMode;
    uint32 uAddressing;
    uint32 vAddressing;

    uint8 pad1[ 0x10 ];

    char name[ 32 ];
    char maskName[ 32 ];

    uint8 mipmapCount;
    uint8 unk1;
    bool hasAlpha;

    uint8 pad2;

    uint16 width, height;

    ePVRInternalFormat internalFormat;
    uint32 imageDataStreamSize;
    uint32 unk8;
};
#pragma pack()

};

};