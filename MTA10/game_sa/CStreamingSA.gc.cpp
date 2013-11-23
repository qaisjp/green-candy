/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        game_sa/CStreamingSA.gc.cpp
*  PURPOSE:     Streaming garbage collection logic
*  DEVELOPERS:  Martin Turski <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#include <StdInc.h>

#include "CStreamingSA.utils.hxx"

/*=========================================================
    GarbageCollectPeds

    Purpose:
        Loops through the ped model cache and tries to free
        an used model. Return true if it could free one model.
    Binary offsets:
        (1.0 US and 1.0 EU): 0x0040B340
=========================================================*/
bool __cdecl GarbageCollectPeds( void )
{
    if ( *( (int*)ARRAY_PEDSPECMODEL + 5 ) == -1 || ((bool (__cdecl*)( void ))0x00407410)() )
        return false;

    for ( int *iter = (int*)ARRAY_PEDSPECMODEL; iter != (int*)ARRAY_PEDSPECMODEL + 5; iter++ )
    {
        if ( *iter == -1 )
            continue;

        modelId_t id = (modelId_t)*iter;
        CModelLoadInfoSA& loadInfo = Streaming::GetModelLoadInfo( id );

        if ( loadInfo.m_eLoading == MODEL_LOADED && !( loadInfo.m_flags & 0x06 ) && ppModelInfo[id]->usNumberOfRefs == 0 )
        {
            Streaming::FreeModel( id );
            return true;
        }
    }

    return false;
}

/*=========================================================
    GarbageCollectActiveEntities

    Arguments:
        checkOnScreen - if true then entities which are on screen
                        are not collected
        ignoreFlags - model load infos with those flags are not
                      collected
    Purpose:
        Goes through a list of active entities and destroys
        RenderWare objects of those which are not used anymore.
        If their models are not referenced anymore, they are
        free'd. Returns true if it could free one model.
    Binary offsets:
        (1.0 US and 1.0 EU): 0x00409760
=========================================================*/
struct GarbageCollectStreamingEntities
{
    __forceinline GarbageCollectStreamingEntities( bool checkOnScreen, unsigned int ignoreFlags ) : m_checkOnScreen( checkOnScreen ), m_ignoreFlags( ignoreFlags )
    { }

    // Returning true here continues the loop, returning false stops the loop.
    bool __forceinline OnEntry( const Streaming::streamingChainInfo& info )
    {
        CEntitySAInterface *entity = info.entity;

        if ( entity->m_entityFlags & ( ENTITY_CONTACTED | ENTITY_NOCOLLHIT ) )
            return true;

        // Initialize common values.
        CCameraSAInterface& camera = Camera::GetInterface();
        CPlayerPedSAInterface *player = GetPlayerPed( 0xFFFFFFFF );

        RwCamera *rwcam = camera.m_pRwCamera;
        float farplane = rwcam->m_farplane;

        // Calculate the fading distance.
        CBaseModelInfoSAInterface *model = entity->GetModelInfo();
        float scaledLODDistance = model->fLodDistanceUnscaled * camera.LODDistMultiplier;

        if ( entity->m_entityFlags & ENTITY_BIG )
            scaledLODDistance *= *(float*)0x008CD804;

        const CVector& pos = entity->GetLODPosition();
        const CVector& camPos = camera.Placeable.GetPosition();

        float distance = ( pos - camPos ).Length();

        CEntitySAInterface *finalLOD = entity->GetFinalLOD();

        // Check whether we are loading a scene
        if ( !*(bool*)0x009654BD )
        {
            if ( !m_checkOnScreen || finalLOD->IsOnScreen() )
            {
                unsigned char curArea = entity->m_areaCode;

                // We should not delete RenderWare objects of those entities that are clearly visible.
                if ( Streaming::IsValidStreamingArea( curArea ) &&
                     scaledLODDistance + 50.0f < distance && entity->GetColModel()->m_bounds.fRadius + farplane > distance )
                        return true;
            }
        }

        CModelLoadInfoSA& loadInfo = Streaming::GetModelLoadInfo( entity->GetModelIndex() );

        if ( !loadInfo.IsOnLoader() || loadInfo.m_flags & m_ignoreFlags )
            return true;

        if ( player && player->m_pedFlags.bInVehicle && player->m_vehicleObjective == entity )
            return true;

        entity->DeleteRwObject();

        if ( model->GetRefCount() == 0 )
        {
            // We could successfully free a model, so stop here.
            Streaming::FreeModel( entity->GetModelIndex() );
            return false;
        }
        return true;
    }

    bool m_checkOnScreen;
    unsigned int m_ignoreFlags;
};

bool __cdecl GarbageCollectActiveEntities( bool checkOnScreen, unsigned int ignoreFlags )
{
    return Streaming::GetStreamingEntityChain().ExecuteCustom( GarbageCollectStreamingEntities( checkOnScreen, ignoreFlags ) );
}

