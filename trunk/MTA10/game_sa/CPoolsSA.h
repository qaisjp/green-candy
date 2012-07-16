/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.0
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        game_sa/CPoolsSA.h
*  PURPOSE:     Header file for game entity pools class
*  DEVELOPERS:  Ed Lyons <eai@opencoding.net>
*               Christian Myhre Lundheim <>
*               Cecill Etheredge <ijsf@gmx.net>
*               Jax <>
*               Alberto Alonso <rydencillo@gmail.com>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/
#ifndef __CGAMESA_POOLS
#define __CGAMESA_POOLS

#include <game/CPools.h>
#include <google/dense_hash_map>

template < class type, int max >
class CPool
{
public:
    CPool()
    {
        m_pool = (type*)malloc( sizeof(type) * max );
        m_flags = (unsigned char*)malloc( max );

        m_poolActive = true;

        // Reset all flags
        memset(m_flags, 0x80, max);

        m_max = max;
        m_active = 0;
    }

    void    Clear()
    {
        unsigned int n;

        for (n=0; n<m_max; n++)
        {
            if ( !(m_flags[n] & 0x80) )
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

    type*   Allocate()
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
            return &m_pool[n];
        }

        return NULL;
    }

    type*   Get( unsigned int id )
    {
        return (id >= m_max) && !(m_flags[id] & 0x80) ? &m_pool[id] ? NULL;
    }

    unsigned int    GetIndex( type *entity )
    {
        return (unsigned int)(entity - m_pool) / sizeof(type);
    }

    void    Free( unsigned int id )
    {
        if ( id >= m_max )
            return;

        if ( m_flags[id] & 0x80 )
            return;

        m_flags[id] |= 0x80;
        m_active--;
    }

    void    Free( type *entity )
    {
        Free(GetIndex(entity));
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

typedef CPool <CVehicleSeatPlacementSAInterface, 500> CVehicleSeatPlacementPool;
typedef CPool <CColModelSAInterface, 20000> CColModelPool;

typedef CPool <CPtrNodeSingleSA, 100000> CPtrNodeSinglePool;
typedef CPool <CPtrNodeDoubleSA, 5000> CPtrNodeDoublePool;
typedef CPool <CEntryInfoSA, 500> CEntryInfoPool;

typedef CPool <CTxdInstanceSA, MAX_TXD> CTxdPool;

typedef CPool <CVehicleSAInterface, MAX_VEHICLES> CVehiclePool;
typedef CPool <CPedSAInterface, MAX_PEDS> CPedPool;
typedef CPool <CObjectSAInterface, MAX_OBJECTS> CObjectPool;

typedef CPool <CBuildingSAInterface, MAX_BUILDINGS> CBuildingPool;
typedef CPool <CDummySAInterface, 4000> CDummyPool;

typedef CPool <CTaskSAInterface, 9001> CTaskPool;
typedef CPool <CEventSAInterface, 1337> CEventPool;
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

class CPoolsSA : public CPools
{
public:
                            CPoolsSA();
                            ~CPoolsSA();

    // Vehicles pool
    CVehicle*               AddVehicle( eVehicleTypes eVehicleType );
    CVehicle*               GetVehicle( void *entity );
    unsigned int            GetVehicleRef( CVehicle *veh );
    CVehicle*               GetVehicleFromRef( unsigned int index );
    void                    DeleteAllVehicles();

    // Objects pool
    CObject*                AddObject( unsigned short modelID );
    CObject*                GetObject( void *entity );
    unsigned int            GetObjectRef( CObject *obj );
    CObject*                GetObjectFromRef( unsigned int index );
    void                    DeleteAllObjects();

    // Peds pool
    CPed*                   AddPed( ePedModel ePedType );
    CPed*                   AddCivilianPed( DWORD* pGameInterface );
    CPed*                   GetPed( void *entity );
    unsigned int            GetPedRef( CPed* pPed );
    CPed*                   GetPedFromRef( unsigned int index );
    void                    DeleteAllPeds();

    CEntity*                GetEntity( void *entity );

    // Others
    CBuilding*              AddBuilding( unsigned short modelID );
    void                    DeleteAllBuildings();
    CVehicle*               AddTrain( CVector* vecPosition, DWORD dwModels[], int iSize, bool bDirection );

    int                     GetNumberOfUsedSpaces( ePools pools );
    void                    DumpPoolsStatus();

    unsigned int            GetPoolDefaultCapacity( ePools pool );
    unsigned int            GetPoolCapacity( ePools pool );

private:
    typedef google::dense_hash_map <CObjectSAInterface*, CObjectSA*> gObjectMap;
    typedef google::dense_hash_map <CPedSAInterface*, CPedSA*> gPedMap;

    gObjectMap              m_objectMap;
    gPedMap                 m_pedMap;

    bool                    m_getVehicleEnabled;
};


#define FUNC_GetVehicle                     0x54fff0
#define FUNC_GetVehicleRef                  0x54ffc0
//#define FUNC_GetVehicleCount              0x429510
#define FUNC_GetPed                         0x54ff90
#define FUNC_GetPedRef                      0x54ff60
//#define FUNC_GetPedCount                  0x4A7440
#define FUNC_GetObject                      0x550050
#define FUNC_GetObjectRef                   0x550020
//#define FUNC_GetObjectCount                   0x4A74D0

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
