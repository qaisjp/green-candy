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

static int childapi_notifyDestroy( lua_State *L )
{
    Class& j = *jvalue( index2adr( L, lua_upvalueindex( 1 ) ) );

    if ( j.children.size() != 0 )
        return 0;

    setobj( L, L->top++, &j.destructor );
    lua_call( L, 0, 0 );
    return 0;
}

static inline bool class_preDestructor( lua_State *L, Class& j )
{
    if ( j.destroying )
        return false;

    // Make sure we do not try to destroy twice
    j.destroying = true;

    unsigned int n = 0;
    bool reqWorthy = false;
    size_t size = j.children.size();

    for ( ; n<size; n++ )
    {
        Class& c = *j.children[n];
        c.PushMethod( L, "destroy" );
        lua_call( L, 0, 0 );

        //TODO: This may be risky, I have to analyze whether GC may kill our runtime
        //REPORT: I have seen no errors so far!

        if ( !c.destroyed )
        {
            c.childAPI->PushEnvironment( L );
            setjvalue( L, L->top++, &j );
            lua_pushcclosure( L, childapi_notifyDestroy, 1 );
            lua_setfield( L, -2, "notifyDestroy" );
            L->top--;

            n++;
            reqWorthy = true;
        }
        else
            size--;
    }

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
    // Now there is no more issue with keeping a coroutine state to save a class
    // from destruction. Hahaha!
    if ( reqDestruction && inMethod == 0 )
    {
        lua_yield_shield _ref( L ); // prevent destructor yielding

        if ( !class_preDestructor( L, *this ) )
            return;

        setobj2s( L, L->top, &destructor );
        api_incr_top( L );
        lua_call( L, 0, 0 );
    }
}

void Class::DecrementMethodStack( lua_State *lua )
{
    inMethod--;

    CheckDestruction( lua );
}

void Class::ClearReferences( lua_State *lua )
{
    if ( !refCount )
        return;

    inMethod -= refCount;

    refCount = 0;

    CheckDestruction( lua );
}

void Class::PushMethod( lua_State *L, const char *key )
{
    setobj( L, L->top++, luaH_getstr( methods, luaS_new( L, key ) ) );
}

void Class::SetTransmit( int type, void *entity )
{
    trans[type] = entity;

    transRecent = type;
}

bool Class::GetTransmit( int type, void*& entity )
{
    transMap_t::iterator iter = trans.find( type );

    if ( iter == trans.end() )
        return false;

    entity = iter->second;
    return true;
}

int Class::GetTransmit()
{
    return transRecent;
}

