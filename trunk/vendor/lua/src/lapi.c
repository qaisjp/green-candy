/*
** $Id: lapi.c,v 2.55.1.5 2008/07/04 18:41:18 roberto Exp $
** Lua API
** See Copyright Notice in lua.h
*/

#include "luacore.h"

#include "lapi.h"
#include "lfunc.h"
#include "lstate.h"
#include "lgc.h"
#include "lvm.h"

#include "lapi.hxx"


const char lua_ident[] =
  "$Lua: " LUA_RELEASE " " LUA_COPYRIGHT " $\n"
  "$Authors: " LUA_AUTHORS " $\n"
  "$URL: www.lua.org $\n";


inline const TValue* _getconstantobj( lua_State *L, int idx )
{
    switch( idx )
    {  /* pseudo-indices */
    case LUA_STORAGEINDEX:
        return luaF_getcurraccessor( L );
    case LUA_REGISTRYINDEX:
        return registry(L);
    case LUA_ENVIRONINDEX:  // essentially getcurrenv( L )
    {
        // If there is an enclosing function...
        if ( L->ciStack.GetUsageCount( L ) > 1 )
        {
            // ... use its environment.
            setgcvalue(L, &L->env, curr_func(L)->env);
            return &L->env;
        }
        // Otherwise fall down to the global table.
    }
    case LUA_GLOBALSINDEX:
        return gt(L);
    default:
        return curr_func(L)->ReadUpValue( LUA_GLOBALSINDEX - 1 - idx );
    }
}

inline bool IsValidStackIndex( lua_State *L, int idx )
{
    return ( idx != 0 && -idx <= rt_stackcount(L) );
}

stackOffset_t index2stackindex( lua_State *L, int idx )
{
    stackOffset_t returnIdx = -1;

    lua_assert( idx != 0 );

    if ( idx != 0 )
    {
        if ( idx > 0 )
        {
            returnIdx = idx - 1;
        }
        else if ( idx > LUA_STACKLAST )
        {
            returnIdx = idx;
        }
    }

    return returnIdx;
}

RtCtxItem index2stackadr( lua_State *L, int idx, bool isSecure )
{
    RtCtxItem stackItem;

    // Dispatch the stack item.
    stackOffset_t stackOffset = index2stackindex( L, idx );
    
    stackItem = L->GetCurrentStackFrame().GetStackItem( L, L->rtStack, stackOffset );

    if ( isSecure == false )
    {
        api_check( L, stackItem.Pointer() != NULL );
    }

    return stackItem;
}

ValueAddress fetchstackadr( lua_State *L, int idx )
{
    ValueAddress returnAddr;

    RtCtxItem stackItem = L->GetCurrentStackFrame().GetStackItem( L, L->rtStack, idx );

    ValueContext *stackValueContext = L->rtStack.GetNewVirtualStackItem( L, stackItem );

    returnAddr.Setup( L, stackValueContext );

    return returnAddr;
}

ValueAddress index2adr( lua_State *L, int idx )
{
    ValueAddress returnVal;

    TValue *valueReturn = NULL;
    ValueContext *valCtx = NULL;

    rtStack_t& rtStack = L->rtStack;

    if ( idx > 0 || idx > LUA_STACKLAST )
    {
        RtCtxItem stackItem = index2stackadr( L, idx );

        // Construct the context of access.
        if ( stackItem.Pointer() == NULL )
        {
            api_check( L, 0 );

            // We must not return the nil object, as it cannot be written to.
            // Since we want a graceful quit, we return the thread env as nil.
            setnilvalue( &L->env );

            valueReturn = &L->env;
        }
        else
        {
            valCtx = rtStack.GetNewVirtualStackItem( L, stackItem );
        }
    }
    else
    {
        valueReturn = const_cast <TValue*> ( _getconstantobj( L, idx ) );
    }

    if ( valueReturn )
    {
        valCtx = L->NewStateValueContext( valueReturn );
    }

    returnVal.Setup( L, valCtx );

    return returnVal;
}

// Secure function which ensures that the address returned will not be written to.
ConstValueAddress index2constadr( lua_State *L, int idx )
{
    ConstValueAddress returnVal;

    const TValue *valueReturn = NULL;
    ConstValueContext *valCtx = NULL;

    rtStack_t& rtStack = L->rtStack;

    // For this secure function, we should not use assertions.
    if ( idx > 0 || idx > LUA_STACKLAST )
    {
        RtCtxItem stackItem = index2stackadr( L, idx, true );

        // Construct the context of access.
        if ( stackItem.Pointer() == NULL )
        {
            valueReturn = luaO_nilobject;
        }
        else
        {
            valCtx = rtStack.GetNewVirtualStackItemConst( L, stackItem );
        }
    }
    else
    {
        valueReturn = _getconstantobj( L, idx );
    }

    if ( valueReturn )
    {
        valCtx = L->NewStateValueContext( valueReturn );
    }

    returnVal.Setup( L, valCtx );

    return returnVal;
}

void luaA_pushobject (lua_State *L, const TValue *o)
{
    pushtvalue( L, o );
}


LUA_API int lua_checkstack (lua_State *L, int size)
{
    int res = 1;

    lua_lock(L);

    rtStack_t& rtStack = L->rtStack;

    rtStack.Lock( L );

    if (size > LUAI_MAXCSTACK || (rt_stackcount(L) + size) > LUAI_MAXCSTACK)
    {
        res = 0;  /* stack overflow */
    }
    else if (size > 0)
    {
        luaD_checkstack(L, size);
    }

    rtStack.Unlock( L );

    lua_unlock(L);
    return res;
}

LUA_API void lua_xmove (lua_State *from, lua_State *to, int n)
{
    if ( from == to )
        return;

    lua_lock(to);
    api_checknelems(from, n);
    api_check(from, G(from) == G(to));

    from->GetCurrentStackFrame().CrossMoveTop( from, from->rtStack, &to->GetCurrentStackFrame(), to->rtStack, n );

    lua_unlock(to);
}

LUA_API void lua_xcopy( lua_State *from, lua_State *to, int n )
{
    // Copying implies that we need another copy nevertheless the from == to match!

    lua_lock(to);
    api_checknelems(from, n);
    api_check(from, G(from) == G(to));

    from->GetCurrentStackFrame().CrossCopyTop( from, from->rtStack, &to->GetCurrentStackFrame(), to->rtStack, n );

    lua_unlock(to);
}

LUA_API void lua_setlevel (lua_State *from, lua_State *to)
{
    throw lua_exception( from, LUA_ERRRUN, "unsupported operation (lua_setlevel)" );
}


LUA_API lua_State *lua_newthread (lua_State *L)
{
    lua_State *L1;
    lua_lock(L);
    luaC_checkGC(L);
    L1 = luaE_newthread(L);
    pushthvalue(L, L1);

    // Since the new thread is now on the Lua stack, we can dereference it.
    L1->DereferenceGC( L );

    lua_unlock(L);
    luai_userstatethread(L, L1);
    return L1;
}

LUA_API void lua_newenvironment( lua_State *L )
{
    luaE_newenvironment( L );
}

// Module initialization.
void luaapi_init( lua_config *cfg )
{
    return;
}

void luaapi_shutdown( lua_config *cfg )
{
    return;
}