// Lua resource management
#include "StdInc.h"

static CRegisteredCommands *cmds;
static CEvents *events;
CLuaManager *lua_manager;
CResourceManager *resMan;
static CScriptDebugging *debug;

static inline void loadresource( const filePath& path, void *ud )
{
    filePath relPath;
    ResourceManager::resFileRoot->GetRelativePath( path.c_str(), false, relPath );

    // Get the directory slash away
    relPath.pop_back();

    resMan->Load( relPath );
}

void Lua_Init()
{
    filePath resRoot;
    fileRoot->GetRelativePath( "kf/resources/", false, resRoot );

    ResourceManager::resFileRoot = fileSystem->CreateTranslator( resRoot.c_str() );

    // Start the .lua management
    events = new CEvents;
    lua_manager = new CLuaManager( *events );
    resMan = new CResourceManager( events, *lua_manager );
    LuaManager::m_resMan = resMan;
    debug = &lua_manager->GetDebug();
    cmds = &lua_manager->GetCommands();

    // Setup shared dependencies
    LuaFunctionDefs::SetResourceManager( *resMan );
    LuaFunctionDefs::SetRegisteredCommands( *cmds );
    LuaFunctionDefs::SetDebugging( *debug );
    CLuaFunctionDefs::SetResourceManager( *resMan );
    CLuaFunctionDefs::SetRegisteredCommands( *cmds );
    CLuaFunctionDefs::SetDebugging( *debug );
}

void Lua_Start()
{
    // Create environments for every resource
    ResourceManager::resFileRoot->ScanDirectory( "/", "*", false, loadresource, NULL, NULL );
}

void Lua_Frame()
{
    lua_manager->DoPulse();
}

bool Lua_ProcessCommand( const std::string& cmdName, const std::vector <std::string>& args )
{
    Command *cmd = cmds->Get( cmdName.c_str() );

    if ( !cmd )
        return false;

    return cmd->Execute( args );
}

void Lua_Destroy()
{
    lua_manager->Shutdown();

    delete events;
    delete lua_manager;
}