bool Class::IsTransmit( int type )
{
    return trans.count( type ) == 1;
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

void Class::RequestDestruction()
{
    reqDestruction = true;
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
        luaC_barriert( thread, myClass->internStorage, &val );
        myClass->IncrementMethodStack( thread );

        m_instance = myClass;
        m_thread = thread;
    }

    ~MethodStackAllocation()
    {
        m_instance->DecrementMethodStack( m_thread );
        setobj( m_thread, m_instance->GetSuperMethod( m_thread ), &m_prevSuper );
        luaC_barriert( m_thread, m_instance->internStorage, &m_prevSuper );
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
    int n;

    for ( n=0; n<top; n++ )
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

static int classmethod_fsDestroyRoot( lua_State *L )
{
    Class& j = *jvalue( index2adr( L, lua_upvalueindex( 1 ) ) );

    if ( j.inMethod != 0 )
    {
        j.reqDestruction = true;
        return 0;
    }

    if ( j.destroyed )
        throw lua_exception( L, LUA_ERRRUN, "attempted to destroy a destroyed class!" );

    // I think we do not need to check for a destroyed class here
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

    setobj2s( L, L->top - 3, &j->destructor );

    lua_pushvalue( L, 3 );
    lua_pushvalue( L, 1 );
    lua_pushcclosure( L, classmethod_fsDestroyBridge, 2 );

    setobj( L, &j->destructor, L->top - 1 );
    luaC_forceupdatef( L, jvalue( &j->destructor ) );
    lua_settop( L, 3 );

    lua_pushcclosure( L, classmethod_fsDestroySuper, 3 );
    return 1;
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
    Class::childList_t::iterator iter = j.children.begin();
    Table& tab = *luaH_new( L, j.children.size(), 0 );
    unsigned int n = 1;

    for ( ; iter != j.children.end(); iter++, n++ )
    {
        setjvalue( L, luaH_setnum( L, &tab, n ), *iter );
        luaC_objbarriert( L, &tab, *iter );
    }

    sethvalue( L, L->top++, &tab );
    return 1;
}

static int childapi_notiDestroy( lua_State *L )
{
    return 0;
}

static int childapi_destroy( lua_State *L )
{
    Class& parent = *jvalue( index2adr( L, lua_upvalueindex( 1 ) ) );
    Class& child = *jvalue( index2adr( L, lua_upvalueindex( 2 ) ) );

    parent.children.erase( std::remove( parent.children.begin(), parent.children.end(), &child ), parent.children.end() );

    lua_getfield( L, LUA_ENVIRONINDEX, "notifyDestroy" );
    lua_call( L, 0, 0 );
    return 0;
}

static const luaL_Reg childapi_interface[] =
{
    { "notifyDestroy", childapi_notiDestroy },
    { "destroy", childapi_destroy },
    { NULL, NULL }
};

static int childapi_constructor( lua_State *L )
{
    lua_pushvalue( L, LUA_ENVIRONINDEX );
    lua_pushvalue( L, lua_upvalueindex( 1 ) );
    lua_pushvalue( L, lua_upvalueindex( 2 ) );
    luaL_openlib( L, NULL, childapi_interface, 2 );
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

    luaL_checktype( L, 1, LUA_TCLASS );
    lua_settop( L, 1 );

    Class& c = *jvalue( L->base );

    if ( &c == &j || c.destroying )
    {
        setbvalue( L->top++, false );
        return 1;
    }

    // Prevent circle jerks or pervert child relationships
    Class *parent = c.parent;

    while ( parent )
    {
        if ( parent == &j )
        {
            setbvalue( L->top++, false );
            return 1;
        }

        parent = parent->parent;
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

    setjvalue( L, L->top - 1, &j );
    lua_pushcclosure( L, childapi_constructor, 2 );
    lua_newclass( L );

    // Store the API
    j.parent = &c;
    j.childAPI = jvalue( L->top - 1 );
    j.childAPI->IncrementMethodStack( L );

    setobj( L, L->top++, luaH_getstr( c.internStorage, luaS_new( L, "setChild" ) ) );
    setjvalue( L, L->top++, &j );
    lua_pcall( L, 1, 0, 0 );

    // Add it to the parent's children list
    c.children.push_back( &j );

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

    //TODO: children support
    if ( j->parent )
    {
        j->childAPI->DecrementMethodStack( L );

        j->childAPI->PushMethod( L, "destroy" );
        lua_call( L, 0, 0 );

        j->childAPI = NULL;
        j->parent = NULL;
    }
 
    j->env->metatable = NULL;
    j->env = NULL;
    j->outenv->metatable = NULL;
    j->outenv = NULL;
    j->methods = NULL;
    j->storage = NULL;
    j->destructor = luaO_nilobject_;

    j->destroyed = true;
    j->destroying = false;
    j->reqDestruction = false;
    return 0;
}

static const luaL_Reg classmethods[] = 
{
    { "reference", classmethod_reference },
    { "dereference", classmethod_dereference },
    { "isValidChild", classmethod_isValidChild },
    { "getChildren", classmethod_getChildren },
    { "setParent", classmethod_setParent },
    { "getParent", classmethod_getParent },
    { NULL, NULL }
};

static int methodenv_newindex( lua_State *L )
{
    if ( lua_type( L, 2 ) == LUA_TSTRING )
    {
        Class& j = *jvalue( index2adr( L, lua_upvalueindex( 1 ) ) );
        int t = lua_type( L, 3 );

        if ( t == LUA_TFUNCTION )
        {
            // Acquire the previous method
            TString *key = tsvalue( L->base + 1 );
            Table *methTable;
            const TValue *val = luaH_getstr( j.internStorage, key );

            if ( val->tt == LUA_TFUNCTION )
            {
                setobj( L, L->top++, val );

                methTable = j.internStorage;
            }
            else
            {
                setobj( L, L->top++, luaH_getstr( j.methods, key ) );

                methTable = j.methods;
            }

            setjvalue( L, L->top, &j );
            api_incr_top( L );

            lua_pushvalue( L, 3 );

            sethvalue( L, L->top, j.forceSuper );
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

            // Better avoid using the (limited) stack!
            Closure *method = clvalue( L->top - 1 );
            bool methWhite = iswhite( method ) != 0;

            // Apply the class environment
            method->env = j.env;

            // We have to put it into the environment, too (caching <3)
            if ( val == luaO_nilobject )
            {
                setclvalue( L, luaH_setstr( L, j.storage, key ), method );
                
                if ( methWhite && isblack( j.storage ) )
                    luaC_barrierback( L, j.storage );
            }
            
            setclvalue( L, luaH_setstr( L, methTable, key ), method );

            if ( methWhite && isblack( methTable ) )
                luaC_barrierback( L, methTable );

            return 0;
        }
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
    c->transRecent = -1;
    c->parent = NULL;

    // Set up the environments
    c->env = luaH_new( L, 0, 0 );
    c->outenv = luaH_new( L, 0, 0 );
    c->storage = luaH_new( L, 0, 0 );
    c->methods = luaH_new( L, 0, 0 );
    c->forceSuper = luaH_new( L, 0, 0 );
    c->internStorage = luaH_new( L, 0, 0 );
    setnilvalue( &c->destructor );

    c->env->metatable = meta;
    c->outenv->metatable = outmeta;

    c->superCached = luaS_newlstr( L, "super", 5 );

    // Perform a temporary keep
    ClassConstructionAllocation construction( L, c );

    // Init the forceSuper table
    sethvalue( L, L->top++, c->forceSuper );

    lua_pushcclosure( L, classmethod_fsDestroyHandler, 0 );
    lua_setfield( L, -2, "destroy" );

    lua_pop( L, 1 );

    // Specify the outrange connection
    sethvalue( L, L->top++, outmeta );

    lua_pushboolean( L, false );
    lua_setfield( L, -2, "__metatable" );

    sethvalue( L, L->top++, c->storage );
    sethvalue( L, L->top++, c->methods );

    luaL_openlib( L, NULL, envmeta_interface, 2 );
    lua_pop( L, 1 );

    // Put the meta to stack
    sethvalue( L, L->top++, meta );

    // The upvalue has to be the class
    setjvalue( L, L->top++, c );

    // Create a class-only storage
    sethvalue( L, L->top++, c->internStorage );
    sethvalue( L, L->top++, c->env );
    lua_setfield( L, -2, "_ENV" );
    sethvalue( L, L->top++, c->outenv );
    lua_setfield( L, -2, "_OUTENV" );
    setjvalue( L, L->top++, c );
    lua_setfield( L, -2, "this" );

    // Set some internal functions
    setjvalue( L, L->top++, c );
    luaL_openlib( L, NULL, internStorage_interface, 1 );

    // Give it the environment storage
    sethvalue( L, L->top++, c->storage );

    // We also need the previous environment
    lua_pushvalue( L, LUA_ENVIRONINDEX );

    lua_pushcclosure( L, methodenv_index, 4 );
    lua_setfield( L, -2, "__index" );

    // The upvalue has to be the class
    setjvalue( L, L->top++, c );

    // Method registry
    sethvalue( L, L->top++, c->methods );
    
    // Internal storage
    sethvalue( L, L->top++, c->storage );

    lua_pushcclosure( L, methodenv_newindex, 3 );
    lua_setfield( L, -2, "__newindex" );

    lua_pushboolean( L, false );
    lua_setfield( L, -2, "__metatable" );

    sethvalue( L, L->top - 1, c->env );
    lua_setfenv( L, -nargs - 2 );

    sethvalue( L, L->top++, c->env );

    // Register the interface
    setjvalue( L, L->top++, c );

    luaL_openlib( L, NULL, classmethods, 1 );

    sethvalue( L, L->top - 1, c->methods );

    // Prepare the destructor
    setjvalue( L, L->top++, c );

    setjvalue( L, L->top++, c );
    lua_pushcclosure( L, classmethod_destructor, 1 );
    setobj( L, &c->destructor, L->top - 1 );

    lua_pushcclosure( L, classmethod_fsDestroyRoot, 2 );

    lua_pushvalue( L, -1 );

    lua_setfield( L, -3, "destroy" );

    sethvalue( L, L->top - 2, c->storage );
    lua_setfield( L, -2, "destroy" );

    lua_pop( L, 1 );

    // Apply the environment to the constructor
    sethvalue( L, L->top++, c->env );
    lua_setfenv( L, -nargs - 1 );

    // Call the constructor (class as first arg)
    setjvalue( L, L->top++, c );
    lua_insert( L, -nargs - 1 );

    lua_call( L, nargs + 1, 0 );
    return c;
}

Class::~Class()
{
}

void luaJ_construct( lua_State *L, int nargs )
{
    setjvalue( L, L->top - 1, luaJ_new( L, nargs ));
    api_incr_top( L );
}

// Basic protection for classes
static int protect_index( lua_State *L )
{
    size_t len;
    const char *str = lua_tolstring( L, 2, &len );

    if ( len > 1 && *(unsigned short*)str == 0x5f5f )
        return 0;

    lua_gettable( L, LUA_ENVIRONINDEX );
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