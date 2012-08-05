/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*               (Shared logic for modifications)
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        mods/shared_logic/CClientEntity.cpp
*  PURPOSE:     Base entity class
*  DEVELOPERS:  Ed Lyons <eai@opencoding.net>
*               Christian Myhre Lundheim <>
*               Chris McArthur <>
*               Jax <>
*               Kevin Whiteside <kevuwk@gmail.com>
*               Stanislav Bobrov <lil_toady@hotmail.com>
*               Alberto Alonso <rydencillo@gmail.com>
*               The_GTA <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#include "StdInc.h"

using std::list;

extern CClientGame* g_pClientGame;

static int entity_isValidChild( lua_State *L )
{
    lua_pushboolean( L, lua_type( L, 1 ) == LUA_TCLASS && lua_refclass( L, 1 )->IsTransmit( LUACLASS_ENTITY ) );
    return 1;
}

static const luaL_Reg entity_interface[] =
{
    { "isValidChild", entity_isValidChild },
    { NULL, NULL }
};

static int customdata_newindex( lua_State *L )
{
    if ( lua_type( L, 2 ) != LUA_TSTRING )
        throw lua_exception( SString( "found %s at customdata key, string required", luaL_typename( L, 2 ) ) );

    if ( lua_objlen( L, 2 ) > MAX_CUSTOMDATA_NAME_LENGTH )
        throw lua_exception( "customdata name too long" );

    lua_pushvalue( L, 2 );
    lua_rawget( L, 1 );

    if ( !lua_equal( L, 3, 4 ) )
        return 0;

    lua_pushvalue( L, 2 );
    lua_pushvalue( L, 3 );
    lua_rawset( L, 1 );

    // Trigger the onClientElementDataChange event on us
    CLuaArguments Arguments;
    Arguments.ReadArgument( L, 2 );
    Arguments.ReadArgument( L, 4 );
    Arguments.ReadArgument( L, 3 );
    ((CClientEntity*)lua_touserdata( L, lua_upvalueindex( 1 ) ) )->CallEvent( "onClientElementDataChange", Arguments, true );
    return 0;
}

static int entity_constructor( lua_State *L )
{
    lua_settop( L, 1 );

    CClientEntity *entity = (CClientEntity*)lua_touserdata( L, lua_upvalueindex( 1 ) );

    ILuaClass& j = *lua_refclass( L, 1 );
    j.SetTransmit( LUACLASS_ENTITY, entity );

    lua_pushvalue( L, LUA_ENVIRONINDEX );
    lua_pushvalue( L, lua_upvalueindex( 1 ) );
    luaL_openlib( L, NULL, entity_interface, 1 );

    // Allocate table for custom data
    lua_newtable( L );
    lua_newtable( L ); // metatable

    lua_pushvalue( L, lua_upvalueindex( 1 ) );
    lua_pushcclosure( L, customdata_newindex, 1 );
    lua_setfield( L, 4, "__newindex" );
    
    lua_pushboolean( L, false );
    lua_setfield( L, 4, "__metatable" );
    
    lua_setmetatable( L, 3 );
    lua_setfield( L, LUA_ENVIRONINDEX, "data" );

    lua_pushlstring( L, "entity", 6 );
    lua_setfield( L, LUA_ENVIRONINDEX, "__type" );
    return 0;
}

static int sysentity_index( lua_State *L )
{
    if ( lua_type( L, 2 ) == LUA_TSTRING )
    {
        std::string key = lua_getstring( L, 2 );

        if ( key == "destroy" )
            return 0;

        if ( key == "setParent" )
            return 0;
    }

    lua_getfield( L, LUA_ENVIRONINDEX, "super" );
    lua_insert( L, 1 );
    lua_call( L, 2, 1 );
    return 1;
}

static const luaL_Reg sysentity_interface[] =
{
    { "__index", sysentity_index },
    { NULL, NULL }
};

static int sysentity_constructor( lua_State *L )
{
    CClientEntity *entity = (CClientEntity*)lua_touserdata( L, lua_upvalueindex( 1 ) );

    ILuaClass& j = *lua_refclass( L, 1 );
    j.SetTransmit( LUACLASS_SYSENTITY, entity );

    lua_pushvalue( L, LUA_ENVIRONINDEX );
    lua_pushvalue( L, lua_upvalueindex( 1 ) );
    luaL_openlib( L, NULL, sysentity_interface, 1 );

    lua_pushlstring( L, "sysentity", 9 );
    lua_setfield( L, LUA_ENVIRONINDEX, "__type" );
    return 0;
}

CClientEntity::CClientEntity( ElementID ID, bool system, LuaClass& root ) : LuaElement( root ),
        m_FromRootNode( this ),
        m_ChildrenNode( this ),
        m_Children( &CClientEntity::m_ChildrenNode )
{
    lua_State *L = root.GetVM();

    // Extend our entity instance
    PushStack( root.GetVM() );
    lua_pushlightuserdata( L, this );
    lua_pushcclosure( L, entity_constructor, 1 );
    luaJ_extend( L, -2, 0 );

    if ( system )
    {
        lua_pushlightuserdata( L, this );
        lua_pushcclosure( L, sysentity_constructor, 1 );
        luaJ_extend( L, -2, 0 );
    }
    lua_pop( L, 1 );

    m_bSystemEntity = system;

    // Init
    m_pManager = NULL;
    m_pParent = NULL;
    m_usDimension = 0;
    m_ucSyncTimeContext = 0;
    m_ucInterior = 0;
    m_bDoubleSided = false;
    m_bDoubleSidedInit = false;

    // Need to generate a clientside ID?
    if ( ID == INVALID_ELEMENT_ID )
    {
        ID = CElementIDs::PopClientID ();
    }

    // Set our index in the element array
    m_ID = ID;
    CElementIDs::SetElement ( ID, this );

    m_pCustomData = new CCustomData;
    m_pEventManager = new CMapEventManager;

    m_pAttachedToEntity = NULL;

    strncpy ( m_szTypeName, "unknown", MAX_TYPENAME_LENGTH );
    m_szTypeName [MAX_TYPENAME_LENGTH] = 0;
    m_uiTypeHash = HashString ( m_szTypeName );
    if ( IsFromRoot ( m_pParent ) )
        CClientEntity::AddEntityFromRoot ( m_uiTypeHash, this );

    m_szName [0] = 0;

    m_pElementGroup = NULL;
    m_pModelInfo = NULL;
}

