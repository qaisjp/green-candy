/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        game_sa/CEntitySA.rendersetup.cpp
*  PURPOSE:     Entity render management
*  DEVELOPERS:  Martin Turski <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#include <StdInc.h>
#include "CEntitySA.render.hxx"

/*
    Entity Render Mode System description:

    The Entity Render Mode System allows the runtime to set the way how things
    should be rendered inside of the GTA:SA engine. It has been introduced to
    allow extension of the engine without throwing away old principles.
    - Users that things to behave the old way can specify the original render mode.
    - Users that want to make use of engine-specific rendering fixes can specify
      one of many world rendering modes (meshlocal_alphafix, scene_alphafix, ...).

    AVAILABLE MODES:
    - WORLD_RENDER_ORIGINAL
        This mode promises least infringement with the old way of rendering things.
        It is the way R* has intended things to render inside GTA:SA.
    - WORLD_RENDER_MESHLOCAL_ALPHAFIX
        This mode keeps the sorting of entities like in the original render mode.
        Each atomic geometry consists of multiple meshes. In each rendering callback,
        those meshes are sorted according to opaque and alpha layers. Like this, it
        aims to improve alpha blending on a per-atomic basis.
    - WORLD_RENDER_SCENE_ALPHAFIX
        This mode takes the concept of meshlocal_alphafix and applies it to the whole
        GTA:SA world. Entities are still sorted from back-to-front, but they are now
        in three phases:
            OPAQUE first
            TRANSLUCENT second
            DEPTH last
        This way, sorted entities are least frequent to have alpha issues. In the context
        of each sorted queue, the alpha-lookthrough bug has been fixed.

    For more information, see...
        http://wiki.mtasa.com/wiki/MTA:Eir/functions/engineSetWorldRenderMode
*/

// Variable which decides what render mode is to be used.
static eWorldRenderMode _worldRenderMode = WORLD_RENDER_ORIGINAL;

void Entity::SetWorldRenderMode( eWorldRenderMode mode )
{
    _worldRenderMode = mode;
}

eWorldRenderMode Entity::GetWorldRenderMode( void )
{
    return _worldRenderMode;
}

/*=========================================================
    EntityRender::ReferenceEntityForRendering (MTA extension)

    Arguments:
        entity - the entity to guard during the rendering
                 process
    Purpose:
        Called for each entity that has been requested to
        render during an engine frame. The entity is referenced
        so that its destruction is avoided while it is inside
        of internal rendering lists. This prevents crashes when
        the destructor is called during engine render management.
=========================================================*/
static mainEntityRenderChain_t renderReferenceList( 1000 );

void EntityRender::ReferenceEntityForRendering( CEntitySAInterface *entity )
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

/*=========================================================
    EntityRender::PushUnderwaterEntityForRendering

    Arguments:
        entity - the entity that should be rendered on the
                 underwater sorted list
        distance - the distance of the entity to the camera
    Purpose:
        Pushes the given entity on the underwater rendering
        chain and returns whether it could be successfully
        queued on the chain.
    Binary offsets:
        (1.0 US and 1.0 EU): 0x00733D90
=========================================================*/
bool __cdecl EntityRender::PushUnderwaterEntityForRendering( CEntitySAInterface *entity, float distance )
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

/*=========================================================
    EntityRender::QueueEntityForRendering

    Arguments:
        entity - the entity to queue inside of the rendering system
        camDistance - distance of the given entity to the camera
    Purpose:
        Queues the given entity into the GTA:SA rendering system.
        This function is called for entities that require alpha
        blending for their meshes.
    Binary offsets:
        (1.0 US and 1.0 EU): 0x00734570
=========================================================*/
int __cdecl EntityRender::QueueEntityForRendering( CEntitySAInterface *entity, float camDistance )
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
mainEntityRenderChain_t groundAlphaEntities( 1000 );     // Note that 1000 is also the native limit.
mainEntityRenderChain_t staticRenderEntities( 1000 );
mainEntityRenderChain_t lowPriorityRenderEntities( 1000 );

/*=========================================================
    AllocateEntityRenderSlot (MTA extension)

    Arguments:
        chain - an internal render chain to allocate the entity on
        entity - the entity to put into the render chain
    Purpose:
        Pushes the given entity onto an unsorted internal
        rendering chain. If the rendering chain has reached
        maximum capacity, a new slot is allocated for the
        entity. If allocation has failed, this function
        returns false.
=========================================================*/
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

