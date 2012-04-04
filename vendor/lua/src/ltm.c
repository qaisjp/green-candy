/*
** $Id: ltm.c,v 2.8.1.1 2007/12/27 13:02:25 roberto Exp $
** Tag methods
** See Copyright Notice in lua.h
*/


#include <string.h>

#define ltm_c
#define LUA_CORE

#include "lua.h"

#include "lobject.h"
#include "lstate.h"
#include "lstring.h"
#include "ltable.h"
#include "ltm.h"



const char *const luaT_typenames[] = {
  "nil", "boolean", "userdata", "number",
  "string", "table", "function", "userdata", "class", "thread",
  "proto", "upval"
};


void luaT_init (lua_State *L) {
  static const char *const luaT_eventname[] = {  /* ORDER TM */
    "__index",
    "__newindex",
    "__gc",
    "__mode",
    "__eq",
    "__add",
    "__sub",
    "__mul",
    "__div",
    "__mod",
    "__pow",
    "__unm",
    "__len",
    "__lt",
    "__le",
    "__concat",
    "__call",
    "__redirect",
    "__metatable",
    "__type"
  };
  int i;
  for (i=0; i<TM_N; i++) {
    G(L)->tmname[i] = luaS_new(L, luaT_eventname[i]);
    luaS_fix(G(L)->tmname[i]);  /* never collect these names */
  }
}


/*
** function to be used with macro "fasttm": optimized for absence of
** tag methods
*/
const TValue *luaT_gettm (Table *events, TMS event, TString *ename)
{
  const TValue *tm = luaH_getstr(events, ename);
  lua_assert(event <= TM_EQ);
  if (ttisnil(tm)) {  /* no tag method? */
    events->flags |= cast_byte(1u<<event);  /* cache this fact */
    return NULL;
  }
  else return tm;
}

inline Table* luaT_getmetabyobj( lua_State *L, const TValue *o )
{
    switch( ttype(o) )
    {
    case LUA_TTABLE:
        return hvalue(o)->metatable;
    case LUA_TUSERDATA:
        return uvalue(o)->metatable;
    case LUA_TCLASS:
        return jvalue(o)->meta;
    }

    return G(L)->mt[ttype(o)];
}

const TValue *luaT_gettmbyobj( lua_State *L, const TValue *o, TMS event )
{
    if ( ttype(o) == LUA_TCLASS )
    {
        const TValue *res = luaH_getstr( jvalue(o)->env, G(L)->tmname[event] );
        if ( res )
            return res;
    }
    Table *mt = luaT_getmetabyobj( L, o );
    return (mt ? luaH_getstr( mt, G(L)->tmname[event] ) : luaO_nilobject);
}