/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        game_sa/CPoolsSA.cpp
*  PURPOSE:     Game entity pools
*  DEVELOPERS:  Ed Lyons <eai@opencoding.net>
*               Christian Myhre Lundheim <>
*               Cecill Etheredge <ijsf@gmx.net>
*               Jax <>
*               Alberto Alonso <rydencillo@gmail.com>
*               The_GTA <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#include "StdInc.h"

#define FUNC_InitGamePools  0x00550F10

extern CBaseModelInfoSAInterface **ppModelInfo;

CPtrNodeSinglePool      **ppPtrNodeSinglePool = (CPtrNodeSinglePool**)CLASS_CPtrNodeSingleLinkPool;
CPtrNodeDoublePool      **ppPtrNodeDoublePool = (CPtrNodeDoublePool**)CLASS_CPtrNodeDoubleLinkPool;
CEntryInfoPool          **ppEntryInfoPool = (CEntryInfoPool**)CLASS_CEntryInfoNodePool;

CVehiclePool            **ppVehiclePool = (CVehiclePool**)CLASS_CVehiclePool;
CPedPool                **ppPedPool = (CPedPool**)CLASS_CPedPool;
CBuildingPool           **ppBuildingPool = (CBuildingPool**)CLASS_CBuildingPool;
CObjectPool             **ppObjectPool = (CObjectPool**)CLASS_CObjectPool;

CDummyPool              **ppDummyPool = (CDummyPool**)CLASS_CDummyPool;

CColModelPool           **ppColModelPool = (CColModelPool**)CLASS_CColModelPool;

CTaskPool               **ppTaskPool = (CTaskPool**)CLASS_CTaskPool;
CEventPool              **ppEventPool = (CEventPool**)CLASS_CEventPool;

CPointRoutePool         **ppPointRoutePool = (CPointRoutePool**)CLASS_CPointRoutePool;
CPatrolRoutePool        **ppPatrolRoutePool = (CPatrolRoutePool**)CLASS_CPatrolRoutePool;
CNodeRoutePool          **ppNodeRoutePool = (CNodeRoutePool**)CLASS_CNodeRoutePool;

CTaskAllocatorPool      **ppTaskAllocatorPool = (CTaskAllocatorPool**)CLASS_CTaskAllocatorPool;
CPedIntelligencePool    **ppPedIntelligencePool = (CPedIntelligencePool**)CLASS_CPedIntelligencePool;
CPedAttractorPool       **ppPedAttractorPool = (CPedAttractorPool**)CLASS_CPedAttractorPool;

// MTA Pools
CMTAVehiclePool *mtaVehiclePool;
CMTAPedPool *mtaPedPool;
CMTAObjectPool *mtaObjectPool;
CMTAPlayerDataPool *mtaPlayerDataPool;
CMTAObjectDataPool *mtaObjectDataPool;

CPoolsSA::CPoolsSA()
{
    DEBUG_TRACE("CPoolsSA::CPoolsSA()");

    // Do not let GTA:SA init pools!
    *(unsigned char*)FUNC_InitGamePools = 0xC3;

    *ppPtrNodeSinglePool = new CPtrNodeSinglePool();
    *ppPtrNodeDoublePool = new CPtrNodeDoublePool();
    *ppEntryInfoPool = new CEntryInfoPool();

    *ppVehiclePool = new CVehiclePool();
    *ppPedPool = new CPedPool();
    *ppBuildingPool = new CBuildingPool();
    *ppObjectPool = new CObjectPool();

    *ppDummyPool = new CDummyPool();
    *ppColModelPool = new CColModelPool();

    *ppTaskPool = new CTaskPool();
    *ppEventPool = new CEventPool();

    *ppPointRoutePool = new CPointRoutePool();
    *ppPatrolRoutePool = new CPatrolRoutePool();
    *ppNodeRoutePool = new CNodeRoutePool();

    *ppTaskAllocatorPool = new CTaskAllocatorPool();
    *ppPedIntelligencePool = new CPedIntelligencePool();
    *ppPedAttractorPool = new CPedAttractorPool();

    // Init our pools
    mtaVehiclePool = new CMTAVehiclePool;
    mtaPlayerDataPool = new CMTAPlayerDataPool;
    mtaPedPool = new CMTAPedPool;
    mtaObjectDataPool = new CMTAObjectDataPool;
    mtaObjectPool = new CMTAObjectPool;

    // Initialize the entity cache
    memset( mtaVehicles, 0, sizeof(mtaVehicles) );
    memset( mtaPeds, 0, sizeof(mtaPeds) );
    memset( mtaObjects, 0, sizeof(mtaObjects) );
}