/*=========================================================
    ClearUnorderedRenderChain (MTA extension)

    Arguments:
        chain - an unsorted internal rendering chain that
                should be cleared of its entries
    Purpose:
        Clears an unsorted internal rendering chain. For
        every entity that resides on it, each is dereferenced.
        When the entity rendering reference count reaches zero,
        it should not be used by the GTA:SA rendering system
        anymore.
=========================================================*/
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

/*=========================================================
    ClearEntityRenderChain (MTA extension)

    Arguments:
        chain - the sorted entity rendering chain to clear
    Purpose:
        Clears a given sorted rendering chain. Each entity that
        resides on it is dereferenced, so that it is free to be
        destroyed once the rendering reference count reaches zero.
=========================================================*/
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

/*=========================================================
    EntityRender::ClearEntityRenderChains

    Purpose:
        Clears all entries from the engine rendering chains.
        After that the rendering chains can be filled with
        new entities that should be rendered each game frame.
    Binary offsets:
        (1.0 US and 1.0 EU): 0x00734540
=========================================================*/
void __cdecl EntityRender::ClearEntityRenderChains( void )
{
    // Clear entity references that were cast when calling RenderEntity.
    ClearUnorderedRenderChain( renderReferenceList );

    // Clear render lists.
    ClearEntityRenderChain( GetDefaultEntityRenderChain() );
    GetBoatRenderChain().Clear();
    ClearEntityRenderChain( GetUnderwaterEntityRenderChain() );
    ClearEntityRenderChain( GetAlphaEntityRenderChain() );
}

/*=========================================================
    EntityRender::ClearFallbackRenderChains

    Purpose:
        Clears native entity rendering chains of their entities.
    Note:
        This function has been inlined into SetupWorldRender.
=========================================================*/
void EntityRender::ClearFallbackRenderChains( void )
{
    // Clear all our infinite chains.
    ClearUnorderedRenderChain( groundAlphaEntities );
    ClearUnorderedRenderChain( staticRenderEntities );
    ClearUnorderedRenderChain( lowPriorityRenderEntities );
}

/*=========================================================
    EntityRender::ClearAllRenderChains

    Purpose:
       This function is used to remove references from any
       entity that has been used by the GTA:SA rendering
       system. This way modifications are allowed to destroy
       entities in their frame callback. Previously entity
       pointers were still held active inside of rendering
       chains, so deleting entities was risky.
    Binary offsets:
        (1.0 US and 1.0 EU): 0x00734540
=========================================================*/
// MTA extension hook to optimize render chain management
// Fixes some bugs when entities are referenced and Lua wants to quit.
void __cdecl EntityRender::ClearAllRenderChains( void )
{
    ClearEntityRenderChains();
    ClearFallbackRenderChains();
}

/*=========================================================
    EntityRender::PushEntityOnRenderQueue

    Arguments:
        entity - the entity to push into the rendering system
                 for rendering on this frame
        camDistance - distance from the camera to the entity
    Purpose:
        Pushes a world entity for rendering during this frame.
        According to its properties, it can be sorted or not.
        There are several rendering chains that the entity can
        be put on.
    Binary offsets:
        (1.0 US and 1.0 EU): 0x005534B0
=========================================================*/
void __cdecl EntityRender::PushEntityOnRenderQueue( CEntitySAInterface *entity, float camDistance )
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
    EntityRender::RegisterLowPriorityRenderEntity

    Arguments:
        entity - entity to set up for rendering
    Purpose:
        Adds an entity to the low priority render queue.
        Usually LOD entities reside in here.
    Binary offsets:
        (1.0 US and 1.0 EU): 0x00554AC4
=========================================================*/
void EntityRender::RegisterLowPriorityRenderEntity( CEntitySAInterface *entity )
{
    AllocateEntityRenderSlot( lowPriorityRenderEntities, entity );
}

/*=========================================================
    Entity::GetEntitiesInRenderQueue (MTA utility)

    Purpose:
        Returns a list of all MTA entities that are queued
        inside of the GTA:SA rendering system this frame.
        This function only returns valid values of called
        within the engine rendering management (i.e. through
        the rendering callbacks).
=========================================================*/
inline void AddMTAEntityToList( CEntitySAInterface *entity, CGame::entityList_t& list )
{
    CEntitySA *mtaEntity = Pools::GetEntity( entity );

    if ( mtaEntity )
        list.push_back( mtaEntity );
}

struct CollectMainEntitiesRenderQueue
{
    CollectMainEntitiesRenderQueue( CGame::entityList_t& list ) : m_list( list )
    { }

    inline bool OnEntry( const unorderedEntityRenderChainInfo& info )
    {
        AddMTAEntityToList( info.entity, m_list );
        return true;
    }

