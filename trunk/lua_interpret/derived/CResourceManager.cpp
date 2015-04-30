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

    if ( lint_loadscript( state, &buff[0], relPath.c_str() ) )
    {
        std::cout << "init: " << relPath << "\n";
    }
}

CResource* CResourceManager::Create( const filePath& absPath, const std::string& name )
{
    CFileTranslator *fileRoot = fileSystem->CreateTranslator( absPath.c_str() );

    if ( !fileRoot )
        return NULL;

    filePath resName( name.c_str(), name.size() );

    CResource *res =
        new CResource(
            this,
            *m_luaManager.Create( name, *fileRoot ),
            0,
            resName,
            *fileRoot
        );

    if ( res )
    {
        // The resource must be made a child of the resource manager.
        bool wasMade = res->SetParent( this );

        assert( wasMade == true );

        m_resources.push_back( res );
        m_resByName[ resName ] = res;
    }
    return res;
}

CResource* CResourceManager::Load( const std::string& name )
{
    filePath absPath;
    
    if ( !resFileRoot->GetFullPathFromRoot( ( name + '/' ).c_str(), false, absPath ) || absPath.empty() )
        return NULL;

    CResource *res = Create( absPath, name );

    if ( !res )
        return NULL;

    // Load all .lua scripts
    res->m_fileRoot.ScanDirectory( "/", "*.lua", false, NULL, loadscript, &res->GetVM() );
    return res;
}