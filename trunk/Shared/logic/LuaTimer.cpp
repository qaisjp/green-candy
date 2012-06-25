/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        Shared/logic/LuaTimer.cpp
*  PURPOSE:     Lua timed execution
*  DEVELOPERS:  Oliver Brown <>
*               Christian Myhre Lundheim <>
*               Jax <>
*               Cecill Etheredge <>
*               The_GTA <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#include <StdInc.h>

static int timer_destroy( lua_State *L )
{
    delete (LuaTimer*)lua_touserdata( L, lua_upvalueindex( 1 ) );
}

static const luaL_Reg timer_methods[] =
{
    { "destroy", timer_destroy },
    { NULL, NULL }
};

static int luaconstructor_timer( lua_State *L )
{
    lua_pushvalue( L, LUA_ENVIRONINDEX );
    lua_pushvalue( L, lua_upvalueindex( 1 ) );
    luaL_openlib( L, NULL, timer_methods, 1 );
    return 0;
}

static int _trefget( LuaTimer& timer, lua_State *lua )
{
    lua_pushlightuserdata( lua, &timer );
    lua_pushcclosure( lua, luaconstructor_timer, 1 );
    lua_newclass( lua );
    lua_basicprotect( lua );
    return luaL_ref( lua, LUA_REGISTRYINDEX );
}

LuaTimer::LuaTimer( lua_State *lua, LuaTimerManager *manager, const LuaFunctionRef& ref ) : LuaClass( lua, _trefget( *this, lua ) )
{
    m_ref = ref;
    m_manager = manager;
}

LuaTimer::~LuaTimer()
{
    // Remove it from the internal table
    m_manager->m_list.remove( this );
}

void LuaTimer::Execute( LuaMain *main )
{
    if ( !VERIFY_FUNCTION( m_ref ) )
        return;

    lua_class_reference ref;
    Reference( ref );

    m_args->Call( main, m_ref );

    if ( m_repCount == 1 )
    {
        // Request destruction if not to be repeated anymore
        ref.GetClass()->RequestDestruction();
        return;
    }

    // 0 means infinite repetitions
    if ( m_repCount != 0 )
        m_repCount--;
}

CTickCount LuaTimer::GetTimeLeft()
{
    return m_startTime + m_delay - CTickCount::Now();
}
