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

    size_t          Read( void *pBuffer, size_t sElement, unsigned long iNumElements );
    size_t          Write( void *pBuffer, size_t sElement, unsigned long iNumElements );
    int             Seek( long iOffset, int iType );
    long            Tell();
    bool            IsEOF();
    bool            Stat( struct stat *pFileStats );
    size_t          GetSize();
    void            Flush();
    std::string&    GetPath();

    int             ReadInt();
    short           ReadShort();
    char            ReadByte();
    size_t          WriteInt( int iInt );
    size_t          WriteShort( short iShort );
    size_t          WriteByte( char cByte );

    size_t          Printf( const char *pFormat, ... );
private:
    friend CFile*   File_OpenAbsolute( const char *pPath, const char *pMode );
	
    void*           m_file;
    const char*     m_mode;
    std::string     m_path;
};

class CBufferedFile : public CFile
{
public:
                    ~CBufferedFile();

    size_t          Read( void *pBuffer, size_t sElement, unsigned long iNumElements );
    size_t          Write( void *pBuffer, size_t sElement, unsigned long iNumElements );
    int             Seek( long iOffset, int iType );
    long            Tell();
    bool            IsEOF();
    bool            Stat( struct stat *pFileStats );
    size_t          GetSize();
    void            Flush();
    std::string&    GetPath();

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
    std::string     m_path;
};

class CSystemPathTranslator : public CFileTranslator
{
public:
                    ~CSystemPathTranslator();

    bool            GetFullPath( const char *path, bool allowFile, std::string& output );
    bool            GetRelativePath( const char *path, bool allowFile, std::string& output );
    bool            ChangeDirectory( const char *path );
    void            GetDirectory( std::string& output );

private:
    friend class CFileSystem;

    std::string     m_root;
    std::string     m_currentDir;
    std::vector <std::string>   m_rootTree;
};

class CSystemFileTranslator : public CSystemPathTranslator
{
public:
                    ~CSystemFileTranslator();
                    
    CFile*          Open( const char *path, const char *mode );
    bool            Exists( const char *path );
    bool            Delete( const char *path );
    size_t          Size( const char *path );
    bool            Stat( const char *path, struct stat *stats );

    void            ScanDirectory( const char *directory, const char *wildcard, bool recurse, 
                        void (*dirCallback)( const std::string& directory, void *userdata ), 
                        void (*fileCallback)( const std::string& filename, void *userdata ), 
                        void *userdata );
};

#ifdef _FILESYSTEM_ZIP_SUPPORT

class	CArchiveFileTranslator : public CSystemPathTranslator
{
public:
                    ~CArchiveFileTranslator();

    CFile*          Open( const char *path, const char *mode );
    bool            Exists( const char *path );
    bool            Delete( const char *path );
    size_t          Size( const char *path );
    bool            Stat( const char *path, struct stat *stats );

    void            ScanDirectory( char *directory, char *wildcard, bool recurse, 
                        void (*dirCallback)( const char *directory, void *userdata ), 
                        void (*fileCallback)( const char *filename, void *userdata ), 
                        void *userdata );

    int             GetFlags();

private:
    friend class CFileSystem;

    void*           m_pArchive;
    int             m_iFlags;
};

#endif //_FILESYSTEM_ZIP_SUPPORT


extern CFileTranslator *mtaFileRoot;

class CFileSystem : public CFileSystemInterface
{
public:
                            CFileSystem();
                            ~CFileSystem();
                            
    CFileTranslator*        CreateTranslator( const char *path );
    bool                    IsDirectory( const char *path );
};

CFile*	File_CreateBuffered(CFile *pFile);

CFile*	File_Open(const char *filename, const char *mode);
CFile*	File_OpenBuffered(const char *pFilename, const char *pMode);

#endif //_CFileSystem_