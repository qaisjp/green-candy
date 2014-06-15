/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        Shared/core/fsinternal/CFileSystem.stream.raw.cpp
*  PURPOSE:     Raw OS filesystem file link
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/
#include <StdInc.h>

// Include internal header.
#include "CFileSystem.internal.h"

// List of all active files.
// Exported from the main FileSystem class.
extern std::list <CFile*> *openFiles;

/*===================================================
    CRawFile

    This class represents a file on the system.
    As long as it is present, the file is opened.

    WARNING: using this class directly is discouraged,
        as it uses direct methods of writing into
        hardware. wrap it with CBufferedStreamWrap instead!

    fixme: Port to mac
===================================================*/

CRawFile::CRawFile( const filePath& absFilePath )
{
    openFiles->push_back( this );

    m_path = absFilePath;
}

CRawFile::~CRawFile( void )
{
#ifdef _WIN32
    CloseHandle( m_file );
#elif defined(__linux__)
    fclose( m_file );
#endif //OS DEPENDANT CODE

    openFiles->remove( this );
}

size_t CRawFile::Read( void *pBuffer, size_t sElement, unsigned long iNumElements )
{
#ifdef _WIN32
    DWORD dwBytesRead;

    if (sElement == 0 || iNumElements == 0)
        return 0;

    ReadFile(m_file, pBuffer, sElement * iNumElements, &dwBytesRead, NULL);
    return dwBytesRead / sElement;
#elif defined(__linux__)
    return fread( pBuffer, sElement, iNumElements, m_file );
#else
    return 0;
#endif //OS DEPENDANT CODE
}

size_t CRawFile::Write( const void *pBuffer, size_t sElement, unsigned long iNumElements )
{
#ifdef _WIN32
    DWORD dwBytesWritten;

    if (sElement == 0 || iNumElements == 0)
        return 0;

    WriteFile(m_file, pBuffer, sElement * iNumElements, &dwBytesWritten, NULL);
    return dwBytesWritten / sElement;
#elif defined(__linux__)
    return fwrite( pBuffer, sElement, iNumElements, m_file );
#else
    return 0;
#endif //OS DEPENDANT CODE
}

int CRawFile::Seek( long iOffset, int iType )
{
#ifdef _WIN32
    if (SetFilePointer(m_file, iOffset, NULL, iType) == INVALID_SET_FILE_POINTER)
        return -1;
    return 0;
#elif defined(__linux__)
    return fseek( m_file, iOffset, iType );
#else
    return -1;
#endif //OS DEPENDANT CODE
}

int CRawFile::SeekNative( fsOffsetNumber_t iOffset, int iType )
{
    // TODO.
    return -1;
}

long CRawFile::Tell( void ) const
{
#ifdef _WIN32
    return SetFilePointer( m_file, 0, NULL, FILE_CURRENT );
#elif defined(__linux__)
    return ftell( m_file );
#else
    return -1;
#endif //OS DEPENDANT CODE
}

fsOffsetNumber_t CRawFile::TellNative( void ) const
{
    // TODO
    return 0;
}

bool CRawFile::IsEOF( void ) const
{
#ifdef _WIN32
    return ( SetFilePointer( m_file, 0, NULL, FILE_CURRENT ) >= GetFileSize( m_file, NULL ) );
#elif defined(__linux__)
    return feof( m_file );
#else
    return false;
#endif //OS DEPENDANT CODE
}

bool CRawFile::Stat( struct stat *pFileStats ) const
{
#ifdef _WIN32
    BY_HANDLE_FILE_INFORMATION info;

    if (!GetFileInformationByHandle( m_file, &info ))
        return false;

    pFileStats->st_size = info.nFileSizeLow;
    pFileStats->st_dev = info.nFileIndexLow;
    pFileStats->st_atime = info.ftLastAccessTime.dwLowDateTime;
    pFileStats->st_ctime = info.ftCreationTime.dwLowDateTime;
    pFileStats->st_mtime = info.ftLastWriteTime.dwLowDateTime;
    pFileStats->st_dev = info.dwVolumeSerialNumber;
    pFileStats->st_mode = 0;
    pFileStats->st_nlink = (unsigned short)info.nNumberOfLinks;
    pFileStats->st_rdev = 0;
    pFileStats->st_gid = 0;
    return true;
#elif __linux
    return fstat( fileno( m_file ), pFileStats ) != 0;
#else
    return false;
#endif //OS DEPENDANT CODE
}

#ifdef _WIN32
inline static void TimetToFileTime( time_t t, LPFILETIME pft )
{
    LONGLONG ll = Int32x32To64(t, 10000000) + 116444736000000000;
    pft->dwLowDateTime = (DWORD) ll;
    pft->dwHighDateTime = ll >>32;
}
#endif //_WIN32

void CRawFile::PushStat( const struct stat *stats )
{
#ifdef _WIN32
    FILETIME ctime;
    FILETIME atime;
    FILETIME mtime;

    TimetToFileTime( stats->st_ctime, &ctime );
    TimetToFileTime( stats->st_atime, &atime );
    TimetToFileTime( stats->st_mtime, &mtime );

    SetFileTime( m_file, &ctime, &atime, &mtime );
#elif defined(__linux__)
    struct utimbuf timeBuf;
    timeBuf.actime = stats->st_atime;
    timeBuf.modtime = stats->st_mtime;

    utime( m_path.c_str(), &timeBuf );
#endif //OS DEPENDANT CODE
}

void CRawFile::SetSeekEnd( void )
{
#ifdef _WIN32
    SetEndOfFile( m_file );
#elif defined(__linux__)
    ftruncate64( fileno( m_file ), ftello64( m_file ) );
#endif //OS DEPENDANT CODE
}

size_t CRawFile::GetSize( void ) const
{
#ifdef _WIN32
    return GetFileSize( m_file, NULL );
#elif defined(__linux__)
    struct stat fileInfo;
    fstat( fileno( m_file ), &fileInfo );

    return fileInfo.st_size;
#else
    return 0;
#endif //OS DEPENDANT CODE
}

fsOffsetNumber_t CRawFile::GetSizeNative( void ) const
{
    // TODO.
    return 0;
}

void CRawFile::Flush( void )
{
#ifdef _WIN32
    FlushFileBuffers( m_file );
#elif defined(__linux__)
    fflush( m_file );
#endif //OS DEPENDANT CODE
}

const filePath& CRawFile::GetPath( void ) const
{
    return m_path;
}

bool CRawFile::IsReadable( void ) const
{
    return ( m_access & FILE_ACCESS_READ ) != 0;
}

bool CRawFile::IsWriteable( void ) const
{
    return ( m_access & FILE_ACCESS_WRITE ) != 0;
}