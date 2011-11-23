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
CSystemFileTranslator *mtaFileRoot;

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

void File_ScanDirectory(char *pDirectory, char *pWildcard, bool bRecurse,
                           void (*dirCallback)(const char *pDirectory, void *pUserdata),
                           void (*fileCallback)(const char *pFilename, void *pUserdata),
                           void *pUserdata)
{
    gameTranslator->ScanDirectory(pDirectory, pWildcard, bRecurse, dirCallback, fileCallback, pUserdata);
}

void	File_GetFullPath(const char *pPath, char *pBuffer, size_t sMaxBuffer)
{
    gameTranslator->GetFullPath(pPath, pBuffer, sMaxBuffer);
}

CFile* File_CreateBuffered(CFile *pFile)
{
    CBufferedFile *pBufferedFile = new CBufferedFile();
    pFile->Seek(0, SEEK_SET);

    pBufferedFile->m_pBuffer = (char*)Tag_Malloc(pFile->GetSize(), MEM_FILE);

    if (!pFile->Read(pBufferedFile->m_pBuffer, pFile->GetSize(), 1))
    {
        Tag_Free(pBufferedFile->m_pBuffer);

        delete pBufferedFile;
        return NULL;
    }

    pBufferedFile->m_sSize = pFile->GetSize();
    pBufferedFile->m_iSeek = 0;
    pBufferedFile->m_pPath = (char*)Tag_Strdup(pFile->GetPath(), MEM_FILE);
    return pBufferedFile;
}

CFile* File_ImportBuffered(char *pPath, void *pBuffer, size_t sBuffer)
{
    CBufferedFile *pBufferedFile;

    if (!pBuffer || sBuffer == 0)
        return NULL;

    pBufferedFile = new CBufferedFile();

    pBufferedFile->m_pBuffer = (char*)Tag_Malloc(sBuffer, MEM_FILE);
    Mem_Copy(pBufferedFile->m_pBuffer, pBuffer, sBuffer);

    pBufferedFile->m_sSize = sBuffer;
    pBufferedFile->m_iSeek = 0;
    pBufferedFile->m_pPath = (char*)Tag_Strdup(pPath, MEM_FILE);
    return pBufferedFile;
}

/*=====================================================================
*
* File_OpenAbsolute
*
* Opens a system file.
* The direct access to this function is not allowed due to security reasons.
* If you want to pierce the directory, create a new translator with a specified root!
*
=======================================================================*/

CFile* File_OpenAbsolute(const char *filename, const char *mode)
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
        case 'a':
            Console_DPrintf("Silverback does not support file append mode, ignoring (%s)\n", filename);
            break;
        case 't':
            Console_DPrintf("Silverback does not support text mode, ignoring (%s)\n", filename);
            break;
        case '+':
            if (dwAccess & GENERIC_READ)
                dwAccess |= GENERIC_WRITE;
            else if (dwAccess & GENERIC_WRITE)
                dwAccess |= GENERIC_READ;
            break;
        default:
            Console_DPrintf("Unknown file mode descriptor '%c' (%s)\n", *pModeIter, filename);
            break;
        }

        pModeIter++;
    }
    if (!dwAccess)
    {
        Console_DPrintf("Opening of file '%s' failed, no mode specified\n", filename);
        return NULL;
    }

    if (strlen(filename) == 0 || filename[strlen(filename)-1] == '/')
    {
        Console_DPrintf("weird filename %s, mode %s\n", filename, mode);
    }

    pSystemFile = CreateFile(filename, dwAccess, FILE_SHARE_READ, NULL, dwCreate, 0, NULL);

    if (pSystemFile == INVALID_HANDLE_VALUE)
        return NULL;

    pFile = new CRawFile();
    pFile->m_pFile = pSystemFile;
    pFile->m_pMode = mode;
    pFile->m_pPath = (char*)Tag_Malloc(strlen(filename)+1, MEM_FILE);
    strcpy(pFile->m_pPath, filename);

    openFiles->push_back(pFile);
    return pFile;
}

CFile* File_Open(const char *filename, const char *mode)
{
    return gameTranslator->Open(filename, mode);
}

