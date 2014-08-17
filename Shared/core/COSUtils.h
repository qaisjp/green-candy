/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        Shared/core/COSUtils.h
*  PURPOSE:     Implementation dependant routines for native features
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#ifndef _COMMON_OS_UTILS_
#define _COMMON_OS_UTILS_

#include "CFileSystem.common.h"
//#include "MemoryUtils.h"

// Namespace of abstraction types to be used by any NativePageAllocator implementation.
namespace NativePageAllocation
{
    // Abstraction page handle information.
    struct pageHandleInfo
    {
        void *pAddress;
        size_t memSize;
    };

    // Abstraction memory region information.
    struct pageInfo
    {
        void *pAddress;
        size_t regionSize;
        bool isWritable;
        bool isReadable;
        bool isExecutable;
    };

    // Template struct for NativePageAllocator collision detection.
    struct pageCollider
    {
        // Returns true if the pageHandle request may intersect the page.
        AINLINE bool OnPageCollision( const pageHandleInfo& reqHandle, const pageInfo& info )
        {
            // It can by default.
            return true;
        }
    };
};

// Native OS memory allocation manager that marks pages on RAM to be used by the program.
// This implementation should be used if the API exposed by the OS is not enough for you (malloc, etc).
template <typename colliderType>
struct NativePageAllocator
{
    // Collision detection type.
    colliderType collDetect;

    // Private memory heap to prevent unwanted recursion.
    struct privateAllocator
    {
        inline void* Allocate( size_t memSize, unsigned int )
        {
            return malloc( memSize );
        }

        inline void* Realloc( void *memPtr, size_t memSize, unsigned int )
        {
            return realloc( memPtr, memSize );
        }

        inline void Free( void *memPtr )
        {
            free( memPtr );
        }
    };

    privateAllocator _privateAlloc;

    SYSTEM_INFO _systemInfo;

    inline NativePageAllocator( void )
    {
        GetSystemInfo( &_systemInfo );

        LIST_CLEAR( activeMemoryRanges.root );
        LIST_CLEAR( sortedMemoryRanges.root );
        LIST_CLEAR( activeHandles.root );
    }

    inline ~NativePageAllocator( void )
    {
        // Delete all active page handles.
        while ( !LIST_EMPTY( activeHandles.root ) )
        {
            pageHandle *handle = LIST_GETITEM( pageHandle, activeHandles.root.next, managerNode );

            Free( handle );
        }

        // Now delete any active pages.
        while ( !LIST_EMPTY( activeMemoryRanges.root ) )
        {
            pageAllocation *allocation = LIST_GETITEM( pageAllocation, activeMemoryRanges.root.next, managerNode );

            DeletePageAllocation( allocation );
        }
    }

    typedef sliceOfData <SIZE_T> memBlockSlice_t;

    struct pageAllocation;

    typedef iterativeGrowableArrayEx <pageAllocation*, 8, 0, unsigned int, privateAllocator> residingMemBlocks_t;

    struct pageHandle
    {
        inline pageHandle( PVOID memRangeStart, SIZE_T memRangeSize )
            : requestedMemory( (SIZE_T)memRangeStart, memRangeSize )
        {
            return;
        }

        inline void* GetTargetPointer( void ) const     { return (void*)requestedMemory.GetSliceStartPoint(); }
        inline size_t GetTargetSize( void ) const       { return (size_t)requestedMemory.GetSliceSize(); }

        memBlockSlice_t requestedMemory;    // slice that represents memory that can be accessed by the application

        residingMemBlocks_t residingMemBlocks;  // the memory blocks we are part

        RwListEntry <pageHandle> managerNode;   // entry in the active page handle list
    };

    RwList <pageHandle> activeHandles;

    struct pageAllocation
    {
        inline pageAllocation( LPVOID pageAddress, SIZE_T allocSize )
            : pageSpan( (SIZE_T)pageAddress, allocSize )
        {
            this->pageAddress = pageAddress;
            this->allocSize = allocSize;

            this->refCount = 0;
        }

