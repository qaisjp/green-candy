/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        Shared/core/CFileSystem.Utils.hxx
*  PURPOSE:     FileSystem common utilities
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#ifndef _FILESYSUTILS_
#define _FILESYSUTILS_

// Utility function to create a directory tree vector out of a relative path string.
// Ex.: 'my/path/to/desktop/' -> 'my', 'path', 'to', 'desktop'; file == false
//      'my/path/to/file' -> 'my', 'path', 'to', 'file'; file == true
static inline bool _File_ParseRelativePath( const char *path, dirTree& tree, bool& file )
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

        file = true;
    }
    else
        file = false;

    return true;
}

// Output a path tree into a filePath output.
// This is the reverse of _File_ParseRelativePath.
static void AINLINE _File_OutputPathTree( const dirTree& tree, bool file, filePath& output )
{
    size_t actualDirEntries = tree.size();

    if ( file )
    {
        if ( tree.empty() )
            return;

        actualDirEntries--;
    }

    for ( unsigned int n = 0; n < actualDirEntries; n++ )
    {
        output += tree[n];
        output += '/';
    }

    if ( file )
    {
        output += tree.back();
    }
}

// Get relative path tree nodes
static inline bool _File_ParseRelativeTree( const char *path, const dirTree& root, dirTree& output, bool& file )
{
    dirTree tree;

    if (!_File_ParseRelativePath( path, tree, file ))
        return false;

    if ( file )
    {
        if ( tree.size() <= root.size() )
            return false;
    }
    else if ( tree.size() < root.size() )
        return false;

    dirTree::const_iterator rootIter;
    dirTree::iterator treeIter;

    for ( rootIter = root.begin(), treeIter = tree.begin(); rootIter != root.end(); ++rootIter, ++treeIter )
        if ( *rootIter != *treeIter )
            return false;

    output.insert( output.end(), treeIter, tree.end() );
    return true;
}

extern const filePath _dirBack;

static inline bool _File_ParseDeriviation( const dirTree& curTree, dirTree::const_iterator treeIter, dirTree& output, size_t sizeDiff, bool file )
{
    for ( dirTree::const_iterator rootIter( curTree.begin() ); rootIter != curTree.end(); ++rootIter, ++treeIter, sizeDiff-- )
    {
        if ( !sizeDiff )
        {
            // This cannot fail, as we go down to root at maximum
            for ( ; rootIter != curTree.end(); ++rootIter )
                output.push_back( _dirBack );

            if ( file )
                output.push_back( *treeIter );

            return true;
        }

        if ( *rootIter != *treeIter )
        {
            for ( ; rootIter != curTree.end(); ++rootIter )
                output.push_back( _dirBack );

            break;
        }
    }

    return false;
}

static inline bool _File_ParseDeriviateTreeRoot( const dirTree& pathFind, const dirTree& curTree, dirTree& output, bool file )
{
    dirTree::const_iterator treeIter( pathFind.begin() );

    if ( _File_ParseDeriviation( curTree, treeIter, output, file ? pathFind.size() - 1 : pathFind.size(), file ) )
        return true;

    for ( ; treeIter != pathFind.end(); ++treeIter )
        output.push_back( *treeIter );

    return true;
}

static inline bool _File_ParseDeriviateTree( const char *path, const dirTree& curTree, dirTree& output, bool& file )
{
    dirTree tree = curTree;

    if ( !_File_ParseRelativePath( path, tree, file ) )
        return false;

    return _File_ParseDeriviateTreeRoot( tree, curTree, output, file );
}

// Do the same as above, but derive from current directory
static inline bool _File_ParseRelativeTreeDeriviate( const char *path, const dirTree& root, const dirTree& curTree, dirTree& output, bool& file )
{
    dirTree tree;
    size_t sizeDiff;

    if (!_File_ParseRelativePath( path, tree, file ))
        return false;

    if ( file )
    {
        // File should not count as directory
        if ( tree.size() <= root.size() )
            return false;

        sizeDiff = tree.size() - root.size() - 1;
    }
    else
    {
        if ( tree.size() < root.size() )
            return false;

        sizeDiff = tree.size() - root.size();
    }

    dirTree::const_iterator rootIter;
    dirTree::iterator treeIter;

    for ( rootIter = root.begin(), treeIter = tree.begin(); rootIter != root.end(); ++rootIter, ++treeIter )
        if ( *rootIter != *treeIter )
            return false;

    if ( _File_ParseDeriviation( curTree, treeIter, output, sizeDiff, file ) )
        return true;

    output.insert( output.end(), treeIter, tree.end() );
    return true;
}

inline bool _File_ParseMode( const CFileTranslator& root, const char *path, const char *mode, unsigned int& access, unsigned int& m )
{
    switch( *mode )
    {
    case 'w':
        m = FILE_MODE_CREATE;
        access = FILE_ACCESS_WRITE;
        break;
    case 'r':
        m = FILE_MODE_OPEN;
        access = FILE_ACCESS_READ;
        break;
    case 'a':
        m = root.Exists( path ) ? FILE_MODE_OPEN : FILE_MODE_CREATE;
        access = FILE_ACCESS_WRITE;
        break;
    default:
        return false;
    }

    if ( ( mode[1] == 'b' && mode[2] == '+' ) || mode[1] == '+' )
    {
        access |= FILE_ACCESS_WRITE | FILE_ACCESS_READ;
    }

    return true;
}

