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

static LUA_DECLARE( setParent )
{
    // Make sure that we stay in the resource tree!
    CClientRwObject& element = *(CClientRwObject*)lua_touserdata( L, lua_upvalueindex( 1 ) );

    if ( lua_type( L, 1 ) != LUA_TCLASS )
    {
        lua_getfield( L, LUA_ENVIRONINDEX, "super" );

        // If we have a root, we want to reparent to it
        if ( element.m_root )
            element.m_root->PushStack( L );
        else
            lua_pushnil( L );

        lua_call( L, 1, 1 );
        return 1;
    }

    // We can stay without a root
    if ( !element.m_root )
        goto parent;

    // Check if the new parent is inside our root
    element.m_root->PushStack( L );

    if ( lua_refclass( L, 1 )->IsRootedIn( L, -1 ) )
        goto parent;

    // Failed (i.e. we may not be the child of an outside resource class)
    lua_pushboolean( L, false );
    return 1;

parent:
    lua_getfield( L, LUA_ENVIRONINDEX, "super" );
    lua_pushvalue( L, 1 );
    lua_call( L, 1, 1 );
    return 1;
}

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
    // Some objects - like RpClump - establish connections using the parenthood to a RwFrame.
    // This frame would hold all objects references in the RpClump!
    // By default, objects cannot have children; objects should be cast at their parent frame.
    return 0;
}

static luaL_Reg object_interface_light[] =
{
    LUA_METHOD( getFrame ),
    LUA_METHOD( isValidChild ),
    LUA_METHOD( setParent ),
    { NULL, NULL }
};

static LUA_DECLARE( destroy )
{
    delete (CClientRwObject*)lua_touserdata( L, lua_upvalueindex( 1 ) );

    return 0;
}

static luaL_Reg object_interface[] =
{
    LUA_METHOD( setParent ),
    LUA_METHOD( destroy ),
    { NULL, NULL }
};

static LUA_DECLARE( luaconstructor_object )
{
    CClientRwObject *obj = (CClientRwObject*)lua_touserdata( L, lua_upvalueindex( 1 ) );

    ILuaClass& j = *lua_refclass( L, 1 );
    j.SetTransmit( LUACLASS_RWOBJECT, obj );

    j.RegisterLightInterface( L, object_interface_light, obj );

    lua_pushlstring( L, "rwobject", 8 );
    lua_setfield( L, LUA_ENVIRONINDEX, "__type" );
    return 0;
}

/*
    RwObjects are not LuaClass based, as they should be Garbage Collected.
*/
CClientRwObject::CClientRwObject( lua_State *L, CRwObject& object ) : m_object( object )
{
    // Lua instancing
    lua_pushlightuserdata( L, this );
    lua_pushcclosure( L, luaconstructor_object, 1 );
    lua_newclass( L );

    // Register owner, so we are kept alive

    // Store the class
    m_class = lua_refclass( L, -1 );

    lua_pop( L, 1 );

    m_parent = NULL;
    m_owner = NULL;
    m_root = NULL;
}

CClientRwObject::~CClientRwObject()
{
    UnlinkOwner();

    delete &m_object;
}

void CClientRwObject::UnlinkOwner()
{
    if ( !m_owner )
        return;

    LIST_REMOVE( m_ownerObjects );

    m_owner = NULL;
}

void CClientRwObject::SetOwner( CResource *res )
{
    UnlinkOwner();

    if ( !res )
        return;

    m_owner = res;

    LIST_INSERT( res->m_rwObjects.root, m_ownerObjects );
}

// RenderWare objects will be kept alive by default if RW_LUA_KEEP_ALIVE is set
void CClientRwObject::MarkGC( lua_State *L )
{
#ifdef RW_LUA_KEEP_ALIVE
    // Flag ourselves
    m_class->Propagate( L );

    lua_gcpaycost( L, sizeof( *this ) );
#elif defined(RW_LUA_YIELD_MARK)    // If RW_LUA_YIELD_MARK is set, traversing RW objects gets CPU-heartwarming <3
    lua_gcpaycost( L, 1 );
#endif //RW_LUA_KEEP_ALIVE
}