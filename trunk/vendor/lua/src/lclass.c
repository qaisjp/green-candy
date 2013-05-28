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
#include "lfunc.h"
#include "lvm.h"
#include "ldebug.h"

class ClassMethodRegister : public VirtualClassEntry
{
public:
    ClassMethodRegister( lua_CFunction proto, Table *methTable ) : m_proto( proto ), m_methTable( methTable )
    {
    }

    inline void     BaseCall( lua_State *L, int argCount ) const
    {
        lua_xcopy( L, L, argCount );

        L->base += argCount;

        m_proto( L );

        L->base -= argCount;
    }

    virtual Closure*    Create( lua_State *L, Class *j ) const = 0;
    virtual void    Call( lua_State *L, Class *j, int argCount ) const = 0;

    void    MarkValue( global_State *g ) const
    {
        luaC_markobject( g, m_methTable );
    }

    Table*  GetMethodTable( void ) const
    {
        return m_methTable;
    }

    void* operator new( size_t size, lua_State *L, ClassStringTable *alloc, TString *methName )
    {
        return alloc->GetNodeValue( alloc->AllocateNode( L, methName, size ) );
    }

    void operator delete( void *ptr, lua_State *L, ClassStringTable *alloc, TString *methName )
    {
        alloc->DeleteNode( L, alloc->FindNode( methName ) );
    }

    void operator delete( void *ptr )
    {
        lua_assert( 0 );
    }

protected:
    lua_CFunction m_proto;
    Table* m_methTable;
};

__forceinline void class_allocateRegisterCache( lua_State *L, Class *j )
{
    if ( !j->methodCache )
        j->methodCache = new (L) ClassStringTable;
}

__forceinline Closure* class_getDelayedMethodRegister( lua_State *L, Class *j, TString *methName, bool excludeInternal )
{
    if ( !j->methodCache )
        return NULL;

    STableItemHeader *node = j->methodCache->FindNode( methName );

    if ( !node )
        return NULL;

    ClassMethodRegister *reg = (ClassMethodRegister*)j->methodCache->GetNodeValue( node );
    Table *regTable = reg->GetMethodTable();

    if ( excludeInternal && regTable == j->internStorage )
        return NULL;

    Closure *res = reg->Create( L, j );

    j->SetMethod( L, methName, res, reg->GetMethodTable() );
    j->methodCache->DeleteNode( L, node );
    return res;
}

__forceinline void class_readFromStorage( lua_State *L, Class *j, const TValue *key, StkId val, bool excludeInternal )
{
    const TValue *envRes = luaH_get( j->storage, key );

    if ( envRes != luaO_nilobject )
    {
        setobj( L, val, envRes );
        return;
    }
    else if ( ttype( key ) == LUA_TSTRING )
    {
        // Check the delayed method registry
        TString *methName = tsvalue( key );
        Closure *cached = class_getDelayedMethodRegister( L, j, methName, excludeInternal );

        if ( cached )
        {
            setclvalue( L, val, cached );
            return;
        }
    }

    setnilvalue( val );
}

__forceinline const TValue* class_getFromStorage( lua_State *L, Class *j, const TValue *key, bool excludeInternal )
{
    const TValue *envRes = luaH_get( j->storage, key );

    if ( envRes == luaO_nilobject )
    {
        if ( ttype( key ) == LUA_TSTRING )
        {
            // Check the delayed method registry
            TString *methName = tsvalue( key );
            Closure *cached = class_getDelayedMethodRegister( L, j, methName, excludeInternal );

            if ( cached )
            {
                TValue *slot = luaH_setstr( L, j->storage, methName );
                setclvalue( L, slot, cached );
                luaC_objbarriert( L, j->storage, cached );
                return slot;
            }
        }
    }

    return envRes;
}

__forceinline const TValue* class_getFromStorageString( lua_State *L, Class *j, TString *key, bool excludeInternal )
{
    const TValue *envRes = luaH_getstr( j->storage, key );

    if ( envRes == luaO_nilobject )
    {
        // Check the delayed method registry
        Closure *cached = class_getDelayedMethodRegister( L, j, key, excludeInternal );

        if ( cached )
        {
            TValue *slot = luaH_setstr( L, j->storage, key );
            setclvalue( L, slot, cached );
            luaC_objbarriert( L, j->storage, cached );
            return slot;
        }
    }

    return envRes;
}

void ClassOutEnvDispatch::Index( lua_State *L, const TValue *key, StkId val )
{
    class_readFromStorage( L, m_class, key, val, true );
}

__forceinline void class_checkMethodDelayOverride( lua_State *L, Class *j, const TValue *key )
{
    if ( j->methodCache && ttype( key ) == LUA_TSTRING )
    {
        if ( j->methodCache->FindNode( tsvalue( key ) ) )
            throw lua_exception( L, LUA_ERRRUN, "cannot overwrite methods of class internals" );
    }
}

void ClassOutEnvDispatch::NewIndex( lua_State *L, const TValue *key, StkId val )
{
    class_checkMethodDelayOverride( L, m_class, key );

    TValue *tabidx = luaH_set( L, m_class->storage, key );

    if ( ttype( tabidx ) == LUA_TFUNCTION )
        throw lua_exception( L, LUA_ERRRUN, "cannot overwrite methods of class internals" );

    setobj( L, tabidx, val );
    luaC_barriert( L, m_class->storage, val );
}

