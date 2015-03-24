#include <StdInc.h>

#include "pixelformat.hxx"

#include "txdread.d3d.hxx"
#include "txdread.pvr.hxx"

#include "txdread.common.hxx"

#include "streamutil.hxx"

namespace rw
{

eTexNativeCompatibility pvrNativeTextureTypeProvider::IsCompatibleTextureBlock( BlockProvider& inputProvider ) const
{
    eTexNativeCompatibility texCompat = RWTEXCOMPAT_NONE;

    BlockProvider texNativeImageBlock( &inputProvider );

    texNativeImageBlock.EnterContext();

    try
    {
        if ( texNativeImageBlock.getBlockID() == CHUNK_STRUCT )
        {
            // Here we can check the platform descriptor, since we know it is unique.
            uint32 platformDescriptor = texNativeImageBlock.readUInt32();

            if ( platformDescriptor == PLATFORM_PVR )
            {
                texCompat = RWTEXCOMPAT_ABSOLUTE;
            }
        }
    }
    catch( ... )
    {
        texNativeImageBlock.LeaveContext();

        throw;
    }

    texNativeImageBlock.LeaveContext();

    return texCompat;
}

void pvrNativeTextureTypeProvider::SerializeTexture( TextureBase *theTexture, PlatformTexture *nativeTex, BlockProvider& outputProvider ) const
{
    Interface *engineInterface = theTexture->engineInterface;

    // Cast the texture to our native type.
    NativeTexturePVR *platformTex = (NativeTexturePVR*)nativeTex;

	// Struct
	{
		BlockProvider texImageDataChunk( &outputProvider );
        
        texImageDataChunk.EnterContext();

        try
        {
            // Write the header with meta information.
            pvr::textureMetaHeaderGeneric metaHeader;
            metaHeader.platformDescriptor = PLATFORM_PVR;
            metaHeader.filterMode = theTexture->filterMode;
            metaHeader.uAddressing = theTexture->uAddressing;
            metaHeader.vAddressing = theTexture->vAddressing;
            
            memset( metaHeader.pad1, 0, sizeof(metaHeader.pad1) );

            // Correctly write the name strings (for safety).
            // Even though we can read those name fields with zero-termination safety,
            // the engines are not guarranteed to do so.
            // Also, print a warning if the name is changed this way.
            writeStringIntoBufferSafe( engineInterface, theTexture->name, metaHeader.name, sizeof( metaHeader.name ), theTexture->name, "name" );
            writeStringIntoBufferSafe( engineInterface, theTexture->maskName, metaHeader.maskName, sizeof( metaHeader.maskName ), theTexture->name, "mask name" );

            uint32 mipmapCount = platformTex->mipmaps.size();

            metaHeader.mipmapCount = mipmapCount;
            metaHeader.unk1 = platformTex->unk1;
            metaHeader.hasAlpha = platformTex->hasAlpha;
            metaHeader.pad2 = 0;

            metaHeader.width = platformTex->mipmaps[ 0 ].layerWidth;
            metaHeader.height = platformTex->mipmaps[ 0 ].layerHeight;

            metaHeader.internalFormat = platformTex->internalFormat;
            
            // Calculate the image data section size.
            uint32 imageDataSectionSize = 0;

            for ( uint32 n = 0; n < mipmapCount; n++ )
            {
                uint32 mipDataSize = platformTex->mipmaps[ n ].dataSize;

                imageDataSectionSize += mipDataSize;
                imageDataSectionSize += sizeof( uint32 );
            }

            metaHeader.imageDataStreamSize = imageDataSectionSize;
            metaHeader.unk8 = platformTex->unk8;

            // Write the meta header.
            texImageDataChunk.write( &metaHeader, sizeof(metaHeader) );

            // Write the mipmap data sizes.
            for ( uint32 n = 0; n < mipmapCount; n++ )
            {
                uint32 mipDataSize = platformTex->mipmaps[ n ].dataSize;

                texImageDataChunk.writeUInt32( mipDataSize );
            }

            // Write the picture data now.
            for ( uint32 n = 0; n < mipmapCount; n++ )
            {
                NativeTexturePVR::mipmapLayer& mipLayer = platformTex->mipmaps[ n ];

                uint32 mipDataSize = mipLayer.dataSize;

                texImageDataChunk.write( mipLayer.texels, mipDataSize );
            }
        }
        catch( ... )
        {
            texImageDataChunk.LeaveContext();

            throw;
        }

        texImageDataChunk.LeaveContext();
	}

    // Write the extensions.
    engineInterface->SerializeExtensions( theTexture, outputProvider );
}

inline bool getPVRCompressionTypeFromInternalFormat( ePVRInternalFormat internalFormat, EPVRTPixelFormat& pvrFormatOut )
{
    EPVRTPixelFormat compressionPixelType;

    if ( internalFormat == GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG )
    {
        compressionPixelType = ePVRTPF_PVRTCI_4bpp_RGB;
    }
    else if ( internalFormat == GL_COMPRESSED_RGB_PVRTC_2BPPV1_IMG )
    {
        compressionPixelType = ePVRTPF_PVRTCI_2bpp_RGB;
    }
    else if ( internalFormat == GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG )
    {
        compressionPixelType = ePVRTPF_PVRTCI_4bpp_RGBA;
    }
    else if ( internalFormat == GL_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG )
    {
        compressionPixelType = ePVRTPF_PVRTCI_2bpp_RGBA;
    }
    else
    {
        return false;
    }

    pvrFormatOut = compressionPixelType;
    return true;
}

void pvrNativeTextureTypeProvider::GetPixelDataFromTexture( Interface *engineInterface, void *objMem, pixelDataTraversal& pixelsOut )
{
    NativeTexturePVR *platformTex = (NativeTexturePVR*)objMem;

    // Decide to what raster format we should decode to.
    eRasterFormat targetRasterFormat;
    uint32 targetDepth;
    eColorOrdering targetColorOrder = COLOR_BGRA;

    ePVRInternalFormat internalFormat = platformTex->internalFormat;

    if ( internalFormat == GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG ||
         internalFormat == GL_COMPRESSED_RGB_PVRTC_2BPPV1_IMG )
    {
        targetRasterFormat = RASTER_888;
        targetDepth = 32;
    }
    else if ( internalFormat == GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG ||
              internalFormat == GL_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG )
    {
        targetRasterFormat = RASTER_8888;
        targetDepth = 32;
    }
    else
    {
        throw RwException( "failed to determine raster format for PVR texture decompression" );
    }

    // Decompress the PVR texture and put it in RGBA format into the D3D texture.
    uint32 mipmapCount = platformTex->mipmaps.size();

    pixelsOut.mipmaps.resize( mipmapCount );
    {
        using namespace pvrtexture;

        EPVRTPixelFormat compressionPixelType;

        bool gotLibFormat = getPVRCompressionTypeFromInternalFormat( internalFormat, compressionPixelType );

        if ( !gotLibFormat )
        {
            throw RwException( "failed to decompress PVRTC due to unknown internalFormat" );
        }

        // Create source the pixel type descriptor.
        const PixelType pvrSrcPixelType( compressionPixelType );

        // We need a pixel type for the decompressed format.
        const PixelType pvrDstPixelType = PVRStandard8PixelType;

        for ( uint32 n = 0; n < mipmapCount; n++ )
        {
            // Get parameters of this mipmap layer.
            const NativeTexturePVR::mipmapLayer& mipLayer = platformTex->mipmaps[ n ];

            // Create a PVR texture.
            CPVRTextureHeader pvrHeader( pvrSrcPixelType.PixelTypeID, mipLayer.width, mipLayer.height );

            CPVRTexture pvrSourceTexture( pvrHeader, mipLayer.texels );

            // Decompress it.
            bool transcodeSuccess =
                Transcode( pvrSourceTexture, pvrDstPixelType, ePVRTVarTypeUnsignedByteNorm, ePVRTCSpacelRGB );

            assert( transcodeSuccess == true );

            // Get the new texels into the Direct3D texture.
            pixelDataTraversal::mipmapResource newLayer;

            // The raw dimensions match the layer dimensions, because we output in a non-compressed format.
            newLayer.width = mipLayer.layerWidth;
            newLayer.height = mipLayer.layerHeight;

            uint32 srcDataSize = pvrSourceTexture.getDataSize();
            
            newLayer.mipWidth = mipLayer.layerWidth;
            newLayer.mipHeight = mipLayer.layerHeight;

            // Create a new raw texture of the layer dimensions.
            uint32 texelUnitCount = ( newLayer.mipWidth * newLayer.mipHeight );

            uint32 dstDataSize = getRasterDataSize( texelUnitCount, targetDepth );

            newLayer.dataSize = dstDataSize;

            uint32 pvrWidth = pvrSourceTexture.getWidth();
            uint32 pvrHeight = pvrSourceTexture.getHeight();

            const void *srcTexelPtr = pvrSourceTexture.getDataPtr();

            // Allocate new texels.
            newLayer.texels = engineInterface->PixelAllocate( dstDataSize );

            for ( uint32 y = 0; y < pvrHeight; y++ )
            {
                for ( uint32 x = 0; x < pvrWidth; x++ )
                {
                    uint8 r, g, b, a;

                    uint32 pvrColorIndex = PixelFormat::coord2index(x, y, pvrWidth);
                    
                    bool hasColor = browsetexelcolor(srcTexelPtr, PALETTE_NONE, NULL, 0, pvrColorIndex, RASTER_8888, COLOR_RGBA, 32, r, g, b, a);

                    if ( !hasColor )
                    {
                        r = 0;
                        g = 0;
                        b = 0;
                        a = 0;
                    }

                    if ( x < newLayer.mipWidth && y < newLayer.mipHeight )
                    {
                        // Put the color in the correct format.
                        uint32 dstColorIndex = PixelFormat::coord2index(x, y, newLayer.mipWidth);

                        puttexelcolor(newLayer.texels, dstColorIndex, targetRasterFormat, targetColorOrder, targetDepth, r, g, b, a);
                    }
                }
            }

            // Put it into the Direct3D texture.
            pixelsOut.mipmaps[ n ] = newLayer;
        }

        // We have successfully copied the mipmap data to the pixelsOut struct!
    }

    // Copy over general raster properties.
    pixelsOut.rasterFormat = targetRasterFormat;
    pixelsOut.depth = targetDepth;
    pixelsOut.colorOrder = targetColorOrder;
    pixelsOut.paletteType = PALETTE_NONE;
    pixelsOut.paletteData = NULL;
    pixelsOut.paletteSize = 0;
    
    // We always output in a non-compressed format.
    pixelsOut.compressionType = RWCOMPRESS_NONE;

    // Move over advanced properties.
    pixelsOut.hasAlpha = platformTex->hasAlpha;
    pixelsOut.cubeTexture = false;
    pixelsOut.autoMipmaps = false;    // really? what about the other fields?

    pixelsOut.rasterType = 4;   // PowerVR does only store bitmap textures.

    // Since we decompress, we always have newly allocated pixel data.
    pixelsOut.isNewlyAllocated = true;
}

void pvrNativeTextureTypeProvider::SetPixelDataToTexture( Interface *engineInterface, void *objMem, const pixelDataTraversal& pixelsIn, acquireFeedback_t& feedbackOut )
{
    // Allocate a new texture.
    NativeTexturePVR *pvrTex = (NativeTexturePVR*)objMem;

    // We can only accept raw bitmaps here.
    assert( pixelsIn.compressionType == RWCOMPRESS_NONE );

    // Clear any image data that may have been there.
    pvrTex->clearImageData();

    // Give it common parameters.
    bool hasAlpha = pixelsIn.hasAlpha;

    pvrTex->hasAlpha = hasAlpha;

    // Copy over compressed texels.
    eRasterFormat srcRasterFormat = pixelsIn.rasterFormat;
    eColorOrdering srcColorOrder = pixelsIn.colorOrder;
    uint32 srcDepth = pixelsIn.depth;

    ePaletteType srcPaletteType = pixelsIn.paletteType;
    const void *paletteData = pixelsIn.paletteData;
    uint32 paletteSize = pixelsIn.paletteSize;

    // Determine the internal format we are going to compress to.
    ePVRInternalFormat internalFormat = GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG;

    const pixelDataTraversal::mipmapResource& mainMipLayer = pixelsIn.mipmaps[ 0 ];

    bool canBeCompressedHigh = ( mainMipLayer.mipWidth * mainMipLayer.mipHeight ) >= ( 100 * 100 );

    if ( hasAlpha )
    {
        if ( canBeCompressedHigh )
        {
            internalFormat = GL_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG;
        }
        else
        {
            internalFormat = GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG;
        }
    }
    else
    {
        if ( canBeCompressedHigh )
        {
            internalFormat = GL_COMPRESSED_RGB_PVRTC_2BPPV1_IMG;
        }
        else
        {
            internalFormat = GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG;
        }
    }

    uint32 mipmapCount = pixelsIn.mipmaps.size();

    {
        using namespace pvrtexture;

        // Determine the source pixel format.
        const PixelType pvrSrcPixelType = PVRStandard8PixelType;

        // Transform the internal format into a pvrtexlib parameter.
        EPVRTPixelFormat compressionPixelType;

        bool gotLibFormat = getPVRCompressionTypeFromInternalFormat( internalFormat, compressionPixelType );

        if ( !gotLibFormat )
        {
            throw RwException( "failed to compress PVRTC due to unknown internalFormat" );
        }

        const PixelType pvrDstPixelType( compressionPixelType );

        // Determine the block dimensions of the PVR destination texture.
        uint32 pvrBlockWidth, pvrBlockHeight;

        uint32 pvrDepth = getDepthByPVRFormat( internalFormat );

        if ( pvrDepth == 2 )
        {
            pvrBlockWidth = 16;
            pvrBlockHeight = 8;
        }
        else if ( pvrDepth == 4 )
        {
            pvrBlockWidth = 8;
            pvrBlockHeight = 8;
        }
        else
        {
            throw RwException( "failed to compress PVRTC due to unknown compression depth" );
        }

        // Pre-allocate the mipmap array.
        pvrTex->mipmaps.resize( mipmapCount );

        for ( uint32 n = 0; n < mipmapCount; n++ )
        {
            // Get parameters of this mipmap layer.
            const pixelDataTraversal::mipmapResource& mipLayer = pixelsIn.mipmaps[ n ];

            // Create a PVR texture.
            uint32 mipWidth = mipLayer.width;
            uint32 mipHeight = mipLayer.height;

            // We need to determine dimensions that the PVR texture has to use.
            uint32 pvrTexWidth = ALIGN_SIZE( mipWidth, pvrBlockWidth );
            uint32 pvrTexHeight = ALIGN_SIZE( mipHeight, pvrBlockHeight );

            const void *srcTexels = mipLayer.texels;

            CPVRTextureHeader pvrHeader( pvrSrcPixelType.PixelTypeID, pvrTexWidth, pvrTexHeight );

            // Copy stuff into the PVR texture properly.
            CPVRTexture pvrTexture( pvrHeader );

            void *pvrDstBuf = pvrTexture.getDataPtr();

            for ( uint32 y = 0; y < pvrTexHeight; y++ )
            {
                for ( uint32 x = 0; x < pvrTexWidth; x++ )
                {
                    bool hasColor = false;

                    uint8 r, g, b, a;

                    if ( x < mipWidth && y < mipHeight )
                    {
                        uint32 colorIndex = PixelFormat::coord2index(x, y, mipWidth);

                        hasColor = browsetexelcolor(
                            srcTexels, srcPaletteType, paletteData, paletteSize, colorIndex, srcRasterFormat, srcColorOrder, srcDepth, r, g, b, a
                        );
                    }

                    if ( !hasColor )
                    {
                        r = 0;
                        g = 0;
                        b = 0;
                        a = 0;
                    }

                    uint32 pvrColorIndex = PixelFormat::coord2index(x, y, pvrTexWidth);

                    puttexelcolor(pvrDstBuf, pvrColorIndex, RASTER_8888, COLOR_RGBA, 32, r, g, b, a);
                }
            }

            // Transcode it.
            bool transcodeSuccess =
                Transcode( pvrTexture, pvrDstPixelType, ePVRTVarTypeUnsignedByteNorm, ePVRTCSpacelRGB );

            assert( transcodeSuccess == true );

            // Put the result into a new mipmap layer.
            NativeTexturePVR::mipmapLayer newLayer;

            newLayer.width = pvrTexWidth;
            newLayer.height = pvrTexHeight;

            newLayer.layerWidth = mipLayer.mipWidth;
            newLayer.layerHeight = mipLayer.mipHeight;

            uint32 dstDataSize = getRasterDataSize(pvrTexWidth * pvrTexHeight, pvrDepth);

            assert(dstDataSize <= pvrTexture.getDataSize());

            newLayer.texels = engineInterface->PixelAllocate( dstDataSize );

            memcpy( newLayer.texels, pvrTexture.getDataPtr(), dstDataSize );

            newLayer.dataSize = dstDataSize;

            // Put the new layer.
            pvrTex->mipmaps[ n ] = newLayer;
        }
    }

    // Store more advanced properties.
    pvrTex->internalFormat = internalFormat;

    // Since we always compress pixels, we cannot directly acquire.
    feedbackOut.hasDirectlyAcquired = false;
}

void pvrNativeTextureTypeProvider::UnsetPixelDataFromTexture( Interface *engineInterface, void *objMem, bool deallocate )
{
    NativeTexturePVR *nativeTex = (NativeTexturePVR*)objMem;

    if ( deallocate )
    {
        // Delete all pixel data.
        deleteMipmapLayers( engineInterface, nativeTex->mipmaps );
    }

    // Clear mipmap data.
    nativeTex->mipmaps.clear();
}

};