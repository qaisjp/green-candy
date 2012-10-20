/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*               (Shared logic for modifications)
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        mods/shared_logic/CClientColManager.cpp
*  PURPOSE:     Collision entity manager class
*  DEVELOPERS:  Christian Myhre Lundheim <>
*               Jax <>
*               The_GTA <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#include "StdInc.h"

using std::list;
using std::vector;

//
// This is now a mess due to backward compatibilty.
// ** Review before v1.1 release **
//

CClientColManager::~CClientColManager()
{
}

void CClientColManager::DoPulse()
{
    colshapes_t::const_iterator iter = m_List.begin();

    for ( ; iter != m_List.end(); ++iter )
        (*iter)->DoPulse();
}

#ifdef SPATIAL_DATABASE_TESTS

struct CTestSet
{
    struct CTestResult
    {
        struct CEntityPair
        {
            CEntityPair ( CClientColShape* pInColShape, CClientEntity* pInEntity )
                : pColShape(pInColShape)
                , pEntity(pInEntity)
            {}
            CClientColShape* pColShape;
            CClientEntity* pEntity;
        };
        std::vector < CEntityPair > onList;
        std::vector < CEntityPair > offList;
    };
    CTestResult oldWay;
    CTestResult newWay;
    bool bCountOnly;
};

CTestSet testResult;

bool MyDataSortPredicate ( const CTestSet::CTestResult::CEntityPair& d1, const CTestSet::CTestResult::CEntityPair& d2 )
{
    return d1.pColShape == d2.pColShape ? d1.pEntity < d2.pEntity : d1.pColShape < d2.pColShape;
}

bool operator== ( const CTestSet::CTestResult::CEntityPair& d1, const CTestSet::CTestResult::CEntityPair& d2 )
{
    return d1.pColShape == d2.pColShape && d1.pEntity == d2.pEntity;
}

void RemoveSameItems( std::vector < CTestSet::CTestResult::CEntityPair >& listA ,std::vector < CTestSet::CTestResult::CEntityPair >& listB )
{
again:
    vector < CTestSet::CTestResult::CEntityPair > ::const_iterator itA = listA.begin ();
    for ( ; itA != listA.end (); itA++ )
    {
        vector < CTestSet::CTestResult::CEntityPair > ::const_iterator itB = listB.begin ();
        for ( ; itB != listB.end (); itB++ )
        {
            if ( *itA == *itB )
            {
                listA.erase ( itA );
                listB.erase ( itB );
                goto again;
            }
        }
    }
}

