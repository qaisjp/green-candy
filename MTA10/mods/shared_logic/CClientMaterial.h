/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*               (Shared logic for modifications)
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        mods/shared_logic/CClientMaterial.h
*  PURPOSE:     Deathmatch render material management
*  DEVELOPERS:  The_GTA <quiret@gmx.de>
*
*****************************************************************************/


class CClientMaterial : public CClientRenderElement
{
public:
                            CClientMaterial( CClientManager* pManager, ElementID ID, LuaClass& root ) : CClientRenderElement( pManager, ID, root )      {}

    CMaterialItem*          GetMaterialItem( void )                             { return (CMaterialItem*)m_pRenderItem; }
};
