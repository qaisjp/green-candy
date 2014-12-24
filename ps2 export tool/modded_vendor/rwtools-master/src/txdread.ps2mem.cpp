#include <StdInc.h>

#include "txdread.ps2.hxx"

#include "txdread.ps2gsman.hxx"

namespace rw
{

template <typename arrayType>
struct swizzle_crypt
{
public:
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

    inline static void getswizzlecoord(uint32 x, uint32 y, uint32& swizzleX, uint32& swizzleY)
    {
        // =====================================================
        // Algorithm pipeline:
        // + IDTEX4 -> RGBA | (ON PS2 HARDWARE) | -> PSMCT32
        // + IDTEX8 -> RGBA | (ON PS2 HARDWARE) | -> PSMCT32
        // - RGBA -> IDTEX4
        // - RGBA -> IDTEX8
        // =====================================================
        // for details, see http://mtz01-a.stanford.edu/resources/SonyPS2/pdf00005.pdf pages 172+

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

    inline static bool getimagecoord(uint32 x, uint32 y, uint32& imgX, uint32& imgY)
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

    inline void calculateswizzleplane(uint32 imageWidth, uint32 imageHeight, uint32& swizzleWidthOut, uint32& swizzleHeightOut)
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

        // Return to the runtime.
        swizzleWidthOut = swizzleWidth;
        swizzleHeightOut = swizzleHeight;
    }