CPoolsSA::~CPoolsSA()
{
    // Clear all our entities and delete the pools
    mtaVehiclePool->Clear();
    mtaPedPool->Clear();
    mtaObjectPool->Clear();

    delete mtaObjectPool;
    delete mtaObjectDataPool;
    delete mtaPedPool;
    delete mtaPlayerDataPool;
    delete mtaVehiclePool;

    delete *ppPtrNodeSinglePool;
    delete *ppPtrNodeDoublePool;
    delete *ppEntryInfoPool;

    // Seems to be crashy
#if TODO
    delete *ppVehiclePool;
    delete *ppPedPool;
    delete *ppBuildingPool;
    delete *ppObjectPool;

    delete *ppDummyPool;
#endif
    delete *ppColModelPool;

    delete *ppTaskPool;
    delete *ppEventPool;

    delete *ppPointRoutePool;
    delete *ppPatrolRoutePool;
    delete *ppNodeRoutePool;

    delete *ppTaskAllocatorPool;
    delete *ppPedIntelligencePool;
    delete *ppPedAttractorPool;
}

void CPoolsSA::DeleteAllBuildings()
{
    /*
    for ( int i = 0; i < MAX_BUILDINGS; i++ )
    {
        if ( Buildings [i] )
        {
            RemoveBuilding ( Buildings [i] );
        }
    }
    */
}

//////////////////////////////////////////////////////////////////////////////////////////
//                                    VEHICLES POOL                                     //
//////////////////////////////////////////////////////////////////////////////////////////
CVehicleSA *mtaVehicles[MAX_VEHICLES];

// TODO: analyze this and put it into C++ code; will improve understanding and performance!
static inline CVehicleSAInterface* CreateVehicle( unsigned short modelId )
{
    // for SA, we can just call the following function and it should work:
    // SCM_CreateCarForScript(int,class CVector,unsigned char)
    //                              ModelID, Position, IsMissionVehicle
    CVehicleSAInterface *veh;

    DWORD dwFunc = FUNC_CCarCtrlCreateCarForScript;
    _asm
    {
        push    0           // its a mission vehicle
        push    0
        push    0
        push    0           // spawn at 0,0,0
        movzx   eax,modelId
        push    eax
        call    dwFunc
        add     esp, 0x14
        mov     veh, eax
    }

    return veh;
}

CBicycleSA* CPoolsSA::AddBicycle( unsigned short modelID )
{
    DEBUG_TRACE("CBicycle* CPoolsSA::AddBicycle( unsigned short modelID )");

    if ( modelID > MAX_MODELS-1 )
        return NULL;

    // We do not wanna add when the pool is full
    if ( (*ppVehiclePool)->Full() )
        return NULL;

    CVehicleModelInfoSAInterface *info = (CVehicleModelInfoSAInterface*)ppModelInfo[modelID];

    if ( !info || info->GetModelType() != MODEL_VEHICLE || info->m_vehicleType != VEHICLE_BICYCLE )
        return NULL;

    return new CBicycleSA( (CBicycleSAInterface*)CreateVehicle( modelID ) );
}

