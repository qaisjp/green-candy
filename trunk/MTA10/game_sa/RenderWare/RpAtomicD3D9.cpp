/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        game_sa/RenderWare/RpAtomicD3D9.cpp
*  PURPOSE:     RenderWare Atomic Direct3D 9 plugin definitions
*  DEVELOPERS:  Martin Turski <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#include <StdInc.h>

#include "../gamesa_renderware.h"

/*=========================================================
    RenderWare Atomic Direct3D Environment Reflection Map Plugin
=========================================================*/

void* CEnvMapAtomicSA::operator new ( size_t )
{
    return RenderWare::GetEnvMapAtomicPool()->Allocate();
}

void CEnvMapAtomicSA::operator delete ( void *ptr )
{
    RenderWare::GetEnvMapAtomicPool()->Free( (CEnvMapAtomicSA*)ptr );
}

/*=========================================================
    RpAtomicGetEnvironmentReflectionMap

    Purpose:
        Returns the environment reflection map associated
        with the given atomic. It is allocated if it does
        not already exist.
    Binary offsets:
        (1.0 US and 1.0 EU): 0x005D96F0
=========================================================*/
struct _atomicReflectionMapStorePlugin
{
    CEnvMapAtomicSA *envMap;
};

inline int GetAtomicReflectionMapStorePluginOffset( void )
{
    return *(int*)0x008D12C8;
}

inline _atomicReflectionMapStorePlugin* GetAtomicReflectionMapStorePlugin( RpAtomic *atomic )
{
    int pluginOffset = GetAtomicReflectionMapStorePluginOffset();

    if ( pluginOffset == -1 )
        return NULL;

    return ( atomic ) ? ( RW_PLUGINSTRUCT <_atomicReflectionMapStorePlugin> ( atomic, pluginOffset ) ) : ( NULL );
}

CEnvMapAtomicSA* __cdecl RpAtomicGetEnvironmentReflectionMap( RpAtomic *atomic )
{
    CEnvMapAtomicSA *envMap = NULL;

    if ( _atomicReflectionMapStorePlugin *info = GetAtomicReflectionMapStorePlugin( atomic ) )
    {
        envMap = info->envMap;

        if ( !envMap )
        {
            envMap = new CEnvMapAtomicSA( 0, 0, 0 );

            info->envMap = envMap;
        }
    }

    return envMap;
}

/*=========================================================
    RenderWare Atomic Direct3D Bucket Sorting Extension
=========================================================*/
// Include internal bucket sorting definitions.
#include "CRenderWareSA.rtbucket.hxx"

static int _atomicBucketSortPluginOffset = -1;

struct renderBucketListManager
{
    AINLINE void InitField( RenderBucket::RwRenderBucket*& theField )
    {
        theField = NULL;
    }
};
typedef growableArray <RenderBucket::RwRenderBucket*, 4, 0, renderBucketListManager, unsigned int> renderBucketList_t;

struct atomicBucketSortPlugin
{
    atomicBucketSortPlugin( void );
    ~atomicBucketSortPlugin( void );

    renderBucketList_t lastBestBucketPass;

    CRITICAL_SECTION bucketPassLock;

    RwListEntry <atomicBucketSortPlugin> managerNode;
};

static RwList <atomicBucketSortPlugin> atomicBucketContainers;

static CRITICAL_SECTION atomicBucketContainers_lock;

atomicBucketSortPlugin::atomicBucketSortPlugin( void )
{
    // Add ourselves to the atomic bucket container list.
    LIST_INSERT( atomicBucketContainers.root, managerNode );

    InitializeCriticalSection( &bucketPassLock );
}

atomicBucketSortPlugin::~atomicBucketSortPlugin( void )
{
    DeleteCriticalSection( &bucketPassLock );

    // Remove ourselves from the bucket container list.
    LIST_REMOVE( managerNode );
}

