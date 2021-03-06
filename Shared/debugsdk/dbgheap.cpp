/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        debugsdk/dbgheap.cpp
*  PURPOSE:     Heap management tools for error isolation & debugging
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#include <StdInc.h>
#include <MemoryUtils.h>
#include <core/COSUtils.h>

/*
    DebugHeap memory debugging environment

    You can use this tool to find memory corruption and leaks in your C++ projects.
    It supports per-module heaps, so that errors can be isolated to game_sa, multiplayer_sa,
    deathmatch, core, GUI, etc. Use global defines in StdInc.h to set debugging properties...

    USE_HEAP_DEBUGGING
        Enables the heap debugger. The global new and delete operators are overloaded, so
        that the memory allocations are monitored. When the module terminates, all its
        memory is free'd. Requirement for DebugHeap to function.
    USE_FULL_PAGE_HEAP
        Enables full-page heap debugging. This option enables you to catch very crusty
        memory corruption issues (heavy out-of-bounds read/writes, buffer overflows, ...).
        For that the Windows Heap management is skipped. VirtualAlloc is used for every
        memory allocation, so that objects reside on their own pages.

        If full-page heap is disabled, the allocation defaults to the Windows Heap. It
        uses its own heap validation routines.

        Options can be used in combination...

        PAGE_HEAP_INTEGRITY_CHECK
            The memory is guarded by checksums on the object intro and outro regions and the
            remainder of the page is filled with a pattern. Once the memory is free'd or a
            validation is requested, the checksums and the pattern are checked using MEM_INTERRUPT.

            You have to enable this option if page heap memory should be free'd on termination.
        PAGE_HEAP_MEMORY_STATS
            Once the module terminates, all leaked memory is counted and free'd. Statistics
            are printed using OutputDebugString. This option only works with PAGE_HEAP_INTEGRITY_CHECK.
    USE_HEAP_STACK_TRACE
        Performs a stacktrace for every allocation made. This setting is useful to track down complicated
        memory leak situations. Use this only in very controlled scenarios, since it can use a lot of memory.

    You can define the macro MEM_INTERRUPT( bool_expr ) yourself. The most basic content is a redirect
    to assert( bool_expr ). If bool_expr is false, a memory error occured. MEM_INTERRUPT can be invoked
    during initialization, runtime and termination of your module.

    Note that debugging application memory usage in general spawns additional meta-data depending on the
    configuration. Using USE_FULL_PAGE_HEAP, the application will quickly go out of allocatable memory since
    huge chunks are allocated. Your main application may not get to properly initialize itself; test in
    a controlled environment instead!

    FEATURE SET:
        finds memory leaks,
        finds invalid (page heap) object free requests,
        detects memory corruption,
        callstack traces of memory leaks

    DEPENDENCIES:
        COSUtils.h

    version 1.3
*/

#ifdef USE_HEAP_DEBUGGING
#include <windows.h>
#ifndef LIST_GETITEM
#include <rwlist.hpp>
#endif //LIST_GETITEM

#ifndef MEM_INTERRUPT
#define MEM_INTERRUPT( expr )   assert( expr )
#endif //MEM_INTERRUPT

struct NativePageNoIntersectAllocator
{
    AINLINE bool OnPageCollision( const NativePageAllocation::pageHandleInfo& handleInfo, const NativePageAllocation::pageInfo& pageInfo )
    {
        return false;
    }
};

typedef NativePageAllocator <NativePageNoIntersectAllocator> DebugFullPageHeapAllocator;

SYSTEM_INFO g_systemInfo;
DebugFullPageHeapAllocator *_nativeAlloc = NULL;
pfnMemoryAllocWatch _memAllocWatchCallback = NULL;

#ifdef USE_FULL_PAGE_HEAP

// Since we know that memory is allocated along pages, we can check for
// invalid pointers given to the manager.
#define PAGE_MEM_ADJUST( ptr )  (void*)( (char*)ptr - ( (unsigned int)ptr % g_systemInfo.dwPageSize ) )

inline static void* _win32_allocMemPage( size_t memSize )
{
    DebugFullPageHeapAllocator::pageHandle *handle = _nativeAlloc->Allocate( NULL, memSize );

    if ( !handle )
        return NULL;

    return handle->GetTargetPointer();
}