        inline ~pageAllocation( void )
        {
            BOOL freeSuccess = VirtualFree( this->pageAddress, 0, MEM_RELEASE );

            assert( freeSuccess == TRUE );
        }

        inline void RegisterPageHandle( pageHandle *handle )
        {
            this->refCount++;

            handle->residingMemBlocks.AddItem( this );
        }

        inline void UnregisterPageHandle( pageHandle *handle )
        {
            handle->residingMemBlocks.RemoveItem( this );

            this->refCount--;
        }

        inline bool IsBlockBeingUsed( void ) const
        {
            return ( refCount != 0 );
        }

        LPVOID pageAddress;     // address of the memory block
        SIZE_T allocSize;       // number in bytes for the allocation range

        memBlockSlice_t pageSpan;       // slice which spans the allocation range

        unsigned int refCount;          // number of handles using this page

#if 0
        typedef iterativeGrowableArrayEx <pageHandle*, 8, 0, unsigned int, privateAllocator> handleList_t;

        handleList_t handleList;    // list of pageHandle's that reference this allocation
#endif

        RwListEntry <pageAllocation> managerNode;   // node in the NativePageAllocator allocation list.
        RwListEntry <pageAllocation> sortedNode;
    };

    inline SIZE_T GetPageAllocationRange( size_t spanSize )
    {
        // Returns a rounded up value that determines the actual size of a page allocation.
        return ALIGN( (SIZE_T)spanSize, (SIZE_T)_systemInfo.dwPageSize, (SIZE_T)_systemInfo.dwPageSize );
    }

    RwList <pageAllocation> activeMemoryRanges;
    RwList <pageAllocation> sortedMemoryRanges;

    inline void SortedMemoryBlockInsert( pageAllocation *memBlock )
    {
        RwListEntry <pageAllocation> *insertAfter = &sortedMemoryRanges.root;

        SIZE_T insertMemBlockAddress = (SIZE_T)memBlock->pageAddress;

        LIST_FOREACH_BEGIN( pageAllocation, sortedMemoryRanges.root, sortedNode )
            // Get the address of the list item as number.
            SIZE_T memBlockAddress = (SIZE_T)item->pageAddress;

            if ( memBlockAddress < insertMemBlockAddress )
            {
                insertAfter = iter;
                break;
            }
        LIST_FOREACH_END

        LIST_INSERT( *insertAfter, memBlock->sortedNode );
    }

    inline pageAllocation* CreatePageAllocation( LPVOID pageAddress, SIZE_T allocSize )
    {
        pageAllocation *memRange = new (_privateAlloc.Allocate( sizeof( pageAllocation ), 0 )) pageAllocation( pageAddress, allocSize );

        if ( memRange )
        {
            LIST_INSERT( activeMemoryRanges.root, memRange->managerNode );
            SortedMemoryBlockInsert( memRange );
        }

        return memRange;
    }

    inline void DeletePageAllocation( pageAllocation *memRange )
    {
        LIST_REMOVE( memRange->sortedNode );
        LIST_REMOVE( memRange->managerNode );

        memRange->~pageAllocation();

        _privateAlloc.Free( memRange );
    }

    inline bool AcquireNewPages( LPVOID pDesiredAddress, size_t sSpanSize, residingMemBlocks_t& blocksOut )
    {
        bool success = false;
        {
            SIZE_T actualAllocationSize = GetPageAllocationRange( sSpanSize );

            // Attempt to allocate a number of pages in a row.
            LPVOID pageAddress = VirtualAlloc(
                pDesiredAddress,
                actualAllocationSize, MEM_RESERVE | MEM_COMMIT,
                PAGE_READWRITE
            );

            if ( pageAddress != NULL )
            {
                // Register a page allocation.
                pageAllocation *allocation = CreatePageAllocation( pageAddress, actualAllocationSize );

                if ( allocation )
                {
                    blocksOut.AddItem( allocation );

                    success = true;
                }
                
                if ( !success )
                {
                    // We failed to register ourselves, so deallocate the pages.
                    VirtualFree( pageAddress, actualAllocationSize, MEM_RELEASE );
                }
            }
        }
        return success;
    }

