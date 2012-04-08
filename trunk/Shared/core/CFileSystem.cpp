/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        Shared/core/CFileSystem.cpp
*  PURPOSE:     File management
*  DEVELOPERS:  S2Games <http://savage.s2games.com> (historical entry)
*               The_GTA <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#include "StdInc.h"

std::list <CFile*> *openFiles;

CFileSystem *fileSystem;
CFileTranslator *fileRoot;

#ifdef _FILESYSTEM_ZIP_SUPPORT

#include "unzip.h"

list<CArchiveFileTranslator*>*	registeredArchives;

/*==============================================================
    Archive Filesystem Functions

    These functions shall never be given for a mod to control.
    The core engine has to handle all loading shematics, so the mod can use the given space to initialize itself.
===============================================================*/

void Archive_ScanDirectory(char *pDirectory, char *pWildcard, bool bRecurse, 
                              void (*dirCallback)(const char *pDirectory, void *pUserdata), 
                              void (*fileCallback)(const char *pFilename, void *pUserdata), 
                              void *pUserdata)
{
    list <CArchiveFileTranslator*>::iterator iter = registeredArchives->begin();

    for (iter; iter != registeredArchives->end(); iter++)
    {
        (*iter)->ScanDirectory(pDirectory, pWildcard, bRecurse, dirCallback, fileCallback, pUserdata);
    }
}

bool Archive_StatFile(const char *filename, struct stat *stats)
{
    list <CArchiveFileTranslator*>::iterator iter = registeredArchives->begin();

    for (iter; iter != registeredArchives->end(); iter++)
        if ((*iter)->Stat(filename, stats))
            return true;

    return false;
}

// Gets an archive by a absolute path
CArchiveFileTranslator* Archive_GetArchive(const char *pPath)
{
    list<CArchiveFileTranslator*>::iterator iter = registeredArchives->begin();
    char pathBuffer[1024];

    for (iter; iter != registeredArchives->end(); iter++)
    {
        (*iter)->GetDirectory(pathBuffer, 1024);

        if (strcmp(pathBuffer, pPath) == 0)
            return *iter;
    }
    return NULL;
}

CFile* Archive_OpenFile(const char *filename, const char *mode)
{
    list <CArchiveFileTranslator*>::iterator iter = registeredArchives->begin();
    CFile *pFile;

    for (iter; iter != registeredArchives->end(); iter++)
    {
        if ((pFile = (*iter)->Open(filename, mode)))
        {
            char pathBuffer[1024];

            (*iter)->GetDirectory(pathBuffer, 1024);

            Console_DPrintf("Opening '%s' in archive '%s'\n", filename, pathBuffer);
            return pFile;
        }
    }
    return NULL;
}

bool	Archives_FileExists(const char *filename)
{
    list <CArchiveFileTranslator*>::iterator iter = registeredArchives->begin();

    for (iter; iter != registeredArchives->end(); iter++)
    {
        if ((*iter)->Exists(filename))
            return true;
    }
    return false;
}

CArchiveFileTranslator*	Archives_FindPath(const char *pFilename)
{
    list <CArchiveFileTranslator*>::iterator iter = registeredArchives->begin();

    for (iter; iter != registeredArchives->end(); iter++)
        if ((*iter)->Exists(pFilename))
            return *iter;

    return NULL;
}

void Archive_Init()
{
    registeredArchives = new list<CArchiveFileTranslator*>();
}

void Archive_Shutdown()
{
    while (!registeredArchives->empty())
        (*registeredArchives->begin())->Delete();
}

#endif //_FILESYSTEM_ZIP_SUPPORT

