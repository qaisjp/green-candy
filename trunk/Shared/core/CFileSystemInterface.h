/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        Shared/core/CFileSystemInterface.h
*  PURPOSE:     File management
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#ifndef _CFileSystemInterface_
#define _CFileSystemInterface_

typedef std::vector <filePath> dirTree;

enum eFileException
{
    FILE_STREAM_TERMINATED  // user attempts to perform on a terminated file stream, ie. http timeout
};

class CFile abstract
{
public:
    virtual                 ~CFile()
    {
    }

    virtual	size_t          Read( void *buffer, size_t sElement, unsigned long iNumElements ) = 0;
    virtual	size_t          Write( const void *buffer, size_t sElement, unsigned long iNumElements ) = 0;
    virtual	int             Seek( long iOffset, int iType ) = 0;
    virtual	long            Tell() const = 0;
    virtual	bool            IsEOF() const = 0;
    virtual	bool            Stat( struct stat *stats ) const = 0;
    virtual void            PushStat( const struct stat *stats ) = 0;
    virtual void            SetSeekEnd() = 0;
    virtual	size_t          GetSize() const = 0;
    virtual	void            Flush() = 0;
    virtual const filePath& GetPath() const = 0;
    virtual bool            IsReadable() const = 0;
    virtual bool            IsWriteable() const = 0;

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

    virtual size_t          WriteShort( unsigned short iShort )
    {
        return Write( &iShort, sizeof(short), 1 );
    }

    virtual size_t          WriteByte( unsigned char cByte )
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

    size_t                  WriteString( const std::string& input )
    {
        return Write( input.c_str(), 1, input.size() );
    }

    bool                    GetString( std::string& output )
    {
        if ( IsEOF() )
            return false;

        do
        {
            unsigned char c = ReadByte();

            if ( !c || c == '\n' )
                return true;

            output += c;
        }
        while ( !IsEOF() );

        return true;
    }

    bool                    GetString( char *buf, const size_t max )
    {
        size_t n = 0;

        if ( max < 2 || IsEOF() )
            return false;

        do
        {
            unsigned char c = ReadByte();

            if ( !c || c == '\n' )
                goto finish;

            buf[n++] = c;

            if ( n == max - 1 )
                goto finish;
        }
        while ( !IsEOF() );

finish:
        buf[n] = '\0';
        return true;
    }

    template <class type>
    bool    ReadStruct( type& buf )
    {
        return Read( &buf, 1, sizeof( type ) ) == sizeof( type );
    }

    template <class type>
    bool    WriteStruct( type& buf )
    {
        return Write( &buf, 1, sizeof( type ) ) == sizeof( type );
    }
};

typedef void (*pathCallback_t)( const filePath& path, void *userdata );

class CFileTranslator abstract
{
public:
    virtual                 ~CFileTranslator()
    {
    }

    virtual bool            WriteData( const char *path, const char *buffer, size_t size ) = 0;
    virtual bool            CreateDir( const char *path ) = 0;
    virtual CFile*          Open( const char *path, const char *mode ) = 0;
    virtual bool            Exists( const char *path ) const = 0;
    virtual bool            Delete( const char *path ) = 0;
    virtual bool            Copy( const char *src, const char *dst ) = 0;
    virtual bool            Rename( const char *src, const char *dst ) = 0;
    virtual size_t          Size( const char *path ) const = 0;
    virtual bool            Stat( const char *path, struct stat *stats ) const = 0;
    virtual bool            ReadToBuffer( const char *path, std::vector <char>& output ) const = 0;

    virtual bool            GetFullPathTreeFromRoot( const char *path, dirTree& tree, bool& file ) const = 0;
    virtual bool            GetFullPathTree( const char *path, dirTree& tree, bool& file ) const = 0;
    virtual bool            GetRelativePathTreeFromRoot( const char *path, dirTree& tree, bool& file ) const = 0;
    virtual bool            GetRelativePathTree( const char *path, dirTree& tree, bool& file ) const = 0;
    virtual bool            GetFullPathFromRoot( const char *path, bool allowFile, filePath& output ) const = 0;
    virtual bool            GetFullPath( const char *path, bool allowFile, filePath& output ) const = 0;
    virtual bool            GetRelativePathFromRoot( const char *path, bool allowFile, filePath& output ) const = 0;
    virtual bool            GetRelativePath( const char *path, bool allowFile, filePath& output ) const = 0;
    virtual bool            ChangeDirectory( const char *path ) = 0;
    virtual void            GetDirectory( filePath& output ) const = 0;