inline static bool _win32_reallocMemPage( void *ptr, size_t newRegionSize )
{
    DebugFullPageHeapAllocator::pageHandle *handle = _nativeAlloc->FindHandleByAddress( ptr );

    if ( !handle )
        return false;

    return _nativeAlloc->SetHandleSize( handle, newRegionSize );
}

inline static void _win32_freeMemPage( void *ptr )
{
    bool releaseSuccess = _nativeAlloc->FreeByAddress( ptr );

    MEM_INTERRUPT( releaseSuccess );    // pointer to page is invalid

    // This method assures that the pointer given to it is a real
    // pointer that has been previously returned by _win32_allocMemPage.
}

#ifdef PAGE_HEAP_INTEGRITY_CHECK

#ifndef PAGE_MEM_DEBUG_PATTERN
#define PAGE_MEM_DEBUG_PATTERN  0x6A
#endif //PAGE_MEM_DEBUG_PATTERN

#ifndef PAGE_MEM_ACTIVE_DEBUG_PATTERN
#define PAGE_MEM_ACTIVE_DEBUG_PATTERN 0x11
#endif //PAGE_MEM_ACTIVE_DEBUG_PATTERN

#define MEM_PAGE_MOD( bytes )   ( ( (bytes) + g_systemInfo.dwPageSize - 1 ) / g_systemInfo.dwPageSize )

struct _memIntro
{
    unsigned int checksum;
    size_t  objSize;
    RwListEntry <_memIntro> memList;
};

struct _memOutro
{
    unsigned int checksum;
};

RwList <_memIntro>  g_privateMemory;

inline static void _win32_initHeap( void )
{
    LIST_CLEAR( g_privateMemory.root );
}

inline static size_t _getMetaSize( size_t objSize )
{
    return objSize + sizeof( _memIntro ) + sizeof( _memOutro );
}

inline static size_t _win32_getRealPageSize( size_t objSize )
{
    return ( MEM_PAGE_MOD( _getMetaSize( objSize ) ) * g_systemInfo.dwPageSize );
}

inline static void* _win32_allocMem( size_t memSize )
{
    const size_t pageRegionRequestSize = _win32_getRealPageSize( memSize );

    _memIntro *mem = (_memIntro*)_win32_allocMemPage( pageRegionRequestSize );
    _memOutro *outro = (_memOutro*)( (unsigned char*)( mem + 1 ) + memSize );

#ifdef PAGE_HEAP_ERROR_ON_LOWMEM
    MEM_INTERRUPT( mem == NULL );
#else
    if ( mem == NULL )
        return NULL;
#endif //PAGE_HEAP_ERROR_ON_LOWMEM

    // Fill memory with debug pattern
    {
        size_t metaSize = _getMetaSize( memSize );

        memset( mem, PAGE_MEM_ACTIVE_DEBUG_PATTERN, metaSize );
        memset( outro + 1, PAGE_MEM_DEBUG_PATTERN, pageRegionRequestSize - metaSize );
    }

    mem->checksum = 0xCAFEBABE;
    mem->objSize = memSize;
    LIST_APPEND( g_privateMemory.root, mem->memList );

    outro->checksum = 0xBABECAFE;

    return mem + 1;
}

inline static void _win32_checkBlockIntegrity( void *ptr )
{
    _memIntro *intro = (_memIntro*)ptr - 1;
    _memOutro *outro = (_memOutro*)( (unsigned char*)ptr + intro->objSize );

    MEM_INTERRUPT( intro->checksum == 0xCAFEBABE && outro->checksum == 0xBABECAFE );

    size_t allocSize = _win32_getRealPageSize( intro->objSize );
    unsigned char *endptr = (unsigned char*)intro + allocSize;
    unsigned char *seek = (unsigned char*)outro + sizeof(*outro);

    // Check memory integrity
    while ( seek < endptr )
    {
        // If this check fails, memory corruption has happened.
        MEM_INTERRUPT( *seek == PAGE_MEM_DEBUG_PATTERN );

        seek++;
    }

    LIST_VALIDATE( intro->memList );
}