CClientEntity::~CClientEntity()
{
    // Before we do anything, fire the on-destroy event
    CLuaArguments Arguments;
    CallEvent( "onClientElementDestroy", Arguments, true );

    // Remove from parent
    ClearChildren ();
    SetParent ( NULL );

    // Reset our index in the element array
    if ( m_ID != INVALID_ELEMENT_ID )
    {
        CElementIDs::SetElement ( m_ID, NULL );

        // Got a clientside ID? Push it back on our stack so we don't run out of client id's
        if ( IsLocalEntity () )
        {
            CElementIDs::PushClientID ( m_ID );
        }
    }

    // Remove our custom data
    if ( m_pCustomData )
    {
        delete m_pCustomData;
    }

    if ( m_pAttachedToEntity )
    {
        m_pAttachedToEntity->RemoveAttachedEntity ( this );
    }

    for ( attachments_t::iterator iter = m_AttachedEntities.begin() ; iter != m_AttachedEntities.end(); ++iter )
    {
        CClientEntity* pAttachedEntity = *iter;
        if ( pAttachedEntity )
        {
            pAttachedEntity->m_pAttachedToEntity = NULL;
        }
    }

    RemoveAllCollisions ( true );

    if ( m_pEventManager )
    {
        delete m_pEventManager;
        m_pEventManager = NULL;
    }

    if ( m_pElementGroup )
    {
        m_pElementGroup->Remove ( this );
    }

    list < CClientPed* > ::iterator iterUsers = m_OriginSourceUsers.begin ();
    for ( ; iterUsers != m_OriginSourceUsers.end () ; iterUsers++ )
    {
        CClientPed* pModel = *iterUsers;
        if ( pModel->m_interp.pTargetOriginSource == this )
        {
            pModel->m_interp.pTargetOriginSource = NULL;
            pModel->m_interp.bHadOriginSource = true;
        }
    }
    m_OriginSourceUsers.clear ();

    // Unlink our contacts
    list < CClientPed * > ::iterator iterContacts = m_Contacts.begin ();
    for ( ; iterContacts != m_Contacts.end () ; iterContacts++ )
    {
        CClientPed * pModel = *iterContacts;
        if ( pModel->GetCurrentContactEntity () == this )
        {
            pModel->SetCurrentContactEntity ( NULL );
        }
    }
    m_Contacts.clear ();

    // Remove from spatial database
    if ( !g_pClientGame->IsBeingDeleted () )
        GetClientSpatialDatabase ()->RemoveEntity ( this );

    // Ensure intrusive list nodes have been isolated
    assert ( m_FromRootNode.m_pOuterItem == this && !m_FromRootNode.m_pPrev && !m_FromRootNode.m_pNext );
    assert ( m_ChildrenNode.m_pOuterItem == this && !m_ChildrenNode.m_pPrev && !m_ChildrenNode.m_pNext );

    if ( !g_pClientGame->IsBeingDeleted () )
        CClientEntityRefManager::OnEntityDelete ( this );
}

void CClientEntity::SetTypeName ( const char* szName )
{
    CClientEntity::RemoveEntityFromRoot ( m_uiTypeHash, this );
    strncpy ( m_szTypeName, szName, MAX_TYPENAME_LENGTH );
    m_uiTypeHash = HashString ( szName );

    if ( IsFromRoot ( m_pParent ) )
        CClientEntity::AddEntityFromRoot ( m_uiTypeHash, this );
}

bool CClientEntity::CanUpdateSync ( unsigned char ucRemote )
{
    // We can update this element's sync only if the sync time
    // matches or either of them are 0 (ignore).
    return ( m_ucSyncTimeContext == ucRemote || ucRemote == 0 || m_ucSyncTimeContext == 0 );
}

bool CClientEntity::SetParent ( CClientEntity* pParent )
{
    // Lua is top priority, so update it first
    PushMethod( m_lua, "setParent" );
    pParent->PushStack( m_lua );
    lua_call( m_lua, 1, 1 );

    bool rslt = lua_toboolean( m_lua, -1 );
    lua_pop( L, 1 );

    if ( !rslt )
        return false;

    // Get into/out-of FromRoot info
    bool bOldFromRoot = CClientEntity::IsFromRoot( m_pParent );
    bool bNewFromRoot = CClientEntity::IsFromRoot( pParent );

    // Remove us from previous parent's children list
    if ( m_pParent )
        m_pParent->m_Children.remove ( this );

    // Set the new parent
    m_pParent = pParent;

    // Add us to the new parent's children list
    if ( pParent )
        pParent->m_Children.push_back ( this );

    // Moving out of FromRoot?
    if ( bOldFromRoot && !bNewFromRoot )
        CClientEntity::RemoveEntityFromRoot ( m_uiTypeHash, this );

    // Moving into FromRoot?
    if ( !bOldFromRoot && bNewFromRoot )
        CClientEntity::AddEntityFromRoot ( m_uiTypeHash, this );

    return true;
}

