/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.0
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        SharedUtil.File.hpp
*  PURPOSE:
*  DEVELOPERS:  
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#ifdef WIN32
    #include "shellapi.h"
    #include "shlobj.h"
#else
    #include <dirent.h>
#endif

//
// Returns true if the file exists
//
bool SharedUtil::FileExists ( const SString& strFilename )
{
#ifdef WIN32
    DWORD dwAtr = GetFileAttributes ( strFilename );
    if ( dwAtr == INVALID_FILE_ATTRIBUTES )
        return false;
    return ( ( dwAtr & FILE_ATTRIBUTE_DIRECTORY) == 0 );     
#else
    struct stat Info;
    if ( stat ( strFilename, &Info ) == -1 )
        return false;
    return !( S_ISDIR ( Info.st_mode ) );
#endif
}


//
// Returns true if the directory exists
//
bool SharedUtil::DirectoryExists ( const SString& strPath )
{
#ifdef WIN32
    DWORD dwAtr = GetFileAttributes ( strPath );
    if ( dwAtr == INVALID_FILE_ATTRIBUTES )
        return false;
    return ( ( dwAtr & FILE_ATTRIBUTE_DIRECTORY) != 0 );     
#else
    struct stat Info;
    if ( stat ( strPath, &Info ) == -1 )
        return false;
    return ( S_ISDIR ( Info.st_mode ) );
#endif
}


bool SharedUtil::FileLoad ( const SString& strFilename, SString& strBuffer, int iMaxSize )
{
    strBuffer = "";
    std::vector < char > buffer;
    if ( !FileLoad ( strFilename, buffer, iMaxSize ) )
        return false;
    if ( buffer.size () )
        strBuffer = std::string ( &buffer.at ( 0 ), buffer.size () );

    return true;
}

bool SharedUtil::FileSave ( const SString& strFilename, const SString& strBuffer, bool bForce )
{
    return FileSave ( strFilename, strBuffer.length () ? &strBuffer.at ( 0 ) : NULL, strBuffer.length (), bForce );
}

bool SharedUtil::FileAppend ( const SString& strFilename, const SString& strBuffer, bool bForce )
{
    return FileAppend ( strFilename, strBuffer.length () ? &strBuffer.at ( 0 ) : NULL, strBuffer.length (), bForce );
}

bool SharedUtil::FileDelete ( const SString& strFilename, bool bForce )
{
#ifdef WIN32
    if ( bForce )
        SetFileAttributes ( strFilename, FILE_ATTRIBUTE_NORMAL );
#endif
    return unlink ( strFilename ) == 0;
}

bool SharedUtil::FileRename ( const SString& strFilenameOld, const SString& strFilenameNew )
{
#ifdef WIN32
    return MoveFile ( strFilenameOld, strFilenameNew ) != 0;
#else
    return rename ( strFilenameOld, strFilenameNew ) == 0;
#endif
}

//
// Load binary data from a file into an array
//
bool SharedUtil::FileLoad ( const SString& strFilename, std::vector < char >& buffer, int iMaxSize )
{
    buffer.clear ();
    // Open
    FILE* fh = fopen ( strFilename, "rb" );
    if ( !fh )
        return false;
    // Get size
    fseek ( fh, 0, SEEK_END );
    int size = ftell ( fh );
    rewind ( fh );

    int bytesRead = 0;
    if ( size > 0 && size < 1e9 )
    {
        size = Min ( size, iMaxSize );
        // Allocate space
        buffer.assign ( size, 0 );
        // Read into buffer
        bytesRead = fread ( &buffer.at ( 0 ), 1, size, fh );
    }
    // Close
    fclose ( fh );
    return bytesRead == size;
}

//
// Get a file size
//
uint SharedUtil::FileSize ( const SString& strFilename  )
{
    // Open
    FILE* fh = fopen ( strFilename, "rb" );
    if ( !fh )
        return 0;
    // Get size
    fseek ( fh, 0, SEEK_END );
    uint size = ftell ( fh );
    // Close
    fclose ( fh );
    return size;
}


