/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        core/CFileSystem.cpp
*  PURPOSE:     File management
*  DEVELOPERS:  S2Games <http://savage.s2games.com> (historical entry)
*               The_GTA <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#include "StdInc.h"

extern CCore *g_pCore;

CFileSystem *fileSystem;
CFileTranslator *mtaFileRoot;

std::list <CFile*> *openFiles;

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

/*=====================================================================
*
* File_OpenAbsolute
*
* Opens a system file.
* The direct access to this function is not allowed due to security reasons.
* If you want to pierce the directory, create a new translator with a specified root!
*
=======================================================================*/

static inline CFile* File_OpenAbsolute(const char *filename, const char *mode)
{
    void *pSystemFile;
    CRawFile *pFile;
    DWORD dwAccess = 0;
    DWORD dwCreate = 0;
    const char *pModeIter = mode;

    while (*pModeIter)
    {
        switch (*pModeIter)
        {
        case 'w':
            dwCreate = CREATE_ALWAYS;
            dwAccess |= GENERIC_WRITE;
            break;
        case 'r':
            dwCreate = OPEN_EXISTING;
            dwAccess |= GENERIC_READ;
            break;
#ifdef _EXTRA_FILE_ACCESS_MODES
        case 'a':
            break;
        case 't':
            break;
#endif
        case '+':
            if (dwAccess & GENERIC_READ)
                dwAccess |= GENERIC_WRITE;
            else if (dwAccess & GENERIC_WRITE)
                dwAccess |= GENERIC_READ;
            break;
        }

        pModeIter++;
    }
    if (!dwAccess)
        return NULL;

    pSystemFile = CreateFile(filename, dwAccess, FILE_SHARE_READ, NULL, dwCreate, 0, NULL);

    if (pSystemFile == INVALID_HANDLE_VALUE)
        return NULL;

    pFile = new CRawFile();
    pFile->m_file = pSystemFile;
    pFile->m_mode = mode;
    pFile->m_path = filename;

    openFiles->push_back(pFile);
    return pFile;
}

bool File_IsDirectoryAbsolute(const char *pPath)
{
#ifdef WIN32
    DWORD dwAttributes = GetFileAttributes(pPath);

    if (dwAttributes == INVALID_FILE_ATTRIBUTES)
        return false;
    return (dwAttributes & FILE_ATTRIBUTE_DIRECTORY);
#else
    return false;
#endif
}

static inline bool _File_ParseRelativePath( const char *path, std::vector < std::string >& tree, bool *bFile )
{
    std::string entry;
    size_t n = 0;

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

// Output a parsed buffer
static inline bool _File_ParsePath( const char *path, bool allowFile, std::string& output )
{
    std::vector <std::string> tree( 16 );
    unsigned int n;
    bool bFile;

    if (!_File_ParseRelativePath( path, tree, &bFile ))
        return false;

    if ( bFile )
    {
        for (n=0; n<tree.size()-1; n++)
        {
            output += tree[n];
            output += "/";
        }

        if ( allowFile )
            output += tree[n];

        return true;
    }

    for (n=0; n<tree.size(); n++)
    {
        output += tree[n];
        output += "/";
    }

    return true;
}

// Try to create a relative version of a path
static inline bool _File_ParsePathToRelative( const char *path, bool allowFile, const std::vector <std::string>& root, std::string& output )
{
    std::vector <std::string> tree( 16 );
    bool bFile;
    unsigned int n;

    if (!_File_ParseRelativePath( path + 3, tree, &bFile ))
        return false;

    if ( tree.size() < root.size() )
        return false;

    for (n=0; n<root.size(); n++)
        if ( tree[n] != root[n] )
            return false;

    if ( bFile )
    {
        for (n; n<tree.size()-1; n++)
        {
            output += tree[n];
            output += "/";
        }

        if ( allowFile )
            output += tree[n];

        return true;
    }

    for (n; n<tree.size(); n++)
    {
        output += tree[n];
        output += "/";
    }

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
    CloseHandle(m_file);
#endif

    openFiles->remove(this);
}

size_t CRawFile::Read(void *pBuffer, size_t sElement, unsigned long iNumElements)
{
#ifdef _WIN32
    DWORD dwBytesRead;

    if (sElement == 0 || iNumElements == 0)
        return 0;

    ReadFile(m_file, pBuffer, sElement * iNumElements, &dwBytesRead, NULL);
    return dwBytesRead / sElement;
#endif
}

size_t CRawFile::Write(void *pBuffer, size_t sElement, unsigned long iNumElements)
{
#ifdef _WIN32
    DWORD dwBytesWritten;

    if (sElement == 0 || iNumElements == 0)
        return 0;

    WriteFile(m_file, pBuffer, sElement * iNumElements, &dwBytesWritten, NULL);
    return dwBytesWritten / sElement;
#endif
}

int CRawFile::Seek(long iOffset, int iType)
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
    return SetFilePointer(m_file, 0, NULL, FILE_CURRENT);
#endif
}

