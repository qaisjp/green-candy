/*
** $Id: ltm.h,v 2.6.1.1 2007/12/27 13:02:25 roberto Exp $
** Tag methods
** See Copyright Notice in lua.h
*/

#ifndef ltm_h
#define ltm_h


#include "lobject.h"


LUAI_FUNC bool luaT_gettm (lua_State *L, Table *events, TMS event, TString *ename, ConstValueAddress& outTM);
LUAI_FUNC Table* luaT_getmetabyobj( lua_State *L, const TValue *o );
LUAI_FUNC ConstValueAddress luaT_gettmbyobj (lua_State *L, const TValue *o, TMS event);
LUAI_FUNC void luaT_init (lua_State *L);
LUAI_FUNC unsigned int luaT_getnumtypes (lua_State *L);
LUAI_FUNC int luaT_gettype (lua_State *L, unsigned int index);

FASTAPI ConstValueAddress gfasttm( global_State *g, Table *et, TMS e )
{
    ConstValueAddress retAddr;

    if ( et != NULL )
    {
        // Use the table optimization flags.
        // They are reset on any table modification.
        if ( testbit( (et)->flags, (lu_byte)e ) == false )
        {
            luaT_gettm( g->mainthread, et, e, (g)->tmname[e], retAddr );
        }
    }
    return retAddr;
}
FASTAPI ConstValueAddress fasttm( lua_State *L, Table *et, TMS e )      { return gfasttm( G(L), et, e ); }

// Table of old-style type names.
LUAI_DATA const char *const luaT_typenames[];


// Module initialization.
LUAI_FUNC void luaT_moduleinit( lua_config *cfg );
LUAI_FUNC void luaT_moduleshutdown( lua_config *cfg );

#endif
