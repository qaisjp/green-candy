/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.0
*               (Shared logic for modifications)
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        mods/shared_logic/CClientObjectManager.cpp
*  PURPOSE:     Physical object entity manager class
*  DEVELOPERS:  Ed Lyons <eai@opencoding.net>
*               Christian Myhre Lundheim <>
*               Jax <>
*               Alberto Alonso <rydencillo@gmail.com>
*               Kevin Whiteside <kevuwk@gmail.com>
*
*****************************************************************************/

#include "StdInc.h"

using std::list;
using std::vector;

static unsigned int g_uiValidObjectModels[] = {
    2, 0, 0, 0, 0, 0, 0, 0, 0, -4096, -1053185, 4194303, 16127, 0, 0, 0, 0, 0, 0, -128, 
    -515899393, -134217729, -1, -1, 33554431, -1, -1, -1, -14337, -1, -1, -129, -1, 1073741823, -1, -1, -1, -8387585, -1, -1, 
    -1, -1, -536870913, -2146435649, -1, -1, -1, -1, -1, -1, -1, -1, -1, -9, -1, -1, -1, -1, -1, -1, 
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -25, -1, -1, -1, -1, -1, -1, -16385, -1, 34080767, 
    2113536, -4825600, -5, -1, -3145729, -1, -16777217, -33, -1, -1, -1, -131, -201326593, -1, -1, -1, -1, -1, -1, -1, 
    1073741823, -133121, -1, -1, -65, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 
    -1, 1073741823, -64, -1, -1, -1, -1, -1, 134217727, 0, -64, -1, -1, -1, -1, -1, -1, -1, -536870913, -131069, 
    -1, -1, -1, -1, -1, -1, -1, -1, -16384, -1, -1, -1, -1, -1, -1, -1, -1, 524287, -128, -1, 
    -1, -1, -1, -1, -1, -1, -1, 134217727, -524288, -1, -1, -1, -1, -1, -1, 245759, -256, -1, -4097, -1, 
    -1, -1, -1, 1073741823, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -32768, -1, -1, -1, -1, -1, 
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -8388607, -1, -1, -1, -1, -1, -1, -1, -1, 
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -961, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 
    -1, -1, -1, -1, -1, -1, -1, -2096129, -1, -1, -1, -1, -1, -1, -1, -1, -897, -1, -1, -1, 
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 
    -1, -1, -1921, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1040187393, -1, -1, -1, -1, 
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -201326593, -1, -1, -1, 
    -1, -1, -1, -1, -1, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 
    -1, -1, -1, 268435455, -4194304, -1, -1, -241, -1, -1, -1, -1, -1, -1, 7, 0, 0, 0, 0, 0, 
    0, 0, 0, 0, 0, 0, 0, 0, 0, -32768, -1, -1, -1, -1, -671088643, -1, -1, -66060289, -13, -1793, 
    -32257, -245761, -1, -1, -513, 16252911, 0, 0, 0, -131072, 33554431, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -3585, -1, -1, -1, -1, -1, -1, -1, -1, -1, 
    -1, -1, -1, -1, 8388607, 0, 0, 0, 0, 0, 0, -256, -1, -1, -1, -1, -1, -1, -1, -1, 
    -1, -1, -1, -1, -1, -1, -268435449, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 
    -1, 92274687, -65536, -2097153, -1, 595591167, 1, 0, -16777216, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 
    -1, -1, 127, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
    0, 0, 0, 0, 0, 0, 
};

