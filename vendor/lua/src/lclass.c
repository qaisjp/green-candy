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
#include <algorithm>

#include "lapi.h"
#include "lgc.h"
#include "lmem.h"
#include "lstate.h"
#include "ltable.h"
#include "lclass.h"
#include "lauxlib.h"
#include "lstring.h"
#include "lvm.h"

#define LUA_PROTECT_CLASS_META

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

    if ( c->destroyed )
        throw lua_exception( L, LUA_ERRRUN, "attempt to index the environment of a destroyed class" );

    const TValue *val = c->GetEnvValue( L->base + 1 );

    if ( ttype( val ) != LUA_TNIL )
    {
        setobj2s( L, L->top - 1, val );
        return 1;
    }

    TValue res;
    setnilvalue( &res );

    // Otherwise we result in the inherited environments
    for ( Class::envList_t::const_iterator iter = c->envInherit.begin(); iter != c->envInherit.end(); iter++ )
    {
        TValue tabVal;
        sethvalue( L, &tabVal, *iter );
    
        luaV_gettable( L, &tabVal, L->base + 1, &res );

        if ( res.tt != LUA_TNIL )
        {
            setobj2s( L, L->top++, &res );
            return 1;
        }
    }

    return 0;
}

void Class::IncrementMethodStack( lua_State *lua )
{
    inMethod++;
}

inline static void class_unlinkParent( lua_State *L, Class& j )
{
    // Unlink us from our parent, if present
    if ( !j.parent )
        return;

    j.childAPI->PushMethod( L, "destroy" );
    lua_call( L, 0, 0 );

    j.childAPI->DecrementMethodStack( L );

    j.childAPI = NULL;
    j.parent = NULL;
}

static int childapi_notifyDestroy( lua_State *L )
{
    Class& j = *jvalue( index2adr( L, lua_upvalueindex( 1 ) ) );

    const TValue *val = jvalue( index2adr( L, lua_upvalueindex( 2 ) ) )->GetSuperMethod( L );

    // First let all other handlers finish
    if ( ttype( val ) == LUA_TFUNCTION )
    {
        const StkId callObj = L->top++;
        setobj2s( L, callObj, val );
        luaD_call( L, callObj, 0 );
    }

    if ( j.childCount != 0 )
        return 0;

    class_unlinkParent( L, j );

    setclvalue( L, L->top++, j.destructor );
    lua_call( L, 0, 0 );
    return 0;
}

inline static bool class_preDestructor( lua_State *L, Class& j )
{
    if ( j.destroying )
        return false;

    // Make sure we do not try to destroy twice
    j.destroying = true;

    bool reqWorthy = false;

    // We potencially are operating at an unstable state here, since we use child_iter from
    // a destroyed class. It is a thrilling optimization not having to secure it from GC!
    // If crashes report at this loop, I will take care of it (very unlikely due to current GC architecture).
    LIST_FOREACH_BEGIN( Class, j.children.root, child_iter )
        item->PushMethod( L, "destroy" );
        luaD_call( L, L->top - 1, 0 );

        //TODO: This may be risky, I have to analyze whether GC may kill our runtime
        //REPORT: I have seen no errors so far!

        if ( !item->destroyed )
        {
            setjvalue( L, L->top++, &j );
            setjvalue( L, L->top++, item->childAPI );
            lua_pushcclosure( L, childapi_notifyDestroy, 2 );
            item->childAPI->RegisterMethod( L, "notifyDestroy" );

            reqWorthy = true;
        }
    LIST_FOREACH_END

    if ( !reqWorthy )
        class_unlinkParent( L, j );

    return !reqWorthy;
}

bool Class::PreDestructor( lua_State *L )
{
    return class_preDestructor( L, *this );
}

void Class::CheckDestruction( lua_State *L )
{
    // I am not sure whether the stack could corrupt.
    // This needs testing with exception handling
    // Yes, exceptions will not hinder the requested destruction of a class.

    // The new lua thread architecture preserves class referencing!
    // Now there is no issue with keeping a coroutine state to save a class
    // from destruction. Hahaha!
    // For this reason, be sure to prevent yield-deadlocks by using lua_yield_shield
    // because the Lua environment is coroutine-safe. A halted coroutine in a class callback
    // handler runtime will prevent the class' destruction until the runtime left the class'
    // methods!

    // Only problem could occur if the VM is short of memory.
    // Exceptions could be raised within exceptions, and I am unsure how to handle this.
    if ( reqDestruction && inMethod == 0 )
    {
        lua_yield_shield _ref( L ); // prevent destructor yielding

        if ( !class_preDestructor( L, *this ) )
            return;

        setclvalue( L, L->top, destructor );
        luaD_call( L, L->top++, 0 );
    }
}

