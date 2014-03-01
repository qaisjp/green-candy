/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        game_sa/RenderWare/RpLight.cpp
*  PURPOSE:     RenderWare native function implementations
*  DEVELOPERS:  Martin Turski <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*  RenderWare is © Criterion Software
*
*****************************************************************************/

#include <StdInc.h>
#include "gamesa_renderware.h"

void RpLight::SetLightIndex( unsigned int idx )
{
    lightIndex = std::min( idx, (unsigned int)7 );
}

void RpLight::AddToClump( RpClump *clump )
{
    // Bugfix: remove from previous clump
    RemoveFromClump();

    LIST_INSERT( clump->lights.root, clumpLights );

    this->clump = clump;
}

void RpLight::RemoveFromClump()
{
    if ( !clump )
        return;

    LIST_REMOVE( clumpLights );

    clump = NULL;
}

void RpLight::AddToScene_Global( RwScene *scene )
{
    RemoveFromScene();

    this->scene = scene;

    LIST_INSERT( scene->globalLights.root, sceneLights );
}

void RpLight::AddToScene_Local( RwScene *scene )
{
    RemoveFromScene();

    this->scene = scene;

    if ( scene->parent )
        scene->parent->Update();

    LIST_INSERT( scene->localLights.root, sceneLights );
}

void RpLight::AddToScene( RwScene *scene )
{
    if ( subtype < 0x80 )
        AddToScene_Global( scene );
    else
        AddToScene_Local( scene );
}

void RpLight::RemoveFromScene()
{
    if ( !scene )
        return;

    RwSceneRemoveLight( scene, this );
}

void RpLight::SetColor( const RwColorFloat& color )
{
    this->color = color;
    
    // Check whether we are brightness only
    privateFlags = ( color.r == color.g && color.r == color.b );
}

static void* _lightCallback( void *ptr )
{
    // Nothing to synchronize for lights!
    return ptr;
}

RpLight* RpLightCreate( unsigned char type )
{
    RpLight *light = (RpLight*)pRwInterface->m_allocStruct( pRwInterface->m_lightInfo, 0x30012 );

    if ( !light )
        return NULL;

    light->type = RW_LIGHT;
    light->subtype = type;
    light->color.a = 0;
    light->color.r = 0;
    light->color.g = 0;
    light->color.b = 0;

    light->callback = (RwObjectFrame::syncCallback_t)_lightCallback;
    light->flags = 0;
    light->parent = NULL;

    light->radius = 0;
    light->coneAngle = 0;
    light->privateFlags = 0;

    // Clear the list awareness
    LIST_CLEAR( light->sectors.root );
    LIST_INITNODE( light->clumpLights );

    light->flags = 3; // why write it again? R* hack?
    light->frame = pRwInterface->m_frame;

    RwObjectRegister( (void*)0x008D62F8, light );
    return light;
}

// NEW RenderWare function for MTA
RpLight* RpLightClone( const RpLight *src )
{
    RpLight *obj = RpLightCreate( src->subtype );

    if ( !obj )
        return NULL;

    // Copy values over
    obj->flags = src->flags;
    obj->privateFlags = src->privateFlags;
    obj->RemoveFromFrame();

    // Copy important data
    obj->radius = src->radius;
    obj->color = src->color;
    obj->coneAngle = src->coneAngle;
    obj->unk = src->unk;

    // Clone plugin details.
    ((RwPluginRegistry <RpLight>*)0x008D62F8)->CloneObject( obj, src );
    return obj;
}