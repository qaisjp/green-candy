#include "txdread.d3d.genmip.hxx"

#include <ATI_compress.h>

namespace rw
{

enum eATCInternalFormat
{
    ATC_RGB_AMD = 0x8C92,
    ATC_RGBA_EXPLICIT_ALPHA_AMD = 0x8C93,
    ATC_RGBA_INTERPOLATED_ALPHA_AMD = 0x87EE
};

inline uint32 getATCCompressionBlockSize( eATCInternalFormat internalFormat )
{
    uint32 theSize = 0;

    if ( internalFormat == ATC_RGB_AMD )
    {
        theSize = 8;
    }
    else if ( internalFormat == ATC_RGBA_EXPLICIT_ALPHA_AMD )
    {
        theSize = 16;
    }
    else if ( internalFormat == ATC_RGBA_INTERPOLATED_ALPHA_AMD )
    {
        theSize = 16;
    }

    return theSize;
}

struct NativeTextureATC : public PlatformTexture
{
    Interface *engineInterface;

    inline NativeTextureATC( Interface *engineInterface )
    {
        this->engineInterface = engineInterface;

        this->internalFormat = ATC_RGB_AMD;
        this->hasAlpha = false;
        
        // TODO.
        this->unk1 = 0;
        this->unk2 = 0;
    }

    inline NativeTextureATC( const NativeTextureATC& right )
    {
        // Copy parameters.
        this->engineInterface = right.engineInterface;
        this->internalFormat = right.internalFormat;
        this->hasAlpha = right.hasAlpha;
        this->unk1 = right.unk1;
        this->unk2 = right.unk2;

        // Copy mipmaps.
        copyMipmapLayers( this->engineInterface, right.mipmaps, this->mipmaps );
    }

    inline void clearImageData( void )
    {
        // Delete mipmap layers.
        deleteMipmapLayers( this->engineInterface, this->mipmaps );
    }

    inline ~NativeTextureATC( void )
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
        return 0;
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

    // Custom parameters.
    eATCInternalFormat internalFormat;

    bool hasAlpha;

    // Unknowns.
    uint8 unk1;
    uint32 unk2;
};

struct atcNativeTextureTypeProvider : public texNativeTypeProvider
{
    void ConstructTexture( Interface *engineInterface, void *objMem, size_t memSize )
    {
        new (objMem) NativeTextureATC( engineInterface );
    }

    void CopyConstructTexture( Interface *engineInterface, void *objMem, const void *srcObjMem, size_t memSize )
    {
        new (objMem) NativeTextureATC( *(const NativeTextureATC*)srcObjMem );
    }
    
    void DestroyTexture( Interface *engineInterface, void *objMem, size_t memSize )
    {
        ( *(NativeTextureATC*)objMem ).~NativeTextureATC();
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
        // This was never defined.
        return 0;
    }

    inline void Initialize( Interface *engineInterface )
    {

    }

    inline void Shutdown( Interface *engineInterface )
    {

    }
};

namespace atitc
{
#pragma pack(push, 1)
struct textureNativeGenericHeader
{
    uint32 platformDescriptor;

    uint32 filteringMode;
    uint32 uAddressing;
    uint32 vAddressing;

    uint8 pad1[0x10];

    char name[32];
    char maskName[32];

    uint8 mipmapCount;
    uint8 unk1;
    bool hasAlpha;

    uint8 pad2;

    uint16 width, height;

    eATCInternalFormat internalFormat;

    uint32 imageSectionStreamSize;
    uint32 unk2;
};
#pragma pack(pop)
};

};