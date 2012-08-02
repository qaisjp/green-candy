/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*               (Shared logic for modifications)
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        mods/shared_logic/CClientMarker.cpp
*  PURPOSE:     Marker entity class
*  DEVELOPERS:  Ed Lyons <eai@opencoding.net>
*               Christian Myhre Lundheim <>
*               Jax <>
*               Stanislav Bobrov <lil_toady@hotmail.com>
*               The_GTA <quiret@gmx.de>
*
*****************************************************************************/

#include "StdInc.h"

extern CClientGame* g_pClientGame;

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

static const luaL_Reg marker_interface[] =
{
    { NULL, NULL }
};

static int luaconstructor_marker( lua_State *L )
{
    CClientMarker *marker = (CClientMarker*)lua_touserdata( L, lua_upvalueindex( 1 ) );

    ILuaClass& j = *lua_refclass( L, 1 );
    j.SetTransmit( LUACLASS_MARKER, marker );

    lua_pushvalue( L, LUA_ENVIRONINDEX );
    lua_pushvalue( L, lua_upvalueindex( 1 ) );
    luaL_openlib( L, NULL, marker_interface, 1 );

    lua_basicprotect( L );

    lua_pushlstring( L, "marker", 6 );
    lua_setfield( L, LUA_ENVIRONINDEX, "__type" );
    return 0;
}

unsigned int CClientMarker::m_uiStreamedInMarkers = 0;

CClientMarker::CClientMarker ( CClientManager* pManager, ElementID ID, LuaClass& root, bool system, int iMarkerType ) : CClientStreamElement ( pManager->GetMarkerStreamer (), ID, root, system )
{
    // Lua instancing
    lua_State *L = root.GetVM();

    PushStack( L );
    lua_pushlightuserdata( L, this );
    lua_pushcclosure( L, luaconstructor_marker, 1 );
    luaJ_extend( L, -2, 0 );
    lua_pop( L, 1 );

    // Init
    m_pManager = pManager;
    m_pMarkerManager = pManager->GetMarkerManager ();
    m_pCollision = NULL;
    m_pMarker = NULL;

    // Typename
    SetTypeName ( "marker" );

    // Create the marker
    CreateOfType ( iMarkerType );

    // Add us to marker manager list
    m_pMarkerManager->AddToList ( this );
    UpdateSpatialData ();
}

CClientMarker::~CClientMarker ( void )
{
    // Unlink
    Unlink ();

    // Make sure nothing is still referencing us
    m_pManager->UnreferenceEntity ( this );

    // Remove the colshape
    if ( m_pCollision )
        m_pCollision->Delete();

    // Stream out first so the element counter is correct
    StreamOut ();

    if ( m_pMarker )
    {
        // Destroy the marker class
        delete m_pMarker;
        m_pMarker = NULL;
    }
}

void CClientMarker::Unlink ( void )
{
    // Remove us from marker manager list
    m_pMarkerManager->RemoveFromList ( this );
}

void CClientMarker::GetPosition ( CVector& vecPosition ) const
{
    if ( m_pMarker )
    {
        m_pMarker->GetPosition ( vecPosition );
    }
    else
    {
        vecPosition = CVector ();
    }
}

void CClientMarker::SetPosition( const CVector& vecPosition )
{
    if ( m_pMarker ) m_pMarker->SetPosition ( vecPosition );
    if ( m_pCollision ) m_pCollision->SetPosition ( vecPosition );

    // Update our streaming position
    UpdateStreamPosition ( vecPosition );
}

bool CClientMarker::SetMatrix( const RwMatrix& matrix )
{
    if ( m_pMarker )
        m_pMarker->SetMatrix( matrix );

    if ( m_pCollision )
        m_pCollision->SetPosition( matrix.pos );

    // Update our streaming position
    UpdateStreamPosition ( matrix.pos );
    return true;
}

void CClientMarker::DoPulse()
{
    // Update our position/rotation if we're attached
    DoAttaching ();

    // Pulse the element we contain
    if ( m_pMarker ) m_pMarker->DoPulse ();
}


