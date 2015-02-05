/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        Shared/core/CFileSystem.img.translator.compress.cpp
*  PURPOSE:     IMG R* Games archive management
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#include <StdInc.h>

// Include internal (private) definitions.
#include "CFileSystem.internal.h"
#include "CFileSystem.img.internal.h"

// Include compression headers.
// These are very dirty files.
#include <lzo/lzoconf.h>
#include <lzo/lzo1x.h>

extern CFileSystem *fileSystem;

static bool _isLZOInitialized = false;
static unsigned long _lzoRefCount = 0;

// For safety, as LZO is not a very stable library.
static size_t minimumDecompressBufferSize = 1024;

static bool _performLZOChecksumVerify = false;

static unsigned long __cdecl _calculateChecksum( unsigned long c, const void *data, size_t dataSize )
{
    // TODO: fix checksum verification for R* XBOX IMG files.
    return lzo_adler32( c, (const unsigned char*)data, dataSize );
}

xboxIMGCompression::xboxIMGCompression( void )
{
    this->isUsingLZO = false;

    // Prepare checksum calculation.
    this->_checksumCallback = _calculateChecksum;

    // Set the maximum block size that should be used for compression.
    this->compressionMaximumBlockSize = 0x00020000;
}

xboxIMGCompression::~xboxIMGCompression( void )
{
    // Shutdown the LZO library.
    this->ShutdownLZO();
}

void xboxIMGCompression::InitializeLZO( void )
{
    if ( !this->isUsingLZO )
    {
        bool couldInit = false;

        if ( _lzoRefCount == 0 )
        {
            couldInit = ( lzo_init() == LZO_E_OK );
        }
        else
        {
            couldInit = true;
        }

        if ( couldInit )
        {
            // Alright, we initialized.
            this->isUsingLZO = true;

            _lzoRefCount++;
        }
    }
}

void xboxIMGCompression::ShutdownLZO( void )
{
    if ( this->isUsingLZO )
    {
        // Decrement the LZO ref count.
        _lzoRefCount--;

        if ( _lzoRefCount == 0 )
        {
            // There actually is no shutdown.
        }

        // We are not using LZO anymore.
        this->isUsingLZO = false;
    }
}

bool xboxIMGCompression::IsStreamCompressed( CFile *input ) const
{
    // Read the first 4 bytes to verify the magic.
    bool isCompressed = false;

    fsUInt_t magic = 0;

    bool hasReadMagic = input->ReadUInt( magic );

    if ( hasReadMagic )
    {
        bool isLZOCompressed = ( magic == 0x67A3A1CE );

        if ( isLZOCompressed )
        {
            isCompressed = true;
        }
    }

    return isCompressed;
}

struct compressionHeader
{
    fsUInt_t checksum;
    fsUInt_t blockSize;
};

struct perBlockHeader
{
    fsUInt_t unk;
    fsUInt_t uncompressedSize;
    fsUInt_t compressedSize;
};

