/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        Shared/core/fsinternal/CFileSystem.stream.buffered.h
*  PURPOSE:     Buffered stream utilities for block-based streaming
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#ifndef _FILESYSTEM_BLOCK_BASED_STREAMING_
#define _FILESYSTEM_BLOCK_BASED_STREAMING_

class CBufferedStreamWrap : public CFile
{
public:
                        CBufferedStreamWrap( CFile *toBeWrapped, bool deleteOnQuit );
                        ~CBufferedStreamWrap( void );

    size_t              Read            ( void *buffer, size_t sElement, unsigned long iNumElements );
    size_t              Write           ( const void *buffer, size_t sElement, unsigned long iNumElements );
    int                 Seek            ( long iOffset, int iType );
    int                 SeekNative      ( fsOffsetNumber_t iOffset, int iType );
    long                Tell            ( void ) const;
    fsOffsetNumber_t    TellNative      ( void ) const;
    bool                IsEOF           ( void ) const;
    bool                Stat            ( struct stat *stats ) const;
    void                PushStat        ( const struct stat *stats );
    void                SetSeekEnd      ( void );
    size_t              GetSize         ( void ) const;
    fsOffsetNumber_t    GetSizeNative   ( void ) const;
    void                Flush           ( void );
    const filePath&     GetPath         ( void ) const;
    bool                IsReadable      ( void ) const;
    bool                IsWriteable     ( void ) const;

private:
    template <typename unsignedType>
    static inline unsignedType unsignedBarrierSubtract( unsignedType one, unsignedType two, unsignedType barrier )
    {
        if ( ( one + barrier ) < two )
        {
            return barrier;
        }

        return one - two;
    }

    template <typename numberType>
    class sliceOfData
    {
    public:
        inline sliceOfData( numberType startOffset, numberType dataSize )
        {
            this->startOffset = startOffset;
            this->endOffset = startOffset + ( dataSize - 1 );

            // Since we are working with C++, we throw no exceptions.
            // The developer must make sure that this assertion is not triggered.
            assert( startOffset <= endOffset );
        }

        enum eIntersectionResult
        {
            INTERSECT_EQUAL,
            INTERSECT_INSIDE,
            INTERSECT_BORDER_START,
            INTERSECT_BORDER_END,
            INTERSECT_ENCLOSING,
            INTERSECT_FLOATING_START,
            INTERSECT_FLOATING_END,
            INTERSECT_UNKNOWN   // if something went horribly wrong (like NaNs).
        };

        // Static methods for easier result management.
        static bool isBorderIntersect( eIntersectionResult result )
        {
            return ( result == INTERSECT_BORDER_START || result == INTERSECT_BORDER_END );
        }

        static bool isFloatingIntersect( eIntersectionResult result )
        {
            return ( result == INTERSECT_FLOATING_START || result == INTERSECT_FLOATING_END );
        }

        inline numberType GetSliceStartPoint( void ) const
        {
            return startOffset;
        }

        inline numberType GetSliceEndPoint( void ) const
        {
            return endOffset;
        }
   
        inline eIntersectionResult intersectWith( const sliceOfData& right ) const
        {
            // Get generic stuff.
            numberType sliceStartA = startOffset, sliceEndA = endOffset;
            numberType sliceStartB = right.startOffset, sliceEndB = right.endOffset;

            // Handle all cases.
            // We only implement the logic with comparisons only, as it is the most transparent for all number types.
            if ( sliceStartA == sliceStartB && sliceEndA == sliceEndB )
            {
                return INTERSECT_EQUAL;
            }

            if ( sliceStartB >= sliceStartA && sliceEndB <= sliceEndA )
            {
                return INTERSECT_ENCLOSING;
            }

            if ( sliceStartB <= sliceStartA && sliceEndB >= sliceEndA )
            {
                return INTERSECT_INSIDE;
            }

            if ( sliceStartB < sliceStartA && ( sliceEndB >= sliceStartA && sliceEndB <= sliceEndA ) )
            {
                return INTERSECT_BORDER_START;
            }

            if ( sliceEndB > sliceEndA && ( sliceStartB >= sliceStartA && sliceStartB <= sliceEndA ) )
            {
                return INTERSECT_BORDER_END;
            }

            if ( sliceStartB < sliceStartA && sliceEndB < sliceStartA )
            {
                return INTERSECT_FLOATING_END;
            }

            if ( sliceStartB > sliceEndA && sliceEndB > sliceEndA )
            {
                return INTERSECT_FLOATING_START;
            }

            return INTERSECT_UNKNOWN;
        }

    private:
        numberType startOffset;
        numberType endOffset;
    };

    // Type of the number for seeking operations.
    // Use this number sparsely as operations are slow on it.
    typedef fsOffsetNumber_t seekType_t;

    // Type of the slice we use internally.
    typedef sliceOfData <seekType_t> seekSlice_t;

    template <typename bufType, typename numberType>
    class bufferedStorage
    {
        numberType storageSize;
        bufType *storagePtr;

    public:
        inline bufferedStorage( void )
        {
            this->storageSize = 0;
            this->storagePtr = NULL;
        }

        inline ~bufferedStorage( void )
        {
            // Free any used memory.
            DeleteStorage();
        }

        inline void DeleteStorage( void )
        {
            if ( storagePtr )
            {
                delete [] storagePtr;

                storagePtr = NULL;
            }
        }

