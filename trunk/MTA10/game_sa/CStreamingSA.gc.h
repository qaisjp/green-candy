/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        game_sa/CStreamingSA.gc.h
*  PURPOSE:     Streaming garbage collection logic
*  DEVELOPERS:  Martin Turski <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#ifndef _STREAMING_GC_
#define _STREAMING_GC_

// Garbage collection exports.
void __cdecl UnclogMemoryUsage( size_t mem_size );

// Module initialization.
void StreamingGC_Init( void );
void StreamingGC_Shutdown( void );

#endif //_STREAMING_GC_