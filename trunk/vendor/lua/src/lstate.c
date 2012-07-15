/*
** $Id: lstate.c,v 2.36.1.2 2008/01/03 15:20:39 roberto Exp $
** Global State
** See Copyright Notice in lua.h
*/


#include <stddef.h>

#define lstate_c
#define LUA_CORE

#include "lua.h"

#include "ldebug.h"
#include "ldo.h"
#include "lfunc.h"
#include "lgc.h"
#include "llex.h"
#include "lmem.h"
#include "lstate.h"
#include "lstring.h"
#include "ltable.h"
#include "ltm.h"


/*
** Main thread combines a thread state and the global state
*/
typedef struct LG {
  lua_State l;
  global_State g;
} LG;
  


static void stack_init (lua_State *L1, lua_State *L) {
  /* initialize CallInfo array */
  L1->base_ci = luaM_newvector(L, BASIC_CI_SIZE, CallInfo);
  L1->ci = L1->base_ci;
  L1->size_ci = BASIC_CI_SIZE;
  L1->end_ci = L1->base_ci + L1->size_ci - 1;
  /* initialize stack array */
  L1->stack = luaM_newvector(L, BASIC_STACK_SIZE + EXTRA_STACK, TValue);
  L1->stacksize = BASIC_STACK_SIZE + EXTRA_STACK;
  L1->top = L1->stack;
  L1->stack_last = L1->stack+(L1->stacksize - EXTRA_STACK)-1;
  /* initialize first ci */
  L1->ci->func = L1->top;
  setnilvalue(L1->top++);  /* `function' entry for this `ci' */
  L1->base = L1->ci->base = L1->top;
  L1->ci->top = L1->top + LUA_MINSTACK;
}


static void freestack (lua_State *L, lua_State *L1) {
  luaM_freearray(L, L1->base_ci, L1->size_ci, CallInfo);
  luaM_freearray(L, L1->stack, L1->stacksize, TValue);
}


/*
** open parts that may cause memory-allocation errors
*/
static void f_luaopen (lua_State *L, void *ud)
{
    global_State *g = G(L);
    UNUSED(ud);
    stack_init(L, L);  /* init stack */
    luaE_newenvironment( L );
    sethvalue(L, registry(L), luaH_new(L, 0, 2));  /* registry */
    luaS_resize(L, MINSTRTABSIZE);  /* initial size of string table */
    luaT_init(L);
    luaX_init(L);
    luaS_fix(luaS_newliteral(L, MEMERRMSG));
    g->GCthreshold = 4*g->totalbytes;
}


static inline void preinit_state (lua_State *L, global_State *g)
{
    G(L) = g;
    L->stack = NULL;
    L->stacksize = 0;
    L->errorJmp = NULL;
    L->hook = NULL;
    L->hookmask = 0;
    L->basehookcount = 0;
    L->allowhook = 1;
    resethookcount(L);
    L->openupval = NULL;
    L->size_ci = 0;
    L->nCcalls = 0;
    L->status = 0;
    L->base_ci = L->ci = NULL;
    L->savedpc = NULL;
    L->errfunc = 0;
    sethvalue( L, &L->storage, luaH_new( L, 0, 0 ) );
    setnilvalue(gt(L));
}

void* lua_State::operator new( size_t size, lua_Alloc f, void *ud )
{
    return (*f)(ud, NULL, 0, state_size(LG));
}

void lua_State::operator delete( void *ptr )
{
    global_State *g = G(((lua_State*)ptr));
    g->frealloc( g->ud, fromstate(ptr), state_size(LG), 0 );
}

static void close_state (lua_State *L)
{
    global_State *g = G(L);
    luaF_close(L, L->stack);  /* close all upvalues for this thread */
    luaC_freeall(L);  /* collect all objects */
    lua_assert(g->rootgc == L);
    lua_assert(g->strt.nuse == 0);
    luaM_freearray(L, G(L)->strt.hash, G(L)->strt.size, TString *);
    luaZ_freebuffer(L, &g->buff);
    freestack(L, L);
    lua_assert(g->totalbytes == sizeof(LG));
    delete L;
}

static DWORD __stdcall luaE_threadEntryPoint( void *ud )
{
    lua_Thread *L = (lua_Thread*)ud;

    try
    {
        WaitForSingleObject( L->signalBegin, INFINITE );

        lua_call( L, lua_gettop( L ) - 1, LUA_MULTRET );

        L->status = 0;
    }
    catch( lua_exception& e )
    {
        if ( G( e.getThread() )->mainthread != G( L )->mainthread )
        {
            // A different .lua runtime triggered an exception
            L->status = -1;
        }
        else
        {
            L->status = cast_byte( e.status() );  /* mark thread as `dead' */
            luaD_seterrorobj( L, L->status, L->top );
            L->ci->top = L->top;
        }

        lua_settop( L, 0 );
    }
    catch( lua_thread_termination& e )
    {
        L->status = e.status();

        lua_settop( L, 0 );
    }
    catch( ... )
    {
        L->status = -1;

        lua_settop( L, 0 );
    }

    SetEvent( L->signalWait );
    return L->status;
}

lua_Thread::lua_Thread()
{
#ifdef _WIN32
    signalBegin = CreateEvent( NULL, false, false, NULL );
    signalWait = CreateEvent( NULL, false, false, NULL );
#endif
    isMain = false;
    yieldDisabled = false;
}

