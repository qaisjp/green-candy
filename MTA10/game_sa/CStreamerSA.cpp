/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        game_sa/CStreamerSA.cpp
*  PURPOSE:     GTA:SA entity streamer extensions
*  DEVELOPERS:  Martin Turski <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#include "StdInc.h"

#define MAX_RENDER_OBJECTS  0x10000

unsigned int *const VAR_renderObjectCount = (unsigned int*)0x00B76840;
unsigned int *const VAR_renderObject2Count = (unsigned int*)0x00B76844;
unsigned int *const VAR_renderObject3Count = (unsigned int*)0x00B76838;
unsigned int *const VAR_renderObject4Count = (unsigned int*)0x00B7683C;
CEntitySAInterface *renderObjects[MAX_RENDER_OBJECTS];
CEntitySAInterface *renderObjects2[MAX_RENDER_OBJECTS];
CEntitySAInterface *renderObjects3[MAX_RENDER_OBJECTS];
CEntitySAInterface *renderObjects4[MAX_RENDER_OBJECTS];

static const DWORD pft = 0x00404180;
static const DWORD pft2 = 0x00404139;

// Callback class for removing all references to a certain building.
struct _buildingRefRemoval
{
    __forceinline _buildingRefRemoval( const CBuildingSAInterface *instance ) : m_instance( instance )
    { }

    void __forceinline EntryCallback( Streamer::entityLink_t*& entry )
    {
        Streamer::entityLink_t *current = entry;

        if ( current->data == m_instance )
        {
            // Deliberate it.
            entry = current->m_next;
            (*ppPtrNodeSinglePool)->Free( (CPtrNodeSingleSAInterface*)current );
        }
    }

    const CBuildingSAInterface *m_instance;
};

static void __cdecl _CBuilding__RemoveReferences( CBuildingSAInterface *building )
{
    Streamer::ForAllStreamedEntities( _buildingRefRemoval( building ), true );
}

static void __declspec(naked) _CBuilding__Delete( void )
{
    // We have to remove the building from the internal streamer
    __asm
    {
        push esi
        push edi
        mov edi,ecx
        call pft
        push edi
        call _CBuilding__RemoveReferences
        pop edi
        jmp pft2
    }
}

namespace Streamer
{
    /*=========================================================
        Streamer::ResetScanIndices

        Purpose:
            Goes through every single streamed in entity and
            sets its m_scanIndex to zero.
        Binary offsets:
            (1.0 US and 1.0 EU): 0x00563470
    =========================================================*/
    struct _resetScanIndex
    {
        void __forceinline EntryCallback( Streamer::entityLink_t *ptrNode )
        {
            ptrNode->data->m_scanCode = 0;
        }
    };

    void __cdecl ResetScanIndices( void )
    {
        ForAllStreamedEntities( _resetScanIndex(), false );
    }

    /*=========================================================
        Streamer::RequestStaticEntitiesOfSector

        Arguments:
            entryPtr - pointer to the streamer sector entry
            reqFlags - flags to pass to the RequestModel function
        Purpose:
            Loops through all streamed-in entities of a sector
            and requests the models of all static ones.
        Binary offsets:
            (1.0 US and 1.0 EU): 0x0040C450
    =========================================================*/
    // The best way to maintain R* source code is to split recurring logic into functions like these.
    // Definitions of certain parts are easily changed globally this way.
    static inline bool IsTimeRightForStreaming( CBaseModelInfoSAInterface *model )
    {
        ModelInfo::timeInfo *timeInfo = model->GetTimeInfo();

        // If there is no time describing said entities' streaming time,
        // it is valid to stream it all the time.
        if ( !timeInfo )
            return true;

        // Check the GTA clock.
        return Streaming::IsCurrentHourPeriod( timeInfo->m_fromHour, timeInfo->m_toHour );
    }

