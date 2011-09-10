/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.0
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        game_sa/CStreamingSA.h
*  PURPOSE:     Header file for data streamer class
*  DEVELOPERS:  Jax <>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#ifndef __CStreamingSA_H
#define __CStreamingSA_H

#include <game/CStreaming.h>
#include "Common.h"

#define FUNC_CStreaming__RequestModel                    0x4087E0
#define FUNC_LoadAllRequestedModels                      0x40EA10
#define FUNC_CStreaming__HasVehicleUpgradeLoaded         0x407820
#define FUNC_CStreaming_RequestAnimations                0x407120
#define FUNC_CStreaming_RequestSpecialModel              0x409d10

#define MAX_MODELS                                       28835
#define ARRAY_CModelLoadInfo                             0x8E4CC0

class CStreamingSA : public CStreaming
{
public:
    void            RequestModel                    ( unsigned short id, unsigned int flags );
    void            FreeModel                       ( unsigned short id );
    void            LoadAllRequestedModels          ( BOOL bOnlyPriorityModels = 0 );
    bool            HasModelLoaded                  ( unsigned int id );
    bool            IsModelLoading                  ( unsigned int id );
    void            WaitForModel                    ( unsigned int id );
    void            RequestAnimations               ( int iAnimationLibraryBlock, DWORD dwFlags );
    BOOL            HaveAnimationsLoaded            ( int iAnimationLibraryBlock );
    bool            HasVehicleUpgradeLoaded         ( int model );
    void            RequestSpecialModel ( DWORD model, const char * szTexture, DWORD channel );
};

enum eLoadingState
{
    MODEL_UNAVAILABLE,
    MODEL_LOADED,
    MODEL_LOADING,
    MODEL_LOD,    // Perhaps
    MODEL_UNKNOWN,
    FIX_DWORD = 0xFFFFFFFF
};

class CModelLoadInfoSA
{
public:
    unsigned short  m_lodModelID;
    unsigned short  m_unknown4;
    unsigned short  m_unknown5;
    unsigned char   m_flags;
    unsigned char   m_unknown7;
    unsigned short  m_id;
    unsigned short  m_unknown8;
    unsigned int    m_unknown3;
    eLoadingState   m_eLoading;
};

#endif