inline static void _win32_freeMem( void *ptr )
{
    if ( !ptr )
        return;

    void *valid_ptr = (void*)( (_memIntro*)PAGE_MEM_ADJUST( ptr ) + 1 );
    MEM_INTERRUPT( valid_ptr == ptr );

    _win32_checkBlockIntegrity( valid_ptr );

    _memIntro *intro = (_memIntro*)valid_ptr - 1;
    LIST_REMOVE( intro->memList );

    _win32_freeMemPage( intro );
}

inline static size_t _win32_getAllocSize( void *ptr )
{
    if ( !ptr )
        return 0;

    void *valid_ptr = (void*)( (_memIntro*)PAGE_MEM_ADJUST( ptr ) + 1 );
    MEM_INTERRUPT( valid_ptr == ptr );

    _memIntro *intro = (_memIntro*)valid_ptr - 1;

    return intro->objSize;
}

inline static void* _win32_reallocMem( void *ptr, size_t newSize )
{
    if ( !ptr || !newSize )
        return NULL;

    void *out_ptr = NULL;
    {
        void *page_ptr = PAGE_MEM_ADJUST( ptr );

        void *valid_ptr = (void*)( (_memIntro*)page_ptr + 1 );
        MEM_INTERRUPT( valid_ptr == ptr );

        // Verify block contents.
        _win32_checkBlockIntegrity( valid_ptr );

        // Get the meta-data of the old data.
        _memIntro *old_intro = (_memIntro*)page_ptr;

        size_t oldObjSize = old_intro->objSize;

        // Verify that our object size has changed at all
        if ( newSize != oldObjSize )
        {
            // Reallocate to actually required page memory.
            const size_t constructNewSize = _win32_getRealPageSize( newSize ); 
            
            bool reallocSuccess = _win32_reallocMemPage( page_ptr, constructNewSize );

            // The reallocation may fail if the page nesting is too complicated.
            // For this we must move to a completely new block of memory that is size'd appropriately.
            if ( !reallocSuccess )
            {
                // Allocate a new page region of memory.
                void *newMem = _win32_allocMem( newSize );

                // Only process this request if the NT kernel could fetch a new page for us.
                if ( newMem != NULL )
                {
                    // Copy the data contents to the new memory region.
                    void *new_data = newMem;

                    out_ptr = new_data;

                    size_t validDataSize = std::min( newSize, oldObjSize );

                    memcpy( new_data, valid_ptr, validDataSize );
                }

                // Deallocate the old memory.
                _win32_freeMem( valid_ptr );
            }
            else
            {
                out_ptr = valid_ptr;

                // Get new pointers to meta-data.
                _memIntro *intro = old_intro;
                _memOutro *outro = (_memOutro*)( (unsigned char*)valid_ptr + newSize );

                // Rewrite block integrity
                intro->objSize = newSize;
                outro->checksum = 0xBABECAFE;

                // If the object size has increased, write the active debug pattern at the new bytes.
                if ( newSize > oldObjSize )
                {
                    memset( (char*)valid_ptr + oldObjSize,
                        PAGE_MEM_ACTIVE_DEBUG_PATTERN,
                        newSize - oldObjSize
                    );
                }

                // Fill other memory with debug pattern (without killing user data)
                memset( outro + 1,
                    PAGE_MEM_DEBUG_PATTERN,
                    constructNewSize - (sizeof(_memIntro) + sizeof(_memOutro) + newSize)
                );
            }
        }
        else
        {
            // We do not change anything, so return old pointer.
            out_ptr = valid_ptr;
        }
    }
    return out_ptr;
}

inline static void _win32_validateMemory( void )
{
    // Make sure the DebugHeap manager is not damaged.
    LIST_VALIDATE( g_privateMemory.root );

    // Check all blocks in order
    LIST_FOREACH_BEGIN( _memIntro, g_privateMemory.root, memList )
        _win32_checkBlockIntegrity( item + 1 );
    LIST_FOREACH_END
}

