/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.0
*               (Shared logic for modifications)
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        mods/shared_logic/lua/CLuaArgument.h
*  PURPOSE:     Lua argument class header
*  DEVELOPERS:  Ed Lyons <eai@opencoding.net>
*               Jax <>
*               Oliver Brown <>
*               Kevin Whiteside <kevuwk@gmail.com>
*               Christian Myhre Lundheim <>
*               The_GTA <quiret@gmx.de>
*
*****************************************************************************/

#ifndef __CLUAARGUMENT_H
#define __CLUAARGUMENT_H

#include <net/bitstream.h>
#include <string>

class CClientEntity;
class CLuaArguments;

#define LUA_TTABLEREF 9

class CLuaArgument : public LuaArgument
{
public:
    CLuaArgument() : LuaArgument()
    {
    }

    CLuaArgument( bool b ) : LuaArgument( b )
    {
    }

    CLuaArgument( double d ) : LuaArgument( d )
    {
    }

    CLuaArgument( const std::string& str ) : LuaArgument( str )
    {
    }

    CLuaArgument( void *ud ) : LuaArgument( ud )
    {
    }

    CLuaArgument( NetBitStreamInterface& stream ) : LuaArgument( stream )
    {
    }

    CLuaArgument( lua_State *L, int idx ) : LuaArgument( L, idx )
    {
    }

    CLuaArgument( CClientEntity *element );
    ~CLuaArgument();

    virtual LuaArgument*    Clone() const;

    void                    ReadEntity( CClientEntity *element );
    CClientEntity*          GetElement() const;

    bool                    WriteToBitStream( NetBitStreamInterface& bitStream ) const;

protected:
    bool                    ReadTypeFromBitStream( NetBitStreamInterface& stream, int type );

    void                    LogUnableToPacketize( const char *msg ) const;
};

#endif
