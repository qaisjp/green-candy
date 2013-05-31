/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        game_sa/CRenderWareSA.mem.cpp
*  PURPOSE:     RenderWare memory management implementation
*  DEVELOPERS:  Martin Turski <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*  RenderWare is © Criterion Software
*
*****************************************************************************/

#include <StdInc.h>

// This basically is a complete rewrite of the internal RenderWare memory management.

static bool _allowPreallocation = false;

#define DIV_CEIL( num, sector ) (((num) + ((sector) - 1)) / (sector))
#define ALIGN_SIZE( num, sector ) ( ALIGN( (num), (sector), (sector) ) )

__forceinline void _DeallocFreeList( RwFreeList *list )
{
    // free the list again.
    LIST_FOREACH_BEGIN( RwFreeListMemBlock, list->m_memBlocks.root, list )
        LIST_REMOVE( item->list );

        (*ppRwInterface)->m_free( item );
    LIST_FOREACH_END

    if ( list->m_flags == RWALLOC_SELFCONSTRUCT )
        return;

    RwFreeList *freeListInfo = *(RwFreeList**)0x00C9A634;

    if ( list != freeListInfo && freeListInfo )
    {
        (*ppRwInterface)->m_freeStruct( freeListInfo, list );
        return;
    }

    (*ppRwInterface)->m_free( list );
    return;
}

void __cdecl RwFreeListSetPreallocationEnabled( unsigned int enabled )
{
    // Determines whether free lists may preallocate memory during construction.
    _allowPreallocation = enabled != 0;
}

__forceinline RwFreeListMemBlock* _RwFreeListAllocateBlock( RwFreeList *list, unsigned int blockCount, size_t blockSize, size_t metaDataSize, size_t alignment, unsigned int allocFlags )
{
    const size_t allocSize = blockCount * blockSize + metaDataSize + (alignment + 7);

    // Blocks contain a header, metaData and the actual data blocks.
    // The sections are placed in said order.
    RwFreeListMemBlock *block = (RwFreeListMemBlock*)(*ppRwInterface)->m_malloc( allocSize, allocFlags );

    if ( !block )
    {
        _DeallocFreeList( list );
        return NULL;
    }

    LIST_INSERT( list->m_memBlocks.root, block->list );

    void *memBuff = block + 1;

    // Initialize the meta data
    memset( memBuff, 0, metaDataSize );
    return block;
}

RwFreeList* __cdecl RwFreeListCreateEx( unsigned int dataSize, unsigned int blockCount, size_t alignment, unsigned int allocCount, RwFreeList *list, unsigned int allocFlags )
{
    if ( !_allowPreallocation )
        allocCount = 0;

    if ( !alignment )
        alignment = 4;

    if ( !list )
    {
        if ( RwFreeList *freeListInfo = *(RwFreeList**)0x00C9A634 )
            list = (RwFreeList*)(*ppRwInterface)->m_allocStruct( freeListInfo, allocFlags & 0xFF0000 );
        else
            list = (RwFreeList*)(*ppRwInterface)->m_malloc( sizeof(RwFreeList), allocFlags & 0xFF0000 );

        if ( !list )
            return NULL;

        list->m_flags = RWALLOC_RUNTIME;
    }
    else
        list->m_flags = RWALLOC_RUNTIME | RWALLOC_SELFCONSTRUCT;

    size_t blockSize = ALIGN_SIZE( dataSize, alignment );
    size_t metaDataSize = DIV_CEIL( blockCount, 8 );

    list->m_blockSize = blockSize;
    list->m_blockCount = blockCount;
    list->m_metaDataSize = metaDataSize;
    list->m_alignment = alignment;
    LIST_CLEAR( list->m_memBlocks.root );

    while ( allocCount )
    {
        _RwFreeListAllocateBlock( list, blockCount, blockSize, metaDataSize, alignment, allocFlags );

        allocCount--;
    }

    LIST_INSERT( ((RwList <RwFreeList>*)0x00C9A604)->root, list->m_globalLists );
    return list;
}

