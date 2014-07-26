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

void* CEnvMapAtomicSA::operator new ( size_t )
{
    return RenderWare::GetEnvMapAtomicPool()->Allocate();
}

void CEnvMapAtomicSA::operator delete ( void *ptr )
{
    RenderWare::GetEnvMapAtomicPool()->Free( (CEnvMapAtomicSA*)ptr );
}

/*=========================================================
    RenderWare Atomic Direct3D Bucket Sorting Extension
=========================================================*/
// Include internal bucket sorting definitions.
#include "CRenderWareSA.rtbucket.hxx"

/*=========================================================
    RpAtomic::GetWorldBoundingSphere

    Purpose:
        Calculates and returns a bounding sphere in world space
        which entirely contains the geometry.
    Binary offsets:
        (1.0 US): 0x00749330
        (1.0 EU): 0x00749380
=========================================================*/
static int _atomicBucketSortPluginOffset = -1;

struct atomicBucketSortPlugin
{
    RenderBucket::RwRenderBucket *lastBestBucket;
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
    
    info->lastBestBucket = NULL;

    return rwobj;
}

void __cdecl RpAtomicD3D9DestructBucketSort( RpAtomic *rwobj, size_t pluginOffset )
{
    atomicBucketSortPlugin *info = RW_PLUGINSTRUCT <atomicBucketSortPlugin> ( rwobj, pluginOffset );

    // todo.
    
}

RpAtomic* __cdecl RpAtomicD3D9CopyConstructBucketSort( RpAtomic *dstObject, const RpAtomic *srcObject, size_t pluginOffset, unsigned int pluginId )
{
    atomicBucketSortPlugin *dstInfo = RW_PLUGINSTRUCT <atomicBucketSortPlugin> ( dstObject, pluginOffset );
    const atomicBucketSortPlugin *srcInfo = RW_PLUGINSTRUCT <atomicBucketSortPlugin> ( srcObject, pluginOffset );

    // todo.

    return dstObject;
}

void RpAtomicD3D9_RegisterPlugins( void )
{
    _atomicBucketSortPluginOffset = RpAtomicRegisterPlugin( sizeof( atomicBucketSortPlugin ), RW_PLUGIN_BUCKETSORT,
        RpAtomicD3D9ConstructBucketSort,
        RpAtomicD3D9DestructBucketSort,
        RpAtomicD3D9CopyConstructBucketSort
    );
}

bool __cdecl RpAtomicSetContextualRenderBucket( RpAtomic *theAtomic, RenderBucket::RwRenderBucket *theBucket )
{
    bool success = false;

    if ( atomicBucketSortPlugin *thePlugin = GetBucketSortInfo( theAtomic ) )
    {
        // todo.
    }

    return success;
}

RenderBucket::RwRenderBucket* __cdecl RpAtomicGetContextualRenderBucket( RpAtomic *theAtomic )
{
    RenderBucket::RwRenderBucket *theBucket = NULL;

    if ( atomicBucketSortPlugin *thePlugin = GetBucketSortInfo( theAtomic ) )
    {
        theBucket = thePlugin->lastBestBucket;
    }

    return theBucket;
}

// Module initialization.
void RpAtomicD3D9_Init( void )
{
}

void RpAtomicD3D9_Shutdown( void )
{
}