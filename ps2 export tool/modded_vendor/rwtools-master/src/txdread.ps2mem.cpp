#include <StdInc.h>

#include "txdread.ps2.hxx"

namespace rw
{

template <typename arrayType>
struct swizzle_crypt
{
private:
    // TODO: improve these parameters.
    // TODO: rewrite this into a PS2 GS memory layout manager.
    static const uint32 SWIZZLE_BLOCK_WIDTH = 2;     // pair amount
    static const uint32 SWIZZLE_BLOCK_HEIGHT = 2;    // pair amount
    static const uint32 SWIZZLE_MAGNITUDE = 2;
    static const uint32 SWIZZLE_MIRROR_LENGTH =  ( SWIZZLE_BLOCK_WIDTH * 2 );
    static const uint32 SWIZZLE_PATCH_LENGTH = 8;
    static const uint32 SWIZZLE_ALIGNMENT_WIDTH = ( SWIZZLE_PATCH_LENGTH * SWIZZLE_MAGNITUDE );
    static const uint32 SWIZZLE_ALIGNMENT_HEIGHT = ( SWIZZLE_PATCH_LENGTH * SWIZZLE_MAGNITUDE );

    static inline uint32 ALIGN_TRANSFORM_FAST( uint32 num, uint32 alignByPowerOfTwo )
    {
        return ( num & ~(alignByPowerOfTwo-1) );
    }

    inline void getswizzlecoord(uint32 x, uint32 y, uint32& swizzleX, uint32& swizzleY)
    {
        // Get a block coordinate; this is an index into the image array whose x and y coords are aligned.
        uint32 block_location_yDim =
            ALIGN_TRANSFORM_FAST( y, SWIZZLE_ALIGNMENT_HEIGHT );
        uint32 block_location_xDim =
            ALIGN_TRANSFORM_FAST( x, SWIZZLE_ALIGNMENT_WIDTH ) * SWIZZLE_BLOCK_HEIGHT;

        // Create a swap pattern along the y axis based on 0011 1100 0011 1100 ...
        uint32 swap_pattern = ( ( y + SWIZZLE_BLOCK_HEIGHT ) / ( 2 * SWIZZLE_BLOCK_HEIGHT ) ) % ( SWIZZLE_BLOCK_HEIGHT );
        
        // This decides whether to swap data with the neighboring block.
        // 0044 4400 0044 4400 ...
        uint32 swap_selector = swap_pattern * ( SWIZZLE_MIRROR_LENGTH );
        
        // Create a position (pattern) along the y axis based on 0101 2323 4545 6767 0101 ...
        uint32 y_pos_pattern =
            ( ALIGN_TRANSFORM_FAST( y, SWIZZLE_BLOCK_HEIGHT * 2 ) / SWIZZLE_BLOCK_HEIGHT ) + ( y % SWIZZLE_BLOCK_HEIGHT );
        
        uint32 ypos =
            ( y_pos_pattern ) % ( SWIZZLE_PATCH_LENGTH );

        // 0202 4646 8,10,8,10 12,14,12,14 0202 ...
        ypos *= ( SWIZZLE_BLOCK_HEIGHT );
        
        // Dimensionalize the ypos parameters (into a texture array index).
        uint32 column_location_yDim =
            ( ypos );
        // 0,4,8,12 16,20,24,28 0,4,8,12 ...
        // 16,20,24,28 0,4,8,12 16,20,24,28 ...
        uint32 column_location_xDim =
            ( ( x + swap_selector ) % ( SWIZZLE_PATCH_LENGTH ) ) * ( SWIZZLE_MIRROR_LENGTH );

        // Create a pattern of 0011 0011 0011 ...
        uint32 byte_sum_y_pattern =
            ( ( y / SWIZZLE_BLOCK_HEIGHT ) & 1 );
            
        // Create a pattern of 0000 0000 2222 2222 0000 0000 2222 ...
        uint32 byte_sum_x_pattern =
            ( ( x / ( SWIZZLE_BLOCK_WIDTH * 2 ) ) & 2 );
        
        // Set this new calculation as an extension on the x Axis.
        uint32 byte_sum_xDim = ( byte_sum_y_pattern + byte_sum_x_pattern );

        // Remember to use a block-font to read this.

        // Effective end permutation (image coords 2 block coords):
        //-------------------------------------------------------------------
        //          rowmanip        first tuplet           second tuplet
        //-------------------------------------------------------------------
        // matrix: (1st->1st): |0,4,8,12 16,20,24,28| |2,6,10,14 18,22,26,30|
        //         (2nd->3rd): |0,4,8,12 16,20,24,28| |2,6,10,14 18,22,26,30|
        //         (3rd->1st): |17,21,25,29 1,5,9,13| |19,23,27,31 3,7,11,15|
        //         (4th->3rd): |17,21,25,29 1,5,9,13| |19,23,27,31 3,7,11,15|
        //
        // matrix: (5th->5th): |16,20,24,28 0,4,8,12| |18,22,26,30 2,6,10,14|
        //         (6th->7th): |16,20,24,28 0,4,8,12| |18,22,26,30 2,6,10,14|
        //         (7th->5th): |1,5,9,13 17,21,25,29| |3,7,11,15 19,23,27,31|
        //         (8th->7th): |1,5,9,13 17,21,25,29| |3,7,11,15 19,23,27,31|
        // 
        uint32 block_coord_yDim = ( column_location_yDim );
        uint32 block_coord_xDim = ( column_location_xDim + byte_sum_xDim );

        // Put everything together into a texture dimension access.
        // This is in swizzle coordinates.
        swizzleY = ( block_location_yDim + block_coord_yDim );
        swizzleX = ( block_location_xDim + block_coord_xDim );
    }

