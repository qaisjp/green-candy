/*
** $Id: lapi.c,v 2.55.1.5 2008/07/04 18:41:18 roberto Exp $
** Lua API
** See Copyright Notice in lua.h
*/


#include <assert.h>
#include <math.h>
#include <stdarg.h>
#include <string.h>

#define lapi_c
#define LUA_CORE

#include "lua.h"

#include "lapi.h"
#include "ldebug.h"
#include "ldo.h"
#include "lfunc.h"
#include "lgc.h"
#include "lmem.h"
#include "lobject.h"
#include "lstate.h"
#include "lstring.h"
#include "ltable.h"
#include "lclass.h"
#include "ltm.h"
#include "lundump.h"
#include "lvm.h"


const char lua_ident[] =
  "$Lua: " LUA_RELEASE " " LUA_COPYRIGHT " $\n"
  "$Authors: " LUA_AUTHORS " $\n"
  "$URL: www.lua.org $\n";

static GCObject *getcurrenv( lua_State *L )
{
    if ( L->ci == L->base_ci )  /* no enclosing function? */
        return gcvalue(gt(L));  /* use global table as environment */

    return curr_func( L )->env;
}

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
            return cast(TValue *, luaO_nilobject);  // This really is a graceful quit.
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


/*
** basic stack manipulation
*/

LUA_API int lua_gettop (lua_State *L) {
  return cast_int(L->top - L->base);
}

LUA_API void lua_settop (lua_State *L, int idx) {
  lua_lock(L);
  if (idx >= 0) {
    api_check(L, idx <= L->stack_last - L->base);
    while (L->top < L->base + idx)
      setnilvalue(L->top++);
    L->top = L->base + idx;
  }
  else {
    api_check(L, -(idx+1) <= (L->top - L->base));
    L->top += idx+1;  /* `subtract' index (index is negative) */
  }
  lua_unlock(L);
}

LUA_API void lua_remove (lua_State *L, int idx) {
  StkId p;
  lua_lock(L);
  p = index2adr(L, idx);
  api_checkvalidindex(L, p);
  while (++p < L->top) setobjs2s(L, p-1, p);
  L->top--;
  lua_unlock(L);
}

LUA_API void lua_insert (lua_State *L, int idx) {
  StkId p;
  StkId q;
  lua_lock(L);
  p = index2adr(L, idx);
  api_checkvalidindex(L, p);
  for (q = L->top; q>p; q--) setobjs2s(L, q, q-1);
  setobjs2s(L, p, L->top);
  lua_unlock(L);
}

LUA_API void lua_swap( lua_State *L, int idx )
{
	TValue tmp;
	StkId p;
	StkId q;
	lua_lock( L );
	
	p = index2adr( L, idx );
	setobj( L, &tmp, p );

	q = L->top - 1;
	setobj2s( L, p, q );
	setobj2s( L, q, &tmp );

	lua_unlock( L );
}

LUA_API void lua_replace (lua_State *L, int idx)
{
    StkId o;
    lua_lock(L);

    /* explicit test for incompatible code */
    if ( idx == LUA_ENVIRONINDEX && L->ci == L->base_ci )
        luaG_runerror(L, "no calling environment");

    api_checknelems(L, 1);

    if ( idx == LUA_ENVIRONINDEX )
    {
        Closure *func = curr_func(L);
        const TValue *val = L->top - 1;

        func->env = gcvalue(val);
        luaC_barrier(L, func, val);
    }
    else if ( idx == LUA_STORAGEINDEX )
    {
        const TValue *val = L->top - 1;

        setgcvalue( L, &L->storage, gcvalue( val ) );
        luaC_barrier( L, L, val );
    }
    else
    {
        o = index2adr(L, idx);
        api_checkvalidindex(L, o);

        setobj( L, o, L->top - 1 );

        if ( idx < LUA_GLOBALSINDEX )  /* function upvalue? */
            luaC_barrier(L, curr_func(L), L->top - 1);
    }

    L->top--;
    lua_unlock(L);
}

LUA_API void lua_pushvalue (lua_State *L, int idx)
{
    lua_lock(L);
    setobj2s(L, L->top, index2constadr(L, idx));
    api_incr_top(L);
    lua_unlock(L);
}


