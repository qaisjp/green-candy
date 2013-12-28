/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        sdk/game/CRenderWare.h
*  PURPOSE:     RenderWare engine interface
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#ifndef __CRENDERWARE
#define __CRENDERWARE

#include "RenderWare_shared.h"

class CD3DDUMMY;
class CSHADERDUMMY;
class CColModel;

typedef void (*PFN_WATCH_CALLBACK) ( CSHADERDUMMY* pContext, CD3DDUMMY* pD3DDataNew, CD3DDUMMY* pD3DDataOld );

#define MAX_ATOMICS_PER_CLUMP   128

class CRenderWare
{
public:
    virtual class CRpLight*         CreateLight( RpLightType type ) = 0;
    virtual class CRwFrame*         CreateFrame() = 0;
    virtual class CRwCamera*        CreateCamera( int width, int height ) = 0;
    virtual class CModel*           CreateClump() = 0;

    virtual bool                    IsRendering() const = 0;

    virtual void                    EnableEnvMapRendering( bool enabled ) = 0;
    virtual bool                    IsEnvMapRenderingEnabled( void ) const = 0;

    virtual CColModel*              ReadCOL( CFile *file ) = 0;
};

#endif
