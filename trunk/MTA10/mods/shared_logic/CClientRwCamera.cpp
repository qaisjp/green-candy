/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*               (Shared logic for modifications)
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        mods/shared_logic/CClientRwCamera.cpp
*  PURPOSE:     RenderWare camera virtualization
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

    CClientRwCamera *cam = (CClientRwCamera*)lua_touserdata( L, lua_upvalueindex( 1 ) );

    // Force the light into the clump's hierarchy
    // If model is nil, we remove the connection
    if ( model )
    {
        cam->SetModel( model );
        cam->m_cam.AddToModel( &model->m_model );   // do the internal magic :)

        cam->SetParent( model->m_parent );
        cam->SetRoot( model->m_parent );
        model->m_cameras.insert( model->m_cameras.begin(), cam );
    }
    else if ( cam->m_clump )
    {
        cam->m_clump->m_cameras.remove( cam );
        cam->SetRoot( NULL );

        cam->m_cam.RemoveFromModel();
        cam->SetModel( NULL );
    }

    LUA_SUCCESS;
}

static LUA_DECLARE( getClump )
{
    CClientDFF *model = ((CClientRwCamera*)lua_touserdata( L, lua_upvalueindex( 1 ) ))->m_clump;

    if ( !model )
        return 0;

    model->PushStack( L );
    return 1;
}

static LUA_DECLARE( setParent )
{
    if ( ((CClientRwCamera*)lua_touserdata( L, lua_upvalueindex( 1 ) ))->m_cam.IsRendering() )
        throw lua_exception( L, LUA_ERRRUN, "cannot change rwcamera parent during render focus" );

    lua_getfield( L, LUA_ENVIRONINDEX, "super" );
    lua_insert( L, 1 );
    lua_call( L, 1, 1 );
    return 1;
}

static LUA_DECLARE( isRendering )
{
    lua_pushboolean( L, ((CClientRwCamera*)lua_touserdata( L, lua_upvalueindex( 1 ) ))->m_cam.IsRendering() );
    return 1;
}

static LUA_DECLARE( update )
{
    luaL_checktype( L, 1, LUA_TFUNCTION );
    lua_settop( L, 1 );

    CClientRwCamera *cam = ((CClientRwCamera*)lua_touserdata( L, lua_upvalueindex( 1 ) ));
    CRwCamera *rwcam = &cam->m_cam;

    if ( !rwcam->BeginUpdate() )
        throw lua_exception( L, LUA_ERRRUN, "failed to update camera" );

    try
    {
        lua_yield_shield protect( L );  // we may not yield here for security reasons
        lua_call( L, 0, 0 );
    }
    catch( ... )
    {
        rwcam->EndUpdate();
        throw;
    }

    rwcam->EndUpdate();
    return 0;
}

const luaL_Reg rwcamera_interface[] =
{
    LUA_METHOD( setParent ),
    { NULL, NULL }
};

const luaL_Reg rwcamera_interface_light[] =
{
    LUA_METHOD( setClump ),
    LUA_METHOD( getClump ),
    LUA_METHOD( isRendering ),
    LUA_METHOD( update ),
    { NULL, NULL }
};

static LUA_DECLARE( luaconstructor_rwcamera )
{
    CClientRwCamera *cam = (CClientRwCamera*)lua_touserdata( L, lua_upvalueindex( 1 ) );

    ILuaClass& j = *lua_refclass( L, 1 );
    j.SetTransmit( LUACLASS_RWCAMERA, cam );

    j.RegisterLightInterface( L, rwcamera_interface_light, cam );

    lua_pushvalue( L, LUA_ENVIRONINDEX );
    lua_pushvalue( L, lua_upvalueindex( 1 ) );
    luaL_openlib( L, NULL, rwcamera_interface, 1 );

    lua_pushlstring( L, "rwcamera", 8 );
    lua_setfield( L, LUA_ENVIRONINDEX, "__type" );
    return 0;
}

CClientRwCamera::CClientRwCamera( lua_State *L, CRwCamera& cam ) : CClientRwObject( L, cam ), m_cam( cam )
{
    // Lua instancing
    PushStack( L );
    lua_pushlightuserdata( L, this );
    lua_pushcclosure( L, luaconstructor_rwcamera, 1 );
    luaJ_extend( L, -2, 0 );
    lua_pop( L, 1 );

    m_clump = NULL;
}

CClientRwCamera::~CClientRwCamera()
{
}

void CClientRwCamera::SetModel( CClientDFF *model )
{
    if ( model == m_clump )
        return;

    if ( m_clump )
        m_clump->m_cameras.remove( this );

    m_clump = model;

    if ( !model )
        return;

    model->m_cameras.insert( model->m_cameras.begin(), this );
}