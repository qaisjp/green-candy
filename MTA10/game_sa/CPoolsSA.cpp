/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.0
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

#define FUNC_InitGamePools

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

CPoolsSA::CPoolsSA()
{
    DEBUG_TRACE("CPoolsSA::CPoolsSA()");

    // Do not let GTA:SA init pools!
    *(unsigned char*)FUNC_InitGamePools = 0xC3;

    *ppPtrNodeSinglePool = new CPtrNodeSinglePool();
    *ppPtrNodeDoublePool = new CPtrNodeDoublePool();
    *ppEntryInfoPool = new CEntryInfoPool();

    *ppVehiclePool = new CVehiclePool();
    *ppPedPool = new CObjectPool();
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

    m_getVehicleEnabled = true;
}

CPoolsSA::~CPoolsSA()
{
    delete *ppPtrNodeSinglePool;
    delete *ppPtrNodeDoublePool;
    delete *ppEntryInfoPool;

    delete *ppVehiclePool;
    delete *ppPedPool;
    delete *ppBuildingPool;
    delete *ppObjectPool;

    delete *ppDummyPool;
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

void CPoolsSA::DeleteAllBuildings( )
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
CVehicle* CPoolsSA::AddVehicle ( eVehicleTypes eVehicleType )
{
    DEBUG_TRACE("CVehicle* CPoolsSA::AddVehicle ( eVehicleTypes eVehicleType )");
    
    // We do not wanna add when the pool is full
    if ((*ppVehiclePool)->Full())
        return NULL;

    return new CVehicleSA( eVehicleType );
}

CVehicle* CPoolSA::GetVehicle( void *entity )
{
    if ( !m_getVehicleEnabled )
        return NULL;

    return ((CVehicleSAInterface*)entity)->m_vehicle;
}

unsigned int CPoolsSA::GetVehicleRef ( CVehicle* pVehicle )
{
    DEBUG_TRACE("DWORD CPoolsSA::GetVehicleRef ( CVehicle* pVehicle )");

    return (*ppVehiclePool)->GetIndex( ((CVehicleSA*)pVehicle)->GetVehicleInterface() );
}

CVehicle* CPoolsSA::GetVehicleFromRef ( unsigned int index )
{
    DEBUG_TRACE("CVehicle* CPoolsSA::GetVehicleFromRef ( DWORD dwGameRef )");

    CVehicleSAInterface *veh = (*ppVehiclePool)->Get( index );

    if ( !veh )
        return NULL;

    return veh->m_pVehicle;
}

void CPoolsSA::DeleteAllVehicles ( )
{
    DEBUG_TRACE("void CPoolsSA::DeleteAllVehicles ( )");

    (*ppVehiclePool)->Clear();
}

//////////////////////////////////////////////////////////////////////////////////////////
//                                     OBJECTS POOL                                     //
//////////////////////////////////////////////////////////////////////////////////////////
CObject* CPoolsSA::AddObject ( DWORD dwModelID )
{
    CObjectSA *obj;

    DEBUG_TRACE("CObject * CPoolsSA::AddObject ( DWORD dwModelID )");

    if ((*ppObjectPool)->Full())
        return NULL;

    obj = new CObjectSA( dwModelID );

    // We have to create a hash map entry
    m_objectMap.insert( gObjectMap::value_type( obj->m_pInterface, obj ) );

    return obj;
}

CObject* CPoolSA::GetObject( void *entity )
{
    gObjectMap::iterator iter = m_objectMap.find( (CObjectSAInterface*)entity );

    if ( iter == m_objectMap.end() )
        return NULL;

    return (*iter).second;
}

unsigned int CPoolsSA::GetObjectRef ( CObject* pObject )
{
    DEBUG_TRACE("DWORD CPoolsSA::GetObjectRef ( CObject* pObject )");

    return (*ppObjectPool)->GetIndex( ((CObjectSA*)pObject)->m_pInterface );
}

CObject* CPoolsSA::GetObjectFromRef ( unsigned int index )
{
    DEBUG_TRACE("CObject* CPoolsSA::GetObjectFromRef ( DWORD dwGameRef )");

    CObjectSAInterface *obj = (*ppObjectPool)->Get( index );
    gObjectMap::iterator iter;

    if ( !obj )
        return NULL;

    iter = m_objectMap.find( obj );

    if ( iter == m_objectMap.end() )
        return NULL;

    return (*iter).second;
}

void CPoolsSA::DeleteAllObjects ( )
{
    DEBUG_TRACE("void CPoolsSA::DeleteAllObjects ( )");

    (*ppObjectPool)->Clear();

    m_objectMap.clear();
}

//////////////////////////////////////////////////////////////////////////////////////////
//                                       PEDS POOL                                      //
//////////////////////////////////////////////////////////////////////////////////////////
CPed* CPoolsSA::AddPed ( ePedModel ePedType )
{
    DEBUG_TRACE("CPed* CPoolsSA::AddPed ( ePedModel ePedType )");

    if ((*ppPedPool)->Full())
        return NULL;

    return new CPedSA( ePedType );
}

CPed* CPoolsSA::AddCivilianPed ( DWORD* pGameInterface )
{
    DEBUG_TRACE("CPed* CPoolsSA::AddCivilianPed ( DWORD* pGameInterface )");

    CPedSA *ped = new CCivilianPedSA( (CPedSAInterface*)pGameInterface );

    m_pedMap.insert( gPedMap::value_type( (CPedSAInterface*)pGameInterface, ped ) );
    return ped;
}

CPed* CPoolSA::GetPed( void *entity )
{
    gPedMap::iterator iter = m_pedMap.find( (CPedSAInterface*)entity );

    if ( iter == m_pedMap.end() )
        return NULL;

    return (*iter).second;
}

unsigned int CPoolsSA::GetPedRef ( CPed* pPed )
{
    DEBUG_TRACE("DWORD CPoolsSA::GetPedRef ( CPed* pPed )");

    return (*ppPedPool)->GetIndex( ((CPedSA*)pPed)->m_pInterface );
}

CPed* CPoolsSA::GetPedFromRef ( unsigned int index )
{
    DEBUG_TRACE("CPed* CPoolsSA::GetPedFromRef ( DWORD dwGameRef )");

    CPedSAInterface *ped = (*ppPedPool)->Get( index );
    gPedMap::iterator iter;

    if ( !ped )
        return NULL;

    iter = m_pedMap.find( ped );

    if ( iter == m_pedMap.end() )
        return NULL;

    return (*iter).second;
}

void CPoolsSA::DeleteAllPeds ( )
{
    DEBUG_TRACE("void CPoolsSA::DeleteAllPeds ( )");

    (*ppPedPool)->Clear();

    m_pedMap.clear();
}

CEntity* CPoolsSA::GetEntity( void *entity )
{
    switch ( ((CEntitySAInterface*)entity)->m_type )
    {
    case ENTITY_TYPE_PED:
        return GetPed( entity );
    case ENTITY_TYPE_VEHICLE:
        return GetVehicle( entity );
    case ENTITY_TYPE_OBJECT:
        return GetObject( entity );
    }

    return NULL;
}

CBuilding* CPoolsSA::AddBuilding ( DWORD dwModelID )
{
    DEBUG_TRACE("CBuilding * CPoolsSA::AddBuilding ( DWORD dwModelID )");
    
    return new CBuildingSA( dwModelID );
}s

CVehicle* CPoolsSA::AddTrain ( CVector * vecPosition, DWORD dwModels[], int iSize, bool bDirection )
{
    DEBUG_TRACE("CVehicle* CPoolsSA::AddTrain ( CVector * vecPosition, DWORD dwModels[], int iSize, bool bDirection )");

    // clean the existing array
    MemSetFast ( (void *)VAR_TrainModelArray, 0, 32 * sizeof(DWORD) );

    // now load the models we're going to use and add them to the array
    for ( int i = 0; i < iSize; i++ )
    {
        if ( dwModels[i] == 449 || dwModels[i] == 537 || 
            dwModels[i] == 538 || dwModels[i] == 569 || 
            dwModels[i] == 590 )
        {
            MemPutFast < DWORD > ( VAR_TrainModelArray + i * 4, dwModels[i] );
        }
    }

    CVehicleSAInterface * trainBeginning;
    CVehicleSAInterface * trainEnd;

    float fX = vecPosition->fX;
    float fY = vecPosition->fY;
    float fZ = vecPosition->fZ;

    // Disable GetVehicle because CreateMissionTrain calls it before our CVehicleSA instance is inited
    m_getVehicleEnabled = false;

    DWORD dwFunc = FUNC_CTrain_CreateMissionTrain;
    _asm
    {
        push    0 // place as close to point as possible (rather than at node)? (maybe) (actually seems to have an effect on the speed, so changed from 1 to 0)
        push    0 // start finding closest from here 
        push    -1 // node to start at (-1 for closest node)
        lea     ecx, trainEnd
        push    ecx // end of train
        lea     ecx, trainBegining 
        push    ecx // begining of train
        push    0 // train type (always use 0 as thats where we're writing to)
        push    bDirection // direction 
        push    fZ // z
        push    fY // y
        push    fX // x
        call    dwFunc
        add     esp, 0x28
    }

    assert( trainBeginning );
    assert( !(*ppVehiclePool)->Full() );

    // Enable GetVehicle
    m_getVehicleEnabled = true;

    CVehicleSA *trainHead = new CVehicleSA ( trainBegining );
    CVehicleSA *carriage = trainHead;
    
    while ( carriage )
    {
        assert( !(*ppVehiclePool)->Full() );

        CVehicleSAInterface* vehCarriage = carriage->GetNextCarriageInTrain ();

        if ( !vehCarriage )
            break;

        carriage = new CVehicleSA ( vehCarriage );
    }

    // Stops the train from moving at ludacrist speeds right after creation
    // due to some glitch in the node finding in CreateMissionTrain
    CVector vec(0, 0, 0);
    trainHead->SetMoveSpeed ( &vec );

    return trainHead;
}

char szOutput[1024];

void CPoolsSA::DumpPoolsStatus ()
{
    char*  poolNames[] = {"Buildings", "Peds", "Objects", "Dummies", "Vehicles", "ColModels", 
        "Tasks", "Events", "TaskAllocators", "PedIntelligences", "PedAttractors", 
        "EntryInfoNodes", "NodeRoutes", "PatrolRoutes", "PointRoutes", 
        "PointerNodeDoubleLinks", "PointerNodeSingleLinks" };

    int poolSizes[] = {13000,140,350,2500,110,10150,500,200,16,140,64,500,64,32,64,3200,70000};
    int iPosition = 0;
    char percent = '%';

    iPosition += snprintf ( szOutput, 1024, "-----------------\n" );

    for ( int i = 0; i < MAX_POOLS; i++ )
    {
        int usedSpaces = GetNumberOfUsedSpaces ( (ePools)i );
        iPosition += snprintf ( szOutput + iPosition, 1024 - iPosition, 
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

int CPoolsSA::GetPoolDefaultCapacity ( ePools pool )
{
    switch ( pool )
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
    }
    return 0;
}

unsigned int CPoolsSA::GetNumberOfUsedSpaces ( ePools pool )
{
    switch( pool )
    {
    case BUILDING_POOL:
        return (*ppBuildingPool)->GetCount();
    case PED_POOL:
        return (*ppPedPool)->GetCount();
    case OBJECT_POOL:
        return (*ppObjectPool)->GetCount();
    case DUMMY_POOL:
        return (*ppDummyPool)->GetCount();
    case VEHICLE_POOL:
        return (*ppVehiclePool)->GetCount();
    case COL_MODEL_POOL:
        return (*ppColModelPool)->GetCount();
    case TASK_POOL:
        return (*ppTaskPool)->GetCount();
    case EVENT_POOL:
        return (*ppEventPool)->GetCount();
    case TASK_ALLOCATOR_POOL:
        return (*ppTaskAllocatorPool)->GetCount();
    case PED_INTELLIGENCE_POOL:
        return (*ppPedIntelligencePool)->GetCount();
    case PED_ATTRACTOR_POOL:
        return (*ppPedAttractorPool)->GetCount();
    case ENTRY_INFO_NODE_POOL:
        return (*ppEntryInfoPool)->GetCount();
    case NODE_ROUTE_POOL:
        return (*ppNodeRoutePool)->GetCount();
    case PATROL_ROUTE_POOL:
        return (*ppPatrolRoutePool)->GetCount();
    case POINT_ROUTE_POOL:
        return (*ppPointRoutePool)->GetCount();
    case POINTER_DOUBLE_LINK_POOL:
        return (*ppPtrNodeDoublePool)->GetCount();
    case POINTER_SINGLE_LINK_POOL:
        return (*ppPtrNodeSinglePool)->GetCount();
    }

    return 0;
}

unsigned int CPoolsSA::GetPoolCapacity( ePools pool )
{
    switch( pool )
    {
    case BUILDING_POOL:
        return (*ppBuildingPool)->m_max;
    case PED_POOL:
        return (*ppPedPool)->m_max;
    case OBJECT_POOL:
        return (*ppObjectPool)->m_max;
    case DUMMY_POOL:
        return (*ppDummyPool)->m_max;
    case VEHICLE_POOL:
        return (*ppVehiclePool)->m_max;
    case COL_MODEL_POOL:
        return (*ppColModelPool)->m_max;
    case TASK_POOL:
        return (*ppTaskPool)->m_max;
    case EVENT_POOL:
        return (*ppEventPool)->m_max;
    case TASK_ALLOCATOR_POOL:
        return (*ppTaskAllocatorPool)->m_max;
    case PED_INTELLIGENCE_POOL:
        return (*ppPedIntelligencePool)->m_max;
    case PED_ATTRACTOR_POOL:
        return (*ppPedAttractorPool)->m_max;
    case ENTRY_INFO_NODE_POOL:
        return (*ppEntryInfoPool)->m_max;
    case NODE_ROUTE_POOL:
        return (*ppNodeRoutePool)->m_max;
    case PATROL_ROUTE_POOL:
        return (*ppPatrolRoutePool)->m_max;
    case POINT_ROUTE_POOL:
        return (*ppPointRoutePool)->m_max;
    case POINTER_DOUBLE_LINK_POOL:
        return (*ppPtrNodeDoublePool)->m_max;
    case POINTER_SINGLE_LINK_POOL:
        return (*ppPtrNodeSinglePool)->m_max;
    }

    return 0;
}