    inline bool getimagecoord(uint32 x, uint32 y, uint32& imgX, uint32& imgY)
    {
        // Reverse permutation (block coords 2 image coords tuples):
        //-------------------------------------------------------------------
        // entries in this matrix are tuples:
        // - first entry: target x coord
        // - second entry: target y row
        //-------------------------------------------------------------------
        // matrix: (1st row): (0,0),(4,2),(8,0),(12,2)
        //                    (1,0),(5,2),(9,0),(13,2)
        //                    (2,0),(6,2),(10,0),(14,2)
        //                    (3,0),(7,2),(11,0),(15,2)
        //                    (4,0),(0,2),(12,0),(8,2)
        //                    (5,0),(1,2),(13,0),(9,2)
        //                    (6,0),(2,2),(14,0),(10,2)
        //                    (7,0),(3,2),(15,0),(11,2)
        //
        // matrix: (3rd row): (0,1),(4,3),(8,1),(12,3)
        //                    (1,1),(5,3),(9,1),(13,3)
        //                    (2,1),(6,3),(10,1),(14,3)
        //                    (3,1),(7,3),(11,1),(15,3)
        //                    (4,1),(0,3),(12,1),(8,3)
        //                    (5,1),(1,3),(13,1),(9,3)
        //                    (6,1),(2,3),(14,1),(10,3)
        //                    (7,1),(3,3),(15,1),(11,3)
        //
        // matrix: (5th row): (4,4),(0,6),(12,4),(8,6)
        //                    (5,4),(1,6),(13,4),(9,6)
        //                    (6,4),(2,6),(14,4),(10,6)
        //                    (7,4),(3,6),(15,4),(11,6)
        //                    (0,4),(4,6),(8,4),(12,6)
        //                    (1,4),(5,6),(9,4),(13,6)
        //                    (2,4),(6,6),(10,4),(14,6)
        //                    (3,4),(7,6),(11,4),(15,6)
        //
        // matrix: (7th row): (4,5),(0,7),(12,5),(8,7)
        //                    (5,5),(1,7),(13,5),(9,7)
        //                    (6,5),(2,7),(14,5),(10,7)
        //                    (7,5),(3,7),(15,5),(11,7)
        //                    (0,5),(4,7),(8,5),(12,7)
        //                    (1,5),(5,7),(9,5),(13,7)
        //                    (2,5),(6,7),(10,5),(14,7)
        //                    (3,5),(7,7),(11,5),(15,7)
        //-------------------------------------------------------------------
        // this algorithm is undefined every two swizzle rows.
        //
        if ( y % 2 == 1 )
            return false;

        // Turn swizzle coordinates back into image coordinates.
        uint32 swizzle_block_yDim = ALIGN_TRANSFORM_FAST( y, SWIZZLE_MIRROR_LENGTH );
        uint32 swizzle_block_xDim = ALIGN_TRANSFORM_FAST( x, SWIZZLE_ALIGNMENT_WIDTH * SWIZZLE_BLOCK_HEIGHT );

        uint32 swizzle_block_coord_yDim = ( y - swizzle_block_yDim );
        uint32 swizzle_block_coord_xDim = ( x - swizzle_block_xDim );

        uint32 matrix_rowindex = ( swizzle_block_coord_xDim / SWIZZLE_MIRROR_LENGTH );
        uint32 matrix_colindex = ( swizzle_block_coord_xDim % SWIZZLE_MIRROR_LENGTH );

        uint32 matrix_colbyrow = ( matrix_colindex * SWIZZLE_PATCH_LENGTH + matrix_rowindex );

        //

        uint32 imgblock_base_xDim =
            ( SWIZZLE_PATCH_LENGTH * ( matrix_colindex / SWIZZLE_BLOCK_WIDTH ) );

        uint32 imgblock_colpos_split =
            ( matrix_colindex & 0x01 ) * SWIZZLE_MIRROR_LENGTH;

        uint32 imgblock_sectormod =
            ( ( y & 0x04 ) / 4 ) * SWIZZLE_MIRROR_LENGTH;

        uint32 imgblock_xDim =
            imgblock_base_xDim + ( matrix_colbyrow - imgblock_base_xDim - imgblock_colpos_split + imgblock_sectormod ) % ( SWIZZLE_PATCH_LENGTH );
        uint32 imgblock_yDim =
            ( matrix_colindex & 1 ) * 2 + ( swizzle_block_coord_yDim & 2 ) / 2;

        // Rescale the block coordinates to image block bounds.
        uint32 image_block_offX = swizzle_block_xDim / SWIZZLE_BLOCK_HEIGHT;
        uint32 image_block_offY = swizzle_block_yDim;

        // Output the target coordinate.
        imgX = ( image_block_offX + imgblock_xDim );
        imgY = ( image_block_offY + imgblock_yDim );
        return true;
    }

    inline void process_unswizzle_image_section(
        const arrayType *srcData, arrayType *dstDecrypt,
        uint32 imgOffX, uint32 imgOffY, uint32 imgSectionWidth, uint32 imgSectionHeight, uint32 imgWidth,
        uint32 swizzleOffX, uint32 swizzleOffY, uint32 swizzleWidth, uint32 swizzleHeight
        )
    {
        for (uint32 y = 0; y < imgSectionHeight; y++)
        {
            for (uint32 x = 0; x < imgSectionWidth; x++)
            {
                // Get real image coordinates.
                uint32 curX = x + imgOffX;
                uint32 curY = y + imgOffY;

                // Get the swap texture location.
                uint32 swizzleX, swizzleY;
                getswizzlecoord(x, y, swizzleX, swizzleY);

                // Get real swizzle coords.
                uint32 swizzleRealX = swizzleX + swizzleOffX;
                uint32 swizzleRealY = swizzleY + swizzleOffY;

                // Assert that the coordinates are still on our allocated plane.
                // If not, then the texture is not a valid swizzle encrypted plane.
                uint32 targetVal = 0;

                if ( swizzleRealX < swizzleWidth && swizzleRealY < swizzleHeight )
                {
                    // Convert it to an array index.
                    uint32 swizzle_index = PixelFormat::coord2index( swizzleRealX, swizzleRealY, swizzleWidth );

                    // Swap the picture data.
                    srcData->getvalue( swizzle_index, targetVal );
                }

                uint32 dest_index = PixelFormat::coord2index( curX, curY, imgWidth );

                dstDecrypt->setvalue( dest_index, targetVal );
            }
        }
    }

