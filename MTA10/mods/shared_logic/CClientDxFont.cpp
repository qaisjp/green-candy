/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.0
*               (Shared logic for modifications)
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        mods/shared_logic/CClientDxFont.cpp
*  PURPOSE:     Custom font bucket
*  DEVELOPERS:  qwerty
*
*****************************************************************************/

#include <StdInc.h>

static const luaL_Reg dxfont_interface[] =
{
    { NULL, NULL }
};

static int luaconstructor_dxfont( lua_State *L )
{
    CClientDxFont *font = (CClientDxFont*)lua_touserdata( L, lua_upvalueindex( 1 ) );

    ILuaClass& j = *lua_refclass( L, 1 );
    j.SetTransmit( LUACLASS_DXFONT, font );

    lua_pushvalue( L, LUA_ENVIRONINDEX );
    lua_pushvalue( L, lua_upvalueindex( 1 ) );
    luaL_openlib( L, NULL, dxfont_interface, 1 );

    lua_pushlstring( L, "dx-font", 7 );
    lua_setfield( L, LUA_ENVIRONINDEX, "__type" );
    return 0;
}

CClientDxFont::CClientDxFont ( CClientManager* pManager, ElementID ID, LuaClass& root, CDxFontItem* pFontItem ) : CClientRenderElement ( pManager, ID, root )
{
    // Lua instancing
    lua_State *L = root.GetVM();

    PushStack( L );
    lua_pushlightuserdata( L, this );
    lua_pushcclosure( L, luaconstructor_dxfont, 1 );
    luaJ_extend( L, -2, 0 );
    lua_pop( L, 1 );

    SetTypeName ( "dx-font" );
    m_pRenderItem = pFontItem;
}

ID3DXFont* CClientDxFont::GetD3DXFont ( void )
{
    return GetDxFontItem ()->m_pFntNormal;
}
