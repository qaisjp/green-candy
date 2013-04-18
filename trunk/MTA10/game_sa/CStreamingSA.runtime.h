/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        game_sa/CStreamingSA.runtime.h
*  PURPOSE:     GTA:SA internal FileSystem streaming interface
*  DEVELOPERS:  Martin Turski <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#ifndef _STREAMING_INTERNAL_
#define _STREAMING_INTERNAL_

#define MAX_GTA_STREAM_HANDLES  128

namespace Streaming
{
    unsigned int __cdecl    OpenStream( const char *path );
};

void StreamingRuntime_Init( void );
void StreamingRuntime_Shutdown( void );

typedef char streamName[64];

extern HANDLE gtaStreamHandles[MAX_GTA_STREAM_HANDLES];
extern streamName gtaStreamNames[MAX_GTA_STREAM_HANDLES];

#endif //_STREAMING_INTERNAL_