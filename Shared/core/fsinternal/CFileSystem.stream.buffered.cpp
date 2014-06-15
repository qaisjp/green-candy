/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        Shared/core/fsinternal/CFileSystem.stream.buffered.cpp
*  PURPOSE:     Buffered stream utilities for block-based streaming
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/
#include <StdInc.h>

// Include internal header.
#include "CFileSystem.internal.h"

/*===================================================
    CBufferedStreamWrap

    An extension of the raw file that uses buffered IO.
    Since, in reality, hardware is sector based, the
    preferred way of writing data to disk is using buffers.

    Always prefer this class instead of CRawFile!
    Only use raw communication if you intend to put your
    own buffering!

    While a file stream is wrapped usage of the toBeWrapped
    pointer outside of the wrapper class leads to
    undefined behavior.
    
    Arguments:
        toBeWrapped - stream pointer that should be buffered
        deleteOnQuit - if true, toBeWrapped is deleted aswell
                       when this class is deleted

    Cool Ideas:
    -   Create more interfaces that wrap FileSystem streams
        so applying attributes to streams is a simple as
        wrapping a virtual class
===================================================*/

CBufferedStreamWrap::CBufferedStreamWrap( CFile *toBeWrapped, bool deleteOnQuit ) : underlyingStream( toBeWrapped )
{
    internalIOBuffer.AllocateStorage(
        systemCapabilities.GetSystemLocationSectorSize( *toBeWrapped->GetPath().c_str() )
    );

    offsetOfBufferOnFileSpace = 0;

    fileSeek.SetHost( this );

    // Fill the buffer with the beginning content.
    fsOffsetNumber_t actualFileSeek = toBeWrapped->TellNative();

    internalIOBuffer.FillWithFileSector( toBeWrapped );

    toBeWrapped->SeekNative( actualFileSeek, SEEK_SET );
}

CBufferedStreamWrap::~CBufferedStreamWrap( void )
{
    if ( terminateUnderlyingData == true )
    {
        delete underlyingStream;
    }

    underlyingStream = NULL;
}

