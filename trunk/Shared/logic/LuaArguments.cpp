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

LuaArguments::LuaArguments()
{
    m_parent = NULL;
}

LuaArguments::LuaArguments( const LuaArguments& args )
{
    // Copy all the arguments
    CopyRecursive( args );

    m_parent = NULL;
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

void LuaArguments::CopyRecursive( const LuaArguments& args )
{
    // We require a clean setup
    DeleteArguments();

#ifdef _TODO
    tables.insert( std::make_pair( (LuaArguments*)&args, (LuaArguments*)this ) );
#endif

    // Copy all the arguments
    argList_t::const_iterator iter = args.m_args.begin();

    for ( ; iter != args.m_args.end(); iter++ )
        m_args.push_back( (*iter)->Clone() );
}

void LuaArguments::ReadArguments( lua_State *L, int indexStart )
{
    // Tidy up setup :)
    DeleteArguments();

    luaArgRep_t refMap;

    // Start reading arguments until there are none left
    while ( lua_type( L, indexStart ) != LUA_TNONE )
        ReadArgument( L, indexStart++, &refMap );
}

void LuaArguments::ReadTable( lua_State *luaVM, int indexStart, luaArgRep_t *cached )
{
    // Tidy up
    DeleteArguments();

    LUA_CHECKSTACK( luaVM, 2 );

    lua_pushnil( luaVM );  /* first key */

    if ( indexStart < 0 )
        indexStart--;

    // The_GTA: Google for lua table iteration
    while ( lua_next( luaVM, indexStart ) != 0 )
    {
        ReadArgument( luaVM, -2, cached );
        ReadArgument( luaVM, -1, cached );
       
        lua_pop( luaVM, 1 );
    }
}

void LuaArguments::PushArguments( lua_State *luaVM ) const
{
    // Push all our arguments
    std::vector <LuaArgument*>::const_iterator iter = m_args.begin();

    for ( ; iter != m_args.end(); iter++ )
        (*iter)->Push( luaVM );
}

void LuaArguments::PushAsTable( lua_State *L )
{
    // Ensure there is enough space on the Lua stack
    LUA_CHECKSTACK( L, 4 );

    lua_newtable( L );

	// construct the table
    vector <LuaArgument*>::const_iterator iter = m_args.begin();
    for ( ; iter != m_args.end() && (iter+1) != m_args.end(); iter++ )
    {
        (*iter++)->Push( L ); // index
        (*iter)->Push( L ); // value
        lua_settable( L, -3 );
    }
}

void LuaArguments::PushArguments( LuaArguments& args )
{
    std::vector <LuaArgument*>::const_iterator iter = args.IterBegin();

    for ( ; iter != args.IterEnd(); iter++ )
        m_args.push_back( (*iter)->Clone() );
}

bool LuaArguments::Call( LuaMain *lua, const LuaFunctionRef& ref, LuaArguments *res ) const
{
    int top = lua_gettop( lua->GetVirtualMachine() );

    lua->PushReference( ref );
    PushArguments( lua->GetVirtualMachine() );

    // Call the function with our arguments
    if ( !res )
        return lua->PCallStackVoid( m_args.size() );

    return lua->PCallStackResult( m_args.size(), *res );
}

bool LuaArguments::IsIndexedArray()
{
    unsigned int n = 1;
    std::vector <LuaArgument*>::const_iterator iter = m_args.begin ();

    for ( ; iter != m_args.end(); iter+=2, n++ )
    {
        LuaArgument *arg = *iter;

        if ( arg->GetType() != LUA_TNUMBER )
            return false;

        double num = arg->GetNumber();
        unsigned int iNum = (unsigned int)num;

        if ( num != iNum )
            return false;

        if ( n != iNum ) // check if the value matches its index in the table
            return false;
    }

    return true;
}

unsigned int LuaArguments::AddCachedTable( LuaArguments *table )
{
    if ( m_parent )
        return m_parent->AddCachedTable( table );

    table->m_parent = this;

    unsigned int id = m_cachedTables.size();

    // Register the table
    m_cachedTables.push_back( table );
    table->m_cachedID = id;
    return id;
}

LuaArguments* LuaArguments::GetCachedTable( unsigned int idx )
{
    if ( m_parent )
        return m_parent->GetCachedTable( idx );

    if ( idx >= m_cachedTables.size() )
        return NULL;

    return m_cachedTables.at( idx );
}

void LuaArguments::DeleteArguments()
{
    // Delete each item
    argList_t::iterator iter = m_args.begin();

    for ( ; iter != m_args.end(); iter++ )
        delete *iter;

    // Clear the vector
    m_args.clear();

    // Clear cached tables
    if ( !m_parent )
    {
        for ( cached_t::iterator iterc = m_cachedTables.begin(); iterc != m_cachedTables.end(); iterc++ )
            delete *iterc;

        m_cachedTables.clear();
    }
}

void LuaArguments::ValidateTableKeys()
{
    // Remove nil table keys
    std::vector <LuaArgument*>::iterator iter = m_args.begin();

    for ( ; iter != m_args.end(); )
    {
        // Check first in pair
        if ( (*iter)->GetType() == LUA_TNIL )
        {
            // Remove pair
            delete *iter;

            iter = m_args.erase( iter );

            if ( iter != m_args.end() )
            {
                delete *iter;

                iter = m_args.erase( iter );
            }

            // Check if end
            if ( iter == m_args.end() )
                break;
        }
        else
        {
            // Skip second in pair
            iter++;

            // Check if end
            if ( iter == m_args.end() )
                break;

            iter++;
        }
    }
}

#ifndef _KILLFRENZY

bool LuaArguments::WriteToBitStream( NetBitStreamInterface& bitStream, argRep_t *cached ) const
{
    bool success = true;
    std::vector <LuaArgument*>::const_iterator iter = m_args.begin();

    bitStream.WriteCompressed( (unsigned short)m_args.size() );

    if ( !m_parent )
        cached = new argRep_t;

    for ( ; iter != m_args.end(); iter++ )
    {
        if ( !(*iter)->WriteToBitStream( bitStream, cached ) )
            success = false;
    }

    if ( !m_parent )
        delete cached;

    return success;
}

#endif