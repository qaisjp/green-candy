/*****************************************************************************
*
*  PROJECT:     KillFrenzy! 1.0
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        derived/CResourceManager.cpp
*  PURPOSE:     Extension package management
*  DEVELOPERS:  The_GTA <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#include <StdInc.h>

CResourceManager::CResourceManager( CEvents *events, CLuaManager& manager ) : ResourceManager( manager.GetVirtualMachine() ), m_luaManager( manager )
{
}

CResourceManager::~CResourceManager()
{
}

static inline void loadscript( const filePath& path, void *ud )
{
    CLuaMain *main = (CLuaMain*)ud;
    std::vector <char> buf;

    if ( !main->m_fileRoot.ReadToBuffer( path.c_str(), buf ) )
        return;

    if ( buf.size() == 0 )
        return;

    main->LoadScriptFromBuffer( &buf[0], buf.size(), path.c_str(), false );
}

CResource* CResourceManager::Load( const std::string& name )
{
    filePath absPath;
    
    if ( !resFileRoot->GetFullPathFromRoot( ( name + '/' ).c_str(), false, absPath ) || absPath.empty() )
        return NULL;

    CFileTranslator *fileRoot = fileSystem->CreateTranslator( absPath.c_str() );

    CResource *res = new CResource( *m_luaManager.Create( name, *fileRoot ), filePath( name ), *fileRoot );

    // Load all .lua scripts
    fileRoot->ScanDirectory( "/", "*.lua", false, NULL, loadscript, &res->GetVM() );

    m_resources.push_back( res );
    m_resByName[name] = res;
    return res;
}