/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        Shared/logic/LuaClass.h
*  PURPOSE:     Lua type instancing for VM objects
*  DEVELOPERS:  The_GTA <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#ifndef _BASE_LUA_CLASS_
#define _BASE_LUA_CLASS_

class LuaClass
{
public:
    LuaClass( lua_State *lua, int ridx )
    {
        m_lua = lua;
        m_ridx = ridx;
    }

    virtual ~LuaClass()
    {
        lua_pushnil( m_lua );
        lua_rawseti( m_lua, LUA_REGISTRYINDEX, m_ridx );
    }

    inline void PushStack( lua_State *L )
    {
        lua_rawgeti( L, LUA_REGISTRYINDEX, m_ridx );
    }

    void Reference( lua_class_reference& ref )
    {
        PushStack( m_lua );
        new (&ref) lua_class_reference( m_lua, -1 );    // Do not construct the object in this scope
        lua_pop( m_lua, 1 );
    }

    void Reference( luaRefs& refs )
    {
        lua_class_reference *ref = (lua_class_reference*)malloc(sizeof(*ref));
        Reference( *ref );

        refs.push_back( ref );
    }

    void Destroy()
    {
        PushStack( m_lua );
        lua_pushlstring( m_lua, "destroy", 7 );
        lua_gettable( m_lua, -2 );
        lua_call( m_lua, 0, 0 );
        lua_pop( m_lua, 1 );
    }

protected:
    lua_State*              m_lua;
    int                     m_ridx;
};

#endif //_BASE_LUA_CLASS_