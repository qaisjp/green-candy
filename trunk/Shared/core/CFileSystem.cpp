/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        Shared/core/CFileSystem.cpp
*  PURPOSE:     File management
*  DEVELOPERS:  S2 Games <http://savage.s2games.com> (historical entry)
*               Martin Turski <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#include <StdInc.h>
#include <sys/stat.h>

// Include internal header.
#include "fsinternal/CFileSystem.internal.h"

// This variable is exported across the whole FileSystem library.
// It should be used by CRawFile classes that are created.
std::list <CFile*> *openFiles;

CFileSystem *fileSystem = NULL;
CFileTranslator *fileRoot = NULL;

#include "CFileSystem.Utils.hxx"

// Create the class at runtime initialization.
CSystemCapabilities systemCapabilities;

/*=======================================
    CFileSystem

    Management class with root-access functions.
    These methods are root-access. Exposing them
    to a security-critical user-space context is
    not viable.
=======================================*/
static bool _hasBeenInitialized = false;

#ifdef _WIN32

struct MySecurityAttributes
{
    DWORD count;
    LUID_AND_ATTRIBUTES attr[2];
};

#endif //_WIN32

CFileSystem::CFileSystem( void )
{
    // Make sure that there is no second CFileSystem class alive.
    assert( _hasBeenInitialized == false );

    // Set up members.
    m_includeAllDirsInScan = false;
#ifdef _WIN32
    m_hasDirectoryAccessPriviledge = false;
#endif //_WIN32

    char cwd[1024];
    getcwd( cwd, 1023 );

    // Make sure it is a correct directory
    filePath cwd_ex = cwd;
    cwd_ex += '\\';

    openFiles = new std::list<CFile*>;

    // Every application should be able to access itself
    fileRoot = CreateTranslator( cwd_ex );

    // Init Addons
    m_zipExtension.Init();

    fileSystem = this;

    // We have initialized outselves.
    _hasBeenInitialized = true;
}

CFileSystem::~CFileSystem( void )
{
    // Shutdown addons.
    m_zipExtension.Shutdown();

    delete openFiles;

    // We have successfully destroyed FileSystem activity.
    _hasBeenInitialized = false;
}

bool CFileSystem::CanLockDirectories( void )
{
    // We should set special priviledges for the application if
    // running under Win32.
#ifdef _WIN32
    // We assume getting the priviledge once is enough.
    if ( !m_hasDirectoryAccessPriviledge )
    {
        HANDLE token;

        // We need SE_BACKUP_NAME to gain directory access on Windows
        if ( OpenProcessToken( GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &token ) == TRUE )
        {
            MySecurityAttributes priv;

            priv.count = 2; // we want to request two priviledges.

            BOOL backupRequest = LookupPrivilegeValue( NULL, SE_BACKUP_NAME, &priv.attr[0].Luid );

            priv.attr[0].Attributes = SE_PRIVILEGE_ENABLED;

            BOOL restoreRequest = LookupPrivilegeValue( NULL, SE_RESTORE_NAME, &priv.attr[1].Luid );

            priv.attr[1].Attributes = SE_PRIVILEGE_ENABLED;

            if ( backupRequest == TRUE && restoreRequest == TRUE )
            {
                BOOL success = AdjustTokenPrivileges( token, false, (TOKEN_PRIVILEGES*)&priv, sizeof( priv ), NULL, NULL );

                if ( success == TRUE )
                {
                    m_hasDirectoryAccessPriviledge = true;
                }
            }

            CloseHandle( token );
        }
    }
    return m_hasDirectoryAccessPriviledge;
#elif defined(__linux__)
    // We assume that we can always lock directories under Unix.
    return true;
#else
    // No idea abott directory locking on other environments.
    return false;
#endif //OS DEPENDENT CODE
}