/*
** access functions (stack -> C)
*/

LUA_API int lua_type (lua_State *L, int idx)
{
    const TValue *o = index2constadr(L, idx);
    return (o == luaO_nilobject) ? LUA_TNONE : ttype(o);
}

LUA_API const char *lua_typename (lua_State *L, int t)
{
    UNUSED(L);
    return (t == LUA_TNONE) ? "no value" : luaT_typenames[t];
}

LUA_API int lua_iscfunction (lua_State *L, int idx)
{
    const TValue *o = index2constadr(L, idx);
    return iscfunction(o);
}

LUA_API int lua_isnumber (lua_State *L, int idx) {
  TValue n;
  const TValue *o = index2constadr(L, idx);
  return tonumber(o, &n);
}

LUA_API int lua_isstring (lua_State *L, int idx) {
  int t = lua_type(L, idx);
  return (t == LUA_TSTRING || t == LUA_TNUMBER);
}

LUA_API int lua_isuserdata (lua_State *L, int idx) {
  const TValue *o = index2constadr(L, idx);
  return (ttisuserdata(o) || ttislightuserdata(o));
}

LUA_API int lua_rawequal (lua_State *L, int index1, int index2) {
  const TValue *o1 = index2constadr(L, index1);
  const TValue *o2 = index2constadr(L, index2);
  return (o1 == luaO_nilobject || o2 == luaO_nilobject) ? 0
         : luaO_rawequalObj(o1, o2);
}

LUA_API int lua_equal (lua_State *L, int index1, int index2) {
  const TValue *o1, *o2;
  int i;
  lua_lock(L);  /* may call tag method */
  o1 = index2constadr(L, index1);
  o2 = index2constadr(L, index2);
  i = (o1 == luaO_nilobject || o2 == luaO_nilobject) ? 0 : equalobj(L, o1, o2);
  lua_unlock(L);
  return i;
}

LUA_API int lua_lessthan (lua_State *L, int index1, int index2) {
  const TValue *o1, *o2;
  int i;
  lua_lock(L);  /* may call tag method */
  o1 = index2constadr(L, index1);
  o2 = index2constadr(L, index2);
  i = (o1 == luaO_nilobject || o2 == luaO_nilobject) ? 0
       : luaV_lessthan(L, o1, o2);
  lua_unlock(L);
  return i;
}

LUA_API lua_Number lua_tonumber (lua_State *L, int idx)
{
    TValue n;
    const TValue *o = index2constadr(L, idx);
    if (!tonumber(o, &n))
        return 0;
    return nvalue(o);
}


LUA_API lua_Integer lua_tointeger (lua_State *L, int idx)
{
    TValue n;
    const TValue *o = index2constadr(L, idx);

    if (!tonumber(o, &n))
        return 0;

    lua_Integer res;
    lua_Number num = nvalue(o);
    lua_number2integer(res, num);
    return res;
}

// MTA Specific
LUA_API lua_Integer lua_tointegerW (lua_State *L, int idx)
{
    TValue n;
    const TValue *o = index2constadr(L, idx);

    if (!tonumber(o, &n))
        return 0;

    // The_GTA: Do not try to outsmart compilers
    return (int)nvalue(o);
}

LUA_API bool lua_toboolean (lua_State *L, int idx) {
  const TValue *o = index2constadr(L, idx);
  return !l_isfalse(o);
}

LUA_API lua_String lua_tolstring (lua_State *L, int idx, size_t *len)
{
    StkId o = index2adr(L, idx);
    if (!ttisstring(o))
    {
        lua_lock(L);  /* `luaV_tostring' may create a new string */
        if (!luaV_tostring(L, o))
        {  /* conversion failed? */
            if (len != NULL) *len = 0;
            lua_unlock(L);
            return NULL;
        }
        luaC_checkGC(L);
        o = index2adr(L, idx);  /* previous call may reallocate the stack */
        lua_unlock(L);
    }
    if (len != NULL) *len = tsvalue(o)->len;
    return svalue(o);
}


