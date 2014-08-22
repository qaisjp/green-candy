// Lua Closure interface local shared include
#ifndef _LUA_CLOSURE_SHARED_INCLUDE_
#define _LUA_CLOSURE_SHARED_INCLUDE_

#include "lfunc.h"
#include "lgc.h"
#include "lmem.h"
#include "lobject.h"
#include "lstate.h"

// Closure global state plugin.
extern globalStatePluginOffset_t _closureEnvPluginOffset;

struct globalStateClosureEnvPlugin
{
    inline globalStateClosureEnvPlugin( void )
    {
        uvhead.u.l.prev = &uvhead;
        uvhead.u.l.next = &uvhead;
    }

    UpVal uvhead;  /* head of double-linked list of all open upvalues */
};

inline globalStateClosureEnvPlugin* GetGlobalClosureEnv( global_State *g )
{
    return globalStateFactory_t::RESOLVE_STRUCT <globalStateClosureEnvPlugin> ( g, _closureEnvPluginOffset );
}

#endif //_LUA_CLOSURE_SHARED_INCLUDE_