/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.0
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        mods/deathmatch/logic/lua/CLuaArgument.h
*  PURPOSE:     Lua argument handler class
*  DEVELOPERS:  Ed Lyons <>
*               Christian Myhre Lundheim <>
*               Jax <>
*               The_GTA <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#ifndef __CLUAARGUMENT_H
#define __CLUAARGUMENT_H

#include "../common/CBitStream.h"
#include "json.h"

class CElement;
class CLuaArguments;

#define LUA_TTABLEREF 9

class CLuaArgument : public LuaArgument
{
public:
                            CLuaArgument( CElement* pElement );
                            ~CLuaArgument();

    void                    Read( CElement* pElement );
    CElement*               GetElement() const;

    bool                    ReadFromBitStream( NetBitStreamInterface& bitStream );
    bool                    WriteToBitStream( NetBitStreamInterface& bitStream ) const;
    json_object*            WriteToJSONObject( bool serialize = false );
    bool                    ReadFromJSONObject( json_object* object );
};

#endif
