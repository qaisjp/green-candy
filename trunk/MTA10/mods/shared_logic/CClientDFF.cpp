/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*               (Shared logic for modifications)
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        mods/shared_logic/CClientDFF.cpp
*  PURPOSE:     .dff model handling class
*  DEVELOPERS:  Christian Myhre Lundheim <>
*               The_GTA <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#include "StdInc.h"

static const luaL_Reg dff_interface[] =
{
    { NULL, NULL }
};

static int luaconstructor_dff( lua_State *L )
{
    CClientDFF *dff = (CClientDFF*)lua_touserdata( L, lua_upvalueindex( 1 ) );

    ILuaClass& j = *lua_refclass( L, 1 );
    j.SetTransmit( LUACLASS_DFF, dff );

    // Obtain all atomics
    const CModel::atomicList_t& atoms = dff->m_model.GetAtomics();
    CModel::atomicList_t::const_iterator iter = atoms.begin();

    for ( ; iter != atoms.end(); iter++ )
        new CClientAtomic( *dff, dff, **iter );

    lua_pushvalue( L, LUA_ENVIRONINDEX );
    lua_pushvalue( L, lua_upvalueindex( 1 ) );
    luaL_openlib( L, NULL, dff_interface, 1 );

    lua_pushlstring( L, "dff", 3 );
    lua_setfield( L, LUA_ENVIRONINDEX, "__type" );
    return 0;
}

CClientDFF::CClientDFF( LuaClass& root, CModel& model ) : CClientRwObject( root, model ), m_model( model )
{
    lua_State *L = root.GetVM();

    PushStack( L );
    lua_pushlightuserdata( L, this );
    lua_pushcclosure( L, luaconstructor_dff, 1 );
    luaJ_extend( L, -2, 0 );
    lua_pop( L, 1 );
}

CClientDFF::~CClientDFF()
{
    RestreamAll();
}

bool CClientDFF::ReplaceModel( unsigned short id )
{
    if ( !m_model.Replace( id ) )
        return false;

    g_pClientGame->GetManager()->Restream( id );
    return true;
}

bool CClientDFF::HasReplaced( unsigned short id )
{
    return m_model.IsReplaced( id );
}

void CClientDFF::RestoreModel( unsigned short id )
{
    if ( !m_model.Restore( id ) )
        return;

    g_pClientGame->GetManager()->Restream( id );
}

void CClientDFF::RestoreModels()
{
    RestreamAll();

    m_model.RestoreAll();
}

void CClientDFF::RestreamAll() const
{
    std::vector <unsigned short> impList = m_model.GetImportList();
    std::vector <unsigned short>::iterator iter = impList.begin();

    for ( ; iter != impList.end(); iter++ )
        g_pClientGame->GetManager()->Restream( *iter );
}