    inline arrayType* swizzle(uint32& dstEncryptedSize, const arrayType *srcToBeEncryptedData, uint32 imageWidth, uint32 imageHeight, uint32& outSwizzleWidth, uint32& outSwizzleHeight)
    {
        // The swizzled picture is interleaved.
        // The first and third tracks are taken by the left picture side.
        // The second and fourth tracks are taken by the right side.
        uint32 swizzle_interleave_width = imageWidth / 2;

        // Get the swizzle plane size.
        uint32 swizzleWidth, swizzleHeight;

        calculateswizzleplane(imageWidth, imageHeight, swizzleWidth, swizzleHeight);

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
    uint32 clutBasePointer, uint32 clutMemSize
) const
{
    uint32 numMipMaps = this->mipmapCount;

    if ( numMipMaps == 0 )
        return 0;

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

    // Include CLUT.
    {
        uint32 thisOffset = ( clutBasePointer + clutMemSize );

        if ( maxMemOffset < thisOffset )
        {
            maxMemOffset = thisOffset;
        }
    }

    uint32 textureMemoryDataSize = ( maxMemOffset * 64 );

    return ALIGN_SIZE( textureMemoryDataSize, 2048u );
}

eFormatEncodingType NativeTexturePS2::getHardwareRequiredEncoding(uint32 version) const
{
    eFormatEncodingType imageEncodingType = FORMAT_UNKNOWN;

    eRasterFormat rasterFormat = parent->rasterFormat;
    ePaletteType paletteType = this->paletteType;

    if (paletteType == PALETTE_4BIT)
    {
        if (version == rw::GTA3_1 || version == rw::GTA3_2 ||
            version == rw::GTA3_3 || version == rw::GTA3_4)
        {
            imageEncodingType = FORMAT_IDTEX8_COMPRESSED;
        }
        else
        {
            if (this->requiresHeaders || this->hasSwizzle)
            {
                imageEncodingType = FORMAT_TEX32;
            }
            else
            {
                imageEncodingType = FORMAT_IDTEX8_COMPRESSED;
            }
        }
    }
    else if (paletteType == PALETTE_8BIT)
    {
        if (this->requiresHeaders || this->hasSwizzle)
        {
            imageEncodingType = FORMAT_TEX32;
        }
        else
        {
            imageEncodingType = FORMAT_IDTEX8;
        }
    }
    else if (rasterFormat == RASTER_LUM8)
    {
        imageEncodingType = FORMAT_IDTEX8;
    }
    else if (rasterFormat == RASTER_1555 || rasterFormat == RASTER_555 || rasterFormat == RASTER_565 ||
             rasterFormat == RASTER_4444 || rasterFormat == RASTER_16)
    {
        imageEncodingType = FORMAT_TEX16;
    }
    else if (rasterFormat == RASTER_8888 || rasterFormat == RASTER_888 || rasterFormat == RASTER_32)
    {
        imageEncodingType = FORMAT_TEX32;
    }

    return imageEncodingType;
}

bool NativeTexturePS2::swizzleEncryptPS2(uint32 i)
{
    // Determine the encoding formats so it can be stored into PS2 GS memory correctly.
    eFormatEncodingType currentEncoding = this->swizzleEncodingType[i];

    eFormatEncodingType encodeTo = this->getHardwareRequiredEncoding(rw::rwInterface.GetVersion());

    if ( currentEncoding == FORMAT_UNKNOWN || encodeTo == FORMAT_UNKNOWN )
        return false;

    bool operationSuccessful = false;
   
    // Get picture meta information.
    uint32 realImageWidth = this->width[i];
    uint32 realImageHeight = this->height[i];
    uint32 encryptedWidth = realImageWidth;
    uint32 encryptedHeight = realImageHeight;

    if ( currentEncoding != encodeTo )
    {
        uint32 newDataSize;
        void *newtexels;

        newtexels =
            ps2GSPixelEncodingFormats::packImageData(
                currentEncoding, encodeTo,
                texels[i],
                realImageWidth, realImageHeight,
                newDataSize, encryptedWidth, encryptedHeight
            );

        if ( newtexels != NULL )
        {
            operationSuccessful = true;
        }

        if ( operationSuccessful )
        {
            this->dataSizes[i] = newDataSize;

	        delete[] texels[i];
	        texels[i] = newtexels;

            this->swizzleEncodingType[i] = encodeTo;
        }
    }
    else
    {
        operationSuccessful = true;
    }

    if ( operationSuccessful )
    {
        this->swizzleWidth[i] = encryptedWidth;
        this->swizzleHeight[i] = encryptedHeight;
    }

    return operationSuccessful;
}

bool NativeTexturePS2::swizzleDecryptPS2(uint32 i)
{
    eFormatEncodingType currentEncoding = this->swizzleEncodingType[i];

    // Obtain the format that we should decode to.
    eFormatEncodingType decodeTo = getFormatEncodingFromRasterFormat(parent->rasterFormat, this->paletteType);

    if ( currentEncoding == FORMAT_UNKNOWN || decodeTo == FORMAT_UNKNOWN )
        return false;

    bool operationSuccessful = false;

    // Get picture meta information.
    uint32 realImageWidth = this->width[i];
    uint32 realImageHeight = this->height[i];
    uint32 encryptedWidth = this->swizzleWidth[i];
    uint32 encryptedHeight = this->swizzleHeight[i];

    if ( currentEncoding != decodeTo )
    {
        uint32 newDataSize;
        void *newtexels;

        newtexels =
            ps2GSPixelEncodingFormats::unpackImageData(
                currentEncoding, decodeTo,
                texels[i],
                encryptedWidth, encryptedHeight,
                newDataSize,
                realImageWidth, realImageHeight
            );

        if ( newtexels != NULL )
        {
            operationSuccessful = true;
        }

        if ( operationSuccessful )
        {
            this->dataSizes[i] = newDataSize;

	        delete[] texels[i];
	        texels[i] = newtexels;

            this->swizzleEncodingType[i] = decodeTo;
        }
    }
    else
    {
        operationSuccessful = true;
    }

    return operationSuccessful;
}

struct ps2GSMemoryLayoutManager
{
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