bool File_IsDirectoryAbsolute( const char *pPath )
{
#ifdef _WIN32
    DWORD dwAttributes = GetFileAttributes(pPath);

    if (dwAttributes == INVALID_FILE_ATTRIBUTES)
        return false;

    return (dwAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
#else
    return false;
#endif
}

static inline bool _File_ParseRelativePath( const char *path, dirTree& tree, bool *bFile )
{
    filePath entry;

    entry.reserve( MAX_PATH );

    while (*path)
    {
        switch (*path)
        {
        case '\\':
        case '/':
            if ( entry.empty() )
                break;

            if ( entry == "." )
            {
                // We ignore this current path indicator
                entry.clear();
                break;
            }
            else if ( entry == ".." )
            {
                if ( tree.empty() )
                    return false;

                tree.pop_back();
                entry.clear();
                break;
            }

            tree.push_back( entry );
            entry.clear();
            break;
        case ':':
            return false;
#ifdef _WIN32
        case '<':
        case '>':
        case '"':
        case '|':
        case '?':
        case '*':
            return false;
#endif
        default:
            entry += *path;
            break;
        }

        path++;
    }

    if ( !entry.empty() )
    {
        tree.push_back( entry );

        *bFile = true;
    }
    else
        *bFile = false;

    return true;
}

// Output a path tree
static inline void _File_OutputPathTree( dirTree& tree, bool file, filePath& output )
{
    unsigned int n;

    if ( file )
    {
        for (n=0; n<tree.size()-1; n++)
        {
            output += tree[n];
            output += '/';
        }

        output += tree[n];
        return;
    }

    for (n=0; n<tree.size(); n++)
    {
        output += tree[n];
        output += '/';
    }
}

// Get relative path tree nodes
static inline bool _File_ParseRelativeTree( const char *path, dirTree& root, dirTree& output, bool *file )
{
    dirTree tree;
    dirTree::iterator rootIter, treeIter;

    if (!_File_ParseRelativePath( path, tree, file ))
        return false;

    if ( tree.size() < root.size() )
        return false;

    for ( rootIter = root.begin(), treeIter = tree.begin(); rootIter != root.end(); rootIter++, treeIter++ )
        if ( *rootIter != *treeIter )
            return false;

    output.insert( output.end(), treeIter, tree.end() );
    return true;
}

/*===================================================
    CRawFile

    This class represents a file on the system.
    As long as it is present, the file is opened.

    fixme: Port to unix and mac
===================================================*/

CRawFile::~CRawFile()
{
#ifdef _WIN32
    CloseHandle( m_file );
#endif

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
#endif
}

size_t CRawFile::Write( const void *pBuffer, size_t sElement, unsigned long iNumElements )
{
#ifdef _WIN32
    DWORD dwBytesWritten;

    if (sElement == 0 || iNumElements == 0)
        return 0;

    WriteFile(m_file, pBuffer, sElement * iNumElements, &dwBytesWritten, NULL);
    return dwBytesWritten / sElement;
#endif
}

int CRawFile::Seek( long iOffset, int iType )
{
#ifdef _WIN32
    if (SetFilePointer(m_file, iOffset, NULL, iType) == INVALID_SET_FILE_POINTER)
        return -1;
    return 0;
#endif
}

long CRawFile::Tell()
{
#ifdef _WIN32
    return SetFilePointer( m_file, 0, NULL, FILE_CURRENT );
#endif
}

bool CRawFile::IsEOF()
{
#ifdef _WIN32
    return ( SetFilePointer( m_file, 0, NULL, FILE_CURRENT ) == GetFileSize( m_file, NULL ) );
#endif
}

bool CRawFile::Stat( struct stat *pFileStats )
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
    pFileStats->st_nlink = info.nNumberOfLinks;
    pFileStats->st_rdev = 0;
    pFileStats->st_gid = 0;
    return true;
#endif
}

size_t CRawFile::GetSize()
{
#ifdef _WIN32
    return GetFileSize( m_file, NULL );
#endif
}

void CRawFile::SetSize( size_t size )
{
#ifdef _WIN32
    SetFilePointer( m_file, size, NULL, SEEK_SET );

    SetEndOfFile( m_file );
#endif
}

void CRawFile::Flush()
{
#ifdef _WIN32
    FlushFileBuffers( m_file );
#endif
}

filePath& CRawFile::GetPath()
{
    return m_path;
}

bool CRawFile::IsReadable()
{
#ifdef _WIN32
    return ( m_access & GENERIC_READ ) != 0;
#endif
}

