/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.0
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        SharedUtil.File.h
*  PURPOSE:
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#include <core/CFileSystem.common.h>

namespace SharedUtil
{
    //
    // Returns true if the file/directory exists
    //
    bool            FileExists                      ( const SString& strFilename );
    bool            DirectoryExists                 ( const SString& strPath );

    //
    // Load from a file
    //
    bool            FileLoad                        ( const SString& strFilename, std::vector < char >& buffer, int iMaxSize = 0x7FFFFFFF );
    bool            FileLoad                        ( const SString& strFilename, SString& strBuffer, int iMaxSize = 0x7FFFFFFF );

    //
    // Save to a file
    //
    bool            FileSave                        ( const SString& strFilename, const void* pBuffer, unsigned long ulSize, bool bForce = true );
    bool            FileSave                        ( const SString& strFilename, const SString& strBuffer, bool bForce = true );

    //
    // Append to a file
    //
    bool            FileAppend                      ( const SString& strFilename, const void* pBuffer, unsigned long ulSize, bool bForce = true );
    bool            FileAppend                      ( const SString& strFilename, const SString& strBuffer, bool bForce = true );

    //
    // Get a file size
    //
    uint            FileSize                        ( const SString& strFilename );

    //
    // Get startup directory as saved in the registry by the launcher
    // Used in the Win32 Client only
    //
    SString GetMTASABaseDir ( void );

    std::vector <filePath> FindFiles                ( const filePath& strMatch, bool bFiles, bool bDirectories );

    SString         PathJoin                        ( const SString& str1, const SString& str2 );
    SString         PathJoin                        ( const SString& str1, const SString& str2, const SString& str3, const SString& str4 = "", const SString& str5 = "" );

    bool            MkDir                           ( const SString& strInPath, bool bTree = true );
    SString         PathConform                     ( const SString& strInPath );
    SString         PathMakeRelative                ( const SString& strInBasePath, const SString& strInAbsPath );

    bool            FileDelete                      ( const SString& strFilename, bool bForce = true );
    bool            FileRename                      ( const SString& strFilenameOld, const SString& strFilenameNew );
    bool            DelTree                         ( const SString& strPath, const SString& strInsideHere );
    bool            FileCopy                        ( const SString& strSrc, const SString& strDest, bool bForce = true );

    void            ExtractFilename                 ( const SString& strPathFilename, SString* strPath, SString* strFilename );
    bool            ExtractExtention                ( const SString& strFilename, SString* strRest, SString* strExt );
    SString         ExtractPath                     ( const SString& strPathFilename );
    SString         ExtractFilename                 ( const SString& strPathFilename );
    SString         ExtractExtention                ( const SString& strPathFilename );
    SString         ExtractBeforeExtention          ( const SString& strPathFilename );

    void            MakeSureDirExists               ( const SString& strPath );

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

    SString         GetCurrentWorkingDirectory      ( void );
}
