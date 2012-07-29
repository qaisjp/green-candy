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

#include "CBuilding.h"
#include "CObject.h"
#include "CPed.h"

enum ePools
{
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
    virtual CBicycle*               AddBicycle( unsigned short modelId ) = 0;
    virtual CBike*                  AddBike( unsigned short modelId ) = 0;
    virtual CHeli*                  AddHeli( unsigned short modelId ) = 0;
    virtual CPlane*                 AddPlane( unsigned short modelId ) = 0;
    virtual CTrain*                 AddTrain( unsigned short modelId, const CVector& pos, bool direction ) = 0;
    virtual CAutomobileTrailer*     AddTrailer( unsigned short modelId ) = 0;
    virtual CQuadBike*              AddQuadBike( unsigned short modelId ) = 0;
    virtual CMonsterTruck*          AddMonsterTruck( unsigned short modelId ) = 0;
    virtual CAutomobile*            AddAutomobile( unsigned short modelId ) = 0;
    virtual CBoat*                  AddBoat( unsigned short modelId ) = 0;
    virtual CVehicle*               AddVehicle( unsigned short modelId ) = 0;
    virtual CVehicle*               GetVehicle( void *entity ) const = 0;
    virtual CVehicle*               GetVehicleFromRef( unsigned int index ) const = 0;
    virtual void                    DeleteAllVehicles() = 0;

     // Objects pool
    virtual CObject*                AddObject( unsigned short modelID ) = 0;
    virtual CObject*                GetObject( void *entity ) const = 0;
    virtual CObject*                GetObjectFromRef( unsigned int index ) const = 0;
    virtual void                    DeleteAllObjects() = 0;

    // Peds pool
    virtual CPed*                   AddPed( unsigned short modelID ) = 0;
    virtual CPed*                   GetPed( void *entity ) const = 0;
    virtual CPed*                   GetPedFromRef( unsigned int index ) const = 0;
    virtual void                    DeleteAllPeds() = 0;

    // Others
    virtual CBuilding*              AddBuilding( unsigned short model ) = 0;
    virtual CEntity*                GetEntity( void *entity ) const = 0;

    virtual unsigned int            GetNumberOfUsedSpaces( ePools pool ) const = 0;
    virtual void                    DumpPoolsStatus() const = 0;

    virtual unsigned int            GetPoolDefaultCapacity( ePools pool ) const = 0;
    virtual unsigned int            GetPoolCapacity( ePools pool ) const = 0;
};

#endif
