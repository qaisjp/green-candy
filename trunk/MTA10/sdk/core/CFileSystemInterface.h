/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        sdk/core/CFileSystemInterface.h
*  PURPOSE:     File management
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#ifndef _CFileSystemInterface_
#define _CFileSystemInterface_

class CFile
{
public:
    virtual                 ~CFile();

    virtual	size_t          Read( void *pBuffer, size_t sElement, unsigned long iNumElements ) = 0;
    virtual	size_t          Write( void *pBuffer, size_t sElement, unsigned long iNumElements ) = 0;
    virtual	int             Seek( long iOffset, int iType ) = 0;
    virtual	long            Tell() = 0;
    virtual	bool            IsEOF() = 0;
    virtual	bool            Stat( struct stat *pFileStats ) = 0;
    virtual	size_t          GetSize() = 0;
    virtual	void            Flush() = 0;
    virtual std::string&    GetPath() = 0;

    virtual	int             ReadInt() { return 0; }
    virtual	short           ReadShort() { return 0; }
    virtual	char            ReadByte() { return 0; }
    virtual	float           ReadFloat() { return 0; }
    virtual	size_t          WriteInt( int iInt ) { return 0; }
    virtual size_t          WriteShort( short iShort ) { return 0; }
    virtual size_t          WriteByte( char cByte ) { return 0; }
    virtual size_t          WriteFloat( float fFloat ) { return 0; }
    virtual	size_t          Printf( const char *pFormat, ... ) { return 0; }
};

class CFileTranslator
{
public:
    virtual                 ~CFileTranslator() = 0;

    virtual CFile*          Open( const char *path, const char *mode ) = 0;
    virtual bool            Exists( const char *path ) = 0;
    virtual bool            Delete( const char *path ) = 0;
    virtual size_t          Size( const char *path ) = 0;
    virtual bool            Stat( const char *path, struct stat *stats ) = 0;
    virtual bool            GetFullPath( const char *path, bool allowFile, std::string& output ) = 0;
    virtual bool            GetRelativePath( const char *path, bool allowFile, std::string& output ) = 0;
    virtual bool            ChangeDirectory( const char *path ) = 0;
    virtual void            GetDirectory( std::string& output ) = 0;

    virtual void            ScanDirectory( const char *directory, const char *wildcard, bool recurse, 
                                void (*dirCallback)( const std::string& directory, void *userdata ), 
                                void (*fileCallback)( const std::string& path, void *userdata ), 
                                void *userdata ) = 0;
};

class CFileSystemInterface
{
public:
    virtual CFileTranslator*    CreateTranslator( const char *path ) = 0;
};

#endif //_CFileSystemInterface_