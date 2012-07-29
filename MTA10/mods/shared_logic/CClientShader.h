/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*               (Shared logic for modifications)
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        mods/shared_logic/CClientShader.h
*  PURPOSE:     Core shader rendering management for deathmatch
*  DEVELOPERS:  The_GTA <quiret@gmx.de>
*
*****************************************************************************/

#ifndef _CLIENT_SHADER_
#define _CLIENT_SHADER_

#define LUACLASS_SHADER     69

class CClientShader : public CClientMaterial
{
public:
                            CClientShader( CClientManager* pManager, ElementID ID, LuaClass& root, CShaderItem* pShaderItem );

    eClientEntityType       GetType( void ) const                               { return CCLIENTSHADER; }

    // CClientShader methods
    CShaderItem*            GetShaderItem( void )                               { return (CShaderItem*)m_pRenderItem; }

};

#endif //_CLIENT_SHADER_