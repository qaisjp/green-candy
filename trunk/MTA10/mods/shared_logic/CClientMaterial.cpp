/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*               (Shared logic for modifications)
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        mods/shared_logic/CClientMaterial.cpp
*  PURPOSE:
*  DEVELOPERS:  The_GTA <quiret@gmx.de>
*
*****************************************************************************/

#include <StdInc.h>

static const luaL_Reg material_interface[] =
{
    { NULL, NULL }
};

static int luaconstructor_material( lua_State *L )
{
    CClientMaterial *mat = (CClientMaterial*)lua_touserdata( L, lua_upvalueindex( 1 ) );

    ILuaClass& j = *lua_refclass( L, 1 );
    j.SetTransmit( LUACLASS_MATERIAL, mat );

    lua_pushvalue( L, LUA_ENVIRONINDEX );
    lua_pushvalue( L, lua_upvalueindex( 1 ) );
    luaL_openlib( L, NULL, material_interface, 1 );

    lua_pushlstring( L, "material", 8 );
    lua_setfield( L, LUA_ENVIRONINDEX, "__type" );
    return 0;
}

CClientMaterial::CClientMaterial( CClientManager* pManager, ElementID ID, LuaClass& root ) : CClientRenderElement( pManager, ID, root )
{
    // Lua instancing
    lua_State *L = root.GetVM();

    PushStack( L );
    lua_pushlightuserdata( L, this );
    lua_pushcclosure( L, luaconstructor_material, 1 );
    luaJ_extend( L, -2, 0 );
    lua_pop( L, 1 );

    SetTypeName( "material" );
}