bool CRawFile::IsEOF()
{
#ifdef _WIN32
    return (SetFilePointer(m_file, 0, NULL, FILE_CURRENT) == GetFileSize(m_file, NULL));
#endif
}

bool CRawFile::Stat(struct stat *pFileStats)
{
#ifdef _WIN32
    BY_HANDLE_FILE_INFORMATION info;

    if (!GetFileInformationByHandle(m_file, &info))
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
    return GetFileSize(m_file, NULL);
#endif
}

void CRawFile::Flush()
{
#ifdef _WIN32
    FlushFileBuffers(m_file);
#endif
}

std::string& CRawFile::GetPath()
{
    return m_path;
}

int CRawFile::ReadInt()
{
    int iBuffer = 0;

    Read((char*)&iBuffer, sizeof(int), 1);
    return iBuffer;
}

short CRawFile::ReadShort()
{
    short sBuffer = 0;

    Read((char*)&sBuffer, sizeof(short), 1);
    return sBuffer;
}

char CRawFile::ReadByte()
{
    char cBuffer = 0;

    Read((char*)&cBuffer, sizeof(char), 1);
    return cBuffer;
}

size_t CRawFile::WriteInt(int iInt)
{
    return Write((char*)&iInt, sizeof(int), 1);
}

size_t CRawFile::WriteShort(short iShort)
{
    return Write((char*)&iShort, sizeof(short), 1);
}

size_t CRawFile::WriteByte(char cByte)
{
    return Write((char*)&cByte, sizeof(char), 1);
}

size_t CRawFile::Printf(const char *pFormat, ...)
{
    va_list args;
    char cBuffer[1024];

    va_start(args, pFormat);
    _vsnprintf(cBuffer, 1023, pFormat, args);
    va_end(args);

    return Write(cBuffer, 1, strlen(cBuffer));
}

/*=========================================
    CBufferedFile

    Loads a file at open and keeps it in a managed buffer.
=========================================*/

size_t CBufferedFile::Read(void *pBuffer, size_t sElement, unsigned long iNumElements)
{
    long iReadElements = min((m_sSize - m_iSeek) / sElement, iNumElements);
    size_t sRead = iReadElements * sElement;

    if (iNumElements <= 0)
        return 0;

    memcpy(pBuffer, m_pBuffer + m_iSeek, sRead);
    m_iSeek += sRead;
    return iReadElements;
}

size_t CBufferedFile::Write(void *pBuffer, size_t sElement, unsigned long iNumElements)
{
    return 0;
}

int CBufferedFile::Seek(long iOffset, int iType)
{
    switch (iType)
    {
    case SEEK_SET:
        m_iSeek = 0;
        break;
    case SEEK_END:
        m_iSeek = m_sSize;
        break;
    }

    m_iSeek = max(0, min(m_iSeek + iOffset, (long)m_sSize));
    return 0;
}

long CBufferedFile::Tell()
{
    return m_iSeek;
}

bool CBufferedFile::IsEOF()
{
    return (m_iSeek == m_sSize);
}

bool CBufferedFile::Stat(struct stat *pFileStats)
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

std::string& CBufferedFile::GetPath()
{
    return m_path;
}

int CBufferedFile::ReadInt()
{
    int iResult;

    if ((m_sSize - m_iSeek) < sizeof(int))
        return 0;

    iResult = *(int*)(m_pBuffer + m_iSeek);
    m_iSeek += sizeof(int);
    return iResult;
}

short CBufferedFile::ReadShort()
{
    short iResult;

    if ((m_sSize - m_iSeek) < sizeof(short))
        return 0;

    iResult = *(short*)(m_pBuffer + m_iSeek);
    m_iSeek += sizeof(short);
    return iResult;
}

char CBufferedFile::ReadByte()
{
    if (m_sSize == m_iSeek)
        return 0;
    return *(m_pBuffer + m_iSeek++);
}

