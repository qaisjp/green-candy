/*****************************************************************************
*
*  PROJECT:     KillFrenzy! 1.0
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        lua/luavehicle.cpp
*  PURPOSE:     Game Lua vehicle
*  DEVELOPERS:  The_GTA <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#include <StdInc.h>

static int vehicle_destroy( lua_State *L )
{
    return 0;
}

static const luaL_Reg vehicle_interface[] =
{
    { "destroy", vehicle_destroy },
    { NULL, NULL }
};

static int vehicle_constructor( lua_State *L )
{
    CGameVehicle *veh = (CGameVehicle*)lua_touserdata( L, lua_upvalueindex( 1 ) );

    ILuaClass& j = *lua_refclass( L, 1 );
    j.SetTransmit( LUACLASS_VEHICLE, veh );

    lua_pushvalue( L, LUA_ENVIRONINDEX );
    lua_pushvalue( L, lua_upvalueindex( 1 ) );
    luaL_openlib( L, NULL, vehicle_interface, 1 );

    lua_pushlstring( L, "vehicle", 7 );
    lua_setfield( L, LUA_ENVIRONINDEX, "__type" );
    return 0;
}

CGameVehicle::CGameVehicle( LuaClass& root, bool system, CVehicle& veh ) : CGameEntity( root, system, veh )
{
    lua_State *L = root.GetVM();

    PushStack( L );
    lua_pushlightuserdata( L, this );
    lua_pushcclosure( L, vehicle_constructor, 1 );
    luaJ_extend( L, -2, 0 );
}

CGameVehicle::~CGameVehicle()
{
    GetEntity().Destroy();
}