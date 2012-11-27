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
    inline ILuaClass* acqcinf( lua_State *L, int ridx )
    {
        PushStack( L );
        ILuaClass *inf = lua_refclass( L, -1 );
        lua_pop( L, 1 );
        return inf;
    }

public:
    LuaClass( lua_State *lua, int ridx )
    {
        m_lua = lua_getmainstate( lua );    // for security reasons; main state will always be preserved
        m_ridx = ridx;
        m_class = acqcinf( lua, ridx );
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

    inline void PushMethod( lua_State *L, const char *name )
    {
        m_class->PushMethod( L, name );
    }

    inline void PushEnvironment( lua_State *L )
    {
        m_class->PushEnvironment( L );
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

    inline void IncrementMethodStack()
    {
        m_class->IncrementMethodStack( m_lua );
    }

    inline unsigned int GetRefCount() const
    {
        return m_class->GetRefCount();
    }

    inline void DecrementMethodStack()
    {
        m_class->DecrementMethodStack( m_lua );

        // Warning: may be destroyed at this point!
    }

    inline bool IsTransmit( int type )
    {
        return m_class->IsTransmit( type );
    }

    inline lua_State* GetVM()
    {
        return m_lua;
    }

    inline bool IsDestroying() const
    {
        return m_class->IsDestroying();
    }

    inline void Destroy()
    {
        // Prevent .lua referencing
        m_class->ClearReferences( m_lua );

        m_class->PushMethod( m_lua, "destroy" );
        lua_call( m_lua, 0, 0 );

        // At this point the class may be destroyed; do not use it anymore!
    }

    inline void Delete()
    {
        Destroy();
    }

protected:
    lua_State*              m_lua;
    int                     m_ridx;
    ILuaClass*              m_class;
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