LUA_API size_t lua_objlen (lua_State *L, int idx)
{
    StkId o = index2adr(L, idx);

    switch (ttype(o))
    {
    case LUA_TSTRING: return tsvalue(o)->len;
    case LUA_TUSERDATA: return uvalue(o)->len;
    case LUA_TTABLE: return luaH_getn(hvalue(o));
    case LUA_TNUMBER: {
        size_t l;
        lua_lock(L);  /* `luaV_tostring' may create a new string */
        l = (luaV_tostring(L, o) ? tsvalue(o)->len : 0);
        lua_unlock(L);
        return l;
    }
    }
    return 0;
}

LUA_API lua_CFunction lua_tocfunction (lua_State *L, int idx)
{
    const TValue *o = index2constadr(L, idx);
    return (!iscfunction(o)) ? NULL : clvalue(o)->GetCClosure()->f;
}

LUA_API void *lua_touserdata (lua_State *L, int idx)
{
    const TValue *o = index2constadr(L, idx);

    switch (ttype(o))
    {
    case LUA_TUSERDATA: return (rawuvalue(o) + 1);
    case LUA_TLIGHTUSERDATA: return pvalue(o);
    }
    return NULL;
}

LUA_API lua_State *lua_tothread (lua_State *L, int idx) {
  const TValue *o = index2constadr(L, idx);
  return (!ttisthread(o)) ? NULL : thvalue(o);
}


LUA_API const void *lua_topointer (lua_State *L, int idx)
{
    const TValue *o = index2constadr(L, idx);
    switch (ttype(o))
    {
    case LUA_TTABLE:
        return hvalue(o);
    case LUA_TFUNCTION:
        return clvalue(o);
    case LUA_TTHREAD:
        return thvalue(o);
    case LUA_TUSERDATA:
    case LUA_TLIGHTUSERDATA:
        return lua_touserdata(L, idx);
    case LUA_TCLASS:
        return jvalue(o);
    case LUA_TDISPATCH:
        return qvalue(o);
    default:
        return NULL;
    }
}



/*
** push functions (C -> stack)
*/


LUA_API void lua_pushnil (lua_State *L)
{
  lua_lock(L);
  setnilvalue(L->top);
  api_incr_top(L);
  lua_unlock(L);
}


LUA_API void lua_pushnumber (lua_State *L, lua_Number n)
{
  lua_lock(L);
  setnvalue(L->top, n);
  api_incr_top(L);
  lua_unlock(L);
}


LUA_API void lua_pushinteger (lua_State *L, lua_Integer n)
{
  lua_lock(L);
  setnvalue(L->top, cast_num(n));
  api_incr_top(L);
  lua_unlock(L);
}


LUA_API void lua_pushlstring (lua_State *L, const char *s, size_t len)
{
  lua_lock(L);
  luaC_checkGC(L);
  setsvalue2s(L, L->top, luaS_newlstr(L, s, len));
  api_incr_top(L);
  lua_unlock(L);
}


LUA_API void lua_pushstring (lua_State *L, const char *s)
{
  if (s == NULL)
    lua_pushnil(L);
  else
    lua_pushlstring(L, s, strlen(s));
}


LUA_API const char *lua_pushvfstring (lua_State *L, const char *fmt,
                                      va_list argp) {
  const char *ret;
  lua_lock(L);
  luaC_checkGC(L);
  ret = luaO_pushvfstring(L, fmt, argp);
  lua_unlock(L);
  return ret;
}


LUA_API const char *lua_pushfstring (lua_State *L, const char *fmt, ...)
{
  const char *ret;
  va_list argp;
  lua_lock(L);
  luaC_checkGC(L);
  va_start(argp, fmt);
  ret = luaO_pushvfstring(L, fmt, argp);
  va_end(argp);
  lua_unlock(L);
  return ret;
}


LUA_API void lua_pushcclosure (lua_State *L, lua_CFunction fn, int n)
{
  CClosureBasic *cl;
  lua_lock(L);
  luaC_checkGC(L);
  api_checknelems(L, n);
  cl = luaF_newCclosure(L, n, getcurrenv(L));
  cl->f = fn;
  L->top -= n;
  while (n--)
    setobj2n(L, &cl->upvalues[n], L->top+n);
  setclvalue(L, L->top, cl);
  lua_assert(iswhite(cl));
  api_incr_top(L);
  lua_unlock(L);
}


