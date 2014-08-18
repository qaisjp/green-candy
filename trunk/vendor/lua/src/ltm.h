/*
** $Id: ltm.h,v 2.6.1.1 2007/12/27 13:02:25 roberto Exp $
** Tag methods
** See Copyright Notice in lua.h
*/

#ifndef ltm_h
#define ltm_h


#include "lobject.h"


LUAI_FUNC const TValue *luaT_gettm (Table *events, TMS event, TString *ename);
LUAI_FUNC Table* luaT_getmetabyobj( lua_State *L, const TValue *o );
LUAI_FUNC const TValue *luaT_gettmbyobj (lua_State *L, const TValue *o, TMS event);
LUAI_FUNC void luaT_init (lua_State *L);

#ifdef LUA_USE_C_MACROS

#define gfasttm(g,et,e) ((et) == NULL ? NULL : \
  ((et)->flags & (1u<<(e))) ? NULL : luaT_gettm(et, e, (g)->tmname[e]))

#define fasttm(l,et,e)	gfasttm(G(l), et, e)

#else

FASTAPI const TValue* gfasttm( global_State *g, Table *et, TMS e )
{
    if ( et == NULL )
        return NULL;

    const TValue *outTM = NULL;

    if ( testbit( (et)->flags, (lu_byte)e ) == false )
    {
        outTM = luaT_gettm(et, e, (g)->tmname[e]);
    }

    return outTM;
}
FASTAPI const TValue* fasttm( lua_State *L, Table *et, TMS e )          { return gfasttm( G(L), et, e ); }

#endif

LUAI_DATA const char *const luaT_typenames[];


// Module initialization.
LUAI_FUNC void luaT_moduleinit( void );
LUAI_FUNC void luaT_moduleshutdown( void );

#endif