CBikeSA* CPoolsSA::AddBike( unsigned short modelID )
{
    DEBUG_TRACE("CBike* CPoolsSA::AddBike( unsigned short modelID )");

    if ( modelID > MAX_MODELS-1 )
        return NULL;

    // We do not wanna add when the pool is full
    if ( (*ppVehiclePool)->Full() )
        return NULL;

    CVehicleModelInfoSAInterface *info = (CVehicleModelInfoSAInterface*)ppModelInfo[modelID];

    if ( !info || info->GetModelType() != MODEL_VEHICLE )
        return NULL;

    switch( info->m_vehicleType )
    {
    case VEHICLE_BIKE:
        return new CBikeSA( (CBikeSAInterface*)CreateVehicle( modelID ) );
    case VEHICLE_BICYCLE:
        return new CBicycleSA( (CBicycleSAInterface*)CreateVehicle( modelID ) );
    }

    return NULL;
}

CHeliSA* CPoolsSA::AddHeli( unsigned short modelID )
{
    DEBUG_TRACE("CHeli* CPoolsSA::AddHeli( unsigned short modelID )");

    if ( modelID > MAX_MODELS-1 )
        return NULL;

    // We do not wanna add when the pool is full
    if ( (*ppVehiclePool)->Full() )
        return NULL;

    CVehicleModelInfoSAInterface *info = (CVehicleModelInfoSAInterface*)ppModelInfo[modelID];

    if ( !info || info->GetModelType() != MODEL_VEHICLE || info->m_vehicleType != VEHICLE_HELI )
        return NULL;

    return new CHeliSA( (CHeliSAInterface*)CreateVehicle( modelID ) );
}

CPlaneSA* CPoolsSA::AddPlane( unsigned short modelID )
{
    DEBUG_TRACE("CPlane* CPoolsSA::AddPlane( unsigned short modelID )");

    if ( modelID > MAX_MODELS-1 )
        return NULL;

    // We do not wanna add when the pool is full
    if ( (*ppVehiclePool)->Full() )
        return NULL;

    CVehicleModelInfoSAInterface *info = (CVehicleModelInfoSAInterface*)ppModelInfo[modelID];

    if ( !info || info->GetModelType() != MODEL_VEHICLE || info->m_vehicleType != VEHICLE_PLANE )
        return NULL;

    return new CPlaneSA( (CPlaneSAInterface*)CreateVehicle( modelID ) );
}

CTrainSA* CPoolsSA::AddTrain( unsigned short modelID, const CVector& pos, bool direction )
{
    DEBUG_TRACE("CTrain* CPoolsSA::AddTrain( unsigned short modelID, const CVector& pos, bool direction )");

    if ( modelID > MAX_MODELS-1 )
        return NULL;

    // We do not wanna add when the pool is full
    if ( (*ppVehiclePool)->Full() )
        return NULL;

    CVehicleModelInfoSAInterface *info = (CVehicleModelInfoSAInterface*)ppModelInfo[modelID];

    if ( !info || info->GetModelType() != MODEL_VEHICLE || info->m_vehicleType != VEHICLE_TRAIN )
        return NULL;

    // clean the existing array
    memset( (unsigned int*)VAR_TrainModelArray, 0, 32 * sizeof(unsigned int) );

    // We only want one model
    *(unsigned short*)VAR_TrainModelArray = modelID; 

    CTrainSAInterface *trainBeginning;
    CTrainSAInterface *trainEnd;
    float fX = pos.fX;
    float fY = pos.fY;
    float fZ = pos.fZ;

    DWORD dwFunc = FUNC_CTrain_CreateMissionTrain;
    _asm
    {
        push    0 // place as close to point as possible (rather than at node)? (maybe) (actually seems to have an effect on the speed, so changed from 1 to 0)
        push    0 // start finding closest from here 
        push    -1 // node to start at (-1 for closest node)
        lea     ecx, trainEnd
        push    ecx // end of train
        lea     ecx, trainBeginning 
        push    ecx // begining of train
        push    0 // train type (always use 0 as thats where we're writing to)
        push    direction
        push    fZ
        push    fY
        push    fX
        call    dwFunc
        add     esp, 0x28
    }

    CTrainSA *trainHead = new CTrainSA( trainBeginning );
#if 0
    CVehicleSA *carriage = trainHead;
    
    while ( carriage )
    {
        CVehicleSAInterface* vehCarriage = carriage->GetNextCarriageInTrain ();

        if ( !vehCarriage )
            break;

        carriage = new CVehicleSA ( vehCarriage );
    }
#endif //0

    // Stops the train from moving at ludacrist speeds right after creation
    // due to some glitch in the node finding in CreateMissionTrain
    trainHead->SetMoveSpeed( CVector( 0, 0, 0 ) );

    return trainHead;
}

