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

extern CBaseModelInfoSAInterface **ppModelInfo;

// Render chains used for sorted execution of instances.
atomicRenderChain_t boatRenderChain( 50 );                          // Binary offsets: (1.0 US and 1.0 EU): 0x00C880C8; boatRenderChains, orig 20
static entityRenderChain_t defaultEntityRenderChain( 8000 );        // Binary offsets: (1.0 US and 1.0 EU): 0x00C88120; ???, orig 200
static entityRenderChain_t underwaterEntityRenderChain( 4000 );     // Binary offsets: (1.0 US and 1.0 EU): 0x00C88178; ???, orig 100, used to render underwater entities
static entityRenderChain_t alphaEntityRenderChain( 50 );            // Binary offsets: (1.0 US and 1.0 EU): 0x00C881D0; ???, orig 50, used for "grasshouse" model

entityRenderChain_t& GetDefaultEntityRenderChain( void )    { return defaultEntityRenderChain; }
entityRenderChain_t& GetUnderwaterEntityRenderChain( void ) { return underwaterEntityRenderChain; }
entityRenderChain_t& GetAlphaEntityRenderChain( void )      { return alphaEntityRenderChain; }
atomicRenderChain_t& GetBoatRenderChain( void )             { return boatRenderChain; }

void __cdecl HOOK_InitRenderChains( void )
{
    // Removed chain initializations that we localized (see above).
    new ((void*)0x00C88224) pedRenderChain_t( 8000 );                   // ???, orig 100
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

// Make sure the rendering guards the entities it uses.
struct unorderedEntityRenderChainInfo
{
    void InitFirst( void )
    {
    }

    void InitLast( void )
    {
    }

    bool operator <( const unorderedEntityRenderChainInfo& right )
    {
        return false;
    }

    typedef void (__cdecl*callback_t)( CEntitySAInterface *intf, float dist );

    CEntitySAInterface  *entity;
    bool isReferenced;

    inline void Execute( void )
    { }
};

typedef CRenderChainInterface <unorderedEntityRenderChainInfo> mainEntityRenderChain_t;
static mainEntityRenderChain_t renderReferenceList( 1000 );

void RenderSystemEntityReference( CEntitySAInterface *entity )
{
    if ( entity->Reference() )
    {
        // Add it to a list so we clear references on next render cycle.
        unorderedEntityRenderChainInfo chainInfo;
        chainInfo.entity = entity;
        chainInfo.isReferenced = true;

        if ( renderReferenceList.PushRender( &chainInfo ) == NULL )
        {
            renderReferenceList.AllocateNew();

            if ( renderReferenceList.PushRender( &chainInfo ) == NULL )
                __asm int 3;
        }
    }
}

inline void RenderEntityNative( CEntitySAInterface *entity )
{
    // Reference the entity here, for safety.
    RenderSystemEntityReference( entity );

    if ( AreScreenEffectsEnabled() )
    {
        NormalizeDayNight();

        entity->Render();

        RestoreDayNight();
    }
    else
        entity->Render();
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
    }
    else if ( entity->m_rwObject && entity->m_rwObject->m_type == RW_CLUMP )    // the entity may not have a RenderWare object?!
    {
        // This value will be used for internal GTA:SA vehicles.
        // We might aswell use this alpha value of the clump.
        alpha = (unsigned char)((RpClump*)entity->m_rwObject)->m_alpha;
    }

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
    }

    RenderEntityNative( entity );

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

// based on 0x0055458A (1.0 US and 1.0 EU)
__forceinline float GetComplexCameraEntityDistance( const CEntitySAInterface *entity, const CVector& camPos )
{
    CCameraSAInterface& camera = Camera::GetInterface();;

    const CVector& pos = entity->Placeable.GetPosition();
    
    float camDistance = ( camPos - pos ).Length();

    if ( camDistance > 300.0f )
    {
        CBaseModelInfoSAInterface *info = ppModelInfo[entity->m_model];
        float scaledLODDistance = info->fLodDistanceUnscaled * camera.LODDistMultiplier;

        if ( 300.0f < scaledLODDistance && ( scaledLODDistance + 20.0f ) > camDistance )
        {
            return scaledLODDistance - 300.0f + camDistance;
        }
    }

    return camDistance;
}