#ifdef PAGE_HEAP_MEMORY_STATS
inline static void OutputDebugStringFormat( const char *fmt, ... )
{
    char buf[0x10000];
    va_list argv;

    va_start( argv, fmt );
    _vsnprintf( buf, sizeof( buf ), fmt, argv );
    va_end( argv );

    OutputDebugString( buf );
}
#endif //PAGE_HEAP_MEMORY_STATS

inline static void _win32_shutdownHeap( void )
{
    // Make sure the DebugHeap manager is not damaged.
    LIST_VALIDATE( g_privateMemory.root );
    
#ifdef PAGE_HEAP_MEMORY_STATS
    // Memory debugging statistics.
    unsigned int blockCount = 0;
    unsigned int pageCount = 0;
    size_t memLeaked = 0;
#endif //PAGE_HEAP_MEMORY_STATS

    // Check all blocks in order and free them
    while ( !LIST_EMPTY( g_privateMemory.root ) )
    {
        _memIntro *item = LIST_GETITEM( _memIntro, g_privateMemory.root.next, memList );
    
#ifdef PAGE_HEAP_MEMORY_STATS
        // Keep track of stats.
        blockCount++;
        pageCount += MEM_PAGE_MOD( item->objSize + sizeof(_memIntro) + sizeof(_memOutro) );
        memLeaked += item->objSize;
#endif //PAGE_HEAP_MEMORY_STATS

        _win32_freeMem( item + 1 );
    }

#ifdef PAGE_HEAP_MEMORY_STATS
    if ( blockCount != 0 )
    {
        OutputDebugString( "Heap Memory Leak Protocol:\n" );
        OutputDebugStringFormat(
            "* leaked memory: %u\n" \
            "* blocks/pages allocated: %u/%u [%u]\n",
            memLeaked,
            blockCount, pageCount, blockCount * g_systemInfo.dwPageSize
        );
    }
    else
        OutputDebugString( "No memory leaks detected." );
#endif //PAGE_HEAP_MEMORY_STATS
}

#else
inline static void _win32_initHeap( void )
{
    return;
}

inline static void* _win32_allocMem( size_t memSize )
{
    return _win32_allocMemPage( memSize );
}

inline static void* _win32_reallocMem( void *ptr, size_t size )
{
    return ptr;
}

inline static void _win32_freeMem( void *ptr )
{
    if ( !ptr )
        return;

    void *valid_ptr = PAGE_MEM_ADJUST( ptr );
    MEM_INTERRUPT( valid_ptr == ptr );

    _win32_freeMemPage( valid_ptr );
}

inline static void _win32_validateMemory( void )
{
    return;
}

inline static void _win32_shutdownHeap( void )
{
    return;
}
#endif  //PAGE_HEAP_INTEGRITY_CHECK

#else
HANDLE g_privateHeap = NULL;

inline static void _win32_initHeap( void )
{
    g_privateHeap = HeapCreate( 0, 0, 0 );

    unsigned int info = 0;
    HeapSetInformation( g_privateHeap, HeapCompatibilityInformation, &info, sizeof(info) );
}

inline static void* _win32_allocMem( size_t memSize )
{
    return HeapAlloc( g_privateHeap, 0, memSize );
}

inline static void* _win32_reallocMem( void *ptr, size_t size )
{
    return HeapReAlloc( g_privateHeap, 0, ptr, size );
}

inline static void _win32_freeMem( void *ptr )
{
    if ( ptr )
    {
        MEM_INTERRUPT( HeapValidate( g_privateHeap, 0, ptr ) );
        HeapFree( g_privateHeap, 0, ptr );
    }
}

inline static void _win32_validateMemory( void )
{
    MEM_INTERRUPT( HeapValidate( g_privateHeap, 0, NULL ) );
}

inline static void _win32_shutdownHeap( void )
{
    MEM_INTERRUPT( HeapValidate( g_privateHeap, 0, NULL ) );
    HeapDestroy( g_privateHeap );
}
#endif

inline void DbgMemAllocEvent( void *memPtr, size_t memSize )
{
    if ( _memAllocWatchCallback )
    {
        _memAllocWatchCallback( memPtr, memSize );
    }
}

// Block header for correctness.
// We sometimes _MUST_ strip the debug block header.
struct _debugMasterHeader
{
    bool hasDebugInfoHeader;
    bool isSilent;