    inline void process_swizzle_image_section(
        const arrayType *srcToBeEncryptedData, arrayType *dstEncrypted,
        uint32 imgOffX, uint32 imgOffY, uint32 imgSectionWidth, uint32 imgSectionHeight, uint32 imgWidth,
        uint32 swizzleOffX, uint32 swizzleOffY, uint32 swizzleWidth, uint32 swizzleHeight
        )
    {
        for (uint32 y = 0; y < imgSectionHeight; y++)
        {
            for (uint32 x = 0; x < imgSectionWidth; x++)
            {
                // Get the real image coordinates.
                uint32 curX = x + imgOffX;
                uint32 curY = y + imgOffY;

                uint32 swizzleX, swizzleY;
                getswizzlecoord(x, y, swizzleX, swizzleY);

                // Get the real swizzle coords.
                uint32 swizzleRealX = swizzleX + swizzleOffX;
                uint32 swizzleRealY = swizzleY + swizzleOffY;

                // Convert it to an array index.
                uint32 swizzle_index = PixelFormat::coord2index(swizzleRealX, swizzleRealY, swizzleWidth);

                // Swap the picture data.
                uint32 dest_index = PixelFormat::coord2index( curX, curY, imgWidth );

                uint32 swapWith;

                srcToBeEncryptedData->getvalue( dest_index, swapWith );

                dstEncrypted->setvalue( swizzle_index, swapWith );
            }
        }
    }

public:
    inline arrayType* unswizzle(
        uint32& dstDataSize,
        const arrayType *srcData,
        uint32 imageWidth, uint32 imageHeight,
        uint32 swizzleWidth, uint32 swizzleHeight)
    {
        // The actual image is double the given size.
        // This is because swizzle data is interleaved.
        uint32 swizzle_interleave_width = imageWidth / 2;

        uint32 dataCount = ( imageWidth * imageHeight );

        arrayType *dstDecrypt = arrayType::allocate( dataCount );

        // The swizzled image is interleaved.
        // We need to process it twice.
        process_unswizzle_image_section(
            srcData, dstDecrypt,
            0, 0, swizzle_interleave_width, imageHeight, imageWidth,
            0, 0, swizzleWidth, swizzleHeight
        );
        process_unswizzle_image_section(
            srcData, dstDecrypt,
            swizzle_interleave_width, 0, swizzle_interleave_width, imageHeight, imageWidth,
            0, 1, swizzleWidth, swizzleHeight
        );

        dstDataSize = arrayType::sizeitems( dataCount );

        return dstDecrypt;
    }

