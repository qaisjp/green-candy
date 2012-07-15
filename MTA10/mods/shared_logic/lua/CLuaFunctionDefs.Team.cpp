/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.0
*               (Shared logic for modifications)
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        mods/shared_logic/lua/CLuaFunctionDefs.Team.cpp
*  PURPOSE:     Lua function definitions class
*  DEVELOPERS:  Ed Lyons <eai@opencoding.net>
*               Jax <>
*               Cecill Etheredge <ijsf@gmx.net>
*               Kevin Whiteside <kevuwk@gmail.com>
*               Chris McArthur <>
*               Derek Abdine <>
*               Christian Myhre Lundheim <>
*               Stanislav Bobrov <lil_toady@hotmail.com>
*               Alberto Alonso <rydencillo@gmail.com>
*
*****************************************************************************/

#include "StdInc.h"

namespace CLuaFunctionDefs
{
    LUA_DECLARE( getTeamFromName )
    {
        if ( lua_type ( L, 1 ) == LUA_TSTRING )
        {
            const char* szTeamName = lua_tostring ( L, 1 );
            CClientTeam* pTeam = m_pTeamManager->GetTeam ( szTeamName );
            if ( pTeam )
            {
                lua_pushelement ( L, pTeam );
                return 1;
            }
        }
        else
            m_pScriptDebugging->LogBadType( "getTeamFromName" );

        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( getTeamName )
    {
        if ( lua_type ( L, 1 ) == LUA_TLIGHTUSERDATA )
        {
            CClientTeam* pTeam = lua_toteam ( L, 1 );
            if ( pTeam )
            {
                const char* szName = pTeam->GetTeamName ();
                if ( szName )
                {
                    lua_pushstring ( L, szName );
                    return 1;
                }
            }
            else
                m_pScriptDebugging->LogBadPointer( "getTeamName", "team", 1 );
        }
        else
            m_pScriptDebugging->LogBadType( "getTeamName" );

        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( getTeamColor )
    {
        if ( lua_type ( L, 1 ) == LUA_TLIGHTUSERDATA )
        {
            CClientTeam* pTeam = lua_toteam ( L, 1 );
            if ( pTeam )
            {
                unsigned char ucRed, ucGreen, ucBlue;
                pTeam->GetColor ( ucRed, ucGreen, ucBlue );

                lua_pushnumber ( L, ucRed );
                lua_pushnumber ( L, ucGreen );
                lua_pushnumber ( L, ucBlue );
                return 3;
            }
            else
                m_pScriptDebugging->LogBadPointer( "getTeamColor", "team", 1 );
        }
        else
            m_pScriptDebugging->LogBadType( "getTeamColor" );

        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( getTeamFriendlyFire )
    {
        if ( lua_type ( L, 1 ) == LUA_TLIGHTUSERDATA )
        {
            CClientTeam* pTeam = lua_toteam ( L, 1 );
            if ( pTeam )
            {
                bool bFriendlyFire = pTeam->GetFriendlyFire ();
                lua_pushboolean ( L, bFriendlyFire );
                return 1;
            }
            else
                m_pScriptDebugging->LogBadPointer( "getTeamFriendlyFire", "team", 1 );
        }
        else
            m_pScriptDebugging->LogBadType( "getTeamFriendlyFire" );

        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( getPlayersInTeam )
    {
        CLuaMain* pLuaMain = lua_readcontext( L );
        if ( lua_type ( L, 1 ) == LUA_TLIGHTUSERDATA )
        {
            CClientTeam* pTeam = lua_toteam ( L, 1 );
            if ( pTeam )
            {
                lua_newtable ( L );

                unsigned int uiIndex = 0;

                list < CClientPlayer* > ::const_iterator iter = pTeam->IterBegin ();
                for ( ; iter != pTeam->IterEnd () ; iter++ )
                {
                    CClientPlayer* pPlayer = *iter;
                    lua_pushnumber ( L, ++uiIndex );
                    lua_pushelement ( L, pPlayer );
                    lua_settable ( L, -3 );
                }

                return 1;
            }
            else
                m_pScriptDebugging->LogBadPointer( "getPlayersInTeam", "team", 1 );
        }
        else
            m_pScriptDebugging->LogBadType( "getPlayersInTeam" );

        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( countPlayersInTeam )
    {
        if ( lua_type ( L, 1 ) == LUA_TLIGHTUSERDATA )
        {
            CClientTeam* pTeam = lua_toteam ( L, 1 );
            if ( pTeam )
            {
                unsigned int uiCount = pTeam->CountPlayers ();
                lua_pushnumber ( L, uiCount );
                return 1;
            }
            else
                m_pScriptDebugging->LogBadPointer( "countPlayersInTeam", "team", 1 );
        }
        else
            m_pScriptDebugging->LogBadType( "countPlayersInTeam" );

        lua_pushboolean ( L, false );
        return 1;
    }
}