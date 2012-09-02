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
*               The_GTA <quiret@gmx.de>
*
*****************************************************************************/

#include "StdInc.h"
#define DECLARE_PROFILER_SECTION_CLuaArguments
#include "profiler/SharedUtil.Profiler.h"

extern CClientGame* g_pClientGame;

CLuaArguments::CLuaArguments() : LuaArguments()
{
}

CLuaArguments::CLuaArguments( NetBitStreamInterface& stream )
{
    ReadFromBitStream( stream );
}

void CLuaArguments::ReadArgument( lua_State *L, int idx, luaArgRep_t *cached )
{
    m_args.push_back( new CLuaArgument( this, L, idx, cached ) );
}

LuaArgument* CLuaArguments::PushNil()
{
    CLuaArgument *arg = new CLuaArgument;
    arg->m_parent = this;

    m_args.push_back( arg );
    return arg;
}

LuaArgument* CLuaArguments::PushBoolean( bool b )
{
    CLuaArgument *arg = new CLuaArgument( b );
    arg->m_parent = this;

    m_args.push_back( arg );
    return arg;
}

LuaArgument* CLuaArguments::PushNumber( double d )
{
    CLuaArgument *arg = new CLuaArgument( d );
    arg->m_parent = this;

    m_args.push_back( arg );
    return arg;
}

LuaArgument* CLuaArguments::PushString( const std::string& str )
{
    CLuaArgument *arg = new CLuaArgument( str );
    arg->m_parent = this;

    m_args.push_back( arg );
    return arg;
}

LuaArgument* CLuaArguments::PushTableRef( unsigned int idx )
{
    CLuaArgument *arg = new CLuaArgument;
    arg->m_parent = this;

    arg->m_type = LUA_TTABLEREF;
    arg->m_table = m_cachedTables.at( idx );
    return arg;
}

LuaArgument* CLuaArguments::PushUserData( void *ud )
{
    CLuaArgument *arg = new CLuaArgument( ud );
    arg->m_parent = this;

    m_args.push_back( arg );
    return arg;
}

LuaArgument* CLuaArguments::PushArgument( const LuaArgument& other )
{
    CLuaArgument *arg = (CLuaArgument*)other.Clone();
    arg->m_parent = this;

    m_args.push_back( arg );
    return arg;
}

#if 0
LuaArgument* CLuaArguments::PushTable( const LuaArguments& table )
{
    CLuaArgument *arg = new CLuaArgument();
    arg->Read( &table );
    m_args.push_back( arg );
    return arg;
}
#endif

CLuaArgument* CLuaArguments::PushElement( CClientEntity* element )
{
    CLuaArgument *arg = new CLuaArgument( element );
    arg->m_parent = this;

    m_args.push_back( arg );
    return arg;
}

bool CLuaArguments::ReadFromBitStream( NetBitStreamInterface& bitStream )
{
    unsigned short usNumArgs;

    if ( !bitStream.ReadCompressed( usNumArgs ) )
        return true;

    while ( usNumArgs-- )
        m_args.push_back( new CLuaArgument( this, bitStream ) );

    return true;
}