LUA_API void lua_pushboolean (lua_State *L, int b)
{
  lua_lock(L);
  setbvalue(L->top, (b != 0));  /* ensure that true is 1 */
  api_incr_top(L);
  lua_unlock(L);
}


LUA_API void lua_pushlightuserdata (lua_State *L, void *p)
{
  lua_lock(L);
  setpvalue(L->top, p);
  api_incr_top(L);
  lua_unlock(L);
}


LUA_API int lua_pushthread (lua_State *L)
{
  lua_lock(L);
  setthvalue(L, L->top, L);
  api_incr_top(L);
  lua_unlock(L);
  return (G(L)->mainthread == L);
}


LUA_API int lua_pushthreadex( lua_State *L, lua_State *thread )
{
    lua_lock(L);
    setthvalue( L, L->top, thread );
    api_incr_top(L);
    lua_unlock(L);
    return (G(L)->mainthread == thread);
}


/*
** get functions (Lua -> stack)
*/


LUA_API void lua_gettable (lua_State *L, int idx)
{
  const TValue *t;
  lua_lock(L);
  t = index2constadr(L, idx);
  api_checkvalidindex(L, t);
  luaV_gettable(L, t, L->top - 1, L->top - 1);
  lua_unlock(L);
}


LUA_API void lua_getfield (lua_State *L, int idx, const char *k)
{
  const TValue *t;
  TValue key;
  lua_lock(L);
  t = index2constadr(L, idx);
  api_checkvalidindex(L, t);
  setsvalue(L, &key, luaS_new(L, k));
  luaV_gettable(L, t, &key, L->top);
  api_incr_top(L);
  lua_unlock(L);
}


LUA_API void lua_rawget (lua_State *L, int idx)
{
  const TValue *t;
  lua_lock(L);
  t = index2constadr(L, idx);
  api_check(L, ttistable(t));
  setobj2s(L, L->top - 1, luaH_get(hvalue(t), L->top - 1));
  lua_unlock(L);
}


LUA_API void lua_rawgeti (lua_State *L, int idx, int n)
{
  const TValue *o;
  lua_lock(L);
  o = index2constadr(L, idx);
  api_check(L, ttistable(o));
  setobj2s(L, L->top, luaH_getnum(hvalue(o), n));
  api_incr_top(L);
  lua_unlock(L);
}


LUA_API void lua_createtable (lua_State *L, int narray, int nrec)
{
  lua_lock(L);
  luaC_checkGC(L);
  sethvalue(L, L->top, luaH_new(L, narray, nrec));
  api_incr_top(L);
  lua_unlock(L);
}


LUA_API int lua_getmetatable (lua_State *L, int objindex)
{
  Table *mt = NULL;
  int res;
  lua_lock(L);
  mt = luaT_getmetabyobj( L, index2constadr(L, objindex) );
  if (mt == NULL)
    res = 0;
  else {
    sethvalue(L, L->top, mt);
    api_incr_top(L);
    res = 1;
  }
  lua_unlock(L);
  return res;
}


LUA_API void lua_getfenv (lua_State *L, int idx)
{
    const TValue *o;
    lua_lock(L);

    o = index2constadr(L, idx);
    api_checkvalidindex(L, o);

    switch( ttype(o) )
    {
    case LUA_TFUNCTION:
        {
            Closure *cl = clvalue( o );

            // Lua may not retrieve the environment of locked closures
            if ( cl->IsEnvLocked() )
                setnilvalue( L->top );
            else
                setgcvalue(L, L->top, cl->env);
        }
        break;
    case LUA_TUSERDATA:
        setgcvalue(L, L->top, uvalue(o)->env);
        break;
    case LUA_TTHREAD:
        setobj2s(L, L->top,  gt(thvalue(o)));
        break;
    case LUA_TCLASS:
        setgcvalue(L, L->top, jvalue(o)->env);
        break;
    default:
        setnilvalue(L->top);
        break;
    }
    api_incr_top(L);
    lua_unlock(L);
}


LUA_API ILuaClass* lua_getmethodclass( lua_State *L )
{
    return ((CClosureMethodBase*)curr_func( L ))->m_class;
}

LUA_API void* lua_getmethodtrans( lua_State *L )
{
    return ((CClosureMethodTrans*)curr_func( L ))->data;
}

