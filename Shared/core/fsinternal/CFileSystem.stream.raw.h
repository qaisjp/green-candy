/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        Shared/core/fsinternal/CFileSystem.stream.raw.h
*  PURPOSE:     Raw OS filesystem file link
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#ifndef _FILESYSTEM_RAW_OS_LINK_
#define _FILESYSTEM_RAW_OS_LINK_

class CRawFile : public CFile
{
public:
                        CRawFile        ( const filePath& absFilePath );
                        ~CRawFile       ( void );

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
    friend class CSystemFileTranslator;
    friend class CFileSystem;

#ifdef _WIN32
    HANDLE          m_file;
#elif defined(__linux__)
    FILE*           m_file;
#endif //OS DEPENDANT CODE
    DWORD           m_access;
    filePath        m_path;
};

#endif //_FILESYSTEM_RAW_OS_LINK_