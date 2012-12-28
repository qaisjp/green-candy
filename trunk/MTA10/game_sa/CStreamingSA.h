/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        game_sa/CStreamingSA.h
*  PURPOSE:     Header file for data streamer class
*  DEVELOPERS:  Jax <>
*               The_GTA <quiret@gmx.de>
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

#define FUNC_CStreaming__RequestModel                       0x4087E0
#define FUNC_LoadAllRequestedModels                         0x40EA10
#define FUNC_CStreaming__HasVehicleUpgradeLoaded            0x407820
#define FUNC_CStreaming_RequestAnimations                   0x407120
#define FUNC_CStreaming_RequestSpecialModel                 0x409d10

#define MAX_MODELS                                          28835
#define ARRAY_CModelLoadInfo                                0x8E4CC0

#define VAR_StreamerThreadHandle                            0x008E4008

class CStreamingSA : public CStreaming
{
public:
                    CStreamingSA();
                    ~CStreamingSA();

    void            RequestModel( unsigned short id, unsigned int flags );
    void            FreeModel( unsigned short id );
    void            LoadAllRequestedModels( bool onlyPriority = false );
    bool            HasModelLoaded( unsigned int id );
    bool            IsModelLoading( unsigned int id );
    void            WaitForModel( unsigned int id );
    void            RequestAnimations( int idx, unsigned int flags );
    bool            HaveAnimationsLoaded( int idx );
    bool            HasVehicleUpgradeLoaded( int model );
    void            RequestSpecialModel( unsigned short model, const char *tex, unsigned int channel );

    void            SetRequestCallback( streamingRequestCallback_t callback );
    void            SetLoadCallback( streamingLoadCallback_t callback );
    void            SetFreeCallback( streamingFreeCallback_t callback );
};

class CBaseModelInfoSAInterface* __cdecl CStreaming__GetModelByHash( unsigned int hash, unsigned short *id );
class CBaseModelInfoSAInterface* CStreaming__GetModelInfoByName( const char *name, unsigned short startId, unsigned short endId, unsigned short *id );

#include "CStreamingSA.utils.h"

class CIPLFileSA
{
public:
    CIPLFileSA() : m_vecMin( MAX_REGION, -MAX_REGION ), m_vecMax( -MAX_REGION, MAX_REGION )
    {
        m_unk1 = HEIGHT_BOUND;
        m_unk2 = -HEIGHT_BOUND;

        m_unk3 = HEIGHT_BOUND;
        m_unk4 = -HEIGHT_BOUND;

        m_unk5 = 0xFFFF;

        m_unk6 = m_unk7 = m_unk8 = false;
        m_unk9 = true;

        m_unk10 = m_unk11 = false;
    }

    void* operator new ( size_t );
    void operator delete ( void *ptr );

    CVector2D       m_vecMin, m_vecMax;     // 0
    char            m_name[18];             // 16

    short           m_unk1;                 // 34
    short           m_unk2;                 // 36

    short           m_unk3;                 // 38
    short           m_unk4;                 // 40

    unsigned short  m_unk5;                 // 42

    bool            m_unk6;                 // 44
    bool            m_unk7;                 // 45
    bool            m_unk8;                 // 46
    bool            m_unk9;                 // 47, 1
    bool            m_unk10;                // 48
    bool            m_unk11;                // 49

    WORD            m_pad;                  // 50
};

extern class CRwObjectSA *g_replObjectNative[DATA_TEXTURE_BLOCK];
extern class CColModelSA *g_colReplacement[DATA_TEXTURE_BLOCK];
extern class CColModelSAInterface *g_originalCollision[DATA_TEXTURE_BLOCK];

#endif