// Breakable model list
//  Generated by: 
//      1. Extracting model names from object.dat which have a 'Collision Damage Effect' greater than 0
//      2. Converting model names to IDs using objects.xml
static unsigned short g_usBreakableModelList[] = {
           625,    626,    627,    628,    629,    630,    631,    632,    633,    642, 
           643,    644,    646,    650,    716,    717,    737,    738,    792,    858, 
           881,    882,    883,    884,    885,    886,    887,    888,    889,    890, 
           891,    892,    893,    894,    895,    904,    905,    941,    955,    956, 
           959,    961,    990,    993,    996,   1209,   1211,   1213,   1219,   1220, 
          1221,   1223,   1224,   1225,   1226,   1227,   1228,   1229,   1230,   1231, 
          1232,   1235,   1238,   1244,   1251,   1255,   1257,   1262,   1264,   1265, 
          1270,   1280,   1281,   1282,   1283,   1284,   1285,   1286,   1287,   1288, 
          1289,   1290,   1291,   1293,   1294,   1297,   1300,   1302,   1315,   1328, 
          1329,   1330,   1338,   1350,   1351,   1352,   1370,   1373,   1374,   1375, 
          1407,   1408,   1409,   1410,   1411,   1412,   1413,   1414,   1415,   1417, 
          1418,   1419,   1420,   1421,   1422,   1423,   1424,   1425,   1426,   1428, 
          1429,   1431,   1432,   1433,   1436,   1437,   1438,   1440,   1441,   1443, 
          1444,   1445,   1446,   1447,   1448,   1449,   1450,   1451,   1452,   1456, 
          1457,   1458,   1459,   1460,   1461,   1462,   1463,   1464,   1465,   1466, 
          1467,   1468,   1469,   1470,   1471,   1472,   1473,   1474,   1475,   1476, 
          1477,   1478,   1479,   1480,   1481,   1482,   1483,   1514,   1517,   1520, 
          1534,   1543,   1544,   1545,   1551,   1553,   1554,   1558,   1564,   1568, 
          1582,   1583,   1584,   1588,   1589,   1590,   1591,   1592,   1645,   1646, 
          1647,   1654,   1664,   1666,   1667,   1668,   1669,   1670,   1672,   1676, 
          1684,   1686,   1775,   1776,   1949,   1950,   1951,   1960,   1961,   1962, 
          1975,   1976,   1977,   2647,   2663,   2682,   2683,   2885,   2886,   2887, 
          2900,   2918,   2920,   2925,   2932,   2933,   2942,   2943,   2945,   2947, 
          2958,   2959,   2966,   2968,   2971,   2977,   2987,   2988,   2989,   2991, 
          2994,   3006,   3018,   3019,   3020,   3021,   3022,   3023,   3024,   3029, 
          3032,   3036,   3058,   3059,   3067,   3083,   3091,   3221,   3260,   3261, 
          3262,   3263,   3264,   3265,   3267,   3275,   3276,   3278,   3280,   3281, 
          3282,   3302,   3374,   3409,   3460,   3516,   3794,   3795,   3797,   3853, 
          3855,   3864,   3872,   3884,  11103,  12840,  16627,  16628,  16629,  16630, 
         16631,  16632,  16633,  16634,  16635,  16636,  16732,  17968, 
};


int CClientObjectManager::m_iEntryInfoNodeEntries = 0;
int CClientObjectManager::m_iPointerNodeSingleLinkEntries = 0;
int CClientObjectManager::m_iPointerNodeDoubleLinkEntries = 0;

CClientObjectManager::CClientObjectManager ( CClientManager* pManager )
{
    // Initialize members
    m_pManager = pManager;
    m_bCanRemoveFromList = true;
}


CClientObjectManager::~CClientObjectManager ( void )
{
    // Destroy all objects
    DeleteAll ();
}


void CClientObjectManager::DoPulse ( void )
{
    UpdateLimitInfo ();

    CClientObject * pObject = NULL;
    // Loop through all our streamed-in objects
    vector < CClientObject * > ::iterator iter = m_StreamedIn.begin ();
    for ( ; iter != m_StreamedIn.end () ; ++iter )
    {
        pObject = *iter;
        // We should have a game-object here
        assert ( pObject->GetGameObject () );
        pObject->StreamedInPulse ();
    }
}


void CClientObjectManager::DeleteAll ( void )
{
    // Delete all the objects
    m_bCanRemoveFromList = false;
    std::list < CClientObject* > ::const_iterator iter = m_Objects.begin ();

    for ( ; iter != m_Objects.end (); iter++ )
        (*iter)->Delete();

    // Clear the list
    m_Objects.clear ();
    m_bCanRemoveFromList = true;
}


CClientObject* CClientObjectManager::Get ( ElementID ID )
{
    // Grab the element with the given id. Check its type.
    CClientEntity* pEntity = CElementIDs::GetElement ( ID );
    if ( pEntity && pEntity->GetType () == CCLIENTOBJECT )
    {
        return static_cast < CClientObject* > ( pEntity );
    }

    return NULL;
}


CClientObject* CClientObjectManager::Get ( CObject* pObject, bool bValidatePointer )
{
    if ( !pObject ) return NULL;

    if ( bValidatePointer )
    {
        vector < CClientObject* > ::const_iterator iter = m_StreamedIn.begin ();
        for ( ; iter != m_StreamedIn.end (); iter++ )
        {
            if ( (*iter)->GetGameObject () == pObject )
            {
                return *iter;
            }
        }
    }
    else
    {
        return reinterpret_cast < CClientObject* > ( pObject->GetStoredPointer () );
    }
    return NULL;
}


