#include "luacore.h"

#include "lapi.h"
#include "lfunc.h"
#include "lgc.h"
#include "lmem.h"
#include "lobject.h"
#include "lstate.h"
#include "lstring.h"
#include "ltable.h"
#include "lclass.h"

#include "lapi.hxx"


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


LUA_API void lua_pushwideinteger( lua_State *L, lua_WideInteger n )
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
  setbvalue(L->top, (b != 0));  /* ensure that C integer 1 is true */
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
