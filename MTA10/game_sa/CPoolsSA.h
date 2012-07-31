/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        game_sa/CPoolsSA.h
*  PURPOSE:     Header file for game entity pools class
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
#ifndef __CGAMESA_POOLS
#define __CGAMESA_POOLS

#include <game/CPools.h>
#include <google/dense_hash_map>

#include "CModelInfoSA.h"
#include "CClumpModelInfoSA.h"
#include "CVehicleModelInfoSA.h"
#include "CColModelSA.h"
#include "CEntryInfoSA.h"
#include "CTextureSA.h"
#include "CTexDictionarySA.h"
#include "CTextureManagerSA.h"
#include "CEventSA.h"
#include "CBuildingSA.h"
#include "CDummySA.h"
#include "CRouteSA.h"
#include "CVehicleSA.h"
#include "CAutomobileSA.h"
#include "CHeliSA.h"
#include "CBoatSA.h"
#include "CBikeSA.h"
#include "CBicycleSA.h"
#include "CAutomobileTrailerSA.h"
#include "CTrainSA.h"
#include "CMonsterTruckSA.h"
#include "CQuadBikeSA.h"
#include "CPlaneSA.h"
#include "CPedSA.h"
#include "CPlayerPedSA.h"
#include "CObjectSA.h"
#include "CProjectileSA.h"
#include "CTaskAllocatorSA.h"

template <class type, int max, const size_t size = sizeof(type)>
class CPool
{
public:
    CPool()
    {
        m_pool = (type*)malloc( size * max );
        m_flags = (unsigned char*)malloc( max );

        m_poolActive = true;

        // Reset all flags
        memset(m_flags, 0x80, max);

        m_max = max;
        m_active = 0;
    }

    // This function expects the entities to free themselves from this;
    // so do not decrease the count here!
    void    Clear()
    {
        unsigned int n;

        for (n=0; n<m_max; n++)
        {
            if ( m_flags[n] & 0x80 )
                continue;

            delete &m_pool[n];
        }
    }

    ~CPool()
    {
        Clear();

        free(m_pool);
        free(m_flags);
    }

    inline type*    GetOffset( unsigned int id )
    {
        return (type*)( (unsigned int)m_pool + size * id );
    }

    inline type*    Allocate()
    {
        unsigned int n;

        if ( m_active == m_max )
            return NULL;

        for (n=0; n<m_max; n++)
        {
            if ( !(m_flags[n] & 0x80) )
                continue;

            m_flags[n] &= ~0x80;

            m_active++;
            return GetOffset( n );
        }

        return NULL;
    }

    inline type*    Get( unsigned int id )
    {
        return ( (id >= m_max) && !(m_flags[id] & 0x80) ) ? GetOffset( id ) : NULL;
    }

    unsigned int    GetIndex( type *entity )
    {
        return ((unsigned int)entity - (unsigned int)m_pool) / size;
    }

    bool            IsValid( type *entity )
    {
        return entity >= m_pool && GetIndex( entity ) < m_max;
    }

    void    Free( unsigned int id )
    {
        if ( id >= m_max )
            return;

        if ( m_flags[id] & 0x80 )
            return;

        m_flags[id] &= ~0x80;
        m_active--;
    }

    void    Free( type *entity )
    {
        Free( GetIndex( entity ) );
    }

    bool    Full()
    {
        return m_active == m_max;
    }

    unsigned int    GetCount()
    {
        return m_active;
    }

    unsigned int    GetMax()
    {
        return m_max;
    }

    type*           m_pool;
    unsigned char*  m_flags;
    unsigned int    m_max;
    unsigned int    m_active;
    bool            m_poolActive;
};

class CPedAttractorSA
{
public:
    BYTE            m_pad[196];
};

// Rockstar's inheritance trick; keep these chains updated!
#define MAX_VEHICLE_SIZE ( max(sizeof(CHeliSAInterface),max(sizeof(CTrainSAInterface),max(sizeof(CAutomobileSAInterface),max(sizeof(CBikeSAInterface),max(sizeof(CBicycleSAInterface),max(sizeof(CPlaneSAInterface),max(sizeof(CBoatSAInterface),max(sizeof(CAutomobileTrailerSAInterface),max(sizeof(CQuadBikeSAInterface),sizeof(CMonsterTruckSAInterface)))))))))) )

#define MAX_PED_SIZE ( max(sizeof(CPedSAInterface),max(sizeof(CPlayerPedSAInterface),sizeof(CCivilianPedSAInterface))) )

#define MAX_OBJECT_SIZE ( max(sizeof(CObjectSAInterface),sizeof(CProjectileSAInterface)) )

typedef CPool <CVehicleSeatPlacementSAInterface, 500> CVehicleSeatPlacementPool;
typedef CPool <CColModelSAInterface, 20000> CColModelPool;