bool CClientEntity::IsMyChild ( CClientEntity* pEntity, bool bRecursive )
{
    // Is he us?
    if ( pEntity == this )
        return true;

    // Is he our child directly?
    CChildListType ::const_iterator iter = m_Children.begin();

    for ( ; iter != m_Children.end (); iter++ )
    {
        // Return true if this is our child. If not check if he's one of our children's children if we were asked to do a recursive search.
        if ( *iter == pEntity )
            return true;
        else if ( bRecursive && (*iter)->IsMyChild ( pEntity, true ) )
            return true;
    }

    return false;
}

void CClientEntity::ClearChildren()
{
    // Process our children - Move up to our parent
    while ( m_Children.size() )
        (*m_Children.begin())->SetParent ( m_pParent );
}

void CClientEntity::SetID ( ElementID ID )
{
    // Eventually reset what we have at the old ID
    if ( m_ID != INVALID_ELEMENT_ID )
    {
        CElementIDs::SetElement ( m_ID, NULL );

        // Are we a clientside element? Push the unique ID
        if ( IsLocalEntity () )
        {
            CElementIDs::PushClientID ( m_ID );
        }
    }

    // Set our new ID and index in the array
    m_ID = ID;

    if ( ID != INVALID_ELEMENT_ID )
    {
        CElementIDs::SetElement ( ID, this );
    }
}

void CClientEntity::ApplyCustomData( CCustomData *data )
{
    CCustomData::itemList_t::const_iterator iter = data->IterBegin();

    PushStack( m_lua );
    lua_getfield( m_lua, -1, "data" );

    for ( ; iter != data->IterEnd(); iter++ )
    {
        lua_pushlstring( m_lua, (*iter).first.c_str(), (*iter).first.size() );
        (*iter).second.Variable.Push( m_lua );

        lua_settable( L, -3 );
    }

    lua_pop( L, 2 );
}

void CClientEntity::PushCustomData( lua_State *L, const char *key, bool inherit )
{
    // Try our data first
    PushStack( L );
    lua_getfield( L, -1, "data" );
    
    if ( lua_isnil( L, -1 ) && inherit && m_pParent )
    {
        lua_pop( L, 1 );

        // Might try parent's data too
        m_pParent->PushCustomData( L, key, true );
    }

    lua_insert( L, -3 );
    lua_pop( L, 2 );
}

bool CClientEntity::GetCustomDataString ( const char* szName, SString& strOut, bool bInheritData )
{
    bool success;
    PushCustomData( m_lua, szName, bInheritData );

    if ( success = ( lua_isstring( m_lua, -1 ) == 1 ) )
        strOut = lua_tostring( m_lua, -1 );

    lua_pop( m_lua, 1 );
    return success;
}

bool CClientEntity::GetCustomDataInt ( const char* szName, int& iOut, bool bInheritData )
{
    bool success;
    PushCustomData( m_lua, szName, bInheritData );

    if ( success = ( lua_isnumber( m_lua, -1 ) == 1 ) )
        iOut = lua_tonumber( m_lua, -1 );

    lua_pop( m_lua, 1 );
    return success;
}

bool CClientEntity::GetCustomDataFloat ( const char* szName, float& fOut, bool bInheritData )
{
    bool success;
    PushCustomData( m_lua, szName, bInheritData );

    if ( success = ( lua_isnumber( m_lua, -1 ) == 1 ) )
        fOut = lua_tonumber( m_lua, -1 );

    lua_pop( m_lua, 1 );
    return success;
}

bool CClientEntity::GetCustomDataBool ( const char* szName, bool& bOut, bool bInheritData )
{
    PushCustomData( m_lua, szName, bInheritData );

    switch( lua_type( m_lua, -1 ) )
    {
    case LUA_TSTRING:
        bOut = lua_getstring( L, -1 ) == "true";
        break;
    case LUA_TNUMBER:
        bOut = lua_tonumber( L, -1 ) != 0;
        break;
    case LUA_TBOOLEAN:
        bOut = lua_toboolean( L, -1 );
        break;
    default:
        lua_pop( L, 1 );
        return false;
    }

    lua_pop( L, 1 );
    return true;
}

bool CClientEntity::DeleteCustomData ( const char* szName, bool bRecursive )
{
    PushStack( m_lua );
    lua_getfield( m_lua, -1, "data" );
    lua_getfield( m_lua, -1, szName );
   
    bool success = lua_isnil( m_lua, -1 ) == 0;

    if ( success )
    {
        lua_pushnil( m_lua );
        lua_setfield( m_lua, -3, szName );
    }

    lua_pop( m_lua, 3 );

    if ( bRecursive && m_pParent )
        if ( m_pParent->DeleteCustomData( szName, true ) )
            success = true;

    return success;
}

bool CClientEntity::GetMatrix ( RwMatrix& matrix ) const
{
    const CEntity* pEntity = GetGameEntity ();
    if ( pEntity )
    {
        pEntity->GetMatrix( matrix );
        return true;
    }

    return false;
}

