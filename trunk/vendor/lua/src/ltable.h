/*
** $Id: ltable.h,v 2.10.1.1 2007/12/27 13:02:25 roberto Exp $
** Lua tables (hash)
** See Copyright Notice in lua.h
*/

#ifndef ltable_h
#define ltable_h

#include "lobject.h"


#define gnode(t,i)	(&(t)->node[i])
#define gkey(n)		(&(n)->i_key.nk)
#define gval(n)		(&(n)->i_val)
#define gnext(n)	((n)->i_key.nk.next)

#define key2tval(n)	(&(n)->i_key.tvk)


LUAI_FUNC ConstValueAddress     luaH_getnum (lua_State *L, Table *t, int key);
LUAI_FUNC ValueAddress          luaH_setnum (lua_State *L, Table *t, int key);
LUAI_FUNC ConstValueAddress     luaH_getstr (lua_State *L, Table *t, TString *key);
LUAI_FUNC ValueAddress          luaH_setstr (lua_State *L, Table *t, TString *key);
LUAI_FUNC ConstValueAddress     luaH_get (lua_State *L, Table *t, const TValue *key);
LUAI_FUNC ValueAddress          luaH_set (lua_State *L, Table *t, const TValue *key);
LUAI_FUNC Table*                luaH_new (lua_State *L, int narray, int lnhash);
LUAI_FUNC void                  luaH_free (lua_State *L, Table *t);
LUAI_FUNC void                  luaH_resizearray (lua_State *L, Table *t, int nasize);
LUAI_FUNC bool                  luaH_next (lua_State *L, Table *t);
LUAI_FUNC int                   luaH_getn (lua_State *L, Table *t);
LUAI_FUNC Table&                luaH_copy( lua_State *L, const Table& t );


#if defined(LUA_DEBUG)
LUAI_FUNC Node *luaH_mainposition (const Table *t, const TValue *key);
#endif

// Module initialization.
void luaH_init( lua_config *cfg );
void luaH_shutdown( lua_config *cfg );
void luaH_clearRuntimeMemory( global_State *g );

#endif
