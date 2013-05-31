/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        game_sa/CRenderWareSA.render.cpp
*  PURPOSE:     RenderWare rendering API routines
*  DEVELOPERS:  Martin Turski <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*  RenderWare is © Criterion Software
*
*****************************************************************************/

#include <StdInc.h>

__forceinline void RwFrameSyncObjects( RwFrame *frame )
{
    // Call synchronization callbacks for all objects.
    LIST_FOREACH_BEGIN( RwObjectFrame, frame->m_objects.root, m_lFrame )
        item->m_callback( item );
    LIST_FOREACH_END
}

__forceinline void RwFrameSyncChildren( RwFrame *child, unsigned int parentFlags )
{
    for ( ; child != NULL; child = child->m_next )
    {
        unsigned int flags = child->m_privateFlags | parentFlags;

        if ( flags & RW_FRAME_UPDATEMATRIX )
            child->m_modelling.Multiply( child->m_parent->m_ltm, child->m_ltm );
            //((void (__cdecl*)( RwMatrix&, const RwMatrix&, const RwMatrix& ))0x007F18F0)( child->m_ltm, child->m_modelling, child->m_parent->m_ltm );

        RwFrameSyncObjects( child );

        child->m_privateFlags &= ~0x0C;

        RwFrameSyncChildren( child->m_child, flags );
    }
}

__forceinline void RwFrameSyncChildrenOnlyObjects( RwFrame *child )
{
    for ( ; child != NULL; child = child->m_next )
    {
        RwFrameSyncObjects( child );

        child->m_privateFlags &= ~0x08;

        RwFrameSyncChildrenOnlyObjects( child->m_child );
    }
}

__forceinline void RwFrameSyncDirtyList( RwList <RwFrame>& frameRoot )
{
    LIST_FOREACH_BEGIN( RwFrame, frameRoot.root, m_nodeRoot )
        if ( !LIST_ISVALID( *iter ) )
            __asm int 3

        unsigned int flags = item->m_privateFlags;

        if ( flags & RW_FRAME_DIRTY )
        {
            unsigned int dirtFlags = flags & RW_FRAME_UPDATEMATRIX;

            if ( dirtFlags )
                item->m_ltm = item->m_modelling;

            RwFrameSyncObjects( item );

            RwFrameSyncChildren( item->m_child, dirtFlags );
        }
        else
        {
            RwFrameSyncObjects( item );

            RwFrameSyncChildrenOnlyObjects( item->m_child );
        }

        item->m_privateFlags = flags & ~0x0F;
    LIST_FOREACH_END

    // We have no more dirty frames.
    LIST_CLEAR( frameRoot.root );
}

static RwList <RwFrame> mtaFrameDirtyList;

static unsigned int __cdecl RwFrameSyncDirty( void )
{
    RwFrameSyncDirtyList( (*ppRwInterface)->m_nodeRoot );
    RwFrameSyncDirtyList( mtaFrameDirtyList );  // we keep our own list for security reasons.
    return true;
}

void RwFrameAddToDirtyList_MTA( RwFrame *frame )
{
    LIST_INSERT( mtaFrameDirtyList.root, frame->m_nodeRoot );
}

RwList <RwFrame>& RwFrameGetDirtyList_MTA( void )
{
    return mtaFrameDirtyList;
}

void RenderWareRender_Init( void )
{
    // Keep our own forced list (as GTA:SA is playing around with RenderWare internal data)
    LIST_CLEAR( mtaFrameDirtyList.root );
    
    switch( pGame->GetGameVersion() )
    {
    case VERSION_EU_10:
        HookInstall( 0x00809590, (DWORD)RwFrameSyncDirty, 5 );
        break;
    case VERSION_US_10:
        HookInstall( 0x00809550, (DWORD)RwFrameSyncDirty, 5 );
        break;
    }
}

void RenderWareRender_Shutdown( void )
{
}