CClientObject* CClientObjectManager::GetSafe ( CEntity * pEntity )
{
    if ( !pEntity ) return NULL;


    vector < CClientObject* > ::const_iterator iter = m_StreamedIn.begin ();
    for ( ; iter != m_StreamedIn.end (); iter++ )
    {
        if ( dynamic_cast < CEntity * > ( (*iter)->GetGameObject () ) == pEntity )
        {
            return *iter;
        }
    }
    return NULL;
}


bool CClientObjectManager::IsValidModel ( unsigned short ulObjectModel )
{
    bool bIsValid = false;

    if ( ulObjectModel < 20001 )
    {
        unsigned int uiChunk = ulObjectModel / ( sizeof(unsigned int)*8 );
        unsigned int shift = ulObjectModel - ( uiChunk * sizeof(unsigned int)*8 );
        unsigned int bit = 1 << shift;
        bIsValid = !!( g_uiValidObjectModels[uiChunk] & bit );

        // Return whether we can grab the model information or not
        if ( bIsValid && ! g_pGame->GetModelInfo ( ulObjectModel ) )
        {
            bIsValid = false;
        }
    }

    return bIsValid;
}


bool CClientObjectManager::IsBreakableModel ( unsigned long ulObjectModel )
{
    static std::map < unsigned short, short > breakableModelMap;

    // Initialize map if required
    if ( breakableModelMap.size () == 0 )
    {
        for ( uint i = 0; i < sizeof ( g_usBreakableModelList ) / sizeof ( g_usBreakableModelList[0] ) ; i++ )
            breakableModelMap[ g_usBreakableModelList[i] ] = 0;
    }

    return breakableModelMap.find ( (unsigned short)ulObjectModel ) != breakableModelMap.end ();
}


/*
void CClientObjectManager::LoadObjectsAroundPoint ( const CVector& vecPosition, float fRadius )
{
    // Look for objects closer than that in our list
    CClientObject* pObject;
    CVector vecObjectPosition;
    CBoundingBox* pBoundingBox = NULL;
    list < CClientObject* > ::const_iterator iter = m_Objects.begin ();
    for ( ; iter != m_Objects.end (); iter++ )
    {
        // Grab the object
        pObject = *iter;

        if ( pObject->GetDimension () == m_pManager->GetObjectStreamer ()->GetDimension () )
        {
            // Grab the object radius
            float fObjectRadius = 0;
            pBoundingBox = g_pGame->GetModelInfo ( pObject->GetModel () )->GetBoundingBox ();
            if ( pBoundingBox )
            {
                fObjectRadius = pBoundingBox->fRadius;
            }

            // Is it close enough?
            pObject->GetPosition ( vecObjectPosition );
            if ( IsPointNearPoint3D ( vecObjectPosition, vecPosition, fRadius - fObjectRadius ) )
            {
                pObject->Create ();
            }
        }
    }
}
*/


bool CClientObjectManager::ObjectsAroundPointLoaded ( const CVector& vecPosition, float fRadius, unsigned short usDimension, SString* pstrStatus )
{
    // Get list of objects that may be intersecting the sphere
    CClientEntityResult result;
    GetClientSpatialDatabase()->SphereQuery ( result, CSphere ( vecPosition, fRadius ) );

    bool bResult = true;
    // Extract relevant types
    for ( CClientEntityResult::const_iterator it = result.begin () ; it != result.end (); ++it )
    {
        CClientEntity* pEntity = *it;
        if  ( pEntity->GetType () == CCLIENTOBJECT )
        {
            CClientObject* pObject = static_cast < CClientObject* > ( pEntity );
            if ( !pObject->GetGameObject () || !pObject->GetModelInfo ()->IsLoaded () || !pObject->IsStreamedIn () )
            {
                if ( pObject->GetDimension () == usDimension )
                {
                    // Final distance check
                    float fDistSquared = pObject->GetDistanceToBoundingBoxSquared ( vecPosition );
                    if ( fDistSquared < fRadius * fRadius )
                        bResult = false;

                    if ( pstrStatus )
                    {
                        // Debugging information
                        *pstrStatus += SString ( "ID:%05d  Dist:%4.1f  GetGameObject:%d  IsLoaded:%d  IsStreamedIn:%d\n"
                                                ,pObject->GetModel ()
                                                ,sqrtf ( fDistSquared )
                                                ,pObject->GetGameObject () ? 1 : 0
                                                ,pObject->GetModelInfo ()->IsLoaded () ? 1 : 0
                                                ,pObject->IsStreamedIn () ? 1 : 0
                                              );
                    }
                    else
                    if ( !bResult )
                        break;
                }
            }
        }
    }

    return bResult;
}