    inline arrayType* swizzle(uint32& dstEncryptedSize, const arrayType *srcToBeEncryptedData, uint32 imageWidth, uint32 imageHeight, uint32& outSwizzleWidth, uint32& outSwizzleHeight)
    {
        // The swizzled picture is interleaved.
        // The first and third tracks are taken by the left picture side.
        // The second and fourth tracks are taken by the right side.
        uint32 swizzle_interleave_width = imageWidth / 2;

        // Calculate the required infinite swizzle plane size.
        uint32 swizzleMaxX = 1;
        uint32 swizzleMaxY = 1;

        for (uint32 y = 0; y < imageHeight; y++)
        {
            for (uint32 x = 0; x < swizzle_interleave_width; x++)
            {
                uint32 swizzleX, swizzleY;
                getswizzlecoord(x, y, swizzleX, swizzleY);

                // Get the maximum plane coords.
                if ( swizzleMaxX < swizzleX )
                {
                    swizzleMaxX = swizzleX;
                }

                if ( swizzleMaxY < swizzleY )
                {
                    swizzleMaxY = swizzleY;
                }
            }
        }

        // Get the virtual plane size.
        uint32 swizzleVirtualWidth = swizzleMaxX + 1;
        uint32 swizzleVirtualHeight = swizzleMaxY + 1;

        // Make sure we allow interleaving.
        swizzleVirtualHeight += 1;

        // Transform max coordinates into power-of-two swizzle coords.
        uint32 swizzleWidth = 2;
        uint32 swizzleHeight = 2;

        while ( swizzleWidth < swizzleVirtualWidth )
        {
            swizzleWidth *= 2;
        }

        while ( swizzleHeight < swizzleVirtualHeight )
        {
            swizzleHeight *= 2;
        }

        // Turn the swizzle dimensions into an array size.
        uint32 textureArraySize = ( swizzleWidth * swizzleHeight );

        // Calculate a data size that is a multiple of 16.
        uint32 reqDataCount = ALIGN_SIZE( textureArraySize, (uint32)16 );

        // Generate the encrypted data.
        arrayType *dstEncrypted = arrayType::allocate( reqDataCount );

        process_swizzle_image_section(
            srcToBeEncryptedData, dstEncrypted,
            0, 0, swizzle_interleave_width, imageHeight, imageWidth,
            0, 0, swizzleWidth, swizzleHeight
        );
        process_swizzle_image_section(
            srcToBeEncryptedData, dstEncrypted,
            swizzle_interleave_width, 0, swizzle_interleave_width, imageHeight, imageWidth,
            0, 1, swizzleWidth, swizzleHeight
        );

        // Give the data size to the runtime.
        dstEncryptedSize = arrayType::sizeitems( reqDataCount );

        outSwizzleWidth = swizzleWidth;
        outSwizzleHeight = swizzleHeight;

        return dstEncrypted;
    }
};

uint32 NativeTexturePS2::calculateGPUDataSize(
    const uint32 mipmapBasePointer[], const uint32 mipmapMemorySize[], uint32 mipmapMax,
    eMemoryLayoutType memLayoutType,
    uint32 paletteDataSize
) const
{
    uint32 numMipMaps = this->mipmapCount;

    if ( numMipMaps == 0 )
        return 0;

#if 1
    // Calculate the maximum memory offset required.
    uint32 maxMemOffset = 0;

    for ( uint32 n = 0; n < numMipMaps; n++ )
    {
        uint32 thisOffset = ( mipmapBasePointer[n] + mipmapMemorySize[n] );

        if ( maxMemOffset < thisOffset )
        {
            maxMemOffset = thisOffset;
        }
    }

    uint32 textureMemoryDataSize = ( maxMemOffset * 64 );

    uint32 combinedMemoryDataSize = ALIGN_SIZE( textureMemoryDataSize + paletteDataSize, 2048u );
#else
    // Assume uniform depth.
    uint32 depth = this->mipmapDepth[0];

    uint32 textureMemoryDataSize = 0;

    for (uint32 n = 0; n < numMipMaps; n++)
    {
        uint32 curDataSize = this->dataSizes[n];
        const ps2MipmapUnknowns& mipunk = this->mipmapUnknowns[n];

        textureMemoryDataSize += curDataSize / 4 + mipunk.unk1 + mipunk.unk2;
    }
#endif
#if 0
    uint32 gpuMinMemory = textureMemoryDataSize;

    //if ( ( this->rasterFormat & RASTER_AUTOMIPMAP ) != 0 )
    {
        // Deal with some really obscure cases by defining a minimum memory milestone.
        // We do that for security reasons.
        uint32 minMemoryFootprint = 2048;

        uint32 curSplitStart = 0;

        uint32 depth = this->mipmapDepth[ 0 ];

        if ( depth == 4 )
        {
            curSplitStart = 256;
        }
        else if ( depth == 8 )
        {
            curSplitStart = 128;
        }
        else if ( depth == 32 )
        {
            curSplitStart = 64;
        }

        while ( this->width[0] >= curSplitStart || this->height[0] >= curSplitStart )
        {
            minMemoryFootprint *= 2;
            
            curSplitStart *= 2;
        }

        // Adjust minimum memory.
        gpuMinMemory = std::max( gpuMinMemory, minMemoryFootprint );
    }

    return gpuMinMemory;
#else
    return combinedMemoryDataSize;
#endif
}

bool NativeTexturePS2::swizzleEncryptPS2(uint32 i)
{
    assert(this->isSwizzled[i] == false);

    uint32 newDataSize;
    void *newtexels;
    uint32 depth = this->mipmapDepth[i];
   
    // Get picture meta information.
    uint32 realImageWidth = this->width[i];
    uint32 realImageHeight = this->height[i];
    uint32 encryptedWidth = realImageWidth;
    uint32 encryptedHeight = realImageHeight;

    bool operationSuccessful = false;

    if (depth == 4)
    {
        swizzle_crypt <PixelFormat::palette4bit> crypt;
        newtexels =
            crypt.swizzle(
                newDataSize,
                (PixelFormat::palette4bit*)texels[i],
                realImageWidth, realImageHeight,
                encryptedWidth, encryptedHeight
            );

        operationSuccessful = true;
    }
    else if (depth == 8)
    {
        swizzle_crypt <PixelFormat::palette8bit> crypt;
        newtexels =
            crypt.swizzle(
                newDataSize,
                (PixelFormat::palette8bit*)texels[i],
                realImageWidth, realImageHeight,
                encryptedWidth, encryptedHeight
            );

        operationSuccessful = true;
    }
    else
    {
        assert( 0 );
    }

    if ( operationSuccessful )
    {
        this->dataSizes[i] = newDataSize;

        this->swizzleWidth[i] = encryptedWidth;
        this->swizzleHeight[i] = encryptedHeight;

	    delete[] texels[i];
	    texels[i] = newtexels;

        this->isSwizzled[i] = true;
    }

    return operationSuccessful;
}

bool NativeTexturePS2::swizzleDecryptPS2(uint32 i)
{
    assert(this->isSwizzled[i] == true);

    uint32 newDataSize;
    void *newtexels;
    uint32 depth = this->mipmapDepth[i];
   
    // Get picture meta information.
    uint32 realImageWidth = this->width[i];
    uint32 realImageHeight = this->height[i];
    uint32 encryptedWidth = this->swizzleWidth[i];
    uint32 encryptedHeight = this->swizzleHeight[i];

    bool operationSuccessful = false;

    if (depth == 4)
    {
        swizzle_crypt <PixelFormat::palette4bit> crypt;
        newtexels =
            crypt.unswizzle(
                newDataSize,
                (PixelFormat::palette4bit*)texels[i],
                realImageWidth, realImageHeight,
                encryptedWidth, encryptedHeight
            );

        operationSuccessful = true;
    }
    else if (depth == 8)
    {
        swizzle_crypt <PixelFormat::palette8bit> crypt;
        newtexels =
            crypt.unswizzle(
                newDataSize,
                (PixelFormat::palette8bit*)texels[i],
                realImageWidth, realImageHeight,
                encryptedWidth, encryptedHeight
            );

        operationSuccessful = true;
    }
    else
    {
        assert( 0 );
    }

    if ( operationSuccessful )
    {
        this->dataSizes[i] = newDataSize;

	    delete[] texels[i];
	    texels[i] = newtexels;

        this->isSwizzled[i] = false;
    }

    return operationSuccessful;
}

// The PS2 memory is a rectangular device. Basically its a set of pages that can be used for allocating image chunks.
// This class is supposed to emulate the texture allocation behavior.
namespace ps2GSMemoryLayoutArrangements
{
    // Layout arrangements.
    const static uint32 psmct32[4][8] =
    {
        { 0u, 1u, 4u, 5u, 16u, 17u, 20u, 21u },
        { 2u, 3u, 6u, 7u, 18u, 19u, 22u, 23u },
        { 8u, 9u, 12u, 13u, 24u, 25u, 28u, 29u },
        { 10u, 11u, 14u, 15u, 26u, 27u, 30u, 31u }
    };
    const static uint32 psmz32[4][8] =
    {
        { 24u, 25u, 28u, 29u, 8u, 9u, 12u, 13u },
        { 26u, 27u, 30u, 31u, 10u, 11u, 14u, 15u },
        { 16u, 17u, 20u, 21u, 0u, 1u, 4u, 5u },
        { 18u, 19u, 22u, 23u, 2u, 3u, 6u, 7u }
    };
    const static uint32 psmct16[8][4] =
    {
        { 0u, 2u, 8u, 10u },
        { 1u, 3u, 9u, 11u },
        { 4u, 6u, 12u, 14u },
        { 5u, 7u, 13u, 15u },
        { 16u, 18u, 24u, 26u },
        { 17u, 19u, 25u, 27u },
        { 20u, 22u, 28u, 30u },
        { 21u, 23u, 29u, 31u }
    };
    const static uint32 psmz16[8][4] =
    {
        { 24u, 26u, 16u, 18u },
        { 25u, 27u, 17u, 19u },
        { 28u, 30u, 20u, 22u },
        { 29u, 31u, 21u, 23u },
        { 8u, 10u, 0u, 2u },
        { 9u, 11u, 1u, 3u },
        { 12u, 14u, 4u, 6u },
        { 13u, 15u, 5u, 7u }
    };
    const static uint32 psmct16s[8][4] =
    {
        { 0u, 2u, 16u, 18u },
        { 1u, 3u, 17u, 19u },
        { 8u, 10u, 24u, 26u },
        { 9u, 11u, 25u, 27u },
        { 4u, 6u, 20u, 22u },
        { 5u, 7u, 21u, 23u },
        { 12u, 14u, 28u, 30u },
        { 13u, 15u, 29u, 31u }
    };
    const static uint32 psmz16s[8][4] =
    {
        { 24u, 26u, 8u, 10u },
        { 25u, 27u, 9u, 11u },
        { 16u, 18u, 0u, 2u },
        { 17u, 19u, 1u, 3u },
        { 28u, 30u, 12u, 14u },
        { 29u, 31u, 13u, 15u },
        { 20u, 22u, 4u, 6u },
        { 21u, 23u, 5u, 7u }
    };
    const static uint32 psmt8[4][8] =
    {
        { 0u, 1u, 4u, 5u, 16u, 17u, 20u, 21u },
        { 2u, 3u, 6u, 7u, 18u, 19u, 22u, 23u },
        { 8u, 9u, 12u, 13u, 24u, 25u, 28u, 29u },
        { 10u, 11u, 14u, 15u, 26u, 27u, 30u, 31u }
    };
    const static uint32 psmt4[8][4] =
    {
        { 0u, 2u, 8u, 10u },
        { 1u, 3u, 9u, 11u },
        { 4u, 6u, 12u, 14u },
        { 5u, 7u, 13u, 15u },
        { 16u, 18u, 24u, 26u },
        { 17u, 19u, 25u, 27u },
        { 20u, 22u, 28u, 30u },
        { 21u, 23u, 29u, 31u }
    };
};

struct ps2GSMemoryLayoutManager
{
    inline static bool getMemoryLayoutFromPaletteType(ePaletteType paletteType, eMemoryLayoutType& memLayout)
    {
        eMemoryLayoutType theLayout;

        if ( paletteType == PALETTE_4BIT )
        {
            theLayout = PSMT4;
        }
        else if ( paletteType == PALETTE_8BIT )
        {
            theLayout = PSMT8;
        }
        else
        {
            return false;
        }

        memLayout = theLayout;

        return true;
    }

