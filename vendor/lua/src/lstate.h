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



struct lua_longjmp;  /* defined in ldo.c */


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


/*
** `global state', shared by all threads of this state
*/
typedef struct global_State {
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
  lu_mem GCthreshold;
  lu_mem totalbytes;  /* number of bytes currently allocated */
  lu_mem estimate;  /* an estimate of number of bytes actually in use */
  lu_mem gcdept;  /* how much GC is `behind schedule' */
  int gcpause;  /* size of pause between successive GCs */
  int gcstepmul;  /* GC `granularity' */
  lua_CFunction panic;  /* to be called in unprotected errors */
  TValue l_registry;
  lua_State *mainthread;
  UpVal uvhead;  /* head of double-linked list of all open upvalues */
  Table *mt[NUM_TAGS];  /* metatables for basic types */
  TString *tmname[TM_N];  /* array with tag-method names */
  lua_CFunction events[LUA_NUM_EVENTS];
} global_State;


/*
** `per thread' state
*/
class lua_State : public GrayObject
{
public:
    ~lua_State();

    size_t Propagate( global_State *g );

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
    unsigned short baseCcalls;  /* nested C calls when resuming coroutine */
    lu_byte hookmask;
    lu_byte allowhook;
    int basehookcount;
    int hookcount;
    lua_Hook hook;
    TValue l_gt;  /* table of globals */
    TValue env;  /* temporary place for environments */
    GCObject *openupval;  /* list of open upvalues in this stack */
    struct lua_longjmp *errorJmp;  /* current error recover point */
    ptrdiff_t errfunc;  /* current error handling function (stack index) */
#ifdef _WIN32
    HANDLE threadHandle;
    HANDLE signalBegin;
    HANDLE signalWait;
#endif //_WIN32
};


#define G(L)	(L->l_G)


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

/* macro to convert any Lua object into a GCObject */
#define obj2gco(v)	(cast(GCObject *, (v)))


LUAI_FUNC lua_State *luaE_newthread (lua_State *L);
LUAI_FUNC void luaE_terminate( lua_State *L, lua_State *L1 );
LUAI_FUNC void luaE_freethread (lua_State *L, lua_State *L1);

#endif

