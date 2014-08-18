// Lua GC internal definitions.
#ifndef _LUA_GC_INTERNALS_
#define _LUA_GC_INTERNALS_

#include "lobject.h"

extern globalStatePluginOffset_t _gcEnvPluginOffset;

struct globalStateGCEnv
{
    lu_byte gcstate;  /* state of garbage collector */
    int sweepstrgc;  /* position of sweep in `strt' */
    GCObject *rootgc;  /* list of all collectable objects */
    GCObject **sweepgc;  /* position of sweep in `rootgc' */
    GrayObject *gray;  /* list of gray objects */
    GrayObject *grayagain;  /* list of objects to be traversed atomically */
    GrayObject *weak;  /* list of weak tables (to be cleared) */
    GCObject *tmudata;  /* last element of list of userdata to be GC */
    lua_Thread *GCthread; /* garbage collector runtime */
    lu_mem GCthreshold;
    lu_mem GCcollect; /* amount of memory to be collected until stop */
    lu_mem estimate;  /* an estimate of number of bytes actually in use */
    lu_mem gcdept;  /* how much GC is `behind schedule' */
    int gcpause;  /* size of pause between successive GCs */
    int gcstepmul;  /* GC `granularity' */
};

inline globalStateGCEnv* GetGlobalGCEnvironment( global_State *g )
{
    return globalStateFactory_t::RESOLVE_STRUCT <globalStateGCEnv> ( g, _gcEnvPluginOffset );
}

#endif //_LUA_GC_INTERNALS_