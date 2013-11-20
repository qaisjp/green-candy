/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        game_sa/CStreamingSA.loader.h
*  PURPOSE:     Main routines for resource acquisition
*  DEVELOPERS:  Martin Turski <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#ifndef _STREAMING_LOADER_
#define _STREAMING_LOADER_

// Used by streaming
bool __cdecl CheckTXDDependency( modelId_t id );
bool __cdecl CheckAnimDependency( modelId_t id );
bool __cdecl LoadModel( void *buf, modelId_t id, unsigned int threadId );

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

    // There is a maximum of 2 streaming requests internally in the engine.
    // Those slots are parallel to the maximum syncSemaphores.
    // streamingRequest contains model ids which request data throug
    inline streamingRequest&    GetStreamingRequest( unsigned int id )
    {
        if ( id > 2 )
            __asm int 3

        return *( (streamingRequest*)ARRAY_StreamerRequest + id );
    }

    // Loader routines.
    void __cdecl LoadAllRequestedModels( bool onlyPriority );
    void __cdecl PulseLoader( void );
};

// Streaming loader exports.
modelId_t __cdecl ProcessLoadQueue( unsigned int offset, bool favorPriority );
bool __cdecl ProcessStreamingRequest( modelId_t id );
void __cdecl PulseStreamingRequests( void );
void __cdecl PulseStreamingRequest( unsigned int reqId );
void __cdecl PulseStreamingLoader( void );

// Module Initialization.
void StreamingResources_Init( void );
void StreamingResources_Shutdown( void );

#endif //_STREAMING_LOADER_