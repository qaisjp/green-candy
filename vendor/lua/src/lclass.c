/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        vendor/lua/src/lclass.c
*  PURPOSE:     Lua class system extension
*  DEVELOPERS:  The_GTA <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#include "lua.h"

#include "lapi.h"
#include "lgc.h"
#include "lmem.h"
#include "lstate.h"
#include "ltable.h"
#include "lclass.h"
#include "lauxlib.h"
#include "lstring.h"

// Static TValue
static const TValue boolFalse = { 0, LUA_TBOOLEAN };

static int envmeta_index( lua_State *L )
{
    lua_gettable( L, lua_upvalueindex( 1 ) );
    return 1;
}

static int envmeta_newindex( lua_State *L )
{
    lua_pushvalue( L, 2 );
    lua_gettable( L, lua_upvalueindex( 2 ) );

    if ( lua_type( L, 4 ) != LUA_TNIL )
        throw lua_exception( L, LUA_ERRRUN, "cannot overwrite methods of class internals" );

    lua_settop( L, 3 );
    lua_settable( L, lua_upvalueindex( 1 ) );
    return 0;
}

static int methodenv_index( lua_State *L )
{
    Class *c = jvalue( index2adr( L, lua_upvalueindex( 1 ) ) );

    lua_pushvalue( L, 2 );
    lua_pushvalue( L, 2 );

    // Browse the read-only storage
    lua_rawget( L, lua_upvalueindex( 2 ) );

    if ( lua_type( L, 4 ) != LUA_TNIL )
        return 1;

    lua_pop( L, 1 );
    lua_gettable( L, lua_upvalueindex( 3 ) );

    // Check the internal memory
    if ( lua_type( L, 3 ) != LUA_TNIL )
        return 1;

    // Otherwise we result in the previous environment
    lua_pop( L, 1 );
    lua_gettable( L, lua_upvalueindex( 4 ) );
    return 1;
}

void Class::IncrementMethodStack( lua_State *lua )
{
    inMethod++;
}

void Class::DecrementMethodStack( lua_State *lua )
{
    inMethod--;

    // I am not sure whether the stack could corrupt.
    // This needs testing with exception handling
    // Yes, exceptions will not hinder the requested destruction of a class.
    if (reqDestruction && inMethod == 0)
    {
        setobj2s( lua, lua->top, &destructor );
        api_incr_top( lua );
        lua_call( lua, 0, 0 );
    }
}

TValue* Class::GetSuperMethod( lua_State *lua )
{
    return luaH_setstr( lua, internStorage, superCached );
}

// Easy wrapper for C++ support
class MethodStackAllocation
{
public:
    MethodStackAllocation( lua_State *thread, Class *myClass, const TValue& val )
    {
        if ( myClass->destroyed )
            throw lua_exception( thread, LUA_ERRRUN, "attempted to access a destroyed class' method" );

        TValue *superMethod = myClass->GetSuperMethod( thread );

        setobj( thread, &m_prevSuper, superMethod );

        setobj( thread, superMethod, &val );
        myClass->IncrementMethodStack( thread );

        m_instance = myClass;
        m_thread = thread;
    }

    ~MethodStackAllocation()
    {
        m_instance->DecrementMethodStack( m_thread );
        setobj( m_thread, m_instance->GetSuperMethod( m_thread ), &m_prevSuper );
    }

    Class*      m_instance;
    lua_State*  m_thread;
    TValue      m_prevSuper;
};

static int classmethod_root( lua_State *L )
{
    Class *j = (Class*)jvalue( index2adr( L, lua_upvalueindex( 1 ) ) );
    MethodStackAllocation member( L, j, boolFalse );
    int top = lua_gettop( L );

    lua_pushvalue( L, lua_upvalueindex( 2 ) );
    lua_insert( L, 1 );
    lua_call( L, top, LUA_MULTRET );
    return lua_gettop( L );
}

