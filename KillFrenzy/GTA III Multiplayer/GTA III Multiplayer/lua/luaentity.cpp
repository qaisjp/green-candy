/*****************************************************************************
*
*  PROJECT:     KillFrenzy! 1.0
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        lua/luaentity.cpp
*  PURPOSE:     Game Lua entity
*  DEVELOPERS:  The_GTA <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#include <StdInc.h>

static int entity_getPosition( lua_State *L )
{
    CGameEntity& entity = *(CGameEntity*)lua_touserdata( L, lua_upvalueindex( 1 ) );

    vec3_t pos;
    entity.GetEntity().GetPosition( pos );

    lua_pushnumber( L, pos[0] );
    lua_pushnumber( L, pos[1] );
    lua_pushnumber( L, pos[2] );
    return 3;
}

static int entity_setPosition( lua_State *L )
{
    luaL_checktyperange( L, 1, LUA_TNUMBER, 3 );

    CGameEntity& entity = *(CGameEntity*)lua_touserdata( L, lua_upvalueindex( 1 ) );

    entity.GetEntity().SetPosition( (float)lua_tonumber( L, 1 ), (float)lua_tonumber( L, 2 ), (float)lua_tonumber( L, 3 ) );
    return 0;
}

static int entity_getRotation( lua_State *L )
{
    CGameEntity& entity = *(CGameEntity*)lua_touserdata( L, lua_upvalueindex( 1 ) );

    vec3_t rot;
    entity.GetEntity().GetRotation( rot );

    lua_pushnumber( L, rot[0] );
    lua_pushnumber( L, rot[1] );
    lua_pushnumber( L, rot[2] );
    return 3;
}

static int entity_setRotation( lua_State *L )
{
    luaL_checktyperange( L, 1, LUA_TNUMBER, 3 );

    CGameEntity& entity = *(CGameEntity*)lua_touserdata( L, lua_upvalueindex( 1 ) );

    entity.GetEntity().SetRotation( (float)lua_tonumber( L, 1 ), (float)lua_tonumber( L, 2 ), (float)lua_tonumber( L, 3 ) );
    return 0;
}

static int entity_getMatrix( lua_State *L )
{
    CGameEntity& entity = *(CGameEntity*)lua_touserdata( L, lua_upvalueindex( 1 ) );

    lua_creatematrix( L, entity.GetEntity().m_matrix );
    return 1;
}

static int entity_getModelID( lua_State *L )
{
    lua_pushnumber( L, ((CGameEntity*)lua_touserdata( L, lua_upvalueindex( 1 ) ))->GetEntity().m_usModelID );
    return 1;
}

static int entity_setModelID( lua_State *L )
{
    return 0;
}

static int entity_destroy( lua_State *L )
{
    delete (CGameEntity*)lua_touserdata( L, lua_upvalueindex( 1 ) );

    return 0;
}

static const luaL_Reg entity_interface[] =
{
    { "getPosition", entity_getPosition },
    { "setPosition", entity_setPosition },
    { "getRotation", entity_getRotation },
    { "setRotation", entity_setRotation },
    { "getMatrix", entity_getMatrix },
    { "getModelID", entity_getModelID },
    { "setModelID", entity_setModelID },
    { "destroy", entity_destroy },
    { NULL, NULL }
};

static int entity_constructor( lua_State *L )
{
    CGameEntity *entity = (CGameEntity*)lua_touserdata( L, lua_upvalueindex( 1 ) );

    ILuaClass& j = *lua_refclass( L, 1 );
    j.SetTransmit( LUACLASS_ENTITY, entity );

    lua_pushvalue( L, LUA_ENVIRONINDEX );
    lua_pushvalue( L, lua_upvalueindex( 1 ) );
    luaL_openlib( L, NULL, entity_interface, 1 );

    lua_pushlstring( L, "entity", 6 );
    lua_setfield( L, LUA_ENVIRONINDEX, "__type" );

    lua_basicprotect( L );
    return 0;
}

static int sysentity_index( lua_State *L )
{
    if ( lua_type( L, 2 ) == LUA_TSTRING )
    {
        if ( lua_getstring( L, 2 ) == "destroy" )
            return 0;
    }

    lua_getfield( L, LUA_ENVIRONINDEX, "super" );
    lua_insert( L, 1 );
    lua_call( L, 2, 1 );
    return 1;
}

static const luaL_Reg sysentity_interface[] =
{
    { "__index", sysentity_index },
    { NULL, NULL }
};

static int sysentity_constructor( lua_State *L )
{
    lua_pushvalue( L, LUA_ENVIRONINDEX );
    lua_pushvalue( L, lua_upvalueindex( 1 ) );
    luaL_openlib( L, NULL, sysentity_interface, 1 );

    lua_pushlstring( L, "sysentity", 9 );
    lua_setfield( L, LUA_ENVIRONINDEX, "__type" );
    return 0;
}

static inline int _trefget( lua_State *L, bool sys, CGameEntity& entity )
{
    lua_pushlightuserdata( L, &entity );
    lua_pushcclosure( L, entity_constructor, 1 );
    lua_newclass( L );

    if ( sys )
    {
        lua_pushlightuserdata( L, &entity );
        lua_pushcclosure( L, sysentity_constructor, 1 );
        luaJ_extend( L, -2, 0 );
    }

    return luaL_ref( L, LUA_REGISTRYINDEX );
}

CGameEntity::CGameEntity( lua_State *L, bool system, CEntity& entity ) : m_entity( entity ), LuaClass( L, _trefget( L, system, *this ) )
{
    m_system = system;
}

CGameEntity::~CGameEntity()
{
    // We have to destroy the entity in a subclass
}