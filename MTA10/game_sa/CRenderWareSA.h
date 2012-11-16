/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        game_sa/CRenderWareSA.h
*  PURPOSE:     Header file for RenderWare game engine class
*  DEVELOPERS:  Cecill Etheredge <ijsf@gmx.net>
*               The_GTA <quiret@gmx.de>
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
                        ~CRenderWareSA()    {}

    RwMatrix*           AllocateMatrix();
    CRpLight*           CreateLight( RpLightType type );
    CRwFrame*           CreateFrame();
    CRwCamera*          CreateCamera( int width, int height );

    bool                IsRendering() const;

    // uiModelID == 0 means no collisions will be loaded
    RpClump*            ReadDFF( CFile *file, unsigned short usModelID, CColModelSA*& colOut );
    CColModel*          ReadCOL( CFile *file );

    // Positions the front seat by reading out the vector from the 'ped_frontseat' atomic in the clump (RpClump*)
    // and changing the vector in the CModelInfo class identified by the model id (usModelID)
    bool                PositionFrontSeat( RpClump *clump, unsigned short usModelID );
};

// Internal RenderWare functions
RwTexture*  RwFindTexture( const char *name, const char *secName );     // US exe: 0x007F3AC0
RwError*    RwSetError( RwError *info );                                // US exe: 0x00808820
RpLight*    RpLightCreate( unsigned char type );                        // US exe: 0x00752110
RpClump*    RpClumpCreate();                                            // US exe: 0x0074A290
RwCamera*   RwCameraCreate();                                           // US exe: 0x007EE4F0

#endif
