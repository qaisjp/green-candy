/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        game_sa/RenderWare/CRwObjectSA.cpp
*  PURPOSE:     RenderWare RwObject export
*  DEVELOPERS:  The_GTA <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#include <StdInc.h>

CRwObjectSA::CRwObjectSA( RwObject *obj )
{
    m_object = obj;

    // Most things have frames for identification purposes, such as bone names or vehicle parts
    m_frame = obj->m_parent ? new CRwFrameSA( obj->m_parent ) : NULL;
}

CRwObjectSA::~CRwObjectSA()
{
    if ( !IsFrameExtension() )
        SetFrame( NULL );
}

void CRwObjectSA::SetName( const char *name )
{
    m_frame->SetName( name );
}

const char* CRwObjectSA::GetName() const
{
    return m_frame->GetName();
}

unsigned int CRwObjectSA::GetHash() const
{
    return pGame->GetKeyGen()->GetUppercaseKey( GetName() );
}

void CRwObjectSA::SetFrame( CRwFrame *frame )
{
    // If we are a frame extension and were at a previous frame, unregister us
    if ( m_frame && IsFrameExtension() )
    {
        m_frame->m_objects.remove( this );

        ((RwObjectFrame*)GetObject())->RemoveFromFrame();
    }

    m_frame = dynamic_cast <CRwFrameSA*> ( frame );

    if ( !m_frame )
        return;

    // If we are a frame extension, register us at the new frame
    if ( IsFrameExtension() )
    {
        ((RwObjectFrame*)GetObject())->AddToFrame( m_frame->GetObject() );

        m_frame->m_objects.push_back( this );
    }
}

CRwFrame* CRwObjectSA::GetFrame()
{
    return m_frame;
}

const CRwFrame* CRwObjectSA::GetFrame() const
{
    return m_frame;
}