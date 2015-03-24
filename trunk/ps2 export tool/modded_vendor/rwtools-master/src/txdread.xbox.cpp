#include <StdInc.h>

#include "txdread.xbox.hxx"

#include "txdread.d3d.hxx"

#include "pixelformat.hxx"

#include "txdread.d3d.dxt.hxx"

#include "txdread.common.hxx"

#include "pluginutil.hxx"

namespace rw
{

void xboxNativeTextureTypeProvider::DeserializeTexture( TextureBase *theTexture, PlatformTexture *nativeTex, BlockProvider& inputProvider ) const
{
    Interface *engineInterface = theTexture->engineInterface;

    // Read the image data chunk.
    {
        BlockProvider texImageDataBlock( &inputProvider );

        texImageDataBlock.EnterContext();

        try
        {
            if ( texImageDataBlock.getBlockID() == CHUNK_STRUCT )
            {
	            uint32 platform = texImageDataBlock.readUInt32();

	            if (platform != PLATFORM_XBOX)
                {
                    throw RwException( "invalid platform flag for XBOX texture native" );
                }

                // Cast to our native texture format.
                NativeTextureXBOX *platformTex = (NativeTextureXBOX*)nativeTex;

                int engineWarningLevel = engineInterface->GetWarningLevel();

                bool engineIgnoreSecureWarnings = engineInterface->GetIgnoreSecureWarnings();

                // Read the main texture header.
                textureMetaHeaderStructXbox metaInfo;
                texImageDataBlock.read( &metaInfo, sizeof(metaInfo) );

                // Read the texture names.
                {
                    char tmpbuf[ sizeof( metaInfo.name ) + 1 ];

                    // Make sure the name buffer is zero terminted.
                    tmpbuf[ sizeof( metaInfo.name ) ] = '\0';

                    // Move over the texture name.
                    memcpy( tmpbuf, metaInfo.name, sizeof( metaInfo.name ) );

                    theTexture->name = tmpbuf;

                    // Move over the texture mask name.
                    memcpy( tmpbuf, metaInfo.maskName, sizeof( metaInfo.maskName ) );

                    theTexture->maskName = tmpbuf;
                }

                // Read format info.
                metaInfo.formatInfo.parse( *theTexture );

                // Deconstruct the rasterFormat flags.
                bool hasMipmaps = false;        // TODO: actually use this flag.
                bool autoMipmaps = false;

                readRasterFormatFlags( metaInfo.rasterFormat, platformTex->rasterFormat, platformTex->paletteType, hasMipmaps, autoMipmaps );

                platformTex->hasAlpha = ( metaInfo.hasAlpha != 0 );

                uint32 depth = metaInfo.depth;
                uint32 maybeMipmapCount = metaInfo.mipmapCount;

                platformTex->depth = depth;

                platformTex->rasterType = metaInfo.rasterType;

                platformTex->dxtCompression = metaInfo.dxtCompression;

                platformTex->autoMipmaps = autoMipmaps;

                ePaletteType paletteType = platformTex->paletteType;

                // Decide what color order this texture uses.
                eColorOrdering texColorOrder = COLOR_BGRA;

                if ( paletteType != PALETTE_NONE )
                {
                    texColorOrder = COLOR_RGBA;
                }

                platformTex->colorOrder = texColorOrder;

                // Verify the parameters.
                eRasterFormat rasterFormat = platformTex->rasterFormat;

                // - depth
                {
                    bool isValidDepth = true;

                    if (paletteType == PALETTE_4BIT)
                    {
                        if (depth != 4 && depth != 8)
                        {
                            isValidDepth = false;
                        }
                    }
                    else if (paletteType == PALETTE_8BIT)
                    {
                        if (depth != 8)
                        {
                            isValidDepth = false;
                        }
                    }
                    // TODO: find more ways of verification here.

                    if (isValidDepth == false)
                    {
                        // We cannot repair a broken depth.
                        throw RwException( "texture " + theTexture->name + " has an invalid depth" );
                    }
                }

                uint32 remainingImageSectionData = metaInfo.imageDataSectionSize;

                if (paletteType != PALETTE_NONE)
                {
                    uint32 palItemCount = getD3DPaletteCount(paletteType);

                    uint32 palDepth = Bitmap::getRasterFormatDepth( rasterFormat );

                    uint32 paletteDataSize = getRasterDataSize( palItemCount, palDepth );

                    // Do we have palette data in the stream?
                    texImageDataBlock.check_read_ahead( paletteDataSize );

	                void *palData = engineInterface->PixelAllocate( paletteDataSize );

                    try
                    {
	                    texImageDataBlock.read( palData, paletteDataSize );
                    }
                    catch( ... )
                    {
                        engineInterface->PixelFree( palData );

                        throw;
                    }

                    // Write the parameters.
                    platformTex->palette = palData;
	                platformTex->paletteSize = palItemCount;
                }

                // Read all the textures.
                uint32 actualMipmapCount = 0;

                mipGenLevelGenerator mipLevelGen( metaInfo.width, metaInfo.height );

                if ( !mipLevelGen.isValidLevel() )
                {
                    throw RwException( "texture " + theTexture->name + " has invalid dimensions" );
                }

                uint32 dxtCompression = platformTex->dxtCompression;

                bool hasZeroSizedMipmaps = false;

                for (uint32 i = 0; i < maybeMipmapCount; i++)
                {
                    if ( remainingImageSectionData == 0 )
                    {
                        break;
                    }

                    bool couldIncrementLevel = true;

	                if (i > 0)
                    {
                        couldIncrementLevel = mipLevelGen.incrementLevel();
                    }

                    if ( !couldIncrementLevel )
                    {
                        break;
                    }

                    // If any dimension is zero, ignore that mipmap.
                    if ( !mipLevelGen.isValidLevel() )
                    {
                        hasZeroSizedMipmaps = true;
                        break;
                    }

                    // Start a new mipmap layer.
                    NativeTextureXBOX::mipmapLayer newLayer;

                    newLayer.layerWidth = mipLevelGen.getLevelWidth();
                    newLayer.layerHeight = mipLevelGen.getLevelHeight();

                    // Process dimensions.
                    uint32 texWidth = newLayer.layerWidth;
                    uint32 texHeight = newLayer.layerHeight;
                    {
		                // DXT compression works on 4x4 blocks,
		                // align the dimensions.
		                if (dxtCompression != 0)
                        {
			                texWidth = ALIGN_SIZE( texWidth, 4u );
                            texHeight = ALIGN_SIZE( texHeight, 4u );
		                }
                    }

                    newLayer.width = texWidth;
                    newLayer.height = texHeight;

                    // Calculate the data size of this mipmap.
                    uint32 texUnitCount = ( texWidth * texHeight );
                    uint32 texDataSize = 0;

                    if (dxtCompression != 0)
                    {
                        uint32 dxtType = 0;

	                    if (dxtCompression == 0xC)	// DXT1 (?)
                        {
                            dxtType = 1;
                        }
                        else if (dxtCompression == 0xD)
                        {
                            dxtType = 2;
                        }
                        else if (dxtCompression == 0xE)
                        {
                            dxtType = 3;
                        }
                        else if (dxtCompression == 0xF)
                        {
                            dxtType = 4;
                        }
                        else if (dxtCompression == 0x10)
                        {
                            dxtType = 5;
                        }
                        else
                        {
                            throw RwException( "texture " + theTexture->name + " has an unknown compression type" );
                        }
            	        
                        texDataSize = getDXTRasterDataSize(dxtType, texUnitCount);
                    }
                    else
                    {
                        // There should also be raw rasters supported.
                        texDataSize = getRasterDataSize(texUnitCount, depth);
                    }

                    if ( remainingImageSectionData < texDataSize )
                    {
                        throw RwException( "texture " + theTexture->name + " has an invalid image data section size parameter" );
                    }

                    // Decrement the overall remaining size.
                    remainingImageSectionData -= texDataSize;

                    // Store the texture size.
                    newLayer.dataSize = texDataSize;

                    // Do we have texel data in the stream?
                    texImageDataBlock.check_read_ahead( texDataSize );

                    // Fetch the texels.
                    newLayer.texels = engineInterface->PixelAllocate( texDataSize );

                    try
                    {
	                    texImageDataBlock.read( newLayer.texels, texDataSize );
                    }
                    catch( ... )
                    {
                        engineInterface->PixelFree( newLayer.texels );

                        throw;
                    }

                    // Store the layer.
                    platformTex->mipmaps.push_back( newLayer );

                    // Increase mipmap count.
                    actualMipmapCount++;
                }

                if ( actualMipmapCount == 0 )
                {
                    throw RwException( "texture " + theTexture->name + " is empty" );
                }

                if ( remainingImageSectionData != 0 )
                {
                    if ( engineWarningLevel >= 2 )
                    {
                        engineInterface->PushWarning( "texture " + theTexture->name + " appears to have image section meta-data" );
                    }

                    // Skip those bytes.
                    texImageDataBlock.skip( remainingImageSectionData );
                }

                if ( hasZeroSizedMipmaps )
                {
                    if ( !engineIgnoreSecureWarnings )
                    {
                        engineInterface->PushWarning( "texture " + theTexture->name + " has zero sized mipmaps" );
                    }
                }

                // Fix filtering mode.
                fixFilteringMode( *theTexture, actualMipmapCount );

                // Fix the auto-mipmap flag.
                {
                    bool hasAutoMipmaps = platformTex->autoMipmaps;

                    if ( hasAutoMipmaps )
                    {
                        bool canHaveAutoMipmaps = ( actualMipmapCount == 1 );

                        if ( !canHaveAutoMipmaps )
                        {
                            engineInterface->PushWarning( "texture " + theTexture->name + " has an invalid auto-mipmap flag (fixing)" );

                            platformTex->autoMipmaps = false;
                        }
                    }
                }
            }
            else
            {
                engineInterface->PushWarning( "could not find texture image data struct in XBOX texture native" );
            }
        }
        catch( ... )
        {
            texImageDataBlock.LeaveContext();

            throw;
        }

        texImageDataBlock.LeaveContext();
    }

    // Now the extensions.
    engineInterface->DeserializeExtensions( theTexture, inputProvider );
}

static PluginDependantStructRegister <xboxNativeTextureTypeProvider, RwInterfaceFactory_t> xboxNativeTexturePlugin( engineFactory );

void registerXBOXNativeTexture( Interface *engineInterface )
{
    xboxNativeTextureTypeProvider *xboxTexEnv = xboxNativeTexturePlugin.GetPluginStruct( (EngineInterface*)engineInterface );

    if ( xboxTexEnv )
    {
        RegisterNativeTextureType( engineInterface, "XBOX", xboxTexEnv, sizeof( NativeTextureXBOX ) );
    }
}

inline void copyPaletteData(
    Interface *engineInterface,
    ePaletteType srcPaletteType, void *srcPaletteData, uint32 srcPaletteSize, eRasterFormat srcRasterFormat,
    bool isNewlyAllocated,
    ePaletteType& dstPaletteType, void*& dstPaletteData, uint32& dstPaletteSize
)
{
    dstPaletteType = srcPaletteType;

    if ( dstPaletteType != PALETTE_NONE )
    {
        dstPaletteSize = srcPaletteSize;

        if ( isNewlyAllocated )
        {
            // Create a new copy.
            uint32 palRasterDepth = Bitmap::getRasterFormatDepth( srcRasterFormat );

            uint32 palDataSize = getRasterDataSize( dstPaletteSize, palRasterDepth );

            dstPaletteData = engineInterface->PixelAllocate( palDataSize );

            memcpy( dstPaletteData, srcPaletteData, palDataSize );
        }
        else
        {
            // Just move it over.
            dstPaletteData = srcPaletteData;
        }
    }
}

void xboxNativeTextureTypeProvider::GetPixelDataFromTexture( Interface *engineInterface, void *objMem, pixelDataTraversal& pixelsOut )
{
    // Cast to our native format.
    NativeTextureXBOX *platformTex = (NativeTextureXBOX*)objMem;

    // We need to find out how to store the texels into the traversal containers.
    // If we store compressed image data, we can give the data directly to the runtime.
    eCompressionType rwCompressionType = RWCOMPRESS_NONE;

    uint32 srcCompressionType = platformTex->dxtCompression;

    eRasterFormat dstRasterFormat = platformTex->rasterFormat;

    bool hasAlpha = platformTex->hasAlpha;

    bool isSwizzledFormat = false;

    bool canHavePalette = false;

    if (srcCompressionType != 0)
    {
        if (srcCompressionType == 0xc)
        {
            rwCompressionType = RWCOMPRESS_DXT1;

		    if (hasAlpha)
            {
			    dstRasterFormat = RASTER_1555;
            }
		    else
            {
			    dstRasterFormat = RASTER_565;
            }

            isSwizzledFormat = false;
	    }
        else if (srcCompressionType == 0xe)
        {
		    rwCompressionType = RWCOMPRESS_DXT3;

            dstRasterFormat = RASTER_4444;

            isSwizzledFormat = false;
        }
        else if (srcCompressionType == 0xf)
        {
		    rwCompressionType = RWCOMPRESS_DXT4;

            dstRasterFormat = RASTER_4444;

            isSwizzledFormat = false;
	    }
        else if (srcCompressionType == 0x10)
        {
            rwCompressionType = RWCOMPRESS_DXT5;

            dstRasterFormat = RASTER_4444;

            isSwizzledFormat = false;
        }
        else
        {
            // TODO: DXT2, DXT5
            assert( 0 );
        }
    }
    else
    {
        // If there is no compression, the D3DFORMAT field is made up of the rasterFormat.
        isSwizzledFormat = true;

        // We can have a palette.
        canHavePalette = true;
    }

    // If we are swizzled, then we have to create a new copy of the texels which is in linear format.
    // Otherwise we can optimize.
    bool isNewlyAllocated = ( isSwizzledFormat == true );

    uint32 mipmapCount = platformTex->mipmaps.size();

    uint32 depth = platformTex->depth;

    // Allocate virtual mipmaps.
    pixelsOut.mipmaps.resize( mipmapCount );

    for ( uint32 n = 0; n < mipmapCount; n++ )
    {
        const NativeTextureXBOX::mipmapLayer& mipLayer = platformTex->mipmaps[ n ];

        // Fetch all mipmap data onto the stack.
        uint32 mipWidth = mipLayer.width;
        uint32 mipHeight = mipLayer.height;

        uint32 layerWidth = mipLayer.layerWidth;
        uint32 layerHeight = mipLayer.layerHeight;

        void *dstTexels = NULL;
        uint32 dstDataSize = 0;

        if ( isSwizzledFormat == false )
        {
            // We can simply move things over.
            dstTexels = mipLayer.texels;
            dstDataSize = mipLayer.dataSize;
        }
        else
        {
            // Here we have to put the pixels into a linear format.
            NativeTextureXBOX::swizzleMipmapTraversal swizzleTrav;

            swizzleTrav.mipWidth = mipWidth;
            swizzleTrav.mipHeight = mipHeight;
            swizzleTrav.depth = depth;
            swizzleTrav.texels = mipLayer.texels;
            swizzleTrav.dataSize = mipLayer.dataSize;

            NativeTextureXBOX::unswizzleMipmap( engineInterface, swizzleTrav );

            assert( swizzleTrav.newtexels != swizzleTrav.texels );

            mipWidth = swizzleTrav.newWidth;
            mipHeight = swizzleTrav.newHeight;
            dstTexels = swizzleTrav.newtexels;
            dstDataSize = swizzleTrav.newDataSize;
        }

        // Create a new virtual mipmap layer.
        pixelDataTraversal::mipmapResource newLayer;

        newLayer.width = mipWidth;
        newLayer.height = mipHeight;

        newLayer.mipWidth = layerWidth;
        newLayer.mipHeight = layerHeight;

        newLayer.texels = dstTexels;
        newLayer.dataSize = dstDataSize;

        // Store this layer.
        pixelsOut.mipmaps[ n ] = newLayer;
    }

    // If we can have a palette, copy it over.
    ePaletteType dstPaletteType = PALETTE_NONE;
    void *dstPaletteData = NULL;
    uint32 dstPaletteSize = 0;

    if ( canHavePalette )
    {
        copyPaletteData(
            engineInterface,
            platformTex->paletteType, platformTex->palette, platformTex->paletteSize,
            dstRasterFormat, isNewlyAllocated,
            dstPaletteType, dstPaletteData, dstPaletteSize
        );
    }

    // Copy over general raster data.
    pixelsOut.rasterFormat = dstRasterFormat;
    pixelsOut.depth = depth;
    pixelsOut.colorOrder = platformTex->colorOrder;

    pixelsOut.compressionType = rwCompressionType;

    // Give it the palette data.
    pixelsOut.paletteData = dstPaletteData;
    pixelsOut.paletteSize = dstPaletteSize;
    pixelsOut.paletteType = dstPaletteType;

    // Copy over more advanced parameters.
    pixelsOut.hasAlpha = hasAlpha;
    pixelsOut.autoMipmaps = platformTex->autoMipmaps;
    pixelsOut.cubeTexture = false;  // XBOX never has cube textures.
    pixelsOut.rasterType = platformTex->rasterType;

    // Tell the runtime whether we newly allocated those texels.
    pixelsOut.isNewlyAllocated = isNewlyAllocated;
}

void xboxNativeTextureTypeProvider::SetPixelDataToTexture( Interface *engineInterface, void *objMem, const pixelDataTraversal& pixelsIn, acquireFeedback_t& feedbackOut )
{
    // Cast to our native texture container.
    NativeTextureXBOX *xboxTex = (NativeTextureXBOX*)objMem;

    // Clear any previous image data.
    xboxTex->clearTexelData();

    // Move over the texture data.
    eCompressionType rwCompressionType = pixelsIn.compressionType;

    uint32 xboxCompressionType = 0;

    eRasterFormat dstRasterFormat = pixelsIn.rasterFormat;

    bool requiresSwizzling = false;

    bool canHavePaletteData = false;

    if ( rwCompressionType != RWCOMPRESS_NONE )
    {
        if ( rwCompressionType == RWCOMPRESS_DXT1 )
        {
            xboxCompressionType = 0xC;
        }
        else if ( rwCompressionType == RWCOMPRESS_DXT2 )
        {
            xboxCompressionType = 0xD;
        }
        else if ( rwCompressionType == RWCOMPRESS_DXT3 )
        {
            xboxCompressionType = 0xE;
        }
        else if ( rwCompressionType == RWCOMPRESS_DXT4 )
        {
            xboxCompressionType = 0xF;
        }
        else if ( rwCompressionType == RWCOMPRESS_DXT5 )
        {
            xboxCompressionType = 0x10;
        }
        else
        {
            assert( 0 );
        }

        // Compressed rasters are what they are.
        // They do not need swizzling.
        requiresSwizzling = false;

        canHavePaletteData = false;
    }
    else
    {
        // Raw bitmap rasters are always swizzled.
        requiresSwizzling = true;

        canHavePaletteData = true;
    }

    // Store texel data.
    // If we require swizzling, we cannot directly acquire the data.
    bool hasDirectlyAcquired = ( requiresSwizzling == false );

    uint32 mipmapCount = pixelsIn.mipmaps.size();

    uint32 depth = pixelsIn.depth;

    // Allocate the mipmaps on the XBOX texture.
    xboxTex->mipmaps.resize( mipmapCount );

    for ( uint32 n = 0; n < mipmapCount; n++ )
    {
        const pixelDataTraversal::mipmapResource& srcLayer = pixelsIn.mipmaps[ n ];

        // Grab the pixel information onto the stack.
        uint32 mipWidth = srcLayer.width;
        uint32 mipHeight = srcLayer.height;

        uint32 layerWidth = srcLayer.mipWidth;
        uint32 layerHeight = srcLayer.mipHeight;

        // The destination texels.
        void *dstTexels = NULL;
        uint32 dstDataSize = 0;

        // If we require swizzling, then we need to allocate a new copy of the texels
        // as destination buffer.
        if ( requiresSwizzling )
        {
            NativeTextureXBOX::swizzleMipmapTraversal swizzleTrav;

            swizzleTrav.mipWidth = mipWidth;
            swizzleTrav.mipHeight = mipHeight;
            swizzleTrav.depth = depth;
            swizzleTrav.texels = srcLayer.texels;
            swizzleTrav.dataSize = srcLayer.dataSize;

            NativeTextureXBOX::unswizzleMipmap( engineInterface, swizzleTrav );

            assert( swizzleTrav.texels != swizzleTrav.newtexels );

            // Update with the swizzled parameters.
            mipWidth = swizzleTrav.newWidth;
            mipHeight = swizzleTrav.newHeight;
            dstTexels = swizzleTrav.newtexels;
            dstDataSize = swizzleTrav.newDataSize;
        }
        else
        {
            // Just move the texels over.
            dstTexels = srcLayer.texels;
            dstDataSize = srcLayer.dataSize;
        }

        // Store it as mipmap.
        NativeTextureXBOX::mipmapLayer& newLayer = xboxTex->mipmaps[ n ];

        newLayer.width = mipWidth;
        newLayer.height = mipHeight;

        newLayer.layerWidth = layerWidth;
        newLayer.layerHeight = layerHeight;

        newLayer.texels = dstTexels;
        newLayer.dataSize = dstDataSize;
    }

    // Also copy over the palette data.
    ePaletteType dstPaletteType = PALETTE_NONE;
    void *dstPaletteData = NULL;
    uint32 dstPaletteSize = 0;

    if ( canHavePaletteData )
    {
        copyPaletteData(
            engineInterface,
            pixelsIn.paletteType, pixelsIn.paletteData, pixelsIn.paletteSize,
            dstRasterFormat, hasDirectlyAcquired == false,
            dstPaletteType, dstPaletteData, dstPaletteSize
        );
    }

    // Copy general raster information.
    xboxTex->rasterFormat = dstRasterFormat;
    xboxTex->depth = depth;
    xboxTex->colorOrder = pixelsIn.colorOrder;

    // Store the palette.
    xboxTex->paletteType = dstPaletteType;
    xboxTex->palette = dstPaletteData;
    xboxTex->paletteSize = dstPaletteSize;

    // Copy more advanced properties.
    xboxTex->rasterType = pixelsIn.rasterType;
    xboxTex->hasAlpha = pixelsIn.hasAlpha;
    xboxTex->autoMipmaps = pixelsIn.autoMipmaps;

    xboxTex->dxtCompression = xboxCompressionType;

    // Since we have to swizzle the pixels, we cannot directly acquire the texels.
    // If the input was compressed though, we can directly take the pixels.
    feedbackOut.hasDirectlyAcquired = hasDirectlyAcquired;
}

void xboxNativeTextureTypeProvider::UnsetPixelDataFromTexture( Interface *engineInterface, void *objMem, bool deallocate )
{
    NativeTextureXBOX *nativeTex = (NativeTextureXBOX*)objMem;

    if ( deallocate )
    {
        // We simply deallocate everything.
        if ( void *palette = nativeTex->palette )
        {
            engineInterface->PixelFree( palette );
        }

        deleteMipmapLayers( engineInterface, nativeTex->mipmaps );
    }

    // Clear all connections.
    nativeTex->palette = NULL;
    nativeTex->paletteSize = 0;
    nativeTex->paletteType = PALETTE_NONE;

    nativeTex->mipmaps.clear();

    // Reset raster parameters for debugging purposes.
    nativeTex->rasterFormat = RASTER_DEFAULT;
    nativeTex->depth = 0;
    nativeTex->colorOrder = COLOR_BGRA;
    nativeTex->hasAlpha = false;
    nativeTex->autoMipmaps = false;
    nativeTex->dxtCompression = 0;
}

};