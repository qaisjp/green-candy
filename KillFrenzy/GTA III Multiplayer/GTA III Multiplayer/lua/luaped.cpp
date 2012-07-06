/*****************************************************************************
*
*  PROJECT:     KillFrenzy! 1.0
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        lua/luaped.cpp
*  PURPOSE:     Game Lua ped
*  DEVELOPERS:  The_GTA <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#include <StdInc.h>

static const luaL_Reg ped_interface[] =
{
    { NULL, NULL }
};

static int ped_constructor( lua_State *L )
{
    ILuaClass& j = *lua_refclass( L, 1 );
    j.SetTransmit( LUACLASS_PED );

    lua_pushvalue( L, LUA_ENVIRONINDEX );
    lua_pushvalue( L, lua_upvalueindex( 1 ) );
    luaL_openlib( L, NULL, ped_interface, 1 );

    lua_pushlstring( L, "ped", 3 );
    lua_setfield( L, LUA_ENVIRONINDEX, "__type" );
    return 0;
}

CGamePed::CGamePed( lua_State *L, bool system, CPed& ped ) : CGameEntity( L, system, ped )
{
    PushStack( L );
    lua_pushlightuserdata( L, &ped );
    lua_pushcclosure( L, ped_constructor, 1 );
    luaJ_extend( L, -2, 0 );
}

CGamePed::~CGamePed()
{
    GetEntity().Destroy();
}