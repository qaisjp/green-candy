/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        game_sa/RenderWare/RwFrame.cpp
*  PURPOSE:     RenderWare native function implementations
*  DEVELOPERS:  Martin Turski <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*  RenderWare is © Criterion Software
*
*****************************************************************************/

#include <StdInc.h>
#include "gamesa_renderware.h"

// MTA function!
void RwFrame::SetModelling( const RwMatrix& mat )
{
    modelling = mat;

    // Update this frame
    UpdateMTA();
}

// MTA function!
void RwFrame::SetPosition( const CVector& pos )
{
    modelling.vPos = pos;

    // Update this frame
    UpdateMTA();
}

const RwMatrix& RwFrame::GetLTM()
{
    // This function will recalculate the LTM if frame is dirty
    return *RwFrameGetLTM( this );
}

void RwFrame::Link( RwFrame *frame )
{
    // Unlink previous relationship of new child
    frame->Unlink();    // interesting side effect: cached if usage of parent

    // Insert the new child at the beginning
    frame->next = child;
    child = frame;

    frame->parent = this;

    frame->SetRootForHierarchy( root );
    frame->root->ThrowUpdate();     // make sure it is not inside the update queue anymore

    // We need to update the child
    frame->Update();
}

void RwFrame::Unlink()
{
    if ( !parent )
        return;

    if ( parent->child == this )
        parent->child = next;
    else
    {
        RwFrame *prev = parent->child;

        while ( prev->next != this )
            prev = prev->next;

        prev->next = next;
    }

    parent = NULL;
    next = NULL;

    SetRootForHierarchy( this );

    // Update this frame, as it became independent
    Update();
}

void RwFrame::SetRootForHierarchy( RwFrame *root )
{
    this->root = root;

    RwFrame *child = this->child;

    while ( child )
    {
        child->SetRootForHierarchy( root );

        child = child->next;
    }
}

static bool RwFrameGetChildCount( RwFrame *child, unsigned int *count )
{
    child->ForAllChildren( RwFrameGetChildCount, count );

    (*count)++;
    return true;
}

unsigned int RwFrame::CountChildren()
{
    unsigned int count = 0;

    ForAllChildren( RwFrameGetChildCount, &count );
    return count;
}

RwFrame* RwFrame::GetFirstChild()
{
    return child;
}

struct _rwFrameFindName
{
    const char *name;
    RwFrame *result;
};

static bool RwFrameGetFreeByName( RwFrame *child, _rwFrameFindName *info )
{
    if ( child->hierarchyId || strcmp( child->szName, info->name ) != 0 )
        return child->ForAllChildren( RwFrameGetFreeByName, info );

    info->result = child;
    return false;
}

RwFrame* RwFrame::FindFreeChildByName( const char *name )
{
    _rwFrameFindName info;

    info.name = name;

    if ( ForAllChildren( RwFrameGetFreeByName, &info ) )
        return NULL;

    return info.result;
}

static bool RwFrameGetByName( RwFrame *child, _rwFrameFindName *info )
{
    if ( stricmp( child->szName, info->name ) != 0 )
        return child->ForAllChildren( RwFrameGetByName, info );

    info->result = child;
    return false;
}

RwFrame* RwFrame::FindChildByName( const char *name )
{
    _rwFrameFindName info;

    info.name = name;

    if ( ForAllChildren( RwFrameGetByName, &info ) )
        return NULL;

    return info.result;
}

struct _rwFrameFindHierarchy
{
    unsigned int    hierarchy;
    RwFrame*        result;
};

static bool RwFrameGetByHierarchy( RwFrame *child, _rwFrameFindHierarchy *info )
{
    if ( child->hierarchyId != info->hierarchy )
        return child->ForAllChildren( RwFrameGetByHierarchy, info );

    info->result = child;
    return false;
}

RwFrame* RwFrame::FindChildByHierarchy( unsigned int id )
{
    _rwFrameFindHierarchy info;

    info.hierarchy = id;

    if ( ForAllChildren( RwFrameGetByHierarchy, &info ) )
        return NULL;

    return info.result;
}

RwFrame* RwFrame::CloneRecursive() const
{
    RwFrame *cloned = RwFrameCloneRecursive( this, NULL );

    if ( !cloned )
        return NULL;

    cloned->SetUpdating( false );
    cloned->Update();
    return cloned;
}

static bool RwFrameGetAnimHierarchy( RwFrame *frame, RpAnimHierarchy **rslt )
{
    if ( frame->anim )
    {
        *rslt = frame->anim;
        return false;
    }

    return frame->ForAllChildren( RwFrameGetAnimHierarchy, rslt );
}

RwObject* RwFrame::GetFirstObject()
{
    if ( LIST_EMPTY( objects.root ) )
        return NULL;

    return LIST_GETITEM( RwObjectFrame, objects.root.next, lFrame );
}

RwObject* RwFrame::GetFirstObject( unsigned char type )
{
    LIST_FOREACH_BEGIN( RwObjectFrame, objects.root, lFrame )
        if ( item->type == type )
            return item;
    LIST_FOREACH_END

    return NULL;
}