bool xboxIMGCompression::Decompress( CFile *input, CFile *output )
{
    // Make sure we have LZO.
    this->InitializeLZO();

    if ( !this->isUsingLZO )
    {
        // We cannot continue if LZO has failed to initialize.
        return false;
    }

    bool decompressionSuccess = false;

    // Determine what kind of compression we have and decompress.
    fsUInt_t magic = 0;

    bool couldReadMagic = input->ReadUInt( magic );

    if ( couldReadMagic )
    {
        // Also read the compression header.
        compressionHeader header;

        bool couldReadHeader = input->ReadStruct( header );

        if ( couldReadHeader )
        {
            // lzo1x(-999)
            if ( magic == 0x67A3A1CE )
            {
                // Prepare a memory block that will be used as compression buffer.
                void *compressedFileData = malloc( header.blockSize );

                // Copy the work buffer over fast.
                // This leaves the decompress buffer in our class in an invalid state.
                simpleWorkBuffer localDecompressBuffer = this->decompressBuffer;

                bool hasDecompressBufferChanged = false;

                // Make sure we have got any decompression buffer before we start.
                bool hasAllocatedMinimum = localDecompressBuffer.MinimumSize( minimumDecompressBufferSize );

                if ( hasAllocatedMinimum )
                {
                    hasDecompressBufferChanged = true;
                }

                if ( compressedFileData && localDecompressBuffer.IsReady() )
                {
                    size_t segmentRemaining = header.blockSize;

                    bool lzoSuccess = true;

                    checksumCallback_t _checksumCallback = NULL;

                    if ( _performLZOChecksumVerify )
                    {
                        _checksumCallback = this->_checksumCallback;
                    }

                    // Verify the checksum.
                    lzo_uint32_t checksum = 0;

                    if ( _checksumCallback != NULL )
                    {
                        checksum = _checksumCallback( 0, NULL, 0 );
                    }

                    // Read all blocks.
                    while ( true )
                    {
                        // Check whether we reached the end.
                        if ( segmentRemaining == 0 )
                        {
                            // We can safely exit the loop here.
                            break;
                        }

                        // Read the block header.
                        perBlockHeader blockHeader;

                        bool couldReadBlock = input->ReadStruct( blockHeader );

                        if ( !couldReadBlock )
                        {
                            lzoSuccess = false;
                            break;
                        }

                        // Verify that this block is valid.
                        if ( blockHeader.compressedSize > header.blockSize )
                        {
                            lzoSuccess = false;
                            break;
                        }

                        // Check some non-trivial stuff.
                        if ( blockHeader.unk != 4 || blockHeader.compressedSize != blockHeader.uncompressedSize )
                        {
                            lzoSuccess = false;
                            break;
                        }

                        // Read the compressed block.
                        size_t dataReadCount = input->Read( compressedFileData, 1, blockHeader.compressedSize );

                        if ( dataReadCount != blockHeader.compressedSize )
                        {
                            lzoSuccess = false;
                            break;
                        }

repeatDecompress:
                        // Decompress our block.
                        void *decompressBuffer = localDecompressBuffer.GetPointer();

                        lzo_uint realDecompressedSize = localDecompressBuffer.GetSize();

                        int lzoerr = lzo1x_decompress_safe(
                            (const unsigned char*)compressedFileData, blockHeader.compressedSize,
                            (unsigned char*)decompressBuffer, &realDecompressedSize,
                            NULL
                        );

                        // Handle valid errors.
                        if ( lzoerr == LZO_E_OUTPUT_OVERRUN )
                        {
                            // Increase buffer size.
                            localDecompressBuffer.Grow( realDecompressedSize );

                            // Remember that we updated the decompression buffer.
                            hasDecompressBufferChanged = true;

                            // Try again.
                            goto repeatDecompress;
                        }

                        if ( lzoerr != LZO_E_OK )
                        {
                            lzoSuccess = false;
                            break;
                        }

                        // Write the decompressed stuff into the file.
                        output->Write( decompressBuffer, 1, realDecompressedSize );

                        if ( _checksumCallback != NULL )
                        {
                            // Update checksum.
                            checksum = _checksumCallback( checksum, decompressBuffer, realDecompressedSize );
                        }

                        // Decrease the remaining bytes.
                        size_t processedSize = ( blockHeader.compressedSize + sizeof( blockHeader ) );

                        if ( segmentRemaining < processedSize )
                        {
                            lzoSuccess = false;
                            break;
                        }

                        segmentRemaining -= processedSize;
                    }

                    if ( lzoSuccess )
                    {
                        // Verify the checksum.
                        bool isDataValid = true;

                        if ( isDataValid && ( _checksumCallback != NULL ) )
                        {
                            bool isChecksumValid = ( checksum == header.checksum );

                            if ( !isChecksumValid )
                            {
                                isDataValid = false;
                            }
                        }

                        if ( isDataValid )
                        {
                            // If we succeeded, we have got decompressed data in the output stream.
                            decompressionSuccess = true;
                        }
                    }
                }

                // Clean up.
                if ( compressedFileData )
                {
                    free( compressedFileData );
                }

                // Update the decompress buffer.
                if ( hasDecompressBufferChanged )
                {
                    // Validate the decompress buffer in our class again.
                    this->decompressBuffer = localDecompressBuffer;
                }

                // Release our buffer so we do not free it.
                localDecompressBuffer.Release();
            }
        }
    }

    return decompressionSuccess;
}

