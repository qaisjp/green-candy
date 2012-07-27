/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*               (Shared logic for modifications)
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        mods/shared_logic/CClientDummy.cpp
*  PURPOSE:     Dummy entity class
*  DEVELOPERS:  Chris McArthur <>
*               Jax <>
*               The_GTA <quiret@gmx.de>
*
*****************************************************************************/

#include "StdInc.h"

static int luaconstructordummy( lua_State *L )
{
    CClientDummy *dummy = (CClientDummy*)lua_touserdata( L, lua_upvalueindex( 1 ) );

    ILuaClass& j = *lua_refclass( L, 1 );
    j.SetTransmit( LUACLASS_DUMMY, dummy );

    lua_pushvalue( L, LUA_ENVIRONINDEX );
    lua_basicprotect( L );

    lua_pushvalue( L, lua_upvalueindex( 2 ) );
    lua_setfield( L, LUA_ENVIRONINDEX, "__type" );
    return 0;
}

CClientDummy::CClientDummy ( CClientManager* pManager, ElementID ID, const char * szTypeName, LuaClass& root, bool system ) : CClientEntity ( ID, system, root )
{
    lua_State *L = root.GetVM();
    
    // Basic lua business
    PushStack( L );
    lua_pushlightuserdata( L, this );
    lua_pushstring( L, szTypeName );
    lua_pushcclosure( L, luaconstructor_dummy, 2 );
    luaJ_extend( L, -2, 0 );
    lua_pop( L, 1 );

    SetTypeName ( szTypeName );

    m_pManager = pManager;
    if ( pManager )
    {
        m_pGroups = pManager->GetGroups ();

        if ( m_pGroups )
        {
            m_pGroups->AddToList ( this );
        }
    }
    else
    {
        m_pGroups = NULL;
    }
}

CClientDummy::~CClientDummy ( void )
{
    Unlink ();
}


void CClientDummy::Unlink ( void )
{
    if ( m_pGroups )
    {
        m_pGroups->RemoveFromList ( this );
    }
}

