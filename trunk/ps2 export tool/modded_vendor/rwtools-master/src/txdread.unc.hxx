// Platform descriptor, not that important.
#define PLATFORMDESC_UNC_MOBILE     12

#include "txdread.d3d.genmip.hxx"

namespace rw
{

struct NativeTextureMobileUNC
{
    Interface *engineInterface;

    inline NativeTextureMobileUNC( Interface *engineInterface )
    {
        this->engineInterface = engineInterface;
        this->hasAlpha = false;
        this->unk2 = 0;
        this->unk3 = 0;
    }

    inline NativeTextureMobileUNC( const NativeTextureMobileUNC& right )
    {
        Interface *engineInterface = right.engineInterface;

        this->engineInterface = engineInterface;
        this->hasAlpha = right.hasAlpha;
        this->unk2 = right.unk2;
        this->unk3 = right.unk3;

        // Copy over mipmaps.
        copyMipmapLayers( engineInterface, right.mipmaps, this->mipmaps );
    }

    inline void clearTexelData( void )
    {
        deleteMipmapLayers( this->engineInterface, this->mipmaps );
    }

    inline ~NativeTextureMobileUNC( void )
    {
        // Delete all mipmaps.
        this->clearTexelData();
    }

    typedef genmip::mipmapLayer mipmapLayer;

    std::vector <mipmapLayer> mipmaps;

    bool hasAlpha;

    uint32 unk2;
    uint32 unk3;
};

inline void getUNCRasterFormat( bool hasAlpha, eRasterFormat& rasterFormat, eColorOrdering& colorOrder, uint32& depth )
{
    // TODO.
    rasterFormat = RASTER_4444;
    depth = 16;
    colorOrder = COLOR_BGRA;
}

struct uncNativeTextureTypeProvider : public texNativeTypeProvider
{
    void ConstructTexture( Interface *engineInterface, void *objMem, size_t memSize )
    {
        new (objMem) NativeTextureMobileUNC( engineInterface );
    }

    void CopyConstructTexture( Interface *engineInterface, void *objMem, const void *srcObjMem, size_t memSize )
    {
        new (objMem) NativeTextureMobileUNC( *(const NativeTextureMobileUNC*)srcObjMem );
    }
    
    void DestroyTexture( Interface *engineInterface, void *objMem, size_t memSize )
    {
        ( *(NativeTextureMobileUNC*)objMem ).~NativeTextureMobileUNC();
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

namespace mobile_unc
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

    uint32 unk2;

    uint32 imageDataSectionSize;
    uint32 unk3;
};
#pragma pack(pop)
};

};