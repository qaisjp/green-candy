/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.0
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        mods/deathmatch/logic/lua/CLuaArguments.cpp
*  PURPOSE:     Lua argument manager class
*  DEVELOPERS:  Ed Lyons <>
*               Christian Myhre Lundheim <>
*               Jax <>
*               Chris McArthur <>
*               Cecill Etheredge <>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#include "StdInc.h"

#ifndef WIN32
#include <clocale>
#endif

extern CGame* g_pGame;

#ifndef VERIFY_ELEMENT
#define VERIFY_ELEMENT(element) (g_pGame->GetMapManager()->GetRootElement ()->IsMyChild(element,true)&&!element->IsBeingDeleted())
#endif

CLuaArgument* CLuaArguments::PushElement( CElement *element )
{
    CLuaArgument *arg = new CLuaArgument( element );
    m_Arguments.push_back( arg );
    return arg;
}

CLuaArgument* CLuaArguments::PushACL( CAccessControlList *acl )
{
    CLuaArgument *arg = new CLuaArgument;
    arg->ReadUserData( acl );
    m_args.push_back( arg );
    return arg;
}

CLuaArgument* CLuaArguments::PushACLGroup( CAccessControlListGroup *aclGroup )
{
    CLuaArgument *arg = new CLuaArgument;
    arg->ReadUserData( aclGroup );
    m_args.push_back( arg );
    return arg;
}

CLuaArgument* CLuaArguments::PushAccount( CAccount *acc )
{
    CLuaArgument *arg = new CLuaArgument;
    arg->ReadUserData( acc );
    m_args.push_back( arg );
    return arg;
}

CLuaArgument* CLuaArguments::PushResource( CResource *res )
{
    CLuaArgument *arg = new CLuaArgument;
    arg->ReadUserData( res );
    m_args.push_back( arg );
    return arg;
}

CLuaArgument* CLuaArguments::PushTextDisplay( CTextDisplay *disp )
{
    CLuaArgument *arg = new CLuaArgument;
    arg->ReadUserData( disp );
    m_args.push_back( arg );
    return arg;
}

CLuaArgument* CLuaArguments::PushTextItem( CTextItem *item )
{
    CLuaArgument *arg = new CLuaArgument;
    arg->ReadUserData( item );
    m_args.push_back( arg );
    return arg;
}

CLuaArgument* CLuaArguments::PushTimer( CLuaTimer *timer )
{
    CLuaArgument *arg = new CLuaArgument;
    arg->ReadUserData( timer );
    m_args.push_back( arg );
    return arg;
}

bool CLuaArguments::ReadFromBitStream( NetBitStreamInterface& bitStream )
{
    unsigned short usNumArgs;

    if ( !bitStream.ReadCompressed( usNumArgs ) )
        return true;

    while ( usNumArgs-- )
        m_Arguments.push_back( new CLuaArgument( bitStream ) );

    return true;
}

bool CLuaArguments::WriteToJSONString( std::string& json, bool serialize )
{
    json_object *arr = WriteToJSONArray( serialize );

    if ( !arr )
        return false;

    json = json_object_get_string( arr );
    json_object_put( arr ); // dereference - causes a crash, is actually commented out in the example too
    return true;
}

#ifndef VERIFY_ELEMENT
#define VERIFY_ELEMENT(element) (g_pGame->GetMapManager()->GetRootElement ()->IsMyChild(element,true)&&!element->IsBeingDeleted())
#endif

#ifndef VERIFY_RESOURCE
#define VERIFY_RESOURCE(resource) (g_pGame->GetResourceManager()->Exists(resource))
#endif

