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

static LUA_DECLARE( getFrame )
{
    CClientRwFrame *frame = ((CClientRwObject*)lua_touserdata( L, lua_upvalueindex( 1 ) ))->m_parent;

    if ( !frame )
        return 0;

    frame->PushStack( L );
    return 1;
}

static LUA_DECLARE( isValidChild )
{
    // RenderWare objects have a different type of hierarchy.
    // Some objects - like RpClump - establish special connections using the parenthood.
    // By default, objects cannot have children; objects should be cast at their parent frame
    return 0;
}

static luaL_Reg object_interface[] =
{
    LUA_METHOD( getFrame ),
    LUA_METHOD( isValidChild ),
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

CClientRwObject::CClientRwObject( lua_State *L, CRwObject& object ) : LuaElement( L ), m_object( object )
{
    // Lua instancing
    PushStack( L );
    lua_pushlightuserdata( L, this );
    lua_pushcclosure( L, luaconstructor_object, 1 );
    luaJ_extend( L, -2, 0 );
    lua_pop( L, 1 );

    m_parent = NULL;
}

CClientRwObject::~CClientRwObject()
{
    delete &m_object;
}