    inline static bool getMemoryLayoutFromRasterFormat(eRasterFormat rasterFormat, eMemoryLayoutType& memLayout)
    {
        eMemoryLayoutType theLayout;

        if ( rasterFormat == RASTER_1555 ||
             rasterFormat == RASTER_565 ||
             rasterFormat == RASTER_4444 ||
             rasterFormat == RASTER_16 ||
             rasterFormat == RASTER_555 )
        {
            theLayout = PSMCT16S;
        }
        else if ( rasterFormat == RASTER_LUM8 )
        {
            theLayout = PSMT8;
        }
        else if ( rasterFormat == RASTER_888 ||
                  rasterFormat == RASTER_24 )
        {
            theLayout = PSMCT24;
        }
        else if ( rasterFormat == RASTER_8888 ||
                  rasterFormat == RASTER_32 )
        {
            theLayout = PSMCT32;
        }
        else
        {
            return false;
        }

        memLayout = theLayout;

        return true;
    }

    typedef sliceOfData <uint32> memUnitSlice_t;

    struct MemoryRectBase
    {
        typedef memUnitSlice_t side_t;

        side_t x_slice, y_slice;

        inline MemoryRectBase( uint32 blockX, uint32 blockY, uint32 blockWidth, uint32 blockHeight )
            : x_slice( blockX, blockWidth ), y_slice( blockY, blockHeight )
        {
            return;
        }

        inline bool IsColliding( const MemoryRectBase *right ) const
        {
            side_t::eIntersectionResult x_result =
                this->x_slice.intersectWith( right->x_slice );

            side_t::eIntersectionResult y_result =
                this->y_slice.intersectWith( right->y_slice );

            return ( !side_t::isFloatingIntersect( x_result ) && !side_t::isFloatingIntersect( y_result ) );
        }

        inline MemoryRectBase SubRect( const MemoryRectBase *right ) const
        {
            uint32 maxStartX =
                std::max( this->x_slice.GetSliceStartPoint(), right->x_slice.GetSliceStartPoint() );
            uint32 maxStartY =
                std::max( this->y_slice.GetSliceStartPoint(), right->y_slice.GetSliceStartPoint() );

            uint32 minEndX =
                std::min( this->x_slice.GetSliceEndPoint(), right->x_slice.GetSliceEndPoint() );
            uint32 minEndY =
                std::min( this->y_slice.GetSliceEndPoint(), right->y_slice.GetSliceEndPoint() );

            MemoryRectBase subRect(
                maxStartX,
                maxStartY,
                minEndX - maxStartX + 1,
                minEndY - maxStartY + 1
            );

            return subRect;
        }

        inline bool HasSpace( void ) const
        {
            return
                this->x_slice.GetSliceSize() > 0 &&
                this->y_slice.GetSliceSize() > 0;
        }
    };

    struct MemoryRectangle : public MemoryRectBase
    {
        inline MemoryRectangle( uint32 blockX, uint32 blockY, uint32 blockWidth, uint32 blockHeight )
            : MemoryRectBase( blockX, blockY, blockWidth, blockHeight )
        {
            return;
        }

        RwListEntry <MemoryRectangle> node;
    };

    struct VirtualMemoryPage
    {
        inline VirtualMemoryPage( void )
        {
            LIST_CLEAR( allocatedRects.root );
        }

        inline ~VirtualMemoryPage( void )
        {
            LIST_FOREACH_BEGIN( MemoryRectangle, allocatedRects.root, node )
                
                delete item;

            LIST_FOREACH_END

            LIST_CLEAR( allocatedRects.root );
        }

        inline bool IsColliding( const MemoryRectBase *theRect ) const
        {
            LIST_FOREACH_BEGIN( MemoryRectangle, this->allocatedRects.root, node )

                if ( item->IsColliding( theRect ) == true )
                {
                    // There is a collision, so this rectangle is invalid.
                    // Try another position.
                    return true;
                }

            LIST_FOREACH_END

            return false;
        }

        // has a constant blockWidth and blockHeight same for every virtual page with same memLayout.
        // has a constant blocksPerWidth and blocksPerHeight same for every virtual page with same memLayout.
        eMemoryLayoutType memLayout;

        RwList <MemoryRectangle> allocatedRects;

        RwListEntry <VirtualMemoryPage> node;
    };

    struct MemoryPage
    {
        inline MemoryPage( void )
        {
            LIST_CLEAR( vmemList.root );
        }

        inline ~MemoryPage( void )
        {
            LIST_FOREACH_BEGIN( VirtualMemoryPage, vmemList.root, node )

                delete item;

            LIST_FOREACH_END

            LIST_CLEAR( vmemList.root );
        }

        RwList <VirtualMemoryPage> vmemList;

        RwListEntry <MemoryPage> node;

        inline VirtualMemoryPage* GetVirtualMemoryLayout( eMemoryLayoutType layoutType )
        {
            LIST_FOREACH_BEGIN( VirtualMemoryPage, vmemList.root, node )
            
                if ( item->memLayout == layoutType )
                {
                    return item;
                }

            LIST_FOREACH_END

            return NULL;
        }

        inline VirtualMemoryPage* AllocateVirtualMemoryLayout( eMemoryLayoutType layoutType )
        {
            VirtualMemoryPage *newPage = new VirtualMemoryPage();

            newPage->memLayout = layoutType;

            LIST_APPEND( this->vmemList.root, newPage->node );

            return newPage;
        }
    };

    RwList <MemoryPage> pages;

    inline ps2GSMemoryLayoutManager( void )
    {
        LIST_CLEAR( pages.root );
    }

    inline ~ps2GSMemoryLayoutManager( void )
    {
        LIST_FOREACH_BEGIN( MemoryPage, pages.root, node )
            
            delete item;

        LIST_FOREACH_END

        LIST_CLEAR( pages.root );
    }

    // Memory management constants of the PS2 Graphics Synthesizer.
    static const uint32 gsColumnSize = 16 * sizeof(uint32);
    static const uint32 gsBlockSize = gsColumnSize * 4;
    static const uint32 gsPageSize = gsBlockSize * 32;