bool CClientEntity::SetMatrix ( const RwMatrix& matrix )
{
    CEntity * pEntity = GetGameEntity ();
    if ( pEntity )
    {
        pEntity->SetMatrix ( matrix );
        return true;
    }

    return false;
}

void CClientEntity::GetPositionRelative ( CClientEntity * pOrigin, CVector& vecPosition ) const
{
    CVector vecOrigin;
    pOrigin->GetPosition ( vecOrigin );
    GetPosition ( vecPosition );
    vecPosition -= vecOrigin;
}

void CClientEntity::SetPositionRelative ( CClientEntity * pOrigin, const CVector& vecPosition )
{
    CVector vecOrigin;
    pOrigin->GetPosition ( vecOrigin );
    SetPosition ( vecOrigin + vecPosition );
}

bool CClientEntity::IsOutOfBounds ( void )
{
    CVector vecPosition;
    GetPosition ( vecPosition );

    return ( vecPosition.fX < -3000.0f || vecPosition.fX > 3000.0f ||
             vecPosition.fY < -3000.0f || vecPosition.fY > 3000.0f ||
             vecPosition.fZ < -3000.0f || vecPosition.fZ > 3000.0f );
}

void CClientEntity::AttachTo ( CClientEntity* pEntity )
{
    if ( m_pAttachedToEntity )
        m_pAttachedToEntity->RemoveAttachedEntity ( this );

    m_pAttachedToEntity = pEntity;

    if ( m_pAttachedToEntity )
        m_pAttachedToEntity->AddAttachedEntity ( this );

    InternalAttachTo ( pEntity );
}

void CClientEntity::InternalAttachTo ( CClientEntity* pEntity )
{
    CPhysical * pThis = dynamic_cast < CPhysical * > ( GetGameEntity () );
    if ( pThis )
    {
        if ( pEntity )
        {
            switch ( pEntity->GetType () )
            {
                case CCLIENTVEHICLE:
                {
                    CVehicle * pGameVehicle = static_cast < CClientVehicle* > ( pEntity )->GetGameVehicle ();
                    if ( pGameVehicle )
                    {
                        pThis->AttachTo( *pGameVehicle, m_vecAttachedPosition, m_vecAttachedRotation );
                    }
                    break;
                }
                case CCLIENTPED:
                case CCLIENTPLAYER:
                {
                    CPlayerPed * pGamePed = static_cast < CClientPed* > ( pEntity )->GetGamePlayer ();
                    if ( pGamePed )
                    {
                        pThis->AttachTo( *pGamePed, m_vecAttachedPosition, m_vecAttachedRotation );                
                    }
                    break;
                }
                case CCLIENTOBJECT:
                {
                    CObject * pGameObject = static_cast < CClientObject* > ( pEntity )->GetGameObject ();
                    if ( pGameObject )
                    {
                        pThis->AttachTo( *pGameObject, m_vecAttachedPosition, m_vecAttachedRotation );
                    }
                    break;
                }
                case CCLIENTPICKUP:
                {
                    CObject * pGameObject = static_cast < CClientPickup * > ( pEntity )->GetGameObject ();
                    if ( pGameObject )
                    {
                        pThis->AttachTo( *pGameObject, m_vecAttachedPosition, m_vecAttachedRotation );
                    }
                    break;
                }
            }
        }
        else
        {
            pThis->DetachFrom( 0, 0, 0, 0 );
        }
    }
}

void CClientEntity::GetAttachedOffsets ( CVector & vecPosition, CVector & vecRotation )
{
    vecPosition = m_vecAttachedPosition;
    vecRotation = m_vecAttachedRotation;
}

void CClientEntity::SetAttachedOffsets ( CVector & vecPosition, CVector & vecRotation )
{
    CPhysical * pThis = dynamic_cast < CPhysical * > ( GetGameEntity () );
    if ( pThis )
    {
        pThis->SetAttachedOffsets ( vecPosition, vecRotation );
    }
    m_vecAttachedPosition = vecPosition;
    m_vecAttachedRotation = vecRotation;
}

bool CClientEntity::AddEvent ( CLuaMain* pLuaMain, const char* szName, const LuaFunctionRef& iLuaFunction, bool bPropagated )
{
    return m_pEventManager->Add ( pLuaMain, szName, iLuaFunction, bPropagated );
}

bool CClientEntity::CallEvent ( const char* szName, const CLuaArguments& Arguments, bool bCallOnChildren )
{
    CEvents* pEvents = g_pClientGame->GetEvents();

    // Make sure our event-manager knows we're about to call an event
    pEvents->PrePulse();

    // Call the event on our parents/us first
    CallParentEvent ( szName, Arguments, this );

    if ( bCallOnChildren )
    {
        // Call it on all our children
        CallEventNoParent ( szName, Arguments, this );
    }

    // Tell the event manager that we're done calling the event
    pEvents->PostPulse();

    // Return whether it got cancelled or not
    return ( !pEvents->WasCancelled () );
}

void CClientEntity::CallEventNoParent ( const char* szName, const CLuaArguments& Arguments, CClientEntity* pSource )
{
    // Reference ourselves so we cannot be deleted during eventcalls
    lua_class_reference ref;
    Reference( ref );

    // Call it on us if this isn't the same class it was raised on
    //TODO not sure why the null check is necessary (eAi)
    if ( pSource != this && m_pEventManager != NULL )
    {
        m_pEventManager->Call ( szName, Arguments, pSource, this );
    }

    // Call it on all our children
    if ( !m_Children.empty () )
    {
        CChildListType ::const_iterator iter = m_Children.begin ();

        for ( ; iter != m_Children.end (); iter++ )
            (*iter)->CallEventNoParent ( szName, Arguments, pSource );
    }
}

