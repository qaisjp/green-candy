/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        game_sa/CRenderWareSA.rtbucket.cpp
*  PURPOSE:     Bucket rendering optimizations using rendering metadata
*  DEVELOPERS:  Martin Turski <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

//todo: will work on this system when I have lots of free time.
// It will improve the FPS a huge amount.

#include "StdInc.h"
#include "RenderWare/RwRenderTools.hxx"

// Include internal definitions.
#include "CRenderWareSA.rtbucket.hxx"

// Keep statistics about our efficiency.
struct runtimeStatistics
{
    AINLINE runtimeStatistics( void )
    {
        Reset();
    }

    AINLINE void Reset( void )
    {
        maxEntriesPerBucket = 0;
        totalRenderedEntries = 0;
        totalNumberOfBuckets = 0;
    }

    unsigned int maxEntriesPerBucket;
    unsigned int totalRenderedEntries;
    unsigned int totalNumberOfBuckets;
};

static runtimeStatistics _currentPassStats;
static runtimeStatistics _lastPassStats;

namespace RenderBucket
{
    typedef RwRenderBucket Pipelineable;

    // RenderBucket variables.
    static bool isInPhase = false;

    // Pointer to the currently rendering atomic object.
    static RpAtomic *currentlyRenderingAtomic = NULL;

    struct pipelineArrayManager
    {
        AINLINE void InitField( Pipelineable*& data )
        {
            return;
        }
    };
    typedef growableArray <Pipelineable*, 4, 0, pipelineArrayManager, unsigned int> pipelineArray_t;

    // Array of all data on the virtual pipeline.
    static pipelineArray_t pipelineData;

    static Pipelineable *currentPipeData = NULL;

    // Allocator so we do not polute the memory environment.
    typedef CachedConstructedClassAllocator <RwRenderBucket> bucketAlloc_t;

    static bucketAlloc_t bucketAlloc( 512 );

    // Utility functions.
    RwRenderBucket* AllocateRenderBucket( void )
    {
        RwRenderBucket *renderBucket = bucketAlloc.Allocate();

        // Initialize the members.
        renderBucket->renderState.Capture();
        renderBucket->renderItems.Clear();

        pipelineData.AddItem( renderBucket );

        currentPipeData = renderBucket;

        return renderBucket;
    }

    void FreeRenderBucket( RwRenderBucket *bucket )
    {
        bucket->renderState.Terminate();

        bucketAlloc.Free( bucket );
    }

    RwRenderBucket* FindRenderBucket( void )
    {
        RwRenderBucket *currentBucket = NULL;

        // Check the most notable render bucket first.
        // This one is very likely to succeed.
        if ( currentBucket == NULL )
        {
            RwRenderBucket *lastBestBucket = RpAtomicGetContextualRenderBucket( NULL );

            if ( lastBestBucket && lastBestBucket->renderState.IsCurrent() )
            {
                currentBucket = lastBestBucket;
            }
        }

        if ( currentBucket == NULL )
        {
            RwRenderBucket *likelyBucket = currentPipeData;

            if ( likelyBucket && likelyBucket->renderState.IsCurrent() )
            {
                currentBucket = likelyBucket;
            }
        }

        if ( currentBucket == NULL )
        {
            unsigned int n = pipelineData.GetCount();

            while ( n != 0 )
            {
                RwRenderBucket *rtBucket = pipelineData.Get( --n );

                if ( rtBucket && rtBucket != currentPipeData )
                {
                    if ( rtBucket->renderState.IsCurrent() )
                    {
                        currentBucket = rtBucket;
                        break;
                    }
                }
            }
        }

        if ( currentBucket == NULL )
        {
            currentBucket = AllocateRenderBucket();

            _currentPassStats.totalNumberOfBuckets++;
        }

        return currentBucket;
    }

    static void LockStateMachines( void )
    {
        // Lock all state machines.
        g_renderStateManager.LockLocalState();
        g_textureStageStateManager.LockLocalState();
        g_transformationStateManager.LockLocalState();
        g_samplerStateManager.LockLocalState();
        g_lightingStateManager.LockLocalState();
    }

