#include <StdInc.h>

#include "txdread.d3d.hxx"

#include "streamutil.hxx"

namespace rw
{

eTexNativeCompatibility d3dNativeTextureTypeProvider::IsCompatibleTextureBlock( BlockProvider& inputProvider ) const
{
    eTexNativeCompatibility texCompat = RWTEXCOMPAT_NONE;

    BlockProvider texNativeImageBlock( &inputProvider );

    texNativeImageBlock.EnterContext();

    try
    {
        if ( texNativeImageBlock.getBlockID() == CHUNK_STRUCT )
        {
            // For now, simply check the platform descriptor.
            // It can either be Direct3D 8 or Direct3D 9.
            uint32 platformDescriptor = texNativeImageBlock.readUInt32();

            if ( platformDescriptor == PLATFORM_D3D8 )
            {
                // This is a sure guess.
                texCompat = RWTEXCOMPAT_ABSOLUTE;
            }
            else if ( platformDescriptor == PLATFORM_D3D9 )
            {
                // Since Wardrum Studios has broken the platform descriptor rules, we can only say "maybe".
                texCompat = RWTEXCOMPAT_MAYBE;
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

void d3dNativeTextureTypeProvider::SerializeTexture( TextureBase *theTexture, PlatformTexture *nativeTex, BlockProvider& outputProvider ) const
{
    Interface *engineInterface = theTexture->engineInterface;

    NativeTextureD3D *platformTex = (NativeTextureD3D*)nativeTex;

    // Make sure the texture has some qualities before it can even be written.
    ePaletteType paletteType = platformTex->paletteType;

    uint32 compressionType = platformTex->dxtCompression;

    // Get the platform of our texture.
    NativeTextureD3D::ePlatformType platformType = platformTex->platformType;

    if ( platformType == NativeTextureD3D::PLATFORM_D3D8 )
    {
        // Check the color order if we are not compressed.
        if ( compressionType == 0 )
        {
            eColorOrdering requiredColorOrder = COLOR_BGRA;

            if ( paletteType != PALETTE_NONE )
            {
                requiredColorOrder = COLOR_RGBA;
            }

            if ( platformTex->colorOrdering != requiredColorOrder )
            {
                throw RwException( "texture " + theTexture->name + " has an invalid color order for writing" );
            }
        }
    }
    else if ( platformType == NativeTextureD3D::PLATFORM_D3D9 )
    {
        if ( !platformTex->hasD3DFormat )
        {
            throw RwException( "texture " + theTexture->name + " has no representation in Direct3D 9" );
        }
    }
    else
    {
        throw RwException( "unknown Direct3D platform for serialization" );
    }

    uint32 serializePlatform;

    if ( platformType == NativeTextureD3D::PLATFORM_D3D8 )
    {
        serializePlatform = 8;
    }
    else if ( platformType == NativeTextureD3D::PLATFORM_D3D9 )
    {
        serializePlatform = 9;
    }
    else
    {
        assert( 0 );
    }

	// Struct
	{
		BlockProvider texNativeImageStruct( &outputProvider );

        texNativeImageStruct.EnterContext();

        try
        {
            d3d::textureMetaHeaderStructGeneric metaHeader;
            metaHeader.platformDescriptor = serializePlatform;
            metaHeader.texFormat.set( *theTexture );

            // Correctly write the name strings (for safety).
            // Even though we can read those name fields with zero-termination safety,
            // the engines are not guarranteed to do so.
            // Also, print a warning if the name is changed this way.
            writeStringIntoBufferSafe( engineInterface, theTexture->name, metaHeader.name, sizeof( metaHeader.name ), theTexture->name, "name" );
            writeStringIntoBufferSafe( engineInterface, theTexture->maskName, metaHeader.maskName, sizeof( metaHeader.maskName ), theTexture->name, "mask name" );

            // Construct raster flags.
            uint32 mipmapCount = platformTex->mipmaps.size();

            metaHeader.rasterFormat = generateRasterFormatFlags( platformTex->rasterFormat, paletteType, mipmapCount > 1, platformTex->autoMipmaps );

            texNativeImageStruct.write( &metaHeader, sizeof(metaHeader) );

            if (platformType == NativeTextureD3D::PLATFORM_D3D8)
            {
			    texNativeImageStruct.writeUInt32( platformTex->hasAlpha );
		    }
            else if (platformType == NativeTextureD3D::PLATFORM_D3D9)
            {
		        texNativeImageStruct.writeUInt32( (uint32)platformTex->d3dFormat );
		    }
            else
            {
                assert( 0 );
            }

            d3d::textureMetaHeaderStructDimInfo dimInfo;
            dimInfo.width = platformTex->mipmaps[ 0 ].layerWidth;
            dimInfo.height = platformTex->mipmaps[ 0 ].layerHeight;
            dimInfo.depth = platformTex->depth;
            dimInfo.mipmapCount = mipmapCount;
            dimInfo.rasterType = platformTex->rasterType;
            dimInfo.pad1 = 0;

            texNativeImageStruct.write( &dimInfo, sizeof(dimInfo) );

		    if (platformType == NativeTextureD3D::PLATFORM_D3D8)
            {
			    texNativeImageStruct.writeUInt8( compressionType );
            }
            else if (platformType == NativeTextureD3D::PLATFORM_D3D9)
            {
                d3d::textureContentInfoStruct contentInfo;
                contentInfo.hasAlpha = platformTex->hasAlpha;
                contentInfo.isCubeTexture = platformTex->isCubeTexture;
                contentInfo.autoMipMaps = platformTex->autoMipmaps;
                contentInfo.isCompressed = ( compressionType != 0 );
                contentInfo.pad = 0;

                texNativeImageStruct.write( (const char*)&contentInfo, sizeof(contentInfo) );
            }

		    /* Palette */
		    if (paletteType != PALETTE_NONE)
            {
                // Make sure we write as much data as the system expects.
                uint32 reqPalCount = getD3DPaletteCount(paletteType);

                uint32 palItemCount = platformTex->paletteSize;

                // Get the real data size of the palette.
                uint32 palRasterDepth = Bitmap::getRasterFormatDepth(platformTex->rasterFormat);

                uint32 paletteDataSize = getRasterDataSize( palItemCount, palRasterDepth );

                uint32 palByteWriteCount = writePartialBlockSafe(texNativeImageStruct, platformTex->palette, paletteDataSize, getRasterDataSize(reqPalCount, palRasterDepth));
        
                assert( palByteWriteCount * 8 / palRasterDepth == reqPalCount );
		    }

		    /* Texels */
		    for (uint32 i = 0; i < mipmapCount; i++)
            {
                const NativeTextureD3D::mipmapLayer& mipLayer = platformTex->mipmaps[ i ];

			    uint32 texDataSize = mipLayer.dataSize;

			    texNativeImageStruct.writeUInt32( texDataSize );

                const void *texelData = mipLayer.texels;

			    texNativeImageStruct.write( texelData, texDataSize );
		    }
        }
        catch( ... )
        {
            texNativeImageStruct.LeaveContext();

            throw;
        }

        texNativeImageStruct.LeaveContext();
	}

	// Extension
	engineInterface->SerializeExtensions( theTexture, outputProvider );
}

// Pixel movement functions.
void d3dNativeTextureTypeProvider::GetPixelDataFromTexture( Interface *engineInterface, void *objMem, pixelDataTraversal& pixelsOut )
{
    // Cast to our native texture type.
    NativeTextureD3D *platformTex = (NativeTextureD3D*)objMem;

    // The pixel capabilities system has been mainly designed around PC texture optimization.
    // This means that we should be able to directly copy the Direct3D surface data into pixelsOut.
    // If not, we need to adjust, make a new library version.

    // We need to decide how to traverse palette runtime optimization data.

    // Determine the compression type.
    eCompressionType rwCompressionType = RWCOMPRESS_NONE;

    uint32 dxtType = platformTex->dxtCompression;

    if ( dxtType != 0 )
    {
        if ( dxtType == 1 )
        {
            rwCompressionType = RWCOMPRESS_DXT1;
        }
        else if ( dxtType == 2 )
        {
            rwCompressionType = RWCOMPRESS_DXT2;
        }
        else if ( dxtType == 3 )
        {
            rwCompressionType = RWCOMPRESS_DXT3;
        }
        else if ( dxtType == 4 )
        {
            rwCompressionType = RWCOMPRESS_DXT4;
        }
        else if ( dxtType == 5 )
        {
            rwCompressionType = RWCOMPRESS_DXT5;
        }
        else
        {
            throw RwException( "unsupported DXT compression" );
        }
    }

    // Put ourselves into the pixelsOut struct!
    pixelsOut.rasterFormat = platformTex->rasterFormat;
    pixelsOut.depth = platformTex->depth;
    pixelsOut.colorOrder = platformTex->colorOrdering;
    pixelsOut.paletteType = platformTex->paletteType;
    pixelsOut.paletteData = platformTex->palette;
    pixelsOut.paletteSize = platformTex->paletteSize;
    pixelsOut.compressionType = rwCompressionType;
    pixelsOut.hasAlpha = platformTex->hasAlpha;

    pixelsOut.autoMipmaps = platformTex->autoMipmaps;
    pixelsOut.cubeTexture = platformTex->isCubeTexture;
    pixelsOut.rasterType = platformTex->rasterType;

    // Now, the texels.
    uint32 mipmapCount = platformTex->mipmaps.size();

    pixelsOut.mipmaps.resize( mipmapCount );

    for ( uint32 n = 0; n < mipmapCount; n++ )
    {
        const NativeTextureD3D::mipmapLayer& srcLayer = platformTex->mipmaps[ n ];

        pixelDataTraversal::mipmapResource newLayer;

        newLayer.width = srcLayer.width;
        newLayer.height = srcLayer.height;
        newLayer.mipWidth = srcLayer.layerWidth;
        newLayer.mipHeight = srcLayer.layerHeight;

        newLayer.texels = srcLayer.texels;
        newLayer.dataSize = srcLayer.dataSize;

        // Put this layer.
        pixelsOut.mipmaps[ n ] = newLayer;
    }

    // We never allocate new texels, actually.
    pixelsOut.isNewlyAllocated = false;
}

void d3dNativeTextureTypeProvider::SetPixelDataToTexture( Interface *engineInterface, void *objMem, const pixelDataTraversal& pixelsIn, acquireFeedback_t& feedbackOut )
{
    // We want to remove our current texels and put the new ones into us instead.
    // By chance, the runtime makes sure the texture is already empty.
    // So optimize your routine to that.

    NativeTextureD3D *nativeTex = (NativeTextureD3D*)objMem;

    // Remove our own texels first, since the runtime wants to overwrite them.
    nativeTex->clearTexelData();

    // Determine the compression type.
    uint32 dxtType;

    eCompressionType rwCompressionType = pixelsIn.compressionType;

    eRasterFormat dstRasterFormat = pixelsIn.rasterFormat;

    bool hasAlpha = pixelsIn.hasAlpha;

    if ( rwCompressionType == RWCOMPRESS_NONE )
    {
        // TODO: actually, before we can acquire texels, we MUST make sure they are in
        // a compatible format. If they are not, then we will most likely allocate
        // new pixel information, instead in a compatible format. The same has to be
        // made for the XBOX implementation.

        dxtType = 0;
    }
    else if ( rwCompressionType == RWCOMPRESS_DXT1 )
    {
        // TODO: do we properly handle DXT1 with alpha...?

        dxtType = 1;

        dstRasterFormat = ( hasAlpha ) ? ( RASTER_1555 ) : ( RASTER_565 );
    }
    else if ( rwCompressionType == RWCOMPRESS_DXT2 )
    {
        dxtType = 2;

        dstRasterFormat = RASTER_4444;
    }
    else if ( rwCompressionType == RWCOMPRESS_DXT3 )
    {
        dxtType = 3;

        dstRasterFormat = RASTER_4444;
    }
    else if ( rwCompressionType == RWCOMPRESS_DXT4 )
    {
        dxtType = 4;
        
        dstRasterFormat = RASTER_4444;
    }
    else if ( rwCompressionType == RWCOMPRESS_DXT5 )
    {
        dxtType = 5;

        dstRasterFormat = RASTER_4444;
    }
    else
    {
        throw RwException( "unknown pixel compression type" );
    }

    // Update our own texels.
    nativeTex->rasterFormat = dstRasterFormat;
    nativeTex->depth = pixelsIn.depth;
    nativeTex->palette = pixelsIn.paletteData;
    nativeTex->paletteSize = pixelsIn.paletteSize;
    nativeTex->paletteType = pixelsIn.paletteType;
    nativeTex->colorOrdering = pixelsIn.colorOrder;
    nativeTex->hasAlpha = hasAlpha;

    nativeTex->autoMipmaps = pixelsIn.autoMipmaps;
    nativeTex->isCubeTexture = pixelsIn.cubeTexture;
    nativeTex->rasterType = pixelsIn.rasterType;

    nativeTex->dxtCompression = dxtType;

    bool canDirectlyAcquire = true;

    // Apply the pixel data.
    uint32 mipmapCount = pixelsIn.mipmaps.size();

    nativeTex->mipmaps.resize( mipmapCount );

    for ( uint32 n = 0; n < mipmapCount; n++ )
    {
        const pixelDataTraversal::mipmapResource& srcLayer = pixelsIn.mipmaps[ n ];

        NativeTextureD3D::mipmapLayer newLayer;

        newLayer.width = srcLayer.width;
        newLayer.height = srcLayer.height;
        newLayer.layerWidth = srcLayer.mipWidth;
        newLayer.layerHeight = srcLayer.mipHeight;

        newLayer.texels = srcLayer.texels;
        newLayer.dataSize = srcLayer.dataSize;

        // Store the layer.
        nativeTex->mipmaps[ n ] = newLayer;
    }

    // We need to set the Direct3D format field.
    nativeTex->updateD3DFormat();

    // For now, we can always directly acquire pixels.
    feedbackOut.hasDirectlyAcquired = canDirectlyAcquire;
}

void d3dNativeTextureTypeProvider::UnsetPixelDataFromTexture( Interface *engineInterface, void *objMem, bool deallocate )
{
    // Just remove the pixels from us.
    NativeTextureD3D *nativeTex = (NativeTextureD3D*)objMem;

    if ( deallocate )
    {
        // Delete all pixels.
        deleteMipmapLayers( engineInterface, nativeTex->mipmaps );

        // Delete palette.
        if ( nativeTex->paletteType != PALETTE_NONE )
        {
            if ( void *paletteData = nativeTex->palette )
            {
                engineInterface->PixelFree( paletteData );

                nativeTex->palette = NULL;
            }

            nativeTex->paletteType = PALETTE_NONE;
        }
    }

    // Unset the pixels.
    nativeTex->mipmaps.clear();

    nativeTex->palette = NULL;
    nativeTex->paletteType = PALETTE_NONE;
    nativeTex->paletteSize = 0;

    // Reset general properties for cleanliness.
    nativeTex->rasterFormat = RASTER_DEFAULT;
    nativeTex->depth = 0;
    nativeTex->hasD3DFormat = false;
    nativeTex->dxtCompression = 0;
    nativeTex->hasAlpha = false;
    nativeTex->colorOrdering = COLOR_BGRA;
}

}