void CClientEntity::CallParentEvent ( const char* szName, const CLuaArguments& Arguments, CClientEntity* pSource )
{
    // Call the event on us
    if ( m_pEventManager )
    {
        m_pEventManager->Call ( szName, Arguments, pSource, this );
    }

    // Call parent's handler
    if ( m_pParent )
    {
        m_pParent->CallParentEvent ( szName, Arguments, pSource );
    }
}

bool CClientEntity::DeleteEvent ( CLuaMain* pLuaMain, const char* szName, const LuaFunctionRef& iLuaFunction )
{
    return m_pEventManager->Delete ( pLuaMain, szName, iLuaFunction );
}

void CClientEntity::DeleteEvents ( CLuaMain* pLuaMain, bool bRecursive )
{
    // Delete it from our events
    m_pEventManager->Delete ( pLuaMain );

    // Delete it from all our children's events
    if ( bRecursive )
    {
        CChildListType ::const_iterator iter = m_Children.begin ();
        for ( ; iter != m_Children.end (); iter++ )
        {
            (*iter)->DeleteEvents ( pLuaMain, true );
        }
    }
}

void CClientEntity::DeleteAllEvents ( void )
{
    m_pEventManager->DeleteAll ();
}

void CClientEntity::CleanUpForVM ( CLuaMain* pLuaMain, bool bRecursive )
{
    // Delete all our events and custom datas attached to that VM
    DeleteEvents ( pLuaMain, false );

    // If recursive, do it on our children too
    if ( bRecursive )
    {
        CChildListType ::const_iterator iter = m_Children.begin ();
        for ( ; iter != m_Children.end (); iter++ )
        {
            (*iter)->CleanUpForVM ( pLuaMain, true );
        }
    }
}

CClientEntity* CClientEntity::FindChild ( const char* szName, unsigned int uiIndex, bool bRecursive )
{
    // Is it our name?
    unsigned int uiCurrentIndex = 0;
    if ( strcmp ( szName, m_szName ) == 0 )
    {
        if ( uiIndex == 0 )
        {
            return this;
        }
        else
        {
            ++uiCurrentIndex;
        }
    }

    // Find it among our children
    return FindChildIndex ( szName, uiIndex, uiCurrentIndex, bRecursive );
}

CClientEntity* CClientEntity::FindChildIndex ( const char* szName, unsigned int uiIndex, unsigned int& uiCurrentIndex, bool bRecursive )
{
    // Look among our children
    CChildListType ::const_iterator iter = m_Children.begin ();
    for ( ; iter != m_Children.end (); iter++ )
    {
        CClientEntity* pChild = *iter;
        // Name matches?
        if ( strcmp ( pChild->GetName (), szName ) == 0 )
        {
            // Does the index match? If it doesn't, increment it and keep searching
            if ( uiIndex == uiCurrentIndex )
            {
                return pChild;
            }
            else
            {
                ++uiCurrentIndex;
            }
        }

        // Tell this child to search too if recursive
        if ( bRecursive )
        {
            CClientEntity* pEntity = pChild->FindChildIndex ( szName, uiIndex, uiCurrentIndex, true );
            if ( pEntity )
            {
                return pEntity;
            }
        }
    }

    // Doesn't exist within us
    return NULL;
}

CClientEntity* CClientEntity::FindChildByType ( const char* szType, unsigned int uiIndex, bool bRecursive )
{
    // Is it our type?
    unsigned int uiCurrentIndex = 0;
    if ( strcmp ( szType, GetTypeName () ) == 0 )
    {
        if ( uiIndex == 0 )
        {
            return this;
        }
        else
        {
            ++uiCurrentIndex;
        }
    }

    // Find it among our children
    unsigned int uiNameHash = HashString ( szType );
    return FindChildByTypeIndex ( uiNameHash, uiIndex, uiCurrentIndex, bRecursive );
}

CClientEntity* CClientEntity::FindChildByTypeIndex ( unsigned int uiTypeHash, unsigned int uiIndex, unsigned int& uiCurrentIndex, bool bRecursive )
{
    // Look among our children
    CChildListType ::const_iterator iter = m_Children.begin ();
    for ( ; iter != m_Children.end (); iter++ )
    {
        // Name matches?
        if ( (*iter)->GetTypeHash() == uiTypeHash )
        {
            // Does the index match? If it doesn't, increment it and keep searching
            if ( uiIndex == uiCurrentIndex )
            {
                return *iter;
            }
            else
            {
                ++uiCurrentIndex;
            }
        }

        // Tell this child to search too if recursive
        if ( bRecursive )
        {
            CClientEntity* pEntity = (*iter)->FindChildByTypeIndex ( uiTypeHash, uiIndex, uiCurrentIndex, true );
            if ( pEntity )
            {
                return pEntity;
            }
        }
    }

    // Doesn't exist within us
    return NULL;
}

void CClientEntity::FindAllChildrenByType ( const char* szType, lua_State* luaVM, bool bStreamedIn )
{
    // Add all children of the given type to the table
    unsigned int uiIndex = 0;
    unsigned int uiTypeHash = HashString ( szType );

    if ( this == g_pClientGame->GetRootEntity () )
    {
        GetEntitiesFromRoot ( uiTypeHash, luaVM, bStreamedIn );
    }
    else
    {
        FindAllChildrenByTypeIndex ( uiTypeHash, luaVM, uiIndex, bStreamedIn );
    }
}

