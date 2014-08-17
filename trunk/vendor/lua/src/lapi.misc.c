#include "luacore.h"

#include "lapi.h"
#include "ldebug.h"
#include "lgc.h"
#include "lstate.h"
#include "lstring.h"
#include "ltable.h"
#include "lclass.h"
#include "lvm.h"

#include "lapi.hxx"


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

    if ( !iscollectable( val ) )
        return NULL;

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