#include "luacore.h"

#include "lapi.h"
#include "lfunc.h"
#include "lgc.h"
#include "lstring.h"
#include "ltable.h"
#include "lclass.h"
#include "lvm.h"

#include "lapi.hxx"


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

