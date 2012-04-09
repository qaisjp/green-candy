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

CLuaArgument* CLuaArguments::PushAccount ( CAccount* pAccount )
{
    CLuaArgument* pArgument = new CLuaArgument;
    pArgument->ReadUserData ( pAccount );
    m_Arguments.push_back ( pArgument );
    return pArgument;
}


CLuaArgument* CLuaArguments::PushResource ( CResource* pResource )
{
    CLuaArgument* pArgument = new CLuaArgument;
    pArgument->ReadUserData ( pResource );
    m_Arguments.push_back ( pArgument );
    return pArgument;
}


CLuaArgument* CLuaArguments::PushTextDisplay ( CTextDisplay* pTextDisplay )
{
    CLuaArgument* pArgument = new CLuaArgument;
    pArgument->ReadUserData ( pTextDisplay );
    m_Arguments.push_back ( pArgument );
    return pArgument;
}


CLuaArgument* CLuaArguments::PushTextItem ( CTextItem* pTextItem )
{
    CLuaArgument* pArgument = new CLuaArgument;
    pArgument->ReadUserData ( pTextItem );
    m_Arguments.push_back ( pArgument );
    return pArgument;
}


CLuaArgument* CLuaArguments::PushTimer ( CLuaTimer* pLuaTimer )
{
    CLuaArgument* pArgument = new CLuaArgument;
    pArgument->ReadUserData ( pLuaTimer );
    m_Arguments.push_back ( pArgument );
    return pArgument;
}


void CLuaArguments::DeleteArguments ( void )
{
    // Delete each item
    vector < CLuaArgument* > ::iterator iter = m_Arguments.begin ();
    for ( ; iter != m_Arguments.end (); iter++ )
    {
        delete *iter;
    }

    // Clear the vector
    m_Arguments.clear ();
}


void CLuaArguments::ValidateTableKeys ( void )
{
    // Iterate over m_Arguments as pairs
    // If first is LUA_TNIL, then remove pair
    vector < CLuaArgument* > ::iterator iter = m_Arguments.begin ();
    for ( ; iter != m_Arguments.end () ; )
    {
        // Check first in pair
        if ( (*iter)->GetType () == LUA_TNIL )
        {
            // Remove pair
            delete *iter;
            iter = m_Arguments.erase ( iter );
            if ( iter != m_Arguments.end () )
            {
                delete *iter;
                iter = m_Arguments.erase ( iter );
            }
            // Check if end
            if ( iter == m_Arguments.end () )
                break;
        }
        else
        {
            // Skip second in pair
            iter++;
            // Check if end
            if ( iter == m_Arguments.end () )
                break;

            iter++;
        }
    }
}


bool CLuaArguments::ReadFromBitStream ( NetBitStreamInterface& bitStream, std::vector < CLuaArguments* > * pKnownTables )
{
    bool bKnownTablesCreated = false;
    if ( !pKnownTables )
    {
        pKnownTables = new std::vector < CLuaArguments* > ();
        bKnownTablesCreated = true;
    }

    unsigned short usNumArgs;
    if ( bitStream.ReadCompressed ( usNumArgs ) )
    {
        pKnownTables->push_back ( this );
        for ( unsigned short us = 0 ; us < usNumArgs ; us++ )
        {
            CLuaArgument* pArgument = new CLuaArgument ( bitStream, pKnownTables );
            m_Arguments.push_back ( pArgument );
        }
    }

    if ( bKnownTablesCreated )
        delete pKnownTables;

    return true;
}


bool CLuaArguments::WriteToBitStream ( NetBitStreamInterface& bitStream, std::map < CLuaArguments*, unsigned long > * pKnownTables ) const
{
    bool bKnownTablesCreated = false;
    if ( !pKnownTables )
    {
        pKnownTables = new std::map < CLuaArguments*, unsigned long > ();
        bKnownTablesCreated = true;
    }

    bool bSuccess = true;
    pKnownTables->insert ( make_pair ( (CLuaArguments *)this, pKnownTables->size () ) );
    bitStream.WriteCompressed ( static_cast < unsigned short > ( m_Arguments.size () ) );
    vector < CLuaArgument* > ::const_iterator iter = m_Arguments.begin ();
    for ( ; iter != m_Arguments.end () ; iter++ )
    {
        CLuaArgument* pArgument = *iter;
        if ( !pArgument->WriteToBitStream ( bitStream, pKnownTables ) )
        {
            bSuccess = false;
        }
    }

    if ( bKnownTablesCreated )
        delete pKnownTables;

    return bSuccess;
}

