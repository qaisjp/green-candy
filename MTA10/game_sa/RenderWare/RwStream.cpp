/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        game_sa/RwStream.cpp
*  PURPOSE:     RenderWare RwStream definition
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*  RenderWare is � Criterion Software
*
*****************************************************************************/

#include <StdInc.h>

inline unsigned int RwThrowException( unsigned int errId )
{
    return errId;
}

inline void RwCallException( int err1, unsigned int err2 )
{
    RwError errorInfo;

    errorInfo.err1 = err1;
    errorInfo.err2 = RwThrowException( err2 );

    RwSetError( &errorInfo );
}

inline void RwThrowFileTypeException( void )
{
    RwCallException( 1, 0x0E );
}

inline void RwThrowFileModeException( void )
{
    RwCallException( 1, 0x0D );
}

inline void* _openFile( const char *name, const char *mode )
{
    return pRwInterface->m_fileOpen( name, mode );
}

inline size_t _readFile( void *buf, size_t sElem, unsigned int elemCount, void *fp )
{
    return pRwInterface->m_fileRead( buf, sElem, elemCount, fp );
}

inline size_t _writeFile( const void *buf, size_t sElem, unsigned int elemCount, void *fp )
{
    return pRwInterface->m_fileWrite( buf, sElem, elemCount, fp );
}

inline int _seekFile( void *fp, long seek, int type )
{
    return pRwInterface->m_fileSeek( fp, seek, type );
}

inline int _tellFile( void *fp )
{
    return pRwInterface->m_fileTell( fp );
}

inline int _closeFile( void *fp )
{
    return pRwInterface->m_fileClose( fp );
}

/*=========================================================
    RwStreamInitialize

    Arguments:
        memory - spot to use for stream construction
        isAllocated - tells the system whether the memory
                      ptr belongs to the stream freelist
        type - specifies the stream type (file, filename, buf, cb)
        mode - opening/operation mode of the stream
        ud - userdata dispatched by stream type
    Purpose:
        Constructs a RenderWare stream on the given memory
        location and returns it if the operation was successful.
    Binary offsets:
        (1.0 US): 0x007EC810
        (1.0 EU): 0x007EC850
=========================================================*/
RwStream* __cdecl RwStreamInitialize( void *memory, int isAllocated, RwStreamType type, RwStreamMode mode, void *ud )
{
    if ( !memory )
        return NULL;

    // Create a stream at the given memory pointer.
    RwStream *stream = (RwStream*)memory;

    // Set up the stream.
    stream->type = type;
    stream->mode = mode;
    stream->allocated = isAllocated;

    switch( type )
    {
    case STREAM_TYPE_FILE:
        if ( _tellFile( ud ) == -1 )
            return NULL;

        stream->data.file = ud;
        break;
    case STREAM_TYPE_FILENAME:
        {
            void *handle = NULL;
            const char *path = (const char*)ud;

            switch( mode )
            {
            case STREAM_MODE_READ:
                handle = _openFile( path, "rb" );
                break;
            case STREAM_MODE_WRITE:
                handle = _openFile( path, "wb" );
                break;
            case STREAM_MODE_APPEND:
                handle = _openFile( path, "ab" );
                break;
            default:
                RwThrowFileModeException();
                break;
            }

            if ( !handle )
            {
                RwCallException( 1, 0x80000002 );
                return NULL;
            }

            stream->data.file = handle;
        }
        break;
    case STREAM_TYPE_BUFFER:
        {
            RwBuffer *buf = (RwBuffer*)ud;

            switch( mode )
            {
            case STREAM_MODE_READ:
                stream->data.position = 0;
                stream->data.size = buf->size;
                stream->data.dataPtr = buf->ptr;
                break;
            case STREAM_MODE_WRITE:
                stream->data.position = 0;
                stream->data.size = 0;
                stream->data.dataPtr = NULL;
                break;
            case STREAM_MODE_APPEND:
                stream->data.position = buf->size;
                stream->data.size = buf->size;
                stream->data.dataPtr = buf->ptr;
                break;
            default:
                RwThrowFileModeException();
                break;
            }
            
        }
        break;
    case STREAM_TYPE_CALLBACK:
        stream->data = *(RwStreamTypeData*)ud;
        break;
    default:
        RwThrowFileTypeException();
        break;
    }

    return stream;
}

