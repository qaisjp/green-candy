/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*               (Shared logic for modifications)
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        mods/shared_logic/CClientColModel.cpp
*  PURPOSE:     Model collision (.col file) entity class
*  DEVELOPERS:  Christian Myhre Lundheim <>
*               The_GTA <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#include "StdInc.h"

static const luaL_Reg colmodel_interface[] =
{
    { NULL, NULL }
};

static int luaconstructor_colmodel( lua_State *L )
{
    CClientColModel *col = (CClientColModel*)lua_touserdata( L, lua_upvalueindex( 1 ) );

    ILuaClass& j = *lua_refclass( L, 1 );
    j.SetTransmit( LUACLASS_COLMODEL, col );

    lua_pushvalue( L, LUA_ENVIRONINDEX );
    lua_pushvalue( L, lua_upvalueindex( 1 ) );
    luaL_openlib( L, NULL, colmodel_interface, 1 );

    lua_pushlstring( L, "colmodel", 8 );
    lua_setfield( L, LUA_ENVIRONINDEX, "__type" );
    return 0;
}

CClientColModel::CClientColModel( lua_State *L ) : LuaElement( L )
{
    // Lua instancing
    PushStack( L );
    lua_pushlightuserdata( L, this );
    lua_pushcclosure( L, luaconstructor_colmodel, 1 );
    luaJ_extend( L, -2, 0 );
    lua_pop( L, 1 );

    m_pColModel = NULL;
}

CClientColModel::~CClientColModel()
{
    delete m_pColModel;
}

bool CClientColModel::LoadCol( CFile *file )
{
    // Not already got a col model?
    if ( m_pColModel )
        return false;

    // Load the collision file
    m_pColModel = g_pGame->GetRenderWare()->ReadCOL( file );

    return m_pColModel != NULL;
}

bool CClientColModel::Replace( unsigned short usModel )
{
    return m_pColModel->Replace( usModel );
}

bool CClientColModel::Restore( unsigned short usModel )
{
    return m_pColModel->Restore( usModel );
}

void CClientColModel::RestoreAll()
{
    m_pColModel->RestoreAll();
}

bool CClientColModel::HasReplaced( unsigned short usModel ) const
{
    return m_pColModel->IsReplaced( usModel );
}