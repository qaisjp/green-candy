/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*               (Shared logic for modifications)
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        mods/shared_logic/CClientDxFont.h
*  PURPOSE:     Custom font bucket
*  DEVELOPERS:  The_GTA <quiret@gmx.de>
*
*****************************************************************************/

#ifndef _CLIENT_DX_FONT_
#define _CLIENT_DX_FONT_

#define LUACLASS_DXFONT     86

class CClientDxFont : public CClientRenderElement
{
public:
                            CClientDxFont( CClientManager* pManager, ElementID ID, LuaClass& root, CDxFontItem* pFontItem );

    eClientEntityType       GetType( void ) const                               { return CCLIENTDXFONT; }

    // CClientDxFont methods
    CDxFontItem*            GetDxFontItem( void )                               { return (CDxFontItem*)m_pRenderItem; }
    ID3DXFont*              GetD3DXFont( void );
};

#endif //_CLIENT_DX_FONT_