bool CLuaArguments::WriteToJSONString ( std::string& strJSON, bool bSerialize )
{
    json_object * my_array = WriteToJSONArray ( bSerialize );
    if ( my_array )
    {
        strJSON = json_object_get_string ( my_array );
        json_object_put ( my_array ); // dereference - causes a crash, is actually commented out in the example too
        return true;
    }
    return false;
}

json_object * CLuaArguments::WriteToJSONArray ( bool bSerialize )
{
    json_object * my_array = json_object_new_array();
    vector < CLuaArgument* > ::const_iterator iter = m_Arguments.begin ();
    for ( ; iter != m_Arguments.end () ; iter++ )
    {
        CLuaArgument* pArgument = *iter;
        json_object * object = pArgument->WriteToJSONObject ( bSerialize );
        if ( object )
        {
            json_object_array_add(my_array, object);
        }
        else
        {
            break;
        }
    }
    return my_array;
}

json_object * CLuaArguments::WriteTableToJSONObject ( bool bSerialize, std::map < CLuaArguments*, unsigned long > * pKnownTables )
{
    bool bKnownTablesCreated = false;
    if ( !pKnownTables )
    {
        pKnownTables = new std::map < CLuaArguments*, unsigned long > ();
        bKnownTablesCreated = true;
    }

    pKnownTables->insert ( std::make_pair ( this, pKnownTables->size () ) );

    bool bIsArray = true;
    unsigned int iArrayPos = 1; // lua arrays are 1 based
    vector < CLuaArgument* > ::const_iterator iter = m_Arguments.begin ();
    for ( ; iter != m_Arguments.end () ; iter+=2 )
    {
        CLuaArgument* pArgument = *iter;
        if ( pArgument->GetType() == LUA_TNUMBER )
        {
            double num = pArgument->GetNumber();
            unsigned int iNum = static_cast < unsigned int > ( num );
            if ( num == iNum )
            {
                if ( iArrayPos != iNum ) // check if the value matches its index in the table
                {
                    bIsArray = false;
                    break;
                }
            }
            else
            {
                bIsArray = false;
                break;
            }
        }
        else
        {
            bIsArray = false;
            break;
        }
        iArrayPos++;
    }
    
    if ( bIsArray )
    {
        json_object * my_array = json_object_new_array();
        vector < CLuaArgument* > ::const_iterator iter = m_Arguments.begin ();
        for ( ; iter != m_Arguments.end () ; iter++ ) 
        {
            iter++; // skip the key values
            CLuaArgument* pArgument = *iter;
            json_object * object = pArgument->WriteToJSONObject ( bSerialize, pKnownTables );
            if ( object )
            {
                json_object_array_add(my_array, object);
            }
            else
            {
                break;
            }
        }
        if ( bKnownTablesCreated )
            delete pKnownTables;
        return my_array;
    }
    else
    {
        json_object * my_object = json_object_new_object();
        iter = m_Arguments.begin ();
        for ( ; iter != m_Arguments.end () ; iter++ )
        {
            char szKey[255];
            szKey[0] = '\0';
            CLuaArgument* pArgument = *iter;
            if ( !pArgument->WriteToString(szKey, 255) ) // index
                break;
            iter++;
            pArgument = *iter;
            json_object * object = pArgument->WriteToJSONObject ( bSerialize, pKnownTables ); // value

            if ( object )
            {
                json_object_object_add(my_object, szKey, object);
            }
            else
            {            
                break;
            }
        }
        if ( bKnownTablesCreated )
            delete pKnownTables;
        return my_object;
    }
}


bool CLuaArguments::ReadFromJSONString ( const char* szJSON )
{
    json_object* object = json_tokener_parse ( szJSON );
    if ( !is_error(object) )
    {
        if ( json_object_get_type ( object ) == json_type_array )
        {
            bool bSuccess = true;

            std::vector < CLuaArguments* > knownTables;
            
            for(int i=0; i < json_object_array_length(object); i++) 
            {
                json_object *arrayObject = json_object_array_get_idx(object, i);
                CLuaArgument * pArgument = new CLuaArgument();
                bSuccess = pArgument->ReadFromJSONObject ( arrayObject, &knownTables );
                m_Arguments.push_back ( pArgument ); // then the value
                if ( !bSuccess )
                    break;
            }
            json_object_put ( object ); // dereference
            return bSuccess;
        }
        json_object_put ( object ); // dereference
    }
//    else
//        g_pGame->GetScriptDebugging()->LogError ( "Could not parse invalid JSON object.");
 //   else
//        g_pGame->GetScriptDebugging()->LogError ( "Could not parse HTTP POST request, ensure data uses JSON.");
    return false;
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