/*=========================================================
    RwStreamOpen

    Arguments:
        type - specifies the stream type (file, filename, buf, cb)
        mode - opening/operation mode of the stream
        ud - userdata dispatched by steam type
    Purpose:
        Allocates a new RenderWare stream on the heap and
        returns it if the operation was successful.
    Binary offsets:
        (1.0 US): 0x007ECEF0
        (1.0 EU): 0x007ECF30
=========================================================*/
RwStream* __cdecl RwStreamOpen( RwStreamType type, RwStreamMode mode, void *ud )
{
    void *memory = pRwInterface->m_allocStruct( pRwInterface->m_streamInfo, 0x30404 );

    RwStream *stream = RwStreamInitialize( memory, true, type, mode, ud );

    if ( !stream )
    {
        pRwInterface->m_freeStruct( pRwInterface->m_streamInfo, memory );
        return NULL;
    }

    return stream;
}

/*=========================================================
    RwStreamReadBlocks

    Arguments:
        stream - RenderWare stream pointer
        buf - buffer to write bytes to
        size - amount of bytes to read
    Purpose:
        Attempts to read from the given stream and returns
        how many bytes have been read.
    Binary offsets:
        (1.0 US): 0x007EC9D0
        (1.0 EU): 0x007ECA10
=========================================================*/
unsigned int __cdecl RwStreamReadBlocks( RwStream *stream, void *buf, unsigned int size )
{
    unsigned int readCount = 0;

    switch( stream->type )
    {
    case STREAM_TYPE_FILE:
    case STREAM_TYPE_FILENAME:
        {
            void *fp = stream->data.file;

            readCount = _readFile( buf, 1, size, fp );

            if ( readCount != size )
            {
                int err = _closeFile( fp );

                RwCallException( 1, !err ? 5 : 0x8000001A );
            }
        }
        break;
    case STREAM_TYPE_BUFFER:
        {
            unsigned int position = stream->data.position;
            unsigned int dataSize = stream->data.size;

            unsigned int leftToRead = dataSize - position;

            if ( size > leftToRead )
            {
                size = leftToRead;

                RwCallException( 1, 5 );
            }

            memcpy( buf, (const char*)stream->data.dataPtr + position, size );

            stream->data.position += size;

            readCount = size;
        }
        break;
    case STREAM_TYPE_CALLBACK:
        readCount = stream->data.callbackRead( stream->data.ptr_callback, buf, size );
        break;
    default:
        RwThrowFileTypeException();
        break;
    }

    return readCount;
}

