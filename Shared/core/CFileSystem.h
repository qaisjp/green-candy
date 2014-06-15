/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        Shared/core/CFileSystem.h
*  PURPOSE:     File management
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#ifndef _CFileSystem_
#define _CFileSystem_

#define FILE_FLAG_TEMPORARY     0x00000001
#define FILE_FLAG_UNBUFFERED    0x00000002
#define FILE_FLAG_GRIPLOCK      0x00000004
#define FILE_FLAG_WRITESHARE    0x00000008

// Include extensions (public headers only)
#include "CFileSystem.zip.h"

class CFileSystem : public CFileSystemInterface
{
public:
                            CFileSystem             ( void );
                            ~CFileSystem            ( void );

    void                    InitZIP                 ( void );
    void                    DestroyZIP              ( void );

    CFileTranslator*        CreateTranslator        ( const char *path );
    CArchiveTranslator*     OpenArchive             ( CFile& file );

    CArchiveTranslator*     CreateZIPArchive        ( CFile& file );

    // Function to cast a CFileTranslator into a CArchiveTranslator.
    // If not possible, it returns NULL.
    CArchiveTranslator*     GetArchiveTranslator    ( CFileTranslator *translator );

    // Insecure functions
    bool                    IsDirectory             ( const char *path );
#ifdef _WIN32
    bool                    WriteMiniDump           ( const char *path, _EXCEPTION_POINTERS *except );
#endif //_WIN32
    bool                    Exists                  ( const char *path );
    size_t                  Size                    ( const char *path );
    bool                    ReadToBuffer            ( const char *path, std::vector <char>& output );

    // Settings.
    void                    SetIncludeAllDirectoriesInScan  ( bool enable )             { m_includeAllDirsInScan = enable; }
    bool                    GetIncludeAllDirectoriesInScan  ( void ) const              { return m_includeAllDirsInScan; }

    // Members.
    bool                    m_includeAllDirsInScan;     // decides whether ScanDir implementations should apply patterns on directories
};

// These variables are exported for easy usage by the application.
// It is common sense that an application has a local filesystem and resides on a space.
extern CFileSystem *fileSystem;     // the local filesystem
extern CFileTranslator *fileRoot;   // the space it resides on

#endif //_CFileSystem_
