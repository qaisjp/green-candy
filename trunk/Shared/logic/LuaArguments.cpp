/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*               (Shared logic for modifications)
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        Shared/logic/LuaArguments.cpp
*  PURPOSE:     Lua arguments manager class header
*  DEVELOPERS:  Ed Lyons <eai@opencoding.net>
*               Jax <>
*               Oliver Brown <>
*               Kevin Whiteside <kevuwk@gmail.com>
*               Christian Myhre Lundheim <>
*               Chris McArthur <>
*               Cecill Etheredge <ijsf@gmx.net>
*               The_GTA <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#include "StdInc.h"
#define DECLARE_PROFILER_SECTION_CLuaArguments
#include "profiler/SharedUtil.Profiler.h"

#ifndef VERIFY_ENTITY
#define VERIFY_ENTITY(entity) (CStaticFunctionDefinitions::GetRootElement()->IsMyChild(entity,true)&&!entity->IsBeingDeleted())
#endif


LuaArguments::LuaArguments()
{
}

LuaArguments::LuaArguments( NetBitStreamInterface& stream )
{
    ReadFromBitStream( stream );
}

LuaArguments::LuaArguments( const LuaArguments& args )
{
    // Copy all the arguments
    CopyRecursive( args );
}

LuaArguments::~LuaArguments()
{
    DeleteArguments();
}

LuaArgument* LuaArguments::operator [] ( unsigned int pos ) const
{
    if ( pos >= m_args.size() )
        return NULL;

    return m_args.at( pos );
}

const LuaArguments& LuaArguments::operator = ( const LuaArguments& args )
{
    CopyRecursive( args );

    // Return the given reference allowing for chaining
    return args;
}

void LuaArguments::CopyRecursive( const LuaArguments& args, std::map <LuaArguments*, LuaArguments*>& tables )
{
    // We require a clean setup
    DeleteArguments();

    tables.insert( std::make_pair( (LuaArguments*)&args, (LuaArguments*)this ) );

    // Copy all the arguments
    vector <LuaArgument*>::const_iterator iter = args.m_args.begin();

    for ( ; iter != args.m_args.end(); iter++ )
        m_args.push_back( new LuaArgument( **iter, tables ) );
}

void LuaArguments::ReadArguments( lua_State* luaVM, int indexStart )
{
    // Tidy up setup :)
    DeleteArguments();

    std::map <const void*, LuaArguments*> tables;

    // Start reading arguments until there are none left
    while ( lua_type( luaVM, indexStart ) != LUA_TNONE )
        m_args.push_back( new LuaArgument( luaVM, indexStart++, tables ) );
}

void LuaArguments::ReadTable( lua_State* luaVM, int indexStart, std::map <const void*, LuaArguments*>& tables )
{
    tables.insert( std::make_pair( lua_topointer( luaVM, indexStart ), this ) );

    // Tidy up
    DeleteArguments();

    LUA_CHECKSTACK( luaVM, 2 );

    lua_pushnil( luaVM );  /* first key */

    if ( indexStart < 0 )
        indexStart--;

    // The_GTA: Google for lua table iteration
    while ( lua_next( luaVM, indexStart ) != 0 )
    {
        m_args.push_back( new CLuaArgument( luaVM, -2, tables ) );
        m_args.push_back( new CLuaArgument( luaVM, -1, tables ) );
       
        lua_pop( luaVM, 1 );
    }
}

void LuaArguments::ReadArgument( lua_State* luaVM, int index )
{
    m_args.push_back( new CLuaArgument( luaVM, index ) );
}

void LuaArguments::PushArguments( lua_State* luaVM ) const
{
    // Push all our arguments
    std::vector <LuaArgument*>::const_iterator iter = m_args.begin();

    for ( ; iter != m_args.end(); iter++ )
        (*iter)->Push( luaVM );
}