RwFreeList* __cdecl RwFreeListCreate( size_t dataSize, unsigned int blockCount, size_t alignment, unsigned int allocFlags )
{
    return RwFreeListCreateEx( dataSize, blockCount, alignment, 1, NULL, allocFlags );
}

unsigned int __cdecl RwFreeListDestroy( RwFreeList *list )
{
    LIST_REMOVE( list->m_globalLists );

    _DeallocFreeList( list );
    return 1;
}

void __cdecl RwFreeListSetFlags( RwFreeList *list, unsigned int flags )
{
    list->m_flags = flags & ~1;
}

unsigned int __cdecl RwFreeListGetFlags( RwFreeList *list )
{
    return list->m_flags;
}

#define REVERSE_BIT_FIELD( cnt ) ( 0x80 >> (cnt) )
#define IS_BIT_SET_REVERSE( num, cnt ) ( IS_ANY_FLAG( (num), REVERSE_BIT_FIELD( cnt ) ) )

__forceinline void* __cdecl RwFreeListBlockAllocateChunk( RwFreeListMemBlock *block, unsigned int blockCount, size_t blockSize, size_t alignment, size_t metaDataSize )
{
    unsigned char *metaData = block->GetMetaData();
    
    for ( unsigned int i = 0; i < metaDataSize; i++ )
    {
        unsigned char byte = metaData[i];

        if ( byte == 0xFF )
        {
            blockCount -= 8;
            continue;
        }

        for ( unsigned int j = 0; j < 8; j++, blockCount-- )
        {
            if ( blockCount == 0 )
                break;  // was an error introduced here because of not wanting to use goto?

            unsigned char checkAgainst = REVERSE_BIT_FIELD( j );

            if ( !IS_ANY_FLAG( byte, checkAgainst ) )
            {
                // Mark as used.
                byte |= checkAgainst;
                metaData[i] = byte;

                // Return the chunk
                if ( void *data = block->GetBlockPointer( i * 8 + j, blockSize, alignment, metaDataSize ) )
                    return data;
            }
        }
    }

    return NULL;
}

void* __cdecl RwFreeListAllocate( RwFreeList *list, unsigned int allocFlags )
{
    RwList <RwFreeListMemBlock>& root = list->m_memBlocks;
    size_t metaDataSize = list->m_metaDataSize;
    size_t blockSize = list->m_blockSize;
    size_t alignment = list->m_alignment;

    LIST_FOREACH_BEGIN( RwFreeListMemBlock, root.root, list )
        if ( void *data = RwFreeListBlockAllocateChunk( item, list->m_blockCount, blockSize, alignment, metaDataSize ) )
            return data;
    LIST_FOREACH_END

    RwFreeListMemBlock *block = _RwFreeListAllocateBlock( list, list->m_blockCount, blockSize, metaDataSize, alignment, allocFlags );

    // Allocate first chunk
    block->SetBlockUsed( 0 );
    return block->GetBlockPointer( 0, blockSize, alignment, metaDataSize );
}

__forceinline RwFreeListMemBlock* _RwFreeListGetValidBlock( RwFreeList *list, void *ptr, size_t metaDataSize, size_t blockSize, size_t alignment, void*& firstBlock )
{
    RwList <RwFreeListMemBlock>& root = list->m_memBlocks;
    
    // Check whether block pointer is valid.
    LIST_FOREACH_BEGIN( RwFreeListMemBlock, root.root, list )
        void *fBlock = item->GetBlockPointer( 0, blockSize, alignment, metaDataSize );

        if ( ptr >= fBlock &&
             ptr <= (unsigned char*)fBlock + list->m_blockCount * blockSize )
        {
            firstBlock = fBlock;
            return item;
        }
    LIST_FOREACH_END

    return NULL;
}

__forceinline bool RwFreeListBlockFree( RwFreeListMemBlock *block, size_t metaDataSize, unsigned char *metaData )
{
    // Free the block if possible (that is, it ain't used anymore)
    for ( unsigned int n = 0; n < metaDataSize; n++ )
    {
        if ( metaData[n] != 0 )
            return false;
    }

    LIST_REMOVE( block->list );

    (*ppRwInterface)->m_free( block );
    return true;
}