/*=========================================================
    RwStreamWriteBlocks

    Arguments:
        stream - RenderWare stream pointer
        buf - buffer to read bytes from
        size - amount of bytes to write
    Purpose:
        Writes bytes into the given RenderWare stream and
        returns the stream if the operation was without
        errors.
    Binary offsets:
        (1.0 US): 0x007ECB30
        (1.0 EU): 0x007ECB70
=========================================================*/
RwStream* __cdecl RwStreamWriteBlocks( RwStream *stream, const void *buf, unsigned int size )
{
    switch( stream->type )
    {
    case STREAM_TYPE_FILE:
    case STREAM_TYPE_FILENAME:
        {
            void *fp = stream->data.file;

            unsigned int writeCount = _writeFile( buf, 1, size, fp );

            if ( writeCount != size )
                RwCallException( 1, 0x8000001C );
        }
        break;
    case STREAM_TYPE_BUFFER:
        {
            void *writeBuf = stream->data.dataPtr;

            if ( !writeBuf )
            {
                writeBuf = pRwInterface->m_memory.m_malloc( RW_STREAM_WRITEBUF_SIZE, 0x30404 );

                if ( !writeBuf )
                {
                    RwCallException( 1, 0x80000013 );
                    return NULL;
                }

                stream->data.dataPtr = writeBuf;
                stream->data.size = RW_STREAM_WRITEBUF_SIZE;
            }

            unsigned int position = stream->data.position;
            unsigned int size = stream->data.size;

            unsigned int leftToWrite = size - position;

            if ( size > leftToWrite )
            {
                size_t newMemSize = position + std::max( (unsigned int)RW_STREAM_WRITEBUF_SIZE, size );

                writeBuf = pRwInterface->m_memory.m_realloc( writeBuf, newMemSize );

                if ( !writeBuf )
                {
                    RwCallException( 1, 0x80000013 );
                    return NULL;
                }

                stream->data.dataPtr = writeBuf;
                stream->data.size = newMemSize;
            }

            memcpy( (char*)writeBuf + position, buf, size );

            stream->data.position += size;
        }
        break;
    case STREAM_TYPE_CALLBACK:
        {
            size_t writeCount = stream->data.callbackWrite( stream->data.ptr_callback, buf, size );

            if ( writeCount == 0 )
                return NULL;
        }
        break;
    default:
        RwThrowFileTypeException();
        break;
    }

    return stream;
}

/*=========================================================
    RwStreamSkip

    Arguments:
        stream - RenderWare stream pointer
        count - amount of bytes to skip ahead
    Purpose:
        Moves the file pointer forward by count. Returns
        the stream if the operation was without errors,
        NULL otherwise.
    Binary offsets:
        (1.0 US): 0x007ECD00
        (1.0 EU): 0x007ECD40
=========================================================*/
RwStream* __cdecl RwStreamSkip( RwStream *stream, unsigned int count )
{
    if ( count == 0 )
        return stream;

    switch( stream->type )
    {
    case STREAM_TYPE_FILE:
    case STREAM_TYPE_FILENAME:
        {
            void *fp = stream->data.file;

            int err = _seekFile( fp, count, SEEK_CUR );

            if ( err != 0 )
            {
                err = _closeFile( fp );

                if ( err != 0 )
                    RwCallException( 1, 5 );

                return NULL;
            }
        }
        break;
    case STREAM_TYPE_BUFFER:
        {
            unsigned int newPos = stream->data.position + count;
            unsigned int size = stream->data.size;

            if ( newPos > size )
            {
                stream->data.position = size;

                RwCallException( 1, 5 );
                return NULL;
            }

            stream->data.position = newPos;
        }
        break;
    case STREAM_TYPE_CALLBACK:
        {
            void *ret = stream->data.callbackSeek( stream->data.ptr_callback, count );

            if ( !ret )
                return NULL;
        }
        break;
    default:
        RwThrowFileTypeException();
        break;
    }

    return stream;
}

/*=========================================================
    RwStreamSeek (MTA extension)

    Arguments:
        stream - RenderWare stream pointer
        offset - value to offset the stream pointer by
        type - ANSI seek type descriptor
    Purpose:
        Attempts to seek the current stream by an offset.
        Returns the stream handle if successful, NULL otherwise.
    Note:
        Not all RenderWare stream types support this operation.
=========================================================*/
RwStream* RwStreamSeek( RwStream *stream, long offset, int type )
{
    if ( offset == 0 )
        return stream;

    bool success = false;

    switch( stream->type )
    {
    case STREAM_TYPE_FILE:
    case STREAM_TYPE_FILENAME:
        {
            void *fp = stream->data.file;

            int err = _seekFile( fp, offset, type );

            if ( err != 0 )
            {
                err = _closeFile( fp );

                if ( err != 0 )
                    RwCallException( 1, 5 );

                return NULL;
            }
        }
        break;
    case STREAM_TYPE_BUFFER:
        {
            unsigned int newPos;
            long base = 0;

            switch( type )
            {
            case SEEK_CUR:      base = stream->data.position; break;
            case SEEK_END:      base = stream->data.size; break;
            case SEEK_SET:      base = 0; break;
            default:
                RwCallException( 1, 5 );
                return NULL;
            }

            if ( offset < 0 )
            {
                long realOffset = -offset;

                if ( base < realOffset )
                {
                    newPos = 0;
                }
                else
                    newPos = base - realOffset;
            }
            else
            {
                newPos = std::min( (long)stream->data.size, base + offset );
            }

            stream->data.position = newPos;
        }
        break;
    case STREAM_TYPE_CALLBACK:
        // We would break binary compatibility if we implemented this, unfortunately.
        success = false;
        break;
    default:
        RwThrowFileTypeException();
        break;
    }

    return success ? stream : NULL;
}

