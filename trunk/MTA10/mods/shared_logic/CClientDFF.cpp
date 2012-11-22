/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*               (Shared logic for modifications)
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        mods/shared_logic/CClientDFF.cpp
*  PURPOSE:     .dff model handling class
*  DEVELOPERS:  Christian Myhre Lundheim <>
*               The_GTA <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#include "StdInc.h"

static LUA_DECLARE( clone )
{
    ( new CClientDFF( L, *((CClientDFF*)lua_touserdata( L, lua_upvalueindex( 1 )))->m_model.Clone() ) )->PushStack( L );
    return 1;
}

static LUA_DECLARE( render )
{
    ((CClientDFF*)lua_touserdata( L, lua_upvalueindex( 1 )))->m_model.Render();
    return 0;
}

static LUA_DECLARE( getAtomics )
{
    CClientDFF::atomics_t& list = ((CClientDFF*)lua_touserdata( L, lua_upvalueindex( 1 )))->m_atomics;

    lua_settop( L, 0 );
    lua_createtable( L, list.size(), 0 );

    unsigned int n = 1;

    for ( CClientDFF::atomics_t::iterator iter = list.begin(); iter != list.end(); iter++, n++ )
    {
        (*iter)->PushStack( L );
        lua_rawseti( L, 1, n );
    }

    return 1;
}

static LUA_DECLARE( getLights )
{
    CClientDFF::lights_t& list = ((CClientDFF*)lua_touserdata( L, lua_upvalueindex( 1 )))->m_lights;

    lua_settop( L, 0 );
    lua_createtable( L, list.size(), 0 );

    unsigned int n = 1;

    for ( CClientDFF::lights_t::iterator iter = list.begin(); iter != list.end(); iter++, n++ )
    {
        (*iter)->PushStack( L );
        lua_rawseti( L, 1, n );
    }

    return 1;
}

static LUA_DECLARE( getCameras )
{
    CClientDFF::cameras_t& list = ((CClientDFF*)lua_touserdata( L, lua_upvalueindex( 1 )))->m_cameras;

    lua_settop( L, 0 );
    lua_createtable( L, list.size(), 0 );

    unsigned int n = 1;

    for ( CClientDFF::cameras_t::iterator iter = list.begin(); iter != list.end(); iter++, n++ )
    {
        (*iter)->PushStack( L );
        lua_rawseti( L, 1, n );
    }

    return 1;
}

static LUA_DECLARE( replaceModel )
{
    unsigned short model;

    LUA_ARGS_BEGIN;
    argStream.ReadNumber( model );
    LUA_ARGS_END;

    lua_pushboolean( L, ((CClientDFF*)lua_touserdata( L, lua_upvalueindex( 1 )))->ReplaceModel( model ) );
    return 1;
}

static LUA_DECLARE( isReplaced )
{
    unsigned short model;

    LUA_ARGS_BEGIN;
    argStream.ReadNumber( model );
    LUA_ARGS_END;

    lua_pushboolean( L, ((CClientDFF*)lua_touserdata( L, lua_upvalueindex( 1 ) ) )->HasReplaced( model ) );
    return 1;
}

static LUA_DECLARE( getReplaced )
{
    lua_settop( L, 0 );

    std::vector <unsigned short> impList = ((CClientDFF*)lua_touserdata( L, lua_upvalueindex( 1 )))->m_model.GetImportList();
    std::vector <unsigned short>::const_iterator iter = impList.begin();
    int n = 1;

    lua_createtable( L, impList.size(), 0 );

    for ( ; iter != impList.end(); iter++ )
    {
        lua_pushnumber( L, *iter );
        lua_rawseti( L, 1, n++ );
    }

    return 1;
}

static LUA_DECLARE( restoreModel )
{
    unsigned short model;

    LUA_ARGS_BEGIN;
    argStream.ReadNumber( model );
    LUA_ARGS_END;

    ((CClientDFF*)lua_touserdata( L, lua_upvalueindex( 1 )))->RestoreModel( model );
    LUA_SUCCESS;
}

static LUA_DECLARE( restoreAll )
{
    ((CClientDFF*)lua_touserdata( L, lua_upvalueindex( 1 )))->RestoreModels();
    return 0;
}

