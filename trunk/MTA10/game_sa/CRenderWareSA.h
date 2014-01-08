/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        game_sa/CRenderWareSA.h
*  PURPOSE:     Header file for RenderWare game engine class
*  DEVELOPERS:  Cecill Etheredge <ijsf@gmx.net>
*               Martin Turski <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#ifndef __CRENDERWARESA
#define __CRENDERWARESA

#define WIN32_LEAN_AND_MEAN

#include <game/CRenderWare.h>

#include "CModelInfoSA.h"
#include "CColModelSA.h"

class CRenderWareSA : public CRenderWare
{
public:
                        CRenderWareSA( enum eGameVersion version );
                        ~CRenderWareSA();

    RwMatrix*           AllocateMatrix();
    CRpLight*           CreateLight( RpLightType type );
    CRwFrame*           CreateFrame();
    CRwCamera*          CreateCamera( int width, int height );
    CModel*             CreateClump();

    bool                IsRendering() const;
    
    void                EnableEnvMapRendering( bool enabled );
    bool                IsEnvMapRenderingEnabled( void ) const;

    // Lighting utilities.
    void                SetGlobalLightingAlwaysEnabled( bool enabled );
    bool                IsGlobalLightingAlwaysEnabled( void ) const;

    void                SetLocalLightingAlwaysEnabled( bool enabled );
    bool                IsLocalLightingAlwaysEnabled( void ) const;

    // uiModelID == 0 means no collisions will be loaded
    RpClump*            ReadDFF( CFile *file, unsigned short usModelID, CColModelSA*& colOut );
    CColModel*          ReadCOL( CFile *file );

    // Positions the front seat by reading out the vector from the 'ped_frontseat' atomic in the clump (RpClump*)
    // and changing the vector in the CModelInfo class identified by the model id (usModelID)
    bool                PositionFrontSeat( RpClump *clump, unsigned short usModelID );
};

extern IDirect3DDevice9 *const *g_renderDevice;

// Include sub modules
#include "CRenderWareSA.rwapi.h"
#include "CRenderWareSA.pipeline.h"
#include "CRenderWareSA.mem.h"
#include "CRenderWareSA.render.h"
#include "CRenderWareSA.rtcallback.h"
#include "CRenderWareSA.lighting.h"

#endif