/*=========================================================
    RwStreamClose

    Arguments:
        stream - RenderWare stream pointer
        count - amount of bytes to skip ahead
    Purpose:
        Terminates activity on the given RenderWare stream.
        If ud != NULL and the stream is of BUFFER type and
        it was not in reading mode, the ud pointer is
        of RwBuffer type and in it wll be written the writeBuffer
        of the closing stream.
    Binary offsets:
        (1.0 US): 0x007ECE20
        (1.0 EU): 0x007ECE60
=========================================================*/
int __cdecl RwStreamClose( RwStream *stream, void *ud )
{
    bool success = false;

    switch( stream->type )
    {
    case STREAM_TYPE_FILE:
        success = true;
        break;
    case STREAM_TYPE_FILENAME:
        {
            int err = _closeFile( stream->data.file );

            success = ( err != 0 );
        }
        break;
    case STREAM_TYPE_BUFFER:
        success = true;

        if ( stream->mode != STREAM_MODE_READ )
        {
            if ( ud )
            {
                RwBuffer *buf = (RwBuffer*)ud;

                buf->ptr = stream->data.dataPtr;
                buf->size = stream->data.position;
            }
            else
            {
                // Fixed some memory leak.
                void *memory = stream->data.dataPtr;

                if ( memory )
                    pRwInterface->m_memory.m_free( memory );
            }
        }
        break;
    case STREAM_TYPE_CALLBACK:
        {
            RwIOCallbackClose closecb = stream->data.callbackClose;

            if ( closecb )
                closecb( stream->data.ptr_callback );

            success = true;
        }
    default:
        RwThrowFileTypeException();
        break;
    }

    if ( stream->allocated )
    {
        pRwInterface->m_freeStruct( pRwInterface->m_streamInfo, stream );
    }

    return success;
}

void RwStream_Init( void )
{
    // Hook stream functions (to the demise of the network module)
    switch( pGame->GetGameVersion() )
    {
    case VERSION_EU_10:
        HookInstall( 0x007EC850, (DWORD)RwStreamInitialize, 5 );
        HookInstall( 0x007ECF30, (DWORD)RwStreamOpen, 5 );
        HookInstall( 0x007ECA10, (DWORD)RwStreamReadBlocks, 5 );
        HookInstall( 0x007ECB70, (DWORD)RwStreamWriteBlocks, 5 );
        HookInstall( 0x007ECD40, (DWORD)RwStreamSkip, 5 );
        HookInstall( 0x007ECE60, (DWORD)RwStreamClose, 5 );
        break;
    case VERSION_US_10:
        HookInstall( 0x007EC810, (DWORD)RwStreamInitialize, 5 );
        HookInstall( 0x007ECEF0, (DWORD)RwStreamOpen, 5 );
        HookInstall( 0x007EC9D0, (DWORD)RwStreamReadBlocks, 5 );
        HookInstall( 0x007ECB30, (DWORD)RwStreamWriteBlocks, 5 );
        HookInstall( 0x007ECD00, (DWORD)RwStreamSkip, 5 );
        HookInstall( 0x007ECE20, (DWORD)RwStreamClose, 5 );
        break;
    }
}

void RwStream_Shutdown( void )
{
}