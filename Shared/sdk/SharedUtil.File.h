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

namespace SharedUtil
{
    //
    // Ensure all directories exist to the file
    //
    void            MakeSureDirExists               ( const SString& strPath );

    SString         PathConform                     ( const SString& strInPath );
    SString         PathJoin                        ( const SString& str1, const SString& str2 );
    SString         PathJoin                        ( const SString& str1, const SString& str2, const SString& str3, const SString& str4 = "", const SString& str5 = "" );
    void            ExtractFilename                 ( const SString& strPathFilename, SString* strPath, SString* strFilename );
    bool            ExtractExtention                ( const SString& strFilename, SString* strRest, SString* strExt );
    SString         ExtractPath                     ( const SString& strPathFilename );
    SString         ExtractFilename                 ( const SString& strPathFilename );
    SString         ExtractExtention                ( const SString& strPathFilename );
    SString         ExtractBeforeExtention          ( const SString& strPathFilename );

    bool            DelTree                         ( const SString& strPath, const SString& strInsideHere );
    bool            MkDir                           ( const SString& strInPath, bool bTree = true );
    SString         GetCurrentWorkingDirectory      ( void );
    SString         GetCurrentDirectory             ( void );
    SString         GetWindowsDirectory             ( void );
    std::vector < SString > FindFiles               ( const SString& strMatch, bool bFiles, bool bDirectories );
    SString         MakeUniquePath                  ( const SString& strPathFilename );

    SString         GetSystemLocalAppDataPath       ( void );
    SString         GetSystemCommonAppDataPath      ( void );
    SString         GetSystemTempPath               ( void );
    SString         GetMTADataPath                  ( void );
    SString         GetMTATempPath                  ( void );
}
