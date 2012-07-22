/*****************************************************************************
*
*  PROJECT:     KillFrenzy! 1.0
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        lua/luavehicle.h
*  PURPOSE:     Game Lua vehicle
*  DEVELOPERS:  The_GTA <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#ifndef _GAME_VEHICLE_
#define _GAME_VEHICLE_

#define LUACLASS_VEHICLE    11

class CGameVehicle : public CGameEntity
{
public:
                            CGameVehicle( LuaClass& root, bool system, CVehicle& entity );
                            ~CGameVehicle();

    inline CVehicle&        GetEntity()     { return (CVehicle&)m_entity; }
};

#endif //_GAME_VEHICLE_