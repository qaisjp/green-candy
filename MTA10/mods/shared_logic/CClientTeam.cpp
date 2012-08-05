/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*               (Shared logic for modifications)
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        mods/shared_logic/CClientTeam.cpp
*  PURPOSE:     Team entity class
*  DEVELOPERS:  Jax <>
*               The_GTA <quiret@gmx.de>
*
*****************************************************************************/

#include "StdInc.h"

using std::list;

static const luaL_Reg team_interface[] =
{
    { NULL, NULL }
};

static int luaconstructor_team( lua_State *L )
{
    CClientTeam *team = (CClientTeam*)lua_touserdata( L, lua_upvalueindex( 1 ) );

    ILuaClass& j = *lua_refclass( L, 1 );
    j.SetTransmit( LUACLASS_TEAM, team );

    lua_pushvalue( L, LUA_ENVIRONINDEX );
    lua_pushvalue( L, lua_upvalueindex( 1 ) );
    luaL_openlib( L, NULL, team_interface, 1 );

    lua_pushlstring( L, "team", 4 );
    lua_setfield( L, LUA_ENVIRONINDEX, "__type" );
    return 0;
}

CClientTeam::CClientTeam( CClientManager* pManager, ElementID ID, LuaClass& root, bool system, char* szName, unsigned char ucRed, unsigned char ucGreen, unsigned char ucBlue ) : CClientEntity( ID, system, root )
{
    // Lua instancing
    lua_State *L = root.GetVM();

    PushStack( L );
    lua_pushlightuserdata( L, this );
    lua_pushcclosure( L, luaconstructor_team, 1 );
    luaJ_extend( L, -2, 0 );
    lua_pop( L, 1 );

    m_pManager = pManager;
    m_pTeamManager = pManager->GetTeamManager ();

    m_szTeamName = NULL;

    SetTypeName ( "team" );

    SetTeamName ( szName );
    SetColor ( ucRed, ucGreen, ucBlue );
    SetFriendlyFire ( true );

    m_pTeamManager->AddToList ( this );
}


CClientTeam::~CClientTeam ( void )
{
    RemoveAll ();
    Unlink ();
    delete [] m_szTeamName;
}


void CClientTeam::Unlink ( void )
{
    m_pTeamManager->RemoveFromList ( this );
}


void CClientTeam::AddPlayer ( CClientPlayer* pPlayer, bool bChangePlayer )
{
    m_List.push_back ( pPlayer );
    if ( bChangePlayer )
        pPlayer->SetTeam ( this, false );
}


void CClientTeam::RemovePlayer ( CClientPlayer* pPlayer, bool bChangePlayer )
{
    if ( !m_List.empty() ) m_List.remove ( pPlayer );

    if ( bChangePlayer )
        pPlayer->SetTeam ( NULL, false );
}


void CClientTeam::RemoveAll ( void )
{
    list < CClientPlayer* > ::const_iterator iter = m_List.begin ();
    for ( ; iter != m_List.end (); iter++ )
    {
        (*iter)->SetTeam ( NULL, false );
    }
    m_List.clear ();
}


bool CClientTeam::Exists ( CClientPlayer* pPlayer )
{
    list < CClientPlayer* > ::const_iterator iter = m_List.begin ();
    for ( ; iter != m_List.end (); iter++ )
    {
        if ( *iter == pPlayer )
            return true;
    }

    return false;
}


void CClientTeam::SetTeamName ( char* szName )
{
    delete [] m_szTeamName;
    m_szTeamName = NULL;

    if ( szName )
    {
        m_szTeamName = new char [ strlen ( szName ) + 1 ];
        strcpy ( m_szTeamName, szName );
    }
}


void CClientTeam::GetColor ( unsigned char& ucRed, unsigned char& ucGreen, unsigned char& ucBlue )
{
    ucRed = m_ucRed;
    ucGreen = m_ucGreen;
    ucBlue = m_ucBlue;
}


void CClientTeam::SetColor ( unsigned char ucRed, unsigned char ucGreen, unsigned char ucBlue )
{
    m_ucRed = ucRed;
    m_ucGreen = ucGreen;
    m_ucBlue = ucBlue;
}