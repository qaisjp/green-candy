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

CClientColModel::CClientColModel( LuaClass& root ) : LuaElement( root )
{
    // Lua instancing
    lua_State *L = root.GetVM();

    PushStack( L );
    lua_pushlightuserdata( L, this );
    lua_pushcclosure( L, luaconstructor_colmodel, 1 );
    luaJ_extend( L, -2, 0 );
    lua_pop( L, 1 );

    // Add us to DFF manager's list
    m_pColModelManager->AddToList ( this );
}

CClientColModel::~CClientColModel ( void )
{
    // Remove us from DFF manager's list
    m_pColModelManager->RemoveFromList ( this );

    // Unreplace our cols
    RestoreAll ();

    if ( m_pColModel )
        delete m_pColModel;
}

bool CClientColModel::LoadCol( CFile *file )
{
    // Not already got a col model?
    if ( !m_pColModel )
    {
        // Load the collision file
        m_pColModel = g_pGame->GetRenderWare ()->ReadCOL( file );

        // Success if the col model is != NULL
        return m_pColModel != NULL;
    }

    // Failed. Already loaded
    return false;
}

bool CClientColModel::Replace ( unsigned short usModel )
{
    // We have a model loaded?
    if ( m_pColModel )
    {
        // Check if anyone has replaced that model already. If so make it forget that
        // it replaced it. We're the new replacer.
        CClientColModel* pAlready = m_pColModelManager->GetElementThatReplaced ( usModel );
        if ( pAlready )
        {
            pAlready->m_Replaced.remove ( usModel );
        }

        // Replace the collisions
        CModelInfo* pModelInfo = g_pGame->GetModelInfo ( usModel );
        pModelInfo->SetColModel ( m_pColModel );

        // Remember that we replaced it
        m_Replaced.push_back ( usModel );

        // Success
        return true;
    }

    // Failed
    return false;
}

void CClientColModel::Restore ( unsigned short usModel )
{
    // Restore it
    InternalRestore ( usModel );

    // Remove the restored id from the list
    m_Replaced.remove ( usModel );
}

void CClientColModel::RestoreAll ( void )
{
    // Loop through our replaced ids
    std::list < unsigned short > ::iterator iter = m_Replaced.begin ();
    for ( ; iter != m_Replaced.end (); iter++ )
    {
        // Restore this model
        InternalRestore ( *iter );
    }

    // Clear the list
    m_Replaced.clear ();
}

bool CClientColModel::HasReplaced ( unsigned short usModel ) const
{
    // Loop through our replaced ids
    std::list < unsigned short > ::const_iterator iter = m_Replaced.begin ();
    for ( ; iter != m_Replaced.end (); iter++ )
    {
        // Is this the given ID
        if ( *iter == usModel )
        {
            // We have replaced it
            return true;
        }
    }

    // We have not replaced it
    return false;
}

void CClientColModel::InternalRestore ( unsigned short usModel )
{
    // Grab the model info and restore it
    CModelInfo* pModel = g_pGame->GetModelInfo ( usModel );
    pModel->RestoreColModel ();
}