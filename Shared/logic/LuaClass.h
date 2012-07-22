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

    inline void Reference( lua_class_reference& ref )
    {
        PushStack( m_lua );
        new (&ref) lua_class_reference( m_lua, -1 );    // Do not construct the object in this scope
        lua_pop( m_lua, 1 );
    }

    inline void Reference( luaRefs& refs )
    {
        lua_class_reference *ref = (lua_class_reference*)malloc(sizeof(*ref));
        Reference( *ref );

        refs.push_back( ref );
    }

    inline lua_State* GetVM()
    {
        return m_lua;
    }

    inline void Destroy()
    {
        PushStack( m_lua );

        // Prevent .lua referencing
        ILuaClass& j = *lua_refclass( m_lua, lua_gettop( m_lua ) );
        j.ClearReferences( m_lua );

        j.PushMethod( m_lua, "destroy" );
        lua_call( m_lua, 0, 0 );
        lua_pop( m_lua, 1 );
    }

protected:
    lua_State*              m_lua;
    int                     m_ridx;
};

static inline void luaJ_extend( lua_State *L, int idx, int nargs )
{
    ILuaClass& j = *lua_refclass( L, idx );

    int offcl = -nargs - 1;

    // Make it class root
    j.PushEnvironment( L );
    lua_setfenv( L, offcl - 1 );

    lua_pushvalue( L, idx );

    if ( offcl != -1 )
        lua_insert( L, offcl );

    lua_call( L, nargs + 1, 0 );
}

#endif //_BASE_LUA_CLASS_