LUA_API void lua_pushmethodsuper( lua_State *L )
{
    lua_lock( L );

    CClosureMethodBase *method = (CClosureMethodBase*)curr_func( L );

    if ( method->super )
    {
        setclvalue( L, L->top++, method->super );
    }
    else
    {
        setnilvalue( L->top++ );
    }

    lua_unlock( L );
}

LUA_API void lua_getclass( lua_State *L )
{
    lua_lock( L );

    StkId classId = L->top - 1;

    lua_assert( iscollectable( classId ) );

    Class *j = gcvalue( classId )->GetClass();

    if ( j )
    {
        setjvalue( L, classId, j );
    }
    else
    {
        setnilvalue( classId );
    }

    lua_unlock( L );
}


/*
** set functions (stack -> Lua)
*/


LUA_API void lua_settable (lua_State *L, int idx)
{
  const TValue *t;
  lua_lock(L);
  api_checknelems(L, 2);
  t = index2constadr(L, idx);
  api_checkvalidindex(L, t);
  luaV_settable(L, t, L->top - 2, L->top - 1);
  L->top -= 2;  /* pop index and value */
  lua_unlock(L);
}


LUA_API void lua_setfield (lua_State *L, int idx, const char *k)
{
    const TValue *t;
    TValue key;
    lua_lock(L);
    api_checknelems(L, 1);
    t = index2constadr(L, idx);
    api_checkvalidindex(L, t);
    setsvalue(L, &key, luaS_new(L, k));
    luaV_settable(L, t, &key, L->top - 1);
    L->top--;  /* pop value */
    lua_unlock(L);
}


LUA_API void lua_setfieldl( lua_State *L, int idx, const char *k, size_t l )
{
    const TValue *t;
    TValue key;
    lua_lock(L);
    api_checknelems(L, 1);
    t = index2constadr(L, idx);
    api_checkvalidindex(L, t);
    setsvalue(L, &key, luaS_newlstr(L, k, l));
    luaV_settable(L, t, &key, L->top - 1);
    L->top--;  /* pop value */
    lua_unlock(L);
}


LUA_API void lua_rawset (lua_State *L, int idx) {
  const TValue *t;
  lua_lock(L);
  api_checknelems(L, 2);
  t = index2constadr(L, idx);
  api_check(L, ttistable(t));
  setobj2t(L, luaH_set(L, hvalue(t), L->top-2), L->top-1);
  luaC_barriert(L, hvalue(t), L->top-1);
  L->top -= 2;
  lua_unlock(L);
}


LUA_API void lua_rawseti (lua_State *L, int idx, int n) {
  const TValue *o;
  lua_lock(L);
  api_checknelems(L, 1);
  o = index2constadr(L, idx);
  api_check(L, ttistable(o));
  setobj2t(L, luaH_setnum(L, hvalue(o), n), L->top-1);
  luaC_barriert(L, hvalue(o), L->top-1);
  L->top--;
  lua_unlock(L);
}


LUA_API int lua_setmetatable (lua_State *L, int objindex)
{
    const TValue *obj;
    Table *mt;
    lua_lock(L);
    api_checknelems(L, 1);
    obj = index2constadr(L, objindex);
    api_checkvalidindex(L, obj);

    if (ttisnil(L->top - 1))
        mt = NULL;
    else
    {
        api_check(L, ttistable(L->top - 1));
        mt = hvalue(L->top - 1);
    }

    switch (ttype(obj))
    {
    case LUA_TTABLE:
        {
            Table *h = hvalue(obj);

            h->metatable = mt;
            if ( mt )
            {
                luaC_objbarriert(L, h, mt);
                mt->flags = 0;   // reset the metatable flags
            }
        }
        break;
    case LUA_TUSERDATA:
        {
            uvalue(obj)->metatable = mt;
            if (mt)
                luaC_objbarrier(L, rawuvalue(obj), mt);
        }
        break;
    default:
        {
            L->mt[ttype(obj)] = mt;
        }
        break;
    }

    L->top--;
    lua_unlock(L);
    return 1;
}

