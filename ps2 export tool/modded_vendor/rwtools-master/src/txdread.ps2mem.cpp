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

uint32 NativeTexturePS2::calculateGPUDataSize(uint32 palDataSize) const
{
    uint32 numMipMaps = this->mipmapCount;

    if ( numMipMaps == 0 )
        return 0;

    // Assume uniform depth.
    uint32 depth = this->mipmapDepth[0];

    uint32 textureMemoryDataSize = 0;

    for (uint32 n = 0; n < numMipMaps; n++)
    {
        uint32 curDataSize = this->dataSizes[n];
        const ps2MipmapUnknowns& mipunk = this->mipmapUnknowns[n];

        textureMemoryDataSize += curDataSize / 4 + mipunk.unk1 + mipunk.unk2;
    }

    uint32 gpuMinMemory = ALIGN_SIZE( textureMemoryDataSize + palDataSize, (uint32)2048 );

    //if ( ( this->rasterFormat & RASTER_AUTOMIPMAP ) != 0 )
    {
        // Deal with some really obscure cases by defining a minimum memory milestone.
        // We do that for security reasons.
        uint32 minMemoryFootprint = 2048;

        uint32 curSplitStart = 0;

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

}