static int classmethod_super( lua_State *L )
{
    Class *j = (Class*)jvalue( index2adr( L, lua_upvalueindex( 2 ) ) );
    MethodStackAllocation member( L, j, *index2adr( L, lua_upvalueindex( 1 ) ) );
    int top = lua_gettop( L );

    lua_pushvalue( L, lua_upvalueindex( 3 ) );
    lua_insert( L, 1 );
    lua_call( L, top, LUA_MULTRET );
    return lua_gettop( L );
}

static int classmethod_forceSuperRoot( lua_State *L )
{
    Class *j = (Class*)jvalue( index2adr( L, lua_upvalueindex( 1 ) ) );
    MethodStackAllocation member( L, j, boolFalse );
    int top = lua_gettop( L );

    lua_pushvalue( L, lua_upvalueindex( 2 ) );
    lua_insert( L, 1 );
    lua_call( L, top, 0 );
    return 0;
}

static int classmethod_forceSuper( lua_State *L )
{
    Class *j = (Class*)jvalue( index2adr( L, lua_upvalueindex( 2 ) ) );
    MethodStackAllocation member( L, j, boolFalse );
    int top = lua_gettop( L );
    int n;

    for (n=0; n<top; n++)
        lua_pushvalue( L, n+1 );

    lua_pushvalue( L, lua_upvalueindex( 3 ) );
    lua_insert( L, 1 );
    lua_call( L, top, 0 );

    lua_pushvalue( L, lua_upvalueindex( 1 ) );
    lua_insert( L, 1 );
    lua_call( L, top, 0 );
    return 0;
}

static int classmethod_registerForcedSuper( lua_State *L )
{
    return 0;
}

static int classmethod_fsDestroyRoot( lua_State *L )
{
    Class *j = jvalue( index2adr( L, lua_upvalueindex( 1 ) ) );

    if ( j->inMethod != 0 )
    {
        j->reqDestruction = true;
        return 0;
    }

    // I think we do not need to check for a destroyed class here
    if ( j->reqDestruction )
        return 0;

    lua_pushvalue( L, lua_upvalueindex( 2 ) );
    lua_pcall( L, 0, 0, 0 );
    return 0;
}

static int classmethod_fsDestroySuper( lua_State *L )
{
    Class *j = jvalue( index2adr( L, lua_upvalueindex( 2 ) ) );

    if ( j->inMethod != 0 )
    {
        j->reqDestruction = true;
        return 0;
    }

    if ( j->destroyed )
        throw lua_exception( L, LUA_ERRRUN, "attempted to destroy a destroyed class!" );
    
    if ( j->reqDestruction )
        return 0;

    lua_pushvalue( L, lua_upvalueindex( 3 ) );
    lua_pcall( L, 0, 0, 0 );

    lua_pushvalue( L, lua_upvalueindex( 1 ) );
    lua_pcall( L, 0, 0, 0 );
    return 0;
}

static int classmethod_fsDestroyBridge( lua_State *L )
{
    // No check required, internal method
    lua_pushvalue( L, lua_upvalueindex( 1 ) );
    lua_pcall( L, 0, 0, 0 );

    lua_pushvalue( L, lua_upvalueindex( 2 ) );
    lua_pcall( L, 0, 0, 0 );
    return 0;
}

static int classmethod_fsDestroyHandler( lua_State *L )
{
    Class *j = jvalue( index2adr( L, 2 ) );

    setobj2s( L, L->top - 3, &j->destructor );

    lua_pushvalue( L, 3 );
    lua_pushvalue( L, 1 );
    lua_pushcclosure( L, classmethod_fsDestroyBridge, 2 );

    setobj( L, &j->destructor, L->top - 1 );
    lua_settop( L, 3 );

    lua_pushcclosure( L, classmethod_fsDestroySuper, 3 );
    return 1;
}

static int classmethod_reference( lua_State *L )
{
    Class *j = jvalue( index2adr( L, lua_upvalueindex( 1 ) ) );

    j->inMethod++;
    j->refCount++;
    return 0;
}

static int classmethod_dereference( lua_State *L )
{
    Class *j = jvalue( index2adr( L, lua_upvalueindex( 1 ) ) );

    j->inMethod--;
    j->refCount--;
    return 0;
}

