/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        game_sa/CTextureManagerSA.shader.h
*  PURPOSE:     Internal texture management definitions
*               SubInterface of RenderWare
*  DEVELOPERS:  Martin Turski <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#ifndef _TEXTUREMANAGER_SHADER_IMPL_
#define _TEXTUREMANAGER_SHADER_IMPL_

//
// STexInfo and SShadInfo are used for mapping GTA world textures to shaders
//
struct STexInfo;
struct SShadInfo
{
    SShadInfo ( const SString& strMatch, CSHADERDUMMY* pShaderData, float fOrderPriority )
        : strMatch ( strMatch.ToLower () )
        , pShaderData ( pShaderData )
        , fOrderPriority ( fOrderPriority )
    {
    }
    const SString           strMatch;           // Always lower case
    CSHADERDUMMY* const     pShaderData;
    const float             fOrderPriority;
    std::set < STexInfo* >  associatedTexInfoMap;
};

struct STexInfo
{
    STexInfo ( ushort usTxdId, const SString& strTextureName, CD3DDUMMY* pD3DData )
        : usTxdId ( usTxdId )
        , strTextureName ( strTextureName.ToLower () )
        , pD3DData ( pD3DData )
        , pAssociatedShadInfo ( NULL )
    {
    }
    const ushort            usTxdId;
    const SString           strTextureName;     // Always lower case
    CD3DDUMMY* const        pD3DData;
    SShadInfo*              pAssociatedShadInfo;
};

#endif //_TEXTUREMANAGER_SHADER_IMPL_