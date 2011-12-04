/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.0
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        sdk/game/CRenderWare.h
*  PURPOSE:     RenderWare engine interface
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#ifndef __CRENDERWARE
#define __CRENDERWARE

#include <list>
#include "RenderWare_shared.h"

class CD3DDUMMY;
class CSHADERDUMMY;

typedef void (*PFN_WATCH_CALLBACK) ( CSHADERDUMMY* pContext, CD3DDUMMY* pD3DDataNew, CD3DDUMMY* pD3DDataOld );

#define MAX_ATOMICS_PER_CLUMP   128

class CRenderWare
{
public:
    // redo this class, do not export renderware into deathmatch
    virtual unsigned short      GetTXDIDForModelID          ( unsigned short model) = 0;
    virtual void                InitWorldTextureWatch       ( PFN_WATCH_CALLBACK pfnWatchCallback ) = 0;
    virtual bool                AddWorldTextureWatch        ( CSHADERDUMMY* pShaderData, const char* szMatch, float fShaderPriority ) = 0;
    virtual void                RemoveWorldTextureWatch     ( CSHADERDUMMY* pShaderData, const char* szMatch ) = 0;
    virtual void                RemoveWorldTextureWatchByContext ( CSHADERDUMMY* pShaderData ) = 0;
    virtual void                PulseWorldTextureWatch      () = 0;
    virtual void                GetModelTextureNames        ( std::vector < SString >& outNameList, unsigned short model ) = 0;
    virtual void                GetTxdTextures              ( std::vector < class RwTexture* >& outTextureList, unsigned short txd ) = 0;
    virtual const SString&      GetTextureName              ( CD3DDUMMY* pD3DData ) = 0;
};

#endif
