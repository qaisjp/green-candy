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

CLuaArguments::CLuaArguments( NetBitStreamInterface& stream )
{
    ReadFromBitStream( stream );
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
        m_Arguments.push_back( new CLuaArgument( bitStream ) );

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