    char pad[2];
};

// General debug block header.
struct _debugBlockHeader
{
#ifdef USE_HEAP_STACK_TRACE
    std::string callStackPrint;
#endif //USE_HEAP_STACK_TRACE
    RwListEntry <_debugBlockHeader> node;
};

static bool _isInManager = false;
static RwList <_debugBlockHeader> _dbgAllocBlocks;

inline bool _doesRequireBlockHeader( void )
{
    bool doesRequire = false;

#ifdef USE_HEAP_STACK_TRACE
    doesRequire = true;
#endif

    return doesRequire;
}

inline void _fillDebugMasterHeader( _debugMasterHeader *header, bool hasBlockHeader, bool isSilent )
{
    new (header) _debugMasterHeader;

    header->hasDebugInfoHeader = hasBlockHeader;
    header->isSilent = isSilent;
}

inline void _fillDebugBlockHeader( _debugBlockHeader *blockHeader, bool shouldInitExpensiveExtensions )
{
    // Construct the header.
    new (blockHeader) _debugBlockHeader;

    LIST_APPEND( _dbgAllocBlocks.root, blockHeader->node );

    // Fill it depending on extensions.
#ifdef USE_HEAP_STACK_TRACE
    if ( shouldInitExpensiveExtensions )
    {
        DbgTrace::IEnvSnapshot *snapshot = DbgTrace::CreateEnvironmentSnapshot();

        if ( snapshot )
        {
            blockHeader->callStackPrint = snapshot->ToString();

            delete snapshot;
        }
    }
#endif
}

inline void _killDebugMasterHeader( _debugMasterHeader *header )
{
    header->~_debugMasterHeader();
}

inline void _killDebugBlockHeader( _debugBlockHeader *blockHeader )
{
    // Unlist us.
    LIST_REMOVE( blockHeader->node );

    blockHeader->~_debugBlockHeader();
}

inline void* DbgMallocNative( size_t memSize )
{
    void *memPtr = NULL;

    size_t requiredMemBlockSize = memSize;

    bool requiresBlockHeader = _doesRequireBlockHeader();

    bool hasBlockHeader = false;

    if ( requiresBlockHeader )
    {
        // If we have the possibility to include any kind of headers, we need a master header.
        requiredMemBlockSize += sizeof( _debugMasterHeader );

        // Check whether we should include the debug block header.
        // This one has useful information about how a block came to be.
        if ( _isInManager == false )
        {
            requiredMemBlockSize += sizeof( _debugBlockHeader );

            hasBlockHeader = true;
        }
    }

    bool resetManagerFlag = false;

    if ( _isInManager == false )
    {
        _isInManager = true;

        resetManagerFlag = true;
    }

    // Allocate the memory.
    memPtr = _win32_allocMem( requiredMemBlockSize );

    DbgMemAllocEvent( memPtr, requiredMemBlockSize );

    if ( requiresBlockHeader )
    {
        // Also fill the block header if we have it.
        bool isSilent = ( resetManagerFlag == false );

        if ( hasBlockHeader )
        {
            _debugBlockHeader *blockHeader = (_debugBlockHeader*)memPtr;

            _fillDebugBlockHeader( blockHeader, isSilent == false );

            memPtr = ( blockHeader + 1 );
        }

        // We must construct the master header last.
        _debugMasterHeader *masterHeader = (_debugMasterHeader*)memPtr;

        _fillDebugMasterHeader( masterHeader, hasBlockHeader, isSilent );

        memPtr = ( masterHeader + 1 );
    }

    if ( resetManagerFlag )
    {
        _isInManager = false;
    }

    return memPtr;
}