/*=======================================
    CSystemPathTranslator

    Filesystem path translation utility
=======================================*/

void CSystemPathTranslator::GetDirectory( std::string& output )
{
    output = m_currentDir;
}

bool CSystemPathTranslator::ChangeDirectory( const char *path )
{
    return GetRelativePath( path, false, m_currentDir );
}

bool CSystemPathTranslator::GetFullPath( const char *path, bool allowFile, std::string& output )
{
    output += m_root;
    return GetRelativePath( path, allowFile, output );
}

bool CSystemPathTranslator::GetRelativePath( const char *path, bool allowFile, std::string& output )
{
    std::string target;

    if (!*path)
        return false;

    if ( path[0] == '/' )
        return _File_ParsePath( path + 1, allowFile, output );
    else if ( path[1] == ':' )
    {
        if ( path[0] != m_root[0] )
            return false;   // drive mismatch
        
        return _File_ParsePathToRelative( path, allowFile, m_rootTree, output );
    }

    target += m_currentDir;
    target += path;
    return _File_ParsePath( target.c_str(), allowFile, output );
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

int CArchiveFileTranslator::GetFlags()
{
    return m_iFlags;
}

#endif //_FILESYSTEM_ZIP_SUPPORT

/*=======================================
    CSystemFileTranslator

    Default file translator
=======================================*/

CSystemFileTranslator::~CSystemFileTranslator()
{
    
}

CFile* CSystemFileTranslator::Open( const char *path, const char *mode )
{
    std::string output;

    if ( !GetFullPath( path, true, output ) )
        return NULL;

    return File_OpenAbsolute( output.c_str(), mode );
}

bool CSystemFileTranslator::Exists( const char *path )
{
    std::string output;
    struct stat tmp;

    if ( !GetFullPath( path, true, output ) )
        return false;

    return stat( output.c_str(), &tmp ) == 0;
}

bool CSystemFileTranslator::Stat(const char *path, struct stat *stats)
{
    std::string output;
    
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

void CSystemFileTranslator::ScanDirectory( const char *directory, const char *wildcard, bool recurse, 
                                            void (*dirCallback)( const std::string& dir, void *userdata), 
                                            void (*fileCallback)( const std::string& path, void *userdata), 
                                            void *userdata )
{
    WIN32_FIND_DATA		finddata;
    HANDLE				handle;
    std::string         output;
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

        if (handle != INVALID_HANDLE_VALUE)
        {
            do
            {
                if ( finddata.dwFileAttributes & (FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM | FILE_ATTRIBUTE_TEMPORARY | FILE_ATTRIBUTE_DIRECTORY) )
                    continue;

                std::string filename = output;
                filename += finddata.cFileName;

                fileCallback( filename.c_str(), userdata );		

            } while (FindNextFile(handle, &finddata));

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
				
        if ( strcmp(finddata.cFileName, ".") == 0 || strcmp(finddata.cFileName, "..") == 0 )
            continue;

        std::string target = output;
        target += finddata.cFileName;
        target += '/';

        if ( dirCallback )
            dirCallback( target.c_str(), userdata );

        if ( recurse )
            ScanDirectory( target.c_str(), wcard, true, dirCallback, fileCallback, userdata );

    } while (FindNextFile(handle, &finddata));

    FindClose(handle);
}

/*=======================================
    CFileSystem

    Management class
=======================================*/

CFileSystem::CFileSystem()
{
    char pathBuffer[1024];
    GetCurrentDirectory( 1023, pathBuffer );

    openFiles = new std::list<CFile*>;

    mtaFileRoot = CreateTranslator( pathBuffer );
}

CFileSystem::~CFileSystem()
{
    
}

CFileTranslator* CFileSystem::CreateTranslator( const char *path )
{
    CSystemFileTranslator *pTranslator;
    std::string root;
    std::vector <std::string> tree;
    unsigned int n;
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
        GetCurrentDirectory( 1023, pathBuffer );

        root += pathBuffer;
        root += path;

        if (!_File_ParseRelativePath( root.c_str() + 3, tree, &bFile ))
            return NULL;

        root.clear();
    }

    if ( bFile )
        return NULL;

    for (n=0; n<tree.size(); n++)
    {
        root += tree[n];
        root += "/";
    }

    if (!File_IsDirectoryAbsolute( root.c_str() ))
        return NULL;

    pTranslator = new CSystemFileTranslator();
    pTranslator->m_root = root;
    pTranslator->m_rootTree = tree;
    return pTranslator;
}