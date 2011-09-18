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
#include "Common.h"
#include <windows.h>
#include <stdio.h>


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


class CRenderWareSA : public CRenderWare
{
    public:
                        CRenderWareSA               ( enum eGameVersion version );
                        ~CRenderWareSA              ( void ) {};

    // uiModelID == 0 means no collisions will be loaded (be careful! seems crashy!)
    RpClump *           ReadDFF                     ( const char * szDFF, unsigned short usModelID );
    void                DestroyDFF                  ( RpClump * pClump );
    CColModel *         ReadCOL                     ( const char * szCOLFile );
    void                ReplaceCollisions           ( CColModel * pColModel, unsigned short usModelID );

    // Positions the front seat by reading out the vector from the 'ped_frontseat' atomic in the clump (RpClump*)
    // and changing the vector in the CModelInfo class identified by the model id (usModelID)
    bool                PositionFrontSeat           ( RpClump * pClump, unsigned short usModelID );
    unsigned int        LoadAtomics                 ( RpClump * pClump, RpAtomicContainer * pAtomics );
    void                ReplaceAllAtomicsInModel    ( RpClump * pSrc, unsigned short usModelID );
    void                ReplaceAllAtomicsInClump    ( RpClump * pDst, RpAtomicContainer * pAtomics, unsigned int uiAtomics );
    void                ReplaceWheels               ( RpClump * pClump, RpAtomicContainer * pAtomics, unsigned int uiAtomics, const char * szWheel = "wheel" );
    void                RepositionAtomic            ( RpClump * pDst, RpClump * pSrc, const char * szName );
    void                AddAllAtomics               ( RpClump * pDst, RpClump * pSrc );
    void                ReplaceVehicleModel         ( RpClump * pNew, unsigned short usModelID );

    // szName should be without the part suffix (e.g. 'door_lf' or 'door_rf', and not 'door_lf_dummy')
    bool                ReplacePartModels           ( RpClump * pClump, RpAtomicContainer * pAtomics, unsigned int uiAtomics, const char * szName );
};

#endif