bool CRawFile::IsWriteable()
{
#ifdef _WIN32
    return ( m_access & GENERIC_WRITE ) != 0;
#endif
}

/*=========================================
    CBufferedFile

    Loads a file at open and keeps it in a managed buffer.
=========================================*/

CBufferedFile::~CBufferedFile()
{
}

size_t CBufferedFile::Read( void *pBuffer, size_t sElement, unsigned long iNumElements )
{
    long iReadElements = min( ( m_sSize - m_iSeek ) / sElement, iNumElements );
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

    m_iSeek = max( 0, min( m_iSeek + iOffset, (long)m_sSize ) );
    return 0;
}

long CBufferedFile::Tell()
{
    return m_iSeek;
}

bool CBufferedFile::IsEOF()
{
    return ( m_iSeek == m_sSize );
}

bool CBufferedFile::Stat( struct stat *pFileStats )
{
    pFileStats->st_dev = -1;
    pFileStats->st_ino = -1;
    pFileStats->st_mode = -1;
    pFileStats->st_nlink = -1;
    pFileStats->st_uid = -1;
    pFileStats->st_gid = -1;
    pFileStats->st_rdev = -1;
    pFileStats->st_size = m_sSize;
    pFileStats->st_atime = 0;
    pFileStats->st_ctime = 0;
    pFileStats->st_mtime = 0;
    return 0;
}

size_t CBufferedFile::GetSize()
{
    return m_sSize;
}

void CBufferedFile::Flush()
{
    return;
}

filePath& CBufferedFile::GetPath()
{
    return m_path;
}

int CBufferedFile::ReadInt()
{
    int iResult;

    if ( ( m_sSize - m_iSeek ) < sizeof(int) )
        return 0;

    iResult = *(int*)( m_pBuffer + m_iSeek );
    m_iSeek += sizeof(int);
    return iResult;
}

short CBufferedFile::ReadShort()
{
    short iResult;

    if ( (m_sSize - m_iSeek) < sizeof(short) )
        return 0;

    iResult = *(short*)(m_pBuffer + m_iSeek);
    m_iSeek += sizeof(short);
    return iResult;
}

char CBufferedFile::ReadByte()
{
    if ( m_sSize == m_iSeek )
        return 0;

    return *(m_pBuffer + m_iSeek++);
}

bool CBufferedFile::IsReadable()
{
    return true;
}

bool CBufferedFile::IsWriteable()
{
    return false;
}

/*=======================================
    CSystemPathTranslator

    Filesystem path translation utility
=======================================*/

void CSystemPathTranslator::GetDirectory( filePath& output )
{
    output = m_currentDir;
}

bool CSystemPathTranslator::ChangeDirectory( const char *path )
{
    filePath dir;
    GetRelativePath( path, false, dir );

    if ( !GetRelativePath( path, false, dir ) || !File_IsDirectoryAbsolute( m_root + dir ) )
        return false;

    m_currentDir = dir;
    return true;
}

bool CSystemPathTranslator::GetFullPathTree( const char *path, dirTree& tree, bool *file )
{
    tree = m_rootTree;
    return GetRelativePathTree( path, tree, file );
}

bool CSystemPathTranslator::GetRelativePathTree( const char *path, dirTree& tree, bool *file )
{
    filePath target;

    if (!*path)
        return false;

    if ( path[0] == '/' )
        return _File_ParseRelativePath( path + 1, tree, file );
    else if ( path[1] == ':' )
    {
        if ( !path[2] )
            return false;

        if ( path[0] != m_root[0] )
            return false;   // drive mismatch
        
        return _File_ParseRelativeTree( path + 3, m_rootTree, tree, file );
    }

    // This could be optimized
    target += m_currentDir;
    target += path;
    return _File_ParseRelativePath( target.c_str(), tree, file );
}

bool CSystemPathTranslator::GetFullPath( const char *path, bool allowFile, filePath& output )
{
    output += m_root;
    return GetRelativePath( path, allowFile, output );
}