static const luaL_Reg dff_interface[] =
{
    LUA_METHOD( clone ),
    LUA_METHOD( render ),
    LUA_METHOD( getAtomics ),
    LUA_METHOD( getLights ),
    LUA_METHOD( getCameras ),
    LUA_METHOD( replaceModel ),
    LUA_METHOD( isReplaced ),
    LUA_METHOD( getReplaced ),
    LUA_METHOD( restoreModel ),
    LUA_METHOD( restoreAll ),
    { NULL, NULL }
};

static void RwFrameObjectAcquire( CClientRwObject *obj, CClientDFF *model )
{
    eRwType type = obj->GetObject().GetType();

    union
    {
        CClientRwObject *rwobj;
        CClientAtomic *atom;
        CClientLight *light;
        CClientRwCamera *cam;
    };

    switch( type )
    {
    case RW_ATOMIC:
        atom = (CClientAtomic*)obj;
        atom->m_clump = model;

        model->m_atomics.insert( model->m_atomics.begin(), atom );
        break;
    case RW_LIGHT:
        light = (CClientLight*)obj;
        light->m_clump = model;

        model->m_lights.insert( model->m_lights.begin(), light );
        break;
    case RW_CAMERA:
        cam = (CClientRwCamera*)obj;
        cam->m_clump = model;

        model->m_cameras.insert( model->m_cameras.begin(), cam );
        break;
    }

    rwobj->SetRoot( model->m_parent );
}

static void RwFrameInspect( CClientRwFrame *frame, CClientDFF *model )
{
    // Inspect all children...
    {
        CClientRwFrame::children_t& list = frame->m_children;

        for ( CClientRwFrame::children_t::iterator iter = list.begin(); iter != list.end(); iter++ )
            RwFrameInspect( *iter, model );
    }

    // ... and objects (just like in game-layer)
    {
        CClientRwFrame::objects_t& list = frame->m_objects;

        for ( CClientRwFrame::objects_t::iterator iter = list.begin(); iter != list.end(); iter++ )
            RwFrameObjectAcquire( *iter, model );
    }
}

static int luaconstructor_dff( lua_State *L )
{
    CClientDFF *dff = (CClientDFF*)lua_touserdata( L, lua_upvalueindex( 1 ) );

    ILuaClass& j = *lua_refclass( L, 1 );
    j.SetTransmit( LUACLASS_DFF, dff );

    lua_pushvalue( L, LUA_ENVIRONINDEX );
    lua_pushvalue( L, lua_upvalueindex( 1 ) );
    luaL_openlib( L, NULL, dff_interface, 1 );

    lua_pushlstring( L, "dff", 3 );
    lua_setfield( L, LUA_ENVIRONINDEX, "__type" );
    return 0;
}

CClientDFF::CClientDFF( lua_State *L, CModel& model ) : CClientRwObject( L, model ), m_model( model )
{
    // Lua instancing
    PushStack( L );
    lua_pushlightuserdata( L, this );
    lua_pushcclosure( L, luaconstructor_dff, 1 );
    luaJ_extend( L, -2, 0 );
    lua_pop( L, 1 );

    RwFrameInspect( m_parent = new CClientRwFrame( L, *model.GetFrame() ), this );
    m_parent->IncrementMethodStack();
    // I do not know whether we can let the Lua runtime destroy the frame
    // For security reasons I disable this possibility
    // Feel free to discuss about this with me (The_GTA)
}

CClientDFF::~CClientDFF()
{
    RestreamAll();

    // Delete our hierarchy
    m_parent->Delete(); m_parent->DecrementMethodStack();
}

bool CClientDFF::ReplaceModel( unsigned short id )
{
    if ( !m_model.Replace( id ) )
        return false;

    g_pClientGame->GetManager()->Restream( id );
    return true;
}

bool CClientDFF::HasReplaced( unsigned short id ) const
{
    return m_model.IsReplaced( id );
}

void CClientDFF::RestoreModel( unsigned short id )
{
    if ( !m_model.Restore( id ) )
        return;

    g_pClientGame->GetManager()->Restream( id );
}

void CClientDFF::RestoreModels()
{
    RestreamAll();

    m_model.RestoreAll();
}

void CClientDFF::RestreamAll() const
{
    std::vector <unsigned short> impList = m_model.GetImportList();
    std::vector <unsigned short>::iterator iter = impList.begin();

    for ( ; iter != impList.end(); iter++ )
        g_pClientGame->GetManager()->Restream( *iter );
}