    struct memoryLayoutProperties_t
    {
        uint32 pixelWidthPerBlock, pixelHeightPerBlock;
        uint32 widthBlocksPerPage, heightBlocksPerPage;

        const uint32 *const* blockArrangement;

        memUnitSlice_t pageDimX, pageDimY;
    };

    inline static void getMemoryLayoutProperties(eMemoryLayoutType memLayout, memoryLayoutProperties_t& layoutProps)
    {
        uint32 pixelWidthPerColumn = 0;
        uint32 pixelHeightPerColumn = 0;

        // For safety.
        layoutProps.blockArrangement = NULL;

        if ( memLayout == PSMT4 )
        {
            pixelWidthPerColumn = 32;
            pixelHeightPerColumn = 4;

            layoutProps.widthBlocksPerPage = 4;
            layoutProps.heightBlocksPerPage = 8;

            layoutProps.blockArrangement = (const uint32*const*)ps2GSMemoryLayoutArrangements::psmt4;
        }
        else if ( memLayout == PSMT8 )
        {
            pixelWidthPerColumn = 16;
            pixelHeightPerColumn = 4;

            layoutProps.widthBlocksPerPage = 8;
            layoutProps.heightBlocksPerPage = 4;

            layoutProps.blockArrangement = (const uint32*const*)ps2GSMemoryLayoutArrangements::psmt8;
        }
        else if ( memLayout == PSMCT32 || memLayout == PSMCT24 ||
                  memLayout == PSMZ32 || memLayout == PSMZ24 )
        {
            pixelWidthPerColumn = 8;
            pixelHeightPerColumn = 2;

            layoutProps.widthBlocksPerPage = 8;
            layoutProps.heightBlocksPerPage = 4;

            if ( memLayout == PSMCT32 || memLayout == PSMCT24 )
            {
                layoutProps.blockArrangement = (const uint32*const*)ps2GSMemoryLayoutArrangements::psmct32;
            }
            else if ( memLayout == PSMZ32 || memLayout == PSMZ24 )
            {
                layoutProps.blockArrangement = (const uint32*const*)ps2GSMemoryLayoutArrangements::psmz32;
            }
        }
        else if ( memLayout == PSMCT16 || memLayout == PSMCT16S ||
                  memLayout == PSMZ16 || memLayout == PSMZ16S )
        {
            pixelWidthPerColumn = 16;
            pixelHeightPerColumn = 2;

            layoutProps.widthBlocksPerPage = 4;
            layoutProps.heightBlocksPerPage = 8;

            if ( memLayout == PSMCT16 )
            {
                layoutProps.blockArrangement = (const uint32*const*)ps2GSMemoryLayoutArrangements::psmct16;
            }
            else if ( memLayout == PSMCT16S )
            {
                layoutProps.blockArrangement = (const uint32*const*)ps2GSMemoryLayoutArrangements::psmct16s;
            }
            else if ( memLayout == PSMZ16 )
            {
                layoutProps.blockArrangement = (const uint32*const*)ps2GSMemoryLayoutArrangements::psmz16;
            }
            else if ( memLayout == PSMZ16S )
            {
                layoutProps.blockArrangement = (const uint32*const*)ps2GSMemoryLayoutArrangements::psmz16s;
            }
        }
        else
        {
            // TODO.
            assert( 0 );
        }

        // Expand to block dimensions.
        layoutProps.pixelWidthPerBlock = pixelWidthPerColumn;
        layoutProps.pixelHeightPerBlock = pixelHeightPerColumn * 4;

        // Set up the page dimensions.
        layoutProps.pageDimX = memUnitSlice_t( 0, layoutProps.widthBlocksPerPage );
        layoutProps.pageDimY = memUnitSlice_t( 0, layoutProps.heightBlocksPerPage );
    }

    inline MemoryPage* GetPage( uint32 pageIndex )
    {
        uint32 n = 0;

        // Try to fetch an existing page.
        LIST_FOREACH_BEGIN( MemoryPage, pages.root, node )

            if ( n++ == pageIndex )
            {
                return item;
            }

        LIST_FOREACH_END

        // Allocate missing pages.
        MemoryPage *allocPage = NULL;

        while ( n++ <= pageIndex )
        {
            allocPage = new MemoryPage();

            LIST_APPEND( pages.root, allocPage->node );
        }

        return allocPage;
    }

    inline static uint32 getTextureBasePointer(const memoryLayoutProperties_t& layoutProps, uint32 pageX, uint32 pageY, uint32 bufferWidth, uint32 blockOffsetX, uint32 blockOffsetY)
    {
        // Get block index from the dimensional coordinates.
        // This requires a dispatch according to the memory layout.
        uint32 blockIndex = 0;
        {
            const uint32 *const *blockArrangement = layoutProps.blockArrangement;

            const uint32 *row = (const uint32*)( blockArrangement + blockOffsetY * layoutProps.widthBlocksPerPage );

            blockIndex = row[ blockOffsetX ];
        }

        // Allocate the texture at the current position in the buffer.
        uint32 pageIndex = ( pageY * bufferWidth + pageX );

        return ( pageIndex * 32 + blockIndex );
    }