CAutomobileTrailerSA* CPoolsSA::AddTrailer( unsigned short modelID )
{
    DEBUG_TRACE("CAutomobileTrailer* CPoolsSA::AddTrailer( unsigned short modelID )");

    if ( modelID > MAX_MODELS-1 )
        return NULL;

    // We do not wanna add when the pool is full
    if ( (*ppVehiclePool)->Full() )
        return NULL;

    CVehicleModelInfoSAInterface *info = (CVehicleModelInfoSAInterface*)ppModelInfo[modelID];

    if ( !info || info->GetModelType() != MODEL_VEHICLE || info->m_vehicleType != VEHICLE_AUTOMOBILETRAILER )
        return NULL;

    return new CAutomobileTrailerSA( (CAutomobileTrailerSAInterface*)CreateVehicle( modelID ) );
}

CQuadBikeSA* CPoolsSA::AddQuadBike( unsigned short modelID )
{
    DEBUG_TRACE("CQuadBike* CPoolsSA::AddQuadBike( unsigned short modelID )");

    if ( modelID > MAX_MODELS-1 )
        return NULL;

    // We do not wanna add when the pool is full
    if ( (*ppVehiclePool)->Full() )
        return NULL;

    CVehicleModelInfoSAInterface *info = (CVehicleModelInfoSAInterface*)ppModelInfo[modelID];

    if ( !info || info->GetModelType() != MODEL_VEHICLE || info->m_vehicleType != VEHICLE_QUADBIKE )
        return NULL;

    return new CQuadBikeSA( (CQuadBikeSAInterface*)CreateVehicle( modelID ) );
}

CMonsterTruckSA* CPoolsSA::AddMonsterTruck( unsigned short modelID )
{
    DEBUG_TRACE("CMonsterTruck* CPoolsSA::AddMonsterTruck( unsigned short modelID )");

    if ( modelID > MAX_MODELS-1 )
        return NULL;

    // We do not wanna add when the pool is full
    if ( (*ppVehiclePool)->Full() )
        return NULL;

    CVehicleModelInfoSAInterface *info = (CVehicleModelInfoSAInterface*)ppModelInfo[modelID];

    if ( !info || info->GetModelType() != MODEL_VEHICLE || info->m_vehicleType != VEHICLE_MONSTERTRUCK )
        return NULL;

    return new CMonsterTruckSA( (CMonsterTruckSAInterface*)CreateVehicle( modelID ) );
}

CAutomobileSA* CPoolsSA::AddAutomobile( unsigned short modelID )
{
    DEBUG_TRACE("CAutomobile* CPoolsSA::AddAutomobile( unsigned short modelID )");

    if ( modelID > MAX_MODELS-1 )
        return NULL;

    // We do not wanna add if the pool is full
    if ( (*ppVehiclePool)->Full() )
        return NULL;

    CVehicleModelInfoSAInterface *info = (CVehicleModelInfoSAInterface*)ppModelInfo[modelID];

    if ( !info || info->GetModelType() != MODEL_VEHICLE )
        return NULL;

    switch( info->m_vehicleType )
    {
    case VEHICLE_PLANE:
        return AddPlane( modelID );
    case VEHICLE_HELI:
        return AddHeli( modelID );
    case VEHICLE_CAR:
        return new CAutomobileSA( (CAutomobileSAInterface*)CreateVehicle( modelID ) );
    case VEHICLE_AUTOMOBILETRAILER:
        return AddTrailer( modelID );
    case VEHICLE_QUADBIKE:
        return AddQuadBike( modelID );
    case VEHICLE_MONSTERTRUCK:
        return AddMonsterTruck( modelID );
    }

    return NULL;
}