    // Check whether we can request the model of said entity for special streaming.
    // Note that this function is only valid in the context of static building streaming.
    static inline bool IsEntityStreamable( CEntitySAInterface *entity )
    {
        // Only proceed if the model is not already loaded.
        if ( !Streaming::GetModelLoadInfo( entity->m_model ).m_eLoading == MODEL_LOADED )
            return false;

        // Only process static entities.
        if ( !IS_ANY_FLAG( entity->m_entityFlags, ENTITY_STATIC ) )
            return false;

        // Entity has to reside in streamable area.
        if ( !Streaming::IsValidStreamingArea( entity->m_areaCode ) )
            return false;

        // Has to be marked as stream-able.
        if ( IS_ANY_FLAG( entity->m_entityFlags, ENTITY_NOSTREAM ) )
            return false;

        // Has to be marked as visible.
        if ( !IS_ANY_FLAG( entity->m_entityFlags, ENTITY_VISIBLE ) )
            return false;

        // The entity has passed general tests.
        return true;
    }

    // Decide whether we can request the model depending on a _heat-up_ phase.
    static inline bool PreRequest( CEntitySAInterface *entity, CBaseModelInfoSAInterface *model )
    {
        // The time has to be right ;)
        if ( !IsTimeRightForStreaming( model ) )
            return false;

        // The_GTA: this appears to be a hack/bugfix by R*, since entities
        // could lose their RenderWare objects while they are still loaded.
        // Does not make sense to me if the system should be without limits.
        if ( model->pRwObject && !entity->m_rwObject )
            entity->CreateRwObject();

        return true;
    }

    void __cdecl RequestStaticEntitiesOfSector( const streamSectorEntry& entryPtr, unsigned int reqFlags )
    {
        for ( const Streamer::entityLink_t *entry = entryPtr.GetList(); entry != NULL; entry = entry->m_next )
        {
            CEntitySAInterface *entity = entry->data;

            // Ignore certain entities whose scan code should be ignored.
            unsigned short applyScanCode = *(unsigned short*)0x00B7CD78;

            if ( entity->m_scanCode != applyScanCode )
            {
                entity->m_scanCode = applyScanCode;

                // Can we stream the entity at all?
                if ( !IsEntityStreamable( entity ) )
                    return;

                CBaseModelInfoSAInterface *model = ppModelInfo[entity->m_model];

                if ( PreRequest( entity, model ) )
                {
                    // Request it!
                    Streaming::RequestModel( entity->m_model, reqFlags );
                }
            }
        }
    }

    /*=========================================================
        Streamer::RequestStaticEntities

        Arguments:
            entryPtr - pointer to the streamer sector entry
            reqFlags - flags to pass to the RequestModel function
        Purpose:
            Loops through all streamed-in entities of a sector
            and requests models matching the sector criteria.
        Binary offsets:
            (1.0 US and 1.0 EU): 0x0040C270
    =========================================================*/
    void __cdecl RequestStaticEntities(
        const streamSectorEntry& entryPtr, float x, float y,
        float minX, float minY, float maxX, float maxY,
        float camFarClip, unsigned int reqFlags
    )
    {
        // We deal with squared distances because we save on sqrt operations
        // when calculating camera->entity distance later on.
        camFarClip *= camFarClip;

        for ( const Streamer::entityLink_t *entry = entryPtr.GetList(); entry != NULL; entry = entry->m_next )
        {
            CEntitySAInterface *entity = entry->data;

            // Ignore certain entities whose scan code should be ignored.
            unsigned short applyScanCode = *(unsigned short*)0x00B7CD78;

            if ( entity->m_scanCode != applyScanCode )
            {
                entity->m_scanCode = applyScanCode;

                // Can we stream the entity at all?
                if ( !IsEntityStreamable( entity ) )
                    return;

                CBaseModelInfoSAInterface *model = ppModelInfo[entity->m_model];
                CCameraSAInterface& camera = Camera::GetInterface();
                
                float scaledLODDistance = model->GetLODDistance() * camera.LODDistMultiplier;
                scaledLODDistance *= scaledLODDistance;

                // Maximize the used LOD distance so we avoid popping of buildings
                // when just using camera far clip distance.
                float highFarClip = std::max( scaledLODDistance, camFarClip );

                // Check whether the entity is inside our streaming quad.
                const CVector& entityPos = entity->Placeable.GetPosition();

                if ( entityPos.fX <= minX || entityPos.fY >= maxX ||
                     entityPos.fY <= minY || entityPos.fY >= maxY )
                    continue;

                // Calculate the squared entity->camera distance.
                float cameraEntityDistSquared = ( CVector2D( x, y ) - CVector2D( entityPos.fX, entityPos.fY ) ).LengthSquared();

                if ( cameraEntityDistSquared >= highFarClip )
                    continue;

                if ( PreRequest( entity, model ) )
                {
                    // Request it!
                    Streaming::RequestModel( entity->m_model, reqFlags );
                }
            }
        }
    }

