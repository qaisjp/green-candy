/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        Shared/logic/LuaElement.cpp
*  PURPOSE:     Lua internalization management entity
*  DEVELOPERS:  The_GTA <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#include <StdInc.h>

static int element_setParent( lua_State *L )
{
    luaL_checktype( L, 1, LUA_TCLASS );

    lua_settop( L, 1 );

    // Make sure that we stay in the resource tree!
    LuaElement& element = *(LuaElement*)lua_touserdata( L, lua_upvalueindex( 1 ) );

    // We can stay without a root
    if ( !element.m_root )
    {
        lua_getfield( L, LUA_ENVIRONINDEX, "super" );
        lua_pushvalue( L, 1 );
        lua_call( L, 1, 1 );
        return 1;
    }

    element.m_root->PushStack( L );

    lua_pushvalue( L, 1 );

    while ( lua_type( L, 3 ) == LUA_TCLASS )
    {
        ILuaClass *j = lua_refclass( L, 3 );

        if ( !j->IsTransmit( LUACLASS_ELEMENT ) )
            break;

        if ( lua_equal( L, 2, 3 ) )
        {
            lua_getfield( L, LUA_ENVIRONINDEX, "super" );
            lua_pushvalue( L, 1 );
            lua_call( L, 1, 1 );
            return 1;
        }

        j->PushParent( L );
        lua_remove( L, 3 );
    }

    // We may not be the child of an outside resource class
    lua_pushboolean( L, false );
    return 1;
}

static int element_destroy( lua_State *L )
{
    delete (LuaElement*)lua_touserdata( L, lua_upvalueindex( 1 ) );

    return 0;
}

static const luaL_Reg element_interface[] =
{
    { "setParent", element_setParent },
    { "destroy", element_destroy },
    { NULL, NULL }
};

static int luaconstructor_element( lua_State *L )
{
    LuaElement *element = (LuaElement*)lua_touserdata( L, lua_upvalueindex( 1 ) );

    ILuaClass& j = *lua_refclass( L, 1 );
    j.SetTransmit( LUACLASS_ELEMENT, element );

    // Register the element interface
    lua_pushvalue( L, LUA_ENVIRONINDEX );
    lua_pushvalue( L, lua_upvalueindex( 1 ) );
    luaL_openlib( L, NULL, element_interface, 1 );

    // Put basic protection against modification from scripts
    lua_basicprotect( L );

    // Define our type
    lua_pushlstring( L, "element", 7 );
    lua_setfield( L, LUA_ENVIRONINDEX, "__type" );
    return 0;
}

static inline int _trefget( lua_State *L, LuaElement *el )
{
    lua_pushlightuserdata( L, el );
    lua_pushcclosure( L, luaconstructor_element, 1 );
    lua_newclass( L );
    return luaL_ref( L, LUA_REGISTRYINDEX );
}

LuaElement::LuaElement( lua_State *L ) : LuaClass( L, _trefget( L, this ) )
{
    // Root has to be initialized after the object's construction
    m_root = NULL;
}

LuaElement::~LuaElement()
{
}

void LuaElement::SetRoot( LuaClass *root )
{
    // We can also have no root
    // It is unnecessary to remove the parent if no more root
    if ( !( m_root = root ) )
        return;

    // Check whether we are in the root already
    // If so, we do not need to be reparented
    root->PushStack( m_lua );
    m_class->PushParent( m_lua );

    while ( lua_type( m_lua, -1 ) == LUA_TCLASS )
    {
        if ( lua_equal( m_lua, -2, -1 ) )
        {
            lua_pop( m_lua, 2 );
            return;
        }

        ILuaClass *parent = lua_refclass( m_lua, -1 );
        lua_pop( m_lua, 1 );

        parent->PushParent( m_lua );
    }

    lua_pop( m_lua, 2 );

    // Reparent it so we are in the correct tree
    m_class->PushMethod( m_lua, "setParent" );
    root->PushStack( m_lua );
    lua_call( m_lua, 1, 0 );
}