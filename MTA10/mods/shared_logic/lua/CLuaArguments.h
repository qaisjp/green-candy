/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*               (Shared logic for modifications)
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        mods/shared_logic/lua/CLuaArguments.h
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
*****************************************************************************/

#ifndef __CLUAARGUMENTS_H
#define __CLUAARGUMENTS_H

#include <net/bitstream.h>
#include "CLuaArgument.h"
#include <vector>

#if MTA_DEBUG
    // Tight allocation in debug to find trouble.
    #define LUA_CHECKSTACK(vm,space) lua_checkstack(vm, (space) )
#else
    // Extra room in release to avoid trouble.
    #define LUA_CHECKSTACK(vm,space) lua_checkstack(vm, (space)*2 )
#endif

class CLuaArguments : public LuaArguments
{
public:
                                                        CLuaArguments();
                                                        CLuaArguments( NetBitStreamInterface& stream );

    void                                                ReadArgument( lua_State *L, int idx, luaArgRep_t *cached );

    LuaArgument*                                        PushNil();
    LuaArgument*                                        PushBoolean( bool b );
    LuaArgument*                                        PushNumber( double num );
    LuaArgument*                                        PushString( const std::string& str );
    LuaArgument*                                        PushTableRef( unsigned int idx );
    LuaArgument*                                        PushUserData( void *data );
    LuaArgument*                                        PushArgument( const LuaArgument& argument );
    LuaArgument*                                        PushTable( const LuaArguments& table );
    CLuaArgument*                                       PushElement( CClientEntity *element );

    bool                                                ReadFromBitStream( NetBitStreamInterface& bitStream );
};

#endif