SString SharedUtil::PathConform ( const SString& strPath )
{
    // Make slashes the right way and remove duplicates, except for UNC type indicators
#if WIN32
    SString strTemp = strPath.Replace ( "/", PATH_SEPERATOR );
#else
    SString strTemp = strPath.Replace ( "\\", PATH_SEPERATOR );
#endif
    // Remove slash duplicates
    size_t iFirstDoubleSlash = strTemp.find ( PATH_SEPERATOR PATH_SEPERATOR );
    if ( iFirstDoubleSlash == std::string::npos )
        return strTemp;     // No duplicates present

    // If first double slash is not at the start, then treat as a normal duplicate if:
    //      1. It is not preceeded by a colon, or
    //      2. Another single slash is before it
    if ( iFirstDoubleSlash > 0 )
    {
        if ( strTemp.SubStr ( iFirstDoubleSlash - 1, 1 ) != ":" || strTemp.find ( PATH_SEPERATOR ) < iFirstDoubleSlash  )
        {
            // Replace all duplicate slashes
            return strTemp.Replace ( PATH_SEPERATOR PATH_SEPERATOR, PATH_SEPERATOR, true );
        }
    }

    return strTemp.Left ( iFirstDoubleSlash + 1 ) + strTemp.SubStr ( iFirstDoubleSlash + 1 ).Replace ( PATH_SEPERATOR PATH_SEPERATOR, PATH_SEPERATOR, true );
}

SString SharedUtil::PathJoin ( const SString& str1, const SString& str2 )
{
    return PathConform ( str1 + PATH_SEPERATOR + str2 );
}

SString SharedUtil::PathJoin ( const SString& str1, const SString& str2, const SString& str3, const SString& str4, const SString& str5 )
{
    SString strResult = str1 + PATH_SEPERATOR + str2 + PATH_SEPERATOR + str3;
    if ( str4.length () )
       strResult += PATH_SEPERATOR + str4; 
    if ( str5.length () )
       strResult += PATH_SEPERATOR + str5;
    return PathConform ( strResult );
}

// Remove base path from the start of abs path
SString SharedUtil::PathMakeRelative ( const SString& strInBasePath, const SString& strInAbsPath )
{
    SString strBasePath = PathConform ( strInBasePath );
    SString strAbsPath = PathConform ( strInAbsPath );
    if ( strAbsPath.BeginsWithI ( strBasePath ) )
    {
        return strAbsPath.SubStr ( strBasePath.length () ).TrimStart ( PATH_SEPERATOR );
    }
    return strAbsPath;
}



#ifdef WIN32
///////////////////////////////////////////////////////////////
//
// DelTree
//
//
//
///////////////////////////////////////////////////////////////
bool SharedUtil::DelTree ( const SString& strPath, const SString& strInsideHere )
{
    // Safety: Make sure strPath is inside strInsideHere
    if ( strPath.ToLower ().substr ( 0, strInsideHere.length () ) != strInsideHere.ToLower () )
    {
        assert ( 0 );
        return false;
    }

    DWORD dwBufferSize = strPath.length () + 3;
    char *szBuffer = static_cast < char* > ( alloca ( dwBufferSize ) );
    memset ( szBuffer, 0, dwBufferSize );
    strncpy ( szBuffer, strPath, strPath.length () );
    SHFILEOPSTRUCT sfos;

    sfos.hwnd = NULL;
    sfos.wFunc = FO_DELETE;
    sfos.pFrom = szBuffer;
    sfos.pTo = NULL;
    sfos.fFlags = FOF_NOCONFIRMATION | FOF_NOERRORUI | FOF_SILENT | FOF_ALLOWUNDO;

    int status = SHFileOperation(&sfos);
    return status == 0;
}
#endif


///////////////////////////////////////////////////////////////
//
// MkDir
//
// Returns true if the directory is created or already exists
// TODO: Make bTree off option work
//
///////////////////////////////////////////////////////////////
bool SharedUtil::MkDir ( const SString& strInPath, bool bTree )
{
    SString strPath = PathConform ( strInPath );
    MakeSureDirExists ( strPath + PATH_SEPERATOR );
    return DirectoryExists ( strPath );
}

//
// Ensure all directories exist to the file
//
void SharedUtil::MakeSureDirExists ( const SString& strPath )
{
    std::vector < SString > parts;
    PathConform ( strPath ).Split ( PATH_SEPERATOR, parts );

    // Find first dir that already exists
    int idx = parts.size () - 1;
    for ( ; idx >= 0 ; idx-- )
    {
        SString strTemp = SString::Join ( PATH_SEPERATOR, parts, 0, idx );
        if ( DirectoryExists ( strTemp ) )
            break;        
    }

    // Make non existing dirs only
    idx++;
    for ( ; idx < (int)parts.size () ; idx++ )
    {
        SString strTemp = SString::Join ( PATH_SEPERATOR, parts, 0, idx );
        // Call mkdir on this path
        #ifdef WIN32
            mkdir ( strTemp );
        #else
            mkdir ( strTemp ,0775 );
        #endif
    }
}


