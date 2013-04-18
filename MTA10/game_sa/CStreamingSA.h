/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        game_sa/CStreamingSA.h
*  PURPOSE:     Header file for data streamer class
*  DEVELOPERS:  Jax <>
*               Martin Turski <quiret@gmx.de>
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
#define FUNC_RequestVehicleUpgrade                          0x408C70

#define FUNC_CStreaming_RequestAnimations                   0x407120
#define FUNC_CStreaming_RequestSpecialModel                 0x409d10

#define MAX_MODELS                                          28835

#define ARRAY_StreamerRequest   0x008E4A60
#define MAX_STREAMING_REQUESTS  16

// Allocated at ARRAY_StreamerRequest
struct streamingRequest //size: 152
{
public:
    int     ids[MAX_STREAMING_REQUESTS];        // 0
    size_t  sizes[MAX_STREAMING_REQUESTS];      // 64

    BYTE    pad[24];                            // 128
};

namespace Streaming
{
    // Allocated dynamically in the streaming initialization
    struct syncSemaphore    //size: 48
    {
        unsigned int    blockOffset;            // 0
        unsigned int    blockCount;             // 4
        void*           buffer;                 // 8
        BYTE            pad;                    // 12
        bool            terminating;            // 13
        bool            threadActive;           // 14, true if the streaming thread is working on this
        BYTE            pad2;                   // 15
        unsigned int    resultCode;             // 16
        HANDLE          semaphore;              // 20
        HANDLE          file;                   // 24
        OVERLAPPED      overlapped;             // 28
    };

    inline streamingRequest&    GetStreamingRequest( unsigned int id )
    {
        return *( (streamingRequest*)ARRAY_StreamerRequest + id );
    }
};

class CStreamingSA : public CStreaming
{
public:
                    CStreamingSA                ( void );
                    ~CStreamingSA               ( void );

    void            RequestModel                ( unsigned short id, unsigned int flags );
    void            FreeModel                   ( unsigned short id );
    void            LoadAllRequestedModels      ( bool onlyPriority = false );
    bool            HasModelLoaded              ( unsigned int id );
    bool            IsModelLoading              ( unsigned int id );
    void            WaitForModel                ( unsigned int id );
    void            RequestAnimations           ( int idx, unsigned int flags );
    bool            HaveAnimationsLoaded        ( int idx );
    void            RequestVehicleUpgrade       ( unsigned short model, unsigned int flags );
    bool            HasVehicleUpgradeLoaded     ( int model );
    void            RequestSpecialModel         ( unsigned short model, const char *tex, unsigned int channel );

    void            SetRequestCallback          ( streamingRequestCallback_t callback );
    void            SetLoadCallback             ( streamingLoadCallback_t callback );
    void            SetFreeCallback             ( streamingFreeCallback_t callback );
};

#include "CStreamingSA.init.h"
#include "CStreamingSA.utils.h"
#include "CStreamingSA.runtime.h"

extern class CRwObjectSA *g_replObjectNative[DATA_TEXTURE_BLOCK];
extern class CColModelSA *g_colReplacement[DATA_TEXTURE_BLOCK];
extern class CColModelSAInterface *g_originalCollision[DATA_TEXTURE_BLOCK];

#endif