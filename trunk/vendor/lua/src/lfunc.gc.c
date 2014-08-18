#include "luacore.h"

#include "lfunc.hxx"

// Include garbage collector definitions.
#include "lgc.interface.hxx"

void luaF_gcruntime( lua_State *L )
{
    global_State *g = G(L);

    globalStateClosureEnvPlugin *closureEnv = GetGlobalClosureEnv( g );

    if ( closureEnv )
    {
        /* remark occasional upvalues of (maybe) dead threads */
        for ( UpVal *uv = closureEnv->uvhead.u.l.next; uv != &closureEnv->uvhead; uv = uv->u.l.next )
        {
            lua_assert( uv->u.l.next->u.l.prev == uv && uv->u.l.prev->u.l.next == uv );

            if ( isgray(uv) )
            {
                markvalue(g, uv->v);
            }
        }
    }
}