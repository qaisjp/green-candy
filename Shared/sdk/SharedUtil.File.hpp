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

#ifdef WIN32
#ifdef MTA_CLIENT

filePath SharedUtil::GetSystemLocalAppDataPath()
{
    char szResult[MAX_PATH] = "";
    SHGetFolderPath( NULL, CSIDL_LOCAL_APPDATA, NULL, 0, szResult );
    return szResult;
}

filePath SharedUtil::GetSystemCommonAppDataPath()
{
    char szResult[MAX_PATH] = "";
    SHGetFolderPath( NULL, CSIDL_COMMON_APPDATA, NULL, 0, szResult );
    return szResult;
}

filePath SharedUtil::GetSystemTempPath()
{
    char szResult[4030] = "";
    GetTempPath( 4000, szResult );
    return szResult;
}

filePath SharedUtil::GetMTADataPath()
{
    filePath path = GetSystemCommonAppDataPath();
    path += "/MTA San Andreas All/";
    path += GetMajorVersionString();
    path += "/";
    return path;
}

filePath SharedUtil::GetMTATempPath()
{
    filePath path = GetSystemTempPath();
    path += "MTA";
    path += GetMajorVersionString();
    path += "/";
    return path;
}
#endif
#endif

#ifdef WIN32

filePath SharedUtil::GetCurrentDirectory()
{
    char szCurDir[1024] = "";
    ::GetCurrentDirectory( sizeof(szCurDir), szCurDir );
    return szCurDir;
}

filePath SharedUtil::GetWindowsDirectory()
{
    char szWinDir[MAX_PATH] = "";
    ::GetWindowsDirectory( szWinDir, sizeof(szWinDir) );
    return szWinDir;
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

void SharedUtil::ExtractFilename ( const SString& in, SString* strPath, SString* strFilename )
{
    if ( !in.Split ( PATH_SEPERATOR, strPath, strFilename, -1 ) )
        if ( strFilename )
            *strFilename = in;
}

bool SharedUtil::ExtractExtention( const SString& in, SString* strMain, SString* strExt )
{
    return in.Split ( ".", strMain, strExt, -1 );
}

SString SharedUtil::ExtractFilename( const SString& in )
{
    SString strFilename;
    ExtractFilename ( in, NULL, &strFilename );
    return strFilename;
}

SString SharedUtil::ExtractPath( const SString& strPathFilename )
{
    SString strPath;
    ExtractFilename ( strPathFilename, &strPath, NULL );
    return strPath;
}

SString SharedUtil::ExtractExtention( const SString& strPathFilename )
{
    SString strExt;
    ExtractExtention ( strPathFilename, NULL, &strExt );
    return strExt;
}

SString SharedUtil::ExtractBeforeExtention( const SString& strPathFilename )
{
    SString strMain;
    ExtractExtention ( strPathFilename, &strMain, NULL );
    return strMain;
}

#ifdef WIN32
filePath SharedUtil::MakeUniquePath( const filePath& path )
{
    unsigned int n = 0;
    const char *ext;
    filePath pre, post;
    filePath strPath = path.GetPath();

    if ( ext = path.GetExtension() )
    {
        pre = strPath;
        post = ".";
        post += ext;
    }
    else
        pre = path;

    filePath strTest = path;

    while ( GetFileAttributes( strTest ) != INVALID_FILE_ATTRIBUTES )
        strTest = SString( "%s_%d%s", pre, n++, post );

    return strTest;
}
#endif

#ifdef WIN32
///////////////////////////////////////////////////////////////
//
// FindFiles
//
// Find all files or directories at a path
// The_GTA: INSECURE! Use the filesystem instead!
//
///////////////////////////////////////////////////////////////
std::vector <filePath> SharedUtil::FindFiles( const filePath& path, bool bFiles, bool bDirectories )
{
    std::vector <filePath> res;
    filePath match = path;

    if ( path.IsDirectory() )
        match += "*";

    WIN32_FIND_DATA findData;
    HANDLE hFind = FindFirstFile( match, &findData );

    if ( hFind == INVALID_HANDLE_VALUE )
        return res;

    do
    {
        if ( ( findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) ? bDirectories : bFiles )
            if ( strcmp( findData.cFileName, "." ) && strcmp( findData.cFileName, ".." ) )
                res.push_back( findData.cFileName );
    }
    while ( FindNextFile( hFind, &findData ) );

    FindClose( hFind );
    return res;
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