    inline bool allocateTexture(eMemoryLayoutType memLayoutType, const memoryLayoutProperties_t& layoutProps, uint32 texelWidth, uint32 texelHeight, uint32& texBasePointer, uint32& texMemSize, uint32& texBufferWidthOut)
    {
        // Scale up texel dimensions.
        uint32 alignedTexelWidth = ALIGN_SIZE( texelWidth, layoutProps.pixelWidthPerBlock );
        uint32 alignedTexelHeight = ALIGN_SIZE( texelHeight, layoutProps.pixelHeightPerBlock );

        // Get block dimensions.
        uint32 texelBlockWidth = ( alignedTexelWidth / layoutProps.pixelWidthPerBlock );
        uint32 texelBlockHeight = ( alignedTexelHeight / layoutProps.pixelHeightPerBlock );

        // Get the width in pages.
        uint32 pageMaxBlockWidth = ALIGN_SIZE( texelBlockWidth, layoutProps.widthBlocksPerPage );

        uint32 texelPageWidth = pageMaxBlockWidth / layoutProps.widthBlocksPerPage;

        // Get the height in pages.
        uint32 pageMaxBlockHeight = ALIGN_SIZE( texelBlockHeight, layoutProps.heightBlocksPerPage );

        uint32 texelPageHeight = pageMaxBlockHeight / layoutProps.heightBlocksPerPage;

        // Get the minimum required texture buffer width.
        // It must be aligned to the page dimensions.
        // This value should be atleast 2.
        uint32 texBufferWidth = ( texelPageWidth * layoutProps.widthBlocksPerPage * layoutProps.pixelWidthPerBlock ) / 64;

        // TODO: this is not the real buffer width yet.

        // Loop through all pages and try to find the correct placement for the new texture.
        uint32 pageX = 0;
        uint32 pageY = 0;
        uint32 blockOffsetX = 0;
        uint32 blockOffsetY = 0;

        bool validAllocation = false;

        if ( texelPageWidth > 1 || texelPageHeight > 1 )
        {
            uint32 currentSideIter = 1;

            while ( true )
            {
                bool isProperAllocation = false;

                // Go down lines until we found a suitable position in the buffer for this page.
                {
                    // The alloc area is a rectangle placed linearly.
                    uint32 pageBlockOffX =
                        pageX * layoutProps.widthBlocksPerPage;

                    MemoryRectBase pageAllocArea(
                        pageBlockOffX + pageY * pageMaxBlockWidth,
                        0,
                        texelBlockWidth,
                        texelBlockHeight
                    );

                    bool hasFoundCollision = false;

                    // Check from the (0,0) point of all pages.
                    for ( uint32 y = 0; y < texelPageHeight; y++ )
                    {
                        for ( uint32 x = 0; x < texelPageWidth; x++ )
                        {
                            uint32 real_x = ( x + pageX );
                            uint32 real_y = ( y + pageY );

                            // Calculate the real index of this page.
                            uint32 pageIndex = ( texelPageWidth * real_y + real_x );

                            MemoryPage *thePage = this->GetPage( pageIndex );

                            // Collide our page rect with the contents of this page.
                            VirtualMemoryPage *vmemLayout = thePage->GetVirtualMemoryLayout( memLayoutType );

                            if ( vmemLayout )
                            {
                                bool isCollided = vmemLayout->IsColliding( &pageAllocArea );

                                if ( isCollided )
                                {
                                    hasFoundCollision = true;
                                    break;
                                }
                            }
                        }

                        if ( hasFoundCollision )
                        {
                            break;
                        }
                    }

                    if ( !hasFoundCollision )
                    {
                        // We appear to be okay. We can allocate in this area!
                        isProperAllocation = true;
                    }
                }

                // Check that we got to allocate anything.
                if ( isProperAllocation )
                {
                    // Mark our region and return!
                    validAllocation = true;
                    break;
                }

                pageY++;
            }
        }
        else
        {
            uint32 layoutStartX = layoutProps.pageDimX.GetSliceStartPoint();
            uint32 layoutStartY = layoutProps.pageDimY.GetSliceStartPoint();

            while ( true )
            {
                bool allocationSuccessful = false;

                // Try to allocate on the memory plane.
                {
                    uint32 pageIndex = ( pageY * texelPageWidth + pageX );

                    // The current page.
                    MemoryPage *currentPage = this->GetPage( pageIndex );

                    VirtualMemoryPage *vmemLayout = currentPage->GetVirtualMemoryLayout( memLayoutType );

                    bool canAllocateOnPage = true;

                    if ( vmemLayout )
                    {
                        MemoryRectBase thisRect(
                            layoutStartX,
                            layoutStartY,
                            texelBlockWidth,
                            texelBlockHeight
                        );

                        // We have to assume that we cannot allocate on this page.
                        canAllocateOnPage = false;

                        while ( true )
                        {
                            // Make sure we are not outside of the page dimensions.
                            {
                                memUnitSlice_t::eIntersectionResult x_result =
                                    thisRect.x_slice.intersectWith( layoutProps.pageDimX );

                                if ( x_result != memUnitSlice_t::INTERSECT_INSIDE && x_result != memUnitSlice_t::INTERSECT_EQUAL )
                                {
                                    // Advance to next line.
                                    thisRect.x_slice.SetSlicePosition( layoutStartX );
                                    thisRect.y_slice.OffsetSliceBy( 1 );
                                }

                                memUnitSlice_t::eIntersectionResult y_result =
                                    thisRect.y_slice.intersectWith( layoutProps.pageDimY );

                                if ( y_result != memUnitSlice_t::INTERSECT_INSIDE && y_result != memUnitSlice_t::INTERSECT_EQUAL )
                                {
                                    // This page is not it.
                                    break;
                                }
                            }

                            uint32 pageBlockOffX =
                                pageX * layoutProps.widthBlocksPerPage;

                            MemoryRectBase actualRect(
                                thisRect.x_slice.GetSliceStartPoint() + pageBlockOffX + pageY * pageMaxBlockWidth,
                                thisRect.y_slice.GetSliceStartPoint(),
                                texelBlockWidth,
                                texelBlockHeight
                            );

                            bool foundFreeSpot = ( vmemLayout->IsColliding( &actualRect ) == false );

                            // If there are no conflicts on our page, we can allocate on it.
                            if ( foundFreeSpot == true )
                            {
                                blockOffsetX = thisRect.x_slice.GetSliceStartPoint();
                                blockOffsetY = thisRect.y_slice.GetSliceStartPoint();

                                canAllocateOnPage = true;
                                break;
                            }

                            // We need to advance our position.
                            thisRect.x_slice.OffsetSliceBy( 1 );
                        }
                    }
                    
                    // If we can allocate on this page, then we succeeded!
                    if ( canAllocateOnPage == true )
                    {
                        allocationSuccessful = true;
                    }
                }

                // If the allocation has been successful, break.
                if ( allocationSuccessful )
                {
                    validAllocation = true;
                    break;
                }

                // We need to try from the next page.
                pageX++;

                // If the page is the limit, then restart and go to next line.
                if ( pageX == texelPageWidth )
                {
                    pageX = 0;

                    pageY++;
                }
            }
        }

        // This may trigger if we overshot memory capacity.
        if ( validAllocation == false )
            return false;

        // Calculate the texture base pointer.
        texBasePointer = getTextureBasePointer(layoutProps, pageX, pageY, texelPageWidth, blockOffsetX, blockOffsetY);

        // Calculate the required memory size.
        {
            uint32 texelBlockWidthOffset = ( texelBlockWidth - 1 ) + blockOffsetX;
            uint32 texelBlockHeightOffset = ( texelBlockHeight - 1 ) + blockOffsetY;

            uint32 finalPageX = pageX + texelBlockWidthOffset / layoutProps.widthBlocksPerPage;
            uint32 finalPageY = pageY + texelBlockHeightOffset / layoutProps.heightBlocksPerPage;

            uint32 finalBlockOffsetX = texelBlockWidthOffset % layoutProps.widthBlocksPerPage;
            uint32 finalBlockOffsetY = texelBlockHeightOffset % layoutProps.heightBlocksPerPage;

            uint32 texEndOffset =
                getTextureBasePointer(layoutProps, finalPageX, finalPageY, texelPageWidth, finalBlockOffsetX, finalBlockOffsetY);

            uint32 memSizeInBlocks = ( ( texEndOffset + 1 ) - texBasePointer );

            // Get the size that the GPU understands.
            texMemSize = memSizeInBlocks;//( memSizeInBlocks * layoutProps.pixelWidthPerBlock ) / 64;
        }

        // Give the texture buffer width to the runtime.
        texBufferWidthOut = texBufferWidth;

        // Add our collision rectangles onto the pages we allocated.
        MemoryRectBase pageAllocArea(
            pageX * layoutProps.widthBlocksPerPage + blockOffsetX,
            pageY * layoutProps.heightBlocksPerPage + blockOffsetY,
            texelBlockWidth,
            texelBlockHeight
        );

        for ( uint32 allocPageY = 0; allocPageY < texelPageHeight; allocPageY++ )
        {
            for ( uint32 allocPageX = 0; allocPageX < texelPageWidth; allocPageX++ )
            {
                uint32 realPageX = ( allocPageX + pageX );
                uint32 realPageY = ( allocPageY + pageY );

                uint32 pageBlockOffX =
                    layoutProps.pageDimX.GetSliceStartPoint() + realPageX * layoutProps.widthBlocksPerPage;
                uint32 pageBlockOffY =
                    layoutProps.pageDimY.GetSliceStartPoint() + realPageY * layoutProps.heightBlocksPerPage;

                MemoryRectBase pageZone(
                    pageBlockOffX,
                    pageBlockOffY,
                    layoutProps.widthBlocksPerPage,
                    layoutProps.heightBlocksPerPage
                );

                MemoryRectBase subRectAllocZone = pageZone.SubRect( &pageAllocArea );

                // If there is a zone to include, we do that.
                if ( subRectAllocZone.HasSpace() )
                {
                    // Transform the subrect onto a linear zone.
                    uint32 blockLocalX =
                        subRectAllocZone.x_slice.GetSliceStartPoint() - pageBlockOffX;
                    uint32 blockLocalY =
                        subRectAllocZone.y_slice.GetSliceStartPoint() - pageBlockOffY;

                    uint32 pageIndex = ( realPageY * texelPageWidth + realPageX );

                    MemoryPage *thePage = this->GetPage( pageIndex );

                    VirtualMemoryPage *vmemLayout = thePage->GetVirtualMemoryLayout( memLayoutType );

                    if ( !vmemLayout )
                    {
                        vmemLayout = thePage->AllocateVirtualMemoryLayout( memLayoutType );
                    }

                    if ( vmemLayout )
                    {
                        MemoryRectangle *memRect =
                            new MemoryRectangle(
                                blockLocalX + realPageX * layoutProps.widthBlocksPerPage + realPageY * pageMaxBlockWidth,
                                blockLocalY,
                                subRectAllocZone.x_slice.GetSliceSize(),
                                subRectAllocZone.y_slice.GetSliceSize()
                            );

                        if ( memRect )
                        {
                            LIST_INSERT( vmemLayout->allocatedRects.root, memRect->node );
                        }
                    }
                }
            }
        }

        return true;
    }
};

bool NativeTexturePS2::allocateTextureMemory(uint32 mipmapBasePointer[], uint32 mipmapBufferWidth[], uint32 mipmapMemorySize[], uint32 maxMipmaps, eMemoryLayoutType& memLayoutTypeOut) const
{
    uint32 mainTexWidth = this->swizzleWidth[0];
    uint32 mainTexHeight = this->swizzleHeight[0];
    uint32 depth = this->mipmapDepth[0];

    eRasterFormat pixelFormatRaster = parent->rasterFormat;
    ePaletteType paletteType = this->paletteType;

    uint32 mipmapCount = this->mipmapCount;

    // Try to get a valid memory model from the palette type.
    eMemoryLayoutType memLayout;

    if ( !ps2GSMemoryLayoutManager::getMemoryLayoutFromPaletteType( paletteType, memLayout ) )
    {
        // If not from the palette, we get it from the pixel format.
        if ( !ps2GSMemoryLayoutManager::getMemoryLayoutFromRasterFormat( pixelFormatRaster, memLayout ) )
        {
            // We do not know how to handle this raster, so quit.
            return false;
        }
    }

    // Get format properties.
    ps2GSMemoryLayoutManager::memoryLayoutProperties_t layoutProps;

    ps2GSMemoryLayoutManager::getMemoryLayoutProperties( memLayout, layoutProps );

    // Perform the allocation.
    {
        // Create a new memory environment.
        ps2GSMemoryLayoutManager gsMem;

        // Get page dimensions
        uint32 pixelWidthPerPage = ( layoutProps.widthBlocksPerPage * layoutProps.pixelWidthPerBlock );
        uint32 pixelHeightPerPage = ( layoutProps.heightBlocksPerPage * layoutProps.pixelHeightPerBlock );

        for ( uint32 n = 0; n < mipmapCount; n++ )
        {
            // Get the texel dimensions of this texture.
            uint32 texelWidth = this->swizzleWidth[n];
            uint32 texelHeight = this->swizzleHeight[n];

            uint32 texBasePointer = 0;
            uint32 texBufferWidth = 0;
            uint32 texMemSize = 0;

            bool allocationSuccess = gsMem.allocateTexture( memLayout, layoutProps, texelWidth, texelHeight, texBasePointer, texMemSize, texBufferWidth );

            // If we fail to allocate any texture, we must terminate here.
            if ( !allocationSuccess )
            {
                return false;
            }

            if ( n >= maxMipmaps )
            {
                // We do not know how to handle more mipmaps than the hardware allows.
                // For safety reasons terminate.
                return false;
            }

            // Store the results.
            mipmapBasePointer[ n ] = texBasePointer;

            // Store the size of the texture in memory.
            mipmapMemorySize[ n ] = texMemSize;

            // Also store our texture buffer width.
            mipmapBufferWidth[ n ] = texBufferWidth;
        }

        // Normalize all the remaining fields.
        for ( uint32 n = mipmapCount; n < maxMipmaps; n++ )
        {
            mipmapBasePointer[ n ] = 0;
            mipmapMemorySize[ n ] = 0;
            mipmapBufferWidth[ n ] = 1;
        }
    }

    // Make sure buffer sizes are in their limits.
    for ( uint32 n = 0; n < mipmapCount; n++ )
    {
        uint32 thisBase = mipmapBasePointer[n];
        
        if ( thisBase >= 0x4000 )
        {
            return false;
        }

        uint32 thisBufferWidth = mipmapBufferWidth[n];

        if ( thisBufferWidth >= 64 )
        {
            return false;
        }
    }

    // Give the final memory layout to the runtime.
    memLayoutTypeOut = memLayout;

    return true;
}

}