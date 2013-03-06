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

SYSTEM_INFO g_systemInfo;
HANDLE g_privateHeap = NULL;

#ifdef USE_FULL_PAGE_HEAP

#ifdef PAGE_HEAP_INTEGRITY_CHECK

#define MEM_PAGE_MOD( bytes )   ( ( (bytes) + g_systemInfo.dwPageSize - 1 ) / g_systemInfo.dwPageSize )

struct _memIntro
{
    unsigned int checksum;
    size_t  objSize;
};

struct _memOutro
{
    unsigned int checksum;
};

inline static void* _win32_allocMem( size_t memSize )
{
    _memIntro *mem = (_memIntro*)VirtualAlloc( NULL, memSize + sizeof(_memIntro) + sizeof(_memOutro), MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE );
    _memOutro *outro = (_memOutro*)( (unsigned char*)( mem + 1 ) + memSize );

    // Fill memory with debug pattern
    memset( mem, 0x6A, MEM_PAGE_MOD( memSize ) * g_systemInfo.dwPageSize );

    mem->checksum = 0xCAFEBABE;
    mem->objSize = memSize;

    outro->checksum = 0xBABECAFE;

    return mem + 1;
}

inline static void _win32_freeMem( void *ptr )
{
    if ( !ptr )
        return;

    _memIntro *intro = (_memIntro*)ptr - 1;
    _memOutro *outro = (_memOutro*)( (unsigned char*)ptr + intro->objSize );

    if ( intro->checksum != 0xCAFEBABE || outro->checksum != 0xBABECAFE )
        __asm int 3

    size_t allocSize = MEM_PAGE_MOD( intro->objSize ) * g_systemInfo.dwPageSize;
    unsigned char *endptr = (unsigned char*)intro + allocSize;
    unsigned char *seek = (unsigned char*)outro + sizeof(*outro);

    // Check memory integrity
    while ( seek < endptr )
    {
        if ( *seek != 0x6A )
            __asm int 3

        seek++;
    }

    assert( VirtualFree( intro, 0, MEM_RELEASE ) );
}

#else
inline static void* _win32_allocMem( size_t memSize )
{
    return VirtualAlloc( NULL, memSize, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE );
}

inline static void _win32_freeMem( void *ptr )
{
    if ( !ptr )
        return;

    assert( VirtualFree( ptr, 0, MEM_RELEASE ) );
}
#endif  //PAGE_HEAP_INTEGRITY_CHECK

#else
inline static void* _win32_allocMem( size_t memSize )
{
    return HeapAlloc( g_privateHeap, 0, memSize );
}

inline static void _win32_freeMem( void *ptr )
{
    if ( ptr )
        HeapFree( g_privateHeap, 0, ptr );
}
#endif

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

    GetSystemInfo( &g_systemInfo );
#endif
}

void DbgHeap_Shutdown()
{
#ifdef USE_HEAP_DEBUGGING
    HeapValidate( g_privateHeap, 0, NULL );
    HeapDestroy( g_privateHeap );
#endif
}