static float CalculateFadingAlpha( CBaseModelInfoSAInterface *info, const CEntitySAInterface *entity, float camDistance, float camFarClip )
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

float CEntitySAInterface::GetFadingAlpha( void ) const
{
    float camDistance = GetComplexCameraEntityDistance( this, Camera::GetInterface().Placeable.GetPosition() );

    return (unsigned char)CalculateFadingAlpha( ppModelInfo[m_model], this, camDistance, *(float*)0x00B7C4F0 );
}

inline void InitModelRendering( CBaseModelInfoSAInterface *info )
{
    if ( info->renderFlags & RENDER_ADDITIVE )
        (*ppRwInterface)->m_deviceCommand( (eRwDeviceCmd)11, 2 );
}

inline void ShutdownModelRendering( CBaseModelInfoSAInterface *info )
{
    if ( info->renderFlags & RENDER_ADDITIVE )
        (*ppRwInterface)->m_deviceCommand( (eRwDeviceCmd)11, 6 );
}

static void __cdecl RenderAtomicWithAlpha( CBaseModelInfoSAInterface *info, RpAtomic *atom, unsigned int alpha )
{
    InitModelRendering( info );

    RpAtomicRenderAlpha( atom, alpha );

    ShutdownModelRendering( info );
}

static void __cdecl RenderClumpWithAlpha( CBaseModelInfoSAInterface *info, RpClump *clump, unsigned int alpha )
{
    InitModelRendering( info );

    LIST_FOREACH_BEGIN( RpAtomic, clump->m_atomics.root, m_atomics )
        if ( item->IsVisible() )
            RpAtomicRenderAlpha( item, alpha );
    LIST_FOREACH_END

    ShutdownModelRendering( info );
}

static void __cdecl DefaultRenderEntityHandler( CEntitySAInterface *entity, float camDistance )
{
    RwObject *rwobj = entity->m_rwObject;

    if ( !rwobj )
        return;

    CBaseModelInfoSAInterface *info = ppModelInfo[entity->m_model];

    if ( info->renderFlags & RENDER_NOSHADOW )
        (*ppRwInterface)->m_deviceCommand( (eRwDeviceCmd)8, 0 );

    if ( entity->m_entityFlags & ENTITY_FADE )
    {
        (*ppRwInterface)->m_deviceCommand( (eRwDeviceCmd)30, 0 );

        // Make sure we keep this entity alive till next render cycle.
        // This is to make sure that we do not crash due to deep render links.
        RenderSystemEntityReference( entity );

        unsigned int alpha = (unsigned char)CalculateFadingAlpha( info, entity, camDistance, *(float*)0x00B76848 );

        unsigned int flags = entity->m_entityFlags;
        flags |= ENTITY_RENDERING;

        entity->m_entityFlags = flags;

        if ( flags & ENTITY_BACKFACECULL )
            (*ppRwInterface)->m_deviceCommand( (eRwDeviceCmd)20, 1 );

        unsigned char lightIndex = entity->SetupLighting();

        if ( rwobj->m_type == RW_ATOMIC )
            RenderAtomicWithAlpha( info, (RpAtomic*)rwobj, alpha );
        else
            RenderClumpWithAlpha( info, (RpClump*)rwobj, alpha );

        entity->RemoveLighting( lightIndex );

        flags &= ~ENTITY_RENDERING;
        entity->m_entityFlags = flags;

        if ( flags & ENTITY_BACKFACECULL )
            (*ppRwInterface)->m_deviceCommand( (eRwDeviceCmd)20, 2 );

        (*ppRwInterface)->m_deviceCommand( (eRwDeviceCmd)30, 100 );
    }
    else
    {
        if ( !*(unsigned int*)VAR_currArea && info->renderFlags & RENDER_NOSHADOW )
            (*ppRwInterface)->m_deviceCommand( (eRwDeviceCmd)30, 100 );
        else
            (*ppRwInterface)->m_deviceCommand( (eRwDeviceCmd)30, 0 );

        RenderEntity( entity );
    }

    if ( info->renderFlags & RENDER_NOSHADOW )
        (*ppRwInterface)->m_deviceCommand( (eRwDeviceCmd)8, 1 );
}