    /*=========================================================
        Streamer::RequestSquaredSectorEntities

        Arguments:
            reqPos - position to request around of
            reqFlags - flags to pass to the RequestModel function
        Purpose:
            Requests models of entities around the given reqPos
            position. The position given to it should be the
            camera's origin, since the request quad has camFarClip
            width and height without rotation.
        Binary offsets:
            (1.0 US and 1.0 EU): 0x0040C520
    =========================================================*/
    static inline void IncrementScanCode( void )
    {
        unsigned int streamScanCode = *(unsigned short*)0x00B7CD78;

        if ( streamScanCode == 0xFFFF )
        {
            ResetScanIndices();

            // We start again with index 1.
            // 0 is reserved for new entities.
            *(unsigned short*)0x00B7CD78 = 1;
        }
        else
            *(unsigned short*)0x00B7CD78 += 1;
    }

    template <typename numType>
    inline numType CropDimensionMin( numType num )
    {
        return std::max( num, 0 );
    }

    template <typename numType>
    inline numType CropDimensionMax( numType num, const int arraySize )
    {
        return std::min( num, (numType)( arraySize - 1 ) );
    }

    template <typename numType>
    inline numType GetValidSectorCoord( numType num, const int arraySize )
    {
        return CropDimensionMax( CropDimensionMin( num ), arraySize );
    }

    template <typename arrayType>
    __forceinline arrayType& GetSectorEntry( arrayType sector[], const int arraySize, int x, int y )
    {
        // Even though the sector validity check may be redundant here, this is how R* defined it.
        // The optimizer has not removed it for some reason.
        return sector[ GetValidSectorCoord( x, arraySize ) + GetValidSectorCoord( y, arraySize ) * arraySize ];
    }

    template <typename callbackType>
    void __forceinline LoopArrayWorldSectors(
        const int arraySize, float x, float y, float maxPos, float sectorSize, callbackType& cb
    )
    {
        // Create the request quad.
        // sectorSize is expected to be a positive number.
        float minX = x - sectorSize;
        float minY = y - sectorSize;
        float maxX = x + sectorSize;
        float maxY = y + sectorSize;

        // Offset to center all positions into sector center.
        double sectorCenterOffset = (double)arraySize / 2.0f;

        // Because the selection rectangle expands from the middle of (x,y),
        // it can only extend to (maxPos / 2.0). arraySize / maxPos is the stride
        // for every item inside of the array.
        double posStride = (double)arraySize / maxPos / 2.0f;

        // Get the sectorSize parameters in array cell width/height.
        // Since this is a width/height, we do not offset it by sectorCenterOffset.
        int sectorSizeArray = cb.CoordToIndex( sectorSize * posStride );

        // Get the array position of the center coordinates.
        int sectorCenterX = cb.CoordToIndex( x * posStride + sectorCenterOffset );
        int sectorCenterY = cb.CoordToIndex( y * posStride + sectorCenterOffset );

        // Get the coordinates for the sector scan.
        // These are the beginning units for the array reference.
        int sectorX = cb.CoordToIndex( minX * posStride + sectorCenterOffset );
        int sectorY = cb.CoordToIndex( minY * posStride + sectorCenterOffset );

        // The coordinates must start at 0
        // This enforces that streaming does not happen outside the GTA:SA boundaries.
        // The streaming arrays do not work otherwise anyway.
        sectorX = CropDimensionMin( sectorX );
        sectorY = CropDimensionMin( sectorY );

        // Now for the maximum sector coordinates.
        // These are the maximum units for the array reference.
        int sectorMaxX = cb.CoordToIndex( maxX * posStride + sectorCenterOffset );
        int sectorMaxY = cb.CoordToIndex( maxY * posStride + sectorCenterOffset );

        // The coordinates may not overshoot arraySize.
        // These sectors appear to be a*a quads.
        sectorMaxX = CropDimensionMax( sectorMaxX, arraySize );
        sectorMaxY = CropDimensionMax( sectorMaxY, arraySize );

        // Notify that we are about to enter loop.
        cb.Init();

        // Loop through the sector.
        for ( int iter_y = sectorY; iter_y <= sectorMaxY; iter_y++ )
        {
            for ( int iter_x = sectorX; iter_x <= sectorMaxX; iter_x++ )
            {
                // Notify the callback structure.
                cb.OnEntry( iter_x, iter_y, sectorCenterX, sectorCenterY, minX, minY, maxX, maxY, sectorSize, sectorSizeArray );
            }
        }
    }

