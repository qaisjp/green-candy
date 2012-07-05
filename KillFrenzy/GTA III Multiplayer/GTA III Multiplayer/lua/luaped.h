/*****************************************************************************
*
*  PROJECT:     KillFrenzy! 1.0
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        lua/luaped.h
*  PURPOSE:     Game Lua ped
*  DEVELOPERS:  The_GTA <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#ifndef _GAME_PED_
#define _GAME_PED_

#define LUACLASS_PED 12

class CGamePed : public CGameEntity
{
public:
                        CGamePed( lua_State *L, bool system, CPed& ped );
                        ~CGamePed();
};

#endif //_GAME_PED_