LUA_API int lua_setfenv (lua_State *L, int idx)
{
    const TValue *o;
    int res = 1;
    lua_lock(L);
    api_checknelems(L, 1);
    o = index2constadr(L, idx);
    api_checkvalidindex(L, o);
    api_check(L, iscollectable(L->top - 1));

    switch( ttype(o) )
    {
    case LUA_TFUNCTION:
        {
            Closure *cl = clvalue(o);

            if ( cl->IsEnvLocked() )
                goto fail;

            cl->env = gcvalue(L->top - 1);
        }
        break;
    case LUA_TUSERDATA:
        uvalue(o)->env = gcvalue(L->top - 1);
        break;
    case LUA_TTHREAD:
        setgcvalue(L, gt(thvalue(o)), gcvalue(L->top - 1));
        break;
    default:
fail:
        res = 0;
        goto end;
    }

    luaC_objbarrier(L, gcvalue(o), gcvalue(L->top - 1));

end:
    L->top--;
    lua_unlock(L);
    return res;
}


/*
** `load' and `call' functions (run Lua code)
*/


#define adjustresults(L,nres) \
    { if (nres == LUA_MULTRET && L->top >= L->ci->top) L->ci->top = L->top; }


#define checkresults(L,na,nr) \
     api_check(L, (nr) == LUA_MULTRET || (L->ci->top - L->top >= (nr) - (na)))
	

LUA_API void lua_call (lua_State *L, int nargs, int nresults)
{
    StkId func;
    lua_lock(L);
    api_checknelems(L, nargs+1);
    checkresults(L, nargs, nresults);
    func = L->top - (nargs+1);
    luaD_call(L, func, nresults);
    adjustresults(L, nresults);
    lua_unlock(L);
}



/*
** Execute a protected call.
*/
struct CallS {  /* data to `f_call' */
  StkId func;
  int nresults;
};


static void f_call (lua_State *L, void *ud) {
  struct CallS *c = cast(struct CallS *, ud);
  luaD_call(L, c->func, c->nresults);
}



LUA_API int lua_pcallex (lua_State *L, int nargs, int nresults, int errfunc, lua_Debug *debug)
{
  struct CallS c;
  int status;
  ptrdiff_t func;
  lua_lock(L);
  api_checknelems(L, nargs+1);
  checkresults(L, nargs, nresults);
  if (errfunc == 0)
    func = 0;
  else
  {
    StkId o = index2adr(L, errfunc);
    api_checkvalidindex(L, o);
    func = savestack(L, o);
  }
  c.func = L->top - (nargs+1);  /* function to be called */
  c.nresults = nresults;
  status = luaD_pcall(L, f_call, &c, savestack(L, c.func), func, debug);
  adjustresults(L, nresults);
  lua_unlock(L);
  return status;
}


LUA_API int lua_pcall (lua_State *L, int nargs, int nresults, int errfunc)
{
    return lua_pcallex( L, nargs, nresults, errfunc, NULL );
}


/*
** Execute a protected C call.
*/
struct CCallS {  /* data to `f_Ccall' */
  lua_CFunction func;
  void *ud;
};


static void f_Ccall (lua_State *L, void *ud) {
  struct CCallS *c = cast(struct CCallS *, ud);
  CClosure *cl;
  cl = luaF_newCclosure(L, 0, getcurrenv(L));
  cl->f = c->func;
  setclvalue(L, L->top, cl);  /* push function */
  api_incr_top(L);
  setpvalue(L->top, c->ud);  /* push only argument */
  api_incr_top(L);
  luaD_call(L, L->top - 2, 0);
}


LUA_API int lua_cpcall (lua_State *L, lua_CFunction func, void *ud) {
  struct CCallS c;
  int status;
  lua_lock(L);
  c.func = func;
  c.ud = ud;
  status = luaD_pcall(L, f_Ccall, &c, savestack(L, L->top), 0, NULL);
  lua_unlock(L);
  return status;
}


LUA_API int lua_load (lua_State *L, lua_Reader reader, void *data,
                      const char *chunkname) {
  ZIO z;
  int status;
  lua_lock(L);
  if (!chunkname) chunkname = "?";
  luaZ_init(L, &z, reader, data);
  status = luaD_protectedparser(L, &z, chunkname);
  lua_unlock(L);
  return status;
}


