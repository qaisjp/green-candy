/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*               (Shared logic for modifications)
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        mods/shared_logic/CClientRwObject.cpp
*  PURPOSE:     RenderWare object virtualization
*  DEVELOPERS:  The_GTA <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#include "StdInc.h"

static LUA_DECLARE( setName )
{
    const char *name;

    LUA_ARGS_BEGIN;
    argStream.ReadString( name );
    LUA_ARGS_END;

    ((CClientRwObject*)lua_touserdata( L, lua_upvalueindex( 1 ) ))->SetName( name );
    LUA_SUCCESS;
}

static LUA_DECLARE( getName )
{
    const std::string& name = ((CClientRwObject*)lua_touserdata( L, lua_upvalueindex( 1 ) ))->GetName();

    lua_pushlstring( L, name.c_str(), name.size() );
    return 1;
}

static LUA_DECLARE( getHash )
{
    lua_pushnumber( L, ((CClientRwObject*)lua_touserdata( L, lua_upvalueindex( 1 ) ))->GetHash() );
    return 1;
}

static luaL_Reg object_interface[] =
{
    LUA_METHOD( setName ),
    LUA_METHOD( getName ),
    LUA_METHOD( getHash ),
    { NULL, NULL }
};

static LUA_DECLARE( luaconstructor_object )
{
    CClientRwObject *obj = (CClientRwObject*)lua_touserdata( L, lua_upvalueindex( 1 ) );

    ILuaClass& j = *lua_refclass( L, 1 );
    j.SetTransmit( LUACLASS_RWOBJECT, obj );

    lua_pushvalue( L, LUA_ENVIRONINDEX );
    lua_pushvalue( L, lua_upvalueindex( 1 ) );
    luaL_openlib( L, NULL, object_interface, 1 );

    lua_pushlstring( L, "rwobject", 8 );
    lua_setfield( L, LUA_ENVIRONINDEX, "__type" );
    return 0;
}

CClientRwObject::CClientRwObject( LuaClass& root, CRwObject& object ) : LuaElement( root ), m_object( object )
{
    // Lua instancing
    lua_State *L = root.GetVM();

    PushStack( L );
    lua_pushlightuserdata( L, this );
    lua_pushcclosure( L, luaconstructor_object, 1 );
    luaJ_extend( L, -2, 0 );
    lua_pop( L, 1 );
}

CClientRwObject::~CClientRwObject()
{
    delete &m_object;
}