__forceinline void class_checkEnvIndex( lua_State *L, Class *j )
{
    if ( j->destroyed )
        throw lua_exception( L, LUA_ERRRUN, "attempt to index the environment of a destroyed class" );
}

void ClassEnvDispatch::Index( lua_State *L, const TValue *key, StkId value )
{
    class_checkEnvIndex( L, m_class );

    const TValue *val = m_class->GetEnvValue( L, key );

    if ( ttype( val ) != LUA_TNIL )
    {
        setobj( L, value, val );
        return;
    }

    TValue tabVal;

    // Otherwise we result in the inherited environments
    for ( Class::envList_t::const_iterator iter = m_class->envInherit.begin(); iter != m_class->envInherit.end(); iter++ )
    {
        (*iter)->Index( L, key, &tabVal );

        if ( ttype( &tabVal ) != LUA_TNIL )
        {
            setobj( L, value, &tabVal );
            return;
        }
    }

    // No return value.
    setnilvalue( value );
}

void Class::Index( lua_State *L, const TValue *key, StkId val )
{
    if ( destroyed )
        throw lua_exception( L, LUA_ERRRUN, "cannot index a destroyed class" );

    const TValue *tm = class_getFromStorageString( L, this, G(L)->tmname[TM_INDEX], true );

    if ( tm != luaO_nilobject )
    {
        if ( ttisfunction( tm ) )
        {
            TValue tmp;
            setobj( L, &tmp, key );

            ptrdiff_t storestk = savestack( L, val );
            luaD_checkstack( L, 3 );
            
            StkId func = L->top++;
            setclvalue( L, func, clvalue( tm ) );
            setjvalue( L, L->top++, this );
            setobj( L, L->top++, &tmp );
            luaD_call( L, func, 1 );

            setobj( L, restorestack( L, storestk ), --L->top );
        }
        else
            luaV_gettable( L, tm, key, val );
    }
    else
    {
        // We redirect our call to the outer environment
        outenv->Index( L, key, val );
    }
}

void Class::NewIndex( lua_State *L, const TValue *key, StkId val )
{
    if ( destroyed )
        throw lua_exception( L, LUA_ERRRUN, "cannot index a destroyed class" );

    const TValue *tm = class_getFromStorageString( L, this, G(L)->tmname[TM_NEWINDEX], true );

    if ( tm != luaO_nilobject )
    {
        if ( ttisfunction( tm ) )
        {
            // Temporary storage for security (problem: unknown location [stack, table, ...] + allocation)
            TValue _key, _val;
            setobj( L, &_key, key );
            setobj( L, &_val, val );

            luaD_checkstack( L, 4 );    // allocate a new stack

            StkId func = L->top++;  // remember the function stack id

            // Work with the new stack
            setclvalue( L, func, clvalue( tm ) );
            setjvalue( L, L->top++, this );
            setobj( L, L->top++, &_key );
            setobj( L, L->top++, &_val );
            luaD_call( L, func, 0 );
        }
        else
        {
            // Repeat the process with the index object
            // For security reasons, we should increment the callstack depth
            callstack_ref indexRef( *L );

            luaV_settable( L, tm, key, val );
        }
    }
    else
    {
        // We redirect our call to the outer environment
        outenv->NewIndex( L, key, val );
    }
}

void Class::IncrementMethodStack( lua_State *lua )
{
    lua_assert( !destroyed );

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

    // We potentially are operating at an unstable state here, since we use child_iter from
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
    lua_assert( !destroyed );

    setjvalue( L, L->top++, this );
}

void Class::PushMethod( lua_State *L, const char *key )
{
    lua_assert( !destroyed );

    const TValue *val = GetEnvValueString( L, key );

    if ( ttype( val ) != LUA_TFUNCTION )
    {
        setnilvalue( L->top++ );
    }
    else
    {
        setobj( L, L->top++, val );
    }
}

void Class::CallMethod( lua_State *L, const char *key )
{
    
}

void Class::SetTransmit( int type, void *entity )
{
    lua_assert( !destroyed );

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
   
    for ( unsigned int n = 0; n < transCount; n++ )
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

    for ( unsigned int n = 0; n < transCount; n++ )
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
    lua_assert( !destroyed );

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
    lua_assert( !destroyed );

    setgcvalue( L, L->top++, env );
}

void Class::PushOuterEnvironment( lua_State *L )
{
    lua_assert( !destroyed );

    setgcvalue( L, L->top++, outenv );
}

void Class::PushChildAPI( lua_State *L )
{
    lua_assert( !destroyed );

    if ( !parent )
    {
        setnilvalue( L->top++ );
        return;
    }

    setjvalue( L, L->top++, childAPI );
}

void Class::PushParent( lua_State *L )
{
    lua_assert( !destroyed );

    if ( !parent )
    {
        setnilvalue( L->top++ );
        return;
    }

    setjvalue( L, L->top++, parent );
}

const TValue* Class::GetEnvValue( lua_State *L, const TValue *key )
{
    // Browse the read-only storage
    const TValue *res = luaH_get( internStorage, key );

    if ( ttype( res ) != LUA_TNIL )
        return res;

    return class_getFromStorage( L, this, key, false );
}

