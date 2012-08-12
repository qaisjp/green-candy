/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.0
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
                        CRenderWareSA               ( enum eGameVersion version );
                        ~CRenderWareSA              () {};

    RwMatrix*           AllocateMatrix              ();

    // uiModelID == 0 means no collisions will be loaded
    RpClump*            ReadDFF                     ( CFile *file, unsigned short usModelID, CColModelSA*& colOut );
    CColModel*          ReadCOL                     ( CFile *file );

    // Positions the front seat by reading out the vector from the 'ped_frontseat' atomic in the clump (RpClump*)
    // and changing the vector in the CModelInfo class identified by the model id (usModelID)
    bool                PositionFrontSeat           ( RpClump * pClump, unsigned short usModelID );
    unsigned int        LoadAtomics                 ( RpClump * pClump, RpAtomicContainer * pAtomics );
    void                ReplaceAllAtomicsInModel    ( RpClump * pSrc, unsigned short usModelID );
    void                ReplaceAllAtomicsInClump    ( RpClump * pDst, RpAtomicContainer * pAtomics, unsigned int uiAtomics );
    void                ReplaceWheels               ( RpClump * pClump, RpAtomicContainer * pAtomics, unsigned int uiAtomics, const char * szWheel = "wheel" );
    void                RepositionAtomic            ( RpClump * pDst, RpClump * pSrc, const char * szName );
    void                AddAllAtomics               ( RpClump * pDst, RpClump * pSrc );

    // szName should be without the part suffix (e.g. 'door_lf' or 'door_rf', and not 'door_lf_dummy')
    bool                ReplacePartModels           ( RpClump * pClump, RpAtomicContainer * pAtomics, unsigned int uiAtomics, const char * szName );
};

#endif