RwFreeList* __cdecl RwFreeListFree( RwFreeList *list, void *ptr )
{
    size_t metaDataSize = list->m_metaDataSize;
    size_t blockSize = list->m_blockSize;
    size_t alignment = list->m_alignment;

    void *firstBlock;
    RwFreeListMemBlock *memBlock = _RwFreeListGetValidBlock( list, ptr, metaDataSize, blockSize, alignment, firstBlock );

    if ( !memBlock )
        return NULL;    // failed to find valid block

    unsigned int blockOffset = (char*)ptr - (char*)firstBlock;
    unsigned int blockIndex = blockOffset / list->m_blockSize;

    unsigned char *metaData = memBlock->GetMetaData();

    // Deallocate the block
    RwFreeListMemBlock::SetBlockUsed( metaData, blockIndex, false );

    if ( list->m_flags & RWALLOC_RUNTIME )
        RwFreeListBlockFree( memBlock, metaDataSize, metaData );

    return list;
}

// todo: for all blocks function (US: 0x00801E90)

size_t __cdecl RwFreeListPurge( RwFreeList *list )
{
    RwList <RwFreeListMemBlock>& root = list->m_memBlocks;
    size_t metaDataSize = list->m_metaDataSize;
    unsigned int purgedItems = 0;

    LIST_FOREACH_BEGIN( RwFreeListMemBlock, root.root, list )
        LIST_REMOVE( item->list );

        if ( RwFreeListBlockFree( item, metaDataSize, item->GetMetaData() ) )
            purgedItems++;
        else
        {
            LIST_INSERT( root.root, item->list );
        }
    LIST_FOREACH_END

    return purgedItems * list->m_blockSize;
}

size_t __cdecl RwFreeListPurgeAll( void )
{
    size_t totalPurged = 0;

    LIST_FOREACH_BEGIN( RwFreeList, ((RwList <RwFreeList>*)0x00C9A604)->root, m_globalLists )
        size_t purged = RwFreeListPurge( item );

        if ( purged != 0 )
            totalPurged += purged;
    LIST_FOREACH_END

    return totalPurged;
}

void RenderWareMem_Init( void )
{
    switch( pGame->GetGameVersion() )
    {
    case VERSION_EU_10:
        HookInstall( 0x008019B0, (DWORD)RwFreeListSetPreallocationEnabled, 5 );
        HookInstall( 0x008019C0, (DWORD)RwFreeListCreate, 5 );
        HookInstall( 0x008019F0, (DWORD)RwFreeListCreateEx, 5 );
        HookInstall( 0x00801C50, (DWORD)RwFreeListSetFlags, 5 );
        HookInstall( 0x00801C60, (DWORD)RwFreeListGetFlags, 5 );
        HookInstall( 0x00801C70, (DWORD)RwFreeListAllocate, 5 );
        HookInstall( 0x00801d90, (DWORD)RwFreeListFree, 5 );
        HookInstall( 0x00801E40, (DWORD)RwFreeListPurge, 5 );
        HookInstall( 0x00801FD0, (DWORD)RwFreeListPurgeAll, 5 );
        break;
    case VERSION_US_10:
        HookInstall( 0x00801970, (DWORD)RwFreeListSetPreallocationEnabled, 5 );
        HookInstall( 0x00801980, (DWORD)RwFreeListCreate, 5 );
        HookInstall( 0x008019B0, (DWORD)RwFreeListCreateEx, 5 );
        HookInstall( 0x00801C10, (DWORD)RwFreeListSetFlags, 5 );
        HookInstall( 0x00801C20, (DWORD)RwFreeListGetFlags, 5 );
        HookInstall( 0x00801C30, (DWORD)RwFreeListAllocate, 5 );
        HookInstall( 0x00801D50, (DWORD)RwFreeListFree, 5 );
        HookInstall( 0x00801E00, (DWORD)RwFreeListPurge, 5 );
        HookInstall( 0x00801F90, (DWORD)RwFreeListPurgeAll, 5 );
        break;
    }
}

void RenderWareMem_Shutdown( void )
{
}
