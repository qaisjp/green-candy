/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.0
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        core/CModuleLoader.cpp
*  PURPOSE:     Dynamic module loading
*  DEVELOPERS:  Derek Abdine <>
*               Alberto Alonso <rydencillo@gmail.com>
*               The_GTA <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#include "StdInc.h"

using std::string;

CModuleLoader::CModuleLoader( const string& ModuleName )
    : m_bStatus( false )
{
    LoadModule( ModuleName );
}

CModuleLoader::CModuleLoader()
    : m_bStatus( false ) 
{
    m_hLoadedModule = NULL;
}
 
CModuleLoader::~CModuleLoader()
{
    UnloadModule();
}

bool CModuleLoader::LoadModule( const filePath& path )
{
    m_hLoadedModule = LoadLibrary( path.c_str() );

    if ( m_hLoadedModule == NULL )
    {
        char szError[2048];

        FormatMessage( FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, GetLastError(), LANG_NEUTRAL, szError, sizeof(szError), NULL );
        
        throw std::exception( szError );
    }

    return m_bStatus;
}

void CModuleLoader::UnloadModule()
{
    if ( !m_hLoadedModule )
        return;

    FreeLibrary( m_hLoadedModule );

    m_hLoadedModule = NULL;
    m_bStatus = false;
    m_strLastError = "";
}

void* CModuleLoader::GetFunctionPointer( const string& FunctionName )
{
    if ( !m_bStatus )
        return NULL;

    return (void*)GetProcAddress( m_hLoadedModule, FunctionName.c_str() );
}