void CClientObjectManager::OnCreation ( CClientObject * pObject )
{
    m_StreamedIn.push_back ( pObject );
    UpdateLimitInfo ();
}


void CClientObjectManager::OnDestruction ( CClientObject * pObject )
{
    ListRemove ( m_StreamedIn, pObject );
    UpdateLimitInfo ();
}


void CClientObjectManager::UpdateLimitInfo ( void )
{
    CPools* pPools = g_pGame->GetPools ();
    m_iEntryInfoNodeEntries = pPools->GetNumberOfUsedSpaces( ENTRY_INFO_NODE_POOL );
    m_iPointerNodeSingleLinkEntries = pPools->GetNumberOfUsedSpaces( POINTER_SINGLE_LINK_POOL );
    m_iPointerNodeDoubleLinkEntries = pPools->GetNumberOfUsedSpaces( POINTER_DOUBLE_LINK_POOL );
}


bool CClientObjectManager::IsObjectLimitReached ( void )
{
    // Make sure we haven't run out of entry node info's, single link nodes or double link nodes
    #define MAX_ENTRYINFONODES          3500    // Real limit is 4096
    #define MAX_POINTERNODESINGLELINK   65000   // Real limit is 70000
    #define MAX_POINTERNODESDOUBLELINK  3600    // Real limit is 4000

    // If we've run out of either of these limit, don't allow more objects
    if ( m_iEntryInfoNodeEntries >= MAX_ENTRYINFONODES ||
         m_iPointerNodeSingleLinkEntries >= MAX_POINTERNODESINGLELINK ||
         m_iPointerNodeDoubleLinkEntries >= MAX_POINTERNODESDOUBLELINK )
    {
        // Limit reached
        return true;
    }

    // Allow max 250 objects at once for now.
    // TODO: The real limit is up to 350 but we're limited by other limits.
    return g_pGame->GetPools()->GetNumberOfUsedSpaces( OBJECT_POOL ) >= 500;
}


void CClientObjectManager::RestreamObjects ( unsigned short usModel )
{
    // Store the affected vehicles
    CClientObject* pObject;
    std::list < CClientObject* > ::const_iterator iter = IterBegin ();
    for ( ; iter != IterEnd (); iter++ )
    {
        pObject = *iter;

        // Streamed in and same vehicle ID?
        if ( pObject->IsStreamedIn () && pObject->GetModel () == usModel )
        {
            // Stream it out for a while until streamed decides to stream it
            // back in eventually
            pObject->StreamOutForABit ();
        }
    }
}


list < CClientObject* > ::const_iterator CClientObjectManager::IterGet ( CClientObject* pObject )
{
    // Find it in our list
    list < CClientObject* > ::const_iterator iter = m_Objects.begin ();
    for ( ; iter != m_Objects.end (); iter++ )
    {
        if ( *iter == pObject )
        {
            return iter;
        }
    }

    // We couldn't find it
    return m_Objects.begin ();
}


list < CClientObject* > ::const_reverse_iterator CClientObjectManager::IterGetReverse ( CClientObject* pObject )
{
    // Find it in our list
    list < CClientObject* > ::reverse_iterator iter = m_Objects.rbegin ();
    for ( ; iter != m_Objects.rend (); iter++ )
    {
        if ( *iter == pObject )
        {
            return iter;
        }
    }

    // We couldn't find it
    return m_Objects.rbegin ();
}


void CClientObjectManager::RemoveFromList ( CClientObject* pObject )
{
    if ( m_bCanRemoveFromList )
    {
        if ( !m_Objects.empty() ) m_Objects.remove ( pObject );
    }
}


bool CClientObjectManager::Exists ( CClientObject* pObject )
{
    list < CClientObject* > ::const_iterator iter = m_Objects.begin ();
    for ( ; iter != m_Objects.end (); iter++ )
    {
        if ( *iter == pObject )
        {
            return true;
        }
    }

    return false;
}
