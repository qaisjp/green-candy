/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*               (Shared logic for modifications)
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        mods/shared_logic/CClientAtomic.cpp
*  PURPOSE:     Atomic mesh class
*  DEVELOPERS:  The_GTA <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#include "StdInc.h"

static luaL_Reg atomic_interface[] =
{
    { NULL, NULL }
};

static int luaconstructor_atomic( lua_State *L )
{
    CClientAtomic *atom = (CClientAtomic*)lua_touserdata( L, lua_upvalueindex( 1 ) );

    ILuaClass& j = *lua_refclass( L, 1 );
    j.SetTransmit( LUACLASS_ATOMIC, atom );

    lua_pushvalue( L, LUA_ENVIRONINDEX );
    lua_pushvalue( L, lua_upvalueindex( 1 ) );
    luaL_openlib( L, NULL, atomic_interface, 1 );

    lua_pushlstring( L, "atomic", 6 );
    lua_setfield( L, LUA_ENVIRONINDEX, "__type" );
    return 0;
}

CClientAtomic::CClientAtomic( LuaClass& root, CClientDFF *model, CRpAtomic& atom ) : CClientRwObject( root, atom ), m_atomic( atom )
{
    // Lua instancing
    lua_State *L = root.GetVM();

    PushStack( L );
    lua_pushlightuserdata( L, this );
    lua_pushcclosure( L, luaconstructor_atomic, 1 );
    luaJ_extend( L, -2, 0 );
    lua_pop( L, 1 );

    m_clump = model;
}

CClientAtomic::~CClientAtomic()
{
    if ( m_clump )
        m_clump->m_atomics.remove( this );
}