static int classmethod_destructor( lua_State *L )
{
    Class *j = jvalue( index2adr( L, lua_upvalueindex( 1 ) ) );

    // TODO: children support
 
    j->env->metatable = NULL;
    j->env = NULL;
    j->outenv->metatable = NULL;
    j->outenv = NULL;
    j->methods = NULL;
    j->storage = NULL;
    j->destructor = luaO_nilobject_;

    j->destroyed = true;
    return 0;
}

static const luaL_Reg classmethods[] = 
{
    { "reference", classmethod_reference },
    { "dereference", classmethod_dereference },
    { NULL, NULL }
};

static int methodenv_newindex( lua_State *L )
{
    Class *j = jvalue( index2adr( L, lua_upvalueindex( 1 ) ) );
    int t = lua_type( L, 3 );

    if ( t == LUA_TFUNCTION )
    {
        // Acquire the previous method
        lua_pushvalue( L, 2 );
        lua_rawget( L, lua_upvalueindex( 2 ) );

        setjvalue( L, L->top, j );
        api_incr_top( L );

        lua_pushvalue( L, 3 );

        sethvalue( L, L->top, j->forceSuper );
        api_incr_top( L );
        lua_pushvalue( L, 2 );
        lua_rawget( L, 7 );

        if ( lua_type( L, 8 ) == LUA_TFUNCTION )
        {
            lua_insert( L, 4 );
            lua_settop( L, 7 );
            lua_call( L, 3, 1 );
        }
        else
        {
            lua_settop( L, 6 );

            if ( lua_type( L, 4 ) != LUA_TNIL )
                lua_pushcclosure( L, classmethod_super, 3 );
            else
            {
                lua_remove( L, 4 );
                lua_pushcclosure( L, classmethod_root, 2 );
            }
        }

        // Give it the environment
        sethvalue( L, L->top, j->env );
        api_incr_top( L );
        lua_setfenv( L, 3 );
        lua_remove( L, 3 );

        // We have to put it into the environment, too (caching <3)
        lua_pushvalue( L, 2 );
        lua_pushvalue( L, 3 );

        lua_settable( L, lua_upvalueindex( 2 ) );
        lua_settable( L, lua_upvalueindex( 3 ) );
        return 0;
    }

    lua_pushvalue( L, 2 );
    lua_rawget( L, lua_upvalueindex( 2 ) );

    if ( lua_type( L, 4 ) != LUA_TNIL )
        throw lua_exception( L, LUA_ERRRUN, "class methods cannot be overwritten" );

    lua_settop( L, 3 );

    lua_settable( L, lua_upvalueindex( 3 ) );
    return 0;
}

static const luaL_Reg envmeta_interface[] =
{
    { "__index", envmeta_index },
    { "__newindex", envmeta_newindex },
    { NULL, NULL }
};

class ClassConstructionAllocation
{
public:
    ClassConstructionAllocation( lua_State *L, Class *j )
    {
        setjvalue( L, L->top, j );
        api_incr_top( L );

        m_id = luaL_ref( L, LUA_REGISTRYINDEX );
        m_thread = L;
    }

    ~ClassConstructionAllocation()
    {
        luaL_unref( m_thread, LUA_REGISTRYINDEX, m_id );
    }

private:
    lua_State*  m_thread;
    int         m_id;
};