CClientMarker::eMarkerType CClientMarker::GetMarkerType ( void ) const
{
    // Grab the marker class type
    unsigned int uiMarkerType = m_pMarker->GetMarkerType ();
    switch ( uiMarkerType )
    {
        // If it's a checkpoint, it can be either a checkpoint or a ring
        case CClientMarkerCommon::CLASS_CHECKPOINT:
        {
            unsigned long ulCheckpointType = static_cast < CClientCheckpoint* > ( m_pMarker )->GetCheckpointType ();
            if ( ulCheckpointType == CClientCheckpoint::TYPE_NORMAL )
                return MARKER_CHECKPOINT;
            else
                return MARKER_RING;
        }

        // If it's a 3d marker it can either be a cylinder or an arrow
        case CClientMarkerCommon::CLASS_3DMARKER:
        {
            unsigned long ul3DType = static_cast < CClient3DMarker* > ( m_pMarker )->Get3DMarkerType ();
            if ( ul3DType == CClient3DMarker::TYPE_CYLINDER )
                return MARKER_CYLINDER;
            else
                return MARKER_ARROW;
        }

        // Corona
        case CClientMarkerCommon::CLASS_CORONA:
            return MARKER_CORONA;
    }

    return MARKER_INVALID;
}


void CClientMarker::SetMarkerType ( CClientMarker::eMarkerType eType )
{
    // Different from current type?
    eMarkerType eCurrentType = GetMarkerType ();
    if ( eCurrentType != eType )
    {
        // Current type is a checkpoint and new type is a ring?
        if ( eCurrentType == MARKER_CHECKPOINT && eType == MARKER_RING )
        {
            // Just change the type
            static_cast < CClientCheckpoint* > ( m_pMarker ) ->SetCheckpointType ( CClientCheckpoint::TYPE_RING );
        }

        // Or current type is a ring and new type is a checkpoint?
        if ( eCurrentType == MARKER_RING && eType == MARKER_CHECKPOINT )
        {
            // Just change the type
            static_cast < CClientCheckpoint* > ( m_pMarker ) ->SetCheckpointType ( CClientCheckpoint::TYPE_NORMAL );
        }

        // Current type is a cylinder and new type is an arrow
        if ( eCurrentType == MARKER_CYLINDER && eType == MARKER_ARROW )
        {
            // Just change the type
            static_cast < CClient3DMarker* > ( m_pMarker ) ->Set3DMarkerType ( CClient3DMarker::TYPE_ARROW );
        }

        // Current type is an arrow and new type is an cylinder
        if ( eCurrentType == MARKER_ARROW && eType == MARKER_CYLINDER )
        {
            // Just change the type
            static_cast < CClient3DMarker* > ( m_pMarker ) ->Set3DMarkerType ( CClient3DMarker::TYPE_CYLINDER );
        }

        // No easy way of changing the type. Different classes. Remember position and color and recreate it.
        CVector vecPosition;
        m_pMarker->GetPosition ( vecPosition );
        bool bVisible = m_pMarker->IsVisible ();
        float fSize = m_pMarker->GetSize ();
        SColor color = m_pMarker->GetColor ();
        bool bStreamedIn = IsStreamedIn ();

        // Destroy the old.
        delete m_pMarker;
        m_pMarker = NULL;

        // Create a new one of the correct type
        CreateOfType ( eType );

        // Set the properties back
        SetPosition ( vecPosition );
        SetSize ( fSize );
        SetColor ( color );
        SetVisible ( bVisible );

        // Stream it in if it was streamed in
        if ( bStreamedIn )
        {
            m_pMarker->StreamIn ();
        }
    }
}

CClient3DMarker* CClientMarker::Get3DMarker ( void )
{
    if ( m_pMarker->GetMarkerType () == CClientMarkerCommon::CLASS_3DMARKER )
        return static_cast < CClient3DMarker* > ( m_pMarker );

    return NULL;
}

