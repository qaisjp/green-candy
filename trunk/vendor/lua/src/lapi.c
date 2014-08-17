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
    case LUA_ENVIRONINDEX:  // essentially getcurrenv( L );
    {
        if ( L->nCcalls )
        {
            setgcvalue(L, &L->env, curr_func(L)->env);
            return &L->env;
        }
    }
    case LUA_GLOBALSINDEX:
        return gt(L);
    default:
        return curr_func(L)->ReadUpValue( LUA_GLOBALSINDEX - 1 - idx );
    }
}

TValue* index2adr( lua_State *L, int idx )
{
    if ( idx > 0 )
    {
        TValue *o = L->base + (idx - 1);
        api_check(L, idx <= L->ci->top - L->base);

        if ( o >= L->top )
        {
            // We must not return the nil object, as it cannot be written to.
            // Since we want a graceful quit, we return the thread env as nil.
            setnilvalue( &L->env );
            return &L->env;
        }
        else
            return o;
    }
    else if ( idx > LUA_STACKLAST )
    {
        api_check(L, idx != 0 && -idx <= L->top - L->base);
        return L->top + idx;
    }

    return (TValue*)_getconstantobj( L, idx );
}

// Secure function which ensures that the address returned will not be written to.
const TValue* index2constadr( lua_State *L, int idx )
{
    // For this secure function, we should not use assertions.
    if ( idx > 0 )
    {
        const TValue *o = L->base + (idx - 1);

        if ( o >= L->top )
            return luaO_nilobject;
        else
            return o;
    }
    else if ( idx > LUA_STACKLAST )
    {
        if ( idx != 0 && -idx <= L->top - L->base )
            return L->top + idx;
        else
            return luaO_nilobject;
    }

    return _getconstantobj( L, idx );
}

void luaA_pushobject (lua_State *L, const TValue *o) {
  setobj2s(L, L->top, o);
  api_incr_top(L);
}


LUA_API int lua_checkstack (lua_State *L, int size) {
  int res = 1;
  lua_lock(L);
  if (size > LUAI_MAXCSTACK || (L->top - L->base + size) > LUAI_MAXCSTACK)
    res = 0;  /* stack overflow */
  else if (size > 0) {
    luaD_checkstack(L, size);
    if (L->ci->top < L->top + size)
      L->ci->top = L->top + size;
  }
  lua_unlock(L);
  return res;
}

LUA_API void lua_xmove (lua_State *from, lua_State *to, int n)
{
    int i;

    if ( from == to )
        return;

    lua_lock(to);
    api_checknelems(from, n);
    api_check(from, G(from) == G(to));
    api_check(from, to->ci->top - to->top >= n);
    from->top -= n;

    for (i = 0; i < n; i++)
        setobj2s( to, to->top++, from->top + i );

    lua_unlock(to);
}

LUA_API void lua_xcopy( lua_State *from, lua_State *to, int n )
{
    int i;

    // Copying implies that we need another copy nevertheless the from == to match!

    lua_lock(to);
    api_checknelems(from, n);
    api_check(from, G(from) == G(to));
    api_check(from, to->ci->top - to->top >= n);

    StkId top = from->top - n;

    for ( i = 0; i < n; i++ )
        setobj2s( to, to->top++, top + i );

    lua_unlock(to);
}

LUA_API void lua_setlevel (lua_State *from, lua_State *to) {
  to->nCcalls = from->nCcalls;
}


LUA_API lua_State *lua_newthread (lua_State *L) {
  lua_State *L1;
  lua_lock(L);
  luaC_checkGC(L);
  L1 = luaE_newthread(L);
  setthvalue(L, L->top, L1);
  api_incr_top(L);
  lua_unlock(L);
  luai_userstatethread(L, L1);
  return L1;
}

LUA_API void lua_newenvironment( lua_State *L )
{
    luaE_newenvironment( L );
}

// Module initialization.
void luaapi_init( void )
{
    return;
}

void luaapi_shutdown( void )
{
    return;
}