CBoatSA* CPoolsSA::AddBoat( unsigned short modelID )
{
    DEBUG_TRACE("CBoat* CPoolsSA::AddBoat( unsigned short modelID )");

    if ( modelID > MAX_MODELS-1 )
        return NULL;

    // We do not wanna add if the pool is full
    if ( (*ppVehiclePool)->Full() )
        return NULL;

    CVehicleModelInfoSAInterface *info = (CVehicleModelInfoSAInterface*)ppModelInfo[modelID];

    if ( !info || info->GetModelType() != MODEL_VEHICLE || info->m_vehicleType != VEHICLE_BOAT )
        return NULL;

    return new CBoatSA( (CBoatSAInterface*)CreateVehicle( modelID ) );
}

CVehicleSA* CPoolsSA::AddVehicle( unsigned short modelID )
{
    DEBUG_TRACE("CVehicle* CPoolsSA::AddVehicle( unsigned short modelID )");

    if ( modelID > MAX_MODELS-1 )
        return NULL;

    CVehicleModelInfoSAInterface *info = (CVehicleModelInfoSAInterface*)ppModelInfo[modelID];

    if ( !info || info->GetModelType() != MODEL_VEHICLE )
        return NULL;

    switch( info->m_vehicleType )
    {
    case VEHICLE_TRAIN:
        return AddTrain( modelID, CVector( 0, 0, 0 ), true );
    case VEHICLE_BIKE:
    case VEHICLE_BICYCLE:
        return AddBike( modelID );
    case VEHICLE_PLANE:
    case VEHICLE_HELI:
    case VEHICLE_CAR:
    case VEHICLE_AUTOMOBILETRAILER:
    case VEHICLE_QUADBIKE:
    case VEHICLE_MONSTERTRUCK:
        return AddAutomobile( modelID );
    case VEHICLE_BOAT:
        return AddBoat( modelID );
    }

    return NULL;
}

// Safe retrieval of our instance
CVehicleSA* CPoolsSA::GetVehicle( void *entity ) const
{
    unsigned int id = (*ppVehiclePool)->GetIndex( (CVehicleSAInterface*)entity );
    
    if ( id > MAX_VEHICLES-1 )
        return NULL;

    return mtaVehicles[id];
}

CVehicleSA* CPoolsSA::GetVehicleFromRef( unsigned int index ) const
{
    DEBUG_TRACE("CVehicle* CPoolsSA::GetVehicleFromRef( unsigned int index ) const");

    if ( index > MAX_VEHICLES-1 )
        return NULL;

    return mtaVehicles[index];
}

void CPoolsSA::DeleteAllVehicles()
{
    DEBUG_TRACE("void CPoolsSA::DeleteAllVehicles()");

    (*ppVehiclePool)->Clear();
}

//////////////////////////////////////////////////////////////////////////////////////////
//                                     OBJECTS POOL                                     //
//////////////////////////////////////////////////////////////////////////////////////////
CObjectSA *mtaObjects[MAX_OBJECTS];

static inline CObjectSAInterface* CreateObject( unsigned short model )
{
    DWORD CObjectCreate = FUNC_CObject_Create;  
    CObjectSAInterface *obj;
    _asm
    {
        push    1
        movzx   eax,model
        push    eax
        call    CObjectCreate
        add     esp, 8
        mov     obj, eax
    }

    pGame->GetWorld()->Add( obj );
    return obj;
}

CObjectSA* CPoolsSA::AddObject( unsigned short modelId )
{
    DEBUG_TRACE("CObject* CPoolsSA::AddObject( unsigned short modelId )");

    if ( (*ppObjectPool)->Full() )
        return NULL;

    CBaseModelInfoSAInterface *info = ppModelInfo[modelId];

    return new CObjectSA( CreateObject( modelId ) );
}