inline atomicBucketSortPlugin* GetBucketSortInfo( RpAtomic *atomic )
{
    int pluginOffset = _atomicBucketSortPluginOffset;

    if ( pluginOffset == -1 )
        return NULL;

    return ( atomic ) ? ( RW_PLUGINSTRUCT <atomicBucketSortPlugin> ( atomic, pluginOffset ) ) : ( NULL );
}

inline const atomicBucketSortPlugin* GetBucketSortInfoConst( const RpAtomic *atomic )
{
    int pluginOffset = _atomicBucketSortPluginOffset;

    if ( pluginOffset == -1 )
        return NULL;

    return ( atomic ) ? ( RW_PLUGINSTRUCT <const atomicBucketSortPlugin> ( atomic, pluginOffset ) ) : ( NULL );
}

RpAtomic* __cdecl RpAtomicD3D9ConstructBucketSort( RpAtomic *rwobj, size_t pluginOffset )
{
    atomicBucketSortPlugin *info = RW_PLUGINSTRUCT <atomicBucketSortPlugin> ( rwobj, pluginOffset );
    
    EnterCriticalSection( &atomicBucketContainers_lock );
    {
        // Construct the struct.
        new (info) atomicBucketSortPlugin;
    }
    LeaveCriticalSection( &atomicBucketContainers_lock );

    return rwobj;
}

void __cdecl RpAtomicD3D9DestructBucketSort( RpAtomic *rwobj, size_t pluginOffset )
{
    atomicBucketSortPlugin *info = RW_PLUGINSTRUCT <atomicBucketSortPlugin> ( rwobj, pluginOffset );

    EnterCriticalSection( &atomicBucketContainers_lock );
    {
        // If we have render buckets, dereference them.
        for ( unsigned int n = 0; n < info->lastBestBucketPass.GetSizeCount(); n++ )
        {
            RenderBucket::RwRenderBucket *theBucket = info->lastBestBucketPass.GetFast( n );

            if ( theBucket )
            {
                theBucket->Dereference();

                info->lastBestBucketPass.SetFast( NULL, n );
            }
        }

        // Destruct the info.
        info->~atomicBucketSortPlugin();
    }
    LeaveCriticalSection( &atomicBucketContainers_lock );
}

RpAtomic* __cdecl RpAtomicD3D9CopyConstructBucketSort( RpAtomic *dstObject, const RpAtomic *srcObject, size_t pluginOffset, unsigned int pluginId )
{
    atomicBucketSortPlugin *dstInfo = RW_PLUGINSTRUCT <atomicBucketSortPlugin> ( dstObject, pluginOffset );
    const atomicBucketSortPlugin *srcInfo = RW_PLUGINSTRUCT <atomicBucketSortPlugin> ( srcObject, pluginOffset );

    EnterCriticalSection( &atomicBucketContainers_lock );
    {
        // Clear plugin information from the destination, as the struct was constructed already.
        dstInfo->lastBestBucketPass.SetSizeCount( 0 );

        // Take over all the buckets from the source info.
        for ( unsigned int n = 0; n < srcInfo->lastBestBucketPass.GetSizeCount(); n++ )
        {
            RenderBucket::RwRenderBucket *theBucket = srcInfo->lastBestBucketPass.GetFast( n );

            if ( theBucket )
            {
                // Reference the bucket.
                theBucket->Reference();

                // Add it to the dstInfo container.
                dstInfo->lastBestBucketPass.SetItem( theBucket, n );
            }
        }
    }
    LeaveCriticalSection( &atomicBucketContainers_lock );

    return dstObject;
}

static CExecThreadSA *uniquenessThread = NULL;

