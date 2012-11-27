/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*               (Shared logic for modifications)
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        mods/shared_logic/CClientTXD.cpp
*  PURPOSE:     TXD manager class
*  DEVELOPERS:  Christian Myhre Lundheim <>
*               Cecill Etheredge <ijsf@gmx.net>
*               arc_
*               The_GTA <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#include <StdInc.h>

typedef std::list <unsigned short> imports_t;

static LUA_DECLARE( setGlobalEmitter )
{
    ((CClientTXD*)lua_touserdata( L, lua_upvalueindex( 1 ) ))->m_txd.SetGlobalEmitter();
    return 0;
}

static const luaL_Reg txd_interface[] =
{
    LUA_METHOD( setGlobalEmitter ),
    { NULL, NULL }
};

typedef std::list <CTexture*> textures_t;

static int luaconstructor_txd( lua_State *L )
{
    CClientTXD *txd = (CClientTXD*)lua_touserdata( L, lua_upvalueindex( 1 ) );

    ILuaClass& j = *lua_refclass( L, 1 );
    j.SetTransmit( LUACLASS_TXD, txd );

    // Create game textures
    textures_t& tex = txd->m_txd.GetTextures();
    textures_t::iterator iter = tex.begin();

    for ( ; iter != tex.end(); iter++ )
        ( new CClientGameTexture( L, **iter ) )->SetTXD( txd );

    lua_pushvalue( L, LUA_ENVIRONINDEX );
    lua_pushvalue( L, lua_upvalueindex( 1 ) );
    luaL_openlib( L, NULL, txd_interface, 1 );

    lua_pushlstring( L, "txd", 3 );
    lua_setfield( L, LUA_ENVIRONINDEX, "__type" );
    return 0;
}

CClientTXD::CClientTXD( lua_State *L, CTexDictionary& txd ) : CClientRwObject( L, txd ), m_txd( txd )
{
    // Lua instancing
    PushStack( L );
    lua_pushlightuserdata( L, this );
    lua_pushcclosure( L, luaconstructor_txd, 1 );
    luaJ_extend( L, -2, 0 );
    lua_pop( L, 1 );
}

CClientTXD::~CClientTXD()
{
    while ( !LIST_EMPTY( m_textures.root ) )
    {
        CClientGameTexture *item = LIST_GETITEM( CClientGameTexture, m_textures.root.next, m_textures );

        if ( item->GetRefCount() != 0 )
            item->RemoveFromTXD();  // The texture might be used somewhere, so we unlink it
        else
            item->Destroy();
    }

    delete &m_txd;
}

bool CClientTXD::Import( unsigned short id )
{
    if ( !m_txd.Import( id ) )
        return false;

    g_pClientGame->GetManager()->Restream( id );
    return true;
}

bool CClientTXD::IsImported( unsigned short id ) const
{
    return m_txd.IsImported( id );
}

void CClientTXD::Remove( unsigned short id )
{
    if ( !m_txd.Remove( id ) )
        return;

    g_pClientGame->GetManager()->Restream( id );
}

void CClientTXD::RemoveAll()
{
    imports_t imports = m_txd.GetImportedList();
    imports_t::const_iterator iter = imports.begin();

    m_txd.ClearImports();

    for ( ; iter != imports.end(); iter++ )
        g_pClientGame->GetManager()->Restream( *iter );
}
