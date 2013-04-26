/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        game_sa/CEntitySA.render.cpp
*  PURPOSE:     Entity render utilities
*  DEVELOPERS:  Martin Turski <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#include "StdInc.h"

void __cdecl HOOK_InitRenderChains( void )
{
    new ((void*)0x00C88070) atomicRenderChain_t( 50 );      // vehicleRenderChains, orig 20
    new ((void*)0x00C880C8) atomicRenderChain_t( 50 );      // boatRenderChains, orig 20
    new ((void*)0x00C88120) entityRenderChain_t( 4000 );    // ???, orig 200
    new ((void*)0x00C88178) entityRenderChain_t( 3000 );    // ???, orig 100
    new ((void*)0x00C881D0) entityRenderChain_t( 1500 );    // ???, orig 50
    new ((void*)0x00C88224) pedRenderChain_t( 3000 );       // ???, orig 100
}

static bool __cdecl AreScreenEffectsEnabled( void )
{
    return !*(bool*)0x00C402B7 && ( *(bool*)0x00C402B8 || *(bool*)0x00C402B9 );
}

static void __cdecl SetupNightVisionLighting( void )
{
    // Check whether effects and night vision are enabled
    if ( !*(bool*)0x00C402B8 || *(bool*)0x00C402B7 )
        return;

    RwColorFloat color( 0, 1.0f, 0, 1.0f );

    (*(RpLight**)0x00C886E8)->SetColor( color );    // first light
    (*(RpLight**)0x00C886EC)->SetColor( color );    // second light
}

static void __cdecl SetupInfraRedLighting( void )
{
    // Check if effects and infra red are enabled
    if ( !*(bool*)0x00C402B9 || *(bool*)0x00C402B7 )
        return;

    RwColorFloat color( 0, 0, 1.0f, 1.0f );

    (*(RpLight**)0x00C886E8)->SetColor( color );    // first light
    (*(RpLight**)0x00C886EC)->SetColor( color );    // second light
}

float _tempDayNightBalance = 0;

static void __cdecl NormalizeDayNight( void )
{
    if ( *(bool*)0x00C402B9 )
        return;

    _tempDayNightBalance = *(float*)0x008D12C0;
    *(float*)0x008D12C0 = 1.0f;
}

static void __cdecl RestoreDayNight( void )
{
    if ( *(bool*)0x00C402B9 )
        return;

    *(float*)0x008D12C0 = _tempDayNightBalance;
}

void __cdecl RenderEntity( CEntitySAInterface *entity )
{
    // Do not render peds in the game world if they are inside a vehicle
    // (they need a special render pass to prevent drawing errors)
    if ( entity->m_type == ENTITY_TYPE_PED && ((CPedSAInterface*)entity)->IsDrivingVehicle() )
        return;

    unsigned char id = entity->SetupLighting();

    // This way we skip heap allocations
    StaticAllocator <RwRenderStateLock, 1> rsAlloc;
    RwRenderStateLock *alphaRef;

    // Grab the MTA entity
    CEntitySA *mtaEntity = pGame->GetPools()->GetEntity( entity );

    unsigned char alpha = 255;

    if ( mtaEntity )
    {
        // Perform alpha calculations
        if ( entity->m_type == ENTITY_TYPE_VEHICLE )
            alpha = ((CVehicleSA*)mtaEntity)->GetAlpha();
        else if ( entity->m_type == ENTITY_TYPE_OBJECT )
            alpha = ((CObjectSA*)mtaEntity)->GetAlpha();
        else if ( entity->m_type == ENTITY_TYPE_PED )
            alpha = ((CPedSA*)mtaEntity)->GetAlpha();

        if ( alpha != 255 )
        {
            // This has to stay enabled anyway
            RwD3D9SetRenderState( D3DRS_ALPHABLENDENABLE, true );
            RwD3D9SetRenderState( D3DRS_ALPHAFUNC, D3DCMP_GREATER );
            RwD3D9SetRenderState( D3DRS_ALPHATESTENABLE, true );
            RwD3D9SetRenderState( D3DRS_ALPHAREF, 100 );

            // Ensure the RenderStates necessary for proper alpha blending
            alphaRef = new (rsAlloc.Allocate()) RwRenderStateLock( D3DRS_ALPHAREF, 0x00 );
            RwD3D9ApplyDeviceStates();
        }
    }

    // Prepare entity for rendering/enter frame
    if ( entity->m_type == ENTITY_TYPE_VEHICLE )
    {
        // Set up global variables for fast rendering
        CacheVehicleRenderCameraSettings( alpha, entity->m_rwObject );

        // Clear previous vehicle atomics from render chains
        ClearVehicleRenderChains();

        CVehicleSAInterface *veh = (CVehicleSAInterface*)entity;

        // Render the passengers
        veh->RenderPassengers();

        // Prepare vehicle rendering (i.e. texture replacement)
        veh->SetupRender( (CVehicleSA*)mtaEntity );
    }
    else if ( !( entity->m_entityFlags & ENTITY_BACKFACECULL ) )
    {
        // Change texture stage
        (*ppRwInterface)->m_deviceCommand( (eRwDeviceCmd)20, 1 );
    }

    if ( AreScreenEffectsEnabled() )
    {
        if ( *(bool*)0x00C402B8 )
            SetupNightVisionLighting();

        if ( *(bool*)0x00C402B9 )
            SetupInfraRedLighting();

        NormalizeDayNight();

        entity->Render();

        RestoreDayNight();
    }
    else
        entity->Render();

    // Restore values and unset entity rendering status/leave frame
    if ( entity->m_type == ENTITY_TYPE_VEHICLE )
    {
        BOOL_FLAG( entity->m_entityFlags, ENTITY_RENDERING, true );

        // Render the delayed atomics
        ExecuteVehicleRenderChains( alpha );

        BOOL_FLAG( entity->m_entityFlags, ENTITY_RENDERING, false );

        CVehicleSAInterface *veh = (CVehicleSAInterface*)entity;

        // Leave rendering stage
        veh->LeaveRender();
    }   
    else if ( !( entity->m_entityFlags & ENTITY_BACKFACECULL ) )
    {
        // Set texture stage back to two
        (*ppRwInterface)->m_deviceCommand( (eRwDeviceCmd)20, 2 );
    }

    // Does the entity have alpha enabled?
    if ( alpha != 255 )
    {
        // Remove the RenderState locks
        alphaRef->~RwRenderStateLock();
        RwD3D9ApplyDeviceStates();
    }

    entity->RemoveLighting( id );
}

void EntityRender_Init( void )
{
    // Do some interesting patches
    HookInstall( 0x00733A20, (DWORD)HOOK_InitRenderChains, 5 );
    HookInstall( 0x00553260, (DWORD)RenderEntity, 5 );
}

void EntityRender_Shutdown( void )
{
}