void Class::DecrementMethodStack( lua_State *lua )
{
    inMethod--;

    CheckDestruction( lua );
}

// Used to clear referebces put by scripts; MTA clears them for
// every internal class prior to destruction
void Class::ClearReferences( lua_State *lua )
{
    if ( !refCount )
        return;

    inMethod -= refCount;

    refCount = 0;

    CheckDestruction( lua );
}

void Class::Push( lua_State *L )
{
    setjvalue( L, L->top++, this );
}

void Class::PushMethod( lua_State *L, const char *key )
{
    setobj( L, L->top++, luaH_getstr( methods, luaS_new( L, key ) ) );
}

void Class::SetTransmit( int type, void *entity )
{
	unsigned int idx = transCount++;

	// Grow the array
	trans = (trans_t*)luaM_realloc_( _lua, trans, idx * sizeof(trans_t), transCount * sizeof(trans_t) );

	// Append the item
    trans_t& item = trans[idx];
	item.id = (unsigned char)type;
	item.ptr = entity;
}

bool Class::GetTransmit( int type, void*& entity )
{
    if ( destroyed )
        return false;

	if ( !trans )
		return false;
   
    for ( unsigned int n=0; n < transCount; n++ )
	{
		trans_t& item = trans[n];

		if ( item.id == (unsigned char)type )
		{
			entity = item.ptr;
			return true;
		}
	}

	return false;
}

int Class::GetTransmit() const
{
	if ( !trans )
		return -1;

    return trans[transCount - 1].id;
}

bool Class::IsTransmit( int type ) const
{
	if ( !trans )
		return false;

    for ( unsigned int n=0; n < transCount; n++ )
	{
		trans_t& item = trans[n];

		if ( item.id == (unsigned char)type )
			return true;
	}

	return false;
}

bool Class::IsDestroying() const
{
    return destroying;
}

bool Class::IsDestroyed() const
{
    return destroyed;
}

bool Class::IsRootedIn( Class *root ) const
{
    Class *elder = parent;

    while ( elder )
    {
        if ( root == elder )
            return true;

        elder = elder->parent;
    }

    return false;
}

bool Class::IsRootedIn( lua_State *L, int idx ) const
{
    return IsRootedIn( jvalue( index2adr( L, idx ) ) );
}

void Class::PushEnvironment( lua_State *L )
{
    sethvalue( L, L->top++, env );
}

void Class::PushOuterEnvironment( lua_State *L )
{
    sethvalue( L, L->top++, outenv );
}

void Class::PushChildAPI( lua_State *L )
{
    if ( !parent )
    {
        setnilvalue( L->top++ );
        return;
    }

    setjvalue( L, L->top++, childAPI );
}

void Class::PushParent( lua_State *L )
{
    if ( !parent )
    {
        setnilvalue( L->top++ );
        return;
    }

    setjvalue( L, L->top++, parent );
}

const TValue* Class::GetEnvValue( const TValue *key )
{
    // Browse the read-only storage
    const TValue *res = luaH_get( internStorage, key );

    if ( ttype( res ) != LUA_TNIL )
        return res;

    return luaH_get( storage, key );
}

const TValue* Class::GetEnvValueString( lua_State *L, const char *name )
{
    const TString *key = luaS_new( L, name );
    const TValue *res = luaH_getstr( internStorage, key );

    if ( ttype( res ) != LUA_TNIL )
        return res;

    return luaH_getstr( storage, key );
}

void Class::RequestDestruction()
{
    reqDestruction = true;
}

TValue* Class::SetSuperMethod( lua_State *L )
{
    return luaH_setstr( L, internStorage, G(L)->superCached );
}

const TValue* Class::GetSuperMethod( lua_State *L )
{
    return luaH_getstr( internStorage, G(L)->superCached );
}

// Easy wrapper for C++ support
class MethodStackAllocation
{
public:
    MethodStackAllocation( lua_State *thread, Class *myClass, const TValue& val )
    {
        if ( myClass->destroyed )
            throw lua_exception( thread, LUA_ERRRUN, "attempt to access a destroyed class' method" );

        TValue *superMethod = myClass->SetSuperMethod( thread );

        setobj( thread, &m_prevSuper, superMethod );

        setobj( thread, superMethod, &val );
        luaC_barriert( thread, myClass->internStorage, &val );
        myClass->IncrementMethodStack( thread );

        m_instance = myClass;
        m_thread = thread;
    }