typedef CPool <CPtrNodeSingleSA, 100000> CPtrNodeSinglePool;
typedef CPool <CPtrNodeDoubleSA, 5000> CPtrNodeDoublePool;
typedef CPool <CEntryInfoSA, 500> CEntryInfoPool;

typedef CPool <CTxdInstanceSA, MAX_TXD> CTxdPool;

typedef CPool <CVehicleSAInterface, MAX_VEHICLES, MAX_VEHICLE_SIZE> CVehiclePool;
typedef CPool <CPedSAInterface, MAX_PEDS, MAX_PED_SIZE> CPedPool;
typedef CPool <CObjectSAInterface, MAX_OBJECTS, MAX_OBJECT_SIZE> CObjectPool;

typedef CPool <CBuildingSAInterface, MAX_BUILDINGS> CBuildingPool;
typedef CPool <CDummySAInterface, 4000> CDummyPool;

typedef CPool <CTaskSAInterface, 9001> CTaskPool;
typedef CPool <CEventSAInterface, 5000> CEventPool;
typedef CPool <CPointRouteSA, 64> CPointRoutePool;
typedef CPool <CPatrolRouteSA, 32> CPatrolRoutePool;
typedef CPool <CNodeRouteSA, 64> CNodeRoutePool;
typedef CPool <CTaskAllocatorSA, 16> CTaskAllocatorPool;

typedef CPool <CPedIntelligenceSAInterface, MAX_PEDS> CPedIntelligencePool;
typedef CPool <CPedAttractorSA, 64> CPedAttractorPool;

// They have to be defined somewhere!
extern CVehicleSeatPlacementPool** ppVehicleSeatPlacementPool;
extern CColModelPool** ppColModelPool;

extern CPtrNodeSinglePool** ppPtrNodeSinglePool;
extern CPtrNodeDoublePool** ppPtrNodeDoublePool;
extern CEntryInfoPool** ppEntryInfoPool;

extern CTxdPool** ppTxdPool;

extern CVehiclePool** ppVehiclePool;
extern CPedPool** ppPedPool;
extern CObjectPool** ppObjectPool;

extern CBuildingPool** ppBuildingPool;
extern CDummyPool** ppDummyPool;

extern CTaskPool** ppTaskPool;
extern CEventPool** ppEventPool;
extern CPointRoutePool** ppPointRoutePool;
extern CPatrolRoutePool** ppPatrolRoutePool;
extern CNodeRoutePool** ppNodeRoutePool;
extern CTaskAllocatorPool** ppTaskAllocatorPool;

extern CPedIntelligencePool** ppPedIntelligencePool;
extern CPedAttractorPool** ppPedAttractorPool;

// MTA pools; lets use the trick ourselves, shall we? :P
// Do not forget to extend this chain once new interfaces are spotted!
#define MAX_MTA_VEHICLE_SIZE ( max(sizeof(CVehicleSA),max(sizeof(CTrainSA),max(sizeof(CPlaneSA),max(sizeof(CHeliSA),max(sizeof(CBikeSA),max(sizeof(CBicycleSA),max(sizeof(CAutomobileTrailerSA),max(sizeof(CBoatSA),max(sizeof(CQuadBikeSA),sizeof(CMonsterTruckSA)))))))))) )

#define MAX_MTA_PED_SIZE ( max(sizeof(CPedSA),max(sizeof(CPlayerPedSA),sizeof(CCivilianPedSA))) )

#define MAX_MTA_OBJECT_SIZE ( max(sizeof(CObjectSA),sizeof(CProjectileSA)) )

typedef CPool <CVehicleSA, MAX_VEHICLES, MAX_MTA_VEHICLE_SIZE> CMTAVehiclePool;
typedef CPool <CPedSA, MAX_PEDS, MAX_MTA_PED_SIZE> CMTAPedPool;
typedef CPool <CObjectSA, MAX_OBJECTS, MAX_MTA_OBJECT_SIZE> CMTAObjectPool;

extern CMTAVehiclePool *mtaVehiclePool;
extern CMTAPedPool *mtaPedPool;
extern CMTAObjectPool *mtaObjectPool;

class CPoolsSA : public CPools
{
public:
                            CPoolsSA();
                            ~CPoolsSA();

public:
    // Vehicles pool
    CBicycle*               AddBicycle( unsigned short modelId );
    CBike*                  AddBike( unsigned short modelId );
    CHeli*                  AddHeli( unsigned short modelId );
    CPlane*                 AddPlane( unsigned short modelId );
    CTrain*                 AddTrain( unsigned short modelId, const CVector& pos, bool direction );
    CAutomobileTrailer*     AddTrailer( unsigned short modelId );
    CQuadBike*              AddQuadBike( unsigned short modelId );
    CMonsterTruck*          AddMonsterTruck( unsigned short modelId );
    CAutomobile*            AddAutomobile( unsigned short modelId );
    CBoat*                  AddBoat( unsigned short modelId );
    CVehicle*               AddVehicle( unsigned short modelID );
    CVehicle*               GetVehicle( void *entity ) const;
    CVehicle*               GetVehicleFromRef( unsigned int index ) const;
    void                    DeleteAllVehicles();