CObjectSA* CPoolsSA::GetObject( void *entity ) const
{
    unsigned int id = (*ppObjectPool)->GetIndex( (CObjectSAInterface*)entity );

    if ( id > MAX_OBJECTS-1 )
        return NULL;

    return mtaObjects[ id ];
}

CObjectSA* CPoolsSA::GetObjectFromRef( unsigned int index ) const
{
    DEBUG_TRACE("CObject* CPoolsSA::GetObjectFromRef( unsigned int index ) const");

    if ( index > MAX_OBJECTS-1 )
        return NULL;

    return mtaObjects[index];
}

void CPoolsSA::DeleteAllObjects()
{
    DEBUG_TRACE("void CPoolsSA::DeleteAllObjects()");

    (*ppObjectPool)->Clear();
}

//////////////////////////////////////////////////////////////////////////////////////////
//                                       PEDS POOL                                      //
//////////////////////////////////////////////////////////////////////////////////////////
CPedSA *mtaPeds[MAX_PEDS];

inline static CPlayerPedSAInterface* CreatePlayerPed()
{
    // based on CPlayerPed::SetupPlayerPed (R*)
    DWORD CPlayerPedConstructor = FUNC_CPlayerPedConstructor;
    CPlayerPedSAInterface *player = (CPlayerPedSAInterface*)(*ppPedPool)->Allocate();

    _asm
    {
        mov     ecx, player
        push    0 // set to 0 and they'll behave like AI peds
        push    1
        call    CPlayerPedConstructor
    }

    return player;
}

CPedSA* CPoolsSA::AddPed( unsigned short modelId )
{
    DEBUG_TRACE("CPed* CPoolsSA::AddPed( unsigned short modelId )");

    if ( (*ppPedPool)->Full() )
        return NULL;

    if ( modelId > MAX_MODELS-1 )
        return NULL;

    CBaseModelInfoSAInterface *info = ppModelInfo[modelId];

    if ( !info || info->GetModelType() != MODEL_PED )
        return NULL;

    return new CPlayerPedSA( CreatePlayerPed(), modelId, false );
}

inline static CCivilianPedSAInterface* CreateCivilianPed()
{
    // based on CCivilianPed::SetupPlayerPed (R*)
    DWORD CCivilianPedConstructor = FUNC_CCivilianPedConstructor;
    CCivilianPedSAInterface *ped = (CCivilianPedSAInterface*)(*ppPedPool)->Allocate();

    _asm
    {
        mov     ecx,ped
        push    0 // set to 0 and they'll behave like AI peds
        push    1 // ped type
        call    CCivilianPedConstructor
    }

    //pGame->GetWorld()->Add( ped );
    return ped;
}

CPedSA* CPoolsSA::AddCivilianPed( unsigned short modelID )
{
    DEBUG_TRACE("CPed* CPoolsSA::AddCivilianPed( unsigned short modelID )");

    if ( (*ppPedPool)->Full() )
        return NULL;

    return new CCivilianPedSA( CreateCivilianPed(), modelID );
}

CPedSA* CPoolsSA::AddCivilianPed( void *entity )
{
    DEBUG_TRACE("CPed* CPoolsSA::AddCivilianPed( void *entity )");

    return new CCivilianPedSA( (CCivilianPedSAInterface*)entity, ((CCivilianPedSAInterface*)entity)->m_model );
}

CPedSA* CPoolsSA::GetPed( void *entity ) const
{
    unsigned int id = (*ppPedPool)->GetIndex( (CPedSAInterface*)entity );

    if ( id >= MAX_PEDS )
        return NULL;

    return mtaPeds[ id ];
}

CPedSA* CPoolsSA::GetPedFromRef( unsigned int index ) const
{
    DEBUG_TRACE("CPed* CPoolsSA::GetPedFromRef( unsigned int index ) const");

    if ( index > MAX_PEDS )
        return NULL;

    return mtaPeds[index];
}