    static void UnlockStateMachines( void )
    {
        // Unlock all state machines.
        g_renderStateManager.UnlockLocalState();
        g_textureStageStateManager.UnlockLocalState();
        g_transformationStateManager.UnlockLocalState();
        g_samplerStateManager.UnlockLocalState();
        g_lightingStateManager.UnlockLocalState();
    }

    // Task runtime that handles sorting.
    struct renderQueueSortingItem
    {
        RwD3D9RenderCallbackData lastRenderCall;
        renderSystemState sysState;
    };
    typedef SynchronizedHyperQueue <renderQueueSortingItem> renderSortingQueue_t;

    static renderSortingQueue_t renderSortingQueue( 256 );

    static renderSortingQueue_t processedList( 512 );

    struct SimpleRenderTaskQueueApplicator
    {
        renderQueueSortingItem& theItem;

        AINLINE SimpleRenderTaskQueueApplicator( renderQueueSortingItem& theItem ) : theItem( theItem )
        {
            return;
        }

        AINLINE void PutItem( renderQueueSortingItem& theField )
        {
            theField.sysState = theItem.sysState;
        }
    };

    static void __stdcall RenderBucketSortingTask( CExecTaskSA *theTask, void *memPtr )
    {
        // Do nothing for initialization.
        CFiberSA *theFiber = theTask->runtimeFiber;

        theFiber->yield();

        // Handle sorting requests, forever.
        while ( true )
        {
            renderQueueSortingItem theItem;

            renderSortingQueue.GetSheduledItem( theItem );

            // todo.

            // Yield once finished.
            theFiber->yield();
        }
    }

    static CExecTaskSA *sortingTask = NULL;

    struct renderProcessItem
    {
        RwRenderBucket *rwBucket;
        unsigned int bucketItemIndex;
    };

    struct renderProcessItemArrayManager
    {
        AINLINE void InitField( renderProcessItem& theItem )
        {
            return;
        }
    };
    typedef growableArray <renderProcessItem, 12, 0, renderProcessItemArrayManager, unsigned int> renderProcessItemArray_t;

    static renderProcessItemArray_t renderProcessItemArray;
}

void RenderBucket::Initialize( void )
{
    // Set up runtime data.
    isInPhase = false;
    currentPipeData = NULL;

    // Create the sorting task for multi-threaded execution.
    sortingTask = pGame->GetExecutiveManager()->CreateTask( RenderBucketSortingTask, NULL );

    assert( sortingTask != NULL );
}

void RenderBucket::Shutdown( void )
{
    if ( sortingTask )
    {
        // Close our task.
        pGame->GetExecutiveManager()->CloseTask( sortingTask );

        sortingTask = NULL;
    }

    assert( isInPhase == false );
}

void RenderBucket::BeginPass( void )
{
    assert( isInPhase == false );

    // Notify all state managers.
    g_vertexStreamStateManager.BeginBucketPass();
    g_renderStateManager.BeginBucketPass();
    g_textureStageStateManager.BeginBucketPass();
    g_samplerStateManager.BeginBucketPass();
    g_lightingStateManager.BeginBucketPass();
    g_transformationStateManager.BeginBucketPass();

    // Reset statistics.
    _currentPassStats.Reset();

    // Enter the phase.
    isInPhase = true;
}

namespace RenderBucket
{
    const static D3DVERTEXELEMENT9 bucketRaster_instanceData[] = 
    {
        D3DDECL_END()
    };

    struct BucketedRasterizer
    {
        AINLINE BucketedRasterizer( void )
        {
            currentBucket = NULL;
        }

        AINLINE void setContext( RwRenderBucket *theBucket )
        {
            if ( currentBucket != theBucket )
            {
                theBucket->renderState.AcquireContext();

                currentBucket = theBucket;
            }
        }

        

        RwRenderBucket *currentBucket;
    };
}

static const bool allowBucketRendering = false;

