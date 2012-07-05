/*****************************************************************************
*
*  PROJECT:     KillFrenzy! 1.0
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        lua/luaplayer.cpp
*  PURPOSE:     Game Lua player
*  DEVELOPERS:  The_GTA <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#include <StdInc.h>

CGamePlayer::CGamePlayer( lua_State *L, CPed& player ) : CGamePed( L, true, player )
{
}

CGamePlayer::~CGamePlayer()
{
}