// Binary offsets: (1.0 US and 1.0 EU): 0x00733D90
bool __cdecl PushUnderwaterEntityForRendering( CEntitySAInterface *entity, float distance )
{
    entityRenderInfo chainInfo;
    chainInfo.distance = distance;
    chainInfo.entity = entity;
    chainInfo.callback = DefaultRenderEntityHandler;

    entityRenderChain_t::renderChain *node = GetUnderwaterEntityRenderChain().PushRender( &chainInfo );

    if ( node )
        node->m_entry.isReferenced = entity->Reference();

    return node != NULL;
}

int __cdecl QueueEntityForRendering( CEntitySAInterface *entity, float camDistance )
{
    // This function has been optimized a little.
    // Rockstar created two depthLevel instances, we only do once.
    entityRenderInfo level;
    level.callback = DefaultRenderEntityHandler;
    level.entity = entity;
    level.distance = camDistance;

    entityRenderChain_t::renderChain *node = NULL;

    // Try to display the "grasshouse" model
    if ( entity->m_model == *(short*)0x008CD6F4 )
    {
        node = GetAlphaEntityRenderChain().PushRender( &level );
    }
    else
    {
        // If the entity is underwater, it needs a different rendering order
        if ( entity->m_entityFlags & ENTITY_UNDERWATER )
            return PushUnderwaterEntityForRendering( entity, camDistance );
        else
        {
            // Do default render
            node = GetDefaultEntityRenderChain().PushRender( &level );
        }
    }

    // Guard the entity.
    if ( node )
        node->m_entry.isReferenced = entity->Reference();

    return node != NULL;
}

// Native GTA:SA uses static arrays to render world instances.
// That system has become deprecated, because we can render infinite objects.
// Hence, I want to use allocatable lists.
static mainEntityRenderChain_t groundAlphaEntities( 1000 );
static mainEntityRenderChain_t staticRenderEntities( 1000 );
static mainEntityRenderChain_t lowPriorityRenderEntities( 1000 );

inline bool AllocateEntityRenderSlot( mainEntityRenderChain_t& chain, CEntitySAInterface *entity )
{
    unorderedEntityRenderChainInfo chainInfo;
    chainInfo.entity = entity;

    // Attempt to allocate a new render slot.
    mainEntityRenderChain_t::renderChain *node = chain.PushRender( &chainInfo );

    if ( !node )
    {
        // Make sure we can. It can only fail if we are out of memory.
        chain.AllocateNew();

        node = chain.PushRender( &chainInfo );
    }

    // If successful, we reference the entity to prevent its destruction by the system.
    if ( node )
        node->m_entry.isReferenced = entity->Reference();

    return node != NULL;
}

void __forceinline ClearUnorderedRenderChain( mainEntityRenderChain_t& chain )
{
    while ( mainEntityRenderChain_t::renderChain *iter = chain.GetFirstUsed() )
    {
        if ( iter->m_entry.isReferenced )
        {
            // Dereference the entity.
            iter->m_entry.entity->Dereference();
        }

        chain.RemoveItem( iter );
    }
}

inline void ClearEntityRenderChain( entityRenderChain_t& chain )
{
    while ( entityRenderChain_t::renderChain *iter = chain.GetFirstUsed() )
    {
        if ( iter->m_entry.isReferenced )
        {
            // Dereference the entity.
            iter->m_entry.entity->Dereference();
        }

        chain.RemoveItem( iter );
    }
}

// Binary offsets: (1.0 US and 1.0 EU): 0x00734540
void __cdecl ClearEntityRenderChains( void )
{
    // Clear entity references that were cast when calling RenderEntity.
    ClearUnorderedRenderChain( renderReferenceList );

    // Clear render lists.
    ClearEntityRenderChain( GetDefaultEntityRenderChain() );
    GetBoatRenderChain().Clear();
    ClearEntityRenderChain( GetUnderwaterEntityRenderChain() );
    ClearEntityRenderChain( GetAlphaEntityRenderChain() );
}

void __cdecl PushEntityOnRenderQueue( CEntitySAInterface *entity, float camDistance )
{
    CBaseModelInfoSAInterface *model = entity->GetModelInfo();

    model->bHasBeenPreRendered = false;

    if ( !IS_ANY_FLAG( entity->m_entityFlags, ENTITY_FADE ) )
    {
        if ( IS_ANY_FLAG( entity->m_entityFlags, ENTITY_RENDER_LAST ) )
        {
            if ( QueueEntityForRendering( entity, camDistance ) )
            {
                entity->m_entityFlags &= ~ENTITY_FADE;
                return;
            }
        }
    }
    else if ( QueueEntityForRendering( entity, camDistance ) )
        return;

    if ( entity->m_numLOD > 0 && !IS_ANY_FLAG( entity->m_entityFlags, ENTITY_UNDERWATER ) )
    {
        AllocateEntityRenderSlot( groundAlphaEntities, entity );
    }
    else
    {
        AllocateEntityRenderSlot( staticRenderEntities, entity );
    }
}

