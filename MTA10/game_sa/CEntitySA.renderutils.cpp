/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        game_sa/CEntitySA.renderutils.cpp
*  PURPOSE:     Entity render utilities
*  DEVELOPERS:  Martin Turski <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#include <StdInc.h>
#include "CEntitySA.render.hxx"

bool __cdecl EntityRender::AreScreenEffectsEnabled( void )
{
    return !*(bool*)0x00C402B7 && ( *(bool*)0x00C402B8 || *(bool*)0x00C402B9 );
}

void __cdecl EntityRender::SetupNightVisionLighting( void )
{
    // Check whether effects and night vision are enabled
    if ( !*(bool*)0x00C402B8 || *(bool*)0x00C402B7 )
        return;

    RwColorFloat color( 0, 1.0f, 0, 1.0f );

    (*(RpLight**)0x00C886E8)->SetColor( color );    // first light
    (*(RpLight**)0x00C886EC)->SetColor( color );    // second light
}

void __cdecl EntityRender::SetupInfraRedLighting( void )
{
    // Check if effects and infra red are enabled
    if ( !*(bool*)0x00C402B9 || *(bool*)0x00C402B7 )
        return;

    RwColorFloat color( 0, 0, 1.0f, 1.0f );

    (*(RpLight**)0x00C886E8)->SetColor( color );    // first light
    (*(RpLight**)0x00C886EC)->SetColor( color );    // second light
}

float _tempDayNightBalance = 0;

void __cdecl EntityRender::NormalizeDayNight( void )
{
    if ( *(bool*)0x00C402B9 )
        return;

    _tempDayNightBalance = *(float*)0x008D12C0;
    *(float*)0x008D12C0 = 1.0f;
}

void __cdecl EntityRender::RestoreDayNight( void )
{
    if ( *(bool*)0x00C402B9 )
        return;

    *(float*)0x008D12C0 = _tempDayNightBalance;
}

/*=========================================================
    EntityRender::GetComplexCameraEntityDistance

    Arguments:
        entity - the entity to push into the rendering system
                 for rendering on this frame
        camDistance - distance from the camera to the entity
    Purpose:
        This routine returns the actual camera distance of
        the given entity. It returns a normal distance up to
        300 game units. If further away, it optimizes the distance.
    Binary offsets:
        (1.0 US and 1.0 EU): 0x0055458A (derived from)
=========================================================*/
float EntityRender::GetComplexCameraEntityDistance( const CEntitySAInterface *entity, const CVector& camPos )
{
    CCameraSAInterface& camera = Camera::GetInterface();;

    const CVector& pos = entity->Placeable.GetPosition();
    
    float camDistance = ( camPos - pos ).Length();

    if ( camDistance > 300.0f )
    {
        CBaseModelInfoSAInterface *info = entity->GetModelInfo();
        float scaledLODDistance = info->fLodDistanceUnscaled * camera.LODDistMultiplier;

        if ( 300.0f < scaledLODDistance && ( scaledLODDistance + 20.0f ) > camDistance )
        {
            return scaledLODDistance - 300.0f + camDistance;
        }
    }

    return camDistance;
}

/*=========================================================
    EntityRender::CalculateFadingAlpha

    Arguments:
        info - the model info associated with the given entity
        entity - the entity to get the fading alpha of
        camDistance - camera distance of the given entity
        camFarClip - draw distance to use for this calculation
    Purpose:
        Returns the alpha value that should be used to fade
        GTA:SA entities over a distance. The alpha value depends
        on the model LOD distance, the farclip, the camera distance,
        the model bounding radius and whether the model is declared
        "big".
    Binary offsets:
        (1.0 US and 1.0 EU): 0x00732500 (based on)
=========================================================*/
float EntityRender::CalculateFadingAlpha( CBaseModelInfoSAInterface *info, const CEntitySAInterface *entity, float camDistance, float camFarClip )
{
    float sectorDivide = 20.0f;
    float lodScale = pGame->GetCamera()->GetInterface()->LODDistMultiplier;
    float distAway = entity->GetColModel()->m_bounds.fRadius + camFarClip;
    float unscaledLODDistance = info->fLodDistanceUnscaled;
    float scaledLODDistance = unscaledLODDistance * lodScale;

    float useDist = distAway;

    if ( scaledLODDistance < distAway )
        useDist = scaledLODDistance;

    if ( !entity->m_lod )
    {
        float useDist2 = unscaledLODDistance;

        if ( unscaledLODDistance > useDist )
            useDist2 = useDist;

        if ( useDist2 > 150.0f )
            sectorDivide = useDist2 * 0.06666667f + 10.0f;

        if ( entity->m_entityFlags & ENTITY_BIG )
        {
            useDist *= *(float*)0x008CD804;
        }
    }

    useDist += 20.0f;
    useDist -= camDistance;
    useDist /= sectorDivide;

    if ( useDist < 0.0f )
        return 0;

    if ( useDist > 1.0f )
        useDist = 1;

    return useDist * info->ucAlpha;
}

/*=========================================================
    CEntitySAInterface::GetFadingAlpha (MTA extension)

    Purpose:
        Returns the fading alpha value that is used for this
        entity under normal circumstances. Using the return value
        the runtime can determine whether this entity is visible
        for the player or not.
=========================================================*/
float CEntitySAInterface::GetFadingAlpha( void ) const
{
    using namespace EntityRender;

    float camDistance = GetComplexCameraEntityDistance( this, Camera::GetInterface().Placeable.GetPosition() );

    return (unsigned char)CalculateFadingAlpha( ppModelInfo[m_model], this, camDistance, *(float*)0x00B7C4F0 );
}

// Binary offsets: (1.0 US and 1.0 EU): 0x00735C70
void __cdecl EntityRender::DisableSecondLighting( void )
{
    GetSecondLight()->SetLightActive( false );
}

// Binary offsets: (1.0 US and 1.0 EU): 0x00735D30
void __cdecl EntityRender::SetupPostProcessLighting( void )
{
    GetFirstLight()->SetColor( *(RwColorFloat*)0x00C886D4 );
    GetFirstLight()->SetLightActive( true );
}

unsigned char CEntitySAInterface::_SetupLighting( void )
{
    if ( !IS_ANY_FLAG( m_entityFlags, ENTITY_LIGHTING ) )
        return 0;

    GetSecondLight()->SetLightActive( true );

    float brightness = ((float (__cdecl*)( const CVector& pos, unsigned int, CEntitySAInterface* ))0x006FFBB0)( Placeable.GetPosition(), 0, this );

    ((void (__cdecl*)( float brightness ))0x00735D90)( (float)( brightness * 0.5f ) );

    return 1;
}

void CEntitySAInterface::_RemoveLighting( unsigned char id )
{
    if ( id == 0 )
        return;

    EntityRender::SetupPostProcessLighting();
    EntityRender::DisableSecondLighting();

    ((void (__cdecl*)( void ))0x006FFFE0)();
}

/*=========================================================
    EntityRender::GetEntityCameraDistance

    Arguments:
        entity - the entity to get the camera distance of
    Purpose:
        Returns the general-purpose camera distance of the given entity.
    Note:
        This function has been inlined into several places in the
        GTA:SA engine. Basically, it is used whenever the actual camera
        distance is required during the rendering process.
=========================================================*/
float EntityRender::GetEntityCameraDistance( CEntitySAInterface *entity )
{
    return ( entity->Placeable.GetPosition() - *(CVector*)0x00B76870 ).Length();
}