void CClientEntity::FindAllChildrenByTypeIndex ( unsigned int uiTypeHash, lua_State* luaVM, unsigned int& uiIndex, bool bStreamedIn )
{
    // Our type matches?
    if ( m_uiTypeHash == uiTypeHash )
    {
        // Only streamed in elements?
        if ( !bStreamedIn || !IsStreamingCompatibleClass() || 
             reinterpret_cast < CClientStreamElement* > ( this )->IsStreamedIn() )
        {
            // Add it to the table
            lua_pushnumber ( luaVM, ++uiIndex );
            PushStack( luaVM );
            lua_settable ( luaVM, -3 );
        }
    }

    // Call us on the children
    CChildListType ::const_iterator iter = m_Children.begin ();
    for ( ; iter != m_Children.end (); iter++ )
    {
        (*iter)->FindAllChildrenByTypeIndex ( uiTypeHash, luaVM, uiIndex, bStreamedIn );
    }
}

void CClientEntity::GetChildren ( lua_State* luaVM )
{
    // Add all our children to the table on top of the given lua main's stack
    unsigned int uiIndex = 0;
    CChildListType ::const_iterator iter = m_Children.begin ();
    for ( ; iter != m_Children.end (); iter++ )
    {
        // Add it to the table
        lua_pushnumber ( luaVM, ++uiIndex );
        (*iter)->PushStack( luaVM );
        lua_settable ( luaVM, -3 );
    }
}

bool CClientEntity::CollisionExists ( CClientColShape* pShape )
{
    list < CClientColShape* > ::iterator iter = m_Collisions.begin ();
    for ( ; iter != m_Collisions.end () ; iter++ )
    {
        if ( *iter == pShape )
        {
            return true;
        }
    }
    return false;
}

void CClientEntity::RemoveAllCollisions ( bool bNotify )
{
    if ( !m_Collisions.empty () )
    {
        if ( bNotify )
        {
            list < CClientColShape* > ::iterator iter = m_Collisions.begin ();
            for ( ; iter != m_Collisions.end () ; iter++ )
            {
                (*iter)->RemoveCollider ( this );
            }
        }

        m_Collisions.clear ();
    }
}

bool CClientEntity::IsEntityAttached ( CClientEntity* pEntity )
{
    list < CClientEntity* > ::iterator iter = m_AttachedEntities.begin ();
    for ( ; iter != m_AttachedEntities.end (); iter++ )
    {
        if ( *iter == pEntity )
            return true;
    }

    return false;
}

void CClientEntity::ReattachEntities ( void )
{
    // Jax: this should be called on streamIn/creation

    if ( m_pAttachedToEntity )
    {
        InternalAttachTo ( m_pAttachedToEntity );
    }

    // Reattach any entities attached to us
    list < CClientEntity* > ::iterator iter = m_AttachedEntities.begin ();
    for ( ; iter != m_AttachedEntities.end (); iter++ )
    {
        (*iter)->InternalAttachTo ( this );
    }  
}

bool CClientEntity::IsAttachable()
{
    switch( GetType() )
    {
    case CCLIENTPED:
    case CCLIENTPLAYER:
    case CCLIENTRADARMARKER:
    case CCLIENTVEHICLE:
    case CCLIENTOBJECT:
    case CCLIENTMARKER:
    case CCLIENTPICKUP:
    case CCLIENTSOUND:
    case CCLIENTCOLSHAPE:
        return true;
    }

    return false;
}

bool CClientEntity::IsAttachToable()
{
    switch( GetType() )
    {
    case CCLIENTPED:
    case CCLIENTPLAYER:
    case CCLIENTRADARMARKER:
    case CCLIENTVEHICLE:
    case CCLIENTOBJECT:
    case CCLIENTMARKER:
    case CCLIENTPICKUP:
    case CCLIENTSOUND:
    case CCLIENTCOLSHAPE:
        return true;
    }
    return false;
}

void CClientEntity::DoAttaching()
{
    if ( m_pAttachedToEntity )
    {
        RwMatrix matrix, returnMatrix;
        m_pAttachedToEntity->GetMatrix( matrix );

        AttachedMatrix( matrix, returnMatrix, m_vecAttachedPosition, m_vecAttachedRotation );

        SetMatrix ( returnMatrix );
    }
}

unsigned int CClientEntity::GetTypeID ( const char* szTypeName )
{
    if ( strcmp ( szTypeName, "dummy" ) == 0 )              return CCLIENTDUMMY;
    else if ( strcmp ( szTypeName, "ped" ) == 0 )           return CCLIENTPED;
    else if ( strcmp ( szTypeName, "player" ) == 0 )        return CCLIENTPLAYER;
    else if ( strcmp ( szTypeName, "projectile" ) == 0 )    return CCLIENTPROJECTILE;
    else if ( strcmp ( szTypeName, "vehicle" ) == 0 )       return CCLIENTVEHICLE;
    else if ( strcmp ( szTypeName, "object" ) == 0 )        return CCLIENTOBJECT;
    else if ( strcmp ( szTypeName, "marker" ) == 0 )        return CCLIENTMARKER;
    else if ( strcmp ( szTypeName, "blip" ) == 0 )          return CCLIENTRADARMARKER;
    else if ( strcmp ( szTypeName, "pickup" ) == 0 )        return CCLIENTPICKUP;
    else if ( strcmp ( szTypeName, "radararea" ) == 0 )     return CCLIENTRADARAREA;
    else if ( strcmp ( szTypeName, "sound" ) == 0 )         return CCLIENTSOUND;
    else                                                    return CCLIENTUNKNOWN;
}

