/*
** $Id: lstate.h,v 2.24.1.2 2008/01/03 15:20:39 roberto Exp $
** Global State
** See Copyright Notice in lua.h
*/

#ifndef lstate_h
#define lstate_h

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif //_WIN32

#include "lua.h"

#include "lobject.h"
#include "ltm.h"
#include "lfiber.h"


// Internal functions
LUAI_FUNC TValue *index2adr (lua_State *L, int idx);
LUAI_FUNC const TValue *index2constadr( lua_State *L, int idx );

// Since we use advanced memory allocation techniques, we depend on custom
// memory management templates.
#include <MemoryUtils.h>

typedef StaticPluginClassFactory <global_State> globalStateFactory_t;

typedef globalStateFactory_t::pluginOffset_t globalStatePluginOffset_t;

extern globalStateFactory_t globalStateFactory;

// Put plugin ids here!
#define GLOBAL_STATE_PLUGIN_ALLOC_HOLD          0x00000000
#define GLOBAL_STATE_PLUGIN_MAIN_STATE          0x00000001
#define GLOBAL_STATE_PLUGIN_STRING_META         0x00000002

enum eLuaThreadStatus : unsigned char
{
	THREAD_RUNNING,
	THREAD_SUSPENDED,
	THREAD_TERMINATED
};

class lua_Thread : public lua_State
{
public:
    lua_Thread();
    ~lua_Thread();

    lu_mem  GetTypeSize( global_State *g ) const;

    void    SetMainThread( bool enable )        { isMain = enable; }
    bool    IsThread()                          { return !isMain; }

    bool    AllocateRuntime();
    bool    IsRuntimeAllocated() const          { return fiber != NULL; }

    void    SetYieldDisabled( bool disable )    { yieldDisabled = disable; }
    bool    IsYieldDisabled()                   { return yieldDisabled; }
    
    inline void resume()
    {
        Fiber _callee;
        callee = &_callee;

        luaX_switch( &_callee, fiber );
    }

    inline void yield()
    {
        status = THREAD_SUSPENDED;

        // Optimized yield qswitch
        luaX_qswitch( fiber, callee );

        status = THREAD_RUNNING;
    }

    bool isMain;
    bool yieldDisabled;
    Fiber *callee;
    Fiber *fiber;
    eLuaThreadStatus status;
	int errorCode;

    RwListEntry <lua_Thread> threadNode;
};

/* macros to convert a GCObject into a specific value */
#define rawgco2ts(o)	check_exp((o)->tt == LUA_TSTRING, (TString*)(o))
#define gco2ts(o)	(rawgco2ts(o))
#define rawgco2u(o)	check_exp((o)->tt == LUA_TUSERDATA, (Udata*)(o))
#define gco2u(o)	(rawgco2u(o))
#define gco2cl(o)	check_exp((o)->tt == LUA_TFUNCTION, (Closure*)(o))
#define gco2h(o)	check_exp((o)->tt == LUA_TTABLE, (Table*)(o))
#define gco2j(o)    check_exp((o)->tt == LUA_TCLASS, (Class*)(o))
#define gco2p(o)	check_exp((o)->tt == LUA_TPROTO, (Proto*)(o))
#define gco2uv(o)	check_exp((o)->tt == LUA_TUPVAL, (UpVal*)(o))
#define ngcotouv(o) check_exp((o) == NULL || (o)->tt == LUA_TUPVAL, (UpVal*)(o))
#define gco2th(o)	check_exp((o)->tt == LUA_TTHREAD, (lua_State*)(o))


LUAI_FUNC lua_Thread* luaE_newthread (lua_State *L);
LUAI_FUNC void luaE_newenvironment( lua_State *L );
LUAI_FUNC void luaE_terminate( lua_Thread *L );
LUAI_FUNC void luaE_freethread (lua_State *L, lua_State *L1);

#endif