/*=========================================================
    Streaming::AddActiveEntity

    Arguments:
        entity - pointer to the entity interface
    Purpose:
        Adds this entity to the streaming garbage collection
        management. Returns a node if successful. If not
        successful, GTA:SA will destroy the RenderWare
        data associated to the entity.
    Binary offsets:
        (1.0 US and 1.0 EU): 0x00409650
=========================================================*/
static bool allowInfiniteStreaming = true;
static bool strictNodeDistribution = false;

struct FreeStreamingEntity
{
    bool __forceinline OnEntry( const Streaming::streamingChainInfo& info )
    {
        CEntitySAInterface *entity = info.entity;

        if ( !IS_ANY_FLAG( entity->m_entityFlags, ENTITY_NOCOLLHIT | ENTITY_STATIC ) )
        {
            entity->DeleteRwObject();
            return false;
        }

        return true;
    }
};

Streaming::streamingEntityReference_t* __cdecl Streaming::AddActiveEntity( CEntitySAInterface *entity )
{
    // Possible bugfix, no idea how this happens.
    if ( !entity )
        return NULL;

    // Create the depth level information.
    streamingChainInfo chainInfo;
    chainInfo.entity = entity;

    streamingEntityChain_t& gcMan = GetStreamingEntityChain();

    streamingEntityReference_t *ref = gcMan.PushRender( &chainInfo );

    if ( !ref )
    {
        // The_GTA: I have updated this logic to allow for many different
        // world streaming allocations.
        if ( allowInfiniteStreaming && !strictNodeDistribution )
        {
            // Allocating a new node here will avoid collection of
            // active entities, hence the world will be very clear.
            gcMan.AllocateNew();

            ref = gcMan.PushRender( &chainInfo );
        }

        if ( !ref )
        {
            // This is the default allocation that first tries to free
            // the RenderWare data of an active entity.
            gcMan.ExecuteCustom( FreeStreamingEntity() );

            ref = gcMan.PushRender( &chainInfo );
            // If this fails to allocate too, god mercy.
            // Lag ensues!
        }

        if ( !ref && allowInfiniteStreaming && strictNodeDistribution )
        {
            // ... or we may aswell put no limits.
            gcMan.AllocateNew();

            ref = gcMan.PushRender( &chainInfo );

            // can only fail if no more heap memory available.
        }
    }

    return ref;
}

/*=========================================================
    Streaming::RemoveActiveEntity

    Arguments:
        ref - node in the streaming gc system
    Purpose:
        Removes the node from the streaming garbage collection
        system.
    Binary offsets:
        (1.0 US and 1.0 EU): 0x00409710
=========================================================*/
static Streaming::streamingEntityReference_t *recentStreamingNode = NULL;

void __cdecl Streaming::RemoveActiveEntity( Streaming::streamingEntityReference_t *ref )
{
    // We could have failed to allocate it.
    if ( !ref )
        return;

    // Make sure we keep the recentStreamingNode valid.
    if ( recentStreamingNode == ref )
        recentStreamingNode = ref->next;

    // Free the reference from the system.
    // It can now be taken by another entity.
    Streaming::GetStreamingEntityChain().RemoveItem( ref );
}

/*=========================================================
    Streaming::InitRecentGCNode

    Purpose:
        Initializes the recent GC node. No idea what this
        system is used for. It appears to be obfuscated by
        securom.
    Binary offsets:
        (1.0 US and 1.0 EU): 0x004096C0
=========================================================*/
void __cdecl Streaming::InitRecentGCNode( void )
{
    recentStreamingNode = &GetStreamingEntityChain().m_root;
}

/*=========================================================
    Streaming::SetRecentGCNode

    Arguments:
        node - the streaming gc node to set most recent
    Purpose:
        Sort of reorders the streaming gc nodes by updating
        the most recent streaming node. This way the
        streaming gc manager list is shuffled.
    Binary offsets:
        (1.0 US and 1.0 EU): 0x004096D0
=========================================================*/
void __cdecl Streaming::SetRecentGCNode( streamingEntityReference_t *node )
{
    // If already the most recent, skip.
    streamingEntityReference_t *recentNode = recentStreamingNode;

    if ( recentNode == node )
        return;

    // Do a quick re-insert.
    LIST_REMOVE( *node );
    LIST_APPEND( *recentNode, *node );

    recentStreamingNode = node;
}

/*=========================================================
    FreeDynamicEntityModel

    Purpose:
        Tries to free models which were recently loaded and
        have remained in the loader cache. Returns whether
        it could free at least one model.
    Binary offsets:
        (1.0 US and 1.0 EU): 0x0040CFD0
=========================================================*/
bool __cdecl FreeDynamicEntityModel( void )
{
    return ((bool (__cdecl*)( void ))0x0040C020)();
}