///////////////////////////////////////////////////////////////
//
// FileCopy
//
// Copies a single file.
//
///////////////////////////////////////////////////////////////
bool SharedUtil::FileCopy ( const SString& strSrc, const SString& strDest, bool bForce )
{
    if ( bForce )
        MakeSureDirExists ( strDest );

#ifdef WIN32
    if ( bForce )
        SetFileAttributes ( strDest, FILE_ATTRIBUTE_NORMAL );
#endif

    FILE* fhSrc = fopen ( strSrc, "rb" );
    if ( !fhSrc )
    {
        return false;
    }

    FILE* fhDst = fopen ( strDest, "wb" );
    if ( !fhDst )
    {
        fclose ( fhSrc );
        return false;
    }

    char cBuffer[16384];
    while ( true )
    {
        size_t dataLength = fread ( cBuffer, 1, 16384, fhSrc );
        if ( dataLength == 0 )
            break;
        fwrite ( cBuffer, 1, dataLength, fhDst );
    }

    fclose ( fhSrc );
    fclose ( fhDst );
    return true;
}


#ifdef WIN32
///////////////////////////////////////////////////////////////
//
// GetCurrentWorkingDirectory
//
//
//
///////////////////////////////////////////////////////////////
SString SharedUtil::GetCurrentWorkingDirectory ( void )
{
    char szCurDir [ 1024 ] = "";
    ::GetCurrentDirectory ( sizeof ( szCurDir ), szCurDir );
    return szCurDir;
}

#endif


#ifdef WIN32

#else

std::vector < SString > SharedUtil::FindFiles ( const SString& strMatch, bool bFiles, bool bDirectories )
{
    std::vector < SString > strResult;

    DIR *Dir;
    struct dirent *DirEntry;

    if ( ( Dir = opendir ( strMatch ) ) )
    {
        while ( ( DirEntry = readdir ( Dir ) ) != NULL )
        {
            // Skip dotted entries
            if ( strcmp ( DirEntry->d_name, "." ) && strcmp ( DirEntry->d_name, ".." ) )
            {
                struct stat Info;
                bool bIsDir = false;

                SString strPath = PathJoin ( strMatch, DirEntry->d_name );

                // Determine the file stats
                if ( lstat ( strPath, &Info ) != -1 )
                    bIsDir = S_ISDIR ( Info.st_mode );

                if ( bIsDir ? bDirectories : bFiles )
                    strResult.push_back ( DirEntry->d_name );
            }
        }
    }
    return strResult;
}
#endif

//
// Turns a relative MTASA path i.e. "MTA\file.dat"
// into an absolute MTASA path i.e. "C:\Program Files\MTA San Andreas\MTA\file.dat"
//
SString SharedUtil::CalcMTASAPath ( const SString& strPath )
{
    return PathJoin ( GetMTASABaseDir(), strPath );
}

//
// Save binary data to a file
//
bool SharedUtil::FileSave ( const SString& strFilename, const void* pBuffer, unsigned long ulSize, bool bForce )
{
#ifdef WIN32
    if ( bForce )
        SetFileAttributes ( strFilename, FILE_ATTRIBUTE_NORMAL );
#endif

    if ( bForce )
        MakeSureDirExists ( strFilename );

    FILE* fh = fopen ( strFilename, "wb" );
    if ( !fh )
        return false;

    bool bSaveOk = true;
    if ( ulSize )
        bSaveOk = ( fwrite ( pBuffer, 1, ulSize, fh ) == ulSize );
    fclose ( fh );
    return bSaveOk;
}


//
// Append binary data to a file
//
bool SharedUtil::FileAppend ( const SString& strFilename, const void* pBuffer, unsigned long ulSize, bool bForce )
{
#ifdef WIN32
    if ( bForce )
        SetFileAttributes ( strFilename, FILE_ATTRIBUTE_NORMAL );
#endif

    FILE* fh = fopen ( strFilename, "ab" );
    if ( !fh )
        return false;

    bool bSaveOk = true;
    if ( ulSize )
        bSaveOk = ( fwrite ( pBuffer, 1, ulSize, fh ) == ulSize );
    fclose ( fh );
    return bSaveOk;
}