/*=========================================================
    RegisterLowPriorityRenderEntity

    Arguments:
        entity - entity to set up for rendering
    Purpose:
        Adds an entity to the low priority render queue.
        Usually LOD entities reside in here.
    Binary offsets:
        (1.0 US and 1.0 EU): 0x00554AC4
=========================================================*/
void RegisterLowPriorityRenderEntity( CEntitySAInterface *entity )
{
    AllocateEntityRenderSlot( lowPriorityRenderEntities, entity );
}

struct SetupChainEntitiesForRender
{
    bool __forceinline OnEntry( const entityRenderInfo& info )
    {
        if ( info.callback == DefaultRenderEntityHandler )
        {
            CEntitySAInterface *entity = info.entity;

            entity->m_entityFlags &= ~ENTITY_OFFSCREEN;

            entity->PreRender();
        }

        return true;
    }
};

// System callbacks to notify the mods about important progress.
static gameEntityPreRenderCallback_t _preRenderCallback = NULL;
static gameEntityRenderCallback_t _renderCallback = NULL;
static gameEntityRenderUnderwaterCallback_t _renderUnderwaterCallback = NULL;
static gameEntityPostProcessCallback_t _renderPostProcessCallback = NULL;

void Entity::SetPreRenderCallback( gameEntityPreRenderCallback_t callback )                 { _preRenderCallback = callback; }
void Entity::SetRenderCallback( gameEntityRenderCallback_t callback )                       { _renderCallback = callback; }
void Entity::SetRenderUnderwaterCallback( gameEntityRenderUnderwaterCallback_t callback )   { _renderUnderwaterCallback = callback; }
void Entity::SetRenderPostProcessCallback( gameEntityPostProcessCallback_t callback )       { _renderPostProcessCallback = callback; }

struct SetupDefaultEntitiesForRender
{
    bool __forceinline OnEntry( unorderedEntityRenderChainInfo& info )
    {
        info.entity->PreRender();
        return true;
    }
};

void __cdecl PreRender( void )
{
    groundAlphaEntities.ExecuteCustom( SetupDefaultEntitiesForRender() );
    staticRenderEntities.ExecuteCustom( SetupDefaultEntitiesForRender() );

    // Left out unused render list.
    // Probably left in for Rockstars internal development tools (like map editor).

    // We notify our system.
    if ( _preRenderCallback )
        _preRenderCallback();

    lowPriorityRenderEntities.ExecuteCustom( SetupDefaultEntitiesForRender() );

    GetDefaultEntityRenderChain().ExecuteCustom( SetupChainEntitiesForRender() );
    GetUnderwaterEntityRenderChain().ExecuteCustom( SetupChainEntitiesForRender() );

    ((void (__cdecl*)( void ))0x00707FA0)();
}

inline void ClearFallbackRenderChains( void )
{
    // Clear all our infinite chains.
    ClearUnorderedRenderChain( groundAlphaEntities );
    ClearUnorderedRenderChain( staticRenderEntities );
    ClearUnorderedRenderChain( lowPriorityRenderEntities );
}