void LuaArguments::PushAsTable( lua_State* luaVM, std::map <LuaArguments*, int>& tables )
{
    // Ensure there is enough space on the Lua stack
    LUA_CHECKSTACK ( luaVM, 4 );

    bool bKnownTablesCreated = false;
    if ( !pKnownTables )
    {
        pKnownTables = new std::map < CLuaArguments*, int > ();
        bKnownTablesCreated = true;

		lua_newtable ( luaVM );
		// using registry to make it fail safe, else we'd have to carry
		// either lua top or current depth variable between calls
		lua_setfield ( luaVM, LUA_REGISTRYINDEX, "cache" );
    }

    lua_newtable ( luaVM );

	// push it onto the known tables
	int size = pKnownTables->size();
	lua_getfield ( luaVM, LUA_REGISTRYINDEX, "cache" );
	lua_pushnumber ( luaVM, ++size );
	lua_pushvalue ( luaVM, -3 );
	lua_settable ( luaVM, -3 );
	lua_pop ( luaVM, 1 );
    pKnownTables->insert ( std::make_pair ( (CLuaArguments *)this, size ) );

    vector < CLuaArgument* > ::const_iterator iter = m_Arguments.begin ();
    for ( ; iter != m_Arguments.end () && (iter+1) != m_Arguments.end (); iter ++ )
    {
        (*iter++)->Push( luaVM, pKnownTables ); // index
        (*iter)->Push( luaVM, pKnownTables ); // value
        lua_settable( luaVM, -3 );
    }

    if ( bKnownTablesCreated )
	{
		// clear the cache
		lua_pushnil ( luaVM );
		lua_setfield ( luaVM, LUA_REGISTRYINDEX, "cache" );
        delete pKnownTables;
	}
}

void CLuaArguments::PushArguments ( CLuaArguments& Arguments )
{
    vector < CLuaArgument* > ::const_iterator iter = Arguments.IterBegin ();
    for ( ; iter != Arguments.IterEnd (); iter++ )
    {
        CLuaArgument* pArgument = new CLuaArgument ( **iter );
        m_Arguments.push_back ( pArgument );
    }
}

bool CLuaArguments::Call ( CLuaMain* pLuaMain, const CLuaFunctionRef& iLuaFunction, CLuaArguments * returnValues ) const
{
    assert ( pLuaMain );
    TIMEUS startTime = GetTimeUs ();

    // Add the function name to the stack and get the event from the table
    lua_State* luaVM = pLuaMain->GetVirtualMachine ();
    assert ( luaVM );
    LUA_CHECKSTACK ( luaVM, 1 );
    int luaStackPointer = lua_gettop ( luaVM );
    lua_getref ( luaVM, iLuaFunction.ToInt () );

    // Push our arguments onto the stack
    PushArguments ( luaVM );

    // Call the function with our arguments
    pLuaMain->ResetInstructionCount ();

    int iret = lua_pcall ( luaVM, m_Arguments.size (), LUA_MULTRET, 0 );
    if ( iret == LUA_ERRRUN || iret == LUA_ERRMEM )
    {
        SString strRes = ConformResourcePath ( lua_tostring( luaVM, -1 ) );
        
        vector <SString> vecSplit;
        strRes.Split ( ":", vecSplit );
        
        if ( vecSplit.size ( ) >= 3 )
        {
            SString strFile = vecSplit[0];
            int     iLine   = atoi ( vecSplit[1].c_str ( ) );
            SString strMsg  = vecSplit[2].substr ( 1 );
            
            g_pClientGame->GetScriptDebugging()->LogError ( strFile, iLine, strMsg );
        }
        else
            g_pClientGame->GetScriptDebugging()->LogError ( luaVM, "%s", strRes.c_str () );

        // cleanup the stack
        while ( lua_gettop ( luaVM ) - luaStackPointer > 0 )
            lua_pop ( luaVM, 1 );

        return false; // the function call failed
    }
    else
    {
        int iReturns = lua_gettop ( luaVM ) - luaStackPointer;

        if ( returnValues != NULL )
        {
            for ( int i = - iReturns; i <= -1; i++ )
            {
                returnValues->ReadArgument ( luaVM, i );
            }
        }

        // cleanup the stack
        while ( lua_gettop ( luaVM ) - luaStackPointer > 0 )
            lua_pop ( luaVM, 1 );
    }
        
    CClientPerfStatLuaTiming::GetSingleton ()->UpdateLuaTiming ( pLuaMain, pLuaMain->GetFunctionTag ( iLuaFunction.m_iFunction ), GetTimeUs() - startTime );
    return true;
}


