/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        core/CFileSystem.h
*  PURPOSE:     File management
*  DEVELOPERS:  S2Games <http://savage.s2games.com> (historical entry)
*               The_GTA <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#ifndef _CFileSystem_
#define _CFileSystem_

#define MAX_FILE_BLOCKS 1024

typedef struct
{
    char name[5];
    int pos;		//position in buffer
    unsigned int length;
    byte *data;		//data pointer
} block_t;

typedef struct
{
    int num_blocks;
    int _num_allocated;

    block_t *blocks;
} blockList_t;

#define ARCHIVE_NORMAL					0x00
#define ARCHIVE_OFFICIAL 				0x01
#define ARCHIVE_THIS_CONNECTION_ONLY 	0x02

class CRawFile : public CFile
{
public:
                    ~CRawFile();

    size_t          Read( void *pBuffer, size_t sElement, long iNumElements );
    size_t          Write( void *pBuffer, size_t sElement, long iNumElements );
    int             Seek( long iOffset, int iType );
    long            Tell();
    bool            IsEOF();
    bool            Stat( struct stat *pFileStats );
    size_t          GetSize();
    void            Flush();
    const char*     GetPath();

    int             ReadInt();
    short           ReadShort();
    char            ReadByte();
    size_t          WriteInt( int iInt );
    size_t          WriteShort( short iShort );
    size_t          WriteByte( char cByte );

    size_t          Printf( const char *pFormat, ... );
    size_t          GetString( char *pBuffer, size_t sMaxLength );
private:
    friend CFile*   File_OpenAbsolute( const char *pPath, const char *pMode );
	
    void*           m_pFile;
    const char*     m_pMode;
    char*           m_pPath;
};

class CBufferedFile : public CFile
{
public:
                    ~CBufferedFile();

    size_t          Read( void *pBuffer, size_t sElement, long iNumElements );
    size_t          Write( void *pBuffer, size_t sElement, long iNumElements );
    int             Seek( long iOffset, int iType );
    long            Tell();
    bool            IsEOF();
    bool            Stat( struct stat *pFileStats );
    size_t          GetSize();
    void            Flush();
    const char*     GetPath();

    int             ReadInt();
    short           ReadShort();
    char            ReadByte();

    size_t          GetString( char *pBuffer, size_t sMaxLength );
private:
    friend CFile*   File_CreateBuffered( CFile *pFile );
    friend CFile*   File_ImportBuffered( char *pPath, void *pBuffer, size_t sBuffer );

    char*           m_pBuffer;
    long            m_iSeek;
    size_t          m_sSize;
    char*           m_pPath;
};

class CSystemFileTranslator
{
public:
                    ~CSystemFileTranslator();

    CFile*          Open( const char *pPath, const char *pMode );
    bool            Exists( const char *pPath );
    size_t          Size( const char *pPath );
    void            GetDirectory( char *pBuffer, size_t sMaxBuffer, bool bSystem );
    bool            GetFullPath( const char *pPath, char *pBuffer, size_t sMaxBuffer );
    bool            ChangeDirectory( const char *pPath );
    bool            Stat( const char *pPath, struct stat *pStats );

    void            ScanDirectory( char *pDirectory, char *pWildcard, bool bRecurse, 
                        void (*dirCallback)( const char *pDirectory, void *pUserdata ), 
                        void (*fileCallback)( const char *pFilename, void *pUserdata ), 
                        void *pUserdata );

    eAccessLevel    GetAccessLevel();
    eAccessLevel    GetPathAccessLevel( const char *pPath );
private:
    friend class CFileSystem;

    char            m_root[2048];
    char            m_currentDir[1024];
    unsigned int    m_uiTreeDepth;
    char            m_rootTree[32][MAX_PATH];

    eAccessLevel    m_eAccessPriviledge;
};

#ifdef _FILESYSTEM_ZIP_SUPPORT