// Test version
void CClientColManager::DoHitDetection ( const CVector& vecNowPosition, float fRadius, CClientEntity* pEntity, CClientColShape * pJustThis, bool bChildren )
{
    // Prepare
    testResult = CTestSet ();
    testResult.bCountOnly = true;

    // Do old way
    DoHitDetectionOld ( vecNowPosition, fRadius, pEntity, pJustThis, bChildren );

    // Do new way
    if ( pJustThis )
        DoHitDetectionForColShape ( pJustThis );
    else
        DoHitDetectionForEntity ( vecNowPosition, 0.0f, pEntity );

    // Check test results

    // Order lists
    std::sort( testResult.oldWay.onList.begin(), testResult.oldWay.onList.end(), MyDataSortPredicate );
    std::sort( testResult.oldWay.offList.begin(), testResult.oldWay.offList.end(), MyDataSortPredicate );
    std::sort( testResult.newWay.onList.begin(), testResult.newWay.onList.end(), MyDataSortPredicate );
    std::sort( testResult.newWay.offList.begin(), testResult.newWay.offList.end(), MyDataSortPredicate );

    #ifdef SPATIAL_DATABASE_DEBUG_OUTPUTC
        // Report count
        if ( testResult.oldWay.onList.size () || testResult.newWay.onList.size () )
            OutputDebugLine( SString ( "onList  colshape:%d  newWay:%d  oldWay:%d", pJustThis!=NULL, testResult.newWay.onList.size (), testResult.oldWay.onList.size () ) );

        if ( testResult.oldWay.offList.size () || testResult.newWay.offList.size () )
            OutputDebugLine( SString ( "offList  colshape:%d  newWay:%d  oldWay:%d", pJustThis!=NULL, testResult.newWay.offList.size (), testResult.oldWay.offList.size () ) );

        // Report mismatch
        if ( testResult.oldWay.onList != testResult.newWay.onList )
            OutputDebugLine( SString ( "onList mismatch  colshape:%d  newWay:%d  oldWay:%d", pJustThis!=NULL, testResult.newWay.onList.size (), testResult.oldWay.onList.size () ) );

        if ( testResult.oldWay.offList != testResult.newWay.offList )
            OutputDebugLine( SString ( "offList mismatch  colshape:%d  newWay:%d  oldWay:%d", pJustThis!=NULL, testResult.newWay.offList.size (), testResult.oldWay.offList.size () ) );
    #endif

    // Diff
    RemoveSameItems( testResult.oldWay.onList, testResult.newWay.onList );
    RemoveSameItems( testResult.oldWay.offList, testResult.newWay.offList );

    // Output errors
    for ( vector < CTestSet::CTestResult::CEntityPair > ::const_iterator it = testResult.oldWay.onList.begin (); it != testResult.oldWay.onList.end (); it++ )
    {
        CClientColShape* pColShape = (*it).pColShape;
        CClientEntity* pEntity = (*it).pEntity;
        CLogger::ErrorPrintf( "oldWay.onList has extra entity type: %d %s", pEntity->GetType (), pEntity->GetTypeName () );
    }

    for ( vector < CTestSet::CTestResult::CEntityPair > ::const_iterator it = testResult.oldWay.offList.begin (); it != testResult.oldWay.offList.end (); it++ )
    {
        CClientColShape* pColShape = (*it).pColShape;
        CClientEntity* pEntity = (*it).pEntity;
        CLogger::ErrorPrintf( "oldWay.offList has extra entity type: %d %s", pEntity->GetType (), pEntity->GetTypeName () );
    }

    for ( vector < CTestSet::CTestResult::CEntityPair > ::const_iterator it = testResult.newWay.onList.begin (); it != testResult.newWay.onList.end (); it++ )
    {
        CClientColShape* pColShape = (*it).pColShape;
        CClientEntity* pEntity = (*it).pEntity;
        CLogger::ErrorPrintf( "newWay.onList has extra entity type: %d %s", pEntity->GetType (), pEntity->GetTypeName () );
    }

    for ( vector < CTestSet::CTestResult::CEntityPair > ::const_iterator it = testResult.newWay.offList.begin (); it != testResult.newWay.offList.end (); it++ )
    {
        CClientColShape* pColShape = (*it).pColShape;
        CClientEntity* pEntity = (*it).pEntity;
        CLogger::ErrorPrintf( "newWay.offList has extra entity type: %d %s", pEntity->GetType (), pEntity->GetTypeName () );
    }

    // Finally, use the old way to DoHitDetection
    testResult = CTestSet ();
    testResult.bCountOnly = false;
    DoHitDetectionOld ( vecNowPosition, fRadius, pEntity, pJustThis, bChildren );
}