// Try to find a bucket that matches this bucket...
static RenderBucket::RwRenderBucket* FindSameBucket(
    RenderBucket::renderBuckets_t& bucketCache, 
    const RenderBucket::RwRenderBucket *comparator )
{
    using namespace RenderBucket;

    for ( unsigned int n = 0; n < bucketCache.GetCount(); n++ )
    {
        RwRenderBucket *item = bucketCache.Get( n );

        if ( item != comparator && item->renderState.CompareWith( comparator->renderState ) )
        {
            return item;
        }
    }

    return NULL;
}

// Thread that is supposed to increase bucket unique-ness over time.
static void __stdcall BucketUniquenessQuantifierThread( CExecThreadSA *thread, void *userdata )
{
    using namespace RenderBucket;

    RenderBucket::renderBuckets_t bucketCache;

    while ( true )
    {
        // Initialize the bucket cache and get currently active render buckets.
        bucketCache.Clear();

        RenderBucket::GetActiveRenderBuckets( bucketCache );

        // For all atomic bucket sort plugins...
        EnterCriticalSection( &atomicBucketContainers_lock );
        {
            LIST_FOREACH_BEGIN( atomicBucketSortPlugin, atomicBucketContainers.root, managerNode )
                atomicBucketSortPlugin *bucketContainer = item;

                // For all active buckets...
                EnterCriticalSection( &bucketContainer->bucketPassLock );

                renderBucketList_t& bucketPass = bucketContainer->lastBestBucketPass;

                for ( unsigned int n = 0; n < bucketPass.GetSizeCount(); n++ )
                {
                    RwRenderBucket *bucket = bucketPass.Get( n );

                    if ( bucket )
                    {
                        // ... try to find a bucket that matches it.
                        RwRenderBucket *sameBucket = FindSameBucket( bucketCache, bucket );

                        // If we found a same bucket...
                        if ( sameBucket )
                        {
                            // delete the old one and place the new one.
                            bucket->Dereference();

                            bucketPass.SetItem( sameBucket, n );

                            // Reference the new bucket.
                            sameBucket->Reference();
                        }
                    }

                    // Allow other runtimes to get precedence.
                    LeaveCriticalSection( &bucketContainer->bucketPassLock );
                    EnterCriticalSection( &bucketContainer->bucketPassLock );
                }

                LeaveCriticalSection( &bucketContainer->bucketPassLock );
            LIST_FOREACH_END
        }
        LeaveCriticalSection( &atomicBucketContainers_lock );

        // Release our buckets again.
        for ( unsigned int n = 0; n < bucketCache.GetCount(); n++ )
        {
            RwRenderBucket *bucket = bucketCache.Get( n );

            bucket->Dereference();
        }

        // Sleep for some time.
        Sleep( 1000 );
    }

    return;
}

void RpAtomicD3D9_RegisterPlugins( void )
{
    _atomicBucketSortPluginOffset = RpAtomicRegisterPlugin( sizeof( atomicBucketSortPlugin ), RW_PLUGIN_BUCKETSORT,
        RpAtomicD3D9ConstructBucketSort,
        RpAtomicD3D9DestructBucketSort,
        RpAtomicD3D9CopyConstructBucketSort
    );

    // If the plugin got successfully created...
    if ( _atomicBucketSortPluginOffset != -1 )
    {
        // Initialize the list of atomic bucket containers
        LIST_CLEAR( atomicBucketContainers.root );

#ifdef RENDERWARE_STATEMAN_THREADING_SUPPORT
        // Start our thread that should merge buckets which are same by device states.
        uniquenessThread = pGame->GetExecutiveManager()->CreateThread( BucketUniquenessQuantifierThread, NULL, 0 );

        if ( CExecThreadSA *thread = uniquenessThread )
        {
            // Start our thread if we successfully created it.
            thread->Resume();
        }
#endif
    }
}

/*=========================================================
    RpAtomicContextualRenderSetPassIndex (MTA extension/plugin)

    Arguments:
        passIndex - the pass index to set for the current
            rendering context
    Purpose:
        Sets the current pass index for the current
        rendering context.
=========================================================*/
static unsigned int contextualPassIndex = 0;
static unsigned int contextualStageIndex = 0;