CFile* File_OpenBuffered(const char *pFilename, const char *pMode)
{
    CFile *pFile = File_Open(pFilename, pMode);
    CFile *pBufferedFile;

    if (!pFile)
        return NULL;
    pBufferedFile = File_CreateBuffered(pFile);
    pFile->Close();

    return pBufferedFile;
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
        switch (*pPath)
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

// Parse a path so it will not turn out illegal
bool File_ParsePath( const std::string& path, std::string& output )
{
    std::vector <std::string> tree( 16 );
    unsigned int n;
    bool bFile;

    // We have to handle absolute path, too
    if ( path.length() > 2 && path[1] == ':' )
    {
        if (!_File_ParseRelativePath( path.c_str() + 3, tree, &bFile))
            return false;

        output = SString("%c:/", pPath[0]);
    }
    else
    {
        if (!_File_ParseRelativePath( pPath, dirTree, 32, &uiTreeDepth, &bFile ))
            return false;

        output.clear();
    }

    if ( bFile )
    {
        for (n=0; n<uiTreeDepth-1; n++)
            output += SString("%s/", dirTree[n]);

        output += dirTree[uiTreeDepth-1];
    }
    else
    {
        if ( uiTreeDepth == 0 )
        {
            pBuffer[0] = 0;
            return true;
        }

        for (n=0; n<uiTreeDepth; n++)
            output += SString("%s/", dirTree[n]);
    }
    return true;
}

// Get the directory correctly parsed
bool File_ParseDirectory(const char *pPath, char *pBuffer, size_t sMaxBuffer)
{
    unsigned int n;
    unsigned int uiTreeDepth;
    char dirTree[32][MAX_PATH];
    bool bFile;

    if (sMaxBuffer == 0)
        return false;

    // We have to handle absolute path, too
    if (strlen(pPath) > 2 && pPath[1] == ':')
    {
        if (sMaxBuffer < 4)
            return false;

        if (!_File_ParseRelativePath(pPath + 3, dirTree, 32, &uiTreeDepth, &bFile))
            return false;

        pBuffer[0] = pPath[0];
        pBuffer[1] = ':';
        pBuffer[2] = '/';
        pBuffer[3] = 0;
    }
    else
    {
        if (!_File_ParseRelativePath(pPath, dirTree, 32, &uiTreeDepth, &bFile))
            return false;

        pBuffer[0] = 0;
    }

    if (bFile)
    {
        if (uiTreeDepth == 1)
        {
            pBuffer[0] = 0;
            return true;
        }

        uiTreeDepth--;
    }
    else if (uiTreeDepth == 0)
    {
        pBuffer[0] = 0;
        return true;
    }

    for (n=0; n<uiTreeDepth; n++)
        strncat(pBuffer, fmt("%s/", dirTree[n]), sMaxBuffer);

    return true;
}

// Try to create a relative version of a path
static inline bool _File_ParsePathToRelative( const char *path, const std::vector <std::string>& tree, std::string& output )
{
    std::vector <std::string> pathTree( 16 );
    bool bFile;
    unsigned int n;

    if (!_File_ParseRelativePath( path + 3, pathTree, &bFile ))
        return false;

    if ( pathTree.size() < tree.size() )
        return false;

    for (n=0; n<tree.size(); n++)
        if ( pathTree[n] != tree[n] )
            return false;

    output.clear();

    if ( bFile )
    {
        for (n; n<pathTree.size()-1; n++)
        {
            output += pathTree[n];
            output += "/";
        }

        output += pathTree[n];
    }
    else
    {
        for (n; n<pathTree.size(); n++)
        {
            output += pathTree[n];
            output += "/";
        }
    }
    return true;
}

// Create a relative directory path
static inline bool _File_ParseDirectoryToRelative( const char *path, const std::vector <std::string>& tree, std::string& output )
{
    std::vector <std::string> pathTree( 16 );
    bool bFile;
    unsigned int n;

    if (!_File_ParseRelativePath( path + 3, pathTree, &bFile ))
        return false;

    if ( pathTree.size() < tree.size() )
        return false;

    for (n=0; n<tree.size(); n++)
        if ( pathTree[n] != tree[n] )
            return false;

    output.clear();

    if ( bFile )
    {
        if ( pathTree.size() == tree.size() )
            return false;

        pathTree.pop_back();
    }

    for (n; n<pathTree.size(); n++)
    {
        output += pathTree[n];
        output += "/";
    }
    return true;
}

void File_SeekToNewLine(CFile *pFile)
{
    while (!pFile->IsEOF() && pFile->ReadByte() != '\n');
}

/*===================================================
    CRawFile

    This class represents a file on the system.
    As long as it is present, the file is opened.

    fixme: Port to unix and mac
===================================================*/

CRawFile::~CRawFile()
{
    Tag_Free(m_pPath);

#ifdef _WIN32
    CloseHandle(m_pFile);
#endif

    openFiles->remove(this);
}

size_t CRawFile::Read(void *pBuffer, size_t sElement, long iNumElements)
{
#ifdef _WIN32
    DWORD dwBytesRead;

    if (sElement == 0 || iNumElements == 0)
        return 0;

    ReadFile(m_pFile, pBuffer, sElement * iNumElements, &dwBytesRead, NULL);
    return dwBytesRead / sElement;
#endif
}

size_t CRawFile::Write(void *pBuffer, size_t sElement, long iNumElements)
{
#ifdef _WIN32
    DWORD dwBytesWritten;

    if (sElement == 0 || iNumElements == 0)
        return 0;

    WriteFile(m_pFile, pBuffer, sElement * iNumElements, &dwBytesWritten, NULL);
    return dwBytesWritten / sElement;
#endif
}

int CRawFile::Seek(long iOffset, int iType)
{
#ifdef _WIN32
    if (SetFilePointer(m_pFile, iOffset, NULL, iType) == INVALID_SET_FILE_POINTER)
        return -1;
    return 0;
#endif
}

long CRawFile::Tell()
{
#ifdef _WIN32
    return SetFilePointer(m_pFile, 0, NULL, FILE_CURRENT);
#endif
}

bool CRawFile::IsEOF()
{
#ifdef _WIN32
    return (SetFilePointer(m_pFile, 0, NULL, FILE_CURRENT) == GetFileSize(m_pFile, NULL));
#endif
}

bool CRawFile::Stat(struct stat *pFileStats)
{
#ifdef _WIN32
    BY_HANDLE_FILE_INFORMATION info;
    if (!GetFileInformationByHandle(m_pFile, &info))
        return false;

    pFileStats->st_size = (long)info.nFileSizeLow >> 32 | info.nFileSizeHigh;
    pFileStats->st_dev = (long)info.nFileIndexLow >> 32 | info.nFileSizeHigh;
    pFileStats->st_atime = (long)info.ftLastAccessTime.dwLowDateTime >> 32 | info.ftLastAccessTime.dwHighDateTime;
    pFileStats->st_ctime = (long)info.ftCreationTime.dwLowDateTime >> 32 | info.ftCreationTime.dwHighDateTime;
    pFileStats->st_mtime = (long)info.ftLastWriteTime.dwLowDateTime >> 32 | info.ftLastWriteTime.dwLowDateTime;
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
    return GetFileSize(m_pFile, NULL);
#endif
}

void CRawFile::Flush()
{
#ifdef _WIN32
    FlushFileBuffers(m_pFile);
#endif
}

const char*	CRawFile::GetPath()
{
    return m_pPath;
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

size_t CRawFile::GetString(char *pBuffer, size_t sMaxLength)
{
    int i = 0;

    if (sMaxLength == 0)
        return 0;

    while (i < sMaxLength - 1)
    {
        pBuffer[i] = ReadByte();
        if (!pBuffer[i] || pBuffer[i] == '\n')
            break;
        //only increment i if we have a valid character
        if (pBuffer[i] != '\r')
            i++;
    }
    pBuffer[i] = 0;
    return i;
}

/*=========================================
    CBufferedFile

    Loads a file at open and keeps it in a managed buffer.
=========================================*/

size_t CBufferedFile::Read(void *pBuffer, size_t sElement, long iNumElements)
{
    long iReadElements = MIN((m_sSize - m_iSeek) / sElement, iNumElements);
    size_t sRead = iReadElements * sElement;

    if (iNumElements <= 0)
        return 0;

    Mem_Copy(pBuffer, m_pBuffer + m_iSeek, sRead);
    m_iSeek += sRead;
    return iReadElements;
}

size_t CBufferedFile::Write(void *pBuffer, size_t sElement, long iNumElements)
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
    m_iSeek = MAX(0, MIN(m_iSeek + iOffset, m_sSize));

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

const char* CBufferedFile::GetPath()
{
    return m_pPath;
}

void CBufferedFile::Close()
{
    Tag_Free(m_pPath);
    Tag_Free(m_pBuffer);
    delete this;
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

size_t CBufferedFile::GetString(char *pBuffer, size_t sMaxLength)
{
    int i = 0;

    if (sMaxLength == 0)
        return 0;

    while (i < sMaxLength - 1)
    {
        pBuffer[i] = ReadByte();
        if (!pBuffer[i] || pBuffer[i] == '\n')
            break;
        //only increment i if we have a valid character
        if (pBuffer[i] != '\r')
            i++;
    }
    pBuffer[i] = 0;
    return i;
}

#ifdef _FILESYSTEM_ZIP_SUPPORT

/*=======================================
    CArchiveFileTranslator

    Translator to manage files in archives.
=======================================*/

CArchiveFileTranslator* Archive_CreateTranslator(const char *pPath, enum eAccessLevel eAccess, int iFlags)
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

CFile* CArchiveFileTranslator::Open(const char *pPath, const char *pMode)
{
    char pathBuffer[1024];

    if (!*pPath)
        return NULL;

    if (pPath[1] == ':')
    {
        if (!_File_ParsePathToRelative(pPath, m_rootTree, m_uiTreeDepth, pathBuffer, 1024))
            return NULL;
    }
    else if (!File_ParsePath(pPath, pathBuffer, 1024))
        return NULL;

    return ZIP_OpenFileBuffered(m_pArchive, pathBuffer, pMode);
}

bool CArchiveFileTranslator::Exists(const char *pPath)
{
    char pathBuffer[1024];

    if (!*pPath)
        return false;

    if (pPath[1] == ':')
    {
        if (!_File_ParsePathToRelative(pPath, m_rootTree, m_uiTreeDepth, pathBuffer, 1024))
            return false;
    }
    else if (!File_ParsePath(pPath, pathBuffer, 1024))
        return false;

    return ZIP_FileExists(m_pArchive, pathBuffer);
}

size_t CArchiveFileTranslator::Size(const char *pPath)
{
    char pathBuffer[1024];
    struct stat fileStats;

    if (!*pPath)
        return 0;

    if (pPath[1] == ':')
    {
        if (!_File_ParsePathToRelative(pPath, m_rootTree, m_uiTreeDepth, pathBuffer, 1024))
            return 0;
    }
    else if (!File_ParsePath(pPath, pathBuffer, 1024))
        return 0;

    if (!ZIP_StatFile(m_pArchive, pathBuffer, &fileStats))
        return 0;

    return fileStats.st_size;
}

void CArchiveFileTranslator::GetDirectory(char *pBuffer, size_t sMaxBuffer)
{
    memcpy(pBuffer, m_pPath, min(strlen(m_pPath)+1, sMaxBuffer));
}

void CArchiveFileTranslator::ScanDirectory(char *pDirectory, char *pWildcard, bool bRecurse, 
                                              void (*dirCallback)(const char *pDirectory, void *pUserdata), 
                                              void (*fileCallback)(const char *pFilename, void *pUserdata), 
                                              void *pUserdata)
{
    char pathBuffer[1024];

    if (!*pDirectory)
        return;

    if (pDirectory[1] == ':')
    {
        if (!_File_ParseDirectoryToRelative(pDirectory, m_rootTree, m_uiTreeDepth, pathBuffer, 1024))
            return;
    }
    else if (!File_ParseDirectory(pDirectory, pathBuffer, 1024))
        return;

    ZIP_ScanDirectory(m_pArchive, m_pRoot, pathBuffer, pWildcard, bRecurse, dirCallback, fileCallback, pUserdata);
}

bool CArchiveFileTranslator::Stat(const char *pPath, struct stat *pStats)
{
    char pathBuffer[1024];

    if (!*pPath)
        return false;

    if (pPath[1] == ':')
    {
        if (!_File_ParsePathToRelative(pPath, m_rootTree, m_uiTreeDepth, pathBuffer, 1024))
            return false;
    }
    else if (!File_ParsePath(pPath, pathBuffer, 1024))
        return false;

    return ZIP_StatFile(m_pArchive, pathBuffer, pStats);
}

eAccessLevel CArchiveFileTranslator::GetAccessLevel()
{
    return m_eAccessPriviledge;
}

int CArchiveFileTranslator::GetFlags()
{
    return m_iFlags;
}

void CArchiveFileTranslator::GetHash(char pHash[MAX_HASH_SIZE], int *pHashLength)
{
    memcpy(pHash, m_hash, m_iHashLength);

    *pHashLength = m_iHashLength;
}

void CArchiveFileTranslator::Delete()
{
    ZIP_Close(m_pArchive);

    Tag_Free(m_pPath);
    Tag_Free(m_pRoot);

    registeredArchives->remove(this);
    delete this;
}

#endif //_FILESYSTEM_ZIP_SUPPORT

/*=======================================
    CSystemFileTranslator

    Default file translator
=======================================*/

CSystemFileTranslator* File_CreateTranslator(const char *pRootPath, enum eAccessLevel eAccess)
{
    CSystemFileTranslator *pTranslator;
    char pathBuffer[1024];
    char dirTree[32][MAX_PATH];
    unsigned int uiTreeDepth;
    bool bFile;
    unsigned int n;

    // We have to handle absolute path, too
    if (strlen(pRootPath) > 2 && pRootPath[1] == ':')
    {
        if (!_File_ParseRelativePath(pRootPath + 3, dirTree, 32, &uiTreeDepth, &bFile))
            return NULL;

        if (uiTreeDepth == 0)
            return NULL;

        pathBuffer[0] = pRootPath[0];
        pathBuffer[1] = ':';
        pathBuffer[2] = '/';
        pathBuffer[3] = 0;
    }
    else
    {
        strncpy(pathBuffer, System_GetRootDir(), 1023);
        strncat(pathBuffer, pRootPath, 1023);

        if (!_File_ParseRelativePath(pathBuffer + 3, dirTree, 32, &uiTreeDepth, &bFile))
            return NULL;

        pathBuffer[3] = 0;
    }

    if (bFile)
        return NULL;

    for (n=0; n<uiTreeDepth; n++)
        strncat(pathBuffer, fmt("%s/", dirTree[n]), 1024);

    if (!File_IsDirectoryAbsolute(pathBuffer))
        return NULL;

    pTranslator = new CSystemFileTranslator();
    pTranslator->m_eAccessPriviledge = eAccess;

    strcpy(pTranslator->m_root, pathBuffer);

    memcpy(pTranslator->m_rootTree, dirTree, sizeof(pTranslator->m_rootTree));
    pTranslator->m_uiTreeDepth = uiTreeDepth;
    pTranslator->m_currentDir[0] = 0;
    return pTranslator;
}

CFile* CSystemFileTranslator::Open(const char *pPath, const char *pMode)
{
    char pathBuffer[1024];
    char full_system_path[1024];
    bool bWriting = false;
    CFile *pFile;

    if (!*pPath)
        return NULL;

    if (pPath[0] == '/')
    {
        if (!File_ParsePath(pPath + 1, pathBuffer, 1024))
            return NULL;

        snprintf(full_system_path, 1023, "%s%s", m_root, pathBuffer);
    }
    else
    {
        if (pPath[1] == ':')
        {
            if (!_File_ParsePathToRelative(pPath, m_rootTree, m_uiTreeDepth, pathBuffer, 1024))
                return NULL;

            snprintf(full_system_path, 1023, "%s%s", m_root, pathBuffer);
        }
        else
        {
            snprintf(pathBuffer, 1023, "%s%s", m_currentDir, pPath);

            if (!File_ParsePath(pathBuffer, pathBuffer, 1024))
                return false;

            snprintf(full_system_path, 1023, "%s%s", m_root, pathBuffer);
        }
    }

    return File_OpenAbsolute(full_system_path, pMode);
}

bool CSystemFileTranslator::Exists(const char *pPath)
{
    STRUCT_STAT tmp;
    bool bExists = false;
    char pathBuffer[1024];
    char full_system_path[1024];

    if (!*pPath)
        return false;

    if (pPath[0] == '/')
    {
        if (!File_ParsePath(pPath + 1, pathBuffer, 1024))
            return false;

        snprintf(full_system_path, 1023, "%s%s", m_root, pathBuffer);
    }
    else
    {
        if (pPath[1] == ':')
        {
            if (!_File_ParsePathToRelative(pPath, m_rootTree, m_uiTreeDepth, pathBuffer, 1024))
                return false;

            snprintf(full_system_path, 1023, "%s%s", m_root, pathBuffer);
        }
        else
        {
            snprintf(pathBuffer, 1023, "%s%s", m_currentDir, pPath);

            if (!File_ParsePath(pathBuffer, pathBuffer, 1024))
                return false;

            snprintf(full_system_path, 1023, "%s%s", m_root, pathBuffer);
        }
    }

    if (archive_precedence.integer == 1)
        bExists = Archives_FileExists(full_system_path);

    if (!bExists)
        bExists = (stat(full_system_path, &tmp) == 0);

    if (!bExists && archive_precedence.integer != 1)
        bExists = Archives_FileExists(full_system_path);

    return bExists;
}

void CSystemFileTranslator::GetDirectory(char *pBuffer, size_t sMaxBuffer, bool bSystem)
{
    if (bSystem)
        snprintf(pBuffer, sMaxBuffer, "%s%s", m_root, m_currentDir);
    else
        snprintf(pBuffer, sMaxBuffer, "/%s", m_currentDir);
}

bool CSystemFileTranslator::ChangeDirectory(const char *pPath)
{
    char pathBuffer[1024];

    if (pPath[0] == '/')
    {
        if (!File_ParseDirectory(pPath + 1, pathBuffer, 1024))
            return false;
    }
    else
    {
        if (pPath[1] == ':')
        {
            if (!_File_ParseDirectoryToRelative(pPath, m_rootTree, m_uiTreeDepth, pathBuffer, 1024))
                return false;
        }
        else
        {
            snprintf(pathBuffer, 1023, "%s%s", m_currentDir, pPath);

            if (!File_ParseDirectory(pathBuffer, pathBuffer, 1024))
                return false;
        }
    }

    memcpy(m_currentDir, pathBuffer, 1024);
    return true;
}

bool CSystemFileTranslator::Stat(const char *pPath, struct stat *pStats)
{
    bool ret = false;
    char pathBuffer[1024];
    char full_system_path[1024];
    OVERHEAD_INIT;

    if (!*pPath)
        return false;

    if (pPath[0] == '/')
    {
        if (!File_ParsePath(pPath + 1, pathBuffer, 1024))
            return NULL;

        snprintf(full_system_path, 1023, "%s%s", m_root, pathBuffer);
    }
    else
    {
        if (pPath[1] == ':')
        {
            if (!_File_ParsePathToRelative(pPath, m_rootTree, m_uiTreeDepth, pathBuffer, 1024))
                return NULL;

            snprintf(full_system_path, 1023, "%s%s", m_root, pathBuffer);
        }
        else
        {
            snprintf(pathBuffer, 1023, "%s%s", m_currentDir, pPath);

            if (!File_ParsePath(pathBuffer, pathBuffer, 1024))
                return false;

            snprintf(full_system_path, 1023, "%s%s", m_root, pathBuffer);
        }
    }

    if (archive_precedence.integer == 1)
        ret = Archive_StatFile(full_system_path, pStats);

    if (!ret)
        ret = (stat(full_system_path, pStats) == 0);

    if (!ret && archive_precedence.integer != 1)
        ret = Archive_StatFile(full_system_path, pStats);

    OVERHEAD_COUNT(OVERHEAD_IO);
    return ret;
}

size_t CSystemFileTranslator::Size(const char *pPath)
{
    STRUCT_STAT fstats;
    char pathBuffer[1024];
    char full_system_path[1024];
    OVERHEAD_INIT;

    if (!*pPath)
        return 0;

    if (pPath[0] == '/')
    {
        if (!File_ParsePath(pPath + 1, pathBuffer, 1024))
            return 0;
    }
    else
    {
        if (pPath[1] == ':')
        {
            if (!_File_ParsePathToRelative(pPath, m_rootTree, m_uiTreeDepth, pathBuffer, 1024))
                return 0;
        }
        else
        {
            snprintf(pathBuffer, 1023, "%s%s", m_currentDir, pPath);

            if (!File_ParsePath(pathBuffer, pathBuffer, 1024))
                return 0;
        }
    }
    snprintf(full_system_path, 1023, "%s%s", m_root, pathBuffer);

    if (archive_precedence.integer == 1 && Archive_StatFile(full_system_path, &fstats))
    {
        OVERHEAD_COUNT(OVERHEAD_IO);
        return fstats.st_size;
    }

    if (File_Stat(pathBuffer, &fstats))
    {
        OVERHEAD_COUNT(OVERHEAD_IO);
        return fstats.st_size;
    }

    if (archive_precedence.integer != 1 && Archive_StatFile(full_system_path, &fstats))
    {
        OVERHEAD_COUNT(OVERHEAD_IO);
        return fstats.st_size;
    }

    OVERHEAD_COUNT(OVERHEAD_IO);
    return 0;
}

bool CSystemFileTranslator::GetFullPath(const char *pPath, std::string& output)
{
    char pathBuffer[1024];

    if (!*pPath)
        return false;

    if (sMaxBuffer == 0)
        return false;

    sMaxBuffer--;

    if (pPath[0] == '/')
    {
        if (!File_ParsePath(pPath + 1, pathBuffer, 1023))
            return false;

        snprintf(pBuffer, sMaxBuffer, "%s%s", m_root, pathBuffer);
    }
    else if (pPath[1] == ":")
    {
        if (!_File_ParsePathToRelative(pPath, m_rootTree, m_uiTreeDepth, pathBuffer, 1023))
            return NULL;

        snprintf(pathBuffer, 1023, "%s%s", m_root, pathBuffer);
    }
    else
    {
        snprintf(pathBuffer, 1023, "%s%s", m_currentDir, pPath);

        if (!File_ParsePath(pathBuffer, pathBuffer, 1023))
            return false;

        snprintf(pBuffer, sMaxBuffer, "%s%s", m_root, pathBuffer);
    }
    return true;
}

void CSystemFileTranslator::ScanDirectory(char *pDirectory, char *pWildcard, bool bRecurse, 
                                             void (*dirCallback)(const char *pDirectory, void *pUserdata), 
                                             void (*fileCallback)(const char *pFilename, void *pUserdata), 
                                             void *pUserdata)
{
    WIN32_FIND_DATA		finddata;
    HANDLE				handle;
    char				searchstring[1024];
    char				pathBuffer[1024];
    char				wcard[256];
    char				fullDirectory[1024];

    if (!*pDirectory)
        return;

    if (pDirectory[0] == '/')
    {
        if (!File_ParseDirectory(pDirectory + 1, pathBuffer, 1024))
            return;
    }
    else
    {
        if (pDirectory[1] == ':')
        {
            if (!_File_ParseDirectoryToRelative(pDirectory, m_rootTree, m_uiTreeDepth, pathBuffer, 1024))
                return;
        }
        else
        {
            snprintf(pathBuffer, 1023, "%s%s", m_currentDir, pDirectory);

            if (!File_ParseDirectory(pathBuffer, pathBuffer, 1024))
                return;
        }
    }
    snprintf(fullDirectory, 1023, "%s%s", m_root, pathBuffer);

    Archive_ScanDirectory(fullDirectory, pWildcard, bRecurse, dirCallback, fileCallback, pUserdata);

    if (!pWildcard)
        strcpy(wcard, "*");
    else
        strncpy(wcard, pWildcard, 255);

    //first search for files only
    if (fileCallback)
    {
        BPrintf(searchstring, 1023, "%s%s", fullDirectory, wcard);

        Cvar_SetVar(&sys_enumdir, fullDirectory);

        handle = FindFirstFile(searchstring, &finddata);

        if (handle != INVALID_HANDLE_VALUE)
        {
            do
            {
                if (!(finddata.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN) &&
                    !(finddata.dwFileAttributes & FILE_ATTRIBUTE_SYSTEM) &&
                    !(finddata.dwFileAttributes & FILE_ATTRIBUTE_TEMPORARY) &&
                    !(finddata.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
                {
                    snprintf(pathBuffer, 1023, "%s%s", fullDirectory, finddata.cFileName);

                    fileCallback(pathBuffer, pUserdata);		
                }
            } while (FindNextFile(handle, &finddata));
        }

        FindClose(handle);
    }

    //next search for subdirectories only
    BPrintf(searchstring, 1023, "%s*", fullDirectory);
    searchstring[1023] = 0;

    handle = FindFirstFile(searchstring, &finddata);

    if (handle == INVALID_HANDLE_VALUE)
        return;

    do
    {
        if (!(finddata.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN) &&
            !(finddata.dwFileAttributes & FILE_ATTRIBUTE_SYSTEM) &&
            !(finddata.dwFileAttributes & FILE_ATTRIBUTE_TEMPORARY))
        {
            if (finddata.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            {					
                if (strcmp(finddata.cFileName, ".") && strcmp(finddata.cFileName, "..") && strcmp(finddata.cFileName, "CVS"))
                {
                    BPrintf(pathBuffer, 1023, "%s%s/", fullDirectory, finddata.cFileName);

                    Cvar_SetVar(&sys_enumdir, pathBuffer);

                    if (dirCallback)
                        dirCallback(pathBuffer, pUserdata);

                    if (bRecurse)
                    {
                        ScanDirectory(pathBuffer, wcard, true, dirCallback, fileCallback, pUserdata);
                    }			
                }
            }
        }
    } while (FindNextFile(handle, &finddata));

    FindClose(handle);

    Cvar_SetVar(&sys_enumdir, "");
}

eAccessLevel CSystemFileTranslator::GetAccessLevel()
{
    return m_eAccessPriviledge;
}

eAccessLevel CSystemFileTranslator::GetPathAccessLevel(const char *pPath)
{
    STRUCT_STAT stats;
    CArchiveFileTranslator *pArchive;
    char pathBuffer[1024];
    char full_system_path[1024];

    if (pPath[0] == '/')
    {
        if (!File_ParsePath(pPath + 1, pathBuffer, 1024))
            return ACCESS_GUEST;

        snprintf(full_system_path, 1023, "%s%s", m_root, pathBuffer);
    }
    else
    {
        if (pPath[1] == ':')
        {
            if (!_File_ParsePathToRelative(pPath, m_rootTree, m_uiTreeDepth, pathBuffer, 1024))
                return ACCESS_GUEST;

            snprintf(full_system_path, 1023, "%s%s", m_root, pathBuffer);
        }
        else
        {
            snprintf(pathBuffer, 1023, "%s%s", m_currentDir, pPath);

            if (!File_ParsePath(pathBuffer, pathBuffer, 1024))
                return ACCESS_GUEST;

            snprintf(full_system_path, 1023, "%s%s", m_root, pathBuffer);
        }
    }

    if (archive_precedence.integer == 1 && (pArchive = Archives_FindPath(full_system_path)))
        return pArchive->GetAccessLevel();
    if (stat(full_system_path, &stats) == 0)
        return ACCESS_GAME;
    if (archive_precedence.integer != 1 && (pArchive = Archives_FindPath(full_system_path)))
        return pArchive->GetAccessLevel();
    return ACCESS_GUEST;
}

/*=======================================
    CFileSystem

    Management class
=======================================*/

CFileSystem::CFileSystem()
{
    char pathBuffer[1024];
    GetCurrentDirectory( 1023, pathBuffer );

    openFiles = new list<CFile*>();

    mtaFileRoot = CreateTranslator( pathBuffer );
}

CFileSystem::~CFileSystem()
{
    
}

CFileTranslator* CFileSystem::CreateTranslator( const std::string& path )
{
    CSystemFileTranslator *pTranslator;
    std::string root;
    std::vector <std::string> tree;
    unsigned int n;
    bool bFile;

    // We have to handle absolute path, too
    if ( path.length() > 2 && path[1] == ':' )
    {
        if (!_File_ParseRelativePath( path.substr( 3 ), tree, &bFile ))
            return NULL;

        if (uiTreeDepth == 0)
            return NULL;

        root = SString("%c:/", path[0]);
    }
    else
    {
        strncpy(pathBuffer, System_GetRootDir(), 1023);
        strncat(pathBuffer, pRootPath, 1023);

        if (!_File_ParseRelativePath(pathBuffer + 3, dirTree, 32, &uiTreeDepth, &bFile))
            return NULL;

        pathBuffer[3] = 0;
    }

    if (bFile)
        return NULL;

    for (n=0; n<uiTreeDepth; n++)
        strncat(pathBuffer, fmt("%s/", dirTree[n]), 1024);

    if (!File_IsDirectoryAbsolute(pathBuffer))
        return NULL;

    pTranslator = new CSystemFileTranslator();
    pTranslator->m_eAccessPriviledge = eAccess;

    strcpy(pTranslator->m_root, pathBuffer);

    memcpy(pTranslator->m_rootTree, dirTree, sizeof(pTranslator->m_rootTree));
    pTranslator->m_uiTreeDepth = uiTreeDepth;
    pTranslator->m_currentDir[0] = 0;
    return pTranslator;
}