// Binary offsets: (1.0 US and 1.0 EU): 0x005556E0
void __cdecl SetupWorldRender( void )
{
    CCameraSAInterface& camera = Camera::GetInterface();

    const CVector& camPos = camera.Placeable.GetPosition();

    unsigned int unkResult = ((unsigned int (__cdecl*)( CVector camPos ))0x0072D9F0)( camPos );

    *(bool*)0x00B745C0 = IS_ANY_FLAG( unkResult, 0x880 );

    if ( !IS_ANY_FLAG( unkResult, 0x80 ) )
    {
        *(bool*)0x00B745C1 = !IS_ANY_FLAG( unkResult, 0x8 );
    }
    else
        *(bool*)0x00B745C1 = true;

    *(float*)0x008CD804 = 1.0f;

    *(bool*)0x00B76851 = false;

    // Calculate big entity LOD scale.
    float newBigScale = *(float*)0x008CD804;

    CPlayerPedSAInterface *ped = GetPlayerPed( -1 );

    if ( ped && ped->m_areaCode == 0 )
    {
        float groundLevel1 = camera.GetGroundLevel( 0 );
        float groundLevel2 = camera.GetGroundLevel( 2 );

        if ( camPos.fZ - groundLevel1 > 50.0f && camPos.fZ - groundLevel2 > 10.0f )
        {
            if ( GetPlayerVehicle( -1, false ) )
                *(bool*)0x00B76851 = true;
        }

        if ( camPos.fZ > 80.0f )
        {
            float divResult = ( camPos.fZ - *(float*)0x008CD808 ) / ( *(float*)0x008CD80C / *(float*)0x008CD808 );

            divResult = std::max( 1.0f, divResult );

            newBigScale = ( *(float*)0x008CD810 - 1.0f ) * divResult;
        }
    }

    *(float*)0x008CD804 = *(float*)0x00B7C548 * newBigScale;

    ((void (__cdecl*)( void ))0x00726DF0)();
    ((void (__cdecl*)( void ))0x007201C0)();

    // Clear all render queues.
    ClearFallbackRenderChains();

    // Set up render globals.
    *(CVector*)0x00B76870 = camPos;
    *(float*)0x00B7684C = camera.Placeable.GetHeading();

    *(float*)0x00B76848 = camera.m_pRwCamera->m_farplane;

    *(void**)0x00B745D0 = (void*)0x00C8E0E0;
    *(void**)0x00B745CC = (void*)0x00C900C8;

    // Prepare stuff for rendering.
    ((void (__cdecl*)( void ))0x00554FE0)();
    ((void (__cdecl*)( void ))0x00553770)();

    // Initialize the recent GC node system that sorts stuff.
    Streaming::InitRecentGCNode();
}

// MTA extension hook to optimize render chain management
// Fixes some bugs when entities are referenced and Lua wants to quit.
void __cdecl ClearAllRenderChains( void )
{
    ClearEntityRenderChains();
    ClearFallbackRenderChains();
}

inline float GetEntityCameraDistance( CEntitySAInterface *entity )
{
    return ( entity->Placeable.GetPosition() - *(CVector*)0x00B76870 ).Length();
}

// Binary offsets: (1.0 US and 1.0 EU): 0x00735C70
void __cdecl DisableSecondLighting( void )
{
    (*(RpLight**)0x00C886EC)->m_flags = 0;
}

// Binary offsets: (1.0 US and 1.0 EU): 0x00735D30
void __cdecl SetupPostProcessLighting( void )
{
    (*(RpLight**)0x00C886E8)->SetColor( *(RwColorFloat*)0x00C886D4 );
}

struct PostProcessEntities
{
     bool __forceinline OnEntry( unorderedEntityRenderChainInfo& info )
    {
        CEntitySAInterface *entity = info.entity;

        if ( entity->m_type == ENTITY_TYPE_BUILDING )
        {
            CBaseModelInfoSAInterface *model = entity->GetModelInfo();

            if ( IS_ANY_FLAG( entity->GetModelInfo()->flags, RENDER_STATIC ) )
            {
                RenderEntityNative( entity );
            }
        }

        return true;
    }
};

// Binary offsets: (1.0 US and 1.0 EU): 0x00553A10
void __cdecl PostProcessRenderEntities( void )
{
    pRwInterface->m_deviceCommand( (eRwDeviceCmd)14, 1 );
    pRwInterface->m_deviceCommand( (eRwDeviceCmd)12, 1 );
    pRwInterface->m_deviceCommand( (eRwDeviceCmd)20, 2 );
    
    // Configure lighting.
    DisableSecondLighting();
    SetupPostProcessLighting();

    // Pre process the entities!
    staticRenderEntities.ExecuteCustom( PostProcessEntities() );

    // Notify the modifications.
    if ( _renderPostProcessCallback )
        _renderPostProcessCallback();
}