void CPoolsSA::DeleteAllPeds()
{
    DEBUG_TRACE("void CPoolsSA::DeleteAllPeds()");

    (*ppPedPool)->Clear();
}

// This function is insecure, because it accesses the entity prior to validation
// Has to stay this way for optimization purposes; usage only in 100% secure scenarios!
CEntitySA* CPoolsSA::GetEntity( void *entity ) const
{
    if ( !entity )
        return NULL;

    switch( ((CEntitySAInterface*)entity)->m_type )
    {
    case ENTITY_TYPE_PED:       return GetPed( entity );
    case ENTITY_TYPE_VEHICLE:   return GetVehicle( entity );
    case ENTITY_TYPE_OBJECT:    return GetObject( entity );
    }

    return NULL;
}

CBuildingSA* CPoolsSA::AddBuilding( unsigned short modelID )
{
    DEBUG_TRACE("CBuilding* CPoolsSA::AddBuilding( unsigned short modelID )");

    if ( (*ppBuildingPool)->Full() )
        return NULL;
    
    return new CBuildingSA( modelID );
}

char szOutput[1024];

void CPoolsSA::DumpPoolsStatus() const
{
    char*  poolNames[] = {"Buildings", "Peds", "Objects", "Dummies", "Vehicles", "ColModels", 
        "Tasks", "Events", "TaskAllocators", "PedIntelligences", "PedAttractors", 
        "EntryInfoNodes", "NodeRoutes", "PatrolRoutes", "PointRoutes", 
        "PointerNodeDoubleLinks", "PointerNodeSingleLinks" };

    int iPosition = 0;
    char percent = '%';

    iPosition += snprintf ( szOutput, 1024, "-----------------\n" );

    for ( int i = 0; i < MAX_POOLS; i++ )
    {
        int usedSpaces = GetNumberOfUsedSpaces ( (ePools)i );
        iPosition += snprintf( szOutput + iPosition, 1024 - iPosition, 
            "%s: %d (%d) (%.2f%c)\n", 
            poolNames[i], 
            usedSpaces, 
            GetPoolCapacity( (ePools)i ), 
            ((float)usedSpaces / (float)GetPoolCapacity( (ePools)i ) * 100), 
            percent );
    }

#ifdef MTA_DEBUG
    OutputDebugString ( szOutput );
#endif
}

unsigned int CPoolsSA::GetPoolDefaultCapacity( ePools pool ) const
{
    switch( pool )
    {
    case BUILDING_POOL:             return 13000;
    case PED_POOL:                  return 140;
    case OBJECT_POOL:               return 350;          // Modded to 700   @ CGameSA.cpp
    case DUMMY_POOL:                return 2500;
    case VEHICLE_POOL:              return 110;
    case COL_MODEL_POOL:            return 10150;
    case TASK_POOL:                 return 500;          // Modded to 5000   @ CGameSA.cpp
    case EVENT_POOL:                return 200;          // Modded to 9001   @ CMultiplayerSA.cpp
    case TASK_ALLOCATOR_POOL:       return 16;
    case PED_INTELLIGENCE_POOL:     return 140;
    case PED_ATTRACTOR_POOL:        return 64;
    case ENTRY_INFO_NODE_POOL:      return 500;          // Modded to 4096   @ CMultiplayerSA.cpp
    case NODE_ROUTE_POOL:           return 64;
    case PATROL_ROUTE_POOL:         return 32;
    case POINT_ROUTE_POOL:          return 64;
    case POINTER_DOUBLE_LINK_POOL:  return 3200;         // Modded to 4000   @ CMultiplayerSA.cpp
    case POINTER_SINGLE_LINK_POOL:  return 70000;
    case ENV_MAP_MATERIAL_POOL:     return 4096;
    case ENV_MAP_ATOMIC_POOL:       return 1024;
    case SPEC_MAP_MATERIAL_POOL:    return 4096;
    }
    return 0;
}