size_t CBufferedStreamWrap::Read( void *buffer, size_t sElement, unsigned long iNumElements )
{
    // Add simple error checking.
    // It could be fatal to the application to introduce an infinite loop here.
    unsigned int methodRepeatCount = 0;

repeatMethod:
    methodRepeatCount++;

    if ( methodRepeatCount == 1024 )
        throw std::exception( "infinite buffered read repetition count" );

    // Do the actual logic.
    size_t requestedReadCount = sElement * iNumElements;

    seekType_t localFileSeek = fileSeek.Tell();

    // Create the slices for the seeking operation.
    // We will collide them against each other.
    seekSlice_t readSlice( localFileSeek, localFileSeek + requestedReadCount );
    seekSlice_t bufferSlice( this->offsetOfBufferOnFileSpace, this->offsetOfBufferOnFileSpace + internalIOBuffer.GetStorageSize() );

    seekSlice_t::eIntersectionResult intResult = readSlice.intersectWith( bufferSlice );

    seekType_t totalReadCount = 0;

    if ( intResult == seekSlice_t::INTERSECT_INSIDE )
    {
        internalIOBuffer.GetDataSequence( localFileSeek, (char*)buffer, requestedReadCount, totalReadCount );

        fileSeek.Seek( localFileSeek + totalReadCount );
    }
    else if ( intResult == seekSlice_t::INTERSECT_BORDER_START )
    {
        // Everything read-able has to fit inside client memory.
        // A size_t is assumed to be as big as the client memory allows.
        size_t sliceStartOffset = (size_t)( bufferSlice.GetSliceStartPoint() - localFileSeek );
        
        // First read from the file natively, to reach the buffer border.
        if ( sliceStartOffset > 0 )
        {
            size_t actualReadCount = 0;

            actualReadCount = underlyingStream->Read( buffer, 1, sliceStartOffset );

            assert( actualReadCount == sliceStartOffset );

            // Update the file seek.
            localFileSeek += sliceStartOffset;
            fileSeek.Seek( localFileSeek );
        }

        // Now lets read the remainder from the buffer.
        size_t sliceReadRemainderCount = (size_t)( requestedReadCount - sliceStartOffset );

        if ( sliceReadRemainderCount > 0 )
        {
            // Set the file seek to current.
            fileSeek.Update();

            char *outBuf = (char*)buffer + sliceStartOffset;

            seekType_t actualMemRead = 0;

            internalIOBuffer.GetDataSequence( 0, outBuf, sliceReadRemainderCount, actualMemRead );

            assert( actualMemRead == sliceReadRemainderCount );
        }
    }
    else if ( intResult == seekSlice_t::INTERSECT_BORDER_END )
    {
        size_t sliceEndOffset = (size_t)( bufferSlice.GetSliceEndPoint() - localFileSeek );

        // Read what can be read from the native buffer.
        if ( sliceEndOffset > 0 )
        {
            seekType_t sliceReadInCount = bufferSlice.GetSliceEndPoint() - sliceEndOffset;

            seekType_t actualMemRead = 0;

            internalIOBuffer.GetDataSequence( sliceReadInCount, (char*)buffer, sliceEndOffset, actualMemRead );

            assert( actualMemRead == sliceEndOffset );

            // Update the local file seek.
            localFileSeek += sliceEndOffset;
            fileSeek.Seek( localFileSeek );
        }

        // Read from the native file now.
        size_t sliceReadRemainderCount = (size_t)( requestedReadCount - sliceEndOffset );

        if ( sliceReadRemainderCount > 0 )
        {
            // Orient the underlying stream.
            fileSeek.Update();

            size_t actualMemRead = 0;

            char *outBuffer = (char*)buffer + sliceEndOffset;

            actualMemRead = underlyingStream->Read( outBuffer, 1, sliceReadRemainderCount );

            assert( actualMemRead == sliceReadRemainderCount );
        }
    }
    else if ( intResult == seekSlice_t::INTERSECT_ENCLOSING )
    {
        // Read the beginning segment, that is native file memory.
        size_t sliceStartOffset = (size_t)( bufferSlice.GetSliceStartPoint() - localFileSeek );

        if ( sliceStartOffset > 0 )
        {
            size_t actualReadCount = 0;

            actualReadCount = underlyingStream->Read( buffer, 1, sliceStartOffset );

            assert( actualReadCount == sliceStartOffset );

            // Update the seek ptr.
            fileSeek.Seek( localFileSeek += sliceStartOffset );
        }

        // Put the content of the entire internal buffer into the output buffer.
        {
            char *outputBuf = (char*)buffer + sliceStartOffset;

            seekType_t actualInternalRead = 0;

            internalIOBuffer.GetDataSequence( 0, outputBuf, internalIOBuffer.GetStorageSize(), actualInternalRead );

            assert( actualInternalRead == internalIOBuffer.GetStorageSize() );

            fileSeek.Seek( localFileSeek += internalIOBuffer.GetStorageSize() );
        }

        // Read the part after the internal buffer slice.
        size_t sliceEndOffset = (size_t)( readSlice.GetSliceEndPoint() - bufferSlice.GetSliceEndPoint() );

        if ( sliceEndOffset > 0 )
        {
            char *outputBuf = (char*)buffer + sliceStartOffset + internalIOBuffer.GetStorageSize();

            size_t actualReadCount = 0;

            fileSeek.Update();

            actualReadCount = underlyingStream->Read( outputBuf, 1, sliceEndOffset );

            assert( actualReadCount == sliceEndOffset );

            fileSeek.Seek( localFileSeek += sliceEndOffset );
        }
    }
    else if ( seekSlice_t::isFloatingIntersect( intResult ) )
    {
        // Align the buffer so that the file seek is inside the buffer (begin of the reader slice)
        seekType_t newBufferOffset = ALIGN_SIZE( localFileSeek, internalIOBuffer.GetStorageSize() );

        this->offsetOfBufferOnFileSpace = newBufferOffset;

        // Read data into the buffer.
        fileSeek.Seek( newBufferOffset );
        fileSeek.Update();

        internalIOBuffer.FillWithFileSector( underlyingStream );

        // Attempt to repeat reading.
        goto repeatMethod;
    }
    else
    {
        // We have no hit in any way that we can detect.
        // Throw an exception.
        assert( 0 );
    }

    return (size_t)totalReadCount;
}

size_t CBufferedStreamWrap::Write( const void *buffer, size_t sElement, unsigned long iNumElements )
{
    // TODO: write this method.
    return 0;
}

int CBufferedStreamWrap::Seek( long iOffset, int iType )
{
    // Update the seek with a normal number.
    // This is a faster method than SeekNative.
    long offsetBase = 0;

    if ( iType == SEEK_CUR )
    {
        offsetBase = (long)fileSeek.Tell();
    }
    else if ( iType == SEEK_SET )
    {
        offsetBase = 0;
    }
    else if ( iType == SEEK_END )
    {
        offsetBase = underlyingStream->GetSize();
    }

    fileSeek.Seek( (long)( offsetBase + iOffset ) );
    return 0;
}

int CBufferedStreamWrap::SeekNative( fsOffsetNumber_t iOffset, int iType )
{
    // Update the seek with a bigger number.
    seekType_t offsetBase = 0;

    if ( iType == SEEK_CUR )
    {
        offsetBase = fileSeek.Tell();
    }
    else if ( iType == SEEK_SET )
    {
        offsetBase = 0;
    }
    else if ( iType == SEEK_END )
    {
        offsetBase = (seekType_t)underlyingStream->GetSizeNative();
    }

    fileSeek.Seek( offsetBase + iOffset );
    return 0;
}

long CBufferedStreamWrap::Tell( void ) const
{
    return (long)fileSeek.Tell();
}

fsOffsetNumber_t CBufferedStreamWrap::TellNative( void ) const
{
    return fileSeek.Tell();
}

bool CBufferedStreamWrap::IsEOF( void ) const
{
    // Update the underlying stream's seek ptr and see if it finished.
    const_cast <virtualStreamSeekPtr&> ( fileSeek ).Update();

    return underlyingStream->IsEOF();
}

