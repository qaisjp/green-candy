/*****************************************************************************
*
*  PROJECT:     Lua Interpreter
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        derived/CResourceManager.cpp
*  PURPOSE:     Lua resource manager class
*  DEVELOPERS:  Martin Turski <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#include <StdInc.h>

CResourceManager::CResourceManager( CLuaManager& manager ) : ResourceManager( manager.GetVirtualMachine() ), m_luaManager( manager )
{
}

CResourceManager::~CResourceManager()
{
}

static inline void loadscript( const filePath& path, void *ud )
{
    CLuaMain *main = (CLuaMain*)ud;
    lua_State *state = main->GetVirtualMachine();

    filePath relPath;
    fileRoot->GetRelativePath( path, true, relPath );

    std::vector <char> buff;
    fileRoot->ReadToBuffer( path, buff );

    // Zero terminate
    buff.push_back( 0 );

    if ( luaL_loadstring( state, &buff[0] ) != 0 )
    {
        cout << "failed to load library " << relPath << "\n";
        return;
    }

    if ( lua_pcall( state, 0, 0, 0 ) != 0 )
    {
        cout << "failed to run library " << relPath << "\n";
        cout << lua_tostring( state, -1 ) << "\n";

        lua_pop( state, 1 );
        return;
    }

    cout << "init: " << relPath << "\n";
}

CResource* CResourceManager::Load( const std::string& name )
{
    filePath absPath;
    
    if ( !resFileRoot->GetFullPathFromRoot( ( name + '/' ).c_str(), false, absPath ) || absPath.empty() )
        return NULL;

    CFileTranslator *fileRoot = fileSystem->CreateTranslator( absPath.c_str() );

    if ( !fileRoot )
        return NULL;

    CResource *res = new CResource( *m_luaManager.Create( name, *fileRoot ), 0, filePath( name ), *fileRoot );

    // Load all .lua scripts
    fileRoot->ScanDirectory( "/", "*.lua", false, NULL, loadscript, &res->GetVM() );

    m_resources.push_back( res );
    m_resByName[name] = res;
    return res;
}