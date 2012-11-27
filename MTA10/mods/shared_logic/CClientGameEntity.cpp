/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*               (Shared logic for modifications)
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        mods/shared_logic/CClientGameEntity.cpp
*  PURPOSE:     Game rendered entity instance
*  DEVELOPERS:  The_GTA <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#include <StdInc.h>

static LUA_DECLARE( setColModel )
{
    CClientColModel *col;

    LUA_ARGS_BEGIN;
    argStream.ReadClass( col, LUACLASS_COLMODEL, NULL );
    LUA_ARGS_END;

    ((CClientGameEntity*)lua_touserdata( L, lua_upvalueindex( 1 ) ))->SetColModel( col );
    LUA_SUCCESS;
}

static LUA_DECLARE( getColModel )
{
    CClientColModel *col = ((CClientGameEntity*)lua_touserdata( L, lua_upvalueindex( 1 ) ))->GetColModel();

    if ( !col )
        return 0;

    col->PushStack( L );
    return 1;
}

static luaL_Reg gameentity_interface[] =
{
    LUA_METHOD( setColModel ),
    LUA_METHOD( getColModel ),
    { NULL, NULL }
};

static LUA_DECLARE( luaconstructor_gameentity )
{
    CClientGameEntity *entity = (CClientGameEntity*)lua_touserdata( L, lua_upvalueindex( 1 ) );

    ILuaClass& j = *lua_refclass( L, 1 );
    j.SetTransmit( LUACLASS_GAMEENTITY, entity );

    lua_pushvalue( L, LUA_ENVIRONINDEX );

    lua_pushvalue( L, lua_upvalueindex( 1 ) );
    luaL_openlib( L, NULL, gameentity_interface, 1 );

    lua_pushlstring( L, "gameentity", 10 );
    lua_setfield( L, LUA_ENVIRONINDEX, "__type" );
    return 0;
}

CClientGameEntity::CClientGameEntity( CClientStreamer *pStreamer, ElementID ID, lua_State *L, bool system ) : CClientStreamElement( pStreamer, ID, L, system )
{
    // Lua instancing
    PushStack( L );
    lua_pushlightuserdata( L, this );
    lua_pushcclosure( L, luaconstructor_gameentity, 1 );
    luaJ_extend( L, -2, 0 );
    lua_pop( L, 1 );

    m_colModel = NULL;
}

CClientGameEntity::~CClientGameEntity()
{
    SetColModel( NULL );
}

void CClientGameEntity::SetColModel( CClientColModel *col )
{
    if ( col == m_colModel )
        return;

    CEntity *entity = GetGameEntity();

    if ( m_colModel )
    {
        if ( entity )
            entity->SetColModel( NULL );

        m_colModel->DecrementMethodStack();
        m_colModel = NULL;
    }

    if ( !col )
        return;

    m_colModel = col;
    col->IncrementMethodStack();

    if ( entity )
        entity->SetColModel( col->m_pColModel );
}

void CClientGameEntity::NotifyCreate()
{
    CEntity *entity = GetGameEntity();

    // Apply collision if present
    if ( m_colModel )
        entity->SetColModel( m_colModel->m_pColModel );

    CClientStreamElement::NotifyCreate();
}