bool xboxIMGCompression::Compress( CFile *input, CFile *output )
{
    // Make sure we have LZO.
    this->InitializeLZO();

    if ( !this->isUsingLZO )
    {
        // We cannot continue if LZO failed to initialize.
        return false;
    }

    // TODO: maybe add write-count verification?

    // Write the magic.
    output->WriteUInt( 0x67A3A1CE );

    // Remember this position in the output file.
    fsOffsetNumber_t genericHeaderOffset = output->TellNative();

    // Write a dummy generic header.
    compressionHeader mainHeader;
    mainHeader.blockSize = 0;       // we will fill this out later.
    mainHeader.checksum = 0;        // TODO: how to calculate this field?

    output->WriteStruct( mainHeader );

    // Prepare the LZO compression environment.
    bool lzoSuccess = false;

    size_t lzoWorkBufferSize = LZO1X_999_MEM_COMPRESS;

    void *lzoCompressionWorkMemory = malloc( lzoWorkBufferSize );

    // Allocate block buffers.
    simpleWorkBuffer uncompressedFileData;

    uncompressedFileData.MinimumSize( this->compressionMaximumBlockSize );

    // We use the decompression buffer as compression buffer.
    simpleWorkBuffer compressionBuffer = this->decompressBuffer;

    bool hasCompressionBufferChanged = false;

    // Make sure we have got something in the compression buffer.
    // Since there is no safe compression, we must use the stuff that Oberhummer uses...
    // His library is bad. We cannot ensure that our stuff does not crash. :/
    uint32 requiredCompressionBufferSize = uncompressedFileData.GetSize() + uncompressedFileData.GetSize() / 16 + 64 + 3;

    bool hasInitializedCompressBuffer = compressionBuffer.MinimumSize( requiredCompressionBufferSize );

    if ( hasInitializedCompressBuffer )
    {
        hasCompressionBufferChanged = true;
    }

    if ( lzoCompressionWorkMemory && compressionBuffer.IsReady() && uncompressedFileData.IsReady() )
    {
        // Do the stuff.
        bool compressionSuccess = true;

        unsigned long rawChecksum = 0;

        checksumCallback_t _checksumCallback = this->_checksumCallback;

        if ( _checksumCallback != NULL )
        {
            // Start with the root checksum.
            rawChecksum = _checksumCallback( 0, NULL, 0 );
        }

        size_t streamSize = 0;

        // Process the blocks.
        while ( true )
        {
            // If we have reached the end of the stream, quit.
            if ( input->IsEOF() )
            {
                // Safe exit.
                break;
            }

            // Read from the file stream.
            void *uncompressedDataBuffer = uncompressedFileData.GetPointer();

            size_t compressionDataSize = input->Read( uncompressedDataBuffer, 1, uncompressedFileData.GetSize() );

            // If we could not read anything, we kinda failed.
            if ( compressionDataSize == 0 )
            {
                compressionSuccess = false;
                break;
            }

            // Calculate the checksum of the raw data.
            if ( _checksumCallback != NULL )
            {
                rawChecksum = _checksumCallback( rawChecksum, uncompressedDataBuffer, compressionDataSize );
            }

repeatCompression:
            // Perform the compression.
            void *compressedDataBuffer = compressionBuffer.GetPointer();

            lzo_uint realCompressedSize = compressionBuffer.GetSize();

            int lzoerr = lzo1x_999_compress(
                (const unsigned char*)uncompressedDataBuffer, compressionDataSize,
                (unsigned char*)compressedDataBuffer, &realCompressedSize,
                lzoCompressionWorkMemory
            );

            // Process some valid errors.
            if ( lzoerr == LZO_E_OUTPUT_OVERRUN )
            {
                // Increase buffer size.
                compressionBuffer.Grow( realCompressedSize );

                // Remember to update the decompressBuffer field.
                hasCompressionBufferChanged = true;

                // Repeat compression.
                goto repeatCompression;
            }

            // Now if we get an error, we are screwed.
            if ( lzoerr != LZO_E_OK )
            {
                compressionSuccess = false;
                break;
            }

            // Write the block into the output stream.
            perBlockHeader blockHeader;
            blockHeader.compressedSize = realCompressedSize;
            blockHeader.uncompressedSize = realCompressedSize;  // ???
            blockHeader.unk = 4;                                // ???

            output->WriteStruct( blockHeader );

            // Now write the compressed data.
            output->Write( compressedDataBuffer, 1, realCompressedSize );

            // Increase the actual stream size.
            streamSize += sizeof( blockHeader ) + realCompressedSize;
        }

        if ( compressionSuccess )
        {
            // Update the generic header.
            mainHeader.blockSize = streamSize;
            mainHeader.checksum = rawChecksum;

            // If we succeeded in compressing the file, we succeeded in life :)
            lzoSuccess = true;
        }
    }

    // Clean up.
    if ( lzoCompressionWorkMemory )
    {
        free( lzoCompressionWorkMemory );
    }

    // Update the decompressBuffer.
    if ( hasCompressionBufferChanged )
    {
        this->decompressBuffer = compressionBuffer;
    }

    // Release to not free the concurrent buffer.
    compressionBuffer.Release();

    if ( lzoSuccess )
    {
        // Update the main header.
        fsOffsetNumber_t endOffset = output->TellNative();

        output->SeekNative( genericHeaderOffset, SEEK_SET );

        // Write the header.
        output->WriteStruct( mainHeader );

        // Seek back to where we left off after compressing.
        output->SeekNative( endOffset, SEEK_SET );
    }

    return lzoSuccess;
}