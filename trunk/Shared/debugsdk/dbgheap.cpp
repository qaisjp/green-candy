/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        debugsdk/dbgheap.cpp
*  PURPOSE:     Heap management tools for error isolation & debugging
*  DEVELOPERS:  The_GTA <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#include <StdInc.h>

#ifdef USE_HEAP_DEBUGGING

HANDLE g_privateHeap = NULL;

inline static void* _win32_allocMem( size_t memSize )
{
    return HeapAlloc( g_privateHeap, 0, memSize );
}

void* operator new( size_t memSize ) throw(std::bad_alloc)
{
    void *mem = _win32_allocMem( memSize );

    if ( !mem )
        throw std::bad_alloc( "failed to allocate memory" );

    return mem;
}

void* operator new( size_t memSize, const std::nothrow_t ) throw()
{
    return _win32_allocMem( memSize );
}

void* operator new[]( size_t memSize ) throw(std::bad_alloc)
{
    void *mem = _win32_allocMem( memSize );

    if ( !mem )
        throw std::bad_alloc( "failed to allocate memory" );

    return mem;
}

void* operator new[]( size_t memSize, const std::nothrow_t ) throw()
{
    return _win32_allocMem( memSize );
}

inline static void _win32_freeMem( void *ptr )
{
    if ( ptr )
        HeapFree( g_privateHeap, 0, ptr );
}

void operator delete( void *ptr ) throw()
{
    _win32_freeMem( ptr );
}

void operator delete[]( void *ptr ) throw()
{
    _win32_freeMem( ptr );
}

#endif

void DbgHeap_Init()
{
#ifdef USE_HEAP_DEBUGGING
    g_privateHeap = HeapCreate( 0, 0, 0 );
#endif
}

void DbgHeap_Shutdown()
{
#ifdef USE_HEAP_DEBUGGING
    HeapDestroy( g_privateHeap );
#endif
}