    inline bool PageAcquireHelper(
        LPVOID pDesiredAddress, size_t sSpanSize,
        residingMemBlocks_t& allocatedBlocks,
        bool& addedToAllocatedBlocks,
        residingMemBlocks_t& hostBlocks,
        bool& addedToHostBlocks )
    {
        residingMemBlocks_t _newlyAllocatedPages;

        bool newPageSuccess = AcquireNewPages( pDesiredAddress, sSpanSize, _newlyAllocatedPages );

        if ( newPageSuccess )
        {
            for ( unsigned int n = 0; n < _newlyAllocatedPages.GetCount(); n++ )
            {
                pageAllocation *thePage = _newlyAllocatedPages.GetFast( n );

                hostBlocks.AddItem( thePage );
                allocatedBlocks.AddItem( thePage );
            }

            addedToAllocatedBlocks = true;

            addedToHostBlocks = true;
        }

        return newPageSuccess;
    }

    inline bool ReserveMemoryOnRegion(
        memBlockSlice_t& requestedMemRange,
        residingMemBlocks_t& allocatedPages, bool& allocatedNewPage,
        residingMemBlocks_t& hostPages, bool& hasHostPages )
    {
        // Copy important intersection parameters.
        void *desiredAddress = (void*)requestedMemRange.GetSliceStartPoint();
        size_t spanSize = (size_t)requestedMemRange.GetSliceSize();

        bool validAllocation = true;

        // Create a private copy of the sorted memory ranges list.
        // We assume that we are the only thread that modifies this list.
        residingMemBlocks_t privateSortedMemoryRanges;

        LIST_FOREACH_BEGIN( pageAllocation, sortedMemoryRanges.root, managerNode )
            privateSortedMemoryRanges.AddItem( item );
        LIST_FOREACH_END

        bool operationComplete = false;

        for ( unsigned int n = 0; n < privateSortedMemoryRanges.GetCount(); n++ )
        {
            pageAllocation *item = privateSortedMemoryRanges.GetFast( n );

            // Default operation switches.
            bool allocateMissingBeforePages = false;
            bool requireCurrentPage = false;
            bool requiresConsecutivePage = false;

            // Collide both slices.
            memBlockSlice_t::eIntersectionResult intResult = requestedMemRange.intersectWith( item->pageSpan );

            // If there was a collision, check if it is allowed.
            // If not, then we have to abort with an invalid allocation request.
            if ( !memBlockSlice_t::isFloatingIntersect( intResult ) && intResult != memBlockSlice_t::INTERSECT_UNKNOWN )
            {
                // By default, we can intersect.
                bool allowIntersect = true;
                {
                    // Ask the collider if we can intersect.
                    NativePageAllocation::pageHandleInfo handleInfo;
                    handleInfo.pAddress = desiredAddress;
                    handleInfo.memSize = spanSize;

                    NativePageAllocation::pageInfo pageInfo;
                    pageInfo.pAddress = (void*)item->pageAddress;
                    pageInfo.regionSize = (size_t)item->allocSize;
                    // todo: add page information.

                    allowIntersect = collDetect.OnPageCollision( handleInfo, pageInfo );
                }
                if ( !allowIntersect )
                {
                    // We have encountered an invalid request and must terminate.
                    validAllocation = false;
                    break;
                }
            }

            if ( intResult == memBlockSlice_t::INTERSECT_INSIDE ||
                 intResult == memBlockSlice_t::INTERSECT_EQUAL )
            {
                // Since we either are inside the host page or are exactly the same region,
                // we can include it and leave.
                requireCurrentPage = true;
                operationComplete = true;
            }
            else if ( intResult == memBlockSlice_t::INTERSECT_BORDER_END )
            {
                // There are pages before any currently allocated page that require validation before
                // we can proceed.
                allocateMissingBeforePages = true;

                // Since the allocation ends in the current page (which is already allocated),
                // we can include it and terminate.
                requireCurrentPage = true;
                operationComplete = true;
            }
            else if ( intResult == memBlockSlice_t::INTERSECT_BORDER_START )
            {
                // The allocation request has started in the current (allocated) page and
                // proceeds into unknown territory from here on.
                // Add the current page into the host pages list and make sure that each page
                // on the way to the end is allocated.
                requireCurrentPage = true;
                requiresConsecutivePage = true;
            }
            else if ( intResult == memBlockSlice_t::INTERSECT_ENCLOSING )
            {
                // The allocation request region encloses the current resident page. We must
                // (conditionally) handle region before and after the allocated blocks (if
                // these ranges are not 0).
                if ( requestedMemRange.GetSliceStartPoint() != item->pageSpan.GetSliceStartPoint() )
                {
                    // We have memory before the resident page.
                    allocateMissingBeforePages = true;
                }
                
                requireCurrentPage = true;

                if ( requestedMemRange.GetSliceEndPoint() != item->pageSpan.GetSliceEndPoint() )
                {
                    // We need to be looking out for additional pages after us.
                    requiresConsecutivePage = true;
                }
                else
                {
                    // We have actually finished operation, because there cannot be anything after us.
                    operationComplete = true;
                }
            }
            else if ( intResult == memBlockSlice_t::INTERSECT_FLOATING_END )
            {
                // We have not reached the page yet that can host our memory request.
                // We should just continue searching.
            }
            else if ( intResult == memBlockSlice_t::INTERSECT_FLOATING_START )
            {
                // We could not find a page that can host us.
                // The memory region has to be allocated entirely new.
                operationComplete = true;
            }
            else
            {
                // It is unknown how to handle this event.
                // It should never happen.
                assert( 0 );
            }

            if ( allocateMissingBeforePages )
            {
                SIZE_T invalidRegionStart = requestedMemRange.GetSliceStartPoint();
                SIZE_T invalidRegionEnd = item->pageSpan.GetSliceStartPoint();

                size_t invalidRegionSpan = (size_t)( invalidRegionEnd - invalidRegionStart );
                void *invalidRegionPtr = (void*)invalidRegionStart;

                bool validationSuccess = PageAcquireHelper(
                    invalidRegionPtr, invalidRegionSpan,
                    allocatedPages, allocatedNewPage,
                    hostPages, hasHostPages
                );

                if ( !validationSuccess )
                {
                    validAllocation = false;
                    break;
                }
            }

            // Include the current page into the host pages list.
            if ( requireCurrentPage )
            {
                hostPages.AddItem( item );

                hasHostPages = true;
            }

            // We have finished our operation..?
            if ( operationComplete )
            {
                // Since we have finished, we do not require anything else anymore.
                requiresConsecutivePage = false;
                break;
            }

            // We have to continue and allocate after our current page.
            if ( requiresConsecutivePage )
            {
                requestedMemRange.SetSliceStartPoint( item->pageSpan.GetSliceEndPoint() + (SIZE_T)1 );
            }
        }

        // Check whether we have completed our operation.
        if ( !operationComplete )
        {
            // We (still) need to allocate the region that is defined by requestedMemRange!
            SIZE_T invalidRegionStart = requestedMemRange.GetSliceStartPoint();
            SIZE_T invalidRegionEnd = requestedMemRange.GetSliceEndPoint();

            size_t invalidRegionSpan = (size_t)( invalidRegionEnd - invalidRegionStart );
            void *invalidRegionPtr = (void*)invalidRegionStart;

            bool validationSuccess = PageAcquireHelper(
                invalidRegionPtr, invalidRegionSpan,
                allocatedPages, allocatedNewPage,
                hostPages, hasHostPages
            );

            if ( !validationSuccess )
            {
                // Fail.
                validAllocation = false;
            }
        }

        return validAllocation;
    }

