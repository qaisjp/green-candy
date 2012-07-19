/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        derived/CLuaManager.cpp
*  PURPOSE:     Derived lua hyperstructure management
*  DEVELOPERS:  The_GTA <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#include <StdInc.h>

CLuaManager::CLuaManager() : LuaManager( *g_pClientGame->GetEvents(), m_debug ), m_debug( this ), m_commands( *this )
{
}

CLuaManager::~CLuaManager()
{
}

static void LoadCFunctions( lua_State *L )
{
    using namespace CLuaFunctionDefs;


}

CLuaMain* CLuaManager::Create( const std::string& name, CFileTranslator& fileRoot )
{
    CLuaMain& main = *new CLuaMain( *this, fileRoot );
    main.SetName( name );

    Init( &main );
    LoadCFunctions( *main );

    m_structures.push_back( &main );
    return &main;
}

bool CLuaManager::Remove( LuaMain *lua )
{
    if ( !LuaManager::Remove( lua ) )
        return false;

    m_commands.CleanUp( lua );
    return true;
}