inline static json_object* json_CreateObjectFromArgument( LuaArgument& arg, bool serialize )
{
    switch( arg->GetType() )
    {
    case LUA_TNIL:
        return json_object_new_int( 0 );
    case LUA_TBOOLEAN:
        return json_object_new_boolean( arg.GetBoolean() );
    case LUA_TTABLE:
#ifdef _TODO
        if ( pKnownTables && pKnownTables->find ( m_pTableData ) != pKnownTables->end () )
        {
            char szTableID[10];
            snprintf ( szTableID, sizeof(szTableID), "^T^%lu", pKnownTables->find ( m_pTableData )->second );
            return json_object_new_string ( szTableID );
        }
        else
        {
            return m_pTableData->WriteTableToJSONObject( bSerialize, pKnownTables );
        }
#endif //_TODO
        break;
    case LUA_TNUMBER:
        int iNum = (int)arg.GetNumber();

        if ( iNum == (float)arg.GetNumber() )
            return json_object_new_int( iNum );

        return json_object_new_double( arg.GetNumber() );
    case LUA_TSTRING:
        if ( arg.GetString().size() > 65535 )
        {
            g_pGame->GetScriptDebugging()->LogError( "Couldn't convert argument list to JSON. Invalid string specified, limit is 65535 characters." );
            return NULL;
        }

        return json_object_new_string_len( arg.GetString().c_str(), arg.GetString().size() );
    case LUA_TLIGHTUSERDATA:
        CElement *pElement = arg.GetElement();
        CResource *pResource = (CResource*)arg.GetLightUserData();
        
        // Elements are dynamic, so storing them is potentially unsafe
        if ( pElement && serialize )
        {
            char szElementID[10];
            snprintf( szElementID, 9, "^E^%d", pElement->GetID().Value() );
            return json_object_new_string( szElementID );
        }
        else if ( VERIFY_RESOURCE( pResource ) )
        {
            char szElementID[ MAX_RESOURCE_NAME_LENGTH + 4 ];
            snprintf( szElementID, MAX_RESOURCE_NAME_LENGTH + 3, "^R^%s", pResource->GetName().c_str() );
            return json_object_new_string( szElementID );
        }

        g_pGame->GetScriptDebugging()->LogError( "Couldn't convert argument list to JSON, only valid elements can be sent." );
        return NULL;
    }

    g_pGame->GetScriptDebugging()->LogError( "Couldn't convert argument list to JSON, unsupported data type. Use Table, Nil, String, Number, Boolean, Resource or Element." );
    return NULL;
}

json_object* CLuaArguments::WriteToJSONArray( bool serialize )
{
    //TODO: add table referencing support
    json_object *arr = json_object_new_array();
    std::vector <LuaArgument*>::const_iterator iter = m_args.begin();

    for ( ; iter != m_args.end() ; iter++ )
    {
        json_object *obj = json_CreateObjectFromArgument( **iter, serialize );

        if ( !obj )
            continue;

        json_object_array_add( arr, obj );
    }
    return arr;
}

json_object* CLuaArguments::WriteTableToJSONObject( bool serialize )
{
    std::vector <LuaArgument*>::iterator iter = m_args.begin();

    if ( IsIndexedArray() )
    {
        json_object *arr = json_object_new_array();

        for ( ; iter != m_args.end(); iter++ ) 
        {
            iter++; // skip the key values

            json_object *obj = json_CreateObjectFromArgument( **iter, serialize );

            if ( !obj )
                break;

            json_object_array_add( arr, obj );
        }

        return arr;
    }

    json_object *dict = json_object_new_object();

    for ( ; iter != m_args.end(); iter++ )
    {
        std::string buf;

        if ( !(*iter)->WriteToString( buf ) ) // index
            break;

        iter++;

        json_object *obj = json_CreateObjectFromArgument( **iter, serialize );

        if ( !obj2 )
            break;

        json_object_object_add( dict, buf.c_str(), obj );
    }

    return dict;
}

inline static CLuaArgument* json_CreateArgumentFromObject( json_object *obj )
{
    if ( is_error( obj ) )
        return NULL;

    if ( !obj )
        return new CLuaArgument; 

    switch( json_object_get_type( obj ) )
    {
    case json_type_null:
        return new CLuaArgument;
    case json_type_boolean:
        return new CLuaArgument( json_object_get_boolean( obj ) == TRUE );
    case json_type_double:
        return new CLuaArgument( json_object_get_double( obj ) );
    case json_type_int:
        return new CLuaArgument( (double)json_object_get_int( object ) );
    case json_type_object:
        m_pTableData = new CLuaArguments();
        m_pTableData->ReadFromJSONObject ( object, pKnownTables );
        m_bWeakTableRef = false;
        m_iType = LUA_TTABLE;
        break;
    case json_type_array:
        m_pTableData = new CLuaArguments();
        m_pTableData->ReadFromJSONArray ( object, pKnownTables );
        m_bWeakTableRef = false;
        m_iType = LUA_TTABLE;
        break;
    case json_type_string:
        {
        char * szString = json_object_get_string ( object );
        if ( strlen(szString) > 3 && szString[0] == '^' && szString[2] == '^' && szString[1] != '^' )
        {
            switch ( szString[1] )
            {
                case 'E': // element
                {
                    int id = atoi ( szString + 3 );
                    CElement * element = NULL;
                    if ( id != INT_MAX && id != INT_MIN && id != 0 )
                        element = CElementIDs::GetElement(id);
                    if ( element )
                    {
                        Read ( element );
                    }
                    else 
                    {
                        // Appears sometimes when a player quits
                        //g_pGame->GetScriptDebugging()->LogError ( NULL, SString ( "Invalid element specified in JSON string '%s'.", szString ) );
                        m_iType = LUA_TNIL;
                    }
                    break;
                }
                case 'R': // resource
                {
                    CResource * resource = g_pGame->GetResourceManager()->GetResource(szString+3);
                    if ( resource )
                    {
                        ReadUserData ((void *)resource);
                    }
                    else 
                    {
                        g_pGame->GetScriptDebugging()->LogError ( NULL, SString ( "Invalid resource specified in JSON string '%s'.", szString ) );
                        m_iType = LUA_TNIL;
                    }
                    break;
                }
                case 'T':   // Table reference
                {
                    unsigned long ulTableID = static_cast < unsigned long > ( atol ( szString + 3 ) );
                    if ( pKnownTables && ulTableID >= 0 && ulTableID < pKnownTables->size () )
                    {
                        m_pTableData = pKnownTables->at ( ulTableID );
                        m_bWeakTableRef = true;
                        m_iType = LUA_TTABLE;
                    }
                    else
                    {
                        g_pGame->GetScriptDebugging()->LogError ( NULL, SString ( "Invalid table reference specified in JSON string '%s'.", szString ) );
                        m_iType = LUA_TNIL;
                    }
                    break;
                }
            }
        }
        else
            Read(std::string ( szString ));
        break;
        }
    default:
        return false;
    }
    return true;
}

