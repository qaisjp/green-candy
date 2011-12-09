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
    filePath&       GetPath();
    bool            IsReadable();
    bool            IsWriteable();

private:
    friend class CSystemFileTranslator;
    friend class CFileSystem;
	
#ifdef _WIN32
    HANDLE          m_file;
    DWORD           m_access;
#endif
    filePath        m_path;
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
    filePath&       GetPath();
    bool            IsReadable();
    bool            IsWriteable();

    int             ReadInt();
    short           ReadShort();
    char            ReadByte();

private:
    char*           m_pBuffer;
    long            m_iSeek;
    size_t          m_sSize;
    filePath        m_path;
};

class CSystemPathTranslator : public CFileTranslator
{
public:
    bool            GetFullPathTree( const char *path, dirTree& tree, bool *file );
    bool            GetRelativePathTree( const char *path, dirTree& tree, bool *file );
    bool            GetFullPath( const char *path, bool allowFile, filePath& output );
    bool            GetRelativePath( const char *path, bool allowFile, filePath& output );
    bool            ChangeDirectory( const char *path );
    void            GetDirectory( filePath& output );

protected:
    friend class CFileSystem;

    filePath        m_root;
    filePath        m_currentDir;
    dirTree         m_rootTree;
};

class CSystemFileTranslator : public CSystemPathTranslator
{
public:
    bool            WriteData( const char *path, const char *buffer, size_t size );
    bool            CreateDir( const char *path );
    CFile*          Open( const char *path, const char *mode );
    bool            Exists( const char *path );
    bool            Delete( const char *path );
    bool            Copy( const char *src, const char *dst );
    bool            Rename( const char *src, const char *dst );
    size_t          Size( const char *path );
    bool            Stat( const char *path, struct stat *stats );
    bool            ReadToBuffer( const char *path, std::vector <char>& output );

    void            ScanDirectory( const char *directory, const char *wildcard, bool recurse, 
                        pathCallback_t dirCallback, 
                        pathCallback_t fileCallback, 
                        void *userdata );

    void            GetDirectories( const char *path, const char *wildcard, bool recurse, std::vector <filePath>& output );
    void            GetFiles( const char *path, const char *wildcard, bool recurse, std::vector <filePath>& output );

private:
    void            _CreateDirTree( dirTree& tree );
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
                        void (*dirCallback)( const filePath& directory, void *userdata ), 
                        void (*fileCallback)( const filePath& filename, void *userdata ), 
                        void *userdata );

private:
    friend class CFileSystem;

    void*           m_pArchive;
};

#endif //_FILESYSTEM_ZIP_SUPPORT


extern CFileTranslator *tempFileRoot;
extern CFileTranslator *mtaFileRoot;
extern CFileTranslator *dataFileRoot;
extern CFileTranslator *modFileRoot;
extern CFileTranslator *newsFileRoot;
extern CFileTranslator *gameFileRoot;

class CFileSystem : public CFileSystemInterface
{
public:
                            CFileSystem();
                            ~CFileSystem();
                            
    CFileTranslator*        CreateTranslator( const char *path );

    // Insecure functions
    bool                    IsDirectory( const char *path );
    bool                    WriteMiniDump( const char *path, _EXCEPTION_POINTERS *except );
    bool                    Exists( const char *path );
    size_t                  Size( const char *path );
    bool                    ReadToBuffer( const char *path, std::vector <char>& output );
};

#endif //_CFileSystem_