void RenderBucket::RenderPass( void )
{
    assert( isInPhase == true );

    if ( !allowBucketRendering )
        return;

    // Reset all state managers to bucket states.
    g_vertexStreamStateManager.SetBucketStates();
    g_renderStateManager.SetBucketStates();
    g_textureStageStateManager.SetBucketStates();
    g_samplerStateManager.SetBucketStates();
    g_lightingStateManager.SetBucketStates();
    g_transformationStateManager.SetBucketStates();

    // Render all items.
    BucketedRasterizer rasterizer;

    for ( unsigned int n = 0; n < renderProcessItemArray.GetCount(); n++ )
    {
        const renderProcessItem& renderItem = renderProcessItemArray.Get( n );

        // Unpack bucket and corresponding item.
        RwRenderBucket *theBucket = renderItem.rwBucket;
        RwRenderBucket::bucketRenderEntry& meshData = theBucket->renderItems.Get( renderItem.bucketItemIndex );

        rasterizer.setContext( theBucket );

    }

    renderProcessItemArray.Clear();
}

void RenderBucket::EndPass( void )
{
    assert( isInPhase == true );

    // Process the pass.
    RenderPass();

    // Free all rendering data.
    for ( unsigned int n = 0; n < pipelineData.GetCount(); n++ )
    {
        FreeRenderBucket( pipelineData.Get( n ) );
    }

    // Clear runtime variables.
    pipelineData.Clear();

    currentPipeData = NULL;

    // Notify the managers.
    g_vertexStreamStateManager.EndBucketPass();
    g_renderStateManager.EndBucketPass();
    g_textureStageStateManager.EndBucketPass();
    g_samplerStateManager.EndBucketPass();
    g_lightingStateManager.EndBucketPass();
    g_transformationStateManager.EndBucketPass();

    // Put finished statistics into the global storage.
    _lastPassStats = _currentPassStats;

    // Leave the phase.
    isInPhase = false;
}

void RenderBucket::SetContextAtomic( RpAtomic *renderObject )
{
    currentlyRenderingAtomic = renderObject;
}

renderBucketStats RenderBucket::GetRuntimeStatistics( void )
{
    renderBucketStats statsOut;
    statsOut.totalNumberOfRenderCalls = _lastPassStats.totalRenderedEntries;
    statsOut.totalNumberOfActiveBuckets = _lastPassStats.totalNumberOfBuckets;
    statsOut.maxRenderCallsPerBucket = _lastPassStats.maxEntriesPerBucket;

    return statsOut;
}

// Since this is the lowest rendering layer, it is very important to keep this code as optimized as possible.
// Rules of thumb:
// * avoid memory allocations as much as possible, since they scale dramatically
// * use growableArray for data that needs scaling
// * use CachedConstructedClassAllocator to create complex classes whose data does not have to be constructed on allocation
// * make the code not care about memory-saving
// * do not allow any memory leak, since its a definite crash
// * make sure performance is 100 FPS at rendering 50 vehicles

struct RenderItemQueueInitializator
{
    const RwD3D9RenderCallbackData& callbackData;

    AINLINE RenderItemQueueInitializator( const RwD3D9RenderCallbackData& theData ) : callbackData( theData )
    {
        return;
    }

    AINLINE void PutItem( RenderBucket::renderQueueSortingItem& theField )
    {
        theField.sysState.Capture();
        theField.lastRenderCall = callbackData;
    }
};

bool RenderBucket::OnCachedRenderCall( const RwD3D9RenderCallbackData& callbackData )
{
    // If we are not in the virtual pipeline phase, ignore.
    if ( !isInPhase )
        return false;

    RwRenderBucket *currentBucket = FindRenderBucket();

    RwRenderBucket::bucketRenderEntry entry;

    renderDataCachedMesh& cachedData = entry.renderData;
    cachedData.Capture();
    cachedData.renderCall = callbackData;

    if ( allowBucketRendering )
    {
        // Store the global index for performance optimizations.
        entry.globListIndex = renderProcessItemArray.GetCount();

        // Add the request to the global render item list.
        renderProcessItem& thisItem = renderProcessItemArray.ObtainItem();
        thisItem.rwBucket = currentBucket;
        thisItem.bucketItemIndex = ( currentBucket->renderItems.GetCount() - 1 );
    }

    currentBucket->renderItems.AddItem( entry );

    // Update statistics.
    {
        if ( _currentPassStats.maxEntriesPerBucket < currentBucket->renderItems.GetCount() )
        {
            _currentPassStats.maxEntriesPerBucket = currentBucket->renderItems.GetCount();
        }

        _currentPassStats.totalRenderedEntries++;
    }

    // TODO: actually handle rendering requests.
    return true;
}