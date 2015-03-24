#include "StdInc.h"

#include "txdread.dxtmobile.hxx"

#include "streamutil.hxx"

namespace rw
{

void dxtMobileNativeTextureTypeProvider::SerializeTexture( TextureBase *theTexture, PlatformTexture *nativeTex, BlockProvider& outputProvider ) const
{
    Interface *engineInterface = theTexture->engineInterface;

    // Write the texture data.
    {
        BlockProvider texImageDataBlock( &outputProvider );

        texImageDataBlock.EnterContext();

        try
        {
            // Write the generic header.
            mobile_dxt::textureNativeGenericHeader metaHeader;
            metaHeader.platformDescriptor = PLATFORMDESC_DXT_MOBILE;

            metaHeader.filteringMode = theTexture->filterMode;
            metaHeader.uAddressing = theTexture->uAddressing;
            metaHeader.vAddressing = theTexture->vAddressing;

            memset( metaHeader.pad1, 0, sizeof( metaHeader.pad1 ) );

            // Correctly write the name strings (for safety).
            // Even though we can read those name fields with zero-termination safety,
            // the engines are not guarranteed to do so.
            // Also, print a warning if the name is changed this way.
            writeStringIntoBufferSafe( engineInterface, theTexture->name, metaHeader.name, sizeof( metaHeader.name ), theTexture->name, "name" );
            writeStringIntoBufferSafe( engineInterface, theTexture->maskName, metaHeader.maskName, sizeof( metaHeader.maskName ), theTexture->name, "mask name" );

            // Cast to our native format.
            NativeTextureMobileDXT *platformTex = (NativeTextureMobileDXT*)nativeTex;

            uint32 mipmapCount = platformTex->mipmaps.size();

            metaHeader.mipmapCount = mipmapCount;
            metaHeader.unk1 = false;
            metaHeader.hasAlpha = platformTex->hasAlpha;
            metaHeader.pad2 = 0;

            metaHeader.width = platformTex->mipmaps[ 0 ].layerWidth;
            metaHeader.height = platformTex->mipmaps[ 0 ].layerHeight;

            metaHeader.internalFormat = platformTex->internalFormat;

            // Calculate the image data section size.
            uint32 imageDataSectionSize = 0;

            for ( uint32 n = 0; n < mipmapCount; n++ )
            {
                uint32 dataSize = platformTex->mipmaps[ 0 ].dataSize;

                imageDataSectionSize += sizeof( uint32 );
                imageDataSectionSize += dataSize;
            }

            metaHeader.imageDataSectionSize = imageDataSectionSize;
            
            metaHeader.unk3 = platformTex->unk3;

            // Write the header.
            texImageDataBlock.write( &metaHeader, sizeof( metaHeader ) );

            // Write the mipmap data sizes.
            for ( uint32 n = 0; n < mipmapCount; n++ )
            {
                uint32 dataSize = platformTex->mipmaps[ n ].dataSize;

                texImageDataBlock.writeUInt32( dataSize );
            }

            // Write the mipmap texels now.
            for ( uint32 n = 0; n < mipmapCount; n++ )
            {
                const NativeTextureMobileDXT::mipmapLayer& mipLayer = platformTex->mipmaps[ n ];

                uint32 dataSize = mipLayer.dataSize;

                const void *theTexels = mipLayer.texels;

                texImageDataBlock.write( theTexels, dataSize );
            }

            // Alright, we are finished.
            // Those should be all of the War Drum Studios formats.
        }
        catch( ... )
        {
            texImageDataBlock.LeaveContext();

            throw;
        }

        texImageDataBlock.LeaveContext();
    }

    // Write extensions.
    engineInterface->SerializeExtensions( theTexture, outputProvider );
}

void dxtMobileNativeTextureTypeProvider::GetPixelDataFromTexture( Interface *engineInterface, void *objMem, pixelDataTraversal& pixelsOut )
{
    // Getting data from this format is very easy and compatible with existing PC formats.
    // I do have to watch out for special cases though, like the DXT1 fiasco (non-alpha/alpha).
    // That is for another time, tho.

    NativeTextureMobileDXT *nativeTex = (NativeTextureMobileDXT*)objMem;

    // Get the compression format we need to use.
    eCompressionType rwCompressionType;

    eS3TCInternalFormat internalFormat = nativeTex->internalFormat;

    if ( internalFormat == COMPRESSED_RGB_S3TC_DXT1 ||
         internalFormat == COMPRESSED_RGBA_S3TC_DXT1 )
    {
        rwCompressionType = RWCOMPRESS_DXT1;
    }
    else if ( internalFormat == COMPRESSED_RGBA_S3TC_DXT3 )
    {
        rwCompressionType = RWCOMPRESS_DXT3;
    }
    else if ( internalFormat == COMPRESSED_RGBA_S3TC_DXT5 )
    {
        rwCompressionType = RWCOMPRESS_DXT5;
    }
    else
    {
        throw RwException( "invalid internalFormat in s3tc_mobile native texture texel fetch" );
    }

    // Copy over mipmap information.
    uint32 mipmapCount = nativeTex->mipmaps.size();

    pixelsOut.mipmaps.resize( mipmapCount );

    for ( uint32 n = 0; n < mipmapCount; n++ )
    {
        const NativeTextureMobileDXT::mipmapLayer& mipLayer = nativeTex->mipmaps[ n ];

        // Create a new virtual layer.
        pixelDataTraversal::mipmapResource newLayer;

        newLayer.width = mipLayer.width;
        newLayer.height = mipLayer.height;

        newLayer.mipWidth = mipLayer.layerWidth;
        newLayer.mipHeight = mipLayer.layerHeight;

        // Just move over the texels.
        newLayer.texels = mipLayer.texels;
        newLayer.dataSize = mipLayer.dataSize;

        // Give it to the runtime.
        pixelsOut.mipmaps[ n ] = newLayer;
    }

    // We have a compressed raster.
    pixelsOut.rasterFormat = RASTER_DEFAULT;
    pixelsOut.depth = 16;
    pixelsOut.colorOrder = COLOR_BGRA;
    pixelsOut.paletteType = PALETTE_NONE;
    pixelsOut.paletteData = NULL;
    pixelsOut.paletteSize = 0;
    
    pixelsOut.compressionType = rwCompressionType;

    // Travel advanced properties.
    pixelsOut.hasAlpha = nativeTex->hasAlpha;
    pixelsOut.autoMipmaps = false;
    pixelsOut.cubeTexture = false;
    pixelsOut.rasterType = 4;

    // We can directly give the pixels to the runtime.
    pixelsOut.isNewlyAllocated = false;
}

void dxtMobileNativeTextureTypeProvider::SetPixelDataToTexture( Interface *engineInterface, void *objMem, const pixelDataTraversal& pixelsIn, acquireFeedback_t& feedbackOut )
{
    // In this routine we can directly acquire DXT1/3/5 texture data.
    // If we receive raw bitmaps, we have to use the RenderWare library to compress it for us.
    // This format is very compatible with PC and XBOX native textures.

    NativeTextureMobileDXT *nativeTex = (NativeTextureMobileDXT*)objMem;

    // Check whether we can convert the compression type into a S3TC internalFormat.
    // If we can, then a simple assignment is enough.
    bool canAssignData = false;

    eCompressionType rwCompressionType = pixelsIn.compressionType;

    bool hasAlpha = pixelsIn.hasAlpha;

    eS3TCInternalFormat internalFormat;

    eCompressionType dstCompressionType = rwCompressionType;

    bool srcHasAlpha = pixelsIn.hasAlpha;

    if ( dstCompressionType == RWCOMPRESS_NONE )
    {
        // We need to compress stuff ourselves.
        // Better let everythin be taken care of by the core.
        canAssignData = false;

        // Decide about the best compression type we should use.
        bool hasCompressionType =
            DecideBestDXTCompressionFormat(
                engineInterface,
                srcHasAlpha,
                true, false, true, false, true,
                dstCompressionType
            );

        if ( hasCompressionType == false )
        {
            throw RwException( "could not decide on a DXT compression type in s3tc_mobile native texture pixel assignment" );
        }
    }

    if ( dstCompressionType == RWCOMPRESS_DXT1 )
    {
        if ( hasAlpha == false )
        {
            internalFormat = COMPRESSED_RGB_S3TC_DXT1;
        }
        else
        {
            internalFormat = COMPRESSED_RGBA_S3TC_DXT1;
        }
        
        canAssignData = true;
    }
    else if ( dstCompressionType == RWCOMPRESS_DXT3 )
    {
        internalFormat = COMPRESSED_RGBA_S3TC_DXT3;

        canAssignData = true;
    }
    else if ( dstCompressionType == RWCOMPRESS_DXT5 )
    {
        internalFormat = COMPRESSED_RGBA_S3TC_DXT5;

        canAssignData = true;
    }
    else
    {
        throw RwException( "invalid input compression type in s3tc_mobile texture native pixel assignment" );
    }

    eRasterFormat srcRasterFormat = pixelsIn.rasterFormat;
    eColorOrdering srcColorOrder = pixelsIn.colorOrder;
    uint32 srcDepth = pixelsIn.depth;

    ePaletteType srcPaletteType = pixelsIn.paletteType;
    const void *srcPaletteData = pixelsIn.paletteData;
    uint32 srcPaletteSize = pixelsIn.paletteSize;

    uint32 mipmapCount = pixelsIn.mipmaps.size();

    nativeTex->mipmaps.resize( mipmapCount );

    for ( uint32 n = 0; n < mipmapCount; n++ )
    {
        const pixelDataTraversal::mipmapResource& mipLayer = pixelsIn.mipmaps[ n ];

        // Get the mipmap data on the stack.
        uint32 mipWidth = mipLayer.width;
        uint32 mipHeight = mipLayer.height;

        uint32 layerWidth = mipLayer.mipWidth;
        uint32 layerHeight = mipLayer.mipHeight;

        void *texels = mipLayer.texels;
        uint32 dataSize = mipLayer.dataSize;

        if ( canAssignData == false )
        {
            // Do DXT compression.
            uint32 newWidth, newHeight;

            void *dstTexels;
            uint32 dstDataSize;

            bool compressionSuccess = ConvertMipmapLayerEx(
                engineInterface,
                mipLayer,
                srcRasterFormat, srcDepth, srcColorOrder, srcPaletteType, srcPaletteData, srcPaletteSize, rwCompressionType,
                RASTER_DEFAULT, 16, COLOR_BGRA, dstCompressionType,
                newWidth, newHeight,
                dstTexels, dstDataSize
            );

            assert( compressionSuccess == false );

            // Update properties.
            mipWidth = newWidth;
            newHeight = newHeight;

            texels = dstTexels;
            dataSize = dstDataSize;
        }

        // Create a new mipmap layer.
        NativeTextureMobileDXT::mipmapLayer newLayer;

        newLayer.width = mipWidth;
        newLayer.height = mipHeight;

        newLayer.layerWidth = layerWidth;
        newLayer.layerHeight = layerHeight;

        newLayer.texels = texels;
        newLayer.dataSize = dataSize;

        // Store it.
        nativeTex->mipmaps[ n ] = newLayer;
    }

    // Store raster properties.
    nativeTex->internalFormat = internalFormat;

    nativeTex->hasAlpha = srcHasAlpha;

    // In case we needed to compress data, we cannot have assigned.
    // In general, it is best to pass precompressed data to this texture format.
    feedbackOut.hasDirectlyAcquired = canAssignData;
}

void dxtMobileNativeTextureTypeProvider::UnsetPixelDataFromTexture( Interface *engineInterface, void *objMem, bool deallocate )
{
    NativeTextureMobileDXT *nativeTex = (NativeTextureMobileDXT*)objMem;

    if ( deallocate )
    {
        // Delete mipmap texels.
        nativeTex->clearTexelData();
    }

    // Simply remove the refences to the texel pointers.
    nativeTex->mipmaps.clear();

    // For simpler debugging, we unset raster information aswell.
    nativeTex->internalFormat = COMPRESSED_RGB_S3TC_DXT1;
    nativeTex->unk3 = 0;
    nativeTex->hasAlpha = false;
}


};