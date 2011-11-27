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

enum eFileException
{
    FILE_STREAM_TERMINATED  // user attempts to perform on a terminated file stream, ie. http timeout
};

class CFile
{
public:
    virtual                 ~CFile() = 0;

    virtual	size_t          Read( void *pBuffer, size_t sElement, unsigned long iNumElements ) = 0;
    virtual	size_t          Write( void *pBuffer, size_t sElement, unsigned long iNumElements ) = 0;
    virtual	int             Seek( long iOffset, int iType ) = 0;
    virtual	long            Tell() = 0;
    virtual	bool            IsEOF() = 0;
    virtual	bool            Stat( struct stat *stats ) = 0;
    virtual	size_t          GetSize() = 0;
    virtual	void            Flush() = 0;
    virtual std::string&    GetPath() = 0;
    virtual bool            IsReadable() = 0;
    virtual bool            IsWriteable() = 0;

    // Utility definitions
    virtual	int             ReadInt()
    {
        int i;

        Read( &i, sizeof(int), 1 );
        return i;
    }

    virtual	short           ReadShort()
    {
        short i;

        Read( &i, sizeof(short), 1 );
        return i;
    }

    virtual	char            ReadByte()
    {
        char i;

        Read( &i, sizeof(char), 1 );
        return i;
    }

    virtual	float           ReadFloat()
    {
        float f;

        Read( &f, sizeof(float), 1 );
        return f;
    }

    virtual	size_t          WriteInt( int iInt )
    {
        return Write( &iInt, sizeof(int), 1 );
    }

    virtual size_t          WriteShort( short iShort )
    {
        return Write( &iShort, sizeof(short), 1 );
    }

    virtual size_t          WriteByte( char cByte )
    {
        return Write( &cByte, sizeof(char), 1 );
    }

    virtual size_t          WriteFloat( float fFloat )
    {
        return Write( &fFloat, sizeof(float), 1 );
    }

    virtual	size_t          Printf( const char *pFormat, ... )
    {
        va_list args;
        char cBuffer[1024];

        va_start(args, pFormat);
        _vsnprintf(cBuffer, 1023, pFormat, args);
        va_end(args);

        return Write(cBuffer, 1, strlen(cBuffer));
    }

    virtual void            GetString( std::string &output )
    {
        for (;;)
        {
            unsigned char c = ReadByte();

            if ( !c || c == '\n' )
                return;

            output += c;
        }
    }
};

class CFileTranslator
{
public:
    virtual                 ~CFileTranslator() = 0;

    virtual bool            WriteData( const char *path, char *buffer, size_t size ) = 0;
    virtual bool            CreateDir( const char *path ) = 0;
    virtual CFile*          Open( const char *path, const char *mode ) = 0;
    virtual bool            Exists( const char *path ) = 0;
    virtual bool            Delete( const char *path ) = 0;
    virtual bool            Copy( const char *src, const char *dst ) = 0;
    virtual bool            Rename( const char *src, const char *dst ) = 0;
    virtual size_t          Size( const char *path ) = 0;
    virtual bool            Stat( const char *path, struct stat *stats ) = 0;

    virtual bool            GetFullPathTree( const char *path, std::vector <std::string>& tree, bool *file ) = 0;
    virtual bool            GetRelativePathTree( const char *path, std::vector <std::string>& tree, bool *file ) = 0;
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