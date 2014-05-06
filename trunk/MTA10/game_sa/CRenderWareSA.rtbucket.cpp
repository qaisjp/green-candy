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

#include "CRenderWareSA.state.rs.hxx"
#include "CRenderWareSA.state.tss.hxx"
#include "CRenderWareSA.state.lighting.hxx"
#include "CRenderWareSA.state.sampler.hxx"
#include "CRenderWareSA.state.transf.hxx"
#include "CRenderWareSA.state.stream.hxx"

// Internal definitions.
namespace RenderBucket
{
    struct renderSystemState
    {
        typedef RwVertexStreamStateManager::capturedState vsState;
        typedef RwRenderStateManager::capturedState rsState;
        typedef RwTextureStageStateManager::capturedState tssState;
        typedef RwSamplerStateManager::capturedState samplState;
        typedef RwLightingStateManager::capturedState lighState;
        typedef RwTransformationStateManager::capturedState transfState;

        // Variables here describe an invariant render state.
        vsState*                        vertexStreamState;
        rsState*                        renderState;
        tssState*                       textureStageState;
        samplState*                     samplerState;
        lighState*                      lightingState;
        transfState*                    transformationState;

        renderSystemState( void )
        {
            vertexStreamState = NULL;
            renderState = NULL;
            textureStageState = NULL;
            samplerState = NULL;
            lightingState = NULL;
            transformationState = NULL;
        }

        ~renderSystemState( void )
        {
            Terminate();
        }

        void Terminate( void )
        {
            if ( vertexStreamState )
            {
                g_vertexStreamStateManager.FreeState( vertexStreamState );

                vertexStreamState = NULL;
            }

            if ( renderState )
            {
                g_renderStateManager.FreeState( renderState );

                renderState = NULL;
            }

            if ( textureStageState )
            {
                g_textureStageStateManager.FreeState( textureStageState );

                textureStageState = NULL;
            }

            if ( samplerState )
            {
                g_samplerStateManager.FreeState( samplerState );

                samplerState = NULL;
            }

            if ( lightingState )
            {
                g_lightingStateManager.FreeState( lightingState );

                lightingState = NULL;
            }

            if ( transformationState )
            {
                g_transformationStateManager.FreeState( transformationState );

                transformationState = NULL;
            }
        }

        void Capture( void )
        {
            // Capture all current states.
            if ( vertexStreamState )
            {
                vertexStreamState->Capture();
            }
            else
            {
                vertexStreamState = g_vertexStreamStateManager.CaptureState();
            }

            if ( renderState )
            {
                renderState->Capture();
            }
            else
            {
                renderState = g_renderStateManager.CaptureState();
            }

            if ( textureStageState )
            {
                textureStageState->Capture();
            }
            else
            {
                textureStageState = g_textureStageStateManager.CaptureState();
            }

            if ( lightingState )
            {
                lightingState->Capture();
            }
            else
            {
                lightingState = g_lightingStateManager.CaptureState();
            }

            if ( transformationState )
            {
                transformationState->Capture();
            }
            else
            {
                transformationState = g_transformationStateManager.CaptureState();
            }

            if ( samplerState )
            {
                samplerState->Capture();
            }
            else
            {
                samplerState = g_samplerStateManager.CaptureState();
            }
        }

        bool IsCurrent( void ) const
        {
            if ( !vertexStreamState->IsCurrent() )
                return false;

            if ( !renderState->IsCurrent() )
                return false;

            if ( !textureStageState->IsCurrent() )
                return false;

            if ( !lightingState->IsCurrent() )
                return false;

            if ( !transformationState->IsCurrent() )
                return false;

            if ( !samplerState->IsCurrent() )
                return false;

            return true;
        }

        void AcquireContext( void )
        {
            vertexStreamState->SetDeviceTo();
            renderState->SetDeviceTo();
            textureStageState->SetDeviceTo();
            lightingState->SetDeviceTo();
            transformationState->SetDeviceTo();
            samplerState->SetDeviceTo();
        }
    };

    struct renderDataCachedMesh
    {
        // Variables here describe states that are not as expensive.
        D3DMATERIAL9                    materialState;
        D3DMATRIX                       worldMatrix;

        void Capture( void )
        {
            RwD3D9GetMaterial( materialState );

            RwD3D9GetTransform( D3DTS_WORLD, &worldMatrix );
        }

        void Apply( void )
        {
            RwD3D9SetMaterial( materialState );

            RwD3D9SetTransform( D3DTS_WORLD, &worldMatrix );
        }

        // Members deciding the rendering call.
        RwD3D9RenderCallbackData        renderCall;
    };

    struct RwRenderBucket
    {
        renderSystemState renderState;

        struct bucketRenderEntry
        {
            renderDataCachedMesh renderData;
            unsigned int globListIndex;
        };

        struct renderDataArrayManager
        {
            AINLINE void InitField( bucketRenderEntry& data )
            {
                return;
            }
        };
        typedef growableArray <bucketRenderEntry, 12, 0, renderDataArrayManager, unsigned int> renderItems_t;

        renderItems_t renderItems;
    };
};

namespace RenderBucket
{
    typedef RwRenderBucket Pipelineable;

    // RenderBucket variables.
    static bool isInPhase = false;

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

        if ( currentPipeData && currentPipeData->renderState.IsCurrent() )
        {
            currentBucket = currentPipeData;
        }

        if ( !currentBucket )
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

        if ( !currentBucket )
        {
            currentBucket = AllocateRenderBucket();
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
    g_renderStateManager.BeginBucketPass();
    g_textureStageStateManager.BeginBucketPass();
    g_samplerStateManager.BeginBucketPass();
    g_lightingStateManager.BeginBucketPass();
    g_transformationStateManager.BeginBucketPass();

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

static const bool allowBucketRendering = true;

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
    g_renderStateManager.EndBucketPass();
    g_textureStageStateManager.EndBucketPass();
    g_samplerStateManager.EndBucketPass();
    g_lightingStateManager.EndBucketPass();
    g_transformationStateManager.EndBucketPass();

    // Leave the phase.
    isInPhase = false;
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

    // TODO: actually handle rendering requests.
    return true;
}