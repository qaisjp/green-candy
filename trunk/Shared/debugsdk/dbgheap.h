/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        debugsdk/dbgheap.h
*  PURPOSE:     Heap management tools for error isolation & debugging
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#ifndef HEAP_DEBUG
#define HEAP_DEBUG

#ifdef USE_HEAP_DEBUGGING

#include <exception>
#include <new>

#pragma warning(push)
#pragma warning(disable: 4290)

// Global allocator
void* operator new( size_t memSize ) throw(std::bad_alloc);
void* operator new( size_t memSize, const std::nothrow_t nothrow ) throw();
void* operator new[]( size_t memSize ) throw(std::bad_alloc);
void* operator new[]( size_t memSize, const std::nothrow_t nothrow ) throw();
void operator delete( void *ptr ) throw();
void operator delete[]( void *ptr ) throw();

// Malloc functions
void* DbgMalloc( size_t size );
void* DbgRealloc( void *ptr, size_t size );
bool DbgAllocGetSize( void *ptr, size_t& sizeOut );
void DbgFree( void *ptr );

#pragma warning(pop)

#endif //USE_HEAP_DEBUGGING

typedef void (__cdecl*pfnMemoryAllocWatch)( void *memPtr, size_t memSize );

void DbgHeap_Init( void );
void DbgHeap_Validate( void );
void DbgHeap_CheckActiveBlocks( void );
void DbgHeap_SetMemoryAllocationWatch( pfnMemoryAllocWatch allocWatchCallback );
void DbgHeap_Shutdown( void );

#endif //HEAP_DEBUG