inline void* DbgReallocNative( void *memPtr, size_t newSize )
{
    bool requiresBlockHeader = _doesRequireBlockHeader();

    void *newPtr = NULL;

    size_t actualNewMemSize = newSize;

    bool hasBlockHeader = false;
    bool isSilent = false;

    if ( requiresBlockHeader )
    {
        actualNewMemSize += sizeof( _debugMasterHeader );

        // Check the master header.
        _debugMasterHeader *masterHeader = (_debugMasterHeader*)memPtr - 1;

        isSilent = masterHeader->isSilent;

        // We might or might not have the block header.
        if ( masterHeader->hasDebugInfoHeader )
        {
            actualNewMemSize += sizeof( _debugBlockHeader );

            // Delete the old block header.
            _debugBlockHeader *oldBlockHeader = (_debugBlockHeader*)masterHeader - 1;

            _killDebugBlockHeader( oldBlockHeader );

            memPtr = oldBlockHeader;

            hasBlockHeader = true;
        }
        else
        {
            memPtr = masterHeader;
        }
    }

    // ReAllocate the memory.
    newPtr = _win32_reallocMem( memPtr, actualNewMemSize );

    DbgMemAllocEvent( newPtr, actualNewMemSize );

    // Resurface the structures.
    if ( requiresBlockHeader )
    {
        if ( hasBlockHeader )
        {
            _debugBlockHeader *blockHeader = (_debugBlockHeader*)newPtr;

            _fillDebugBlockHeader( blockHeader, isSilent == false );

            newPtr = ( blockHeader + 1 );
        }

        // Now the master header.
        _debugMasterHeader *masterHeader = (_debugMasterHeader*)newPtr;

        _fillDebugMasterHeader( masterHeader, hasBlockHeader, isSilent );

        newPtr = ( masterHeader + 1 );
    }

    return newPtr;
}

inline void DbgFreeNative( void *memPtr )
{
    bool requiresBlockHeader = _doesRequireBlockHeader();

    void *actualMemPtr = memPtr;

    if ( requiresBlockHeader )
    {
        // Check the master header.
        _debugMasterHeader *masterHeader = (_debugMasterHeader*)memPtr - 1;

        if ( masterHeader->hasDebugInfoHeader )
        {
            _debugBlockHeader *blockHeader = (_debugBlockHeader*)masterHeader - 1;

            // Deconstruct the block header.
            _killDebugBlockHeader( blockHeader );

            actualMemPtr = blockHeader;
        }
        else
        {
            actualMemPtr = masterHeader;
        }

        // Deconstruct the master header.
        _killDebugMasterHeader( masterHeader );
    }

    _win32_freeMem( actualMemPtr );
}

inline bool DbgAllocGetSizeNative( void *memPtr, size_t& sizeOut )
{
    bool couldGetSize = false;

    size_t theSizeOut = 0;

    bool requiresHeaders = _doesRequireBlockHeader();

    bool hasBlockHeader = false;

    void *blockPtr = memPtr;
    {
        if ( requiresHeaders )
        {
            _debugMasterHeader *masterHeader = (_debugMasterHeader*)memPtr - 1;

            blockPtr = masterHeader;

            if ( masterHeader->hasDebugInfoHeader )
            {
                blockPtr = (_debugBlockHeader*)blockPtr - 1;

                hasBlockHeader = true;
            }
        }
    }

#ifdef USE_FULL_PAGE_HEAP
    {
        theSizeOut = _win32_getAllocSize( blockPtr );

        couldGetSize = true;
    }
#endif //USE_FULL_PAGE_HEAP

    if ( couldGetSize )
    {
        if ( requiresHeaders )
        {
            theSizeOut -= sizeof( _debugMasterHeader );

            if ( hasBlockHeader )
            {
                theSizeOut -= sizeof( _debugBlockHeader );
            }
        }

        sizeOut = theSizeOut;
    }

    return couldGetSize;
}

void* operator new( size_t memSize ) throw(std::bad_alloc)
{
    void *mem = DbgMallocNative( memSize );

    if ( !mem )
        throw std::bad_alloc();

    return mem;
}

void* operator new( size_t memSize, const std::nothrow_t ) throw()
{
    MEM_INTERRUPT( memSize != 0 );

    return DbgMallocNative( memSize );
}

void* operator new[]( size_t memSize ) throw(std::bad_alloc)
{
    MEM_INTERRUPT( memSize != 0 );

    void *mem = DbgMallocNative( memSize );

    if ( !mem )
        throw std::bad_alloc();

    return mem;
}

void* operator new[]( size_t memSize, const std::nothrow_t ) throw()
{
    MEM_INTERRUPT( memSize != 0 );

    return DbgMallocNative( memSize );
}

