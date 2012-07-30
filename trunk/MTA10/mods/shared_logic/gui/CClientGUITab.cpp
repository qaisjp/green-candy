/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*               (Shared logic for modifications)
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        mods/shared_logic/gui/CClientGUITab.cpp
*  PURPOSE:     GUI tab extension
*  DEVELOPERS:  The_GTA <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#include <StdInc.h>

static const luaL_Reg tab_interface[] =
{
    { NULL, NULL }
};

int luaconstructor_guitab( lua_State *L )
{
    CClientGUIElement *gui = (CClientGUIElement*)lua_touserdata( L, lua_upvalueindex( 1 ) );

    ILuaClass& j = *lua_refclass( L, 1 );
    j.SetTransmit( LUACLASS_GUITAB, gui );

    lua_pushvalue( L, LUA_ENVIRONINDEX );
    lua_pushvalue( L, lua_upvalueindex( 1 ) );
    lua_pushlightuserdata( L, gui->GetCGUIElement() );
    luaL_openlib( L, NULL, tab_interface, 2 );

    lua_basicprotect( L );

    lua_pushlstring( L, "gui-tab", 7 );
    lua_setfield( L, LUA_ENVIRONINDEX, "__type" );
    return 0;
}