void CClientColManager::DoHitDetectionOld ( const CVector& vecNowPosition, float fRadius, CClientEntity* pEntity, CClientColShape * pJustThis, bool bChildren )
{
    if ( bChildren )
    {
        list < CClientEntity* > ::const_iterator iter = pEntity->IterBegin ();
        for ( ; iter != pEntity->IterEnd (); iter++ )
        {
            CVector vecPosition;
            (*iter)->GetPosition( vecPosition );
            DoHitDetectionOld ( vecPosition, 0.0f, *iter, pJustThis, true );
        }
        switch ( pEntity->GetType () )
        {
            case CCLIENTRADARMARKER:
            case CCLIENTRADARAREA:
            case CCLIENTTEAM:
            case CCLIENTGUI:
            case CCLIENTCOLSHAPE:
            case CCLIENTDUMMY:
            case SCRIPTFILE:
            case CCLIENTDFF:
            case CCLIENTCOL:
            case CCLIENTTXD:
            case CCLIENTSOUND:
            case CCLIENTHANDLING:
                return;
        }
    }

    vector < CClientColShape * > cloneList;

    if ( !pJustThis )
    {
        // Call the hit detection event on all our shapes
        cloneList = m_List;
    }
    else
    {
        cloneList.push_back ( pJustThis );
    }

    vector < CClientColShape* > ::const_iterator iter = cloneList.begin ();
    for ( ; iter != cloneList.end (); ++iter )
    {
        CClientColShape* pShape = *iter;

        // Enabled and not being deleted?
        if ( !pShape->IsBeingDeleted () && pShape->IsEnabled () )
        {
            // Collided?
            if ( pShape->DoHitDetection ( vecNowPosition, fRadius ) )
            {              
                // If they havn't collided yet
                if ( !pEntity->CollisionExists ( pShape ) )
                {      
                    testResult.oldWay.onList.push_back ( CTestSet::CTestResult::CEntityPair ( pShape, pEntity ) );

                    if ( !testResult.bCountOnly )
                    {
                        // Add the collision and the collider
                        pShape->AddCollider ( pEntity );
                        pEntity->AddCollision ( pShape );

                        // Can we call the event?
                        if ( pShape->GetAutoCallEvent () )
                        {
                            // Call the event
                            CLuaArguments Arguments;
                            Arguments.PushElement ( pEntity );
                            Arguments.PushBoolean ( ( pShape->GetDimension () == pEntity->GetDimension () ) );
                            pShape->CallEvent ( "onClientColShapeHit", Arguments, true );

                            CLuaArguments Arguments2;
                            Arguments2.PushElement ( pShape );
                            Arguments2.PushBoolean ( ( pShape->GetDimension () == pEntity->GetDimension () ) );
                            pEntity->CallEvent ( "onClientElementColShapeHit", Arguments2, true );
                        }

                        // Run whatever callback the collision item might have attached
                        pShape->CallHitCallback ( *pEntity );
                    }
                }
            }
            else
            {
                // If they collided before
                if ( pEntity->CollisionExists ( pShape ) )
                {
                    testResult.oldWay.offList.push_back ( CTestSet::CTestResult::CEntityPair ( pShape, pEntity ) );

                    if ( !testResult.bCountOnly )
                    {
                        // Remove the collision and the collider
                        pShape->RemoveCollider ( pEntity );
                        pEntity->RemoveCollision ( pShape );

                        // Call the event
                        CLuaArguments Arguments;
                        Arguments.PushElement ( pEntity );
                        Arguments.PushBoolean ( ( pShape->GetDimension () == pEntity->GetDimension () ) );
                        pShape->CallEvent ( "onClientColShapeLeave", Arguments, true );

                        CLuaArguments Arguments2;
                        Arguments2.PushElement ( pShape );
                        Arguments2.PushBoolean ( ( pShape->GetDimension () == pEntity->GetDimension () ) );
                        pEntity->CallEvent ( "onClientElementColShapeLeave", Arguments2, true );

                        pShape->CallLeaveCallback ( *pEntity );
                    }
                }
            }
        }
    }
}

#else  // SPATIAL_DATABASE_TESTS

void CClientColManager::DoHitDetection ( const CVector& vecNowPosition, float fRadius, CClientEntity* pEntity, CClientColShape * pJustThis, bool bChildren )
{
    if ( pJustThis )
        DoHitDetectionForColShape ( pJustThis );
    else
        DoHitDetectionForEntity ( vecNowPosition, 0.0f, pEntity );
}

#endif  // SPATIAL_DATABASE_TESTS


//
// Handle the changing state of collision between one colshape and any entity
//
void CClientColManager::DoHitDetectionForColShape ( CClientColShape* pShape )
{
    // Ensure colshape is enabled
    if ( !pShape->IsEnabled() )
        return;

    std::map < CClientEntity*, int > entityList;

    // Get all entities within the sphere
    CSphere querySphere = pShape->GetWorldBoundingSphere ();
    CClientEntityResult result;
    GetClientSpatialDatabase()->SphereQuery ( result, querySphere );
 
    // Extract relevant types
    for ( CClientEntityResult::const_iterator it = result.begin(); it != result.end(); ++it )
    {
        CClientEntity *pEntity = *it;
        switch( pEntity->GetType() )
        {
        case CCLIENTRADARMARKER:
        case CCLIENTRADARAREA:
        case CCLIENTTEAM:
        case CCLIENTGUI:
        case CCLIENTCOLSHAPE:
        case CCLIENTDUMMY:
        case SCRIPTFILE:
        case CCLIENTDFF:
        case CCLIENTCOL:
        case CCLIENTTXD:
        case CCLIENTSOUND:
            break;
        default:
            if ( pEntity->GetParent() )
                entityList[ pEntity ] = 1;

            break;
        }
    }

    // Add existing colliders, so they can be disconnected if required
    for ( list < CClientEntity* > ::const_iterator it = pShape->CollidersBegin () ; it != pShape->CollidersEnd (); ++it )
    {
        entityList[ *it ] = 1;
    }

    // We wanna reference our collider
    pShape->IncrementMethodStack();

    // Test each entity against the colshape
    for ( std::map < CClientEntity*, int > ::const_iterator it = entityList.begin () ; it != entityList.end (); ++it )
    {
        CClientEntity *pEntity = it->first;
        CVector vecPosition;
        pEntity->GetPosition( vecPosition );

        // Reference as we will extensively use it
        pEntity->IncrementMethodStack();

        HandleHitDetectionResult( pShape->DoHitDetection( vecPosition, 0.0f ), pShape, pEntity );

        // The entity can be destroyed
        pEntity->DecrementMethodStack();
    }

    pShape->DecrementMethodStack();
}


