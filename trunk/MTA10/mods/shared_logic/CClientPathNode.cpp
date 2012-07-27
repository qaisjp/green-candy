/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*               (Shared logic for modifications)
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        mods/shared_logic/CClientPathNode.cpp
*  PURPOSE:     Path node entity class
*  DEVELOPERS:  Christian Myhre Lundheim <>
*               Jax <>
*               The_GTA <quiret@gmx.de>
*
*****************************************************************************/

#include "StdInc.h"

using std::list;

static const luaL_Reg pathnode_interface[] =
{
    { NULL, NULL }
};

static int luaconstructor_pathnode( lua_State *L )
{
    CClientPathNode *node = (CClientPathNode*)lua_touserdata( L, lua_upvalueindex( 1 ) );

    ILuaClass& j = *lua_refclass( L, 1 );
    j.SetTransmit( LUACLASS_PATHNODE, node );

    lua_pushvalue( L, LUA_ENVIRONINDEX );
    lua_pushvalue( L, lua_upvalueindex( 1 ) );
    luaL_openlib( L, NULL, pathnode_interface, 1 );

    lua_basicprotect( L );

    lua_pushlstring( L, "pathnode", 8 );
    lua_setfield( L, LUA_ENVIRONINDEX, "__type" );
    return 0;
}

CClientPathNode::CClientPathNode ( CClientManager* pManager, CVector& vecPosition,
                                   CVector& vecRotation, int iTime, ElementID ID, LuaClass& root, ePathNodeStyle PathStyle,
                                   CClientPathNode* pNextNode ) : CClientEntity( ID, true, root )
{
    // Lua interfacing
    lua_State *L = root.GetVM();

    PushStack( L );
    lua_pushlightuserdata( L, this );
    lua_pushcclosure( L, luaconstructor_pathnode, 1 );
    luaJ_extend( L, -2, 0 );
    lua_pop( L, 1 );

    m_pManager = pManager;
    m_pPathManager = pManager->GetPathManager ();

    m_vecPosition = vecPosition;
    m_vecRotation = vecRotation;
    m_iTime = iTime;
    m_Style = PathStyle;
    m_NextNodeID = INVALID_ELEMENT_ID;
    m_pPreviousNode = NULL;
    m_pNextNode = pNextNode;

    m_pPathManager->AddToList ( this );
}


CClientPathNode::~CClientPathNode ( void )
{
    Unlink ();

    if ( m_pPreviousNode )
        m_pPreviousNode->SetNextNode ( NULL );
    if ( m_pNextNode )
        m_pNextNode->SetPreviousNode ( NULL );
}


void CClientPathNode::Unlink ( void )
{
    m_pPathManager->RemoveFromList ( this );
}


void CClientPathNode::DoPulse ( void )
{
    list < CClientEntity* > ::iterator iter = m_List.begin ();
    for ( ; iter != m_List.end (); iter++ )
    {
        // TEST!
        if ( (*iter)->GetType () == CCLIENTVEHICLE )
        {
            CClientVehicle* pVehicle = static_cast < CClientVehicle* > ( *iter );
            CVector vecMoveSpeed;
            pVehicle->GetMoveSpeed ( vecMoveSpeed );
            vecMoveSpeed.fZ = 0.0f;
            pVehicle->SetMoveSpeed ( vecMoveSpeed );   
        }
        (*iter)->SetPosition ( m_vecPosition );        
    }
}

bool CClientPathNode::IsEntityAttached ( CClientEntity* pEntity )
{
    list < CClientEntity* > ::iterator iter = m_List.begin ();
    for ( ; iter != m_List.end (); iter++ )
    {
        if ( *iter == pEntity )
            return true;
    }

    return false;
}


