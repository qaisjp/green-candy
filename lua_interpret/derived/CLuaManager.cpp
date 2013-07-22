/*****************************************************************************
*
*  PROJECT:     Lua Interpreter
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        derived/CLuaManager.cpp
*  PURPOSE:     Lua hyperstructure manager
*  DEVELOPERS:  Martin Turski <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#include <StdInc.h>

CLuaManager::CLuaManager( CEvents& events ) : m_debug( *this ), LuaManager( events, m_debug )
{
}

CLuaManager::~CLuaManager()
{
}

static int lua_newmd5hasher( lua_State *L )
{
    CMD5Hasher *hasher = new CMD5Hasher();
    hasher->Init();

    lua_createmd5hasher( L, hasher );
    return 1;
} 

void Lua_RegisterPublicFunctions( lua_State *L )
{
    luanetbench_open( L );
    luaopen_dxElements( L );
    luawin32_open( L );
    luagl_open( L );
    luafreetype_open( L );
    luabitmap_open( L );
    luadds_open( L );
    luapng_open( L );
    luajpeg_open( L );
    luas2g_open( L );

    lua_register( L, "newmd5hasher", lua_newmd5hasher );

    lua_pushvalue( L, LUA_GLOBALSINDEX );
    luafilesystem_open( L );
    lua_setfield( L, -2, "file" );
}

#undef LUA_REGISTER
#define LUA_REGISTER( L, x ) L->RegisterFunction( #x, x )

static void LoadCFunctions( CLuaMain *L )
{
    lua_State *state = L->GetVirtualMachine();

    Lua_RegisterPublicFunctions( state );

    // Register all Lua Interpreter functions
    using namespace CLuaFunctionDefs;

    LUA_REGISTER( L, newResource );
}

CLuaMain* CLuaManager::Create( const std::string& name, CFileTranslator& fileRoot )
{
    CLuaMain& main = *new CLuaMain( *this, fileRoot );
    main.m_name = name;

    Init( &main );
    LoadCFunctions( &main );

    // Add special globals

    m_structures.push_back( &main );
    return &main;
}

bool CLuaManager::Remove( LuaMain *lua )
{
    if ( !LuaManager::Remove( lua ) )
        return false;

    return true;
}