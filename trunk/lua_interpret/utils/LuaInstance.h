/*****************************************************************************
*
*  PROJECT:     Lua Interpreter
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        utils/LuaInstance.h
*  PURPOSE:     MTA:Lua instance wrapper for less code repetition
*  DEVELOPERS:  Martin Turski <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#ifndef _LUA_INSTANCE_
#define _LUA_INSTANCE_

namespace lua_instance
{
    extern const luaL_Reg _interface[];
};

struct LuaInstance : public LuaClass
{
    static LUA_DECLARE( constructor )
    {
        LuaInstance *inst = (LuaInstance*)lua_touserdata( L, lua_upvalueindex( 1 ) );

        ILuaClass *j = lua_refclass( L, LUA_ENVIRONINDEX );
        
        lua_pushvalue( L, lua_upvalueindex( 1 ) );
        j->RegisterInterface( L, lua_instance::_interface, 1 );
        return 0;
    }

    static ILuaClass* _trefget( lua_State *L, LuaInstance *inst )
    {
        lua_pushlightuserdata( L, inst );
        lua_pushcclosure( L, constructor, 1 );
        lua_newclassex( L, LCLASS_API_LIGHT );

        ILuaClass *j = lua_refclass( L, -1 );
        lua_pop( L, 1 );
        return j;
    }

    LuaInstance( lua_State *L ) : LuaClass( L, _trefget( L, this ) )
    {
        keepAlive = true;
    }

    virtual ~LuaInstance( void )
    {
    }

    inline void Extend( lua_State *L, lua_CFunction constructor )
    {
        PushStack( L );
        lua_pushlightuserdata( L, this );
        lua_pushcclosure( L, constructor, 1 );
        luaJ_extend( L, -2, 0 );
        lua_pop( L, 1 );
    }

    static LUA_DECLARE( disable_setParent )
    {
        lua_pushboolean( L, false );
        return 1;
    }

    inline void DisableKeepAlive( void )
    {
        keepAlive = false;
    }

    void Finalize( lua_State *L )
    {
        m_class->RegisterMethod( L, "setParent", disable_setParent, 0, false );

        DisableKeepAlive();
    }

    void MarkGC( lua_State *L )
    {
        if ( keepAlive )
            LuaClass::MarkGC( L );
    }

    bool keepAlive;
};

#endif //_LUA_INSTANCE_