const TValue* Class::GetEnvValueString( lua_State *L, const char *name )
{
    TString *key = luaS_new( L, name );
    const TValue *res = luaH_getstr( internStorage, key );

    if ( ttype( res ) != LUA_TNIL )
        return res;

    return class_getFromStorageString( L, this, key, false );
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

__forceinline void class_checkDestroyedMethodCall( lua_State *L, Class *j )
{
    if ( j->destroyed )
        throw lua_exception( L, LUA_ERRRUN, "attempt to access a destroyed class' method" );
}

// Easy wrapper for C++ support
class MethodStackAllocation
{
public:
    MethodStackAllocation( lua_State *thread, Class *myClass, Closure *prevSuper )
    {
        class_checkDestroyedMethodCall( thread, myClass );

        TValue *superMethod = myClass->SetSuperMethod( thread );

        setobj( thread, &m_prevSuper, superMethod );

        if ( prevSuper )
        {
            setclvalue( thread, superMethod, prevSuper );
            luaC_objbarriert( thread, myClass->internStorage, prevSuper );
        }
        else
            setbvalue( superMethod, false );

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
    CClosureMethodRedirect *cl = (CClosureMethodRedirect*)curr_func( L );
    MethodStackAllocation member( L, cl->m_class, NULL );
    int top = lua_gettop( L );

    setclvalue( L, L->top++, cl->redirect );
    lua_insert( L, 1 );
    lua_call( L, top, LUA_MULTRET );
    return lua_gettop( L );
}

static int classmethod_super( lua_State *L )
{
    CClosureMethodRedirectSuper *cl = (CClosureMethodRedirectSuper*)curr_func( L );
    MethodStackAllocation member( L, cl->m_class, cl->super );
    int top = lua_gettop( L );

    setclvalue( L, L->top++, cl->redirect );
    lua_insert( L, 1 );
    lua_call( L, top, LUA_MULTRET );
    return lua_gettop( L );
}

// Very fast C++-only prototypes
static int classmethod_rootNative( lua_State *L )
{
    CClosureMethodBase *cl = (CClosureMethodBase*)curr_func( L );
    MethodStackAllocation member( L, cl->m_class, NULL );

    return cl->method( L );
}

static int classmethod_superNative( lua_State *L )
{
    CClosureMethodBase *cl = (CClosureMethodBase*)curr_func( L );
    MethodStackAllocation member( L, cl->m_class, cl->super );

    return cl->method( L );
}

static int classmethod_rootTransNative( lua_State *L )
{    
    CClosureMethodTrans *cl = (CClosureMethodTrans*)curr_func( L );
    Class *j = cl->m_class;
    MethodStackAllocation member( L, j, NULL );

    if ( !j->GetTransmit( cl->trans, cl->data ) )
        throw lua_exception( L, LUA_ERRRUN, "attempt to call a method whose native interface was destroyed" );

    return cl->method( L );
}

static int classmethod_superTransNative( lua_State *L )
{
    CClosureMethodTrans *cl = (CClosureMethodTrans*)curr_func( L );
    Class *j = cl->m_class;
    MethodStackAllocation member( L, j, cl->super );

    if ( !j->GetTransmit( cl->trans, cl->data ) )
        throw lua_exception( L, LUA_ERRRUN, "attempt to call a method whose native interface was destroyed" );

    return cl->method( L );
}

static int classmethod_forceSuperRoot( lua_State *L )
{
    CClosureMethodRedirect *cl = (CClosureMethodRedirect*)curr_func( L );
    MethodStackAllocation member( L, cl->m_class, NULL );

    setclvalue( L, L->top++, cl->redirect );
    lua_insert( L, 1 );
    luaD_call( L, L->base, 0 );
    return 0;
}

static int classmethod_forceSuper( lua_State *L )
{
    CClosureMethodRedirectSuper *cl = (CClosureMethodRedirectSuper*)curr_func( L );
    MethodStackAllocation member( L, cl->m_class, NULL );
    int top = lua_gettop( L );

    // Push the current method which we must call
    setclvalue( L, L->top++, cl->redirect );

    // Push arguments (if any)
    for ( int n = 0; n < top; n++ )
        lua_pushvalue( L, n+1 );

    lua_call( L, top, 0 );

    // Now call the super method
    setclvalue( L, L->top++, cl->super );
    lua_insert( L, 1 );
    luaD_call( L, L->base, 0 );
    return 0;
}

static int classmethod_forceSuperRootNative( lua_State *L )
{
    // Well, should do the same
    classmethod_rootNative( L );
    return 0;
}

static int classmethod_forceSuperNative( lua_State *L )
{
    CClosureMethodBase *cl = (CClosureMethodBase*)curr_func( L );
    MethodStackAllocation member( L, cl->m_class, NULL );
    int top = lua_gettop( L );

    // Copy arguments while protecting slots
    for ( int n = 0; n < top; n++ )
        setobj( L, L->top++, L->base++ );

    cl->method( L );
    lua_settop( L, 0 ); // restore the stack

    // Unprotect slots
    L->base -= top;

    setclvalue( L, L->top++, cl->super );
    lua_insert( L, 1 );
    luaD_call( L, L->base, 0 );
    return 0;
}

static int classmethod_forceSuperRootTransNative( lua_State *L )
{
    CClosureMethodTrans *cl = (CClosureMethodTrans*)curr_func( L );
    Class *j = cl->m_class;
    MethodStackAllocation member( L, j, NULL );

    if ( !j->GetTransmit( cl->trans, cl->data ) )
        throw lua_exception( L, LUA_ERRRUN, "attempt to call a method whose native interface was destroyed" );

    cl->method( L );
    return 0;
}

static int classmethod_forceSuperTransNative( lua_State *L )
{
    CClosureMethodTrans *cl = (CClosureMethodTrans*)curr_func( L );

    if ( !cl->m_class->GetTransmit( cl->trans, cl->data ) )
    {
        setclvalue( L, L->top++, cl );
        lua_insert( L, 1 );
        luaD_call( L, L->base, 0 );
        return 0;
    }

    classmethod_forceSuperNative( L );
    return 0;
}

static Closure* classmethod_fsFSCCHandler( lua_State *L, Closure *newMethod, Class *j, Closure *prevMethod )
{
    if ( !prevMethod )
    {
        CClosureMethodRedirect *cl = luaF_newCmethodredirect( L, j->env, newMethod, j );
        cl->f = classmethod_forceSuperRoot;
        return cl;
    }
    else
    {
        CClosureMethodRedirectSuper *cl = luaF_newCmethodredirectsuper( L, j->env, newMethod, j, prevMethod );
        cl->f = classmethod_forceSuper;
        return cl;
    }
}

static Closure* classmethod_fsFSCCHandlerNative( lua_State *L, lua_CFunction proto, Class *j, Closure *prevMethod, _methodRegisterInfo& info )
{
    int num = info.numUpValues;

    if ( info.isTrans )
    {
        CClosureMethodTrans *cl = luaF_newCmethodtrans( L, num, j->env, j, info.transID );

        if ( prevMethod )
        {
            cl->super = prevMethod;
            cl->f = classmethod_forceSuperTransNative;
        }
        else
        {
            cl->super = NULL;
            cl->f = classmethod_forceSuperRootTransNative;
        }

        cl->method = proto;

        while ( num-- )
            setobj( L, &cl->upvalues[num], L->top-- );

        return cl;
    }
    else
    {
        CClosureMethod *cl = luaF_newCmethod( L, num, j->env, j );

        if ( prevMethod )
        {
            cl->super = prevMethod;
            cl->f = classmethod_forceSuperNative;
        }
        else
        {
            cl->super = NULL;
            cl->f = classmethod_forceSuperRootNative;
        }

        cl->method = proto;

        while ( num-- )
            setobj( L, &cl->upvalues[num], L->top-- );

        return cl;
    }
}

// Register future method definitions for the Forced Super Calling Convention (FSCC)
static int classmethod_registerForcedSuper( lua_State *L )
{
    Class& j = *((CClosureMethodBase*)curr_func( L ))->m_class;

    if ( j.destroyed )
        throw lua_exception( L, LUA_ERRRUN, "attempt to register a forced-super declaration inheritance for a destroyed class" );

    luaL_checktype( L, 1, LUA_TSTRING );

    Class::forceSuperItem item;
    item.cb = classmethod_fsFSCCHandler;
    item.cbNative = classmethod_fsFSCCHandlerNative;
    j.forceSuper->SetItem( L, tsvalue( L->base ), item );
    return 0;
}

static inline void luaJ_envtypeerror( lua_State *L, int type )
{
    lua_pushstring( L, "class environment has to be a real object (got " );
    lua_pushstring( L, lua_typename( L, type ) );
    lua_pushstring( L, ")" );
    lua_concat( L, 3 );
    lua_error( L );
}

static int classmethod_envPutFront( lua_State *L )
{
    if ( !iscollectable( L->top - 1 ) )
        luaJ_envtypeerror( L, ttype( L->top - 1 ) );

    ((CClosureMethodBase*)curr_func( L ))->m_class->EnvPutFront( L );
    return 0;
}

static int classmethod_envPutBack( lua_State *L )
{
    if ( !iscollectable( L->top - 1 ) )
        luaJ_envtypeerror( L, ttype( L->top - 1 ) );

    ((CClosureMethodBase*)curr_func( L ))->m_class->EnvPutBack( L );
    return 0;
}

static int classmethod_envAcquireDispatcher( lua_State *L )
{
    Dispatch *env;

    if ( iscollectable( L->base ) )
        env = ((CClosureMethodBase*)curr_func( L ))->m_class->AcquireEnvDispatcherEx( L, gcvalue( L->base ) );
    else
        env = ((CClosureMethodBase*)curr_func( L ))->m_class->AcquireEnvDispatcher( L );

    setqvalue( L, L->top++, env );
    return 1;
}

static int classmethod_setChild( lua_State *L )
{
    luaL_checktype( L, 1, LUA_TCLASS );

    Class& j = *jvalue( L->base );

    if ( j.parent != ((CClosureMethodBase*)curr_func( L ))->m_class )
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

static Closure* classmethod_fsDestroyHandler( lua_State *L, Closure *newMethod, Class *j, Closure *prevMethod )
{
    // We drop prevMethod, since we apply a guardian before the destructor
    Closure *prevDest = j->destructor;

    // Contruct the bridge
    CClosureBasic *cl = luaF_newCclosure( L, 2, j->env );
    setclvalue( L, &cl->upvalues[0], newMethod );
    setclvalue( L, &cl->upvalues[1], prevDest );
    cl->f = classmethod_fsDestroyBridge;
    cl->isEnvLocked = true;

    j->destructor = cl;
    luaC_forceupdatef( L, cl );

    // The actual new constructor we apply to the class
    CClosureBasic *retCl = luaF_newCclosure( L, 3, j->env );
    setclvalue( L, &retCl->upvalues[0], prevDest );
    setjvalue( L, &retCl->upvalues[1], j );
    setclvalue( L, &retCl->upvalues[2], newMethod );
    retCl->f = classmethod_fsDestroySuper;
    retCl->isEnvLocked = true;
    return retCl;
}

static Closure* classmethod_fsDestroyHandlerNative( lua_State *L, lua_CFunction proto, Class *j, Closure *prevMethod, _methodRegisterInfo& info )
{
    CClosureBasic *cl = luaF_newCclosure( L, info.numUpValues, j->env );
    unsigned char num = info.numUpValues;

    L->top -= num;

    for ( unsigned char n = 0; n < num; n++ )
        setobj( L, &cl->upvalues[n], L->top - 1 + n );

    cl->f = proto;
    cl->isEnvLocked = true;

    return classmethod_fsDestroyHandler( L, cl, j, prevMethod );
}

void ClassMethodDispatch::Index( lua_State *L, const TValue *key, StkId val )
{
    if ( !m_class->IsDestroyed() )
    {
        const TValue *res = m_class->GetEnvValue( L, key );

        if ( ttype( res ) != LUA_TNIL )
        {
            setobj( L, val, res );
            return;
        }
    }

    m_prevEnv->Index( L, key, val );
}

void ClassMethodDispatch::NewIndex( lua_State *L, const TValue *key, StkId val )
{
    class_checkEnvIndex( L, m_class );

    m_class->env->NewIndex( L, key, val );
}

Dispatch* Class::AcquireEnvDispatcherEx( lua_State *L, GCObject *curEnv )
{
    // If the environment matches any of the inherited ones, we skip creating a dispatcher
    if ( curEnv == env )
        return env;

    for ( envList_t::const_iterator iter = envInherit.begin(); iter != envInherit.end(); iter++ )
    {
        if ( curEnv == *iter )
            return env;
    }

    // Create the dispatcher which accesses class env first then curEnv
    return luaQ_newclassmethodenv( L, this, curEnv );
}

Dispatch* Class::AcquireEnvDispatcher( lua_State *L )
{
    return AcquireEnvDispatcherEx( L, gcvalue( index2adr( L, LUA_ENVIRONINDEX ) ) );
}

Closure* Class::GetMethod( lua_State *L, const TString *name, Table*& table )
{
    const TValue *val = luaH_getstr( internStorage, name );

    if ( ttype( val ) == LUA_TFUNCTION )
    {
        table = internStorage;
        return clvalue( val );
    }

    val = luaH_getstr( storage, name );

    if ( ttype( val ) == LUA_TFUNCTION )
    {
        table = storage;
        return clvalue( val );
    }

    if ( methodCache )
    {
        STableItemHeader *node = methodCache->FindNode( name );

        if ( node )
        {
            ClassMethodRegister *reg = (ClassMethodRegister*)methodCache->GetNodeValue( node );
            table = reg->GetMethodTable();
            
            Closure *cl = reg->Create( L, this );
            methodCache->DeleteNode( L, node );     // remove as it no longer serves a purpose

            return cl;
        }
    }

    table = storage;
    return NULL;
}

void Class::SetMethod( lua_State *L, TString *name, Closure *method, Table *table )
{
    bool methWhite = iswhite( method ) != 0;
    
    setclvalue( L, luaH_setstr( L, table, name ), method );

    if ( methWhite && isblack( table ) )
        luaC_barrierback( L, table );
}

__forceinline void Class::RegisterMethod( lua_State *L, TString *methName, bool handlers )
{
    Table *methTable;
    Closure *prevMethod = GetMethod( L, methName, methTable ); // Acquire the previous method

    // Get the method closure and check whether we can apply it
    Closure *cl = clvalue( L->top - 1 );

    // We cannot apply the method if the environment is locked.
    // It usually is an indicator that the closure is a class method already.
    if ( cl->IsEnvLocked() )
        throw lua_exception( L, LUA_ERRRUN, "attempt to set a class method whose environment is locked" );

    // Apply the environment to the new method
    cl->env = AcquireEnvDispatcherEx( L, cl->env );
    luaC_objbarrier( L, cl, cl->env );

    // Lock its environment so the scripter cannot break the class system.
    cl->isEnvLocked = true;

    Closure *handler;

    if ( !handlers )
        goto defaultHandler;

    Class::forceSuperItem *item = forceSuper->GetItem( methName );

    if ( item && item->cb )
    {
        handler = item->cb( L, cl, this, prevMethod );
	}
	else
	{
defaultHandler:
        CClosure *meth;

        if ( prevMethod )
        {
            CClosureMethodRedirectSuper *redirect = luaF_newCmethodredirectsuper( L, env, cl, this, prevMethod );
            redirect->f = classmethod_super;

            meth = redirect;
        }
        else
        {
            CClosureMethodRedirect *redirect = luaF_newCmethodredirect( L, env, cl, this );
            redirect->f = classmethod_root;

            meth = redirect;
        }

        handler = meth;
	}

    // Store the new method
    SetMethod( L, methName, handler, methTable );

    // Pop the raw function
    L->top--;
}

inline void _copyUpvaluesFromStack( lua_State *L, TValue *src, TValue *upval_array, int num )
{
    for ( int n = 0; n < num; n++ )
        setobj( L, upval_array, src + n );
}

__forceinline Closure* class_createBaseMethod( lua_State *L, lua_CFunction proto, Closure *prevMethod, TValue *upvalue_src, unsigned char numUpValues, GCObject *env, Class *j )
{
    CClosureMethod *meth = luaF_newCmethod( L, numUpValues, env, j );

    if ( prevMethod )
    {
        meth->super = prevMethod;
        meth->f = classmethod_superNative;
    }
    else
    {
        meth->super = NULL;
        meth->f = classmethod_rootNative;
    }

    _copyUpvaluesFromStack( L, upvalue_src, meth->upvalues, numUpValues );

    meth->method = proto;
    return meth;
}

__forceinline Closure* class_createMethodTrans( lua_State *L, lua_CFunction proto, Closure *prevMethod, TValue *upvalue_src, unsigned char numUpValues, GCObject *env, Class *j, int trans )
{
    CClosureMethodTrans *meth = luaF_newCmethodtrans( L, numUpValues, env, j, trans );

    if ( prevMethod )
    {
        meth->super = prevMethod;
        meth->f = classmethod_superTransNative;
    }
    else
    {
        meth->super = NULL;
        meth->f = classmethod_rootTransNative;
    }

    _copyUpvaluesFromStack( L, upvalue_src, meth->upvalues, numUpValues );

    meth->method = proto;
    return meth;
}

template <bool handlers = true>
class StatelessMethodRegister : public ClassMethodRegister
{
public:
    StatelessMethodRegister( lua_CFunction proto, Table *methTable ) : ClassMethodRegister( proto, methTable )
    {
    }

    Closure* Create( lua_State *L, Class *j ) const
    {
        return class_createBaseMethod( L, m_proto, NULL, NULL, 0, j->env, j );
    }

    void    Call( lua_State *L, Class *j, int argCount ) const
    {
        class_checkDestroyedMethodCall( L, j );

        BaseCall( L, argCount );
    }

    size_t  GetSize( void ) const
    {
        return sizeof(*this);
    }
};

template <bool handlers = true>
class TransMethodRegister : public ClassMethodRegister
{
public:
    TransMethodRegister( lua_CFunction proto, Table *methTable, int transID ) : ClassMethodRegister( proto, methTable ), m_transID( transID )
    {
    }

    Closure* Create( lua_State *L, Class *j ) const
    {
        // Optimization: do not create the closure if the class does not have the type anymore.
        if ( !j->IsTransmit( m_transID ) )
            return NULL;

        return class_createMethodTrans( L, m_proto, NULL, NULL, 0, j->env, j, m_transID );
    }

    // TODO: Need to work on the transition data availability for this to work.
    // Currently we grab transition data from a special closure class.
    // Need to change it in a special way.
    void    Call( lua_State *L, Class *j, int argCount ) const
    {
        class_checkDestroyedMethodCall( L, j );
        //class_checkTransitionType( L, j, m_transID );

        BaseCall( L, argCount );
    }

    size_t  GetSize( void ) const
    {
        return sizeof(*this);
    }

    int m_transID;
};

__forceinline void class_setMethod( lua_State *L, Class *j, TString *methName, lua_CFunction proto, Closure *prevMethod, Table *methTable, _methodRegisterInfo& info, bool handlers = false )
{
    Closure *handler;

    if ( !handlers )
        goto defaultHandler;

    Class::forceSuperItem *item = j->forceSuper->GetItem( methName );

    if ( item && item->cbNative )
    {
        handler = item->cbNative( L, proto, j, prevMethod, info );
	}
	else
	{
defaultHandler:
        unsigned char num = info.numUpValues;

        if ( num == 0 && !prevMethod )
        {
            class_allocateRegisterCache( L, j );

            if ( handlers )
            {
                if ( info.isTrans )
                    new (L, j->methodCache, methName) TransMethodRegister <true> ( proto, methTable, info.transID );
                else
                    new (L, j->methodCache, methName) StatelessMethodRegister <true> ( proto, methTable );
            }
            else
            {
                if ( info.isTrans )
                    new (L, j->methodCache, methName) TransMethodRegister <false> ( proto, methTable, info.transID );
                else
                    new (L, j->methodCache, methName) StatelessMethodRegister <false> ( proto, methTable );
            }

            // Do not continue registration process.
            // Like a coroutine we are halting the registration process here
            // to continue it on demand.
            // It is a very optimized coroutine, as there is no stack.
            // * I had an idea about creation of optimizable coroutines + prototypes.
            return;
        }
        else
        {
            if ( info.isTrans )
                handler = class_createMethodTrans( L, proto, prevMethod, L->top - num, num, j->env, j, info.transID );
            else
                handler = class_createBaseMethod( L, proto, prevMethod, L->top - num, num, j->env, j );
        }
	}

    // Store the new method
    j->SetMethod( L, methName, handler, methTable );
}

__forceinline void Class::RegisterMethod( lua_State *L, TString *methName, lua_CFunction proto, _methodRegisterInfo& info, bool handlers )
{
    Table *methTable;
    Closure *prevMethod = GetMethod( L, methName, methTable ); // Acquire the previous method

    class_setMethod( L, this, methName, proto, prevMethod, methTable, info, handlers );

    // Pop the upvalues
    L->top -= info.numUpValues;
}

__forceinline void Class::RegisterMethodAt( lua_State *L, TString *methName, lua_CFunction proto, Table *methodTable, _methodRegisterInfo& info, bool handlers )
{
    Table *methTable;
    Closure *prevMethod = GetMethod( L, methName, methTable );

    // Use the user-specified methodTable
    class_setMethod( L, this, methName, proto, prevMethod, methodTable, info, handlers );

    // Pop the upvalues
    L->top -= info.numUpValues;
}

void Class::RegisterMethod( lua_State *L, const char *name, bool handlers )
{
    RegisterMethod( L, luaS_new( L, name ), handlers );
}

void Class::RegisterMethod( lua_State *L, const char *name, lua_CFunction proto, int nupval, bool handlers )
{
    _methodRegisterInfo info;
    info.numUpValues = nupval;

    RegisterMethod( L, luaS_new( L, name ), proto, info, handlers );
}

void Class::RegisterInterface( lua_State *L, const luaL_Reg *intf, int nupval, bool handlers )
{
    StkId upId = L->top - nupval;
    _methodRegisterInfo info;
    info.numUpValues = nupval;

    while ( const char *name = intf->name )
    {
        for ( int n = 0; n < nupval; n++ )
            setobj( L, L->top++, upId + n );

        RegisterMethod( L, luaS_new( L, name ), intf->func, info, handlers );

        intf++;
    }
}

void Class::RegisterInterfaceAt( lua_State *L, const luaL_Reg *intf, int nupval, Table *methodTable, bool handlers )
{
    StkId upId = L->top - nupval;
    _methodRegisterInfo info;
    info.numUpValues = nupval;

    while ( const char *name = intf->name )
    {
        for ( int n = 0; n < nupval; n++ )
            setobj( L, L->top++, upId + n );

        RegisterMethodAt( L, luaS_new( L, name ), intf->func, methodTable, info, handlers );

        intf++;
    }
}

void Class::RegisterMethodTrans( lua_State *L, const char *name, lua_CFunction proto, int nupval, int trans, bool handlers )
{
    _methodRegisterInfo info;
    info.isTrans = true;
    info.transID = trans;
    info.numUpValues = nupval;

    RegisterMethod( L, luaS_new( L, name ), proto, info, handlers );
}

void Class::RegisterInterfaceTrans( lua_State *L, const luaL_Reg *intf, int nupval, int trans, bool handlers )
{
    StkId upId = L->top - nupval;
    _methodRegisterInfo info;
    info.isTrans = true;
    info.transID = trans;
    info.numUpValues = nupval;

    while ( intf->name )
    {
        for ( int n = 0; n < nupval; n++ )
            setobj( L, L->top++, upId + n );

        RegisterMethod( L, luaS_new( L, intf->name ), intf->func, info, handlers );

        intf++;
    }
}

void Class::RegisterInterfaceTransAt( lua_State *L, const luaL_Reg *intf, int nupval, int trans, Table *methodTable, bool handlers )
{
    StkId upId = L->top - nupval;
    _methodRegisterInfo info;
    info.isTrans = true;
    info.transID = trans;
    info.numUpValues = nupval;

    while ( intf->name )
    {
        for ( int n = 0; n < nupval; n++ )
            setobj( L, L->top++, upId + n );

        RegisterMethodAt( L, luaS_new( L, intf->name ), intf->func, methodTable, info, handlers );

        intf++;
    }
}

void Class::RegisterLightMethod( lua_State *L, const char *_name )
{
    // Light methods go directly into the environment
    TString *name = luaS_new( L, _name );
    Closure *cl = clvalue( --L->top );

    // They go into storage only
    SetMethod( L, name, cl, storage );
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
    envInherit.insert( envInherit.begin(), gcvalue( --L->top ) );
}

void Class::EnvPutBack( lua_State *L )
{
    envInherit.push_back( gcvalue( --L->top ) );
}

static int classmethod_reference( lua_State *L )
{
    Class& j = *((CClosureMethodBase*)curr_func( L ))->m_class;

    if ( j.destroying )
        return 0;

    j.inMethod++;
    j.refCount++;
    return 0;
}

static int classmethod_dereference( lua_State *L )
{
    Class& j = *((CClosureMethodBase*)curr_func( L ))->m_class;

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
    Class& j = *((CClosureMethodBase*)curr_func( L ))->m_class;

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
    luaJ_basicextend( L );

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
    return 0;
}

static int classmethod_setParent( lua_State *L )
{
    Class& j = *((CClosureMethodBase*)curr_func( L ))->m_class;

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
    c.PushMethod( L, "setChild" );
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
    Class *j = ((CClosureMethodBase*)curr_func( L ))->m_class;

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
    j->env = NULL;
    j->outenv = NULL;
    j->storage = NULL;
    j->internStorage = NULL;
    j->destructor = NULL;

    delete j->methodCache;
    delete j->forceSuper;

    j->destroyed = true;
    j->destroying = false;
    j->reqDestruction = false;
    return 0;
}

static const luaL_Reg classmethods_c[] = 
{
    { "reference", classmethod_reference },
    { "dereference", classmethod_dereference },
    { "isValidChild", classmethod_isValidChild },
    { "getChildren", classmethod_getChildren },
    { NULL, NULL }
};

static const luaL_Reg classmethods_parenting[] =
{
    { "getParent", classmethod_getParent },
    { "setParent", classmethod_setParent },
    { NULL, NULL }
};

void ClassEnvDispatch::NewIndex( lua_State *L, const TValue *key, StkId val )
{
    Class& j = *m_class;

    // If the class leaked it's environment, outer range code might try to modify the class during obscure scenarios
    // We do not want that, as modification of a destroyed class is obscure!
    if ( j.destroyed )
        throw lua_exception( L, LUA_ERRRUN, "attempt to modify a destroyed class" );

    if ( ttype( val ) == LUA_TFUNCTION )
    {
        if ( ttype( key ) == LUA_TSTRING )
        {
            // Register the method
            setobj( L, L->top++, val );
            j.RegisterMethod( L, tsvalue( key ), true );
        }
    }
    else if ( ttype( luaH_get( j.storage, key ) ) == LUA_TFUNCTION )
    {
        throw lua_exception( L, LUA_ERRRUN, "class methods cannot be overwritten" );
    }
    else
    {
        // Make sure we do not hit delayed method writes
        class_checkMethodDelayOverride( L, m_class, key );

        // Apply a regular member
        setobj( L, luaH_set( L, j.storage, key ), val );
        luaC_barriert( L, j.storage, val );
    }
}

class ClassConstructionAllocation
{
public:
    ClassConstructionAllocation( lua_State *L, Class *j )
    {
        setjvalue( L, L->top++, j );
        m_id = luaL_ref( L, LUA_REGISTRYINDEX );
        m_thread = L;
    }

    ~ClassConstructionAllocation( void )
    {
        luaL_unref( m_thread, LUA_REGISTRYINDEX, m_id );
    }

private:
    lua_State*  m_thread;
    int         m_id;
};

Class* luaJ_new( lua_State *L, int nargs, unsigned int flags )
{
    Closure *constructor = clvalue( L->top - 1 );

    // We cannot construct using closure which are, say, class methods already.
    if ( constructor->IsEnvLocked() )
        throw lua_exception( L, LUA_ERRRUN, "attempt to construct a class using a locked closure" );

    // Lock the environment
    constructor->isEnvLocked = true;

    Class *c = new (L) Class;

    // Link it into the GC system
    c->next = G(L)->mainthread->next;
    G(L)->mainthread->next = c;

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
    c->env = luaQ_newclassenv( L, c );
    c->outenv = luaQ_newclassoutenv( L, c );
    c->storage = luaH_new( L, 0, 0 );
    c->internStorage = luaH_new( L, 0, 0 );

    c->forceSuper = new (L) Class::ForceSuperTable();
    c->methodCache = NULL;  // only allocate once required

    // Perform a temporary keep
    ClassConstructionAllocation construction( L, c );

    // Register the previous environment
    lua_pushvalue( L, LUA_ENVIRONINDEX );
    c->EnvPutBack( L );

    // Init the forceSuper table
    Class::forceSuperItem destrItem;
    destrItem.cb = classmethod_fsDestroyHandler;
    destrItem.cbNative = classmethod_fsDestroyHandlerNative;
    c->forceSuper->SetItem( L, luaS_new( L, "destroy" ), destrItem );

    // Create a class-only storage
    sethvalue( L, L->top++, c->internStorage );
    
    if ( !( flags & LCLASS_API_NO_ENV_AWARENESS ) )
    {
        // Distribute the environment
        setqvalue( L, L->top++, c->env );
        lua_setfield( L, -2, "_ENV" );
        setqvalue( L, L->top++, c->outenv );
        lua_setfield( L, -2, "_OUTENV" );
    }
    setjvalue( L, L->top++, c );
    lua_setfield( L, -2, "this" );

    // Set some internal functions
    c->RegisterInterfaceAt( L, internStorage_interface, 0, c->internStorage );

    if ( !( flags & LCLASS_API_NOENVDISPATCH ) )
    {
        // Include the env dispatcher API
        c->RegisterInterfaceAt( L, internStorage_envDispatch_interface, 0, c->internStorage );
    }

    // Pop the internal storage
    L->top--;

    // Register the interface
    if ( !( flags & LCLASS_NOPARENTING ) )
    {
        // Add functionality for Lua parenting
        c->RegisterInterface( L, classmethods_parenting, 0 );
    }

    // Register the light interface
    c->RegisterInterface( L, classmethods_c, 0 );

    sethvalue( L, L->top++, c->storage );

    // Prepare the destructor
    setjvalue( L, L->top++, c );

    setjvalue( L, L->top++, c );
    lua_pushcclosure( L, classmethod_destructor, 1 );

    c->destructor = clvalue( L->top - 1 );
    luaC_forceupdatef( L, c->destructor );

    lua_pushcclosure( L, classmethod_fsDestroyRoot, 2 );

    lua_setfield( L, -2, "destroy" );

    L->top--;

    // Apply the environment to the constructor
    constructor->env = c->env;
    luaC_objbarrier( L, constructor, c->env );

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

    lua_getmethodclass( L )->PushOuterEnvironment( L );
    lua_pushvalue( L, 2 );
    lua_gettable( L, 3 );
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
    lua_refclass( L, LUA_ENVIRONINDEX )->RegisterInterface( L, bprotect_methods, 0 );
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
    lua_refclass( L, LUA_ENVIRONINDEX )->RegisterMethod( L, "extend", extend_handler, 0, true );
}