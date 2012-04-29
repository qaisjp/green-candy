/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.0
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        SharedUtil.File.h
*  PURPOSE:
*  DEVELOPERS:  
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#ifdef _WIN32
#define _File_PathCharComp( c1, c2 ) ( tolower( c1 ) == tolower( c2 ) )

struct char_traits_i : public std::char_traits <char>
{
    static bool __CLRCALL_OR_CDECL eq( const char left, const char right )
    {
        return toupper(left) == toupper(right);
    }

    static bool __CLRCALL_OR_CDECL ne( const char left, const char right )
    {
        return toupper(left) != toupper(right);
    }

    static bool __CLRCALL_OR_CDECL lt( const char left, const char right )
    {
        return toupper(left) < toupper(right);
    }

    static int __CLRCALL_OR_CDECL compare( const char *s1, const char *s2, size_t n )
    {
        while ( n-- )
        {
            if ( toupper(*s1) < toupper(*s2) )
                return -1;

            if ( toupper(*s1) > toupper(*s2) )
                return 1;

            ++s1; ++s2;
        }

        return 0;
    }

    static const char* __CLRCALL_OR_CDECL find( const char *s, size_t cnt, char a )
    {
        while ( cnt-- && toupper(*s++) != toupper(a) );

        return s;
    }
};

class filePath : public std::basic_string <char, char_traits_i>
{
    typedef std::basic_string <char, char_traits_i> _baseString;

public:
    bool operator ==( const char *right ) const
    {
        return compare( right ) == 0;
    }

    bool operator ==( const std::string& right ) const
    {
        if ( right.size() != size() )
            return false;

        return compare( 0, size(), right.c_str() ) == 0;
    }
#else
#define _File_PathCharComp( c1, c2 ) ( c1 == c2 )

class filePath : public std::string
{
    typedef std::string _baseString;
public:
#endif
    filePath()
        : _baseString()
    {
    }

    filePath( const filePath& right )
        : _baseString( right.c_str(), right.size() )
    { }

    filePath( const std::string& right )
        : _baseString( right.c_str(), right.size() )
    { }

    filePath( const _baseString& right )
        : _baseString( right )
    { }

    filePath( const char *right, size_t len )
        : _baseString( right, len )
    { }

    filePath( const char *right )
        : _baseString( right )
    { }

    filePath GetFilename() const
    {
        size_t pos = rfind( '/' );

        if ( pos == -1 )
            if ( ( pos = rfind( '\\' ) ) == -1 )
                return filePath( *this );

        return substr( pos + 1, size() );
    }

    filePath GetPath() const
    {
        size_t pos = rfind( '/' );

        if ( pos == -1 )
            if ( ( pos = rfind( '\\' ) ) == -1 )
                return filePath();

        return substr( 0, pos );
    }

    const char* GetExtension() const
    {
        size_t pos = rfind( '.' );
        size_t posSlash;

        if ( pos == -1 )
            return NULL;

        posSlash = rfind( '/' );

        if ( posSlash == -1 )
            if ( ( posSlash = rfind( '\\' ) ) == -1 )
                return c_str() + pos;

        if ( posSlash > pos )
            return NULL;
        
        return c_str() + pos;
    }

    bool IsDirectory() const
    {
        if ( empty() )
            return false;

        return *rbegin() == '/' || *rbegin() == '\\';
    }

    char operator []( int idx ) const
    {
        return _baseString::operator []( idx );
    }

    operator const char* () const
    {
        return c_str();
    }

    const char* operator* () const
    {
        return c_str();
    }

    operator SString () const
    {
        return SString( c_str(), size() );
    }

    filePath operator +( const _baseString& right ) const
    {
        return _baseString( *this ) + right;
    }

    filePath operator +( const std::string& right ) const
    {
        return _baseString( *this ).append( right.c_str(), right.size() );
    }

    filePath operator +( const char *right ) const
    {
        return _baseString( *this ) + right;
    }
};

namespace SharedUtil
{
    std::vector <filePath> FindFiles                ( const filePath& strMatch, bool bFiles, bool bDirectories );

    void            ExtractFilename                 ( const SString& strPathFilename, SString* strPath, SString* strFilename );
    bool            ExtractExtention                ( const SString& strFilename, SString* strRest, SString* strExt );
    SString         ExtractPath                     ( const SString& strPathFilename );
    SString         ExtractFilename                 ( const SString& strPathFilename );
    SString         ExtractExtention                ( const SString& strPathFilename );
    SString         ExtractBeforeExtention          ( const SString& strPathFilename );

    filePath        GetCurrentDirectory             ();
#ifdef _WIN32
    filePath        GetWindowsDirectory             ();
#endif
    filePath        MakeUniquePath                  ( const filePath& strPathFilename );

    filePath        GetSystemLocalAppDataPath       ();
    filePath        GetSystemCommonAppDataPath      ();
    filePath        GetSystemTempPath               ();
    filePath        GetMTADataPath                  ();
    filePath        GetMTATempPath                  ();
}
