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
#include "lzio.h"
#include "lfiber.h"


/* table of globals */
#define gt(L)	(&L->l_gt)

/* registry */
#define registry(L)	(&G(L)->l_registry)


/* extra stack space to handle TM calls and some other extras */
#define EXTRA_STACK   5


#define BASIC_CI_SIZE           8

#define BASIC_STACK_SIZE        (2*LUA_MINSTACK)



typedef struct stringtable {
  GCObject **hash;
  lu_int32 nuse;  /* number of elements */
  int size;
} stringtable;


/*
** informations about a call
*/
typedef struct CallInfo {
  StkId base;  /* base for this function */
  StkId func;  /* function index in the stack */
  StkId	top;  /* top for this function */
  const Instruction *savedpc;
  int nresults;  /* expected number of results from this function */
  int tailcalls;  /* number of tail calls lost under this entry */
} CallInfo;


#define curr_func(L)	(clvalue(L->ci->func))
#define ci_func(ci)	(clvalue((ci)->func))
#define f_isLua(ci)	(!ci_func(ci)->isC)
#define isLua(ci)	(ttisfunction((ci)->func) && f_isLua(ci))

// Internal functions
TValue *index2adr (lua_State *L, int idx);

class lua_Thread;

/*
** `global state', shared by all threads of this state
*/
typedef struct global_State
{
  stringtable strt;  /* hash table for strings */
  lua_Alloc frealloc;  /* function to reallocate memory */
  void *ud;         /* auxiliary data to `frealloc' */
  lu_byte currentwhite;
  lu_byte gcstate;  /* state of garbage collector */
  int sweepstrgc;  /* position of sweep in `strt' */
  GCObject *rootgc;  /* list of all collectable objects */
  GCObject **sweepgc;  /* position of sweep in `rootgc' */
  GrayObject *gray;  /* list of gray objects */
  GrayObject *grayagain;  /* list of objects to be traversed atomically */
  GrayObject *weak;  /* list of weak tables (to be cleared) */
  GCObject *tmudata;  /* last element of list of userdata to be GC */
  Mbuffer buff;  /* temporary buffer for string concatentation */
  lua_Thread *GCthread; /* garbage collector runtime */
  lu_mem GCthreshold;
  lu_mem GCcollect; /* amount of memory to be collected until stop */
  lu_mem totalbytes;  /* number of bytes currently allocated */
  lu_mem estimate;  /* an estimate of number of bytes actually in use */
  lu_mem gcdept;  /* how much GC is `behind schedule' */
  int gcpause;  /* size of pause between successive GCs */
  int gcstepmul;  /* GC `granularity' */

  TValue l_registry;
  lua_State *mainthread;
  UpVal uvhead;  /* head of double-linked list of all open upvalues */
  TString *tmname[TM_N];  /* array with tag-method names */
  TString *superCached; /* 'super' */
  Closure *events[LUA_NUM_EVENTS];

  RwList <lua_Thread> threads; /* all existing thread in this machine */
} global_State;

#define tostate(l)      (cast(lua_State *, cast(lu_byte *, l) + LUAI_EXTRASPACE))
#define state_size(x)	(sizeof(x) + LUAI_EXTRASPACE)
#define fromstate(l)	(cast(lu_byte *, (l)) - LUAI_EXTRASPACE)


/*
** `per thread' state
*/
class lua_State : public GrayObject, virtual public ILuaState
{
public:
    ~lua_State();

    // lua_State is always the main thread
    virtual void    SetMainThread( bool enabled )       {}
    virtual bool    IsThread()                          { return false; }

    virtual void    SetYieldDisabled( bool disable )    {}
    virtual bool    IsYieldDisabled()                   { return true; }

    size_t Propagate( global_State *g );

    void* operator new( size_t size, lua_Alloc f, void *ud );
    void operator delete( void *ptr );

    lu_byte status;
    StkId top;  /* first free slot in the stack */
    StkId base;  /* base of current function */
    global_State *l_G;
    CallInfo *ci;  /* call info for current function */
    const Instruction *savedpc;  /* `savedpc' of current function */
    StkId stack_last;  /* last free slot in the stack */
    StkId stack;  /* stack base */
    CallInfo *end_ci;  /* points after end of ci array*/
    CallInfo *base_ci;  /* array of CallInfo's */
    int stacksize;
    int size_ci;  /* size of array `base_ci' */
    unsigned short nCcalls;  /* number of nested C calls */
    lu_byte hookmask;
    bool allowhook;
    int basehookcount;
    int hookcount;
    lua_Hook hook;
    TValue l_gt;  /* table of globals */
    TValue env;  /* temporary place for environments */
    GCObject *openupval;  /* list of open upvalues in this stack */
    ptrdiff_t errfunc;  /* current error handling function (stack index) */
    TValue storage;
    Table *mt[NUM_TAGS];  /* metatables for basic types */
};

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

    void    SetMainThread( bool enable )        { isMain = enable; }
    bool    IsThread()                          { return !isMain; }

    bool    AllocateRuntime();
    bool    IsRuntimeAllocated() const          { return fiber != NULL; }

    void    SetYieldDisabled( bool disable )    { yieldDisabled = disable; }
    bool    IsYieldDisabled()                   { return yieldDisabled; }

    void* operator new( size_t size, lua_State *main )
    {
        return GCObject::operator new( size + LUAI_EXTRASPACE, main );
    }

    void operator delete( void *ptr )
    {
        GCObject::operator delete( fromstate( (lua_Thread*)ptr ), state_size( lua_Thread ) );
    }
    
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