    CGame::entityList_t& m_list;
};

struct CollectEntityRenderChainRenderQueue
{
    CollectEntityRenderChainRenderQueue( CGame::entityList_t& list ) : m_list( list )
    { }

    inline bool OnEntry( const entityRenderInfo& info )
    {
        AddMTAEntityToList( info.entity, m_list );
        return true;
    }

    CGame::entityList_t& m_list;
};

// Rendering utilities.
CGame::entityList_t Entity::GetEntitiesInRenderQueue( void )
{
    CGame::entityList_t list;

    // Process the internal render chains first.
    {
        CollectMainEntitiesRenderQueue queueCollect( list );

        groundAlphaEntities.ExecuteCustom( queueCollect );
        staticRenderEntities.ExecuteCustom( queueCollect );
        lowPriorityRenderEntities.ExecuteCustom( queueCollect );
    }

    // Now process the public ones.
    {
        CollectEntityRenderChainRenderQueue queueCollect( list );

        EntityRender::GetDefaultEntityRenderChain().ExecuteCustom( queueCollect );
        EntityRender::GetUnderwaterEntityRenderChain().ExecuteCustom( queueCollect );
        EntityRender::GetAlphaEntityRenderChain().ExecuteCustom( queueCollect );
    }

    return list;
}

/*=========================================================
    EntityRender::SetupEntityVisibility (CRenderer_SetupEntityVisibility)

    Arguments:
        entity - entity to set up for rendering
        camDistance - optimized distance from camera to entity
    Purpose:
        Initializes the entity for rendering and returns the
        type how the entity should be rendered (WIP).
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

eRenderType __cdecl EntityRender::SetupEntityVisibility( CEntitySAInterface *entity, float& camDistance )
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
            return ENTITY_RENDER_CROSS_ZONES;
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
                        return ENTITY_RENDER_CONTROVERIAL;
                    }

                    if ( activeCam.DirectionWasLooking != 0 )
                    {
                        *(CVehicleSAInterface**)0x00B745D4 = playerVehicle;
                        return ENTITY_RENDER_CONTROVERIAL;
                    }

                    if ( IS_ANY_FLAG( playerVehicle->m_handling->uiModelFlags, MODEL_NOREARWINDOW ) )
                        return ENTITY_RENDER_CONTROVERIAL;

                    if ( vehicleType != VEHICLE_BOAT || modelIndex == VT_REEFER || modelIndex == VT_TROPIC || modelIndex == VT_PREDATOR || modelIndex == VT_SKIMMER )
                    {
                        *(CVehicleSAInterface**)0x00B745D4 = playerVehicle;
                        return ENTITY_RENDER_CONTROVERIAL;
                    }
                }
            }

            if ( IsEntityRenderable( entity ) )
            {
                if ( !entity->IsInStreamingArea() && entity->m_type == ENTITY_TYPE_VEHICLE )
                    return ENTITY_RENDER_CROSS_ZONES;

                if ( !IsEntityValidForDisplay( entity ) )
                    return ENTITY_RENDER_CONTROVERIAL;

                if ( !IS_ANY_FLAG( entity->m_entityFlags, ENTITY_RENDER_LAST ) )
                    return ENTITY_RENDER_DEFAULT;

                entity->m_entityFlags &= ~ENTITY_FADE;

                const CVector& entityPos = entity->Placeable.GetPosition();

                PushEntityOnRenderQueue( entity, ( entityPos - *(CVector*)0x00B76870 ).Length() );
            }

            return ENTITY_RENDER_CROSS_ZONES;
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
                return ENTITY_RENDER_CROSS_ZONES;
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
                return ENTITY_RENDER_CONTROVERIAL;

            if ( !IS_ANY_FLAG( entity->m_entityFlags, ENTITY_RENDER_LAST ) )
                return ENTITY_RENDER_DEFAULT;

            const CVector& entityPos = entity->Placeable.GetPosition();

            QueueEntityForRendering( entity, ( entityPos - *(CVector*)0x00B76870 ).Length() );

            entity->m_entityFlags &= ~ENTITY_FADE;
        }
   
        return ENTITY_RENDER_CROSS_ZONES;
    }

    if ( !entity->IsInStreamingArea() )
        return ENTITY_RENDER_CROSS_ZONES;

    camDistance = GetComplexCameraEntityDistance( entity->m_lod ? entity->m_lod : entity, *(const CVector*)0x00B76870 );

    return ((eRenderType (__cdecl*)( CEntitySAInterface*, CBaseModelInfoSAInterface *model, float camDist, bool ))0x00553F60)( entity, info, camDistance, unkBoolean );
}
