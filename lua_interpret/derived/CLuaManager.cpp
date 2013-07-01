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

#undef LUA_REGISTER
#define LUA_REGISTER( L, x ) L->RegisterFunction( #x, x )

static void LoadCFunctions( CLuaMain *L )
{
    lua_State *state = L->GetVirtualMachine();

    luanetbench_open( state );
    luaopen_dxElements( state );
    luawin32_open( state );
    luagl_open( state );
    luafreetype_open( state );
    luabitmap_open( state );
    luadds_open( state );
    luapng_open( state );
    luajpeg_open( state );
    luas2g_open( state );

    lua_register( state, "newmd5hasher", lua_newmd5hasher );

    lua_pushvalue( state, LUA_GLOBALSINDEX );
    luafilesystem_open( state );
    lua_setfield( state, -2, "file" );
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