class	CArchiveFileTranslator
{
public:
    CFile*          Open( const char *pPath, const char *pMode );
    bool            Exists( const char *pPath );
    size_t          Size( const char *pPath );
    void            GetDirectory( char *pBuffer, size_t sMaxBuffer );
    bool            Stat( const char *pPath, struct stat *pStats );

    void            ScanDirectory( char *pDirectory, char *pWildcard, bool bRecurse, 
                        void (*dirCallback)( const char *pDirectory, void *pUserdata ), 
                        void (*fileCallback)( const char *pFilename, void *pUserdata ), 
                        void *pUserdata );

    int             GetFlags();
    void            GetHash( char pHash[MAX_HASH_SIZE], int *pHashLength );
    eAccessLevel    GetAccessLevel();
    void            Delete();
private:
    friend class CFileSystem;

    void*           m_pArchive;
    unsigned char   m_hash[MAX_HASH_SIZE];
    int             m_iHashLength;
    int             m_iFlags;
    char*           m_pPath;
    char*           m_pRoot;
    unsigned int    m_uiTreeDepth;
    char            m_rootTree[32][MAX_PATH];

    eAccessLevel    m_eAccessPriviledge;
};

#endif //_FILESYSTEM_ZIP_SUPPORT


extern CSystemFileTranslator *mtaFileRoot;

class CFileSystem
{
public:
    
};

void    Archive_Init();
void    Archive_Shutdown();
int     Archive_RegisterArchivesInDir(const char *path);
void    Archive_RegisterOfficialArchives();
CArchiveFileTranslator*	Archive_GetArchive(const char *path);
int     Archive_UnregisterUnusedArchives(); //should only be called on a disconnect

void    File_GetHashStrings(char *hashstring, int size, int xorbits);
bool    File_CompareHashStrings(char *hashString, int xorbits);

CFile*	File_CreateBuffered(CFile *pFile);
CFile*	File_ImportBuffered(char *pPath, void *pBuffer, size_t sBuffer);

CSystemFileTranslator*	File_CreateTranslator(const char *pRootPath, enum eAccessLevel eAccess);
eAccessLevel	File_GetPathAccessLevel(const char *pPath);
bool	File_ParsePath(const char *pPath, char *pBuffer, size_t sMaxBuffer);
bool	File_ParseDirectory(const char *pPath, char *pBuffer, size_t sMaxBuffer);
CFile*	File_Open(const char *filename, const char *mode);
CFile*	File_OpenBuffered(const char *pFilename, const char *pMode);
bool	File_Exists(const char *filename);
bool	File_Stat(const char *filename, struct stat *stats);
#ifdef unix
bool	File_StatAbsolute(const char *filename, struct stat *stats);
int		File_SizeAbsolute(const char *filename);
void	*File_LoadIntoBufferAbsolute(const char *filename, int *length, int tag);
#endif
char	*File_GetNextFileIncrement(int num_digits, const char *basename, const char *ext, char *filename, int size);
void	File_ChangeDir(const char *dir);
void	File_ResetDir();
void	File_GetCurrentDir(char *pBuffer, size_t sMaxBuffer);
size_t	File_Size(const char *filename);
void	File_GetFullPath(const char *pPath, char *pBuffer, size_t sMaxBuffer);
blockList_t *File_AllocBlockList(const void *buf, int buflen, const char *filename_debug);
void	File_FreeBlockList(blockList_t *blocklist);

bool    File_Delete(const char *filename);

void	File_ScanDirectory(char *directory, char *wildcard, bool recurse,
					   void(*dirCallback)(const char *dir, void *userdata),
					   void(*fileCallback)(const char *filename, void *userdata),
					   void *userdata);

void	File_Init();
void	File_Shutdown();

//utilities
void    File_AddOpenFileTracking( CFile *pFile );
void    File_DelOpenFileTracking( CFile *pFile );

#endif //_CFileSystem_