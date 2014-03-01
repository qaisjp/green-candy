/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        game_sa/RenderWare/RwCamera.cpp
*  PURPOSE:     RenderWare native function implementations
*  DEVELOPERS:  Martin Turski <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*  RenderWare is © Criterion Software
*
*****************************************************************************/

#include <StdInc.h>
#include "gamesa_renderware.h"

RwCamera* RwCameraCreate()
{
    RwCamera *cam = (RwCamera*)pRwInterface->m_allocStruct( pRwInterface->m_cameraInfo, 0x30005 );

    if ( !cam )
        return NULL;

    cam->type = RW_CAMERA;
    cam->subtype = 0;
    cam->flags = 0;
    cam->privateFlags = 0;
    cam->parent = NULL;

    cam->callback = (RwObjectFrame::syncCallback_t)0x007EE5A0;
    cam->preCallback = (RwCameraPreCallback)0x007EF370;
    cam->postCallback = (RwCameraPostCallback)0x007EF340;

    cam->screen.x = cam->screen.y = cam->screenInverse.x = cam->screenInverse.y = 1;
    cam->screenOffset.x = cam->screenOffset.y = 0;

    cam->nearplane = 0.05f;
    cam->farplane = 10;
    cam->fog = 5;

    cam->rendertarget = NULL;
    cam->bufferDepth = NULL;
    cam->camType = RW_CAMERA_PERSPECTIVE;

    // Do some plane shifting
    ((void (__cdecl*)( RwCamera* ))0x007EE200)( cam );

    cam->matrix.a = 0;

    RwObjectRegister( (void*)0x008E222C, cam );
    return cam;
}

void RwCamera::BeginUpdate()
{
    preCallback( this );
}

void RwCamera::EndUpdate()
{
    postCallback( this );
}

void RwCamera::AddToClump( RpClump *clump )
{
    // Bugfix: remove from previous clump
    RemoveFromClump();

    LIST_INSERT( clump->cameras.root, clumpCameras );

    this->clump = clump;
}

void RwCamera::RemoveFromClump()
{
    if ( !clump )
        return;

    LIST_REMOVE( clumpCameras );

    clump = NULL;
}