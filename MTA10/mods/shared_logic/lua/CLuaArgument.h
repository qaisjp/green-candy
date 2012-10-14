/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
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
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#ifndef __CLUAARGUMENT_H
#define __CLUAARGUMENT_H

#include <net/bitstream.h>

class CClientEntity;
class CLuaArguments;

#define LUA_TTABLEREF   9
#define LUA_TENTITYREF  10

class CLuaArgument : public LuaArgument
{
    friend class CLuaArguments;
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

    CLuaArgument( LuaArguments *parent, NetBitStreamInterface& stream ) : LuaArgument( parent, stream )
    {
        ReadFromBitStream( stream );
    }

    CLuaArgument( LuaArguments *parent, lua_State *L, int idx, luaArgRep_t *cached ) : LuaArgument( parent, L, idx, cached )
    {
    }

    CLuaArgument( CClientEntity *element );
    ~CLuaArgument();

    virtual LuaArgument*    Clone() const;

    void                    ReadEntity( CClientEntity *element );
    CClientEntity*          GetElement() const;

    void                    Push( lua_State *L ) const;

    bool                    WriteToBitStream( NetBitStreamInterface& bitStream, argRep_t *cached = NULL ) const;
    bool                    ReadTypeFromBitStream( NetBitStreamInterface& stream, int type );

protected:
    void                    LogUnableToPacketize( const char *msg ) const;
};

#endif
