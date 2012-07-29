/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*               (Shared logic for modifications)
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        mods/shared_logic/CClientGuiFont.h
*  PURPOSE:     Custom font bucket
*  DEVELOPERS:  The_GTA <quiret@gmx.de>
*
*****************************************************************************/

#ifndef _CLIENT_GUI_FONT_
#define _CLIENT_GUI_FONT_

#define LUACLASS_GUIFONT    87

class CClientGuiFont : public CClientRenderElement
{
public:
                            CClientGuiFont( CClientManager* pManager, ElementID ID, CGuiFontItem* pFontItem );
                            ~CClientGuiFont( void );

    eClientEntityType       GetType( void ) const                               { return CCLIENTGUIFONT; }
    void                    Unlink( void );

    // CClientGuiFont methods
    CGuiFontItem*           GetGuiFontItem( void )                              { return (CGuiFontItem*)m_pRenderItem; }
    const SString&          GetCEGUIFontName( void );
    void                    NotifyGUIElementAttach( CClientGUIElement* pGUIElement );
    void                    NotifyGUIElementDetach( CClientGUIElement* pGUIElement );

protected:
    std::set < CClientGUIElement* > m_GUIElementUserList;
};

#endif //_CLIENT_GUI_FONT_