    inline pageHandle* Allocate( void *desiredAddress, size_t spanSize )
    {
        pageHandle *theHandle = NULL;
        {
            // Determine the pages that should host the requested memory region.
            residingMemBlocks_t allocatedPages;
            bool allocatedNewPage = false;
            residingMemBlocks_t hostPages;
            bool hasHostPages = false;
            bool validAllocation = true;

            if ( !hasHostPages )
            {
                // If we know the address we should allocate on, we attempt to find regions that have already been allocated
                // so they can host our memory.
                if ( desiredAddress != NULL )
                {
                    memBlockSlice_t requestedMemRange( (SIZE_T)desiredAddress, (SIZE_T)spanSize );

                    validAllocation = ReserveMemoryOnRegion(
                        requestedMemRange,
                        allocatedPages, allocatedNewPage,
                        hostPages, hasHostPages
                    );
                }
            }

            if ( validAllocation )
            {
                if ( !hasHostPages )
                {
                    PageAcquireHelper(
                        desiredAddress, spanSize,
                        allocatedPages, allocatedNewPage,
                        hostPages, hasHostPages
                    );
                }

                if ( hasHostPages )
                {
                    // Make sure we have a valid address.
                    if ( desiredAddress == NULL )
                    {
                        // If it is NULL, it should be set to the address of the first page.
                        desiredAddress = hostPages.Get( 0 )->pageAddress;
                    }

                    // Create a pageHandle to it.
                    pageHandle *newHandle = new (_privateAlloc.Allocate( sizeof( pageHandle ), 0 )) pageHandle( desiredAddress, (SIZE_T)spanSize );

                    if ( newHandle )
                    {
                        // Register it inside the host pages.
                        for ( unsigned int n = 0; n < hostPages.GetCount(); n++ )
                        {
                            pageAllocation *allocation = hostPages.GetFast( n );

                            allocation->RegisterPageHandle( newHandle );
                        }

                        // Register the handle.
                        LIST_INSERT( activeHandles.root, newHandle->managerNode );

                        theHandle = newHandle;
                    }
                }
            }

            if ( theHandle == NULL )
            {
                if ( allocatedNewPage )
                {
                    // Delete all allocated pages.
                    for ( unsigned int n = 0; n < allocatedPages.GetCount(); n++ )
                    {
                        pageAllocation *thePage = allocatedPages.GetFast( n );

                        DeletePageAllocation( thePage );
                    }
                }
            }
        }

        return theHandle;
    }

