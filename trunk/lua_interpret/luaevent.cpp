/*****************************************************************************
*
*  PROJECT:     Lua Interpreter
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        luaevent.cpp
*  PURPOSE:     Lua event handling classes
*  DEVELOPERS:  The_GTA <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#include <StdInc.h>

using namespace LuaFunctionDefs;

static LUA_DECLARE( addEvent )
{
    lua_String name;

    LUA_ARGS_BEGIN;
    argStream.ReadString( name );
    LUA_ARGS_END;

    LuaEventSys *sys = (LuaEventSys*)lua_getmethodtrans( L );

    if ( LuaEvent *evt = sys->Get( name ) )
    {
        LUA_SUCCESS;
    }

    LuaEvent *evt = new LuaEvent;
    LIST_INSERT( sys->events.root, evt->node );
    LIST_CLEAR( evt->handlers.root );
    evt->name = name;
    
    LUA_SUCCESS;
}

static LUA_DECLARE( addEventHandler )
{
    lua_String name;
    LuaFunctionRef ref;

    LUA_ARGS_BEGIN;
    argStream.ReadString( name );
    argStream.ReadFunction( ref );
    argStream.ReadFunctionComplete();
    LUA_ARGS_END;

    LuaEventSys *sys = (LuaEventSys*)lua_getmethodtrans( L );
    LuaEvent *evt = sys->Get( name );

    LUA_CHECK( evt );

    LuaEventHandle *handle = new LuaEventHandle;
    LIST_INSERT( evt->handlers.root, handle->node );
    handle->ref = ref;
    
    LUA_SUCCESS;
}

static LUA_DECLARE( removeEventHandler )
{
    lua_String name;
    
    LUA_ARGS_BEGIN;
    argStream.ReadString( name );
    LUA_ARGS_END;

    LUA_CHECK( lua_type( L, 2 ) == LUA_TFUNCTION );

    LuaEventSys *sys = (LuaEventSys*)lua_getmethodtrans( L );
    LuaEvent *evt = sys->Get( name );

    LUA_CHECK( evt );

    const void *funcptr = lua_topointer( L, 2 );

    LIST_FOREACH_BEGIN( LuaEventHandle, evt->handlers.root, node )
        if ( item->ref.GetPointer() == funcptr )
        {
            LIST_REMOVE( item->node );

            delete item;

            lua_pushboolean( L, true );
            return 1;
        }
    LIST_FOREACH_END

    lua_pushboolean( L, false );
    return 1;
}

inline bool ExecuteEventHandlers( lua_State *L, LuaEventSys *sys, LuaManager *man, LuaEvent *evt, int top )
{
    bool retVal = true;

    LIST_FOREACH_BEGIN( LuaEventHandle, evt->handlers.root, node )
        man->PushReference( L, item->ref );
        
        for ( int n = 2; n <= top; n++ )
            lua_pushvalue( L, n );

        lua_call( L, top - 1, 1 );

        int type = lua_type( L, -1 );
        bool success;

        if ( type == LUA_TNIL )
            success = true;
        else
            success = lua_toboolean( L, -1 );

        lua_settop( L, top );

        if ( !success )
            retVal = false;
    LIST_FOREACH_END

    if ( LuaEventSys *parent = sys->parent )
    {
        LuaEvent *p_evt = parent->Get( evt->name.c_str() );

        if ( p_evt )
            ExecuteEventHandlers( L, parent, man, p_evt, top );
    }

    return retVal;
}

static LUA_DECLARE( triggerEvent )
{
    lua_String name;

    LUA_ARGS_BEGIN;
    argStream.ReadString( name );
    LUA_ARGS_END;

    LuaEventSys *sys = (LuaEventSys*)lua_getmethodtrans( L );
    LuaEvent *evt = sys->Get( name );

    LUA_CHECK( evt );
    
    lua_pushboolean( L, ExecuteEventHandlers( L, sys, lua_readmanager( L ), evt, lua_gettop( L ) ) );
    return 1;
}

static LUA_DECLARE( childAPI_notifyDestroy )
{
    LuaEventSys *childSys = (LuaEventSys*)lua_touserdata( L, lua_upvalueindex( 1 ) );

    childSys->parent = NULL;
    LIST_REMOVE( childSys->childNode );
    return 0;
}

static LUA_DECLARE( setChild )
{
    luaL_checktype( L, 1, LUA_TCLASS );

    LuaEventSys *sys = (LuaEventSys*)lua_getmethodtrans( L );

    lua_pushmethodsuper( L );
    lua_pushvalue( L, 1 );
    lua_call( L, 1, 1 );

    // Check that the child supports system events (only add if it does)
    ILuaClass *childAPI = lua_refclass( L, -1 );
    LuaEventSys *childSys;

    if ( childAPI->GetTransmit( LUACLASS_EVENTSYS, (void*&)childSys ) )
    {
        // Extend the child API
        lua_pushlightuserdata( L, childSys );
        childAPI->RegisterMethod( L, "notifyDestroy", childAPI_notifyDestroy, 1, true );
    }

    return 1;
}

static luaL_Reg eventsys_interface[] =
{
    LUA_METHOD( addEvent ),
    LUA_METHOD( addEventHandler ),
    LUA_METHOD( removeEventHandler ),
    LUA_METHOD( triggerEvent ),
    LUA_METHOD( setChild ),
    { NULL, NULL }
};

static LUA_DECLARE( destroy )
{
    delete (LuaEventSys*)lua_touserdata( L, lua_upvalueindex( 1 ) );

    return 0;
}

static luaL_Reg eventsys_interface_sys[] =
{
    LUA_METHOD( destroy ),
    { NULL, NULL }
};

static LUA_DECLARE( constructor )
{
    LuaEventSys *sys = (LuaEventSys*)lua_touserdata( L, lua_upvalueindex( 1 ) );

    ILuaClass *j = lua_refclass( L, LUA_ENVIRONINDEX );
    j->SetTransmit( LUACLASS_EVENTSYS, sys );

    j->RegisterInterfaceTrans( L, eventsys_interface, 0, LUACLASS_EVENTSYS );

    lua_pushvalue( L, lua_upvalueindex( 1 ) );
    j->RegisterInterface( L, eventsys_interface_sys, 1 );
    return 0;
}

static ILuaClass* _trefget( lua_State *L, LuaEventSys *sys )
{
    lua_pushlightuserdata( L, sys );
    lua_pushcclosure( L, constructor, 1 );
    lua_newclass( L );

    ILuaClass *j = lua_refclass( L, -1 );
    lua_pop( L, 1 );
    return j;
}

inline void common_init( LuaEventSys *sys )
{
    LIST_CLEAR( sys->events.root );
    LIST_CLEAR( sys->children.root );
    sys->parent = NULL;
}

LuaEventSys::LuaEventSys( lua_State *L ) : LuaClass( L, _trefget( L, this ) )
{
    common_init( this );
}

LuaEventSys::LuaEventSys( lua_State *L, ILuaClass *j ) : LuaClass( L, j )
{
    lua_pushlightuserdata( L, this );
    lua_pushcclosure( L, constructor, 1 );
    
    j->PushEnvironment( L );
    lua_setfenv( L, -2 );
    
    j->Push( L );
    lua_call( L, 1, 0 );

    common_init( this );
}

LuaEventSys::~LuaEventSys( void )
{
    // Delete all event registrations.
    LIST_FOREACH_BEGIN( LuaEvent, events.root, node )
        delete item;
    LIST_FOREACH_END
}

LuaEvent* LuaEventSys::Get( const char *name )
{
    LIST_FOREACH_BEGIN( LuaEvent, events.root, node )
        if ( item->name == name )
            return item;
    LIST_FOREACH_END

    return NULL;
}

void luaevent_extend( lua_State *L )
{
    new LuaEventSys( L, lua_refclass( L, LUA_ENVIRONINDEX ) );
}