LUA_API int lua_dump (lua_State *L, lua_Writer writer, void *data)
{
    int status;
    TValue *o;
    lua_lock(L);

    api_checknelems(L, 1);
    o = L->top - 1;

    if (isLfunction(o))
        status = luaU_dump(L, clvalue(o)->GetLClosure()->p, writer, data, 0);
    else
        status = 1;

    lua_unlock(L);
    return status;
}


LUA_API int  lua_status (lua_State *L)
{
	if ( !L->IsThread() )
		return 0;

	return ((lua_Thread*)L)->status;
}


/*
** Garbage-collection function
*/

LUA_API int lua_gc (lua_State *L, int what, int data) {
  int res = 0;
  global_State *g;
  lua_lock(L);
  g = G(L);
  switch (what) {
    case LUA_GCSTOP: {
      g->GCthreshold = MAX_LUMEM;
      break;
    }
    case LUA_GCRESTART: {
      g->GCthreshold = g->totalbytes;
      break;
    }
    case LUA_GCCOLLECT: {
      luaC_fullgc(L);
      break;
    }
    case LUA_GCCOUNT: {
      /* GC values are expressed in Kbytes: #bytes/2^10 */
      res = cast_int(g->totalbytes >> 10);
      break;
    }
    case LUA_GCCOUNTB: {
      res = cast_int(g->totalbytes & 0x3ff);
      break;
    }
    case LUA_GCSTEP: {
      lu_mem a = (cast(lu_mem, data) << 10);
      if (a <= g->totalbytes)
        g->GCthreshold = g->totalbytes - a;
      else
        g->GCthreshold = 0;
      while (g->GCthreshold <= g->totalbytes) {
        luaC_step(L);
        if (g->gcstate == GCSpause) {  /* end of cycle? */
          res = 1;  /* signal it */
          break;
        }
      }
      break;
    }
    case LUA_GCSETPAUSE: {
      res = g->gcpause;
      g->gcpause = data;
      break;
    }
    case LUA_GCSETSTEPMUL: {
      res = g->gcstepmul;
      g->gcstepmul = data;
      break;
    }
    default: res = -1;  /* invalid option */
  }
  lua_unlock(L);
  return res;
}

LUA_API void lua_gcpaycost( lua_State *L, unsigned int cost )
{
    global_State *g = G(L);
    lua_Thread *gcL = g->GCthread;
    
    if ( gcL != L || gcL->status != THREAD_RUNNING )
        return;

    luaC_paycost( g, gcL, cost );
}


/*
** miscellaneous functions
*/

LUA_API void lua_pushtype( lua_State *L, int idx )
{
    const TValue *o = index2constadr( L, idx );

    if ( ttype(o) == LUA_TCLASS )
    {
        if ( jvalue(o)->destroyed )
        {
            lua_pushlstring( L, "destroyed_class", 15 );
            return;
        }

        const TValue *tm = luaT_gettmbyobj( L, o, TM_TYPE );

        switch( ttype(tm) )
        {
        case LUA_TSTRING:
            setobj2s( L, L->top, tm );
            api_incr_top( L );
            return;
        case LUA_TFUNCTION:
            setobj2s( L, L->top, tm );
            api_incr_top( L );
            setobj2s( L, L->top, o );
            api_incr_top( L );
            lua_call( L, 1, 1 );
            return;
        }
    }

    lua_pushstring(L, lua_typename( L, ttype(o) ));
}


LUA_API int lua_error (lua_State *L) {
  lua_lock(L);
  api_checknelems(L, 1);
  luaG_errormsg(L);
  lua_unlock(L);
  return 0;  /* to avoid warnings */
}

LUA_API bool lua_next( lua_State *L, int idx )
{
    const TValue *t;
    lua_lock(L);

    t = index2constadr(L, idx);
    api_check(L, ttistable(t));

    bool more = luaH_next(L, hvalue(t), L->top - 1);

    if ( more )
    {
        api_incr_top(L);
    }
    else  /* no more elements */
        L->top -= 1;  /* remove key */

    lua_unlock(L);
    return more;
}

