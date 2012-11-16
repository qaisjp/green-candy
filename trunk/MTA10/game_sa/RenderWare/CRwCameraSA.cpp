/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        game_sa/RenderWare/CRwCameraSA.cpp
*  PURPOSE:     RenderWare RwCamera export
*  DEVELOPERS:  The_GTA <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#include <StdInc.h>
#include <gamesa_renderware.h>

CRwCameraSA::CRwCameraSA( RwCamera *cam ) : CRwObjectSA( cam )
{
    m_isRendering = false;
    m_model = NULL;
}

CRwCameraSA::~CRwCameraSA()
{
    // Remove from clump
    RemoveFromModel();

    // Also from frame
    SetFrame( NULL );

    // Kill the previously created targets
    DestroyBuffers();

    RwCameraDestroy( GetObject() );
}

void CRwCameraSA::AddToModel( CModel *model )
{
    RemoveFromModel();

    m_model = dynamic_cast <CModelSA*> ( model );

    if ( !m_model )
        return;

    m_model->m_cameras.push_back( this );
    GetObject()->AddToClump( m_model->GetObject() );
}

CModel* CRwCameraSA::GetModel()
{
    return m_model;
}

void CRwCameraSA::RemoveFromModel()
{
    if ( !m_model )
        return;

    GetObject()->RemoveFromClump();

    m_model->m_cameras.remove( this );
    m_model = NULL;
}

void CRwCameraSA::DestroyBuffers()
{
    RwCamera *cam = GetObject();

    if ( cam->m_rendertarget )
        RwRasterDestroy( cam->m_rendertarget );

    if ( cam->m_bufferDepth )
        RwRasterDestroy( cam->m_bufferDepth );
}

void CRwCameraSA::SetRenderSize( int width, int height )
{
    DestroyBuffers();

    GetObject()->m_rendertarget = RwRasterCreate( width, height, 0, RASTER_RENDERTARGET );
    GetObject()->m_bufferDepth = RwRasterCreate( width, height, 0, RASTER_DEPTHBUFFER );
}

void CRwCameraSA::GetRenderSize( int& width, int& height ) const
{
    const RwCamera *cam = GetObject();

    width = cam->m_rendertarget->width;
    height = cam->m_rendertarget->height;
}

bool CRwCameraSA::BeginUpdate()
{
    if ( IsRendering() )
        return true;

    if ( pRwInterface->m_renderCam || !m_frame )
        return false;

    GetObject()->BeginUpdate();

    m_isRendering = true;
    return true;
}

void CRwCameraSA::EndUpdate()
{
    if ( !IsRendering() )
        return;

    GetObject()->EndUpdate();

    m_isRendering = false;
}