    // Objects pool
    CObject*                AddObject( unsigned short modelID );
    CObject*                GetObject( void *entity ) const;
    CObject*                GetObjectFromRef( unsigned int index ) const;
    void                    DeleteAllObjects();

    // Peds pool
    CPed*                   AddPed( unsigned short modelID );
    CPed*                   AddCivilianPed( unsigned short modelID );
    CPed*                   AddCivilianPed( void *ped );
    CPed*                   GetPed( void *entity ) const;
    CPed*                   GetPedFromRef( unsigned int index ) const;
    void                    DeleteAllPeds();

    CEntity*                GetEntity( void *entity ) const;

    // Others
    CBuilding*              AddBuilding( unsigned short modelID );
    void                    DeleteAllBuildings();

    unsigned int            GetNumberOfUsedSpaces( ePools pools ) const;
    void                    DumpPoolsStatus() const;

    unsigned int            GetPoolDefaultCapacity( ePools pool ) const;
    unsigned int            GetPoolCapacity( ePools pool ) const;

private:
    bool                    m_getVehicleEnabled;
};

// Global linkage between internal interfaces and MTA
extern CVehicleSA *mtaVehicles[MAX_VEHICLES];
extern CPedSA *mtaPeds[MAX_PEDS];
extern CObjectSA *mtaObjects[MAX_OBJECTS];


#define FUNC_GetVehicle                     0x54fff0
#define FUNC_GetVehicleRef                  0x54ffc0
//#define FUNC_GetVehicleCount              0x429510
#define FUNC_GetPed                         0x54ff90
#define FUNC_GetPedRef                      0x54ff60
//#define FUNC_GetPedCount                  0x4A7440
#define FUNC_GetObject                      0x550050
#define FUNC_GetObjectRef                   0x550020
//#define FUNC_GetObjectCount               0x4A74D0

#define CLASS_CPool_VehicleModels           0xB4E680

#define CLASS_CPool_Vehicle                 0xB74494
#define CLASS_CPool_Ped                     0xB74490
#define CLASS_CPool_Object                  0xB7449C

#define CLASS_CBuildingPool                 0xb74498
#define CLASS_CPedPool                      0xb74490
#define CLASS_CObjectPool                   0xb7449c
#define CLASS_CDummyPool                    0xb744a0
#define CLASS_CVehiclePool                  0xb74494
#define CLASS_CColModelPool                 0xb744a4
#define CLASS_CTaskPool                     0xb744a8
#define CLASS_CEventPool                    0xb744ac
#define CLASS_CTaskAllocatorPool            0xb744bc
#define CLASS_CPedIntelligencePool          0xb744c0
#define CLASS_CPedAttractorPool             0xb744c4
#define CLASS_CEntryInfoNodePool            0xb7448c
#define CLASS_CNodeRoutePool                0xb744b8
#define CLASS_CPatrolRoutePool              0xb744b4
#define CLASS_CPointRoutePool               0xb744b0
#define CLASS_CPtrNodeDoubleLinkPool        0xB74488
#define CLASS_CPtrNodeSingleLinkPool        0xB74484


#define FUNC_CBuildingPool_GetNoOfUsedSpaces                0x550620
#define FUNC_CPedPool_GetNoOfUsedSpaces                     0x5504A0
#define FUNC_CObjectPool_GetNoOfUsedSpaces                  0x54F6B0
#define FUNC_CDummyPool_GetNoOfUsedSpaces                   0x5507A0
#define FUNC_CVehiclePool_GetNoOfUsedSpaces                 0x42CCF0
#define FUNC_CColModelPool_GetNoOfUsedSpaces                0x550870
#define FUNC_CTaskPool_GetNoOfUsedSpaces                    0x550940
#define FUNC_CEventPool_GetNoOfUsedSpaces                   0x550A10
#define FUNC_CTaskAllocatorPool_GetNoOfUsedSpaces           0x550d50
#define FUNC_CPedIntelligencePool_GetNoOfUsedSpaces         0x550E20
#define FUNC_CPedAttractorPool_GetNoOfUsedSpaces            0x550ef0
#define FUNC_CEntryInfoNodePool_GetNoOfUsedSpaces           0x5503d0
#define FUNC_CNodeRoutePool_GetNoOfUsedSpaces               0x550c80
#define FUNC_CPatrolRoutePool_GetNoOfUsedSpaces             0x550bb0
#define FUNC_CPointRoutePool_GetNoOfUsedSpaces              0x550ae0
#define FUNC_CPtrNodeSingleLinkPool_GetNoOfUsedSpaces       0x550230
#define FUNC_CPtrNodeDoubleLinkPool_GetNoOfUsedSpaces       0x550300

#define FUNC_CTrain_CreateMissionTrain                      0x6F7550
#define VAR_TrainModelArray                                 0x8D44F8

#endif
