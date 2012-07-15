/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*               (Shared logic for modifications)
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        mods/shared_logic/lua/CLuaArguments.cpp
*  PURPOSE:     Lua arguments manager class
*  DEVELOPERS:  Ed Lyons <eai@opencoding.net>
*               Jax <>
*               Oliver Brown <>
*               Kevin Whiteside <kevuwk@gmail.com>
*               Christian Myhre Lundheim <>
*               Chris McArthur <>
*               Cecill Etheredge <ijsf@gmx.net>
*
*****************************************************************************/

#include "StdInc.h"
#define DECLARE_PROFILER_SECTION_CLuaArguments
#include "profiler/SharedUtil.Profiler.h"

using namespace std;

#ifndef VERIFY_ENTITY
#define VERIFY_ENTITY(entity) (CStaticFunctionDefinitions::GetRootElement()->IsMyChild(entity,true)&&!entity->IsBeingDeleted())
#endif
 
extern CClientGame* g_pClientGame;

CLuaArguments::CLuaArguments() : LuaArguments()
{
}

CLuaArguments::CLuaArguments( NetBitStreamInterface& stream )
{
    ReadFromBitStream( stream );
}

void CLuaArguments::ReadArgument( lua_State *L, int idx )
{
    m_args.push_back( new CLuaArgument( L, idx ) );
}

LuaArgument* CLuaArguments::PushNil()
{
    CLuaArgument *arg = new CLuaArgument;
    m_args.push_back( arg );
    return arg;
}

LuaArgument* CLuaArguments::PushBoolean( bool b )
{
    CLuaArgument *arg = new CLuaArgument( b );
    m_args.push_back( arg );
    return arg;
}

LuaArgument* CLuaArguments::PushNumber( double d )
{
    CLuaArgument *arg = new CLuaArgument( d );
    m_args.push_back( arg );
    return arg;
}

LuaArgument* CLuaArguments::PushString( const std::string& str )
{
    CLuaArgument *arg = new CLuaArgument( str );
    m_args.push_back( arg );
    return arg;
}

LuaArgument* CLuaArguments::PushUserData( void *ud )
{
    CLuaArgument *arg = new CLuaArgument( ud );
    m_args.push_back( arg );
    return arg;
}

LuaArgument* CLuaArguments::PushArgument( const LuaArgument& other )
{
    CLuaArgument *arg = other.Clone();
    m_args.push_back( arg );
    return arg;
}

LuaArgument* CLuaArguments::PushTable( const LuaArguments& table )
{
    CLuaArgument *arg = new CLuaArgument();
    arg->Read( &table );
    m_args.push_back( arg );
    return arg;
}

CLuaArgument* CLuaArguments::PushElement( CClientEntity* element )
{
    CLuaArgument *arg = new CLuaArgument( element );
    m_args.push_back( arg );
    return arg;
}

bool CLuaArguments::ReadFromBitStream( NetBitStreamInterface& bitStream )
{
    unsigned short usNumArgs;

    if ( !bitStream.ReadCompressed( usNumArgs ) )
        return true;

    while ( usNumArgs-- )
        m_args.push_back( new CLuaArgument( bitStream ) );

    return true;
}

bool CLuaArguments::WriteToBitStream( NetBitStreamInterface& bitStream ) const
{
    std::vector <LuaArgument*>::const_iterator iter = m_args.begin();

    bitStream.WriteCompressed( (unsigned short)m_args.size() ) );

    for ( ; iter != m_args.end(); iter++ )
    {
        if ( !(*iter)->WriteToBitStream( bitStream ) )
            return false;
    }

    return true;
}