bool CClientEntity::IsStatic ( void )
{
    CEntity * pEntity = GetGameEntity ();
    if ( pEntity )
    {
        return (pEntity->IsStatic () == TRUE);
    }
    return false;
}

void CClientEntity::SetStatic ( bool bStatic )
{
    CEntity * pEntity = GetGameEntity ();
    if ( pEntity )
    {
        pEntity->SetStatic ( bStatic );
    }
}

bool CClientEntity::IsDoubleSided ( void )
{
    CEntity* pEntity = GetGameEntity ();
    if ( pEntity )
    {
        m_bDoubleSidedInit = true;
        m_bDoubleSided = !pEntity->IsBackfaceCulled ();
    }
    return m_bDoubleSided;
}

void CClientEntity::SetDoubleSided ( bool bDoubleSided )
{
    CEntity* pEntity = GetGameEntity ();
    if ( pEntity )
    {
        pEntity->SetBackfaceCulled ( !bDoubleSided );
    }
    m_bDoubleSidedInit = true;
    m_bDoubleSided = bDoubleSided;
}

unsigned char CClientEntity::GetInterior ( void )
{
    CEntity * pEntity = GetGameEntity ();
    if ( pEntity )
    {
        return pEntity->GetAreaCode ();
    }
    return m_ucInterior;
}

void CClientEntity::SetInterior ( unsigned char ucInterior )
{
    CEntity * pEntity = GetGameEntity ();
    if ( pEntity )
    {
        pEntity->SetAreaCode ( ucInterior );
    }
    m_ucInterior = ucInterior;
}

bool CClientEntity::IsOnScreen ( void )
{
    CEntity * pEntity = GetGameEntity ();
    if ( pEntity )
    {
        return pEntity->IsOnScreen ();
    }
    return false;
}

// Entities from root optimization for getElementsByType
typedef CIntrusiveListExt < CClientEntity, &CClientEntity::m_FromRootNode > CFromRootListType;
typedef google::dense_hash_map < unsigned int, CFromRootListType > t_mapEntitiesFromRoot;
static t_mapEntitiesFromRoot    ms_mapEntitiesFromRoot;
static bool                     ms_bEntitiesFromRootInitialized = false;

void CClientEntity::StartupEntitiesFromRoot ()
{
    if ( !ms_bEntitiesFromRootInitialized )
    {
        ms_mapEntitiesFromRoot.set_deleted_key ( (unsigned int)0x00000000 );
        ms_mapEntitiesFromRoot.set_empty_key ( (unsigned int)0xFFFFFFFF );
        ms_bEntitiesFromRootInitialized = true;
    }
}

// Returns true if top parent is root
bool CClientEntity::IsFromRoot ( CClientEntity* pEntity )
{
    if ( !pEntity )
        return false;
    if ( pEntity == g_pClientGame->GetRootEntity() )
        return true;
    return CClientEntity::IsFromRoot ( pEntity->GetParent () );
}

void CClientEntity::AddEntityFromRoot ( unsigned int uiTypeHash, CClientEntity* pEntity, bool bDebugCheck )
{
    // Insert into list
    CFromRootListType& listEntities = ms_mapEntitiesFromRoot [ uiTypeHash ];
    listEntities.remove ( pEntity );
    listEntities.push_front ( pEntity );

    // Apply to child elements as well
    CChildListType ::const_iterator iter = pEntity->IterBegin ();
    for ( ; iter != pEntity->IterEnd (); iter++ )
        CClientEntity::AddEntityFromRoot ( (*iter)->GetTypeHash (), *iter, false );

#if CHECK_ENTITIES_FROM_ROOT
    if ( bDebugCheck )
        _CheckEntitiesFromRoot ( uiTypeHash );
#endif
}

void CClientEntity::RemoveEntityFromRoot ( unsigned int uiTypeHash, CClientEntity* pEntity )
{
    // Remove from list
    t_mapEntitiesFromRoot::iterator find = ms_mapEntitiesFromRoot.find ( uiTypeHash );
    if ( find != ms_mapEntitiesFromRoot.end () )
    {
        CFromRootListType& listEntities = find->second;
        listEntities.remove ( pEntity );
        if ( listEntities.size () == 0 )
            ms_mapEntitiesFromRoot.erase ( find );
    }

    // Apply to child elements as well
    CChildListType ::const_iterator iter = pEntity->IterBegin ();
    for ( ; iter != pEntity->IterEnd (); iter++ )
        CClientEntity::RemoveEntityFromRoot ( (*iter)->GetTypeHash (), *iter );
}

void CClientEntity::GetEntitiesFromRoot ( unsigned int uiTypeHash, lua_State* luaVM, bool bStreamedIn )
{
#if CHECK_ENTITIES_FROM_ROOT
    _CheckEntitiesFromRoot ( uiTypeHash );
#endif

    t_mapEntitiesFromRoot::iterator find = ms_mapEntitiesFromRoot.find ( uiTypeHash );
    if ( find != ms_mapEntitiesFromRoot.end () )
    {
        CFromRootListType& listEntities = find->second;
        CClientEntity* pEntity;
        unsigned int uiIndex = 0;

        for ( CFromRootListType::reverse_iterator i = listEntities.rbegin ();
              i != listEntities.rend ();
              ++i )
        {
            pEntity = *i;

            // Only streamed in elements?
            if ( !bStreamedIn || !pEntity->IsStreamingCompatibleClass() || 
                 reinterpret_cast < CClientStreamElement* > ( pEntity )->IsStreamedIn() )
            {
                // Add it to the table
                lua_pushnumber ( luaVM, ++uiIndex );
                pEntity->PushStack( luaVM );
                lua_settable ( luaVM, -3 );
            }
        }
    }    
}