unsigned int CPoolsSA::GetNumberOfUsedSpaces( ePools pool ) const
{
    switch( pool )
    {
    case BUILDING_POOL:             return (*ppBuildingPool)->GetCount();
    case PED_POOL:                  return (*ppPedPool)->GetCount();
    case OBJECT_POOL:               return (*ppObjectPool)->GetCount();
    case DUMMY_POOL:                return (*ppDummyPool)->GetCount();
    case VEHICLE_POOL:              return (*ppVehiclePool)->GetCount();
    case COL_MODEL_POOL:            return (*ppColModelPool)->GetCount();
    case TASK_POOL:                 return (*ppTaskPool)->GetCount();
    case EVENT_POOL:                return (*ppEventPool)->GetCount();
    case TASK_ALLOCATOR_POOL:       return (*ppTaskAllocatorPool)->GetCount();
    case PED_INTELLIGENCE_POOL:     return (*ppPedIntelligencePool)->GetCount();
    case PED_ATTRACTOR_POOL:        return (*ppPedAttractorPool)->GetCount();
    case ENTRY_INFO_NODE_POOL:      return (*ppEntryInfoPool)->GetCount();
    case NODE_ROUTE_POOL:           return (*ppNodeRoutePool)->GetCount();
    case PATROL_ROUTE_POOL:         return (*ppPatrolRoutePool)->GetCount();
    case POINT_ROUTE_POOL:          return (*ppPointRoutePool)->GetCount();
    case POINTER_DOUBLE_LINK_POOL:  return (*ppPtrNodeDoublePool)->GetCount();
    case POINTER_SINGLE_LINK_POOL:  return (*ppPtrNodeSinglePool)->GetCount();
    case ENV_MAP_MATERIAL_POOL:     return (*ppEnvMapMaterialPool)->GetCount();
    case ENV_MAP_ATOMIC_POOL:       return (*ppEnvMapAtomicPool)->GetCount();
    case SPEC_MAP_MATERIAL_POOL:    return (*ppSpecMapMaterialPool)->GetCount();
    }

    return 0;
}

unsigned int CPoolsSA::GetPoolCapacity( ePools pool ) const
{
    switch( pool )
    {
    case BUILDING_POOL:                 return (*ppBuildingPool)->m_max;
    case PED_POOL:                      return (*ppPedPool)->m_max;
    case OBJECT_POOL:                   return (*ppObjectPool)->m_max;
    case DUMMY_POOL:                    return (*ppDummyPool)->m_max;
    case VEHICLE_POOL:                  return (*ppVehiclePool)->m_max;
    case COL_MODEL_POOL:                return (*ppColModelPool)->m_max;
    case TASK_POOL:                     return (*ppTaskPool)->m_max;
    case EVENT_POOL:                    return (*ppEventPool)->m_max;
    case TASK_ALLOCATOR_POOL:           return (*ppTaskAllocatorPool)->m_max;
    case PED_INTELLIGENCE_POOL:         return (*ppPedIntelligencePool)->m_max;
    case PED_ATTRACTOR_POOL:            return (*ppPedAttractorPool)->m_max;
    case ENTRY_INFO_NODE_POOL:          return (*ppEntryInfoPool)->m_max;
    case NODE_ROUTE_POOL:               return (*ppNodeRoutePool)->m_max;
    case PATROL_ROUTE_POOL:             return (*ppPatrolRoutePool)->m_max;
    case POINT_ROUTE_POOL:              return (*ppPointRoutePool)->m_max;
    case POINTER_DOUBLE_LINK_POOL:      return (*ppPtrNodeDoublePool)->m_max;
    case POINTER_SINGLE_LINK_POOL:      return (*ppPtrNodeSinglePool)->m_max;
    case ENV_MAP_MATERIAL_POOL:         return (*ppEnvMapMaterialPool)->GetMax();
    case ENV_MAP_ATOMIC_POOL:           return (*ppEnvMapAtomicPool)->GetMax();
    case SPEC_MAP_MATERIAL_POOL:        return (*ppSpecMapMaterialPool)->GetMax();
    }

    return 0;
}