struct RenderStaticWorldEntities
{
    bool __forceinline OnEntry( unorderedEntityRenderChainInfo& info )
    {
        CEntitySAInterface *entity = info.entity;

        if ( entity->m_type == ENTITY_TYPE_BUILDING && IS_ANY_FLAG( entity->GetModelInfo()->flags, RENDER_STATIC ) )
            return true;

        bool successfullyRendered = false;

        if ( entity->m_type == ENTITY_TYPE_VEHICLE || entity->m_type == ENTITY_TYPE_PED && ((RpClump*)entity->m_rwObject)->m_alpha != 255 )
        {
            bool isUnderwater = false;

            if ( entity->m_type == ENTITY_TYPE_VEHICLE )
            {
                CVehicleSAInterface *vehicle = (CVehicleSAInterface*)entity;

                if ( vehicle->m_vehicleType == VEHICLE_BOAT )
                {
                    CCamSAInterface& currentCam = Camera::GetInterface().GetActiveCam();

                    if ( currentCam.Mode != 14 )
                    {
                        int dirLook = Camera::GetInterface().GetActiveCamLookDirection();

                        if ( dirLook == 3 || dirLook == 0 )
                        {
                            if ( vehicle->GetRwObject()->m_alpha == 255 )
                            {
                                isUnderwater = true;
                            }
                        }
                    }
                }
                else
                    isUnderwater = IS_ANY_FLAG( vehicle->m_nodeFlags, 0x8000000 );
            }

            float camDistance = GetEntityCameraDistance( entity );
            
            if ( !isUnderwater )
            {
                successfullyRendered = QueueEntityForRendering( entity, camDistance ) == 1;
            }
            else
            {
                successfullyRendered = PushUnderwaterEntityForRendering( entity, camDistance );
            }
        }

        if ( !successfullyRendered )
            RenderEntity( entity );

        return true;
    }
};

struct QuickRenderEntities
{
    bool __forceinline OnEntry( unorderedEntityRenderChainInfo& info )
    {
        RenderEntity( info.entity );
        return true;
    }
};

// Binary offsets: (1.0 US and 1.0 EU): 0x005556E0
void __cdecl RenderWorldEntities( void )
{
    pRwInterface->m_deviceCommand( (eRwDeviceCmd)14, 1 );
    pRwInterface->m_deviceCommand( (eRwDeviceCmd)12, 1 );
    pRwInterface->m_deviceCommand( (eRwDeviceCmd)20, 2 );

    if ( *(unsigned int*)VAR_currArea == 0 )
        pRwInterface->m_deviceCommand( (eRwDeviceCmd)30, 140 );

    staticRenderEntities.ExecuteCustom( RenderStaticWorldEntities() );

    // Notify our system.
    if ( _renderCallback )
        _renderCallback();

    RwCamera *gtaCamera = *(RwCamera**)0x00C1703C;

    gtaCamera->EndUpdate();

    float unk2 = gtaCamera->m_unknown2;

    gtaCamera->m_unknown2 += *(float*)0x008CD814;

    gtaCamera->BeginUpdate();

    groundAlphaEntities.ExecuteCustom( QuickRenderEntities() );

    gtaCamera->EndUpdate();

    gtaCamera->m_unknown2 = unk2;

    gtaCamera->BeginUpdate();
}

// Binary offsets: (1.0 US and 1.0 EU): 0x00733800
void __cdecl RenderGrassHouseEntities( void )
{
    pRwInterface->m_deviceCommand( (eRwDeviceCmd)1, 0 );
    pRwInterface->m_deviceCommand( (eRwDeviceCmd)6, 1 );
    pRwInterface->m_deviceCommand( (eRwDeviceCmd)8, 1 );
    pRwInterface->m_deviceCommand( (eRwDeviceCmd)12, 1 );
    pRwInterface->m_deviceCommand( (eRwDeviceCmd)10, 5 );
    pRwInterface->m_deviceCommand( (eRwDeviceCmd)11, 6 );
    pRwInterface->m_deviceCommand( (eRwDeviceCmd)14, 1 );
    pRwInterface->m_deviceCommand( (eRwDeviceCmd)20, 1 );

    GetAlphaEntityRenderChain().Execute();

    pRwInterface->m_deviceCommand( (eRwDeviceCmd)14, 0 );
}

// Binary offsets: (1.0 US and 1.0 EU): 0x007337D0
void __cdecl RenderUnderwaterEntities( void )
{
    GetUnderwaterEntityRenderChain().Execute();

    // Notify the system.
    if ( _renderUnderwaterCallback )
        _renderUnderwaterCallback();
}

