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

#ifndef _CLIENT_MATERIAL_
#define _CLIENT_MATERIAL_

#define LUACLASS_MATERIAL   89

class CClientMaterial : public CClientRenderElement
{
public:
                            CClientMaterial( CClientManager* pManager, ElementID ID, LuaClass& root );

    CMaterialItem*          GetMaterialItem( void )                             { return (CMaterialItem*)m_pRenderItem; }
};

#endif //_CLIENT_MATERIAL_