/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        game_sa/RenderWare/RpClump.cpp
*  PURPOSE:     RenderWare native function implementations
*  DEVELOPERS:  Martin Turski <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*  RenderWare is © Criterion Software
*
*****************************************************************************/

#include <StdInc.h>
#include "gamesa_renderware.h"

void RpClump::Render()
{
    LIST_FOREACH_BEGIN( RpAtomic, atomics.root, atomics )
        if ( item->IsVisible() )
            item->renderCallback( item );
    LIST_FOREACH_END
}

static bool RwAssignRenderLink( RwFrame *child, RwRenderLink **link )
{
    (*link)->context = child;
    (*link)++;

    child->ForAllChildren( RwAssignRenderLink, link );
    return true;
}

static void RwRenderLinkInit( RwRenderLink *link, void *data )
{
    RwFrame *frame = (RwFrame*)link->context;

    link->flags = 0;

    link->position = frame->GetPosition();

    link->id = -1;
}

static void RwAnimatedRenderLinkInit( RwRenderLink *link, int )
{
    link->flags = 0;
}

void RpClump::InitStaticSkeleton()
{
    RpAtomic *atomic = GetFirstAtomic();
    RwStaticGeometry *geom = CreateStaticGeometry();
    RwRenderLink *link;
    CVector boneOffsets[MAX_BONES];
    unsigned int boneCount;

    if ( !atomic || !atomic->geometry->skeleton )
    {
        // Initialize a non animated mesh
        link = geom->AllocateLink( parent->CountChildren() );

        // Assign all frames
        parent->ForAllChildren( RwAssignRenderLink, &link );

        // Init them
        geom->ForAllLinks( RwRenderLinkInit, (void*)NULL );

        geom->link->flags |= BONE_ROOT;
        return;
    }

    // Grab the number of bones
    boneCount = atomic->geometry->skeleton->boneCount;

    link = geom->AllocateLink( boneCount );

    if ( boneCount != 0 )
    {
        CVector *offset = boneOffsets;
        RpAnimHierarchy *anim = atomic->anim;  
        RwBoneInfo *bone = anim->boneInfo;
        RwAnimInfo *info = anim->anim->info;

        // Get the real bone positions
        GetBoneTransform( boneOffsets );

        // I guess its always one bone ahead...?
        link++;

        for ( unsigned int n = 0; n < boneCount; n++ )
        {
            link->context = info;
            link->id = bone->index;

            // Update the bone offset in the animation
            info->offset = *offset;
            
            info++;
            offset++;
            link++;
            bone++;
        }
    }

    geom->ForAllLinks( RwAnimatedRenderLinkInit, 0 );

    // Flag the first render link, root bone?
    geom->link->flags |= BONE_ROOT;
}

RwStaticGeometry* RpClump::CreateStaticGeometry()
{
    return pStatic = new RwStaticGeometry();
}

RpAnimHierarchy* RpClump::GetAtomicAnimHierarchy()
{
    RpAtomic *atomic = GetFirstAtomic();

    if ( !atomic )
        return NULL;
    
    return atomic->anim;
}

RpAnimHierarchy* RpClump::GetAnimHierarchy()
{
    return parent->GetAnimHierarchy();
}

struct _rwFrameScanHierarchy
{
    RwFrame **output;
    size_t max;
};

static bool RwFrameGetAssignedHierarchy( RwFrame *child, _rwFrameScanHierarchy *info )
{
    if ( child->hierarchyId && child->hierarchyId < info->max )
        info->output[ child->hierarchyId ] = child;

    return child->ForAllChildren( RwFrameGetAssignedHierarchy, info );
}

void RpClump::ScanAtomicHierarchy( RwFrame **atomics, size_t max )
{
    _rwFrameScanHierarchy info;

    info.output = atomics;
    info.max = max;

    parent->ForAllChildren( RwFrameGetAssignedHierarchy, &info );
}

RpAtomic* RpClump::GetFirstAtomic()
{
    if ( LIST_EMPTY( atomics.root ) )
        return NULL;

    return LIST_GETITEM( RpAtomic, atomics.root.next, atomics );
}

RpAtomic* RpClump::GetLastAtomic()
{
    if ( LIST_EMPTY( atomics.root ) )
        return NULL;

    return LIST_GETITEM( RpAtomic, atomics.root.prev, atomics );
}