/*=========================================================
    GarbageCollectModels

    Arguments:
        preserveFlags - if a loadInfo has those flags, it
                        is not free'd
    Purpose:
        Loops through all active models and frees the ones which
        are not referenced or used anymore.
    Binary offsets:
        (1.0 US and 1.0 EU): 0x0040CFD0
=========================================================*/
inline bool AttemptEntityCollection( void )
{
    if ( ((ModelIdContainer*)0x008E4C24)->Count() > 7 || *(unsigned int*)0x00B72914 != 0 && ((ModelIdContainer*)0x008E4C24)->Count() > 4 )
    {
        if ( FreeDynamicEntityModel() )
            return true;
    }

    return false;
}

inline bool AttemptPedCollection( void )
{
    return *(unsigned int*)VAR_PEDSPECMODEL > 4 && GarbageCollectPeds();
}

struct ModelGarbageCollectDispatch : public ModelCheckDispatch <false>
{
    __forceinline bool DoBaseModel( modelId_t id )
    {
        CBaseModelInfoSAInterface *info = ppModelInfo[id];

        return info->usNumberOfRefs == 0;
    }

    __forceinline bool DoTexDictionary( modelId_t id )
    {
        CTxdInstanceSA *inst = (*ppTxdPool)->Get( id );

        return inst->m_references == 0 && !CheckTXDDependency( id );
    }

    __forceinline bool DoAnimation( modelId_t id )
    {
        return pGame->GetAnimManager()->GetAnimBlock( id )->GetRefCount() == 0 && !CheckAnimDependency( id );
    }
    
    // There is logic for scripts too ((1.0 US): 0x0040D108); who cares? :D
};

bool __cdecl GarbageCollectModels( unsigned int preserveFlags )
{
    using namespace Streaming;

    // Collect models in general.
    CModelLoadInfoSA *item = GetGarbageCollectModel();

    for ( ; item != GetLastGarbageCollectModel(); item = GetPrevLoadInfo( item ) )
    {
        unsigned short id = item->GetIndex();

        if ( item->m_flags & preserveFlags )
            continue;

        if ( DefaultDispatchExecute( id, ModelGarbageCollectDispatch() ) )
        {
            FreeModel( id );
            return true;
        }
    }

    CCameraSAInterface& camera = Camera::GetInterface();
    const CVector& camPos = camera.Placeable.GetPosition();

    if ( camPos[2] - camera.GetGroundLevel( 0 ) > 50.0f )
    {
        if ( AttemptPedCollection() )
            return true;

        if ( ((ModelIdContainer*)0x008E4C24)->Count() > 4 && FreeDynamicEntityModel() )
            return true;

        if ( GarbageCollectActiveEntities( false, preserveFlags ) )
            return true;

        if ( *(unsigned int*)VAR_PEDSPECMODEL > 4 && GarbageCollectPeds() )
            return true;

        if ( AttemptEntityCollection() )
            return true;
    }

    return GarbageCollectActiveEntities( true, preserveFlags ) || AttemptEntityCollection() || AttemptPedCollection();
}

/*=========================================================
    DestroyUnimportantWorldInstances

    Arguments:
        reqMem - amount of memory which should be reached
    Purpose:
        Frees as many world instances as it can to meet the
        memory requirement.
    Binary offsets:
        (1.0 US and 1.0 EU): 0x0040D7C0
=========================================================*/
void __cdecl DestroyUnimporantWorldInstances( size_t reqMem )
{
#if 0
    if ( *(size_t*)0x008E4CB4 < reqMem )
        return;

    CCameraSAInterface& camera = Camera::GetInterface();
    const CVector& camPos = camera.Placeable.GetPosition();

#endif
    ((void (__cdecl*)( size_t ))0x0040D7C0)( reqMem );
}

/*=========================================================
    UnclogMemoryUsage

    Arguments:
        mem_size - amount of memory required to free
    Purpose:
        Attempts to free the given amount of streaming memory
        so that the allocation of future resources which require
        that amount will not fail due to memory shortage.
    Binary offsets:
        (1.0 US and 1.0 EU): 0x0040E120
=========================================================*/
void __cdecl UnclogMemoryUsage( size_t mem_size )
{
    // Loop until memory is at desired value.
    while ( *(size_t*)0x008E4CB4 > *(size_t*)0x008A5A80 - mem_size )
    {
        if ( !GarbageCollectModels( 0x20 ) )
        {
            DestroyUnimporantWorldInstances( *(size_t*)0x008A5A80 - mem_size );
            return;
        }
    }
}

void StreamingGC_Init( void )
{
    // Install important hooks.
    HookInstall( 0x0040CFD0, (DWORD)GarbageCollectModels, 5 );
    HookInstall( 0x00409650, (DWORD)Streaming::AddActiveEntity, 5 );
    HookInstall( 0x00409710, (DWORD)Streaming::RemoveActiveEntity, 5 );
    HookInstall( 0x004096C0, (DWORD)Streaming::InitRecentGCNode, 5 );
    HookInstall( 0x004096D0, (DWORD)Streaming::SetRecentGCNode, 5 );
}

void StreamingGC_Shutdown( void )
{
}