// Binary offsets: (1.0 US and 1.0 EU): 0x00733EC0
void __cdecl RenderBoatAtomics( void )
{
    pRwInterface->m_deviceCommand( (eRwDeviceCmd)20, 1 );

    GetBoatRenderChain().Execute();

    pRwInterface->m_deviceCommand( (eRwDeviceCmd)20, 2 );
}

// Binary offsets: (1.0 US and 1.0 EU): 0x00733F10
void __cdecl RenderDefaultOrderedWorldEntities( void )
{
    GetDefaultEntityRenderChain().Execute();

    RenderBoatAtomics();
}

/*=========================================================
    CRenderer_SetupEntityVisibility

    Arguments:
        entity - entity to set up for rendering
        camDistance - optimized distance from camera to entity
    Purpose:
        Initializes the entity for rendering and returns the
        type how the entity should be rendered.
    Binary offsets:
        (1.0 US and 1.0 EU): 0x00554230
=========================================================*/
inline bool IsEntityRenderable( CEntitySAInterface *entity )
{
    return entity->m_rwObject && ( IS_ANY_FLAG( entity->m_entityFlags, ENTITY_VISIBLE ) || ( *(unsigned int*)0x00C7C724 && entity->m_model == 0 ) );
}

inline bool IsEntityValidForDisplay( CEntitySAInterface *entity )
{
    return entity->IsOnScreen() && !entity->CheckScreenValidity();
}

inline bool GetEntityTunnelOptimization( CEntitySAInterface *entity )
{
    return IS_ANY_FLAG( entity->m_entityFlags, ENTITY_TUNNELTRANSITION ) && (
           *(bool*)0x00B745C0 && IS_ANY_FLAG( entity->m_entityFlags, ENTITY_TUNNEL ) ||
           *(bool*)0x00B745C1 && !IS_ANY_FLAG( entity->m_entityFlags, ENTITY_TUNNEL ) );
}

