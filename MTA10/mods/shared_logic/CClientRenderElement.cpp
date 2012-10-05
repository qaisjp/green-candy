/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*               (Shared logic for modifications)
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        mods/shared_logic/CClientRenderElement.cpp
*  PURPOSE:     Client module's version of something renderable
*  DEVELOPERS:  The_GTA <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#include <StdInc.h>

static const luaL_Reg render_interface[] =
{
    { NULL, NULL }
};

static int luaconstructor_render( lua_State *L )
{
    CClientRenderElement *render = (CClientRenderElement*)lua_touserdata( L, lua_upvalueindex( 1 ) );

    ILuaClass& j = *lua_refclass( L, 1 );
    j.SetTransmit( LUACLASS_RENDERELEMENT, render );

    lua_pushvalue( L, LUA_ENVIRONINDEX );
    lua_pushvalue( L, lua_upvalueindex( 1 ) );
    luaL_openlib( L, NULL, render_interface, 1 );

    lua_pushlstring( L, "render-element", 14 );
    lua_setfield( L, LUA_ENVIRONINDEX, "__type" );
    return 0;
}

CClientRenderElement::CClientRenderElement( CClientManager* pManager, ElementID ID, lua_State *L ) : CClientEntity( ID, false, L )
{
    // Lua instancing
    PushStack( L );
    lua_pushlightuserdata( L, this );
    lua_pushcclosure( L, luaconstructor_render, 1 );
    luaJ_extend( L, -2, 0 );
    lua_pop( L, 1 );

    m_pRenderElementManager = pManager->GetRenderElementManager ();
    bDoneUnlink = false;
    m_pRenderItem = NULL;
}

CClientRenderElement::~CClientRenderElement ( void )
{
    Unlink ();
}

void CClientRenderElement::Unlink ( void )
{
    if ( !bDoneUnlink )
    {
        bDoneUnlink = true;
        m_pRenderElementManager->Remove ( this );
    }
}
