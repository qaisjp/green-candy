#ifndef _LUA_API_INTERNALS_
#define _LUA_API_INTERNALS_

#include "lstate.h"

inline GCObject *getcurrenv( lua_State *L )
{
    if ( L->ci == L->base_ci )  /* no enclosing function? */
        return gcvalue(gt(L));  /* use global table as environment */

    return curr_func( L )->env;
}

#endif //_LUA_API_INTERNALS_