    struct _requestSquared
    {
        __forceinline _requestSquared( unsigned int reqFlags, const CVector& reqPos ) : m_reqFlags( reqFlags ), m_reqPos( reqPos )
        {
        }

        void __forceinline Init( void )
        {
            // Increment the scan code to next valid.
            IncrementScanCode();
        }

        void __forceinline OnEntry( int x, int y, int centerX, int centerY, float minX, float minY, float maxX, float maxY, float sectorSize, int sectorSizeArray )
        {
            RequestStaticEntities( *( (const streamSectorEntry*)ARRAY_StreamUpdateSectors + x + y * NUM_StreamUpdateSectorRows ), m_reqPos.fX, m_reqPos.fY, minX, minY, maxX, maxY, sectorSize, m_reqFlags );
        }

        int __forceinline CoordToIndex( double coord ) const
        {
            return (unsigned int)coord;
        }

        unsigned int m_reqFlags;
        const CVector& m_reqPos;
    };

    void __cdecl RequestSquaredSectorEntities( const CVector& reqPos, unsigned int reqFlags )
    {
        LoopArrayWorldSectors( NUM_StreamUpdateSectorRows, reqPos.fX, reqPos.fY, WORLD_BOUNDS, *(float*)0x00B76848, _requestSquared( reqFlags, reqPos ) );
    }

    /*=========================================================
        Streamer::RequestAdvancedSquaredSectorEntities

        Arguments:
            reqPos - position to request around of
            reqFlags - flags to pass to the RequestModel function
        Purpose:
            Requests models of entities around the given reqPos
            position. The position given to it should be the
            camera's origin, since the request quad has camFarClip
            width and height without rotation.
        Binary offsets:
            (1.0 US and 1.0 EU): 0x0040D3F0
        Note:
            This is a more sophisticated version of
            RequestSquaredSectorEntities, as it operates with
            streamer sector distances to abstract away far clip
            operations. They probably wanted to improve the engine's
            performance by using this function in special scenarios.
    =========================================================*/
    struct _requestSquaredFast
    {
        __forceinline _requestSquaredFast( unsigned int reqFlags, const CVector& reqPos ) : m_reqFlags( reqFlags ), m_reqPos( reqPos )
        {
        }

        void __forceinline Init( void )
        {
            // Increment the scan code to next valid.
            IncrementScanCode();
        }

