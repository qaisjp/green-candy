/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.0
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        sdk/game/CPools.h
*  PURPOSE:     Game pool interface
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#ifndef __CGAME_POOLS
#define __CGAME_POOLS

#define MAX_VEHICLES        110
#define MAX_PEDS            140
#define MAX_OBJECTS         1000
#define MAX_BUILDINGS       13000

#include "Common.h"

#include "CAutomobile.h"
#include "CBoat.h"
#include "CBike.h"
#include "CBuilding.h"
#include "CObject.h"
#include "CPed.h"
#include "CVehicle.h"

enum ePools {
    BUILDING_POOL = 0,
    PED_POOL,
    OBJECT_POOL,
    DUMMY_POOL,
    VEHICLE_POOL,
    COL_MODEL_POOL,
    TASK_POOL,
    EVENT_POOL,
    TASK_ALLOCATOR_POOL,
    PED_INTELLIGENCE_POOL,
    PED_ATTRACTOR_POOL,
    ENTRY_INFO_NODE_POOL,
    NODE_ROUTE_POOL,
    PATROL_ROUTE_POOL,
    POINT_ROUTE_POOL,
    POINTER_DOUBLE_LINK_POOL,
    POINTER_SINGLE_LINK_POOL,
    MAX_POOLS
};

class CPools
{
public:
    // Vehicles pool
    virtual CVehicle*       AddVehicle              ( eVehicleTypes eVehicleType ) = 0;
    virtual unsigned int    GetVehicleRef           ( CVehicle* pVehicle ) = 0;
    virtual CVehicle*       GetVehicleFromRef       ( unsigned int index ) = 0;
    virtual unsigned int    GetVehicleCount         () = 0;

    // Objects pool
    virtual CObject*        AddObject               ( DWORD dwModelID ) = 0;
    virtual unsigned int    GetObjectRef            ( CObject* pObject ) = 0;
    virtual CObject*        GetObjectFromRef        ( unsigned int index ) = 0;
    virtual unsigned int    GetObjectCount          () = 0;

    // Peds pool
    virtual CPed*           AddPed                  ( ePedModel ePedType ) = 0;
    virtual unsigned int    GetPedRef               ( CPed* pPed ) = 0;
    virtual CPed*           GetPedFromRef           ( unsigned int index ) = 0;
    virtual unsigned int    GetPedCount             () = 0;

    // Others
    virtual CBuilding*      AddBuilding             ( DWORD dwModelID ) = 0;
    virtual CVehicle*       AddTrain                ( CVector* vecPosition, DWORD dwModels[], int iSize, bool iDirection ) = 0;
    virtual CEntity*        GetEntity               ( DWORD* pGameInterface ) = 0;

    virtual int             GetNumberOfUsedSpaces   ( ePools pool ) = 0;
    virtual void            DumpPoolsStatus         () = 0;

    virtual unsigned int    GetPoolDefaultCapacity  ( ePools pool ) = 0;
    virtual unsigned int    GetPoolCapacity         ( ePools pool ) = 0;
};

#endif