bool CLuaArguments::ReadFromJSONString( const char* szJSON )
{
    json_object* object = json_tokener_parse( szJSON );

    if ( is_error(object) )
        return false;

    if ( json_object_get_type( object ) != json_type_array )
    {
        json_object_put( object );
        return false;
    }

    bool bSuccess = true;
    
    for ( int i=0; i < json_object_array_length( object ); i++ ) 
    {
        json_object *arrayObject = json_object_array_get_idx(object, i);
        CLuaArgument *arg = new CLuaArgument();

        bSuccess = pArgument->ReadFromJSONObject( arrayObject );
        m_Arguments.push_back ( pArgument ); // then the value

        if ( !bSuccess )
            break;
    }
    json_object_put ( object ); // dereference
    return bSuccess;
}

bool CLuaArguments::ReadFromJSONObject ( json_object * object, std::vector < CLuaArguments* > * pKnownTables )
{
    if ( !is_error(object) )
    {
        if ( json_object_get_type ( object ) == json_type_object )
        {
            bool bKnownTablesCreated = false;
            if ( !pKnownTables )
            {
                pKnownTables = new std::vector < CLuaArguments* > ();
                bKnownTablesCreated = true;
            }

            pKnownTables->push_back ( this );

            bool bSuccess = true;
            json_object_object_foreach(object, key, val) 
            {
                CLuaArgument* pArgument = new CLuaArgument ( std::string ( key ) );
                m_Arguments.push_back ( pArgument ); // push the key first
                pArgument = new CLuaArgument ( );
                bSuccess = pArgument->ReadFromJSONObject ( val, pKnownTables ); // then the value
                m_Arguments.push_back ( pArgument );
                if ( !bSuccess )
                    break;
            }

            if ( bKnownTablesCreated )
                delete pKnownTables;
            return bSuccess;
        }
    }
 //   else
//        g_pGame->GetScriptDebugging()->LogError ( "Could not parse invalid JSON object.");
    return false;
}

bool CLuaArguments::ReadFromJSONArray ( json_object * object, std::vector < CLuaArguments* > * pKnownTables )
{
    if ( !is_error(object) )
    {
        if ( json_object_get_type ( object ) == json_type_array )
        {
            bool bKnownTablesCreated = false;
            if ( !pKnownTables )
            {
                pKnownTables = new std::vector < CLuaArguments* > ();
                bKnownTablesCreated = true;
            }

            pKnownTables->push_back ( this );

            bool bSuccess = true;
            for(int i=0; i < json_object_array_length(object); i++) 
            {
                json_object *arrayObject = json_object_array_get_idx(object, i);
                CLuaArgument* pArgument = new CLuaArgument ((double)i+1); // push the key
                m_Arguments.push_back ( pArgument );

                pArgument = new CLuaArgument();
                bSuccess = pArgument->ReadFromJSONObject ( arrayObject, pKnownTables );
                m_Arguments.push_back ( pArgument ); // then the valoue
                if ( !bSuccess )
                    break;
            }

            if ( bKnownTablesCreated )
                delete pKnownTables;
            return bSuccess;
        }
    }
//    else
//        g_pGame->GetScriptDebugging()->LogError ( "Could not parse invalid JSON object.");
    return false;
}