        void __forceinline OnEntry( int x, int y, int centerX, int centerY, float minX, float minY, float maxX, float maxY, float sectorSize, int sectorSizeArray )
        {
            // Provide the distance of the sector center to the current select array cell.
            // These variables are massively optimized in original GTA:SA.
            int sectorCenterToLeftDistance = centerX - x;
            int sectorCenterToTopDistance = centerY - y;

            // Get the request entries of interest.
            const streamPrimarySectorEntry& mainSelector = GetSectorEntry( (const streamPrimarySectorEntry*)ARRAY_StreamSectors, NUM_StreamSectorRows, x, y );
            const streamRepeatSectorEntry& repeatSelector = GetSectorEntry( (const streamRepeatSectorEntry*)ARRAY_StreamRepeatSectors, NUM_StreamRepeatSectorRows, x, y );

            // * We request them in this order.
            const streamSectorEntry& primary = mainSelector.first;
            const streamSectorEntry& secondary = repeatSelector.second;
            const streamSectorEntry& tertiary = mainSelector.second;

            // Check whether we should request with high quality or not.
            // This function ain't chicken wuss, you see.
            // Learn true Kappa by seeing what this optimizes to.
            if ( ( sectorCenterToLeftDistance * sectorCenterToLeftDistance + sectorCenterToTopDistance * sectorCenterToTopDistance ) <= ( sectorSizeArray * sectorSizeArray ) )
            {
                // We can optimize this sauce, because (x,y) is assumed to be the camera position.
                // That removes the need for far clip checks.
                RequestStaticEntitiesOfSector( primary, m_reqFlags );
                RequestStaticEntitiesOfSector( secondary, m_reqFlags );
                RequestStaticEntitiesOfSector( tertiary, m_reqFlags );
            }
            else
            {
                // Go through the entities of said sector carefully.
                RequestStaticEntities( primary, m_reqPos.fX, m_reqPos.fY, minX, minY, maxX, maxY, sectorSize, m_reqFlags );
                RequestStaticEntities( secondary, m_reqPos.fX, m_reqPos.fY, minX, minY, maxX, maxY, sectorSize, m_reqFlags );
                RequestStaticEntities( tertiary, m_reqPos.fX, m_reqPos.fY, minX, minY, maxX, maxY, sectorSize, m_reqFlags );
            }

            // All in all, this is a sophisticated quad-tree engine.
            // ... a function a day keeps the worries away ...
        }

        int __forceinline CoordToIndex( double coord ) const
        {
            return (unsigned int)coord;
        }

        unsigned int m_reqFlags;
        const CVector& m_reqPos;
    };

    void __cdecl RequestAdvancedSquaredSectorEntities( const CVector& reqPos, unsigned int reqFlags )
    {
        float sectorSize = ( *(unsigned int*)VAR_currArea == 0 ) ? 80.0f : 40.0f;

        LoopArrayWorldSectors( NUM_StreamSectorRows, reqPos.fX, reqPos.fY, WORLD_BOUNDS, sectorSize, _requestSquaredFast( reqFlags, reqPos ) );
    }
};

void Streamer_Init( void )
{
    // Hook utility functions.
    HookInstall( 0x00563470, (DWORD)Streamer::ResetScanIndices, 5 );
    HookInstall( 0x0040C450, (DWORD)Streamer::RequestStaticEntitiesOfSector, 5 );
    HookInstall( 0x0040C520, (DWORD)Streamer::RequestSquaredSectorEntities, 5 );
    HookInstall( 0x0040D3F0, (DWORD)Streamer::RequestAdvancedSquaredSectorEntities, 5 );

    // Patch render limitations
    *(unsigned int*)0x00553923 = *(unsigned int*)0x005534F5 = *(unsigned int*)0x00553CB3 = (unsigned int)renderObjects;
    *(unsigned int*)0x00553529 = *(unsigned int*)0x00553944 = *(unsigned int*)0x00553A53 = *(unsigned int*)0x00553B03 = (unsigned int)renderObjects2;
    *(unsigned int*)0x00553965 = (unsigned int)renderObjects3;
    *(unsigned int*)0x00553986 = *(unsigned int*)0x00554AC7 = (unsigned int)renderObjects4;

    // Hook special fixes
    HookInstall( 0x00404130, (DWORD)_CBuilding__Delete, 5 );
}

void Streamer_Shutdown( void )
{

}