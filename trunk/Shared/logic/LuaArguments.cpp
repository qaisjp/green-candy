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

#ifndef VERIFY_ENTITY
#define VERIFY_ENTITY(entity) (CStaticFunctionDefinitions::GetRootElement()->IsMyChild(entity,true)&&!entity->IsBeingDeleted())
#endif


LuaArguments::LuaArguments()
{
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

void LuaArguments::CopyRecursive( const LuaArguments& args )
{
    // We require a clean setup
    DeleteArguments();

    tables.insert( std::make_pair( (LuaArguments*)&args, (LuaArguments*)this ) );

    // Copy all the arguments
    vector <LuaArgument*>::const_iterator iter = args.m_args.begin();

    for ( ; iter != args.m_args.end(); iter++ )
        m_args.push_back( (*iter)->Clone() );
}

void LuaArguments::ReadArguments( lua_State *luaVM, int indexStart )
{
    // Tidy up setup :)
    DeleteArguments();

    std::map <const void*, LuaArguments*> tables;

    // Start reading arguments until there are none left
    while ( lua_type( luaVM, indexStart ) != LUA_TNONE )
        m_args.push_back( new LuaArgument( luaVM, indexStart++, tables ) );
}

void LuaArguments::ReadTable( lua_State *luaVM, int indexStart )
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
        m_args.push_back( new LuaArgument( luaVM, -2, tables ) );
        m_args.push_back( new LuaArgument( luaVM, -1, tables ) );
       
        lua_pop( luaVM, 1 );
    }
}

void LuaArguments::ReadArgument( lua_State *luaVM, int index )
{
    m_args.push_back( new LuaArgument( luaVM, index ) );
}

void LuaArguments::PushArguments( lua_State *luaVM ) const
{
    // Push all our arguments
    std::vector <LuaArgument*>::const_iterator iter = m_args.begin();

    for ( ; iter != m_args.end(); iter++ )
        (*iter)->Push( luaVM );
}

void LuaArguments::PushAsTable( lua_State *luaVM )
{
    // Ensure there is enough space on the Lua stack
    LUA_CHECKSTACK( luaVM, 4 );

    lua_newtable( luaVM );

	// construct the table
    vector <LuaArgument*>::const_iterator iter = m_args.begin();
    for ( ; iter != m_args.end() && (iter+1) != m_args.end(); iter++ )
    {
        (*iter++)->Push( luaVM ); // index
        (*iter)->Push( luaVM ); // value
        lua_settable( luaVM, -3 );
    }
}

void LuaArguments::PushArguments( LuaArguments& args )
{
    vector <LuaArgument*>::const_iterator iter = args.IterBegin();

    for ( ; iter != args.IterEnd(); iter++ )
        m_args.push_back( (*iter)->Clone() );
}

bool LuaArguments::Call( LuaMain *lua, const LuaFunctionRef& ref, LuaArguments *res )
{
    int top = lua_gettop( lua->GetVirtualMachine() );
    bool excpt;

    lua->PushReference( ref );
    PushArguments( lua->GetVirtualMachine() );

    // Call the function with our arguments
    if ( !res )
        return lua->PCallStackVoid( m_args.size() );

    *res = lua->PCallStackResult( m_args.size(), excpt );
    return !excpt;
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

LuaArgument* LuaArguments::PushNil()
{
    LuaArgument *arg = new LuaArgument;
    m_args.push_back( arg );
    return arg;
}

LuaArgument* LuaArguments::PushBoolean( bool b )
{
    LuaArgument *arg = new LuaArgument( b );
    m_args.push_back( arg );
    return arg;
}

LuaArgument* LuaArguments::PushNumber( double d )
{
    LuaArgument *arg = new LuaArgument( d );
    m_args.push_back( arg );
    return arg;
}

LuaArgument* LuaArguments::PushString( const std::string& str )
{
    LuaArgument *arg = new LuaArgument( str );
    m_args.push_back( arg );
    return arg;
}

LuaArgument* LuaArguments::PushUserData( void *ud )
{
    LuaArgument *arg = new LuaArgument( ud );
    m_args.push_back( arg );
    return arg;
}

LuaArgument* LuaArguments::PushArgument( const LuaArgument& other )
{
    LuaArgument *arg = other.clone();
    m_args.push_back( arg );
    return arg;
}

LuaArgument* LuaArguments::PushTable( const LuaArguments& table )
{
    LuaArgument *arg = new LuaArgument();
    arg->Read( table );
    m_args.push_back( arg );
    return arg;
}

void LuaArguments::DeleteArguments()
{
    // Delete each item
    std::vector <LuaArgument*>::iterator iter = m_args.begin();

    for ( ; iter != m_args.end(); iter++ )
        delete *iter;

    // Clear the vector
    m_args.clear();
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