        inline bool HasChanged( void ) const
        {
            // TODO
            return true;
        }

        inline void SetChanged( bool hasChanged )
        {
            // TODO
            return;
        }

        inline void AllocateStorage( numberType storageSize )
        {
            DeleteStorage();

            storagePtr = new bufType( (size_t)storageSize );

            this->storageSize = storageSize;
        }

        inline numberType GetStorageSize( void ) const
        {
            return this->storageSize;
        }

        inline numberType GetDataOperationSliceCount( numberType sequenceOffset ) const
        {
            return unsignedBarrierSubtract( storageSize, sequenceOffset, (numberType)0 );
        }

        inline void SetDataSequence( numberType sequenceOffset, const bufType dataArray[], numberType writeDataCount, numberType& writtenDataCount )
        {
            // Obtain the count we are allowed to operate on.
            // We can only write positive amounts.
            numberType canWriteCount = min( GetDataOperationSliceCount( sequenceOffset ), writeDataCount );

            // Write the actual data.
            if ( canWriteCount > 0 )
            {
                std::copy( dataArray, dataArray + canWriteCount, storagePtr + sequenceOffset );
            }

            // Pass the amount of written data back to the calling runtime.
            writtenDataCount = canWriteCount;
        }

        inline void GetDataSequence( numberType sequenceOffset, bufType dataArray[], numberType readDataCount, numberType& actualReadDataCount ) const
        {
            // Get the amount of data we can read.
            numberType canReadCount = std::min( GetDataOperationSliceCount( sequenceOffset ), readDataCount );

            // Read the data from the buffer.
            if ( canReadCount > 0 )
            {
                const bufType *dataSource = storagePtr + sequenceOffset;

                std::copy( dataSource, dataSource + canReadCount, dataArray );
            }

            // Pass the amount of read data back to the callee.
            actualReadDataCount = canReadCount;
        }

        inline void FillWithFileSector( CFile *srcFile )
        {
            srcFile->Read(
                this->storagePtr,
                1,
                (size_t)this->storageSize
            );
        }

        inline void PushToFile( CFile *dstFile )
        {
            dstFile->Write(
                this->storagePtr,
                1,
                (size_t)this->storageSize
            );
        }
    };

    // The idea behind this class is:
    // Cache the logical stream position, so we avoid system calls when unnecessary.
    struct virtualStreamSeekPtr
    {
        seekType_t seekPtr;         // number containing the current seek into the stream.
        CBufferedStreamWrap *host;  // host class
        bool needsUpdating;         // boolean whether the current seek ptr needs updating.

        inline virtualStreamSeekPtr( void )
        {
            // Remember to set the host inside of the constructor.
            host = NULL;

            seekPtr = 0;    // while host == NULL, this is invalid.
            needsUpdating = false;
        }

        inline ~virtualStreamSeekPtr( void )
        {
            return;
        }

        inline void SetHost( CBufferedStreamWrap *hostStream )
        {
            this->host = hostStream;
            this->seekPtr = hostStream->underlyingStream->TellNative(); // validate the field.
        }

        inline void Seek( seekType_t totalOffset )
        {
            // Has the seek changed at all?
            if ( totalOffset == seekPtr )
                return;

            // Modify the local seek descriptor.
            seekPtr = totalOffset;

            // Mark that we have to update the local seek to the device ptr.
            needsUpdating = true;
        }

        inline seekType_t Tell( void ) const
        {
            return seekPtr;
        }

        inline void Update( void )
        {
            // Check whether we have to update.
            if ( needsUpdating == false )
                return;

            // Push the local seek pointer into the device handle.
            host->underlyingStream->SeekNative( seekPtr, SEEK_SET );

            // We do not have to update anymore.
            needsUpdating = true;
        }
    };

    // Pointer to the underlying stream that has to be buffered.
    CFile *underlyingStream;

    // If true, underlyingStream is terminated when this buffered file terminates.
    bool terminateUnderlyingData;

    bufferedStorage <char, seekType_t> internalIOBuffer;

    // Location of the buffer on the file-space.
    seekType_t offsetOfBufferOnFileSpace;

    // 64bit number that is the file's seek ptr.
    virtualStreamSeekPtr fileSeek;
};

class CBufferedFile : public CFile
{
public:
                    ~CBufferedFile  ( void );

    size_t          Read            ( void *buffer, size_t sElement, unsigned long iNumElements );
    size_t          Write           ( const void *buffer, size_t sElement, unsigned long iNumElements );
    int             Seek            ( long iOffset, int iType );
    long            Tell            ( void ) const;
    bool            IsEOF           ( void ) const;
    bool            Stat            ( struct stat *stats ) const;
    void            PushStat        ( const struct stat *stats );
    void            SetSeekEnd      ( void );
    size_t          GetSize         ( void ) const;
    void            Flush           ( void );
    const filePath& GetPath         ( void ) const;
    bool            IsReadable      ( void ) const;
    bool            IsWriteable     ( void ) const;

    int             ReadInt();
    short           ReadShort();
    char            ReadByte();

private:
    char*           m_pBuffer;
    long            m_iSeek;
    size_t          m_sSize;
    filePath        m_path;
};

#endif //_FILESYSTEM_BLOCK_BASED_STREAMING_