bool CSystemPathTranslator::GetRelativePath( const char *path, bool allowFile, filePath& output )
{
    dirTree tree;
    bool file;

    if ( !GetRelativePathTree( path, tree, &file ) )
        return false;

    if ( file && !allowFile )
    {
        tree.pop_back();

        file = false;
    }

    _File_OutputPathTree( tree, file, output );
    return true;
}

#ifdef _FILESYSTEM_ZIP_SUPPORT

/*=======================================
    CArchiveFileTranslator

    Translator to manage files in archives.
=======================================*/

CArchiveFileTranslator::~CArchiveFileTranslator()
{
    ZIP_Close(m_pArchive);

    Tag_Free(m_pPath);
    Tag_Free(m_pRoot);
}

static inline CArchiveFileTranslator* Archive_CreateTranslator(const char *pPath, enum eAccessLevel eAccess, int iFlags)
{
    CArchiveFileTranslator *pTranslator;
    char pathBuffer[1024];
    char fullBuffer[1024];
    unsigned char hash[MAX_HASH_SIZE];
    int iHashSize;
    unsigned int n;
    char dirTree[32][MAX_PATH];
    unsigned int uiTreeDepth;
    bool bFile;
    unzFile pArchive;

    if ((pTranslator = Archive_GetArchive(pPath)))
        return pTranslator;

    // We have to handle absolute path, too
    if (strlen(pPath) > 2 && pPath[1] == ':')
    {
        if (!_File_ParseRelativePath(pPath + 3, dirTree, 32, &uiTreeDepth, &bFile))
            return NULL;

        pathBuffer[0] = pPath[0];
        pathBuffer[1] = ':';
        pathBuffer[2] = '/';
        pathBuffer[3] = 0;
    }
    else
    {
        strncpy(pathBuffer, System_GetRootDir(), 1023);
        strncat(pathBuffer, pPath, 1023);

        if (!_File_ParseRelativePath(pathBuffer + 3, dirTree, 32, &uiTreeDepth, &bFile))
            return NULL;

        pathBuffer[3] = 0;
    }

    if (!bFile)
        return NULL;

    if (uiTreeDepth < 2)
        return NULL;

    uiTreeDepth--;

    for (n=0; n<uiTreeDepth; n++)
        strncat(pathBuffer, fmt("%s/", dirTree[n]), 1024);

    strcpy(fullBuffer, pathBuffer);
    strncat(fullBuffer, dirTree[n], 1023);

    pArchive = ZIP_Open(fullBuffer, hash, &iHashSize);

    if (!pArchive)
    {
        Console_Printf("Invalid archive '%s' - loading failed\n", fullBuffer);
        return false;
    }
    pTranslator = new CArchiveFileTranslator();

    pTranslator->m_pPath = Tag_Strdup(fullBuffer, MEM_FILE);
    pTranslator->m_pRoot = Tag_Strdup(pathBuffer, MEM_FILE);
    pTranslator->m_pArchive = pArchive;
    pTranslator->m_eAccessPriviledge = eAccess;
    pTranslator->m_iFlags = iFlags;

    memcpy(pTranslator->m_hash, hash, MAX_HASH_SIZE);
    pTranslator->m_iHashLength = iHashSize;

    memcpy(pTranslator->m_rootTree, dirTree, sizeof(pTranslator->m_rootTree));
    pTranslator->m_uiTreeDepth = uiTreeDepth;

    registeredArchives->push_front(pTranslator);

    return pTranslator;
}

CFile* CArchiveFileTranslator::Open( const char *path, const char *mode )
{
    std::string output;

    if ( !GetRelativePath( path, true, output ) )
        return NULL;

    return ZIP_OpenFileBuffered( m_pArchive, output.c_str(), mode );
}

bool CArchiveFileTranslator::Exists( const char *path )
{
    std::string output;

    if ( !GetRelativePath( path, true, output ) )
        return false;

    return ZIP_FileExists( m_pArchive, output.c_str() );
}