    inline pageHandle* FindHandleByAddress( void *pAddress )
    {
        // Just compare addresses of every alive handle and return
        // the one that matches the query.
        LIST_FOREACH_BEGIN( pageHandle, activeHandles.root, managerNode )
            if ( item->GetTargetPointer() == pAddress )
            {
                return item;
            }
        LIST_FOREACH_END

        return NULL;
    }

    // Attempts to update the handle size so that either more or less memory
    // can be used.
    inline bool SetHandleSize( pageHandle *theHandle, size_t newReserveSize )
    {
        return false;
    }

    inline void MemBlockGarbageCollection( pageAllocation *memBlock )
    {
        // If the page is not being used anymore, release it.
        if ( !memBlock->IsBlockBeingUsed() )
        {
            DeletePageAllocation( memBlock );
        }
    }

    inline void Free( pageHandle *memRange )
    {
        // Free the link to the allocated OS memory regions.
        while ( memRange->residingMemBlocks.GetCount() != 0 )
        {
            pageAllocation *memBlock = memRange->residingMemBlocks.Get( 0 );

            memBlock->UnregisterPageHandle( memRange );

            // Clean up memory blocks that are not used anymore.
            MemBlockGarbageCollection( memBlock );
        }

        // Delete and unregister our pageHandle.
        LIST_REMOVE( memRange->managerNode );

        memRange->~pageHandle();

        _privateAlloc.Free( memRange );
    }

    inline bool FreeByAddress( void *pAddress )
    {
        pageHandle *theHandle = FindHandleByAddress( pAddress );

        if ( !theHandle )
            return false;

        Free( theHandle );
        return true;
    }
};

#endif //_COMMON_OS_UTILS_