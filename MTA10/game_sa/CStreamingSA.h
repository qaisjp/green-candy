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

#define DATA_TEXTURE_BLOCK      20000
#define DATA_ANIM_BLOCK         25575

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

class CModelLoadInfoSA  // size: 20
{
public:
    unsigned short  m_lodModelID;       // 0
    unsigned short  m_unknown4;         // 2
    unsigned short  m_unknown5;         // 4
    unsigned char   m_flags;            // 6
    unsigned char   m_unknown7;         // 7
    unsigned short  m_id;               // 8
    unsigned short  m_unknown8;         // 10
    unsigned int    m_unknown3;         // 12
    eLoadingState   m_eLoading;         // 16
};

#endif