size_t CArchiveFileTranslator::Size( const char *path )
{
    struct stat fileStats;
    std::string output;

    if ( !GetRelativePath( path, true, output ) )
        return 0;

    if (!ZIP_StatFile( m_pArchive, output.c_str(), &fileStats ))
        return 0;

    return fileStats.st_size;
}

void CArchiveFileTranslator::ScanDirectory(char *directory, char *wildcard, bool recurse, 
                                              void (*dirCallback)(const char *directory, void *userdata), 
                                              void (*fileCallback)(const char *filename, void *userdata), 
                                              void *userdata)
{
    std::string output;

    if ( !GetRelativePath( directory, false, output ) )
        return;

    ZIP_ScanDirectory( m_pArchive, m_root, output.c_str(), wildcard, recurse, dirCallback, fileCallback, userdata );
}

bool CArchiveFileTranslator::Stat( const char *path, struct stat *stats )
{
    std::string output;

    if ( !GetRelativePath( path, true, output ) )
        return false;

    return ZIP_StatFile(m_pArchive, pathBuffer, pStats);
}

#endif //_FILESYSTEM_ZIP_SUPPORT

/*=======================================
    CSystemFileTranslator

    Default file translator
=======================================*/

bool CSystemFileTranslator::WriteData( const char *path, const char *buffer, size_t size )
{
#ifdef _WIN32
    HANDLE file;
    filePath output = m_root;
    dirTree tree;
    bool isFile;

    if ( !GetRelativePathTree( path, tree, &isFile ) || !isFile )
        return false;

    _File_OutputPathTree( tree, true, output );

    // Make sure directory exists
    tree.pop_back();
    _CreateDirTree( tree );

    if ( !(file = CreateFile( output.c_str(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, 0, NULL )) )
        return false;

    WriteFile( file, buffer, size, NULL, NULL );

    CloseHandle( file );
    return true;
#endif
}

void CSystemFileTranslator::_CreateDirTree( dirTree& tree )
{
    dirTree::iterator iter;
    filePath path;

    for ( iter = tree.begin(); iter != tree.end(); iter++ )
    {
        path += *iter;
        path += '/';

#ifdef _WIN32
        CreateDirectory( path.c_str(), NULL );
#else
        mkdir( path.c_str() );
#endif
    }
}

bool CSystemFileTranslator::CreateDir( const char *path )
{
    dirTree tree;
    bool file;

    if ( !GetRelativePathTree( path, tree, &file ) )
        return false;

    if ( file )
        tree.pop_back();

    _CreateDirTree( tree );
    return true;
}

CFile* CSystemFileTranslator::Open( const char *path, const char *mode )
{
    dirTree tree;
    filePath output = m_root;
    CRawFile *pFile;
    DWORD dwAccess = 0;
    DWORD dwCreate = 0;
    HANDLE sysHandle;
    bool file;

    if ( !GetRelativePathTree( path, tree, &file ) )
        return NULL;

    // We can only open files!
    if ( !file )
        return NULL;

    _File_OutputPathTree( tree, true, output );

#ifdef _WIN32
    while (*mode)
    {
        switch (*mode)
        {
        case 'w':
            dwCreate = CREATE_ALWAYS;
            dwAccess |= GENERIC_WRITE;
            break;
        case 'r':
            dwCreate = OPEN_EXISTING;
            dwAccess |= GENERIC_READ;
            break;
        case 'a':
            dwCreate = Exists( path ) ? OPEN_EXISTING : CREATE_ALWAYS;
            dwAccess = GENERIC_WRITE;
            break;
#ifdef _EXTRA_FILE_ACCESS_MODES
        case 't':
            break;
#endif
        case '+':
            dwAccess |= GENERIC_WRITE | GENERIC_READ;
            break;
        }

        mode++;
    }
    if ( !dwAccess )
        return NULL;

    // Creation requires the dir tree!
    if ( dwCreate == CREATE_ALWAYS )
    {
        tree.pop_back();

        _CreateDirTree( tree );
    }

    sysHandle = CreateFile( output.c_str(), dwAccess, FILE_SHARE_READ, NULL, dwCreate, 0, NULL );

    if ( sysHandle == INVALID_HANDLE_VALUE )
        return NULL;

    pFile = new CRawFile();
    pFile->m_file = sysHandle;
    pFile->m_access = dwAccess;
    pFile->m_path = output;

    if ( *mode == 'a' )
        pFile->Seek( 0, SEEK_END );

    openFiles->push_back( pFile );
    return pFile;
#endif
}

bool CSystemFileTranslator::Exists( const char *path )
{
    filePath output;
    struct stat tmp;

    if ( !GetFullPath( path, true, output ) )
        return false;

    return stat( output.c_str(), &tmp ) == 0;
}

bool CSystemFileTranslator::Delete( const char *path )
{
    filePath output;

    if ( !GetFullPath( path, true, output ) )
        return false;

#ifdef _WIN32
    return DeleteFile( output.c_str() ) != FALSE;
#endif
}

bool CSystemFileTranslator::Copy( const char *src, const char *dst )
{
    filePath source;
    filePath target;
    dirTree dstTree;
    bool file;

    if ( !GetFullPath( src, true, source ) || !GetRelativePathTree( dst, dstTree, &file ) || !file )
        return false;

    _File_OutputPathTree( dstTree, true, target );

    // Make sure dir exists
    dstTree.pop_back();
    _CreateDirTree( dstTree );

#ifdef _WIN32
    return CopyFile( source.c_str(), target.c_str(), false ) != FALSE;
#endif
}

bool CSystemFileTranslator::Rename( const char *src, const char *dst )
{
    filePath source;
    filePath target;
    dirTree dstTree;
    bool file;

    if ( !GetFullPath( src, true, source ) || !GetRelativePathTree( dst, dstTree, &file ) || !file )
        return false;

    _File_OutputPathTree( dstTree, true, target );

    // Make sure dir exists
    dstTree.pop_back();
    _CreateDirTree( dstTree );

#ifdef _WIN32
    return MoveFile( source.c_str(), target.c_str() ) != FALSE;
#endif
}

bool CSystemFileTranslator::Stat( const char *path, struct stat *stats )
{
    filePath output;
    
    if ( !GetFullPath( path, true, output ) )
        return false;

    return stat( output.c_str(), stats ) == 0;
}

size_t CSystemFileTranslator::Size( const char *path )
{
    struct stat fstats;

    if ( !Stat( path, &fstats ) )
        return 0;

    return fstats.st_size;
}

bool CSystemFileTranslator::ReadToBuffer( const char *path, std::vector <char>& output )
{
    filePath sysPath;

    if ( !GetFullPath( path, true, sysPath ) )
        return false;

    return fileSystem->ReadToBuffer( sysPath.c_str(), output );
}

void CSystemFileTranslator::ScanDirectory( const char *directory, const char *wildcard, bool recurse, 
                                            pathCallback_t dirCallback, 
                                            pathCallback_t fileCallback, 
                                            void *userdata )
{
#ifdef _WIN32
    WIN32_FIND_DATA		finddata;
    HANDLE				handle;
    filePath            output;
    std::string         query;
    char				wcard[256];

    if ( !GetFullPath( directory, false, output ) )
        return;

    if ( !wildcard )
        strcpy(wcard, "*");
    else
        strncpy(wcard, wildcard, 255);

    //first search for files only
    if ( fileCallback )
    {
        query = output;
        query += wcard;

        handle = FindFirstFile( query.c_str(), &finddata );

        if ( handle != INVALID_HANDLE_VALUE )
        {
            do
            {
                if ( finddata.dwFileAttributes & (FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM | FILE_ATTRIBUTE_TEMPORARY | FILE_ATTRIBUTE_DIRECTORY) )
                    continue;

                filePath filename = output;
                filename += finddata.cFileName;

                fileCallback( filename.c_str(), userdata );		

            } while ( FindNextFile(handle, &finddata) );

            FindClose( handle );
        }
    }

    if ( !dirCallback && !recurse )
        return;

    //next search for subdirectories only
    query = output;
    query += '*';

    handle = FindFirstFile( query.c_str(), &finddata );

    if ( handle == INVALID_HANDLE_VALUE )
        return;

    do
    {
        if ( finddata.dwFileAttributes & (FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM | FILE_ATTRIBUTE_TEMPORARY) )
            continue;

        if ( !(finddata.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) )
            continue;
		
        // Optimization :)
        if ( *(unsigned short*)finddata.cFileName == 0x2E00 || (*(unsigned short*)finddata.cFileName == 0x2E2E && *(unsigned char*)(finddata.cFileName + 2) == 0x00) )
            continue;

        filePath target = output;
        target += finddata.cFileName;
        target += '/';

        if ( dirCallback )
            dirCallback( target.c_str(), userdata );

        if ( recurse )
            ScanDirectory( target.c_str(), wcard, true, dirCallback, fileCallback, userdata );

    } while ( FindNextFile(handle, &finddata) );

    FindClose( handle );
#endif
}

static void _scanFindCallback( const filePath& path, std::vector <filePath> *output )
{
    output->push_back( path );
}

void CSystemFileTranslator::GetDirectories( const char *path, const char *wildcard, bool recurse, std::vector <filePath>& output )
{
    ScanDirectory( path, wildcard, recurse, (pathCallback_t)_scanFindCallback, NULL, &output );
}

void CSystemFileTranslator::GetFiles( const char *path, const char *wildcard, bool recurse, std::vector <filePath>& output )
{
    ScanDirectory( path, wildcard, recurse, NULL, (pathCallback_t)_scanFindCallback, &output );
}

/*=======================================
    CFileSystem

    Management class
=======================================*/

CFileSystem::CFileSystem()
{
    char cwd[1024];
    getcwd( cwd, 1023 );

    // Make sure it is a correct directory
    filePath cwd_ex = cwd;
    cwd_ex += '\\';

    openFiles = new std::list<CFile*>;

    // Every application should be able to access itself
    fileRoot = CreateTranslator( cwd_ex );

    fileSystem = this;
}

CFileSystem::~CFileSystem()
{
    delete openFiles;
}

CFileTranslator* CFileSystem::CreateTranslator( const char *path )
{
    CSystemFileTranslator *pTranslator;
    filePath root;
    dirTree tree;
    bool bFile;

    if ( !*path )
        return NULL;

    // We have to handle absolute path, too
    if ( path[1] == ':' && path[2] )
    {
        if (!_File_ParseRelativePath( path + 3, tree, &bFile ))
            return NULL;

        root += path[0];
        root += ":/";
    }
    else
    {
        char pathBuffer[1024];
        GetCurrentDirectory( 1024, pathBuffer );

        root = pathBuffer;
        root += path;

        if (!_File_ParseRelativePath( root.c_str() + 3, tree, &bFile ))
            return NULL;

        root.clear();
    }

    if ( bFile )
        tree.pop_back();

    _File_OutputPathTree( tree, false, root );

    if ( !IsDirectory( root.c_str() ) )
        return NULL;

    pTranslator = new CSystemFileTranslator();
    pTranslator->m_root = root;
    pTranslator->m_rootTree = tree;
    return pTranslator;
}

bool CFileSystem::IsDirectory( const char *path )
{
    return File_IsDirectoryAbsolute( path );
}

bool CFileSystem::WriteMiniDump( const char *path, _EXCEPTION_POINTERS *except )
{
#ifdef _WIN32
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

bool CFileSystem::ReadToBuffer( const char *path, std::vector <char>& output )
{
#ifdef _WIN32
    HANDLE file = CreateFile( path, GENERIC_READ, 0, NULL, OPEN_ALWAYS, 0, NULL );
    size_t size;
    DWORD _pf;

    if ( file == INVALID_HANDLE_VALUE )
        return false;

    size = GetFileSize( file, NULL );

    if ( size != 0 )
    {
        output.resize( size );
        output.reserve( size );

        ReadFile( file, &output[0], size, &_pf, NULL );
    }

    CloseHandle( file );
    return true;
#endif
}