// The string must at least have one character length and be zero terminated.
inline bool _File_IgnoreDirectoryScanEntry( const char *name )
{
    return *(unsigned short*)name == 0x002E || (*(unsigned short*)name == 0x2E2E && *(unsigned char*)(name + 2) == 0x00);
}

/*=================================================
    File Pattern Matching

    Produces filePattern_t small bytecode routines
    to effectively pattern-match many entries.
=================================================*/

struct filePatternCommand_t
{
    enum eCommand
    {
        FCMD_STRCMP,
        FCMD_WILDCARD,
        FCMD_REQONE
    };

    eCommand cmd;
};

struct filePatternCommandCompare_t : public filePatternCommand_t
{
    size_t len;
    char string[1];

    filePatternCommandCompare_t( void )
    {
        cmd = filePatternCommand_t::FCMD_STRCMP;
    }

    void* operator new( size_t size, size_t strlen )
    {
        return new char[(size - 1) + strlen];
    }

    void operator delete( void *ptr, size_t strlen )
    {
        delete (char*)ptr;
    }

    void operator delete( void *ptr )
    {
        delete (char*)ptr;
    }
};

struct filePatternCommandWildcard_t : public filePatternCommand_t
{
    size_t len;
    char string[1];

    filePatternCommandWildcard_t( void )
    {
        cmd = filePatternCommand_t::FCMD_WILDCARD;
    }

    void* operator new( size_t size, size_t strlen )
    {
        return new char[(size - 1) + strlen];
    }

    void operator delete( void *ptr, size_t strlen )
    {
        delete (char*)ptr;
    }

    void operator delete( void *ptr )
    {
        delete (char*)ptr;
    }
};

struct filePattern_t
{
    typedef std::vector <filePatternCommand_t*> cmdList_t;
    cmdList_t commands;
};

inline filePatternCommand_t* _File_CreatePatternCommand( filePatternCommand_t::eCommand cmd, const std::vector <char>& tokenBuf )
{
    filePatternCommand_t *outCmd = NULL;
    size_t tokenLen = tokenBuf.size();

    switch( cmd )
    {
    case filePatternCommand_t::FCMD_STRCMP:
        if ( tokenLen != 0 )
        {
            filePatternCommandCompare_t *cmpCmd = new (tokenLen) filePatternCommandCompare_t;
            memcpy( cmpCmd->string, &tokenBuf[0], tokenLen );
            cmpCmd->len = tokenLen;

            outCmd = cmpCmd;
        }
        break;
    case filePatternCommand_t::FCMD_WILDCARD:
        {
            filePatternCommandWildcard_t *wildCmd = new (tokenLen) filePatternCommandWildcard_t;

            if ( tokenLen != 0 )
            {
                memcpy( wildCmd->string, &tokenBuf[0], tokenLen );
            }
            wildCmd->len = tokenLen;

            outCmd = wildCmd;
        }
        break;
    case filePatternCommand_t::FCMD_REQONE:
        outCmd = new filePatternCommand_t;
        outCmd->cmd = filePatternCommand_t::FCMD_REQONE;
        break;
    }

    return outCmd;
}

inline void _File_AddCommandToPattern( filePattern_t *pattern, filePatternCommand_t::eCommand pendCmd, std::vector <char>& tokenBuf )
{
    if ( filePatternCommand_t *cmd = _File_CreatePatternCommand( pendCmd, tokenBuf ) )
    {
        pattern->commands.push_back( cmd );
    }

    tokenBuf.clear();
}

inline filePattern_t* _File_CreatePattern( const char *buf )
{
    std::vector <char> tokenBuf;
    filePatternCommand_t::eCommand pendCmd = filePatternCommand_t::FCMD_STRCMP; // by default, it is string comparison.
    filePattern_t *pattern = new filePattern_t;

    while ( char c = *buf )
    {
        buf++;

        if ( c == '*' )
        {
            _File_AddCommandToPattern( pattern, pendCmd, tokenBuf );

            pendCmd = filePatternCommand_t::FCMD_WILDCARD;
        }
        else if ( c == '?' )
        {
            _File_AddCommandToPattern( pattern, pendCmd, tokenBuf );

            // Directly add our command
            _File_AddCommandToPattern( pattern, filePatternCommand_t::FCMD_REQONE, tokenBuf ),

            // default back to string compare.
            pendCmd = filePatternCommand_t::FCMD_STRCMP;
        }
        else
            tokenBuf.push_back( c );
    }

    _File_AddCommandToPattern( pattern, pendCmd, tokenBuf );

    return pattern;
}

inline bool _File_CompareStrings_Count( const char *primary, const char *secondary, size_t count, bool case_insensitive )
{
    for ( size_t n = 0; n < count; n++ )
    {
        char prim = primary[n];
        char sec = secondary[n];

        bool equal = false;

        if ( case_insensitive )
        {
            equal = ( tolower( prim ) == tolower( sec ) );
        }
        else
        {
            equal = ( prim == sec );
        }

        if ( !equal )
            return false;
    }

    return true;
}

