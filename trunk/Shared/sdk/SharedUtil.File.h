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
    SString         PathConform                     ( const SString& strInPath );
    SString         PathJoin                        ( const SString& str1, const SString& str2 );
    SString         PathJoin                        ( const SString& str1, const SString& str2, const SString& str3, const SString& str4 = "", const SString& str5 = "" );
    void            ExtractFilename                 ( const SString& strPathFilename, SString* strPath, SString* strFilename );
    bool            ExtractExtention                ( const SString& strFilename, SString* strRest, SString* strExt );
    SString         ExtractPath                     ( const SString& strPathFilename );
    SString         ExtractFilename                 ( const SString& strPathFilename );
    SString         ExtractExtention                ( const SString& strPathFilename );
    SString         ExtractBeforeExtention          ( const SString& strPathFilename );

    SString         GetCurrentWorkingDirectory      ();
    SString         GetCurrentDirectory             ();
#ifdef _WIN32
    SString         GetWindowsDirectory             ();
#endif
    std::vector <SString> FindFiles                 ( const SString& strMatch, bool bFiles, bool bDirectories );
    SString         MakeUniquePath                  ( const SString& strPathFilename );

    SString         GetSystemLocalAppDataPath       ();
    SString         GetSystemCommonAppDataPath      ();
    SString         GetSystemTempPath               ();
    SString         GetMTADataPath                  ();
    SString         GetMTATempPath                  ();
}
