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

static LUA_DECLARE( setLTM )
{
    RwMatrix *mat;

    LUA_ARGS_BEGIN;
    argStream.ReadClass( mat, LUACLASS_MATRIX );
    LUA_ARGS_END;

    ((CClientAtomic*)lua_touserdata( L, lua_upvalueindex( 1 )))->m_atomic.GetFrame()->SetLTM( *mat );
    LUA_SUCCESS;
}

static LUA_DECLARE( getLTM )
{
    lua_creatematrix( L, ((CClientAtomic*)lua_touserdata( L, lua_upvalueindex( 1 )))->m_atomic.GetFrame()->GetLTM() );
    return 1;
}

static LUA_DECLARE( setModelling )
{
    RwMatrix *mat;

    LUA_ARGS_BEGIN;
    argStream.ReadClass( mat, LUACLASS_MATRIX );
    LUA_ARGS_END;

    ((CClientAtomic*)lua_touserdata( L, lua_upvalueindex( 1 )))->m_atomic.GetFrame()->SetModelling( *mat );
    LUA_SUCCESS;
}

static LUA_DECLARE( getModelling )
{
    lua_creatematrix( L, ((CClientAtomic*)lua_touserdata( L, lua_upvalueindex( 1 )))->m_atomic.GetFrame()->GetModelling() );
    return 1;
}

static LUA_DECLARE( replaceModel )
{
    unsigned short model;

    LUA_ARGS_BEGIN;
    argStream.ReadNumber( model );
    LUA_ARGS_END;

    lua_pushboolean( L, ((CClientAtomic*)lua_touserdata( L, lua_upvalueindex( 1 )))->ReplaceModel( model ) );
    return 1;
}

static LUA_DECLARE( isReplaced )
{
    unsigned short model;

    LUA_ARGS_BEGIN;
    argStream.ReadNumber( model );
    LUA_ARGS_END;

    lua_pushboolean( L, ((CClientAtomic*)lua_touserdata( L, lua_upvalueindex( 1 ) ) )->HasReplaced( model ) );
    return 1;
}

static LUA_DECLARE( getReplaced )
{
    lua_settop( L, 0 );

    const CRpAtomic::imports_t& impList = ((CClientAtomic*)lua_touserdata( L, lua_upvalueindex( 1 )))->m_atomic.GetImportList();
    CRpAtomic::imports_t::const_iterator iter = impList.begin();
    int n = 1;

    lua_createtable( L, impList.size(), 0 );

    for ( ; iter != impList.end(); iter++ )
    {
        lua_pushnumber( L, *iter );
        lua_rawseti( L, 1, n++ );
    }

    return 1;
}

static LUA_DECLARE( restoreModel )
{
    unsigned short model;

    LUA_ARGS_BEGIN;
    argStream.ReadNumber( model );
    LUA_ARGS_END;

    ((CClientAtomic*)lua_touserdata( L, lua_upvalueindex( 1 )))->RestoreModel( model );
    LUA_SUCCESS;
}

static LUA_DECLARE( restoreAll )
{
    ((CClientAtomic*)lua_touserdata( L, lua_upvalueindex( 1 )))->RestoreModels();
    return 0;
}

static luaL_Reg atomic_interface[] =
{
    LUA_METHOD( setLTM ),
    LUA_METHOD( getLTM ),
    LUA_METHOD( setModelling ),
    LUA_METHOD( getModelling ),
    LUA_METHOD( replaceModel ),
    LUA_METHOD( isReplaced ),
    LUA_METHOD( getReplaced ),
    LUA_METHOD( restoreModel ),
    LUA_METHOD( restoreAll ),
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

bool CClientAtomic::ReplaceModel( unsigned short id )
{
    if ( !m_atomic.Replace( id ) )
        return false;

    g_pClientGame->GetManager()->Restream( id );
    return true;
}

bool CClientAtomic::HasReplaced( unsigned short id )
{
    return m_atomic.IsReplaced( id );
}

void CClientAtomic::RestoreModel( unsigned short id )
{
    if ( !m_atomic.Restore( id ) )
        return;

    g_pClientGame->GetManager()->Restream( id );
}

void CClientAtomic::RestoreModels()
{
    RestreamAll();

    m_atomic.RestoreAll();
}

void CClientAtomic::RestreamAll() const
{
    std::vector <unsigned short> impList = m_atomic.GetImportList();
    std::vector <unsigned short>::iterator iter = impList.begin();

    for ( ; iter != impList.end(); iter++ )
        g_pClientGame->GetManager()->Restream( *iter );
}