// TODO: make sure path resolution does not depend on this.
inline bool _File_DoesNeedCaseInsensitivity( void )
{
#ifdef FILESYSTEM_FORCE_CASE_INSENSITIVE_GLOB
    return true;
#endif //FILESYSTEM_FORCE_CASE_INSENSITIVE_GLOB

#ifdef _WIN32
    return true;
#endif //_WIN32

    return false;
}

inline const char* _File_strnstr( const char *input, size_t input_len, const char *cookie, size_t cookie_len, size_t& off_find )
{
    if ( input_len < cookie_len )
        return NULL;

    if ( cookie_len == 0 )
    {
        off_find = 0;
        return input;
    }

    bool need_case_insensitive = _File_DoesNeedCaseInsensitivity();

    size_t scanEnd = input_len - cookie_len;

    for ( size_t n = 0; n <= scanEnd; n++ )
    {
        if ( _File_CompareStrings_Count( input + n, cookie, cookie_len, need_case_insensitive ) )
        {
            off_find = n;
            return input + n;
        }
    }

    return NULL;
}

inline bool _File_MatchPattern( const char *input, filePattern_t *pattern )
{
    size_t input_len = strlen( input );

    bool need_case_insensitive = _File_DoesNeedCaseInsensitivity();

    for ( filePattern_t::cmdList_t::const_iterator iter = pattern->commands.begin(); iter != pattern->commands.end(); ++iter )
    {
        filePatternCommand_t *genCmd = *iter;

        switch( genCmd->cmd )
        {
        case filePatternCommand_t::FCMD_STRCMP:
            {
                filePatternCommandCompare_t *cmpCmd = (filePatternCommandCompare_t*)genCmd;
                size_t len = cmpCmd->len;

                if ( len > input_len )
                    return false;

                if ( _File_CompareStrings_Count( input, cmpCmd->string, len, need_case_insensitive ) == false )
                    return false;

                input_len -= len;
                input += len;
            }
            break;
        case filePatternCommand_t::FCMD_WILDCARD:
            {
                filePatternCommandWildcard_t *wildCmd = (filePatternCommandWildcard_t*) genCmd;
                const char *nextPos;
                size_t len = wildCmd->len;
                size_t off_find;

                if ( !( nextPos = _File_strnstr( input, input_len, wildCmd->string, len, off_find ) ) )
                    return false;

                input_len -= ( off_find + len );
                input = nextPos + len;
            }
            break;
        case filePatternCommand_t::FCMD_REQONE:
            if ( input_len == 0 )
                return false;

            input_len--;
            input++;
            break;
        }
    }

    return true;
}

inline void _File_DestroyPattern( filePattern_t *pattern )
{
    for ( filePattern_t::cmdList_t::iterator iter = pattern->commands.begin(); iter != pattern->commands.end(); ++iter )
    {
        delete *iter;
    }

    delete pattern;
}

// Function called when any implementation finds a directory.
inline void _File_OnDirectoryFound( filePattern_t *pattern, const char *entryName, const filePath& absPath, pathCallback_t dirCallback, void *userdata )
{
    bool allowDir = true;

    if ( !fileSystem->m_includeAllDirsInScan )
    {
        if ( _File_MatchPattern( entryName, pattern ) == false )
            allowDir = false;
    }

    if ( allowDir )
    {
        dirCallback( absPath, userdata );
    }
}

// Function for creating an OS native directory.
inline bool _File_CreateDirectory( const filePath& thePath )
{
#ifdef __linux__
    if ( mkdir( osPath, FILE_ACCESS_FLAG ) == 0 )
        return true;

    switch( errno )
    {
    case EEXIST:
    case 0:
        return true;
    }

    return false;
#elif defined(_WIN32)
    // Make sure a file with that name does not exist.
    DWORD attrib = INVALID_FILE_ATTRIBUTES;

    filePath pathToMaybeFile = thePath;
    pathToMaybeFile.resize( pathToMaybeFile.size() - 1 );

    if ( const char *ansiPath = pathToMaybeFile.c_str() )
    {
        attrib = GetFileAttributesA( ansiPath );
    }
    else if ( const wchar_t *widePath = pathToMaybeFile.w_str() )
    {
        attrib = GetFileAttributesW( widePath );
    }

    if ( attrib != INVALID_FILE_ATTRIBUTES )
    {
        if ( !( attrib & FILE_ATTRIBUTE_DIRECTORY ) )
            return false;
    }

    BOOL dirSuccess = FALSE;

    if ( const char *ansiPath = thePath.c_str() )
    {
        dirSuccess = CreateDirectoryA( ansiPath, NULL );
    }
    else if ( const wchar_t *widePath = thePath.w_str() )
    {
        dirSuccess = CreateDirectoryW( widePath, NULL );
    }

    return ( dirSuccess == TRUE || GetLastError() == ERROR_ALREADY_EXISTS );
#else
    return false;
#endif //OS DEPENDANT CODE
}

#endif //_FILESYSUTILS_