    virtual void            ScanDirectory( const char *directory, const char *wildcard, bool recurse, 
                                pathCallback_t dirCallback, 
                                pathCallback_t fileCallback, 
                                void *userdata ) const = 0;

    virtual void            GetDirectories( const char *path, const char *wildcard, bool recurse, std::vector <filePath>& output ) const = 0;
    virtual void            GetFiles( const char *path, const char *wildcard, bool recurse, std::vector <filePath>& output ) const = 0;
};

class CArchiveTranslator abstract : public virtual CFileTranslator
{
public:
    virtual void            Save() = 0;
};

class CFileSystemInterface
{
public:
    virtual CFileTranslator*    CreateTranslator( const char *path ) = 0;
    virtual CArchiveTranslator* OpenArchive( CFile& file ) = 0;

    virtual CArchiveTranslator* CreateZIPArchive( CFile& file ) = 0;

    // Insecure, use with caution!
    virtual bool                IsDirectory( const char *path ) = 0;
    virtual bool                Exists( const char *path ) = 0;
    virtual size_t              Size( const char *path ) = 0;
    virtual bool                ReadToBuffer( const char *path, std::vector <char>& output ) = 0;
};

namespace FileSystem
{
    template <class t, typename F>
    inline bool MappedReaderReverse( CFile& file, F f )
    {
        t buf;
        long off;

        file.Seek( -(long)sizeof( buf ), SEEK_END );

        do
        {
            size_t readCount = file.Read( &buf, 1, sizeof( buf ) );

            if ( f( buf, readCount, off ) )
            {
                file.Seek( -(long)readCount + off, SEEK_CUR );
                return true;
            }

        } while ( file.Seek( -(long)sizeof( buf ) * 2 + (long)sizeof( buf ) / 2, SEEK_CUR ) == 0 );

        return false;
    }

    inline void StreamCopy( CFile& src, CFile& dst )
    {
        char buf[8096];

        while ( !src.IsEOF() )
        {
            size_t rb = src.Read( buf, 1, sizeof( buf ) );
            dst.Write( buf, 1, rb );
        }

        dst.SetSeekEnd();
    }

    inline void StreamCopyCount( CFile& src, CFile& dst, size_t cnt )
    {
        size_t toRead;
        char buf[8096];

        while ( ( toRead = min( sizeof( buf ), cnt ) ) != 0 )
        {
            size_t rb = src.Read( buf, 1, toRead );

            cnt -= rb;

            dst.Write( buf, 1, rb );
        }

        dst.SetSeekEnd();
    }

    template <class cb>
    inline void StreamParser( CFile& src, CFile& dst, cb& f )
    {
        char buf[8096];
        char outBuf[16192];
        size_t outSize;

        for (;;)
        {
            size_t rb = src.Read( buf, 1, sizeof( buf ) );

            bool eof = src.IsEOF();
            f.prepare( buf, rb, eof );

            for (;;)
            {
                bool cnt = f.parse( outBuf, sizeof( outBuf ), outSize );
                dst.Write( outBuf, 1, outSize );

                if ( !cnt )
                    break;
            }

            if ( eof )
                break;
        }

        dst.SetSeekEnd();
    }

    template <class cb>
    inline void StreamParserCount( CFile& src, CFile& dst, size_t cnt, cb& f )
    {
        char buf[8096];
        char outBuf[16192];
        size_t outSize;
        size_t toRead;

        for (;;)
        {
            bool eof;

            if ( sizeof( buf ) >= cnt )
            {
                eof = true;

                toRead = cnt;
            }
            else
            {
                eof = false;

                cnt -= toRead = sizeof( buf );
            }

            size_t rb = src.Read( buf, 1, toRead );

            f.prepare( buf, rb, eof );

            for (;;)
            {
                bool cnt = f.parse( outBuf, sizeof( outBuf ), outSize );
                dst.Write( outBuf, 1, outSize );

                if ( !cnt )
                    break;
            }

            if ( eof )
                break;
        }
        
        dst.SetSeekEnd();
    }
}

#endif //_CFileSystemInterface_