CClientCheckpoint* CClientMarker::GetCheckpoint ( void )
{
    if ( m_pMarker->GetMarkerType () == CClientMarkerCommon::CLASS_CHECKPOINT )
        return static_cast < CClientCheckpoint* > ( m_pMarker );

    return NULL;
}


CClientCorona* CClientMarker::GetCorona ( void )
{
    if ( m_pMarker->GetMarkerType () == CClientMarkerCommon::CLASS_CORONA )
        return static_cast < CClientCorona* > ( m_pMarker );

    return NULL;
}


bool CClientMarker::IsHit ( const CVector& vecPosition ) const
{
    return m_pMarker->IsHit ( vecPosition );
}


bool CClientMarker::IsHit ( CClientEntity* pEntity ) const
{
    return m_pMarker->IsHit ( pEntity );
}


bool CClientMarker::IsVisible ( void ) const
{
    return m_pMarker->IsVisible ();
}


void CClientMarker::SetVisible ( bool bVisible )
{
    m_pMarker->SetVisible ( bVisible );
}


SColor CClientMarker::GetColor ( void ) const
{
    return m_pMarker->GetColor ();
}


void CClientMarker::SetColor ( const SColor color )
{
    m_pMarker->SetColor ( color );
}


float CClientMarker::GetSize ( void ) const
{
    return m_pMarker->GetSize ();
}


void CClientMarker::SetSize ( float fSize )
{
    switch ( m_pCollision->GetShapeType() )
    {
        case COLSHAPE_CIRCLE:
        {
            CClientColCircle* pShape = static_cast < CClientColCircle* > ( m_pCollision );
            pShape->SetRadius ( fSize );
            break;
        }
        case COLSHAPE_SPHERE:
        {
            CClientColSphere* pShape = static_cast < CClientColSphere* > ( m_pCollision );
            pShape->SetRadius ( fSize );
            break;
        }
    }
    m_pMarker->SetSize ( fSize );
}


int CClientMarker::StringToType ( const char* szString )
{
    if ( stricmp ( szString, "default" ) == 0 || stricmp ( szString, "checkpoint" ) == 0 )
    {
        return MARKER_CHECKPOINT;
    }
    else if ( stricmp ( szString, "ring" ) == 0 )
    {
        return MARKER_RING;
    }
    else if ( stricmp ( szString, "cylinder" ) == 0 )
    {
        return MARKER_CYLINDER;
    }
    else if ( stricmp ( szString, "arrow" ) == 0 )
    {
        return MARKER_ARROW;
    }
    else if ( stricmp ( szString, "corona" ) == 0 )
    {
        return MARKER_CORONA;
    }
    else
    {
        return MARKER_INVALID;
    }
}


bool CClientMarker::TypeToString ( unsigned int uiType, char* szString )
{
    switch ( uiType )
    {
        case MARKER_CHECKPOINT:
            strcpy ( szString, "checkpoint" );
            return true;

        case MARKER_RING:
            strcpy ( szString, "ring" );
            return true;

        case MARKER_CYLINDER:
            strcpy ( szString, "cylinder" );
            return true;

        case MARKER_ARROW:
            strcpy ( szString, "arrow" );
            return true;

        case MARKER_CORONA:
            strcpy ( szString, "corona" );
            return true;

        default:
            strcpy ( szString, "invalid" );
            return false;
    }
}


bool CClientMarker::IsLimitReached ( void )
{
    return m_uiStreamedInMarkers >= 32;
}


void CClientMarker::StreamIn ( bool bInstantly )
{
    // Not already streamed in?
    if ( !IsStreamedIn ( ) )
    {
        // Stream the marker in
        m_pMarker->StreamIn ();

        // Increment streamed in counter
        ++m_uiStreamedInMarkers;

        // Tell the streamer we've created this object
        NotifyCreate ();
    }
}


void CClientMarker::StreamOut ( void )
{
    // Streamed in?
    if ( IsStreamedIn ( ) )
    {
        // Decrement streamed in counter
        --m_uiStreamedInMarkers;

        // Stream the marker out
        m_pMarker->StreamOut ();
    }
}