Class* luaJ_new( lua_State *L, int nargs )
{
    Class *c = luaM_new( L, Class );
    luaC_link( L, obj2gco( c ), LUA_TCLASS );   // Link it into the GC system
    Table *meta = luaH_new( L, 0, 0 );
    Table *outmeta = luaH_new( L, 0, 0 );

    c->destroyed = false;
    c->reqDestruction = false;
    c->inMethod = 0;
    c->refCount = 0;

    // Set up the environments
    c->env = luaH_new( L, 0, 0 );
    c->outenv = luaH_new( L, 0, 0 );
    c->storage = luaH_new( L, 0, 0 );
    c->methods = luaH_new( L, 0, 0 );
    c->forceSuper = luaH_new( L, 0, 0 );
    c->internStorage = luaH_new( L, 0, 0 );

    c->env->metatable = meta;
    c->outenv->metatable = outmeta;

    c->superCached = luaS_newlstr( L, "super", 5 );

    // Perform a temporary keep
    ClassConstructionAllocation construction( L, c );

    // Init the forceSuper table
    sethvalue( L, L->top, c->forceSuper );
    api_incr_top( L );

    lua_pushcclosure( L, classmethod_fsDestroyHandler, 0 );
    lua_setfield( L, -2, "destroy" );

    lua_pop( L, 1 );

    // Specify the outrange connection
    sethvalue( L, L->top, outmeta );
    api_incr_top( L );

    lua_pushboolean( L, false );
    lua_setfield( L, -2, "__metatable" );

    sethvalue( L, L->top, c->storage );
    api_incr_top( L );

    sethvalue( L, L->top, c->methods );
    api_incr_top( L );

    luaL_openlib( L, NULL, envmeta_interface, 2 );
    lua_pop( L, 1 );

    // Put the meta to stack
    sethvalue( L, L->top, meta );
    api_incr_top( L );

    // The upvalue has to be the class
    setjvalue( L, L->top, c );
    api_incr_top( L );

    // Create a read-only storage
    sethvalue( L, L->top, c->internStorage );
    api_incr_top( L );
    sethvalue( L, L->top, c->env );
    api_incr_top( L );
    lua_setfield( L, -2, "_ENV" );
    sethvalue( L, L->top, c->outenv );
    api_incr_top( L );
    lua_setfield( L, -2, "_OUTENV" );

    // Set some internal functions
    setjvalue( L, L->top, c );
    api_incr_top( L );
    
    lua_pushcclosure( L, classmethod_registerForcedSuper, 1 );
    lua_setfield( L, -2, "registerForcedSuper" );

    // Give it the iternal storage
    sethvalue( L, L->top, c->storage );
    api_incr_top( L );

    // We also need the previous environment
    lua_pushvalue( L, LUA_ENVIRONINDEX );

    lua_pushcclosure( L, methodenv_index, 4 );
    lua_setfield( L, -2, "__index" );

    // The upvalue has to be the class
    setjvalue( L, L->top, c );
    api_incr_top( L );

    // Method registry
    sethvalue( L, L->top, c->methods );
    api_incr_top( L );
    
    // Internal storage
    sethvalue( L, L->top, c->storage );
    api_incr_top( L );

    lua_pushcclosure( L, methodenv_newindex, 3 );
    lua_setfield( L, -2, "__newindex" );

    lua_pushboolean( L, false );
    lua_setfield( L, -2, "__metatable" );

    sethvalue( L, L->top - 1, c->env );
    lua_setfenv( L, -nargs - 2 );

    sethvalue( L, L->top, c->env );
    api_incr_top( L );

    // Register the interface
    setjvalue( L, L->top, c );
    api_incr_top( L );

    luaL_openlib( L, NULL, classmethods, 1 );

    sethvalue( L, L->top - 1, c->methods );

    // Prepare the destructor
    setjvalue( L, L->top, c );
    api_incr_top( L );

    setjvalue( L, L->top, c );
    api_incr_top( L );

    lua_pushcclosure( L, classmethod_destructor, 1 );
    setobj( L, &c->destructor, L->top - 1 );

    lua_pushcclosure( L, classmethod_fsDestroyRoot, 2 );

    lua_pushvalue( L, -1 );

    lua_setfield( L, -3, "destroy" );

    sethvalue( L, L->top - 2, c->storage );
    lua_setfield( L, -2, "destroy" );

    lua_pop( L, 1 );

    // Apply the environment to the constructor
    sethvalue( L, L->top, c->env );
    api_incr_top( L );
    lua_setfenv( L, -nargs - 1 );

    // Call the constructor
    lua_call( L, nargs, 0 );
    return c;
}

void luaJ_free( lua_State *L, Class *c )
{
    luaM_free( L, c );
}

void luaJ_construct( lua_State *L, int nargs )
{
    setjvalue( L, L->top - 1, luaJ_new( L, nargs ));
    api_incr_top( L );
}