// Lua Class type GC runtime.
#include "luacore.h"

#include "lclass.hxx"

#include "lgc.interface.hxx"

void luaJ_gcruntime( lua_State *L )
{
    global_State *g = G(L);

    globalStateClassEnvPlugin *classEnv = GetGlobalClassEnv( g );

    if ( classEnv )
    {
        stringmark( classEnv->superCached );   // 'super' used by classes
    }
}