struct _rwObjectByIndex
{
    unsigned char type;
    unsigned int idx;
    unsigned int curIdx;
    RwObject *rslt;
};

static bool RwObjectGetByIndex( RwObject *obj, _rwObjectByIndex *info )
{
    if ( obj->type != info->type )
        return true;

    if ( info->idx != info->curIdx )
    {
        info->curIdx++;
        return true;
    }

    info->rslt = obj;
    return false;
}

RwObject* RwFrame::GetObjectByIndex( unsigned char type, unsigned int idx )
{
    _rwObjectByIndex info;
    info.type = type;
    info.idx = idx;
    info.curIdx = 0;
    
    return ForAllObjects( RwObjectGetByIndex, &info ) ? NULL : info.rslt;
}

struct _rwObjCntByType
{
    unsigned char type;
    unsigned int cnt;
};

static bool RwFrameCountObjectsByType( RwObject *obj, _rwObjCntByType *info )
{
    if ( obj->type == info->type )
        info->cnt++;

    return true;
}

unsigned int RwFrame::CountObjectsByType( unsigned char type )
{
    _rwObjCntByType info;
    info.type = type;
    info.cnt = 0;

    ForAllObjects( RwFrameCountObjectsByType, &info );
    return info.cnt;
}

RwObject* RwFrame::GetLastObject()
{
    if ( LIST_EMPTY( objects.root ) )
        return NULL;
    
    return LIST_GETITEM( RwObjectFrame, objects.root.prev, lFrame );
}

static bool RwFrameObjectGetVisibleLast( RwObject *obj, RwObject **dst )
{
    if ( obj->IsVisible() )
        *dst = obj;

    return true;
}

RwObject* RwFrame::GetLastVisibleObject()
{
    RwObject *obj = NULL;

    ForAllObjects( RwFrameObjectGetVisibleLast, &obj );
    return obj;
}

RpAtomic* RwFrame::GetFirstAtomic()
{
    return (RpAtomic*)GetFirstObject( RW_ATOMIC );
}

static bool RwObjectAtomicSetComponentFlags( RpAtomic *atomic, unsigned short flags )
{
    atomic->componentFlags |= flags;
    return true;
}

void RwFrame::SetAtomicComponentFlags( unsigned short flags )
{
    ForAllAtomics( RwObjectAtomicSetComponentFlags, flags );
}

static bool RwFrameAtomicBaseRoot( RpAtomic *atomic, RwFrame *root )
{
    RpAtomicSetFrame( atomic, root );
    return true;
}

struct _rwFrameComponentAtomics
{
    RpAtomic **primary;
    RpAtomic **secondary;
};

static bool RwFrameAtomicFindComponents( RpAtomic *atomic, _rwFrameComponentAtomics *info )
{
    if ( atomic->componentFlags & 0x01 )
    {
        if ( info->primary )
            *info->primary = atomic;
    }
    else if ( atomic->componentFlags & 0x02 )
    {
        if ( info->secondary )
            *info->secondary = atomic;
    }

    return true;
}

void RwFrame::FindComponentAtomics( RpAtomic **okay, RpAtomic **damaged )
{
    _rwFrameComponentAtomics info;

    info.primary = okay;
    info.secondary = damaged;

    ForAllAtomics( RwFrameAtomicFindComponents, &info );
}

RpAnimHierarchy* RwFrame::GetAnimHierarchy()
{
    RpAnimHierarchy *anim;

    if ( this->anim )
        return this->anim;

    // We want false, since it has to interrupt == found
    if ( ForAllChildren( RwFrameGetAnimHierarchy, &anim ) )
        return NULL;

    return anim;
}

static void RwFrameCheckUpdateNode( void )
{
    LIST_FOREACH_BEGIN( RwFrame, (*ppRwInterface)->m_nodeRoot.root, nodeRoot )
        if ( !LIST_ISVALID( *iter ) )
            __asm int 3
    LIST_FOREACH_END
}

void RwFrame::_Update( RwList <RwFrame>& list )
{
    unsigned char flagIntegrity = privateFlags;

    if ( !( flagIntegrity & RW_FRAME_UPDATEFLAG ) )
    {
        // Add it to the internal list
        LIST_INSERT( list.root, nodeRoot );
    }

    privateFlags = ( flagIntegrity | RW_FRAME_UPDATEFLAG );
}

void RwFrame::Update()
{
    root->_Update( (*ppRwInterface)->m_nodeRoot );
    privateFlags |= RW_FRAME_UPDATEMATRIX | 8;
}

void RwFrame::UpdateMTA()
{
    root->_Update( RwFrameGetDirtyList_MTA() );
    privateFlags |= RW_FRAME_UPDATEMATRIX | 8;
}

void RwFrame::ThrowUpdate()
{
    if ( !IsWaitingForUpdate() )
        return;

    LIST_REMOVE( nodeRoot );

    SetUpdating( false );
}