void __cdecl RpAtomicContextualRenderSetPassIndex( unsigned int passIndex )
{
    contextualPassIndex = passIndex;
}

/*=========================================================
    RpAtomicContextualRenderSetStageIndex (MTA extension/plugin)

    Arguments:
        stageIndex - the stage index to set for the current
            rendering context
    Purpose:
        Sets the current stage index for the current
        rendering context.
=========================================================*/
void __cdecl RpAtomicContextualRenderSetStageIndex( unsigned int stageIndex )
{
    contextualStageIndex = stageIndex;
}

/*=========================================================
    RpAtomicSetContextualRenderBucket (MTA extension/plugin)

    Arguments:
        theAtomic - the atomic to put the render bucket link in
        theBucket - bucket to put into the atomic
    Purpose:
        Stores a render bucket link into the specified atomic.
        This is used to accelerate bucket sorting in the
        bucket rasterizer.
=========================================================*/
bool __cdecl RpAtomicSetContextualRenderBucket( RpAtomic *theAtomic, RenderBucket::RwRenderBucket *theBucket )
{
    bool success = false;

    if ( atomicBucketSortPlugin *thePlugin = GetBucketSortInfo( theAtomic ) )
    {
        // Only perform if we have a new render bucket.
        RenderBucket::RwRenderBucket *currentBucket = NULL;

        EnterCriticalSection( &thePlugin->bucketPassLock );

        renderBucketList_t& currentPass = thePlugin->lastBestBucketPass;

        if ( contextualPassIndex < currentPass.GetSizeCount() )
        {
            currentBucket = currentPass.Get( contextualPassIndex );
        }

        if ( currentBucket != theBucket )
        {
            // If we had a bucket already, dereference it.
            if ( currentBucket )
            {
                currentBucket->Dereference();

                // Decrement the usage count.
                currentBucket->usageCount--;
            }

            // Assign us the new bucket.
            currentPass.SetItem( theBucket, contextualPassIndex );

            if ( theBucket )
            {
                // Increment the usage count.
                theBucket->usageCount++;
                
                theBucket->Reference();
            }
        }

        LeaveCriticalSection( &thePlugin->bucketPassLock );
    }

    return success;
}

/*=========================================================
    RpAtomicGetContextualRenderBucket (MTA extension/plugin)

    Arguments:
        theAtomic - the atomic to get a render bucket link from
    Purpose:
        Attempts to return a render bucket link from the atomic
        in the current rendering context. If returned, it is
        assumed to contain a render bucket that is most-likely
        same to the current context.
=========================================================*/
RenderBucket::RwRenderBucket* __cdecl RpAtomicGetContextualRenderBucket( RpAtomic *theAtomic )
{
    RenderBucket::RwRenderBucket *theBucket = NULL;

    if ( atomicBucketSortPlugin *thePlugin = GetBucketSortInfo( theAtomic ) )
    {
        renderBucketList_t& currentPass = thePlugin->lastBestBucketPass;

        EnterCriticalSection( &thePlugin->bucketPassLock );

        if ( contextualPassIndex < currentPass.GetSizeCount() )
        {
            theBucket = currentPass.Get( contextualPassIndex );
        }

        LeaveCriticalSection( &thePlugin->bucketPassLock );
    }

    return theBucket;
}

// Module initialization.
void RpAtomicD3D9_Init( void )
{
    uniquenessThread = NULL;

    InitializeCriticalSection( &atomicBucketContainers_lock );
}

void RpAtomicD3D9_Shutdown( void )
{
    DeleteCriticalSection( &atomicBucketContainers_lock );

    if ( uniquenessThread )
    {
        // todo: cleanly shut down the bucketizer thread.

        pGame->GetExecutiveManager()->CloseThread( uniquenessThread );

        uniquenessThread = NULL;
    }
}