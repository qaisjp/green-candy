#include "luacore.h"

#include "lapi.h"
#include "ldebug.h"
#include "lgc.h"

#include "lapi.hxx"


/*
** basic stack manipulation
*/

LUA_API int lua_gettop (lua_State *L) {
  return cast_int(L->top - L->base);
}

LUA_API void lua_settop (lua_State *L, int idx)
{
    lua_lock(L);
    if (idx >= 0)
    {
        api_check(L, idx <= L->stack_last - L->base);

        while (L->top < L->base + idx)
        {
            setnilvalue(L->top++);
        }

        L->top = L->base + idx;

        assert( L->top != NULL );
    }
    else
    {
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
