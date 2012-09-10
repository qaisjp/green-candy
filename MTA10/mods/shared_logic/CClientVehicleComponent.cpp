/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*               (Shared logic for modifications)
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        mods/shared_logic/CClientVehicleComponent.cpp
*  PURPOSE:     Vehicle component class export
*  DEVELOPERS:  The_GTA <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#include "StdInc.h"

static LUA_DECLARE( setPosition )
{
    CVector pos;

    LUA_ARGS_BEGIN;
    argStream.ReadVector( pos );
    LUA_ARGS_END;

    ((CClientVehicleComponent*)lua_touserdata( L, lua_upvalueindex( 1 ) ))->m_component->SetPosition( pos );
    LUA_SUCCESS;
}

static LUA_DECLARE( getPosition )
{
    const CVector& pos = ((CClientVehicleComponent*)lua_touserdata( L, lua_upvalueindex( 1 ) ))->m_component->GetPosition();

    lua_pushnumber( L, pos[0] );
    lua_pushnumber( L, pos[1] );
    lua_pushnumber( L, pos[2] );
    return 3;
}

static LUA_DECLARE( getWorldPosition )
{
    const CVector& pos = ((CClientVehicleComponent*)lua_touserdata( L, lua_upvalueindex( 1 ) ))->m_component->GetWorldPosition();

    lua_pushnumber( L, pos[0] );
    lua_pushnumber( L, pos[1] );
    lua_pushnumber( L, pos[2] );
    return 3;
}

static LUA_DECLARE( setMatrix )
{
    RwMatrix *mat;

    LUA_ARGS_BEGIN;
    argStream.ReadClass( mat, LUACLASS_MATRIX );
    LUA_ARGS_END;

    ((CClientVehicleComponent*)lua_touserdata( L, lua_upvalueindex( 1 ) ))->m_component->SetMatrix( *mat );
    LUA_SUCCESS;
}

static LUA_DECLARE( getMatrix )
{
    lua_creatematrix( L, ((CClientVehicleComponent*)lua_touserdata( L, lua_upvalueindex( 1 ) ))->m_component->GetMatrix() );
    return 1;
}

static LUA_DECLARE( getWorldMatrix )
{
    lua_creatematrix( L, ((CClientVehicleComponent*)lua_touserdata( L, lua_upvalueindex( 1 ) ))->m_component->GetWorldMatrix() );
    return 1;
}

static luaL_Reg component_interface[] =
{
    LUA_METHOD( setPosition ),
    LUA_METHOD( getPosition ),
    LUA_METHOD( getWorldPosition ),
    LUA_METHOD( setMatrix ),
    LUA_METHOD( getMatrix ),
    LUA_METHOD( getWorldMatrix ),
    { NULL, NULL }
};

static LUA_DECLARE( luaconstructor_component )
{
    CClientVehicleComponent *comp = (CClientVehicleComponent*)lua_touserdata( L, lua_upvalueindex( 1 ) );

    ILuaClass& j = *lua_refclass( L, 1 );
    j.SetTransmit( LUACLASS_VEHICLECOMPONENT, comp );

    lua_pushvalue( L, LUA_ENVIRONINDEX );
    lua_pushvalue( L, lua_upvalueindex( 1 ) );
    luaL_openlib( L, NULL, component_interface, 1 );

    lua_pushlstring( L, "vehicle-component", 17 );
    lua_setfield( L, LUA_ENVIRONINDEX, "__type" );
    return 0;
}

CClientVehicleComponent::CClientVehicleComponent( CClientVehicle *veh, unsigned int idx, CVehicleComponent *comp ) : LuaElement( *(LuaClass*)veh )
{
    // Lua instancing
    lua_State *L = veh->GetVM();

    PushStack( L );
    lua_pushlightuserdata( L, this );
    lua_pushcclosure( L, luaconstructor_component, 1 );
    luaJ_extend( L, -2, 0 );
    lua_pop( L, 1 );

    m_idx = idx;
    m_component = comp;
}

CClientVehicleComponent::~CClientVehicleComponent()
{
    // Unlink from vehicle
    GetVehicle()->m_components[m_idx] = NULL;

    delete m_component;
}