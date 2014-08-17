#include "luacore.h"

#include "lapi.h"
#include "lfunc.h"
#include "lgc.h"
#include "lstate.h"
#include "lstring.h"
#include "ltable.h"
#include "lclass.h"
#include "ltm.h"
#include "lvm.h"

#include "lapi.hxx"


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
                luaC_objbarrier(L, uvalue(obj), mt);
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

