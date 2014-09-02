/*
** $Id: ltm.c,v 2.8.1.1 2007/12/27 13:02:25 roberto Exp $
** Tag methods
** See Copyright Notice in lua.h
*/


#include "luacore.h"

#include "lobject.h"
#include "lstate.h"
#include "lstring.h"
#include "ltable.h"
#include "ltm.h"



const char *const luaT_typenames[] = {
  "nil", "boolean", "userdata", "number",
  "string", "table", "function", "userdata", "class", "dispatch", "thread",
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

unsigned int luaT_getnumtypes( lua_State *L )
{
    UNUSED(L);
    return NUMELMS( luaT_typenames );
}

int luaT_gettype( lua_State *L, unsigned int index )
{
    UNUSED(L);
    
    const int theTypes[] =
    {
        LUA_TNIL,
        LUA_TBOOLEAN,
        LUA_TUSERDATA,
        LUA_TNUMBER,
        LUA_TSTRING,
        LUA_TTABLE,
        LUA_TFUNCTION,
        LUA_TUSERDATA,
        LUA_TCLASS,
        LUA_TDISPATCH,
        LUA_TTHREAD,
        LUA_TPROTO,
        LUA_TUPVAL
    };

    int iTypeNum = -1;

    if ( index < NUMELMS(theTypes) )
    {
        iTypeNum = theTypes[ index ];
    }

    return iTypeNum;
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
    events->flags |= bitmask((lu_byte)event);  /* cache this fact */
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
    }

    return L->mt[ttype(o)];
}

const TValue *luaT_gettmbyobj( lua_State *L, const TValue *o, TMS event )
{
    if ( ttype(o) == LUA_TCLASS )
    {
        const TValue *res = luaH_getstr( jvalue(o)->storage, G(L)->tmname[event] );
        if ( res )
            return res;
    }
    Table *mt = luaT_getmetabyobj( L, o );
    return (mt ? luaH_getstr( mt, G(L)->tmname[event] ) : luaO_nilobject);
}

// Module initialization.
void luaT_moduleinit( lua_config *cfg )
{
    return;
}

void luaT_moduleshutdown( lua_config *cfg )
{
    return;
}