void operator delete( void *ptr ) throw()
{
    DbgFreeNative( ptr );
}

void operator delete[]( void *ptr ) throw()
{
    DbgFreeNative( ptr );
}

bool DbgAllocGetSize( void *ptr, size_t& sizeOut )
{
    return DbgAllocGetSizeNative( ptr, sizeOut );
}

void* DbgMalloc( size_t size )
{
    MEM_INTERRUPT( size != 0 );

    return DbgMallocNative( size );
}

void* DbgRealloc( void *ptr, size_t size )
{
    MEM_INTERRUPT( size != 0 );

    return DbgReallocNative( ptr, size );
}

void DbgFree( void *ptr )
{
    if ( ptr != NULL )
    {
        DbgFreeNative( ptr );
    }
}

#endif

// DebugHeap initializator routine.
// Call it before CRT initialization.
void DbgHeap_Init( void )
{
#ifdef USE_HEAP_DEBUGGING
    GetSystemInfo( &g_systemInfo );

    // Initialize watch callbacks.
    _memAllocWatchCallback = NULL;

    _nativeAlloc = new (malloc(sizeof(DebugFullPageHeapAllocator))) DebugFullPageHeapAllocator();

    _win32_initHeap();

    LIST_CLEAR( _dbgAllocBlocks.root );
#endif
}

// DebugHeap memory validation routine.
// Call it if you want to check for memory corruption globally.
void DbgHeap_Validate( void )
{
#ifdef USE_HEAP_DEBUGGING
    _win32_validateMemory();
#endif
}

// DebugHeap memory checkup routine.
// Loops through all memory blocks and tells you about their callstacks.
// Use this in combination with breakpoints.
#pragma optimize("", off)

void DbgHeap_CheckActiveBlocks( void )
{
#ifdef USE_HEAP_DEBUGGING
    // First we must verify that our memory is in a valid state.
    _win32_validateMemory();

#ifdef USE_HEAP_STACK_TRACE
    // Now loop through all blocks.
    LIST_FOREACH_BEGIN( _debugBlockHeader, _dbgAllocBlocks.root, node )

        const std::string& callstack = item->callStackPrint;

        __asm nop       // PUT BREAKPOINT HERE.

    LIST_FOREACH_END
#endif //USE_HEAP_STACK_TRACE

#endif //USE_HEAP_DEBUGGING
}

#pragma optimize("", on)

// DebugHeap memory callback routines.
// Call these to set specific callbacks for memory watching.
void DbgHeap_SetMemoryAllocationWatch( pfnMemoryAllocWatch allocWatchCallback )
{
#ifdef USE_HEAP_DEBUGGING
    _memAllocWatchCallback = allocWatchCallback;
#endif
}

// DebugHeap termination routine.
// Call it after the CRT has terminated itself.
void DbgHeap_Shutdown( void )
{
#ifdef USE_HEAP_DEBUGGING
    _win32_shutdownHeap();
#endif
}

// Alternative entry point.
extern "C"
{
extern int mainCRTStartup( void );
extern int WinMainCRTStartup( void );
}

extern "C" int APIENTRY _DebugInit( void )
{
    DbgHeap_Init();

#ifdef _DEBUG_TRACE_LIBRARY_
    DbgTraceStackSpace stackSpace;  // reserved memory; must be always allocated.

    DbgTrace_Init( stackSpace );
#endif

    int ret = mainCRTStartup();

#ifdef _DEBUG_TRACE_LIBRARY_
    DbgTrace_Shutdown();
#endif
    DbgHeap_Shutdown();
    return ret;
}

#if 0

extern "C" int APIENTRY _Win32DebugInit( void )
{
    DbgHeap_Init();

#if 0
    DbgTraceStackSpace stackSpace;  // reserved memory; must be always allocated.

    DbgTrace_Init( stackSpace );

    // Set up memory debugging routines.
    DbgHeap_SetMemoryAllocationWatch( _DbgHeap_MemAllocWatch );
#endif

    int ret = WinMainCRTStartup();

#if 0
    DbgTrace_Shutdown();
#endif
    DbgHeap_Shutdown();
    return ret;
}

#endif