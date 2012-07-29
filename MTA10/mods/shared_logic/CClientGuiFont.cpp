/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*               (Shared logic for modifications)
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        mods/shared_logic/CClientGuiFont.cpp
*  PURPOSE:     Custom font bucket
*  DEVELOPERS:  The_GTA <quiret@gmx.de>
*
*****************************************************************************/

#include <StdInc.h>

static const luaL_Reg guifont_interface[] =
{
    { NULL, NULL }
};

static int luaconstructor_guifont( lua_State *L )
{
    CClientGuiFont *font = (CClientGuiFont*)lua_touserdata( L, lua_upvalueindex( 1 ) );

    ILuaClass& j = *lua_refclass( L, 1 );
    j.SetTransmit( LUACLASS_GUIFONT, font );

    lua_pushvalue( L, LUA_ENVIRONINDEX );
    lua_pushvalue( L, lua_upvalueindex( 1 ) );
    luaL_openlib( L, NULL, guifont_interface, 1 );

    lua_basicprotect( L );

    lua_pushlstring( L, "gui-font", 8 );
    lua_setfield( L, LUA_ENVIRONINDEX, "__type" );
    return 0;
}

CClientGuiFont::CClientGuiFont ( CClientManager* pManager, ElementID ID, CGuiFontItem* pFontItem ) : CClientRenderElement ( pManager, ID )
{
    SetTypeName ( "gui-font" );
    m_pRenderItem = pFontItem;
}

CClientGuiFont::~CClientGuiFont ( void )
{
    Unlink ();
}


////////////////////////////////////////////////////////////////
//
// CClientGuiFont::Unlink
//
// Remove from manager lists
//
////////////////////////////////////////////////////////////////
void CClientGuiFont::Unlink ( void )
{
    // Make sure GUI elements are not using us
    while ( m_GUIElementUserList.size () )
        (*m_GUIElementUserList.begin ())->SetFont ( "", NULL );

    CClientRenderElement::Unlink ();
}


////////////////////////////////////////////////////////////////
//
// CClientGuiFont::GetGUIFontName
//
// Get name CEGUI uses for this custom font
//
////////////////////////////////////////////////////////////////
const SString& CClientGuiFont::GetCEGUIFontName ( void )
{
    return GetGuiFontItem ()->m_strCEGUIFontName;
}


////////////////////////////////////////////////////////////////
//
// CClientGuiFont::NotifyGUIElementAttach
//
// Keep track of GUI elements using this font
//
////////////////////////////////////////////////////////////////
void CClientGuiFont::NotifyGUIElementAttach ( CClientGUIElement* pGUIElement )
{
    assert ( !MapContains ( m_GUIElementUserList, pGUIElement ) );
    MapInsert ( m_GUIElementUserList, pGUIElement );
}


////////////////////////////////////////////////////////////////
//
// CClientGuiFont::NotifyGUIElementDetach
//
// Keep track of GUI elements using this font
//
////////////////////////////////////////////////////////////////
void CClientGuiFont::NotifyGUIElementDetach ( CClientGUIElement* pGUIElement )
{
    assert ( MapContains ( m_GUIElementUserList, pGUIElement ) );
    MapRemove ( m_GUIElementUserList, pGUIElement );
}