lua_Thread* luaE_newthread( lua_State *L )
{
    unsigned int n;
    lua_Thread *L1 = new (L) lua_Thread;
    luaC_link(L, L1, LUA_TTHREAD);
    preinit_state(L1, G(L));
    stack_init(L1, L);  /* init stack */
    setobj2n(L, gt(L1), gt(L));  /* share table of globals */
    L1->hookmask = L->hookmask;
    L1->basehookcount = L->basehookcount;
    L1->hook = L->hook;
    resethookcount(L1);

    // Inherit the metatables
    for ( n=0; n<NUM_TAGS; n++ )
        L1->mt[n] = L->mt[n];

#ifdef _WIN32
    L1->threadHandle = CreateThread( NULL, 0, luaE_threadEntryPoint, L1, 0, NULL );

    if ( !L1->threadHandle )
        throw lua_exception( L, LUA_ERRRUN, "cannot allocate thread" );
#endif

    lua_assert(iswhite(L1));
    return L1;
}

static void luaE_term()
{
    lua_State *L;

    __asm
    {
        mov L,ebx
    }

    throw lua_thread_termination( L, 0 );
}

void luaE_terminate( lua_Thread *L )
{
    luaF_close(L, L->stack);  /* close all upvalues for this thread */
    lua_assert(L->openupval == NULL);

#ifdef _WIN32
    DWORD status;
    GetExitCodeThread( L->threadHandle, &status );

    if ( status == STILL_ACTIVE )
    {
        // Cleanly terminate the thread by exception
        CONTEXT context;
        context.ContextFlags = CONTEXT_INTEGER | CONTEXT_CONTROL;

        GetThreadContext( L->threadHandle, &context );

        context.Ebx = (DWORD)L;
        context.Eip = (DWORD)luaE_term;

        SetThreadContext( L->threadHandle, &context );

        SetEvent( L->signalBegin );
        
        WaitForSingleObject( L->threadHandle, INFINITE );
    }
#endif
}

lua_State::~lua_State()
{
}

lua_Thread::~lua_Thread()
{
    luaE_terminate( this );
    luai_userstatefree( this );

#ifdef _WIN32
    CloseHandle( threadHandle );
    CloseHandle( signalBegin );
    CloseHandle( signalWait );
#endif //_WIN32

    freestack( l_G->mainthread, this );
}

LUAI_FUNC void luaE_newenvironment( lua_State *L )
{
    Table *g = luaH_new( L, 0, 2 );
    sethvalue( L, gt(L), g );
    luaC_objbarrier( L, g, L );

    unsigned int n;

    for ( n=0; n<NUM_TAGS; n++ )
        L->mt[n] = NULL;
}

LUAI_FUNC lua_State *lua_newstate (lua_Alloc f, void *ud)
{
    unsigned int i;
    lua_State *L;
    global_State *g;
    lua_State *l = new (f, ud) lua_State;

    if ( l == NULL )
        return NULL;

    L = tostate(l);
    g = &((LG *)L)->g;
    L->next = NULL;
    L->tt = LUA_TTHREAD;
    g->currentwhite = bit2mask(WHITE0BIT, FIXEDBIT);
    L->marked = luaC_white(g);
    set2bits(L->marked, FIXEDBIT, SFIXEDBIT);
    g->frealloc = f;
    g->ud = ud;
    g->mainthread = L;
    g->uvhead.u.l.prev = &g->uvhead;
    g->uvhead.u.l.next = &g->uvhead;
    g->GCthreshold = 0;  /* mark it as unfinished state */
    g->strt.size = 0;
    g->strt.nuse = 0;
    g->strt.hash = NULL;
    preinit_state(L, g);
    setnilvalue(registry(L));
    luaZ_initbuffer(L, &g->buff);
    g->panic = NULL;
    g->gcstate = GCSpause;
    g->rootgc = L;
    g->sweepstrgc = 0;
    g->sweepgc = &g->rootgc;
    g->gray = NULL;
    g->grayagain = NULL;
    g->weak = NULL;
    g->tmudata = NULL;
    g->totalbytes = sizeof(LG);
    g->gcpause = LUAI_GCPAUSE;
    g->gcstepmul = LUAI_GCMUL;
    g->gcdept = 0;

    for ( i=0; i<LUA_NUM_EVENTS; i++ )
        g->events[i] = NULL;

    for ( i=0; i<NUM_TAGS; i++ )
        L->mt[i] = NULL;

    if ( luaD_rawrunprotected( L, f_luaopen, NULL ) != 0 )
    {
        /* memory allocation error: free partial state */
        close_state(L);
        return NULL;
    }

    luai_userstateopen(L);
    return L;
}

static void callallgcTM (lua_State *L, void *ud)
{
    UNUSED(ud);
    luaC_callGCTM(L);  /* call GC metamethods for all udata */
}


LUA_API void lua_close (lua_State *L)
{
    L = G(L)->mainthread;  /* only the main thread can be closed */
    lua_lock(L);
    luaF_close(L, L->stack);  /* close all upvalues for this thread */
    luaC_separatefinalization(L, 1);  /* separate udata that have GC metamethods */
    L->errfunc = 0;  /* no error function during GC metamethods */

    do
    {  /* repeat until no more errors */
        L->ci = L->base_ci;
        L->base = L->top = L->ci->base;
        L->nCcalls = 0;
    } while (luaD_rawrunprotected(L, callallgcTM, NULL) != 0);

    lua_assert(G(L)->tmudata == NULL);
    luai_userstateclose(L);
    close_state(L);
}

// MTA specific
LUA_API lua_State* lua_getmainstate (lua_State *L) {
  return G(L)->mainthread;
}