    ~MethodStackAllocation()
    {
        setobj( m_thread, m_instance->SetSuperMethod( m_thread ), &m_prevSuper );
        luaC_barriert( m_thread, m_instance->internStorage, &m_prevSuper );
        m_instance->DecrementMethodStack( m_thread );
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
    Class *j = jvalue( index2adr( L, lua_upvalueindex( 2 ) ) );
    MethodStackAllocation member( L, j, boolFalse );
    int top = lua_gettop( L );

    if ( top )
    {
        for ( int n=0; n<top; n++ )
            lua_pushvalue( L, n+1 );

        lua_pushvalue( L, lua_upvalueindex( 3 ) );
        lua_insert( L, top + 1 );
        lua_call( L, top, 0 );
    }
    else
    {
        lua_pushvalue( L, lua_upvalueindex( 3 ) );
        lua_call( L, 0, 0 );
    }

    lua_pushvalue( L, lua_upvalueindex( 1 ) );
    lua_insert( L, 1 );
    lua_call( L, top, 0 );
    return 0;
}

static int classmethod_fsFSCCHandler( lua_State *L )
{
    if ( lua_isnil( L, 1 ) )
        lua_pushcclosure( L, classmethod_forceSuperRoot, 2 );
    else
        lua_pushcclosure( L, classmethod_forceSuper, 3 );

    return 1;
}

// Register future method definitions for the Forced Super Calling Convention (FSCC)
static int classmethod_registerForcedSuper( lua_State *L )
{
    Class& j = *jvalue( index2adr( L, lua_upvalueindex( 1 ) ) );

    if ( j.destroyed )
        throw lua_exception( L, LUA_ERRRUN, "attempt to register a forced-super declaration inheritance for a destroyed class" );

    luaL_checktype( L, 1, LUA_TSTRING );

    lua_pushcclosure( L, classmethod_fsFSCCHandler, 0 );
    setclvalue( L, luaH_setstr( L, j.forceSuper, tsvalue( L->base ) ), clvalue( L->top - 1 ) );

    luaC_objbarriert( L, j.forceSuper, clvalue( L->top - 1 ) );
    return 0;
}

static int classmethod_envPutFront( lua_State *L )
{
    luaL_checktype( L, 1, LUA_TTABLE );

    jvalue( index2adr( L, lua_upvalueindex( 1 ) ) )->EnvPutFront( L );
    return 0;
}

static int classmethod_envPutBack( lua_State *L )
{
    luaL_checktype( L, 1, LUA_TTABLE );

    jvalue( index2adr( L, lua_upvalueindex( 1 ) ) )->EnvPutBack( L );
    return 0;
}

static int classmethod_envAcquireDispatcher( lua_State *L )
{
    Table *env;

    if ( lua_type( L, 1 ) == LUA_TTABLE )
        env = jvalue( index2adr( L, lua_upvalueindex( 1 ) ) )->AcquireEnvDispatcherEx( L, hvalue( L->base ) );
    else
        env = jvalue( index2adr( L, lua_upvalueindex( 1 ) ) )->AcquireEnvDispatcher( L );

    sethvalue( L, L->top++, env );
    return 1;
}

static int classmethod_setChild( lua_State *L )
{
    luaL_checktype( L, 1, LUA_TCLASS );

    Class& j = *jvalue( L->base );

    if ( j.parent != jvalue( index2adr( L, lua_upvalueindex( 1 ) ) ) )
    {
        setbvalue( L->top++, false );
        return 1;
    }

    setjvalue( L, L->top++, j.childAPI );
    return 1;
}

static const luaL_Reg internStorage_interface[] =
{
    { "registerForcedSuper", classmethod_registerForcedSuper },
    { "setChild", classmethod_setChild },
    { NULL, NULL }
};

static const luaL_Reg internStorage_envDispatch_interface[] =
{
    { "envPutFront", classmethod_envPutFront },
    { "envPutBack", classmethod_envPutBack },
    { "envAcquireDispatcher", classmethod_envAcquireDispatcher },
    { NULL, NULL }
};

static int classmethod_fsDestroyRoot( lua_State *L )
{
    Class& j = *jvalue( index2adr( L, lua_upvalueindex( 1 ) ) );

    if ( j.inMethod != 0 )
    {
        j.reqDestruction = true;
        return 0;
    }

    if ( j.destroyed )
        throw lua_exception( L, LUA_ERRRUN, "attempt to destroy a destroyed class" );

    if ( !class_preDestructor( L, j ) )
        return 0;

    lua_yield_shield _ref( L ); // prevent destructor yielding

    lua_pushvalue( L, lua_upvalueindex( 2 ) );
    lua_pcall( L, 0, 0, 0 );
    return 0;
}

static int classmethod_fsDestroySuper( lua_State *L )
{
    Class& j = *jvalue( index2adr( L, lua_upvalueindex( 2 ) ) );

    if ( j.inMethod != 0 )
    {
        j.reqDestruction = true;
        return 0;
    }

    if ( j.destroyed )
        throw lua_exception( L, LUA_ERRRUN, "attempted to destroy a destroyed class!" );

    if ( !class_preDestructor( L, j ) )
        return 0;

    lua_yield_shield _ref( L ); // prevent destructor yielding

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

    setclvalue( L, L->top - 3, j->destructor );

    lua_pushvalue( L, 3 );
    lua_pushvalue( L, 1 );
    lua_pushcclosure( L, classmethod_fsDestroyBridge, 2 );

    j->destructor = clvalue( L->top - 1 );
    luaC_forceupdatef( L, j->destructor );
    lua_settop( L, 3 );

    lua_pushcclosure( L, classmethod_fsDestroySuper, 3 );
    return 1;
}

static int dispatcher_index( lua_State *L )
{
    const TValue *res = jvalue( index2adr( L, lua_upvalueindex( 3 ) ) )->GetEnvValue( L->base + 1 );

    if ( ttype( res ) != LUA_TNIL )
    {
        setobj2s( L, L->top - 1, res );
        return 1;
    }

    lua_gettable( L, lua_upvalueindex( 2 ) );
    return 1;
}

static int dispatcher_newindex( lua_State *L )
{
    lua_settable( L, lua_upvalueindex( 1 ) );
    return 0;
}

static luaL_Reg dispatcher_interface[] =
{
    { "__index", dispatcher_index },
    { "__newindex", dispatcher_newindex },
    { NULL, NULL }
};

Table* Class::AcquireEnvDispatcherEx( lua_State *L, Table *curEnv )
{
    // If the environment matches any of the inherited ones, we skip creating a dispatcher
    if ( curEnv == env )
        return env;

    for ( envList_t::const_iterator iter = envInherit.begin(); iter != envInherit.end(); iter++ )
    {
        if ( curEnv == *iter )
            return env;
    }

    // Create the dispatcher which accesses class env first then this current one
    Table *dispatch = luaH_new( L, 0, 0 );
    Table *metaDispatch = luaH_new( L, 0, 0 );

    dispatch->metatable = metaDispatch;

    sethvalue( L, L->top++, metaDispatch );

    lua_pushboolean( L, false );
    lua_setfield( L, -2, "__metatable" );

    sethvalue( L, L->top++, env );
    sethvalue( L, L->top++, curEnv );
    setjvalue( L, L->top++, this );
    luaL_openlib( L, NULL, dispatcher_interface, 3 );

    L->top--;
    return dispatch;
}

Table* Class::AcquireEnvDispatcher( lua_State *L )
{
    return AcquireEnvDispatcherEx( L, hvalue( index2adr( L, LUA_ENVIRONINDEX ) ) );
}

Closure* Class::GetMethod( const TString *name, Table*& table )
{
    const TValue *val = luaH_getstr( internStorage, name );

    if ( val->tt == LUA_TFUNCTION )
    {
        table = internStorage;
        return clvalue( val );
    }

    val = luaH_getstr( methods, name );
    table = methods;
    return ttype( val ) == LUA_TFUNCTION ? clvalue( val ) : NULL;
}

void Class::SetMethod( lua_State *L, TString *name, Closure *method, Table *table )
{
    bool methWhite = iswhite( method ) != 0;

    // Cache it in the environment
    if ( table != internStorage )
    {
        setclvalue( L, luaH_setstr( L, storage, name ), method );
        
        if ( methWhite && isblack( storage ) )
            luaC_barrierback( L, storage );
    }
    
    setclvalue( L, luaH_setstr( L, table, name ), method );

    if ( methWhite && isblack( table ) )
        luaC_barrierback( L, table );
}

void Class::RegisterMethod( lua_State *L, TString *methName, bool handlers )
{
    Table *methTable;
    Closure *prevMethod = GetMethod( methName, methTable ); // Acquire the previous method

    // Apply the environment to the new method
    Closure *cl = clvalue( L->top - 1 );
    cl->env = AcquireEnvDispatcherEx( L, cl->env );
    luaC_objbarrier( L, cl, cl->env );

    if ( !handlers )
        goto defaultHandler;

	const TValue *fsVal = luaH_getstr( forceSuper, methName );

	if ( ttype( fsVal ) == LUA_TFUNCTION )
	{
		setclvalue( L, L->top++, clvalue( fsVal ) );

		if ( !prevMethod )
			lua_pushnil( L );
        else
            setclvalue( L, L->top++, prevMethod );

        setjvalue( L, L->top++, this );
		setclvalue( L, L->top++, cl );
		lua_call( L, 3, 1 );
	}
	else
	{
defaultHandler:
        if ( prevMethod )
            setclvalue( L, L->top++, prevMethod );

        setjvalue( L, L->top++, this );
		setclvalue( L, L->top++, cl );

		if ( prevMethod )
			lua_pushcclosure( L, classmethod_super, 3 );
		else
			lua_pushcclosure( L, classmethod_root, 2 );
	}

    // Store the new method
    SetMethod( L, methName, clvalue( L->top - 1 ), methTable );

    // Pop the raw function and the method
    L->top -= 2;
}

void Class::RegisterMethod( lua_State *L, TString *methName, lua_CFunction proto, _methodRegisterInfo& info, bool handlers )
{
    Table *methTable;
    Closure *prevMethod = GetMethod( methName, methTable ); // Acquire the previous method

    
}

void Class::RegisterMethod( lua_State *L, const char *name, bool handlers )
{
    return RegisterMethod( L, luaS_new( L, name ), handlers );
}

void Class::RegisterMethod( lua_State *L, const char *name, lua_CFunction proto, bool handlers )
{

}

void Class::RegisterMethodTrans( lua_State *L, const char *name, lua_CFunction proto, int trans, bool handlers )
{

}

void Class::RegisterLightMethod( lua_State *L, const char *_name )
{
    // Light methods go directly into the environment
    TString *name = luaS_new( L, _name );
    Closure *cl = clvalue( --L->top );

    // They go into storage only
    setclvalue( L, luaH_setstr( L, methods, name ), cl );
    luaC_objbarriert( L, methods, cl );

    setclvalue( L, luaH_setstr( L, storage, name ), cl );
    luaC_objbarriert( L, storage, cl );
}

void Class::RegisterLightMethodTrans( lua_State *L, const char *name, int trans )
{

}

static int classmethod_lightguard( lua_State *L )
{
    // Check for security
    Class& j = *jvalue( index2adr( L, lua_upvalueindex( 2 ) ) );

    if ( j.destroyed )
        throw lua_exception( L, LUA_ERRRUN, "attempt to access a destroyed class' light method" );

    return ((lua_CFunction)uvalue( index2adr( L, lua_upvalueindex( 3 ) )))( L );
}

void Class::RegisterLightInterface( lua_State *L, const luaL_Reg *intf, void *udata )
{
    while ( intf->name )
    {
        setpvalue( L->top++, udata );
        setjvalue( L, L->top++, this );
        setpvalue( L->top++, (void*)intf->func );
        lua_pushcclosure( L, classmethod_lightguard, 3 );

        // Apply environment dispatching
        Closure *cl = clvalue( L->top - 1 );
        cl->env = AcquireEnvDispatcherEx( L, cl->env );
        luaC_objbarrier( L, cl, cl->env );

        RegisterLightMethod( L, intf->name );

        intf++;
    }
}

void Class::RegisterLightInterfaceTrans( lua_State *L, const luaL_Reg *intf, void *udata, int trans )
{

}

void Class::EnvPutFront( lua_State *L )
{
    envInherit.insert( envInherit.begin(), hvalue( --L->top ) );
}

void Class::EnvPutBack( lua_State *L )
{
    envInherit.push_back( hvalue( --L->top ) );
}

static int classmethod_reference( lua_State *L )
{
    Class& j = *jvalue( index2adr( L, lua_upvalueindex( 1 ) ) );

    if ( j.destroying )
        return 0;

    j.inMethod++;
    j.refCount++;
    return 0;
}

static int classmethod_dereference( lua_State *L )
{
    Class& j = *jvalue( index2adr( L, lua_upvalueindex( 1 ) ) );

    if ( j.refCount == 0 )
        return 0;

    j.inMethod--;
    j.refCount--;

    j.CheckDestruction( L );
    return 0;
}

static int classmethod_isValidChild( lua_State *L )
{
    setbvalue( L->top++, lua_type( L, 1 ) == LUA_TCLASS );
    return 1;
}

static int classmethod_getChildren( lua_State *L )
{
    Class& j = *jvalue( index2adr( L, lua_upvalueindex( 1 ) ) );

    Table& tab = *luaH_new( L, j.childCount, 0 );
    unsigned int n = 1;

    LIST_FOREACH_BEGIN( Class, j.children.root, child_iter )
        setjvalue( L, luaH_setnum( L, &tab, n++ ), item );
        luaC_objbarriert( L, &tab, item );
    LIST_FOREACH_END

    sethvalue( L, L->top++, &tab );
    return 1;
}

static int childapi_destroy( lua_State *L )
{
    Class& j = *jvalue( index2adr( L, lua_upvalueindex( 1 ) ) );

    LIST_REMOVE( j.child_iter ); // Child
    j.parent->childCount--;  // Parent

    // We do not have to remove the childAPI from internStorage. Shall we?
    // Remove the parent link
    j.parent = NULL;

    // Tell this event to any possible registree
    // We use a seperate function so we can post this message after unlinking from
    // the parent's children!
    const TValue *method = j.childAPI->GetEnvValueString( L, "notifyDestroy" );

    if ( ttype( method ) != LUA_TNIL )
    {
        setobj2s( L, L->top, method );
        luaD_call( L, L->top++, 0 );
    }

    return 0;
}

static const luaL_Reg childapi_interface[] =
{
    { "destroy", childapi_destroy },
    { NULL, NULL }
};

static int childapi_constructor( lua_State *L )
{
    // The destruction has to be notified to every registree
    // Overwriting the method would otherwise bug the class children runtime
    // since parents want to be notified about their child's unlink event
    // so they can safely destroy themselves
    lua_getfield( L, LUA_ENVIRONINDEX, "registerForcedSuper" );
    lua_pushlstring( L, "notifyDestroy", 13 );
    lua_call( L, 1, 0 );

    lua_pushvalue( L, LUA_ENVIRONINDEX );
    lua_pushvalue( L, lua_upvalueindex( 1 ) );
    luaL_openlib( L, NULL, childapi_interface, 1 );

    luaJ_basicextend( L );
    return 0;
}

static int classmethod_setParent( lua_State *L )
{
    Class& j = *jvalue( index2adr( L, lua_upvalueindex( 1 ) ) );

    if ( j.destroying )
    {
        setbvalue( L->top++, false );
        return 1;
    }

    if ( lua_type( L, 1 ) != LUA_TCLASS )
    {
        // If we have not been passed a valid argument, i.e nil, remove our parent
        if ( j.parent )
        {
            j.childAPI->PushMethod( L, "destroy" );
            lua_call( L, 0, 0 );

            j.childAPI->DecrementMethodStack( L );
        }

        lua_pushboolean( L, true );
        return 1;
    }

    // We want our class as argument only
    lua_settop( L, 1 );

    Class& c = *jvalue( L->base );

    if ( &c == &j || c.destroying || c.destroyed )
    {
        setbvalue( L->top++, false );
        return 1;
    }

    // Prevent circle jerks or pervert child relationships
    if ( c.IsRootedIn( &j ) )
    {
        setbvalue( L->top++, false );
        return 1;
    }

    c.PushMethod( L, "isValidChild" );
    setjvalue( L, L->top++, &j );
    lua_call( L, 1, 1 );

    if ( !lua_toboolean( L, 2 ) )
    {
        setbvalue( L->top++, false );
        return 1;
    }

    if ( j.parent )
    {
        j.childAPI->DecrementMethodStack( L );

        j.childAPI->PushMethod( L, "destroy" );
        lua_call( L, 0, 0 );
    }

    setjvalue( L, --L->top - 1, &j );
    lua_pushcclosure( L, childapi_constructor, 1 );
    lua_newclassex( L, LCLASS_API_NOENVDISPATCH | LCLASS_NOPARENTING );

    // Store the API
    j.parent = &c;
    j.childAPI = jvalue( L->top - 1 );
    j.childAPI->IncrementMethodStack( L );

    // Allow internal storage access to it
    TValue *stm = luaH_setstr( L, j.internStorage, luaS_new( L, "childAPI" ) );
    setjvalue( L, stm, j.childAPI );
    luaC_objbarriert( L, j.internStorage, j.childAPI );

    // Call setChild in protected mode as the process may not be interupted anymore
    setobj( L, L->top++, luaH_getstr( c.internStorage, luaS_new( L, "setChild" ) ) );
    setjvalue( L, L->top++, &j );
    lua_pcall( L, 1, 0, 0 );

    // Add it to the parent's children list
    LIST_APPEND( c.children.root, j.child_iter );
    c.childCount++;

    setbvalue( L->top++, true );
    return 1;
}

static int classmethod_getParent( lua_State *L )
{
    Class *j = jvalue( index2adr( L, lua_upvalueindex( 1 ) ) );

    if ( !j->parent )
    {
        setbvalue( L->top++, false );
        return 1;
    }

    setjvalue( L, L->top++, j->parent );
    return 1;
}

static int classmethod_destructor( lua_State *L )
{
    Class *j = jvalue( index2adr( L, lua_upvalueindex( 1 ) ) );
 
    // Free unnecessary references.
    j->env->metatable = NULL;
    j->env = NULL;
    j->outenv->metatable = NULL;
    j->outenv = NULL;
    j->methods = NULL;
    j->storage = NULL;
    j->internStorage = NULL;
    j->destructor = NULL;

    j->destroyed = true;
    j->destroying = false;
    j->reqDestruction = false;
    return 0;
}

static const luaL_Reg classmethods_light[] = 
{
    { "reference", classmethod_reference },
    { "dereference", classmethod_dereference },
    { "isValidChild", classmethod_isValidChild },
    { "getChildren", classmethod_getChildren },
    { NULL, NULL }
};

static const luaL_Reg classmethods_parenting_light[] =
{
    { "getParent", classmethod_getParent },
    { NULL, NULL }
};

static const luaL_Reg classmethods_parenting[] =
{
    { "setParent", classmethod_setParent },
    { NULL, NULL }
};

static int methodenv_newindex( lua_State *L )
{
    Class& j = *jvalue( index2adr( L, lua_upvalueindex( 1 ) ) );

    // If the class leaked it's environment, outer range code might try to modify the class during obscure scenarios
    // We do not want that, as modification of a destroyed class is obscure!
    if ( j.destroyed )
        throw lua_exception( L, LUA_ERRRUN, "attempt to modify a destroyed class" );

    if ( lua_type( L, 2 ) == LUA_TSTRING )
    {
        int t = lua_type( L, 3 );

        if ( t == LUA_TFUNCTION )
        {
            // Register the method
            j.RegisterMethod( L, tsvalue( L->base + 1 ), true );
            return 0;
        }
    }

    const TValue *key = L->base + 1;

    if ( luaH_get( j.methods, key ) != luaO_nilobject )
        throw lua_exception( L, LUA_ERRRUN, "class methods cannot be overwritten" );

    const TValue *val = L->base + 2;

    setobj( L, luaH_set( L, j.storage, key ), val );
    luaC_barriert( L, j.storage, val );
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
        setjvalue( L, L->top++, j );
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

Class* luaJ_new( lua_State *L, int nargs, unsigned int flags )
{
    Class *c = new (L) Class;

    // Link it into the GC system
    c->next = G(L)->mainthread->next;
    G(L)->mainthread->next = c;

    Table *meta = luaH_new( L, 0, 0 );
    Table *outmeta = luaH_new( L, 0, 0 );

    c->tt = LUA_TCLASS;
    c->marked = luaC_white( G(L) ); // do not collect
    c->destroyed = false;
    c->destroying = false;
    c->reqDestruction = false;
    c->inMethod = 0;
    c->refCount = 0;
    c->trans = NULL;
	c->transCount = 0;
    c->parent = NULL;
    c->childCount = 0;
    LIST_CLEAR( c->children.root );
    c->destructor = NULL;

    // Set up the environments
    c->env = luaH_new( L, 0, 0 );
    c->outenv = luaH_new( L, 0, 0 );
    c->storage = luaH_new( L, 0, 0 );
    c->methods = luaH_new( L, 0, 0 );
    c->forceSuper = luaH_new( L, 0, 0 );
    c->internStorage = luaH_new( L, 0, 0 );

    c->env->metatable = meta;
    c->outenv->metatable = outmeta;

    // Perform a temporary keep
    ClassConstructionAllocation construction( L, c );

    // Register the previous environment
    lua_pushvalue( L, LUA_ENVIRONINDEX );
    c->EnvPutBack( L );

    // Init the forceSuper table
    sethvalue( L, L->top++, c->forceSuper );

    lua_pushcclosure( L, classmethod_fsDestroyHandler, 0 );
    lua_setfield( L, -2, "destroy" );

    L->top--;

    // Specify the outrange connection
    sethvalue( L, L->top++, outmeta );

#ifdef LUA_PROTECT_CLASS_META
    lua_pushboolean( L, false );
    lua_setfield( L, -2, "__metatable" );
#endif //LUA_PROTECT_CLASS_META

    sethvalue( L, L->top++, c->storage );
    sethvalue( L, L->top++, c->methods );

    luaL_openlib( L, NULL, envmeta_interface, 2 );
    L->top--;

    // Put the meta to stack
    sethvalue( L, L->top++, meta );

    // The upvalue has to be the class
    setjvalue( L, L->top++, c );

    // Create a class-only storage
    sethvalue( L, L->top++, c->internStorage );
    
    if ( !( flags & LCLASS_API_NO_ENV_AWARENESS ) )
    {
        // Distribute the environment
        sethvalue( L, L->top++, c->env );
        lua_setfield( L, -2, "_ENV" );
        sethvalue( L, L->top++, c->outenv );
        lua_setfield( L, -2, "_OUTENV" );
    }
    setjvalue( L, L->top++, c );
    lua_setfield( L, -2, "this" );

    // Set some internal functions
    setjvalue( L, L->top++, c );
    luaL_openlib( L, NULL, internStorage_interface, 1 );

    if ( !( flags & LCLASS_API_NOENVDISPATCH ) )
    {
        // Include the env dispatcher API
        setjvalue( L, L->top++, c );
        luaL_openlib( L, NULL, internStorage_envDispatch_interface, 1 );
    }

    // Pop the internal storage
    L->top--;

    lua_pushcclosure( L, methodenv_index, 1 );
    lua_setfield( L, -2, "__index" );

    // Register the method dispatcher
    setjvalue( L, L->top++, c );

    lua_pushcclosure( L, methodenv_newindex, 1 );
    lua_setfield( L, -2, "__newindex" );

#ifdef LUA_PROTECT_CLASS_META
    lua_pushboolean( L, false );
    lua_setfield( L, -2, "__metatable" );
#endif //LUA_PROTECT_CLASS_META

    // Replace the meta (on top) with the env
    sethvalue( L, L->top - 1, c->env );
    lua_setfenv( L, -nargs - 2 );

    sethvalue( L, L->top++, c->env );

    // Register the interface
    if ( !( flags & LCLASS_NOPARENTING ) )
    {
        // Add functionality for Lua parenting
        setjvalue( L, L->top++, c );
        luaL_openlib( L, NULL, classmethods_parenting, 1 );

        // Same goes for light functions
        c->RegisterLightInterface( L, classmethods_parenting_light, c );
    }

    // Register the light interface
    c->RegisterLightInterface( L, classmethods_light, c );

    sethvalue( L, L->top - 1, c->methods );

    // Prepare the destructor
    setjvalue( L, L->top++, c );

    setjvalue( L, L->top++, c );
    lua_pushcclosure( L, classmethod_destructor, 1 );

    c->destructor = clvalue( L->top - 1 );
    luaC_forceupdatef( L, c->destructor );

    lua_pushcclosure( L, classmethod_fsDestroyRoot, 2 );

    lua_pushvalue( L, -1 );

    lua_setfield( L, -3, "destroy" );

    sethvalue( L, L->top - 2, c->storage );
    lua_setfield( L, -2, "destroy" );

    L->top--;

    // Apply the environment to the constructor
    sethvalue( L, L->top++, c->env );
    lua_setfenv( L, -nargs - 2 );

    // Call the constructor (class as first arg)
    setjvalue( L, L->top++, c );
    lua_insert( L, -nargs - 1 );

    lua_call( L, nargs + 1, 0 );
    return c;
}

Class::~Class()
{
	luaM_realloc_( _lua, trans, transCount * sizeof(trans_t), 0 );
}

void luaJ_construct( lua_State *L, int nargs )
{
    Class *j = luaJ_new( L, nargs, 0 );

    setjvalue( L, L->top++, j );
}

// Basic protection for classes
static int protect_index( lua_State *L )
{
    size_t len;
    const char *str = lua_tolstring( L, 2, &len );

    if ( len > 1 && *(unsigned short*)str == 0x5f5f )
        return 0;

    lua_getfield( L, LUA_ENVIRONINDEX, "this" );
    lua_refclass( L, 3 )->PushOuterEnvironment( L );
    lua_pushvalue( L, 2 );
    lua_gettable( L, 4 );
    return 1;
}

static int protect_newindex( lua_State *L )
{
    return 0;
}

static const luaL_Reg bprotect_methods[] =
{
    { "__index", protect_index },
    { "__newindex", protect_newindex },
    { NULL, NULL }
};

void luaJ_basicprotect( lua_State *L )
{
    luaL_openlib( L, NULL, bprotect_methods, 0 );
}

static int extend_handler( lua_State *L )
{
    luaL_checktype( L, 1, LUA_TFUNCTION );

    // Make it class root
    lua_pushvalue( L, LUA_ENVIRONINDEX );
    lua_setfenv( L, 1 );

    lua_getfield( L, LUA_ENVIRONINDEX, "this" );
    lua_insert( L, 2 );

    lua_call( L, lua_gettop( L ) - 1, 0 );
    return 0;
}

void luaJ_basicextend( lua_State *L )
{
    lua_pushcclosure( L, extend_handler, 0 );
    lua_setfield( L, LUA_ENVIRONINDEX, "extend" );
}