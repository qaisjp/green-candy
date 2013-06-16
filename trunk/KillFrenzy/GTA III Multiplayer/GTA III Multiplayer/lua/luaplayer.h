/*****************************************************************************
*
*  PROJECT:     KillFrenzy! 1.0
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        lua/luaplayer.h
*  PURPOSE:     Game Lua player
*  DEVELOPERS:  The_GTA <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#ifndef _GAME_PLAYER_
#define _GAME_PLAYER_

#define LUACLASS_PLAYER 13

class CGamePlayer : public CGamePed
{
public:
                            CGamePlayer( lua_State *L, CPed& player );
                            ~CGamePlayer();
};

#endif //_GAME_PLAYER_