bool CBufferedStreamWrap::Stat( struct stat *stats ) const
{
    // Redirect this functionality to the underlying stream.
    // We are not supposed to modify and of these logical attributes.
    return underlyingStream->Stat( stats );
}

void CBufferedStreamWrap::PushStat( const struct stat *stats )
{
    // Attempt to modify the stream's meta data.
    underlyingStream->PushStat( stats );
}

void CBufferedStreamWrap::SetSeekEnd( void )
{
    // Finishes the stream at the given offset.
    fileSeek.Update();

    underlyingStream->SetSeekEnd();
}

size_t CBufferedStreamWrap::GetSize( void ) const
{
    return underlyingStream->GetSize();
}

fsOffsetNumber_t CBufferedStreamWrap::GetSizeNative( void ) const
{
    return underlyingStream->GetSizeNative();
}

void CBufferedStreamWrap::Flush( void )
{
    // Write our buffer into the file (if it changed).
    if ( internalIOBuffer.HasChanged() )
    {
        // Write the buffer contents to the stream.
        seekType_t currentFileSeek = fileSeek.Tell();

        // Set the file seek to the logical position of the IO buffer.
        // Then write the contents to it.
        fileSeek.Seek( this->offsetOfBufferOnFileSpace );
        fileSeek.Update();

        internalIOBuffer.PushToFile( underlyingStream );

        // Restore the original file seek.
        // This is only being nice; it does not have to be restored.
        fileSeek.Seek( currentFileSeek );

        // We cleared the change.
        internalIOBuffer.SetChanged( false );
    }

    // Write the remaining OS buffers.
    underlyingStream->Flush();
}

const filePath& CBufferedStreamWrap::GetPath( void ) const
{
    return underlyingStream->GetPath();
}

bool CBufferedStreamWrap::IsReadable( void ) const
{
    return underlyingStream->IsReadable();
}

bool CBufferedStreamWrap::IsWriteable( void ) const
{
    return underlyingStream->IsWriteable();
}

/*=========================================
    CBufferedFile

    Loads a file at open and keeps it in a managed buffer.

    Info: this is a deprecated class.
=========================================*/

CBufferedFile::~CBufferedFile( void )
{
    return;
}

size_t CBufferedFile::Read( void *pBuffer, size_t sElement, unsigned long iNumElements )
{
    long iReadElements = std::min( ( m_sSize - m_iSeek ) / sElement, iNumElements );
    size_t sRead = iReadElements * sElement;

    if ( iNumElements == 0 )
        return 0;

    memcpy( pBuffer, m_pBuffer + m_iSeek, sRead );
    m_iSeek += sRead;
    return iReadElements;
}

size_t CBufferedFile::Write( const void *pBuffer, size_t sElement, unsigned long iNumElements )
{
    return 0;
}

int CBufferedFile::Seek( long iOffset, int iType )
{
    switch( iType )
    {
    case SEEK_SET:
        m_iSeek = 0;
        break;
    case SEEK_END:
        m_iSeek = m_sSize;
        break;
    }

    m_iSeek = std::max( 0l, std::min( m_iSeek + iOffset, (long)m_sSize ) );
    return 0;
}

long CBufferedFile::Tell( void ) const
{
    return m_iSeek;
}

bool CBufferedFile::IsEOF( void ) const
{
    return ( (size_t)m_iSeek == m_sSize );
}

bool CBufferedFile::Stat( struct stat *stats ) const
{
    stats->st_dev = -1;
    stats->st_ino = -1;
    stats->st_mode = -1;
    stats->st_nlink = -1;
    stats->st_uid = -1;
    stats->st_gid = -1;
    stats->st_rdev = -1;
    stats->st_size = m_sSize;
    stats->st_atime = 0;
    stats->st_ctime = 0;
    stats->st_mtime = 0;
    return 0;
}

void CBufferedFile::PushStat( const struct stat *stats )
{
    // Does not do anything.
    return;
}

size_t CBufferedFile::GetSize( void ) const
{
    return m_sSize;
}

void CBufferedFile::SetSeekEnd( void )
{
    return;
}

void CBufferedFile::Flush( void )
{
    return;
}

const filePath& CBufferedFile::GetPath( void ) const
{
    return m_path;
}

int CBufferedFile::ReadInt( void )
{
    int iResult;

    if ( ( m_sSize - m_iSeek ) < sizeof(int) )
        return 0;

    iResult = *(int*)( m_pBuffer + m_iSeek );
    m_iSeek += sizeof(int);
    return iResult;
}

short CBufferedFile::ReadShort( void )
{
    short iResult;

    if ( (m_sSize - m_iSeek) < sizeof(short) )
        return 0;

    iResult = *(short*)(m_pBuffer + m_iSeek);
    m_iSeek += sizeof(short);
    return iResult;
}

char CBufferedFile::ReadByte( void )
{
    if ( m_sSize == (size_t)m_iSeek )
        return 0;

    return *(m_pBuffer + m_iSeek++);
}

bool CBufferedFile::IsReadable( void ) const
{
    return true;
}

bool CBufferedFile::IsWriteable( void ) const
{
    return false;
}