CFileTranslator* CFileSystem::CreateTranslator( const char *path )
{
    // Without access to directory locking, this function can not execute.
    if ( !CanLockDirectories() )
        return NULL;

    CSystemFileTranslator *pTranslator;
    filePath root;
    dirTree tree;
    bool bFile;

#ifdef _WIN32
    // We have to handle absolute path, too
    if ( _File_IsAbsolutePath( path ) )
    {
        if (!_File_ParseRelativePath( path + 3, tree, bFile ))
            return NULL;

        root += path[0];
        root += ":/";
    }
#elif defined(__linux__)
    if ( *path == '/' || *path == '\\' )
    {
        if (!_File_ParseRelativePath( path + 1, tree, bFile ))
            return NULL;

        root = "/";
    }
#endif //OS DEPENDANT CODE
    else
    {
        char pathBuffer[1024];
        getcwd( pathBuffer, sizeof(pathBuffer) );

        root = pathBuffer;
        root += "\\";
        root += path;

#ifdef _WIN32
        if (!_File_ParseRelativePath( root.c_str() + 3, tree, bFile ))
            return NULL;

        root.resize( 2 );
        root += "/";
#elif defined(__linux__)
        if (!_File_ParseRelativePath( root.c_str() + 1, tree, bFile ))
            return NULL;

        root = "/";
#endif //OS DEPENDANT CODE
    }

    if ( bFile )
        tree.pop_back();

    _File_OutputPathTree( tree, false, root );

#ifdef _WIN32
    HANDLE dir = CreateFile( root.c_str(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, NULL );

    if ( dir == INVALID_HANDLE_VALUE )
        return NULL;
#elif defined(__linux__)
    DIR *dir = opendir( root.c_str() );

    if ( dir == NULL )
        return NULL;
#else
    if ( !IsDirectory( root.c_str() ) )
        return NULL;
#endif //OS DEPENDANT CODE

    pTranslator = new CSystemFileTranslator();
    pTranslator->m_root = root;
    pTranslator->m_rootTree = tree;

#ifdef _WIN32
    pTranslator->m_rootHandle = dir;
    pTranslator->m_curDirHandle = NULL;
#elif defined(__linux__)
    pTranslator->m_rootHandle = dir;
    pTranslator->m_curDirHandle = NULL;
#endif //OS DEPENDANT CODE

    return pTranslator;
}

bool CFileSystem::IsDirectory( const char *path )
{
    return File_IsDirectoryAbsolute( path );
}

#ifdef _WIN32

// By definition, crash dumps are OS dependant.
// Currently we only support crash dumps on Windows OS.

bool CFileSystem::WriteMiniDump( const char *path, _EXCEPTION_POINTERS *except )
{
#if 0
    CRawFile *file = (CRawFile*)fileRoot->Open( path, "wb" );
    MINIDUMP_EXCEPTION_INFORMATION info;

    if ( !file )
        return false;

    // Create an exception information struct
    info.ThreadId = GetCurrentThreadId();
    info.ExceptionPointers = except;
    info.ClientPointers = false;

    // Write the dump
    MiniDumpWriteDump( GetCurrentProcess(), GetCurrentProcessId(), file->m_file, MiniDumpNormal, &info, NULL, NULL );

    delete file;
#endif

    return true;
}
#endif //_WIN32

bool CFileSystem::Exists( const char *path )
{
    struct stat tmp;

    return stat( path, &tmp ) == 0;
}

size_t CFileSystem::Size( const char *path )
{
    struct stat stats;

    if ( stat( path, &stats ) != 0 )
        return 0;

    return stats.st_size;
}

// Utility to quickly load data from files on the local filesystem.
// Do not export it into user-space since this function has no security restrictions.
bool CFileSystem::ReadToBuffer( const char *path, std::vector <char>& output )
{
#ifdef _WIN32
    HANDLE file = CreateFile( path, GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL );

    if ( file == INVALID_HANDLE_VALUE )
        return false;

    size_t size = GetFileSize( file, NULL );

    if ( size != 0 )
    {
        output.resize( size );
        output.reserve( size );

        DWORD _pf;

        ReadFile( file, &output[0], size, &_pf, NULL );
    }
    else
        output.clear();

    CloseHandle( file );
    return true;
#elif defined(__linux__)
    int iReadFile = open( path, O_RDONLY, 0 );

    if ( iReadFile == -1 )
        return false;

    struct stat read_info;

    if ( fstat( iReadFile, &read_info ) != 0 )
        return false;

    if ( read_info.st_size != 0 )
    {
        output.resize( read_info.st_size );
        output.reserve( read_info.st_size );

        ssize_t numRead = read( iReadFile, &output[0], read_info.st_size );

        if ( numRead == 0 )
        {
            close( iReadFile );
            return false;
        }

        if ( numRead != read_info.st_size )
            output.resize( numRead );
    }
    else
        output.clear();

    close( iReadFile );
    return true;
#else
    return false;
#endif //OS DEPENDANT CODE
}