//
// Handle the changing state of collision between one entity and any colshape
//
void CClientColManager::DoHitDetectionForEntity ( const CVector& vecNowPosition, float fRadius, CClientEntity* pEntity )
{
    std::map < CClientColShape*, int > shortList;

    // Get all entities within the sphere
    CClientEntityResult queryResult;
    GetClientSpatialDatabase()->SphereQuery ( queryResult, CSphere ( vecNowPosition, fRadius ) );

    // Extract colshapes
    for ( CClientEntityResult ::const_iterator it = queryResult.begin () ; it != queryResult.end (); ++it )
        if ( (*it)->GetType () == CCLIENTCOLSHAPE )
            shortList[ (CClientColShape*)*it ] = 1;

    // Add existing collisions, so they can be disconnected if required
    for ( list < CClientColShape* > ::const_iterator it = pEntity->CollisionsBegin () ; it != pEntity->CollisionsEnd (); ++it )
        shortList[ *it ] = 1;

    // Keep the entity alive
    pEntity->IncrementMethodStack();

    // Test each colshape against the entity
    for ( std::map < CClientColShape*, int > ::const_iterator it = shortList.begin () ; it != shortList.end (); ++it )
    {
        CClientColShape* pShape = it->first;

        // Enabled?
        if ( !pShape->IsEnabled() )
            continue;

        pShape->IncrementMethodStack();

        HandleHitDetectionResult( pShape->DoHitDetection( vecNowPosition, fRadius ), pShape, pEntity );

        pShape->DecrementMethodStack();
    }

    pEntity->DecrementMethodStack();
}


//
// Handle the changing state of collision between one colshape and one entity
//
void CClientColManager::HandleHitDetectionResult ( bool bHit, CClientColShape* pShape, CClientEntity* pEntity )
{
    if ( bHit )
    {              
        // If they havn't collided yet
        if ( !pEntity->CollisionExists ( pShape ) )
        {
#ifdef SPATIAL_DATABASE_TESTS
            testResult.newWay.onList.push_back ( CTestSet::CTestResult::CEntityPair ( pShape, pEntity ) );
            if ( !testResult.bCountOnly )
#endif
            {
                // Add the collision and the collider
                pShape->AddCollider ( pEntity );
                pEntity->AddCollision ( pShape );

                // Can we call the event?
                if ( pShape->GetAutoCallEvent () )
                {
                    // Call the event
                    lua_State *L = g_pClientGame->GetLuaManager()->GetVirtualMachine();
                    pEntity->PushStack( L );
                    lua_pushboolean( L, pShape->GetDimension() == pEntity->GetDimension() );
                    pShape->CallEvent( "onClientColShapeHit", L, 2 );

                    pShape->PushStack( L );
                    lua_pushboolean( L, pShape->GetDimension() == pEntity->GetDimension() );
                    pEntity->CallEvent( "onClientElementColShapeHit", L, 2 );
                }

                // Run whatever callback the collision item might have attached
                pShape->CallHitCallback ( *pEntity );
            }
        }
    }
    else
    {
        // If they collided before
        if ( pEntity->CollisionExists ( pShape ) )
        {

#ifdef SPATIAL_DATABASE_TESTS
            testResult.newWay.offList.push_back ( CTestSet::CTestResult::CEntityPair ( pShape, pEntity ) );
            if ( !testResult.bCountOnly )
#endif
            {
                // Remove the collision and the collider
                pShape->RemoveCollider ( pEntity );
                pEntity->RemoveCollision ( pShape );

                // Call the event
                lua_State *L = g_pClientGame->GetLuaManager()->GetVirtualMachine();
                pEntity->PushStack( L );
                lua_pushboolean( L, pShape->GetDimension() == pEntity->GetDimension() );
                pShape->CallEvent( "onClientColShapeLeave", L, 2 );

                pShape->PushStack( L );
                lua_pushboolean( L, pShape->GetDimension() == pEntity->GetDimension() );
                pEntity->CallEvent( "onClientElementColShapeLeave", L, 2 );

                pShape->CallLeaveCallback( *pEntity );
            }
        }
    }
}

bool CClientColManager::Exists( CClientColShape* pShape )
{
    // Return true if it exists
    return ListContains( m_List, pShape );
}

void CClientColManager::RemoveFromList( CClientColShape* pShape )
{
    ListRemove( m_List, pShape );
}
