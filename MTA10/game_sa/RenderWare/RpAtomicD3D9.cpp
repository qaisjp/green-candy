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
    renderBucketList_t lastBestBucketPass;
};

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
    
    // Construct the struct.
    new (info) atomicBucketSortPlugin();

    return rwobj;
}

void __cdecl RpAtomicD3D9DestructBucketSort( RpAtomic *rwobj, size_t pluginOffset )
{
    atomicBucketSortPlugin *info = RW_PLUGINSTRUCT <atomicBucketSortPlugin> ( rwobj, pluginOffset );

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

RpAtomic* __cdecl RpAtomicD3D9CopyConstructBucketSort( RpAtomic *dstObject, const RpAtomic *srcObject, size_t pluginOffset, unsigned int pluginId )
{
    atomicBucketSortPlugin *dstInfo = RW_PLUGINSTRUCT <atomicBucketSortPlugin> ( dstObject, pluginOffset );
    const atomicBucketSortPlugin *srcInfo = RW_PLUGINSTRUCT <atomicBucketSortPlugin> ( srcObject, pluginOffset );

    // Construct the struct at dstInfo.
    new (dstInfo) atomicBucketSortPlugin();

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

    return dstObject;
}

static CExecThreadSA *uniquenessThread = NULL;

// Thread that is supposed to increase bucket unique-ness over time.
static void __stdcall BucketUniquenessQuantifierThread( CExecThreadSA *thread, void *userdata )
{
    // todo: do something.
    return;
}

void RpAtomicD3D9_RegisterPlugins( void )
{
    _atomicBucketSortPluginOffset = RpAtomicRegisterPlugin( sizeof( atomicBucketSortPlugin ), RW_PLUGIN_BUCKETSORT,
        RpAtomicD3D9ConstructBucketSort,
        RpAtomicD3D9DestructBucketSort,
        RpAtomicD3D9CopyConstructBucketSort
    );

    // Start our thread that should merge buckets which are same by device states.
    uniquenessThread = pGame->GetExecutiveManager()->CreateThread( BucketUniquenessQuantifierThread, NULL, 0 );

    if ( CExecThreadSA *thread = uniquenessThread )
    {
        // Start our thread if we successfully created it.
        thread->Resume();
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

        {
            renderBucketList_t& currentPass = thePlugin->lastBestBucketPass;

            if ( contextualPassIndex < currentPass.GetSizeCount() )
            {
                currentBucket = currentPass.Get( contextualPassIndex );
            }
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
            thePlugin->lastBestBucketPass.SetItem( theBucket, contextualPassIndex );

            if ( theBucket )
            {
                // Increment the usage count.
                theBucket->usageCount++;
                
                theBucket->Reference();
            }
        }
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

        if ( contextualPassIndex < currentPass.GetSizeCount() )
        {
            theBucket = currentPass.Get( contextualPassIndex );
        }
    }

    return theBucket;
}

// Module initialization.
void RpAtomicD3D9_Init( void )
{
    uniquenessThread = NULL;
}

void RpAtomicD3D9_Shutdown( void )
{
    if ( uniquenessThread )
    {
        pGame->GetExecutiveManager()->CloseThread( uniquenessThread );

        uniquenessThread = NULL;
    }
}