void CClientMarker::Callback_OnCollision ( CClientColShape& Shape, CClientEntity& Entity )
{
    if ( IS_PLAYER ( &Entity ) )
    {
        // Call the marker hit event
        CLuaArguments Arguments;
        Arguments.PushElement ( &Entity );            // player that hit it
        Arguments.PushBoolean ( ( GetDimension () == Entity.GetDimension () ) ); // matching dimension?
        CallEvent ( "onClientMarkerHit", Arguments, true );
    }
}


void CClientMarker::Callback_OnLeave ( CClientColShape& Shape, CClientEntity& Entity )
{
    if ( IS_PLAYER ( &Entity ) )
    {
        // Call the marker hit event
        CLuaArguments Arguments;
        Arguments.PushElement ( &Entity );            // player that hit it
        Arguments.PushBoolean ( ( Shape.GetDimension () == Entity.GetDimension () ) ); // matching dimension?
        CallEvent ( "onClientMarkerLeave", Arguments, true );
    }
}


void CClientMarker::CreateOfType ( int iType )
{
    if ( m_pCollision )
    {
        m_pCollision->Delete();
        m_pCollision = NULL;
    }

    CVector vecOrigin;
    GetPosition ( vecOrigin );    
    switch ( iType )
    {
        case MARKER_CHECKPOINT:
        {
            CClientCheckpoint* pCheckpoint = new CClientCheckpoint ( this );
            pCheckpoint->SetCheckpointType ( CClientCheckpoint::TYPE_NORMAL );
            m_pMarker = pCheckpoint;
            m_pCollision = new CClientColCircle ( g_pClientGame->GetManager(), NULL, *this, IsSystemEntity(), vecOrigin, GetSize() );
            m_pCollision->m_pOwningMarker = this;
            m_pCollision->SetHitCallback ( this );
            break;
        }

        case MARKER_RING:
        {
            CClientCheckpoint* pCheckpoint = new CClientCheckpoint ( this );
            pCheckpoint->SetCheckpointType ( CClientCheckpoint::TYPE_RING );
            m_pMarker = pCheckpoint;
            m_pCollision = new CClientColSphere ( g_pClientGame->GetManager(), NULL, *this, IsSystemEntity(), vecOrigin, GetSize() );
            m_pCollision->m_pOwningMarker = this;
            m_pCollision->SetHitCallback ( this );
            break;
        }

        case MARKER_CYLINDER:
        {
            CClient3DMarker* p3DMarker = new CClient3DMarker ( this );
            p3DMarker->Set3DMarkerType ( CClient3DMarker::TYPE_CYLINDER );
            m_pMarker = p3DMarker;
            m_pCollision = new CClientColCircle ( g_pClientGame->GetManager(), NULL, *this, IsSystemEntity(), vecOrigin, GetSize() );
            m_pCollision->m_pOwningMarker = this;
            m_pCollision->SetHitCallback ( this );
            break;
        }

        case MARKER_ARROW:
        {
            CClient3DMarker* p3DMarker = new CClient3DMarker ( this );
            p3DMarker->Set3DMarkerType ( CClient3DMarker::TYPE_ARROW );
            m_pMarker = p3DMarker;
            m_pCollision = new CClientColSphere ( g_pClientGame->GetManager(), NULL, *this, IsSystemEntity(), vecOrigin, GetSize() );
            m_pCollision->m_pOwningMarker = this;
            m_pCollision->SetHitCallback ( this );
            break;
        }

        case MARKER_CORONA:
        {
            m_pMarker = new CClientCorona ( this );
            m_pCollision = new CClientColSphere ( g_pClientGame->GetManager(), NULL, *this, IsSystemEntity(), vecOrigin, GetSize() );
            m_pCollision->m_pOwningMarker = this;
            m_pCollision->SetHitCallback ( this );
            break;
        }
        
        default:
            break;
    }
}


CSphere CClientMarker::GetWorldBoundingSphere ( void )
{
    CSphere sphere;
    GetPosition ( sphere.vecPosition );
    //sphere.vecPosition = GetStreamPosition ();
    sphere.fRadius = GetSize ();
    return sphere;
}