eRenderType __cdecl CRenderer_SetupEntityVisibility( CEntitySAInterface *entity, float& camDistance )
{
    modelId_t modelIndex = entity->m_model;
    CBaseModelInfoSAInterface *info = ppModelInfo[modelIndex];

    CAtomicModelInfoSA *atomicInfo = info->GetAtomicModelInfo();

    bool unkBoolean = true;

    if ( entity->m_type == ENTITY_TYPE_VEHICLE && !IS_ANY_FLAG( entity->m_entityFlags, ENTITY_TUNNELTRANSITION ) )
    {
        // Exclude inside-tunnel or outside-tunnel entities depending on game settings.
        // Should optimize FPS a little.
        if ( GetEntityTunnelOptimization( entity ) )
        {
            return ENTITY_RENDER_NONE;
        }
    }

    CCameraSAInterface& camera = Camera::GetInterface();

    eModelType modelType = info->GetModelType();

    if ( !atomicInfo )
    {
        if ( modelType != (eModelType)5 && modelType != (eModelType)4 )
        {
            CVehicleSAInterface *playerVehicle = GetPlayerVehicle( -1, false );

            if ( playerVehicle == entity && *(bool*)0x00B6EC20 && !*(bool*)0x00A43088 )
            {
                unsigned int camIndex = camera.ActiveCam;
                CCamSAInterface& activeCam = camera.m_cams[camIndex];

                eVehicleType vehicleType = playerVehicle->m_vehicleType;

                if ( vehicleType != VEHICLE_BIKE || !IS_ANY_FLAG( ((CBikeSAInterface*)playerVehicle)->m_bikeFlags, 0x80 ) )
                {
                    if ( activeCam.DirectionWasLooking == 3 || modelIndex == VT_RHINO || modelIndex == VT_COACH || *(bool*)0x00B6F04A )
                    {
                        return (eRenderType)2;
                    }

                    if ( activeCam.DirectionWasLooking != 0 )
                    {
                        *(CVehicleSAInterface**)0x00B745D4 = playerVehicle;
                        return (eRenderType)2;
                    }

                    if ( IS_ANY_FLAG( playerVehicle->m_handling->uiModelFlags, MODEL_NOREARWINDOW ) )
                        return (eRenderType)2;

                    if ( vehicleType != VEHICLE_BOAT || modelIndex == VT_REEFER || modelIndex == VT_TROPIC || modelIndex == VT_PREDATOR || modelIndex == VT_SKIMMER )
                    {
                        *(CVehicleSAInterface**)0x00B745D4 = playerVehicle;
                        return (eRenderType)2;
                    }
                }
            }

            if ( IsEntityRenderable( entity ) )
            {
                if ( !entity->IsInStreamingArea() && entity->m_type == ENTITY_TYPE_VEHICLE )
                    return ENTITY_RENDER_NONE;

                if ( !IsEntityValidForDisplay( entity ) )
                    return (eRenderType)2;

                if ( !IS_ANY_FLAG( entity->m_entityFlags, ENTITY_RENDER_LAST ) )
                    return (eRenderType)1;

                entity->m_entityFlags &= ~ENTITY_FADE;

                const CVector& entityPos = entity->Placeable.GetPosition();

                PushEntityOnRenderQueue( entity, ( entityPos - *(CVector*)0x00B76870 ).Length() );
            }

            return ENTITY_RENDER_NONE;
        }
    }
    else if ( modelType == (eModelType)3 )
    {
        ModelInfo::timeInfo *timeInfo = info->GetTimeInfo();

        short timedModel = timeInfo->m_model;

        if ( !Streaming::IsTimeRightForStreaming( timeInfo ) )
        {
            // Delete models that are not required, because they
            // should not render anymore.
            if ( timedModel == -1 || ppModelInfo[timedModel]->GetRwObject() )
            {
                entity->DeleteRwObject();
                return ENTITY_RENDER_NONE;
            }

            unkBoolean = false;
        }
        else if ( timedModel != -1 && ppModelInfo[timedModel]->GetRwObject() )
        {
            info->ucAlpha = 255;
        }
    }
    else if ( IS_ANY_FLAG( entity->m_entityFlags, ENTITY_NOSTREAM ) )
    {
        if ( IsEntityRenderable( entity ) )
        {
            if ( !IsEntityValidForDisplay( entity ) )
                return (eRenderType)2;

            if ( !IS_ANY_FLAG( entity->m_entityFlags, ENTITY_RENDER_LAST ) )
                return (eRenderType)1;

            const CVector& entityPos = entity->Placeable.GetPosition();

            QueueEntityForRendering( entity, ( entityPos - *(CVector*)0x00B76870 ).Length() );

            entity->m_entityFlags &= ~ENTITY_FADE;
        }
   
        return ENTITY_RENDER_NONE;
    }

    if ( !entity->IsInStreamingArea() )
        return ENTITY_RENDER_NONE;

    camDistance = GetComplexCameraEntityDistance( entity->m_lod ? entity->m_lod : entity, *(const CVector*)0x00B76870 );

    return ((eRenderType (__cdecl*)( CEntitySAInterface*, CBaseModelInfoSAInterface *model, float camDist, bool ))0x00553F60)( entity, info, camDistance, unkBoolean );
}

void EntityRender_Init( void )
{
    // Do some interesting patches
    HookInstall( 0x00733A20, (DWORD)HOOK_InitRenderChains, 5 );
    HookInstall( 0x00553260, (DWORD)RenderEntity, 5 );
    HookInstall( 0x00734570, (DWORD)QueueEntityForRendering, 5 );
    HookInstall( 0x00554230, (DWORD)CRenderer_SetupEntityVisibility, 5 );

    HookInstall( 0x005556E0, (DWORD)SetupWorldRender, 5 );
    HookInstall( 0x005534B0, (DWORD)PushEntityOnRenderQueue, 5 );
    HookInstall( 0x00553910, (DWORD)PreRender, 5 );
    HookInstall( 0x00553AA0, (DWORD)RenderWorldEntities, 5 );
    HookInstall( 0x00553A10, (DWORD)PostProcessRenderEntities, 5 );
    HookInstall( 0x00733800, (DWORD)RenderGrassHouseEntities, 5 );
    HookInstall( 0x007337D0, (DWORD)RenderUnderwaterEntities, 5 );
    HookInstall( 0x00733F10, (DWORD)RenderDefaultOrderedWorldEntities, 5 );
    HookInstall( 0x00734540, (DWORD)ClearEntityRenderChains, 5 );

    // Do some optimization.
    PatchCall( 0x0053EBE9, (DWORD)ClearAllRenderChains );
}

void EntityRender_Shutdown( void )
{
}