bool CLuaArguments::CallGlobal ( CLuaMain* pLuaMain, const char* szFunction, CLuaArguments * returnValues ) const
{
    assert ( pLuaMain );
    assert ( szFunction );
    TIMEUS startTime = GetTimeUs ();

    // Add the function name to the stack and get the event from the table
    lua_State* luaVM = pLuaMain->GetVirtualMachine ();
    assert ( luaVM );
    LUA_CHECKSTACK ( luaVM, 1 );
    int luaStackPointer = lua_gettop ( luaVM );
    lua_pushstring ( luaVM, szFunction );
    lua_gettable ( luaVM, LUA_GLOBALSINDEX );

    // Push our arguments onto the stack
    PushArguments ( luaVM );

    // Call the function with our arguments
    pLuaMain->ResetInstructionCount ();

    int iret = lua_pcall ( luaVM, m_Arguments.size (), LUA_MULTRET, 0 );
    if ( iret == LUA_ERRRUN || iret == LUA_ERRMEM )
    {
        std::string strRes = ConformResourcePath ( lua_tostring( luaVM, -1 ) );
        g_pClientGame->GetScriptDebugging()->LogError ( luaVM, "%s", strRes.c_str () );

        // cleanup the stack
        while ( lua_gettop ( luaVM ) - luaStackPointer > 0 )
            lua_pop ( luaVM, 1 );

        return false; // the function call failed
    }
    else
    {
        int iReturns = lua_gettop ( luaVM ) - luaStackPointer;

        if ( returnValues != NULL )
        {
            for ( int i = - iReturns; i <= -1; i++ )
            {
                returnValues->ReadArgument ( luaVM, i );
            }
        }

        // cleanup the stack
        while ( lua_gettop ( luaVM ) - luaStackPointer > 0 )
            lua_pop ( luaVM, 1 );
    }
        
    CClientPerfStatLuaTiming::GetSingleton ()->UpdateLuaTiming ( pLuaMain, szFunction, GetTimeUs() - startTime );
    return true;
}


CLuaArgument* CLuaArguments::PushNil ( void )
{
    CLuaArgument* pArgument = new CLuaArgument;
    m_Arguments.push_back ( pArgument );
    return pArgument;
}


CLuaArgument* CLuaArguments::PushBoolean ( bool bBool )
{
    CLuaArgument* pArgument = new CLuaArgument ( bBool );
    m_Arguments.push_back ( pArgument );
    return pArgument;
}


CLuaArgument* CLuaArguments::PushNumber ( double dNumber )
{
    CLuaArgument* pArgument = new CLuaArgument ( dNumber );
    m_Arguments.push_back ( pArgument );
    return pArgument;
}


CLuaArgument* CLuaArguments::PushString ( const std::string& strString )
{
    CLuaArgument* pArgument = new CLuaArgument ( strString );
    m_Arguments.push_back ( pArgument );
    return pArgument;
}


CLuaArgument* CLuaArguments::PushUserData ( void* pUserData )
{
    CLuaArgument* pArgument = new CLuaArgument ( pUserData );
    m_Arguments.push_back ( pArgument );
    return pArgument;
}


CLuaArgument* CLuaArguments::PushElement ( CClientEntity* pElement )
{
    CLuaArgument* pArgument = new CLuaArgument ( pElement );
    m_Arguments.push_back ( pArgument );
    return pArgument;
}


CLuaArgument* CLuaArguments::PushArgument ( const CLuaArgument& Argument )
{
    CLuaArgument* pArgument = new CLuaArgument ( Argument );
    m_Arguments.push_back ( pArgument );
    return pArgument;
}


CLuaArgument* CLuaArguments::PushTable ( CLuaArguments * table )
{
    CLuaArgument* pArgument = new CLuaArgument (  );
    pArgument->Read(table);
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
