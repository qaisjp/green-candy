/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.0
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        core/CFilePathTranslator.cpp
*  PURPOSE:     Easy interface to file paths
*  DEVELOPERS:  Cecill Etheredge <ijsf@gmx.net>
*               Derek Abdine <>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#include "StdInc.h"

using std::string;

void CFilePathTranslator::GetGTARootDirectory ( string & ModuleRootDirOut )
{
    HMODULE     hMainModule;
    char        szCurrentDir [ 512 ];

    // First, we get the handle to the root module (exe)
    hMainModule = GetModuleHandle ( NULL );

    // Next, we get the full path of the module.
    GetModuleFileName ( hMainModule, szCurrentDir, sizeof ( szCurrentDir ) );
    
    // Strip the module name out of the path.
    PathRemoveFileSpec ( szCurrentDir );

    ModuleRootDirOut = szCurrentDir;
}


void CFilePathTranslator::GetMTASARootDirectory ( string & InstallRootDirOut )
{
    InstallRootDirOut = GetMTASABaseDir ();
}


