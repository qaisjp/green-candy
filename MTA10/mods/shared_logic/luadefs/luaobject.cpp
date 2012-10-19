/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*               (Shared logic for modifications)
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        mods/shared_logic/luadefs/luaobject.cpp
*  PURPOSE:     Object Lua instancing
*  DEVELOPERS:  The_GTA <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#include <StdInc.h>

static LUA_DECLARE( setHealth )
{
    float health;

    LUA_ARGS_BEGIN;
    argStream.ReadNumber( health );
    LUA_ARGS_END;

    ((CClientObject*)lua_touserdata( L, lua_upvalueindex( 1 )))->SetHealth( health );
    LUA_SUCCESS;
}

static LUA_DECLARE( getHealth )
{
    lua_pushnumber( L, ((CClientObject*)lua_touserdata( L, lua_upvalueindex( 1 )))->GetHealth() );
    return 1;
}

static LUA_DECLARE( setModel )
{
    unsigned short model;

    LUA_ARGS_BEGIN;
    argStream.ReadNumber( model );
    LUA_ARGS_END;

    LUA_CHECK( CClientObjectManager::IsValidModel( model ) );

    ((CClientObject*)lua_touserdata( L, lua_upvalueindex( 1 )))->SetModel( model );
    LUA_SUCCESS;
}

static LUA_DECLARE( getModel )
{
    lua_pushnumber( L, ((CClientObject*)lua_touserdata( L, lua_upvalueindex( 1 )))->GetModel() );
    return 1;
}

static LUA_DECLARE( setObjectStatic )
{
    bool enabled;

    LUA_ARGS_BEGIN;
    argStream.ReadBool( enabled );
    LUA_ARGS_END;

    ((CClientObject*)lua_touserdata( L, lua_upvalueindex( 1 )))->SetStatic( enabled );
    LUA_SUCCESS;
}

static LUA_DECLARE( isObjectStatic )
{
    lua_pushboolean( L, ((CClientObject*)lua_touserdata( L, lua_upvalueindex( 1 )))->IsStatic() );
    return 1;
}

static LUA_DECLARE( setObjectScale )
{
    float scale;

    LUA_ARGS_BEGIN;
    argStream.ReadNumber( scale );
    LUA_ARGS_END;

    ((CClientObject*)lua_touserdata( L, lua_upvalueindex( 1 )))->SetScale( scale );
    LUA_SUCCESS;
}

static LUA_DECLARE( getObjectScale )
{
    lua_pushnumber( L, ((CClientObject*)lua_touserdata( L, lua_upvalueindex( 1 )))->GetScale() );
    return 1;
}

static const luaL_Reg object_interface[] =
{
    LUA_METHOD( setHealth ),
    LUA_METHOD( getHealth ),
    LUA_METHOD( setModel ),
    LUA_METHOD( getModel ),
    LUA_METHOD( setObjectStatic ),
    LUA_METHOD( isObjectStatic ),
    LUA_METHOD( setObjectScale ),
    LUA_METHOD( getObjectScale ),
    { NULL, NULL }
};

LUA_DECLARE( luaconstructor_object )
{
    CClientObject *obj = (CClientObject*)lua_touserdata( L, lua_upvalueindex( 1 ));

    ILuaClass& j = *lua_refclass( L, 1 );
    j.SetTransmit( LUACLASS_OBJECT, obj );

    lua_pushvalue( L, LUA_ENVIRONINDEX );
    lua_pushvalue( L, lua_upvalueindex( 1 ) );
    luaL_openlib( L, NULL, object_interface, 1 );

    lua_pushlstring( L, "object", 6 );
    lua_setfield( L, LUA_ENVIRONINDEX, "__type" );
    return 0;
}