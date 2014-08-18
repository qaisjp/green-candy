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
    // WARNING: this class is NOT thread-safe!

    // Collision detection type.
    // Used to define rules of allocation between handles and pages, effectively defining intersections
    // (exclusive pages, read/write barriers, etc).
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

        residingMemBlocks_t residingMemBlocks;  // the memory blocks we are part of, have to be sorted by address!

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

            if ( memBlockAddress > insertMemBlockAddress )
            {
                break;
            }

            insertAfter = iter;
        LIST_FOREACH_END

        LIST_APPEND( *insertAfter, memBlock->sortedNode );
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
            else
            {
                // We have encountered an error while allocating the page.
                DWORD errorCode = GetLastError();

                __asm nop
                
                if ( pDesiredAddress == NULL )
                {
                    // Out of memory error!!!!!!!
                    __asm nop
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

    template <typename callbackType>
    inline void ProcessSortedPages(
        memBlockSlice_t& requestedMemRange,
        const residingMemBlocks_t& privateSortedMemoryRanges,
        callbackType& cb )
    {
        bool operationComplete = false;
        bool requiresConsecutivePage = false;

        for ( unsigned int n = 0; n < privateSortedMemoryRanges.GetCount(); n++ )
        {
            pageAllocation *item = privateSortedMemoryRanges.GetFast( n );

            // Collide both slices.
            memBlockSlice_t::eIntersectionResult intResult = requestedMemRange.intersectWith( item->pageSpan );

            // Notify the callback.
            {
                bool canContinue = cb.OnPageIntersection( requestedMemRange, item, intResult );

                if ( !canContinue )
                    break;
            }

            if ( intResult == memBlockSlice_t::INTERSECT_INSIDE ||
                 intResult == memBlockSlice_t::INTERSECT_EQUAL )
            {
                // Since we either are inside the host page or are exactly the same region,
                // we can include it and leave.
                operationComplete = true;
            }
            else if ( intResult == memBlockSlice_t::INTERSECT_BORDER_END )
            {
                // Since the allocation ends in the current page (which is already allocated),
                // we can include it and terminate.
                operationComplete = true;
            }
            else if ( intResult == memBlockSlice_t::INTERSECT_BORDER_START )
            {
                // The allocation request has started in the current (allocated) page and
                // proceeds into unknown territory from here on.
                // Add the current page into the host pages list and make sure that each page
                // on the way to the end is allocated.
                requiresConsecutivePage = true;
            }
            else if ( intResult == memBlockSlice_t::INTERSECT_ENCLOSING )
            {
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
                break;
            }
            else
            {
                // It is unknown how to handle this event.
                // It should never happen.
                assert( 0 );
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
            cb.OnIncompleteOperation( requestedMemRange );
        }
    }

    struct ReserveMemorySelector
    {
        void *desiredAddress;
        size_t spanSize;

        NativePageAllocator& manager;
        bool validAllocation;
        residingMemBlocks_t allocatedPages; bool allocatedNewPage;
        residingMemBlocks_t& hostPages; bool& hasHostPages;

        inline ReserveMemorySelector(
            void *desiredAddress, size_t spanSize,
            residingMemBlocks_t& hostPages, bool& hasHostPages,
            NativePageAllocator& manager ) : hostPages( hostPages ), hasHostPages( hasHostPages ), manager( manager )
        {
            this->desiredAddress = desiredAddress;
            this->spanSize = spanSize;

            this->validAllocation = true;
        }

        inline ~ReserveMemorySelector( void )
        {
            if ( !IsValidAllocation() )
            {
                if ( allocatedNewPage )
                {
                    for ( unsigned int n = 0; n < allocatedPages.GetCount(); n++ )
                    {
                        pageAllocation *thePage = allocatedPages.GetFast( n );

                        manager.DeletePageAllocation( thePage );
                    }
                }
            }
        }

        inline bool IsValidAllocation( void ) const
        {
            return this->validAllocation;
        }

        inline bool OnPageIntersection(
            const memBlockSlice_t& concurrentRequest, pageAllocation *thePage,
            memBlockSlice_t::eIntersectionResult intResult
        )
        {
            // If there was a collision, check if it is allowed.
            // If not, then we have to abort with an invalid allocation request.
            if ( !memBlockSlice_t::isFloatingIntersect( intResult ) && intResult != memBlockSlice_t::INTERSECT_UNKNOWN )
            {
                // By default, we can intersect.
                bool allowIntersect = true;
                {
                    // Ask the collider if we can intersect.
                    NativePageAllocation::pageHandleInfo handleInfo;
                    handleInfo.pAddress = this->desiredAddress;
                    handleInfo.memSize = this->spanSize;

                    NativePageAllocation::pageInfo pageInfo;
                    pageInfo.pAddress = (void*)thePage->pageAddress;
                    pageInfo.regionSize = (size_t)thePage->allocSize;
                    // todo: add page information.

                    allowIntersect = manager.collDetect.OnPageCollision( handleInfo, pageInfo );
                }
                if ( !allowIntersect )
                {
                    // We have encountered an invalid request and must terminate.
                    this->validAllocation = false;
                    return false;
                }
            }

            // Default operation switches.
            bool requireCurrentPage = false;

            // Dispatch the request with local parameters.
            bool allocateMissingBeforePages = false;

            if ( intResult == memBlockSlice_t::INTERSECT_INSIDE ||
                 intResult == memBlockSlice_t::INTERSECT_EQUAL )
            {
                requireCurrentPage = true;
            }
            else if ( intResult == memBlockSlice_t::INTERSECT_BORDER_END )
            {
                // There are pages before any currently allocated page that require validation before
                // we can proceed.
                allocateMissingBeforePages = true;

                requireCurrentPage = true;
            }
            else if ( intResult == memBlockSlice_t::INTERSECT_BORDER_START )
            {
                requireCurrentPage = true;
            }
            else if ( intResult == memBlockSlice_t::INTERSECT_ENCLOSING ) 
            {
                // The allocation request region encloses the current resident page. We must
                // (conditionally) handle region before and after the allocated blocks (if
                // these ranges are not 0).
                if ( concurrentRequest.GetSliceStartPoint() != thePage->pageSpan.GetSliceStartPoint() )
                {
                    // We have memory before the resident page.
                    allocateMissingBeforePages = true;
                }

                requireCurrentPage = true;
            }

            if ( allocateMissingBeforePages )
            {
                SIZE_T invalidRegionStart = concurrentRequest.GetSliceStartPoint();
                SIZE_T invalidRegionEnd = thePage->pageSpan.GetSliceStartPoint();

                bool validateSuccess = ValidateRegion( invalidRegionStart, invalidRegionEnd );

                if ( !validateSuccess )
                    return false;
            }

            // Include the current page into the host pages list.
            if ( requireCurrentPage )
            {
                this->hostPages.AddItem( thePage );

                this->hasHostPages = true;
            }

            return true;
        }

        inline bool ValidateRegion( SIZE_T invalidRegionStart, SIZE_T invalidRegionEnd )
        {
            size_t invalidRegionSpan = (size_t)( invalidRegionEnd - invalidRegionStart );
            void *invalidRegionPtr = (void*)invalidRegionStart;

            bool validationSuccess = manager.PageAcquireHelper(
                invalidRegionPtr, invalidRegionSpan,
                allocatedPages, allocatedNewPage,
                hostPages, hasHostPages
            );

            if ( !validationSuccess )
            {
                this->validAllocation = false;
            }
            return validationSuccess;
        }

        inline void OnIncompleteOperation( const memBlockSlice_t& finishingMemRange )
        {
            // Only complete the request if we are valid.
            if ( IsValidAllocation() )
            {
                // We (still) need to allocate the region that is defined by requestedMemRange!
                SIZE_T invalidRegionStart = finishingMemRange.GetSliceStartPoint();
                SIZE_T invalidRegionEnd = finishingMemRange.GetSliceEndPoint() + 1;

                ValidateRegion( invalidRegionStart, invalidRegionEnd );
            }
        }
    };

    inline bool ReserveMemoryOnRegion(
        memBlockSlice_t& requestedMemRange,
        residingMemBlocks_t& hostPages, bool& hasHostPages )
    {
        // WARNING: the hostPages array has to be address sorted!

        // Copy important intersection parameters.
        void *desiredAddress = (void*)requestedMemRange.GetSliceStartPoint();
        size_t spanSize = (size_t)requestedMemRange.GetSliceSize();

        // Create a private copy of the sorted memory ranges list.
        // We assume that we are the only thread that modifies this list.
        residingMemBlocks_t privateSortedMemoryRanges;

        LIST_FOREACH_BEGIN( pageAllocation, sortedMemoryRanges.root, sortedNode )
            privateSortedMemoryRanges.AddItem( item );
        LIST_FOREACH_END

        // Handle the request.
        ReserveMemorySelector selector( desiredAddress, spanSize, hostPages, hasHostPages, *this );

        ProcessSortedPages( requestedMemRange, privateSortedMemoryRanges, selector );

        return selector.IsValidAllocation();
    }

    static inline bool IsValidAllocation( void *desiredAddress, size_t spanSize )
    {
        bool isValid = true;

        if ( desiredAddress != NULL )
        {
            // Check that there is no number overflow.
            SIZE_T memDesiredAddress = (SIZE_T)desiredAddress;
            SIZE_T memSpanSize = (SIZE_T)spanSize;

            SIZE_T memAddressBorder = ( memDesiredAddress + memSpanSize );

            // The condition I check here is that if I add those two numbers,
            // the result must be bigger than the source operand I added to.
            isValid = ( memAddressBorder > memDesiredAddress );
        }
        return isValid;
    }

    inline pageHandle* Allocate( void *desiredAddress, size_t spanSize )
    {
        pageHandle *theHandle = NULL;

        // Only proceed if the requested allocation is valid.
        if ( IsValidAllocation( desiredAddress, spanSize ) )
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

    // Helper function to get a signed difference between two unsigned numbers.
    template <typename numberType>
    static inline numberType GetSignedDifference( const numberType& left, const numberType& right, bool& isSigned )
    {
        bool _isSigned = ( left < right );

        numberType result;

        if ( _isSigned )
        {
            result = ( right - left );
        }
        else
        {
            result = ( left - right );
        }

        isSigned = _isSigned;

        return result;
    }

    // Attempts to update the handle size so that either more or less memory
    // can be used.
    inline bool SetHandleSize( pageHandle *theHandle, size_t newReserveSize )
    {
        // Do nothing if the handle size has not changed.
        size_t oldSize = theHandle->GetTargetSize();

        if ( newReserveSize == oldSize )
            return true;

        bool isSigned;
        size_t memSizeDifference = GetSignedDifference( newReserveSize, oldSize, isSigned );

        bool success = true;

        if ( !isSigned )
        {
            // Make sure that this allocation is valid.
            // It can only turn invalid if the memory size is greater than before.
            if ( IsValidAllocation( theHandle->GetTargetPointer(), newReserveSize ) )
            {
                // If the new memory size is greater than the old.
                // Allocate additional memory pages if they are required.
                residingMemBlocks_t hostPages; bool hasHostPages;

                memBlockSlice_t requiredRegion( (SIZE_T)theHandle->GetTargetPointer() + (SIZE_T)oldSize, (SIZE_T)memSizeDifference );

                success = ReserveMemoryOnRegion( requiredRegion, hostPages, hasHostPages );

                // Have we succeeded in reserving the requested memory pages?
                if ( success )
                {
                    // Perform an optimized append of the sorted hosted-pages.
                    // Prevent page duplication of page allocations inside the resident pages of theHandle!
                    for ( unsigned int n = 0; n < hostPages.GetCount(); n++ )
                    {
                        pageAllocation *newPage = hostPages.GetFast( n );

                        // If its the first page, compare it with the last of the resident pages.
                        // If the same, then do not add.
                        bool alreadyExists = false;

                        if ( n == 0 )
                        {
                            pageAllocation *lastConcurrentBlock = NULL;

                            bool hasLast = theHandle->residingMemBlocks.Tail( lastConcurrentBlock );

                            alreadyExists = ( hasLast ) && ( newPage == lastConcurrentBlock );
                        }

                        // If the page does not already exist, add it.
                        if ( !alreadyExists )
                        {
                            // WARNING: this is a sorted append!
                            newPage->RegisterPageHandle( theHandle );
                        }
                    }
                }
            }
        }
        else
        {
            // Otherwise the new memory size is smaller than the old.
            // We potentially have to remove pages from the residency list.
            struct ContextRemovalSelector
            {
                NativePageAllocator& manager;
                pageHandle *contextHandle;
                unsigned int removePagesFromEndCount;
                bool validOperation;

                inline ContextRemovalSelector( NativePageAllocator& manager, pageHandle *theHandle ) : manager( manager )
                {
                    this->contextHandle = theHandle;

                    this->removePagesFromEndCount = 0;
                    this->validOperation = true;
                }

                inline ~ContextRemovalSelector( void )
                {
                    if ( this->validOperation )
                    {
                        // If we need to remove any pages from the end...
                        if ( unsigned int removeFromEndCount = this->removePagesFromEndCount )
                        {
                            // Set a new size to the handle residing memory pages array.
                            while ( removeFromEndCount-- )
                            {
                                pageAllocation *pageToBeDereferenced = NULL;

                                bool hasThePage = this->contextHandle->residingMemBlocks.Tail( pageToBeDereferenced );

                                assert( hasThePage == true );

                                pageToBeDereferenced->UnregisterPageHandle( this->contextHandle );

                                manager.MemBlockGarbageCollection( pageToBeDereferenced );
                            }
                        }
                    }
                }

                inline bool OnPageIntersection(
                    const memBlockSlice_t& concurrentRequest, const pageAllocation *thePage,
                    memBlockSlice_t::eIntersectionResult intResult
                )
                {
                    // Check whether we should remove the handle from the context of the current page.
                    bool removeCurrentPage = false;
                    
                    if ( intResult == memBlockSlice_t::INTERSECT_INSIDE )
                    {
                        // We cannot handle the request if we are found inside of a page.
                        // Terminate with an error.
                        this->validOperation = false;

                        return false;
                    }
                    else if ( intResult == memBlockSlice_t::INTERSECT_BORDER_START )
                    {
                        // The removal range ends in the current page.
                        // This means that the current page is the last page that should be removed.
                        removeCurrentPage = true;
                    }
                    else if ( intResult == memBlockSlice_t::INTERSECT_ENCLOSING )
                    {
                        // Current page is enclosed by the removal range.
                        // This is a safe remove.
                        removeCurrentPage = true;
                    }
                    else if ( intResult == memBlockSlice_t::INTERSECT_BORDER_END )
                    {
                        // The removal range starts in the current page.
                        // Remove this page if the removal range starts at the beginning of the page
                        // (a.k.a. is covering the entire page).
                        if ( concurrentRequest.GetSliceStartPoint() == thePage->pageSpan.GetSliceStartPoint() )
                        {
                            removeCurrentPage = true;
                        }
                    }
                    else if ( intResult == memBlockSlice_t::INTERSECT_EQUAL )
                    {
                        // If the removal request is equal to the page, just remove it.
                        removeCurrentPage = true;
                    }

                    // Remove the current page if necessary.
                    if ( removeCurrentPage )
                    {
                        // Actually do an optimized removal!
                        removePagesFromEndCount++;
                    }

                    return true;
                }

                inline void OnIncompleteOperation( const memBlockSlice_t& finishingMemRange )
                {
                    return;
                }

                inline bool IsValid( void )
                {
                    return this->validOperation;
                }
            };

            memBlockSlice_t requiredRegion( (SIZE_T)theHandle->GetTargetPointer() + (SIZE_T)newReserveSize, (SIZE_T)memSizeDifference );

            // Create a private copy of the sorted memory ranges list.
            // We assume that we are the only thread that modifies this list.
            residingMemBlocks_t privateSortedMemoryRanges;

            LIST_FOREACH_BEGIN( pageAllocation, sortedMemoryRanges.root, sortedNode )
                privateSortedMemoryRanges.AddItem( item );
            LIST_FOREACH_END

            ContextRemovalSelector selector( *this, theHandle );

            ProcessSortedPages( requiredRegion, privateSortedMemoryRanges, selector );
            
            success = selector.IsValid();
        }

        if ( success )
        {
            // Set the new handle region.
            theHandle->requestedMemory.SetSliceEndPoint( (SIZE_T)theHandle->GetTargetPointer() + (SIZE_T)newReserveSize - (SIZE_T)1 );
        }

        return success;
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