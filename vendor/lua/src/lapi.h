/*
** $Id: lapi.h,v 2.2.1.1 2007/12/27 13:02:25 roberto Exp $
** Auxiliary functions from Lua API
** See Copyright Notice in lua.h
*/

#ifndef lapi_h
#define lapi_h


#include "lobject.h"


#define api_checknelems(L, n)	api_check(L, (n) <= (L->top - L->base))

#define api_checkvalidindex(L, i)	api_check(L, (i) != luaO_nilobject)

#if 0
#define api_incr_top(L)   {api_check(L, L->top < L->ci->top); L->top++;}
#else
#define api_incr_top(L)   { if ( L->top >= L->ci->top ) { __asm { int 3 }; } L->top++;}
#endif

LUAI_FUNC void luaA_pushobject (lua_State *L, const TValue *o);

// Module initialization.
LUAI_FUNC void luaapi_init( void );
LUAI_FUNC void luaapi_shutdown( void );

#endif
