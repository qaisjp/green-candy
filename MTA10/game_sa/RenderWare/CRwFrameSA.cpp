/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        game_sa/RenderWare/CRwFrameSA.cpp
*  PURPOSE:     RenderWare RwFrame export
*  DEVELOPERS:  The_GTA <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#include <StdInc.h>
#include <gamesa_renderware.h>

bool CRwFrameSA::RwFrameObjectAssign( RwObject *obj, CRwFrameSA *parent )
{
    CRwObjectSA *inst;

    switch( obj->m_type )
    {
    case RW_ATOMIC:
        inst = new CRpAtomicSA( (RpAtomic*)obj );
        break;
    default:
        // TODO: Add the remaining objects (light, camera)
        assert( 0 );
    }
    parent->m_objects.push_back( inst );

    // Add a back-reference
    inst->m_frame = parent;
    return true;
}

static bool RwFrameChildAssign( RwFrame *child, CRwFrameSA *parent )
{
    CRwFrameSA *frame = new CRwFrameSA( child );

    // Allocate the children and objects
    child->ForAllChildren( RwFrameChildAssign, parent );
    child->ForAllObjects( CRwFrameSA::RwFrameObjectAssign, parent );
    
    // Link it into our hierarchy
    frame->m_frame = parent;

    parent->m_children.push_back( frame );
    return true;
}

CRwFrameSA::CRwFrameSA( RwFrame *obj ) : CRwObjectSA( obj )
{
    // Load all frames into the hierarchy
    obj->ForAllChildren( RwFrameChildAssign, this );
}

CRwFrameSA::~CRwFrameSA()
{
    // Destroy all children and objects
    while ( !m_objects.empty() )
        delete m_objects.front();

    while ( !m_children.empty() )
        delete m_children.front();

    RwFrameDestroy( GetObject() );
}

void CRwFrameSA::Link( CRwFrame *child )
{
    CRwFrameSA *frame = dynamic_cast <CRwFrameSA*> ( child );

    // Unlink previous connection
    frame->Unlink();

    // Do the internal link
    GetObject()->Link( frame->GetObject() );

    // Specify the new root
    frame->m_frame = this;
}

void CRwFrameSA::Unlink()
{
    if ( !m_frame )
        return;

    GetObject()->Unlink();

    m_frame = NULL;
}