    inline static void getMemoryLayoutProperties(eMemoryLayoutType memLayout, eFormatEncodingType encodingType, memoryLayoutProperties_t& layoutProps)
    {
        uint32 pixelWidthPerColumn = 0;
        uint32 pixelHeightPerColumn = 0;

        // For safety.
        layoutProps.blockArrangement = NULL;

        if ( memLayout == PSMT4 && encodingType == FORMAT_IDTEX4 )
        {
            pixelWidthPerColumn = 32;
            pixelHeightPerColumn = 4;

            layoutProps.widthBlocksPerPage = 4;
            layoutProps.heightBlocksPerPage = 8;

            layoutProps.blockArrangement = (const uint32*const*)ps2GSMemoryLayoutArrangements::psmt4;
        }
        else if ( memLayout == PSMT4 && encodingType == FORMAT_IDTEX8_COMPRESSED )
        {
            // TODO: fix this.
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

    inline bool findAllocationRegion(eMemoryLayoutType memLayoutType, uint32 texelBlockWidth, uint32 texelBlockHeight, const memoryLayoutProperties_t& layoutProps, uint32& pageX_out, uint32& pageY_out, uint32& blockX_out, uint32& blockY_out)
    {
        // Get the width in pages.
        uint32 pageMaxBlockWidth = ALIGN_SIZE( texelBlockWidth, layoutProps.widthBlocksPerPage );

        uint32 texelPageWidth = pageMaxBlockWidth / layoutProps.widthBlocksPerPage;

        // Get the height in pages.
        uint32 pageMaxBlockHeight = ALIGN_SIZE( texelBlockHeight, layoutProps.heightBlocksPerPage );

        uint32 texelPageHeight = pageMaxBlockHeight / layoutProps.heightBlocksPerPage;

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

        if ( validAllocation )
        {
            pageX_out = pageX;
            pageY_out = pageY;
            blockX_out = blockOffsetX;
            blockY_out = blockOffsetY;
        }

        return validAllocation;
    }

    inline bool allocateTexture(eMemoryLayoutType memLayoutType, const memoryLayoutProperties_t& layoutProps, uint32 texelWidth, uint32 texelHeight, uint32& texBasePointer, uint32& texMemSize, uint32& texOffX, uint32& texOffY, uint32& texBufferWidthOut)
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

        bool validAllocation = 
            findAllocationRegion(
                memLayoutType, texelBlockWidth, texelBlockHeight,
                layoutProps,
                pageX, pageY, blockOffsetX, blockOffsetY
            );

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

        // Add our collision rectangles onto the pages we allocated.
        MemoryRectBase pageAllocArea(
            pageX * layoutProps.widthBlocksPerPage + blockOffsetX,
            pageY * layoutProps.heightBlocksPerPage + blockOffsetY,
            texelBlockWidth,
            texelBlockHeight
        );

        // Give the target coordinates to the runtime.
        // They are passed as block coordinates.
        {
            texOffX = pageAllocArea.x_slice.GetSliceStartPoint();
            texOffY = pageAllocArea.y_slice.GetSliceStartPoint();
        }

        // Give the texture buffer width to the runtime.
        texBufferWidthOut = texBufferWidth;

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

struct singleMemLayoutGSAllocator
{
    ps2GSMemoryLayoutManager gsMem;

    ps2GSMemoryLayoutManager::memoryLayoutProperties_t layoutProps;

    eMemoryLayoutType pixelMemLayoutType;
    eFormatEncodingType encodingMemLayout;
    eFormatEncodingType encodingPixelMemLayoutType;

    uint32 maxBuffHeight;

    inline singleMemLayoutGSAllocator(
        eFormatEncodingType encodingMemLayout, eFormatEncodingType encodingPixelMemLayoutType,
        eMemoryLayoutType pixelMemLayoutType
    )
    {
        this->pixelMemLayoutType = pixelMemLayoutType;
        this->encodingMemLayout = encodingMemLayout;
        this->encodingPixelMemLayoutType = encodingPixelMemLayoutType;
        this->maxBuffHeight = 0;

        // Get format properties.
        ps2GSMemoryLayoutManager::getMemoryLayoutProperties( pixelMemLayoutType, encodingPixelMemLayoutType, this->layoutProps );
    }

    inline ~singleMemLayoutGSAllocator( void )
    {
        return;
    }

    inline bool allocateTexture(uint32 encodedWidth, uint32 encodedHeight, uint32& texBasePointerOut, uint32& texBufferWidthOut, uint32& texMemSizeOut, uint32& texOffXOut, uint32& texOffYOut)
    {
        uint32 texelWidth, texelHeight;

        bool gotDecodedDimms =
            ps2GSPixelEncodingFormats::getPackedFormatDimensions(
                encodingMemLayout, encodingPixelMemLayoutType, encodedWidth, encodedHeight,
                texelWidth, texelHeight
            );

        uint32 texBasePointer = 0;
        uint32 texBufferWidth = 0;
        uint32 texMemSize = 0;
        uint32 texOffX = 0;
        uint32 texOffY = 0;

        if ( gotDecodedDimms )
        {
            if ( maxBuffHeight < texelHeight )
            {
                maxBuffHeight = texelHeight;
            }

            bool allocationSuccess = gsMem.allocateTexture( this->pixelMemLayoutType, this->layoutProps, texelWidth, texelHeight, texBasePointer, texMemSize, texOffX, texOffY, texBufferWidth );

            // If we fail to allocate any texture, we must terminate here.
            if ( !allocationSuccess )
            {
                return false;
            }

            // Write the results to the runtime.
            texBasePointerOut = texBasePointer;
            texBufferWidthOut = texBufferWidth;
            texMemSizeOut = texMemSize;
            texOffXOut = texOffX;
            texOffYOut = texOffY;
        }

        return gotDecodedDimms;
    }
};

bool NativeTexturePS2::allocateTextureMemory(
    uint32 mipmapBasePointer[], uint32 mipmapBufferWidth[], uint32 mipmapMemorySize[], ps2MipmapTransmissionData mipmapTransData[], uint32 maxMipmaps,
    eMemoryLayoutType& pixelMemLayoutTypeOut,
    uint32& clutBasePointerOut, uint32& clutMemSizeOut, ps2MipmapTransmissionData& clutTransDataOut
) const
{
    // Get the memory layout of the encoded texture.
    eFormatEncodingType encodingMemLayout = this->swizzleEncodingType[ 0 ];

    if ( encodingMemLayout == FORMAT_UNKNOWN )
        return false;

    eFormatEncodingType encodingPixelMemLayoutType = getFormatEncodingFromRasterFormat( this->parent->rasterFormat, this->paletteType );

    if ( encodingPixelMemLayoutType == FORMAT_UNKNOWN )
        return false;

    // Get the memory layout type of our decoded texture data.
    // This is used to fetch texel data from the permuted block correctly.
    eMemoryLayoutType pixelMemLayoutType;
    
    bool gotDecodedMemLayout = getMemoryLayoutFromTexelFormat(encodingPixelMemLayoutType, pixelMemLayoutType);

    if ( gotDecodedMemLayout == false )
        return false;

    // Get the encoding memory layout.
    eMemoryLayoutType encodedMemLayoutType;

    bool gotEncodedMemLayout = getMemoryLayoutFromTexelFormat(encodingMemLayout, encodedMemLayoutType);

    if ( gotEncodedMemLayout == false )
        return false;

    // Get the properties of the encoded mem layout.
    ps2GSMemoryLayoutManager::memoryLayoutProperties_t encodedLayoutProps;

    ps2GSMemoryLayoutManager::getMemoryLayoutProperties(encodedMemLayoutType, encodingMemLayout, encodedLayoutProps);

    uint32 mipmapCount = this->mipmapCount;

    // Perform the allocation.
    {
        singleMemLayoutGSAllocator gsAlloc( encodingMemLayout, encodingPixelMemLayoutType, pixelMemLayoutType );

        // Create a new memory environment.
        ps2GSMemoryLayoutManager gsMem; 

        for ( uint32 n = 0; n < mipmapCount; n++ )
        {
            // Get the texel dimensions of this texture.
            uint32 encodedWidth = this->swizzleWidth[n];
            uint32 encodedHeight = this->swizzleHeight[n];

            uint32 texBasePointer;
            uint32 texMemSize;
            uint32 texBufferWidth;
            uint32 texOffX;
            uint32 texOffY;

            bool hasAllocated = gsAlloc.allocateTexture(encodedWidth, encodedHeight, texBasePointer, texBufferWidth, texMemSize, texOffX, texOffY);

            if ( !hasAllocated )
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

            // Store the target coordinates.
            ps2MipmapTransmissionData& transData = mipmapTransData[ n ];

            // Get the offset in pixels.
            uint32 pixelTexOffX = ( texOffX * encodedLayoutProps.pixelWidthPerBlock );
            uint32 pixelTexOffY = ( texOffY * encodedLayoutProps.pixelHeightPerBlock );

            if (encodingMemLayout == FORMAT_TEX32 && encodingPixelMemLayoutType == FORMAT_IDTEX8_COMPRESSED)
            {
                pixelTexOffX *= 2;
            }

            transData.destX = pixelTexOffX;
            transData.destY = pixelTexOffY % gsAlloc.maxBuffHeight;
        }

        // Normalize all the remaining fields.
        for ( uint32 n = mipmapCount; n < maxMipmaps; n++ )
        {
            mipmapBasePointer[ n ] = 0;
            mipmapMemorySize[ n ] = 0;
            mipmapBufferWidth[ n ] = 1;

            ps2MipmapTransmissionData& transData = mipmapTransData[ n ];

            transData.destX = 0;
            transData.destY = 0;
        }

        // Allocate the palette data at the end.
        uint32 clutBasePointer = 0;
        uint32 clutMemSize = 0;
        ps2MipmapTransmissionData clutTransData;

        clutTransData.destX = 0;
        clutTransData.destY = 0;

        if (this->paletteType != PALETTE_NONE)
        {
            uint32 psmct32_width = 0;
            uint32 psmct32_height = 0;

            if (this->paletteType == PALETTE_4BIT)
            {
                psmct32_width = 8;
                psmct32_height = 3;
            }
            else if (this->paletteType == PALETTE_8BIT)
            {
                psmct32_width = 16;
                psmct32_height = 16;
            }

            //assert(encodingMemLayout == FORMAT_TEX32);

            // Allocate it.
            uint32 _clutBasePointer;
            uint32 _clutMemSize;
            uint32 clutBufferWidth;
            uint32 clutOffX;
            uint32 clutOffY;

            bool hasAllocatedCLUT = gsAlloc.allocateTexture(psmct32_width, psmct32_height, _clutBasePointer, clutBufferWidth, _clutMemSize, clutOffX, clutOffY);

            if ( hasAllocatedCLUT == false )
            {
                return false;
            }

            // Transform to final CLUT coords.
            uint32 clutFinalOffX = clutOffX;
            uint32 clutFinalOffY = clutOffY % gsAlloc.maxBuffHeight;

            // Write to the runtime.
            clutBasePointer = _clutBasePointer;
            clutMemSize = _clutMemSize;

            if (this->paletteType == PALETTE_8BIT)
            {
                clutTransData.destX = clutFinalOffX;
                clutTransData.destY = clutFinalOffY;
            }
        }

        clutBasePointerOut = clutBasePointer;
        clutMemSizeOut = clutMemSize;
        clutTransDataOut = clutTransData;
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
    
    // Give the pixel mem layout type to the runtime.
    pixelMemLayoutTypeOut = pixelMemLayoutType;

    return true;
}

bool NativeTexturePS2::getDebugBitmap( Bitmap& bmpOut ) const
{
    // Setup colors to use for the rectangles.
    struct singleColorSourcePipeline : public Bitmap::sourceColorPipeline
    {
        double red, green, blue, alpha;

        inline singleColorSourcePipeline( void )
        {
            this->red = 0;
            this->green = 0;
            this->blue = 0;
            this->alpha = 1.0;
        }

        uint32 getWidth( void ) const
        {
            return 1;
        }

        uint32 getHeight( void ) const
        {
            return 1;
        }

        void fetchcolor( uint32 colorIndex, double& red, double& green, double& blue, double& alpha )
        {
            red = this->red;
            green = this->green;
            blue = this->blue;
            alpha = this->alpha;
        }
    };

    singleColorSourcePipeline colorSrcPipe;

    // Get the memory layout of the encoded texture.
    eFormatEncodingType encodingMemLayout = this->swizzleEncodingType[ 0 ];

    if ( encodingMemLayout == FORMAT_UNKNOWN )
        return false;

    eFormatEncodingType encodingPixelMemLayoutType = getFormatEncodingFromRasterFormat( this->parent->rasterFormat, this->paletteType );

    if ( encodingPixelMemLayoutType == FORMAT_UNKNOWN )
        return false;

    // Get the memory layout type of our decoded texture data.
    // This is used to fetch texel data from the permuted block correctly.
    eMemoryLayoutType pixelMemLayoutType;
    
    bool gotDecodedMemLayout = getMemoryLayoutFromTexelFormat(encodingPixelMemLayoutType, pixelMemLayoutType);

    if ( gotDecodedMemLayout == false )
        return false;

    // Get the encoding memory layout.
    eMemoryLayoutType encodedMemLayoutType;

    bool gotEncodedMemLayout = getMemoryLayoutFromTexelFormat(encodingMemLayout, encodedMemLayoutType);

    if ( gotEncodedMemLayout == false )
        return false;

    // Get the properties of the encoded mem layout.
    ps2GSMemoryLayoutManager::memoryLayoutProperties_t encodedLayoutProps;

    ps2GSMemoryLayoutManager::getMemoryLayoutProperties(encodedMemLayoutType, encodingMemLayout, encodedLayoutProps);

    uint32 mipmapCount = this->mipmapCount;

    // Perform the allocation.
    {
        singleMemLayoutGSAllocator gsAlloc( encodingMemLayout, encodingPixelMemLayoutType, pixelMemLayoutType );

        for ( uint32 n = 0; n < mipmapCount; n++ )
        {
            // Get the texel dimensions of this texture.
            uint32 encodedWidth = this->swizzleWidth[n];
            uint32 encodedHeight = this->swizzleHeight[n];

            uint32 texBasePointer;
            uint32 texMemSize;
            uint32 texBufferWidth;
            uint32 texOffX;
            uint32 texOffY;

            bool allocationSuccess = gsAlloc.allocateTexture(encodedWidth, encodedHeight, texBasePointer, texBufferWidth, texMemSize, texOffX, texOffY);

            // If we fail to allocate any texture, we cannot draw it.
            if ( allocationSuccess )
            {
                // Get the real width and height.
                uint32 texelWidth = encodedWidth;
                uint32 texelHeight = encodedHeight;

                // Make sure the bitmap is large enough for our drawing.
                uint32 pixelOffX = texOffX * encodedLayoutProps.pixelWidthPerBlock;
                uint32 pixelOffY = texOffY * encodedLayoutProps.pixelHeightPerBlock;

                if (encodingMemLayout == FORMAT_TEX32 && encodingPixelMemLayoutType == FORMAT_IDTEX8_COMPRESSED)
                {
                    texelWidth *= 2;
                    pixelOffX *= 2;
                }

                uint32 reqWidth = pixelOffX + texelWidth;
                uint32 reqHeight = pixelOffY + texelHeight;

                bmpOut.enlargePlane( reqWidth, reqHeight );

                // Set special color depending on mipmap count.
                if ( n == 0 )
                {
                    colorSrcPipe.red = 0.5666;
                    colorSrcPipe.green = 0;
                    colorSrcPipe.blue = 0;
                }
                else if ( n == 1 )
                {
                    colorSrcPipe.red = 0;
                    colorSrcPipe.green = 0.5666;
                    colorSrcPipe.blue = 0;
                }
                else if ( n == 2 )
                {
                    colorSrcPipe.red = 0;
                    colorSrcPipe.green = 0;
                    colorSrcPipe.blue = 1.0;
                }
                else if ( n == 3 )
                {
                    colorSrcPipe.red = 1.0;
                    colorSrcPipe.green = 1.0;
                    colorSrcPipe.blue = 0;
                }
                else if ( n == 4 )
                {
                    colorSrcPipe.red = 0;
                    colorSrcPipe.green = 1.0;
                    colorSrcPipe.blue = 1.0;
                }
                else if ( n == 5 )
                {
                    colorSrcPipe.red = 1.0;
                    colorSrcPipe.green = 1.0;
                    colorSrcPipe.blue = 1.0;
                }
                else if ( n == 6 )
                {
                    colorSrcPipe.red = 0.5;
                    colorSrcPipe.green = 0.5;
                    colorSrcPipe.blue = 0.5;
                }

                // Draw the rectangle.
                bmpOut.draw(
                    colorSrcPipe, pixelOffX, pixelOffY, texelWidth, texelHeight,
                    Bitmap::SHADE_SRCALPHA, Bitmap::SHADE_ONE, Bitmap::BLEND_ADDITIVE
                );
            }
        }

        // Also render the palette if its there.
        if (this->paletteType == PALETTE_8BIT && this->hasPalUnknowns)
        {
            colorSrcPipe.red = 1;
            colorSrcPipe.green = 0.75;
            colorSrcPipe.blue = 0;

            uint32 palWidth = 0;
            uint32 palHeight = 0;

            if (this->paletteType == PALETTE_4BIT)
            {
                palWidth = 8;
                palHeight = 3;
            }
            else if (this->paletteType == PALETTE_8BIT)
            {
                palWidth = 16;
                palHeight = 16;
            }

            bmpOut.enlargePlane( palWidth + this->palUnknowns.destX, palHeight + this->palUnknowns.destY );

            bmpOut.draw(
                colorSrcPipe, this->palUnknowns.destX, this->palUnknowns.destY,
                palWidth, palHeight,
                Bitmap::SHADE_SRCALPHA, Bitmap::SHADE_ONE, Bitmap::BLEND_ADDITIVE
            );
        }
    }

    return true;
}

}