LUA_API void lua_concat (lua_State *L, int n) {
  lua_lock(L);
  api_checknelems(L, n);
  if (n >= 2) {
    luaC_checkGC(L);
    luaV_concat(L, n, cast_int(L->top - L->base) - 1);
    L->top -= (n-1);
  }
  else if (n == 0) {  /* push empty string */
    setsvalue2s(L, L->top, luaS_newlstr(L, "", 0));
    api_incr_top(L);
  }
  /* else n == 1; nothing to do */
  lua_unlock(L);
}


LUA_API lua_Alloc lua_getallocf (lua_State *L, void **ud) {
  lua_Alloc f;
  lua_lock(L);
  if (ud) *ud = G(L)->ud;
  f = G(L)->frealloc;
  lua_unlock(L);
  return f;
}


LUA_API void lua_setallocf (lua_State *L, lua_Alloc f, void *ud) {
  lua_lock(L);
  G(L)->ud = ud;
  G(L)->frealloc = f;
  lua_unlock(L);
}


LUA_API void *lua_newuserdata (lua_State *L, size_t size) {
  Udata *u;
  lua_lock(L);
  luaC_checkGC(L);
  u = luaS_newudata(L, size, getcurrenv(L));
  setuvalue(L, L->top, u);
  api_incr_top(L);
  lua_unlock(L);
  return u + 1;
}

static const char *aux_upvalue (StkId fi, int n, TValue **val)
{
    Closure *f;

    if (!ttisfunction(fi))
        return NULL;

    f = clvalue(fi);

    if (f->isC)
    {
        if ( n < 1 || n > 256 )
            return NULL;

        TValue *up = f->ReadUpValue( (unsigned char)( n - 1 ) );

        if ( up == luaO_nilobject )
            return NULL;

        *val = up;
        return "";
    }
    else
    {
        LClosure *cl = f->GetLClosure();
        Proto *p = cl->p;

        if (!(1 <= n && n <= p->sizeupvalues))
            return NULL;

        *val = cl->upvals[n-1]->v;
        return getstr(p->upvalues[n-1]);
    }
}


LUA_API const char *lua_getupvalue (lua_State *L, int funcindex, int n) {
  const char *name;
  TValue *val;
  lua_lock(L);
  name = aux_upvalue(index2adr(L, funcindex), n, &val);
  if (name) {
    setobj2s(L, L->top, val);
    api_incr_top(L);
  }
  lua_unlock(L);
  return name;
}


LUA_API const char *lua_setupvalue (lua_State *L, int funcindex, int n) {
  const char *name;
  TValue *val;
  StkId fi;
  lua_lock(L);
  fi = index2adr(L, funcindex);
  api_checknelems(L, 1);
  name = aux_upvalue(fi, n, &val);
  if (name) {
    L->top--;
    setobj(L, val, L->top);
    luaC_barrier(L, clvalue(fi), L->top);
  }
  lua_unlock(L);
  return name;
}

LUA_API void lua_setevent( lua_State *L, eLuaEvent evt, lua_CFunction proto )
{
    if ( evt > LUA_NUM_EVENTS-1 )
        return;

    lua_pushcclosure( L, proto, 0 );

    L->top--;
    G(L)->events[evt] = clvalue( L->top );
}

LUA_API void lua_constructclassex( lua_State *L, int nargs, unsigned int flags )
{
    Class *c;
    lua_lock( L );

    c = luaJ_new( L, nargs, flags );
    setjvalue( L, L->top, c );
    api_incr_top( L );

    lua_unlock( L );
}

LUA_API void lua_constructclass( lua_State *L, int nargs )
{
    lua_constructclassex( L, nargs, 0 );
}

LUA_API void lua_newclass( lua_State *L )
{
    lua_constructclass( L, 0 );
}

LUA_API void lua_newclassex( lua_State *L, unsigned int flags )
{
    lua_constructclassex( L, 0, flags );
}

LUA_API ILuaClass* lua_refclass( lua_State *L, int idx )
{
    const TValue *val = index2constadr( L, idx );

    lua_assert( iscollectable( val ) );
    return ( val->value.gc )->GetClass();
}

LUA_API void lua_basicprotect( lua_State *L )
{
    luaJ_basicprotect( L );
}

LUA_API void lua_basicextend( lua_State *L )
{
    luaJ_basicextend( L );
}

LUA_API ILuaState& lua_getstateapi( lua_State *L )
{
    return *L;
}