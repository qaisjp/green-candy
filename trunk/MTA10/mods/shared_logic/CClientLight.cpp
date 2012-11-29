/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*               (Shared logic for modifications)
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        mods/shared_logic/CClientLight.cpp
*  PURPOSE:     RenderWare light virtualization
*  DEVELOPERS:  The_GTA <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#include "StdInc.h"

static LUA_DECLARE( setClump )
{
    CClientDFF *model;

    LUA_ARGS_BEGIN;
    argStream.ReadClass( model, LUACLASS_DFF, NULL );
    LUA_ARGS_END;

    CClientLight *light = (CClientLight*)lua_touserdata( L, lua_upvalueindex( 1 ) );

    // Force the light into the clump's hierarchy
    // If model is nil, we remove the connection
    if ( model )
    {
        light->SetModel( model );
        light->m_light.AddToModel( &model->m_model );   // do the internal magic :)

        light->SetRoot( model->m_parent );
        model->m_lights.insert( model->m_lights.begin(), light );
    }
    else if ( light->m_clump )
    {
        light->m_clump->m_lights.remove( light );
        light->SetRoot( NULL );

        light->m_light.RemoveFromModel();
        light->SetModel( NULL );
    }

    LUA_SUCCESS;
}

static LUA_DECLARE( getClump )
{
    CClientDFF *model = ((CClientLight*)lua_touserdata( L, lua_upvalueindex( 1 ) ))->m_clump;

    if ( !model )
        return 0;

    model->PushStack( L );
    return 1;
}

static LUA_DECLARE( setRadius )
{
    float radius;

    LUA_ARGS_BEGIN;
    argStream.ReadNumber( radius );
    LUA_ARGS_END;

    ((CClientLight*)lua_touserdata( L, lua_upvalueindex( 1 ) ))->m_light.SetRadius( radius );
    LUA_SUCCESS;
}

static LUA_DECLARE( getRadius )
{
    lua_pushnumber( L, ((CClientLight*)lua_touserdata( L, lua_upvalueindex( 1 ) ))->m_light.GetRadius() );
    return 1;
}

static LUA_DECLARE( setColor )
{
    RwColorFloat color;

    LUA_ARGS_BEGIN;
    argStream.ReadNumber( color.r );
    argStream.ReadNumber( color.g );
    argStream.ReadNumber( color.b );
    argStream.ReadNumber( color.a );
    LUA_ARGS_END;

    ((CClientLight*)lua_touserdata( L, lua_upvalueindex( 1 ) ))->m_light.SetColor( color );
    LUA_SUCCESS;
}

static LUA_DECLARE( getColor )
{
    const RwColorFloat& color = ((CClientLight*)lua_touserdata( L, lua_upvalueindex( 1 ) ))->m_light.GetColor();

    lua_pushnumber( L, color.r );
    lua_pushnumber( L, color.g );
    lua_pushnumber( L, color.b );
    lua_pushnumber( L, color.a );
    return 4;
}

static LUA_DECLARE( setLightIndex )
{
    unsigned int idx;

    LUA_ARGS_BEGIN;
    argStream.ReadNumber( idx );
    LUA_ARGS_END;

    LUA_CHECK( idx < 8 );

    ((CClientLight*)lua_touserdata( L, lua_upvalueindex( 1 ) ))->m_light.SetLightIndex( idx );
    LUA_SUCCESS;
}

static LUA_DECLARE( getLightIndex )
{
    lua_pushnumber( L, ((CClientLight*)lua_touserdata( L, lua_upvalueindex( 1 ) ))->m_light.GetLightIndex() );
    return 1;
}

static luaL_Reg light_interface[] =
{
    LUA_METHOD( setClump ),
    LUA_METHOD( getClump ),
    LUA_METHOD( setRadius ),
    LUA_METHOD( getRadius ),
    LUA_METHOD( setColor ),
    LUA_METHOD( getColor ),
    LUA_METHOD( setLightIndex ),
    LUA_METHOD( getLightIndex ),
    { NULL, NULL }
};

static LUA_DECLARE( luaconstructor_light )
{
    CClientLight *light = (CClientLight*)lua_touserdata( L, lua_upvalueindex( 1 ) );

    ILuaClass& j = *lua_refclass( L, 1 );
    j.SetTransmit( LUACLASS_LIGHT, light );

    lua_pushvalue( L, LUA_ENVIRONINDEX );
    lua_pushvalue( L, lua_upvalueindex( 1 ) );
    luaL_openlib( L, NULL, light_interface, 1 );

    lua_pushlstring( L, "rwlight", 7 );
    lua_setfield( L, LUA_ENVIRONINDEX, "__type" );
    return 0;
}

CClientLight::CClientLight( lua_State *L, CClientDFF *model, CRpLight& light ) : CClientRwObject( L, light ), m_light( light )
{
    // Lua interfacing
    PushStack( L );
    lua_pushlightuserdata( L, this );
    lua_pushcclosure( L, luaconstructor_light, 1 );
    luaJ_extend( L, -2, 0 );
    lua_pop( L, 1 );

    m_clump = NULL;

    // Assign the model if given
    SetModel( model );
}

CClientLight::~CClientLight()
{
    SetModel( NULL );
}

void CClientLight::SetModel( CClientDFF *model )
{
    if ( model == m_clump )
        return;

    if ( m_clump )
        m_clump->m_lights.remove( this );

    m_clump = model;

    if ( !model )
        return;

    model->m_lights.insert( model->m_lights.begin(), this );
}