RpAtomic* RpClump::FindNamedAtomic( const char *name )
{
    LIST_FOREACH_BEGIN( RpAtomic, atomics.root, atomics )
        if ( strcmp( item->parent->szName, name ) == 0 )
            return item;
    LIST_FOREACH_END

    return NULL;
}

static bool RwAtomicGet2dfx( RpAtomic *child, RpAtomic **atomic )
{
    // Crashfix, invalid geometry
    if ( !child->geometry )
        return true;

    if ( !child->geometry->_2dfx || child->geometry->_2dfx->count == 0 )
        return true;

    *atomic = child;
    return false;
}

RpAtomic* RpClump::Find2dfx()
{
    RpAtomic *atomic;

    if ( ForAllAtomics( RwAtomicGet2dfx, &atomic ) )
        return NULL;

    return atomic;
}

static bool RwAtomicSetupPipeline( RpAtomic *child, int )
{
    if ( child->IsNight() )
        RpAtomicSetupObjectPipeline( child );
    else if ( child->pipeline == RW_ATOMIC_RENDER_VEHICLE )
        RpAtomicSetupVehiclePipeline( child );

    return true;
}

void RpClump::SetupAtomicRender()
{
    ForAllAtomics( RwAtomicSetupPipeline, 0 );
}

static bool RwAtomicRemoveComponentFlags( RpAtomic *child, unsigned short flags )
{
    child->componentFlags &= ~flags;
    return true;
}

void RpClump::RemoveAtomicComponentFlags( unsigned short flags )
{
    ForAllAtomics( RwAtomicRemoveComponentFlags, flags );
}

static bool RwAtomicFetchMateria( RpAtomic *child, RpMaterials *mats )
{
    child->FetchMateria( *mats );
    return true;
}

void RpClump::FetchMateria( RpMaterials& mats )
{
    ForAllAtomics( RwAtomicFetchMateria, &mats );
}

void RpClump::GetBoneTransform( CVector *offset )
{
    RpAtomic *atomic;
    RpSkeleton *skel;
    RpAnimHierarchy* anim;
    RwBoneInfo *bone;
    RwMatrix *skelMat;
    unsigned int n;
    unsigned int matId = 0;
    unsigned int boneIndexes[20];
    unsigned int *idxPtr = boneIndexes;

    if ( !offset )
        return;

    atomic = GetFirstAtomic();
    skel = atomic->geometry->skeleton;

    anim = atomic->anim;

    // Reset the matrix
    offset->fX = 0;
    offset->fY = 0;
    offset->fZ = 0;

    offset++;

    // Do nothing if the bone count is smaller than 2
    if ( skel->boneCount < 2 )
        return;

    skelMat = skel->boneMatrices + 1;
    bone = anim->boneInfo + 1;

    // We apparrently have the first one initialized already?
    for ( n = 1; n < skel->boneCount; n++ )
    {
        RwMatrix mat;

        RwMatrixInvert( &mat, skelMat );

        pRwInterface->m_matrixTransform3( offset, &mat.vUp, 1, skel->boneMatrices + matId );

        // Some sort of stacking mechanism, maximum 20
        if ( bone->flags & 0x02 )
            *(++idxPtr) = matId;

        if ( bone->flags & 0x01 )
            matId = *(idxPtr--);
        else
            matId = n;

        skelMat++;
        offset++;
        bone++;
    }
}

static RpClump* _clumpCallback( RpClump *clump, void *data )
{
    return clump;
}

RpClump* RpClumpCreate()
{
    RpClump *clump = (RpClump*)pRwInterface->m_allocStruct( pRwInterface->m_clumpInfo, 0x30010 );

    if ( !clump )
        return NULL;

    clump->type = RW_CLUMP;

    LIST_CLEAR( clump->atomics.root );
    LIST_CLEAR( clump->lights.root );
    LIST_CLEAR( clump->cameras.root );

    clump->subtype = 0;
    clump->flags = 0;
    clump->privateFlags = 0;
    clump->parent = NULL;

    LIST_INITNODE( clump->globalClumps );

    clump->callback = _clumpCallback;

    RwObjectRegister( (void*)0x008D6264, clump );
    return clump;
}