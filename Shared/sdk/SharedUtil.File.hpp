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

#ifdef WIN32
#ifdef MTA_CLIENT

SString SharedUtil::GetSystemLocalAppDataPath ( void )
{
    char szResult[MAX_PATH] = "";
    SHGetFolderPath( NULL, CSIDL_LOCAL_APPDATA, NULL, 0, szResult );
    return szResult;
}

SString SharedUtil::GetSystemCommonAppDataPath ( void )
{
    char szResult[MAX_PATH] = "";
    SHGetFolderPath( NULL, CSIDL_COMMON_APPDATA, NULL, 0, szResult );
    return szResult;
}

SString SharedUtil::GetSystemTempPath ( void )
{
    char szResult[4030] = "";
    GetTempPath( 4000, szResult );
    return szResult;
}

SString SharedUtil::GetMTADataPath ( void )
{
    return PathJoin ( GetSystemCommonAppDataPath(), "MTA San Andreas All", GetMajorVersionString () );
}

SString SharedUtil::GetMTATempPath ( void )
{
    return PathJoin ( GetSystemTempPath(), "MTA" + GetMajorVersionString () );
}
#endif
#endif



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

#ifdef WIN32
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
    MakeSureDirExists ( strPath + "\\" );
    return FindFiles ( strPath, false, true ).size () > 0;
}
#endif


#ifdef WIN32

///////////////////////////////////////////////////////////////
//
// GetCurrentDirectory
//
//
//
///////////////////////////////////////////////////////////////
SString SharedUtil::GetCurrentDirectory ( void )
{
    char szCurDir [ 1024 ] = "";
    ::GetCurrentDirectory ( sizeof ( szCurDir ), szCurDir );
    return szCurDir;
}

///////////////////////////////////////////////////////////////
//
// GetWindowsDirectory
//
//
//
///////////////////////////////////////////////////////////////
SString SharedUtil::GetWindowsDirectory ( void )
{
    char szWinDir [ MAX_PATH ] = "";
    ::GetWindowsDirectory ( szWinDir, sizeof ( szWinDir ) );
    return szWinDir;
}
#endif


#ifdef WIN32
///////////////////////////////////////////////////////////////
//
// FindFiles
//
// Find all files or directories at a path
//
///////////////////////////////////////////////////////////////
std::vector < SString > SharedUtil::FindFiles ( const SString& strInMatch, bool bFiles, bool bDirectories )
{
    std::vector < SString > strResult;

    SString strMatch = PathConform ( strInMatch );
    if ( strMatch.Right ( 1 ) == PATH_SEPERATOR )
        strMatch += "*";

    WIN32_FIND_DATA findData;
    HANDLE hFind = FindFirstFile ( strMatch, &findData );
    if( hFind != INVALID_HANDLE_VALUE )
    {
        do
        {
            if ( ( findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) ? bDirectories : bFiles )
                if ( strcmp ( findData.cFileName, "." ) && strcmp ( findData.cFileName, ".." ) )
                    strResult.push_back ( findData.cFileName );
        }
        while( FindNextFile( hFind, &findData ) );
        FindClose( hFind );
    }
    return strResult;
}

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

void SharedUtil::ExtractFilename ( const SString& strPathFilename, SString* strPath, SString* strFilename )
{
    if ( !strPathFilename.Split ( PATH_SEPERATOR, strPath, strFilename, -1 ) )
        if ( strFilename )
            *strFilename = strPathFilename;
}

bool SharedUtil::ExtractExtention ( const SString& strFilename, SString* strMain, SString* strExt )
{
    return strFilename.Split ( ".", strMain, strExt, -1 );
}

SString SharedUtil::ExtractPath ( const SString& strPathFilename )
{
    SString strPath;
    ExtractFilename ( strPathFilename, &strPath, NULL );
    return strPath;
}

SString SharedUtil::ExtractFilename ( const SString& strPathFilename )
{
    SString strFilename;
    ExtractFilename ( strPathFilename, NULL, &strFilename );
    return strFilename;
}

SString SharedUtil::ExtractExtention ( const SString& strPathFilename )
{
    SString strExt;
    ExtractExtention ( strPathFilename, NULL, &strExt );
    return strExt;
}

SString SharedUtil::ExtractBeforeExtention ( const SString& strPathFilename )
{
    SString strMain;
    ExtractExtention ( strPathFilename, &strMain, NULL );
    return strMain;
}

#ifdef WIN32
SString SharedUtil::MakeUniquePath ( const SString& strPathFilename )
{
    SString strBeforeUniqueChar, strAfterUniqueChar;

    SString strPath, strFilename;
    ExtractFilename ( strPathFilename, &strPath, &strFilename );

    SString strMain, strExt;
    if ( ExtractExtention ( strFilename, &strMain, &strExt ) )
    {
        strBeforeUniqueChar = PathJoin ( strPath, strMain );
        strAfterUniqueChar = "." + strExt;
    }
    else
    {
        strBeforeUniqueChar = strPathFilename;
        strAfterUniqueChar = "";
    }

    SString strTest = strPathFilename;
    int iCount = 1;
    while ( GetFileAttributes ( strTest ) != INVALID_FILE_ATTRIBUTES )
    {
        strTest = SString ( "%s_%d%s", strBeforeUniqueChar.c_str (), iCount++, strAfterUniqueChar.c_str () );
    }
    return strTest;
}
#endif
