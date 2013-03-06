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
    L->hook = NULL;
    L->hookmask = 0;
    L->basehookcount = 0;
    L->allowhook = true;
    resethookcount(L);
    L->openupval = NULL;
    L->size_ci = 0;
    L->nCcalls = 0;
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

static void __stdcall luaE_threadEntryPoint( lua_Thread *L )
{
    try
    {
        // First yield is invisible
        L->yield();

        lua_call( L, lua_gettop( L ) - 1, LUA_MULTRET );

		L->errorCode = 0;	// we gracefully quit :)
    }
    catch( lua_exception& e )
    {
        if ( G( e.getThread() )->mainthread != G( L )->mainthread )
        {
            // We do not need any stack values
            lua_settop( L, 0 );

			L->errorCode = LUA_ERRSYS;
        }
        else
        {
			L->errorCode = e.status();

            luaD_seterrorobj( L, L->errorCode, L->top );
            L->ci->top = L->top;
        }
    }
    catch( lua_thread_termination& e )
    {
        lua_settop( L, 0 );

		L->errorCode = e.status();
    }
    catch( ... )
    {
        lua_settop( L, 0 );

		L->errorCode = LUA_ERRSYS;
    }
}

lua_Thread::lua_Thread()
{
    isMain = false;
    yieldDisabled = false;
    callee = NULL;
    fiber = NULL;
	status = THREAD_SUSPENDED;
}

lua_Thread* luaE_newthread( lua_State *L )
{
    unsigned int n;
    lua_Thread *L1 = new (L) lua_Thread;
    luaC_link(L, L1, LUA_TTHREAD);
    LIST_INSERT( G(L)->threads.root, L1->threadNode );  /* we need to be aware of all threads */
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

    // Allocate the OS resources only if necessary!

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
    if ( !L->fiber )
        return;

    Fiber *env = L->fiber;

    if ( L->status != THREAD_TERMINATED )
    {
        // Throw an exception on the fiber
        env->ebx = (unsigned int)L;
        env->eip = (unsigned int)luaE_term;

        // We want to eventually return back
        L->resume();
    }
	else
	{
        luaF_close(L, L->stack);  /* close all upvalues for this thread */
        lua_assert(L->openupval == NULL);

		// Threads clean their environments after themselves
		luaX_closefiber( L, env );

		L->fiber = NULL;
	}
}

bool lua_Thread::AllocateRuntime()
{
    if ( fiber )
        return true;

    fiber = luaX_newfiber( this, 0, luaE_threadEntryPoint );
    
    if ( fiber != NULL )
    {
        // initiate it
        resume();
        return true;
    }

    return false;
}

lua_State::~lua_State()
{
}

lua_Thread::~lua_Thread()
{
    // unlist ourselves
    LIST_REMOVE( threadNode );

    luaE_terminate( this );
    luai_userstatefree( this );

    freestack( l_G->mainthread, this );
}

LUAI_FUNC void luaE_newenvironment( lua_State *L )
{
    Table *g = luaH_new( L, 0, 2 );
    sethvalue( L, gt(L), g );
    luaC_objbarriert( L, g, L );

    for ( unsigned int n = 0; n < NUM_TAGS; n++ )
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
    g->strt.size = 0;
    g->strt.nuse = 0;
    g->strt.hash = NULL;
    g->weak = NULL;
    g->tmudata = NULL;
    g->totalbytes = sizeof(LG);
    // Initialize the garbage collector
    luaC_init( g );
    preinit_state( L, g );
    luaE_newenvironment( L );   // create the first environment; threads will inherit it
    sethvalue(L, &g->l_registry, luaH_new(L, 0, 2));  /* registry */
    luaZ_initbuffer(L, &g->buff);

    for ( i=0; i<LUA_NUM_EVENTS; i++ )
        g->events[i] = NULL;

    for ( i=0; i<NUM_TAGS; i++ )
        L->mt[i] = NULL;

    std::string errMsg;

    // Initialize GCthread
    luaC_initthread( g );

    if ( luaD_rawrunprotected( L, f_luaopen, NULL, errMsg, NULL ) != 0 )
    {
        /* memory allocation error: free partial state */
        close_state(L);
        return NULL;
    }

    // Init runtime globals
    g->superCached = luaS_newlstr( L, "super", 5 );

    luai_userstateopen(L);
    return L;
}

LUA_API void lua_close (lua_State *L)
{
    L = G(L)->mainthread;  /* only the main thread can be closed */

    lua_lock(L);
    luaF_close(L, L->stack);  /* close all upvalues for this thread */

    // Collect all pending memory
    luaC_shutdown( G(L) );

    lua_assert(G(L)->tmudata == NULL);
    luai_userstateclose(L);
    close_state(L);
}

// MTA specific
LUA_API lua_State* lua_getmainstate (lua_State *L) {
  return G(L)->mainthread;
}