#if CHECK_ENTITIES_FROM_ROOT

//
// Check that GetEntitiesFromRoot produces the same results as FindAllChildrenByTypeIndex on the root element
//
void CClientEntity::_CheckEntitiesFromRoot ( unsigned int uiTypeHash )
{
    std::map < CClientEntity*, int > mapResults1;
    g_pClientGame->GetRootEntity()->_FindAllChildrenByTypeIndex ( uiTypeHash, mapResults1 );

    std::map < CClientEntity*, int > mapResults2;
    _GetEntitiesFromRoot ( uiTypeHash, mapResults2 );

    std::map < CClientEntity*, int > :: const_iterator iter1 = mapResults1.begin ();
    std::map < CClientEntity*, int > :: const_iterator iter2 = mapResults2.begin ();

    for ( ; iter1 != mapResults1.end (); ++iter1 )
    {
        CClientEntity* pElement1 = iter1->first;

        if ( mapResults2.find ( pElement1 ) == mapResults2.end () )
        {
            OutputDebugString ( SString ( "Client: 0x%08x  %s is missing from GetEntitiesFromRoot list\n", pElement1, pElement1->GetTypeName () ) );
        }
    }

    for ( ; iter2 != mapResults2.end (); ++iter2 )
    {
        CClientEntity* pElement2 = iter2->first;

        if ( mapResults1.find ( pElement2 ) == mapResults1.end () )
        {
            OutputDebugString ( SString ( "Client: 0x%08x  %s is missing from FindAllChildrenByTypeIndex list\n", pElement2, pElement2->GetTypeName () ) );
        }
    }

    assert ( mapResults1 == mapResults2 );
}

void CClientEntity::_FindAllChildrenByTypeIndex ( unsigned int uiTypeHash, std::map < CClientEntity*, int >& mapResults )
{
    // Our type matches?
    if ( uiTypeHash == m_uiTypeHash )
    {
        // Add it to the table
        assert ( mapResults.find ( this ) == mapResults.end () );
        mapResults [ this ] = 1;

        assert ( this );
        ElementID ID = this->GetID ();
        assert ( ID != INVALID_ELEMENT_ID );
        assert ( this == CElementIDs::GetElement ( ID ) );
        if ( this->IsBeingDeleted () )
            OutputDebugString ( SString ( "Client: 0x%08x  %s is flagged as IsBeingDeleted() but is still in FindAllChildrenByTypeIndex\n", this, this->GetTypeName () ) );
    }

    // Call us on the children
    CChildListType ::const_iterator iter = m_Children.begin ();
    for ( ; iter != m_Children.end (); iter++ )
    {
        (*iter)->_FindAllChildrenByTypeIndex ( uiTypeHash, mapResults );
    }
}

void CClientEntity::_GetEntitiesFromRoot ( unsigned int uiTypeHash, std::map < CClientEntity*, int >& mapResults )
{
    t_mapEntitiesFromRoot::iterator find = ms_mapEntitiesFromRoot.find ( uiTypeHash );
    if ( find != ms_mapEntitiesFromRoot.end () )
    {
        CFromRootListType& listEntities = find->second;
        CClientEntity* pEntity;
        unsigned int uiIndex = 0;

        for ( CFromRootListType::const_reverse_iterator i = listEntities.rbegin ();
              i != listEntities.rend ();
              ++i )
        {
            pEntity = *i;

            assert ( pEntity );
            ElementID ID = pEntity->GetID ();
            assert ( ID != INVALID_ELEMENT_ID );
            assert ( pEntity == CElementIDs::GetElement ( ID ) );
            if ( pEntity->IsBeingDeleted () )
                OutputDebugString ( SString ( "Client: 0x%08x  %s is flagged as IsBeingDeleted() but is still in GetEntitiesFromRoot\n", pEntity, pEntity->GetTypeName () ) );

            assert ( mapResults.find ( pEntity ) == mapResults.end () );
            mapResults [ pEntity ] = 1;
        }
    }    
}

#endif

// TODO: fix crash if game entity is not present
bool CClientEntity::IsCollidableWith( CClientEntity *entity ) const
{
    return GetGameEntity()->IsCollidableWith( entity->GetGameEntity() );
}

void CClientEntity::SetCollidableWith( CClientEntity *entity, bool enable )
{
    GetGameEntity()->SetCollidableWith( entity->GetGameEntity(), enable );
}

CSphere CClientEntity::GetWorldBoundingSphere()
{
    // Default to a point around the entity's position
    CVector vecPosition;
    GetPosition ( vecPosition );
    return CSphere ( vecPosition, 0.f );
}

void CClientEntity::UpdateSpatialData ( void )
{
    GetClientSpatialDatabase ()->UpdateEntity ( this );
}

// Return the distance to the other entity.
// A negative value indicates overlapping bounding spheres
float CClientEntity::GetDistanceBetweenBoundingSpheres ( CClientEntity* pOther )
{
    CSphere sphere = GetWorldBoundingSphere ();
    CSphere otherSphere = pOther->GetWorldBoundingSphere ();
    return ( sphere.vecPosition - otherSphere.vecPosition ).Length () - sphere.fRadius - otherSphere.fRadius;
}
