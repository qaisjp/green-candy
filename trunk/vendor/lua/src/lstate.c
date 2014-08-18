/*
** $Id: lstate.c,v 2.36.1.2 2008/01/03 15:20:39 roberto Exp $
** Global State
** See Copyright Notice in lua.h
*/


#include "luacore.h"

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
#include "lclass.h"
#include "lvm.h"
#include "lapi.h"



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
    luaC_setthreshold( L, 4*g->totalbytes );
}


static inline void preinit_state (lua_State *L, global_State *g)
{
    L->l_G = g;
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

static int guard_code;
static _EXCEPTION_POINTERS *guard_exception;

static int guard_filter( int exceptCode, struct _EXCEPTION_POINTERS *exceptInfo )
{
    guard_code = exceptCode;
    guard_exception = exceptInfo;
    return 1;
}

static void __stdcall luaE_guardedThreadEntryPoint( lua_Thread *L )
{
    __try
    {
        luaE_threadEntryPoint( L );
    }
    __except( guard_filter( GetExceptionCode(), GetExceptionInformation() ) )
    {
        __asm int 3
        SetThreadContext( GetCurrentThread(), guard_exception->ContextRecord );
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

// Factory for thread creation and destruction.
lua_Thread* luaE_newthread( lua_State *L )
{
    lua_Thread *L1 = lua_new <lua_Thread> ( L );

    if ( L1 )
    {
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
        for ( unsigned int n = 0; n < NUM_TAGS; n++ )
        {
            L1->mt[n] = L->mt[n];
        }

        // Allocate the OS resources only if necessary!

        lua_assert(iswhite(L1));
    }
    return L1;
}

lu_mem lua_Thread::GetTypeSize( global_State *g ) const
{
    return sizeof( *this );
}

void luaE_freethread( lua_State *L, lua_State *L1 )
{
    lua_delete <lua_Thread> ( L, (lua_Thread*)L1 );
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

    fiber = luaX_newfiber( this, 0, luaE_guardedThreadEntryPoint );
    
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
    {
        L->mt[n] = NULL;
    }
}

// THIS is the entry point to the Lua library!
// Hence we must manage this using library logic, too.
static unsigned int _libraryReferenceCount = 0;

struct GeneralMemoryAllocator
{
    lua_Alloc allocCallback;
    void *userdata;

    inline GeneralMemoryAllocator( void *userdata, lua_Alloc allocCallback )
    {
        this->allocCallback = allocCallback;
        this->userdata = userdata;
    }

    inline ~GeneralMemoryAllocator( void )
    {
        return;
    }

    inline void* Allocate( size_t memSize )
    {
        return allocCallback( this->userdata, NULL, 0, memSize );
    }

    inline void Free( void *ptr, size_t memSize )
    {
        allocCallback( this->userdata, ptr, memSize, 0 );
    }
};

// Program-wide global state plugin registry.
globalStateFactory_t globalStateFactory;

struct GlobalStateAllocPluginData
{
    inline GlobalStateAllocPluginData( lua_Alloc allocCallback, void *userdata ) : _memAlloc( userdata, allocCallback )
    {
        this->constructor = globalStateFactory.CreateConstructor( _memAlloc );
    }

    inline ~GlobalStateAllocPluginData( void )
    {
        if ( this->constructor )
        {
            globalStateFactory.DeleteConstructor( this->constructor );
        }
    }

    // Structure that is used for bootstrapping with anonymous destruction support.
    GeneralMemoryAllocator _memAlloc;
    globalStateFactory_t::DeferredConstructor <GeneralMemoryAllocator> *constructor;
};

static void close_state (lua_State *L)
{
    global_State *g = G(L);

    luaF_close(L, L->stack);  /* close all upvalues for this thread */
    luaC_freeall(L);  /* collect all objects */
    lua_assert(g->strt.nuse == 0);
    luaM_freearray(L, G(L)->strt.hash, G(L)->strt.size, TString *);

    // Call post-state destructors.
    luaS_stateshutdown( L );

    freestack(L, L);
}

static void dealloc_state( global_State *globalState )
{
    // Delete the plugin object and destroy the allocator info.
    GlobalStateAllocPluginData *allocData = (GlobalStateAllocPluginData*)globalState->allocData;

    // If this assert fails, then GC is flawed because it did not clean up everything.
    // A flawed GC can have many reasons, even can be caused by the application!
    lua_assert(globalState->totalbytes == globalStateFactory.GetClassSize());

    allocData->constructor->Destroy( globalState );

    // Get allocator information to destroy the last bit of data.
    GeneralMemoryAllocator memAlloc( allocData->_memAlloc );

    // Destroy allocation data.
    allocData->~GlobalStateAllocPluginData();

    void *allocDataMem = allocData;

    memAlloc.Free( allocDataMem, sizeof( GlobalStateAllocPluginData ) );
}

struct mainThreadLuaStatePluginInterface : public globalStateFactory_t::pluginInterface
{
    bool OnPluginConstruct( global_State *g, globalStatePluginOffset_t pluginOffset, unsigned int pluginId )
    {
        // Attempt to get the main thread.
        void *mainThreadMemory = globalStateFactory_t::RESOLVE_STRUCT <void> ( g, pluginOffset );

        if ( mainThreadMemory == NULL )
            return false;

        // Construct the lua_State.
        lua_State *L = new (mainThreadMemory) lua_State;

        if ( L == NULL )
            return false;

        // Initialize the main thread.
        L->next = NULL;
        L->tt = LUA_TTHREAD;
        g->currentwhite = bit2mask(WHITE0BIT, FIXEDBIT);
        L->marked = luaC_white(g);
        set2bits(L->marked, FIXEDBIT, SFIXEDBIT);

        for ( unsigned int i = 0; i < NUM_TAGS; i++ )
        {
            L->mt[i] = NULL;
        }

        g->mainthread = L;

        return true;
    }

    bool OnPluginAssign( global_State *dstG, const global_State *srcG, globalStatePluginOffset_t pluginOffset, unsigned int pluginId )
    {
        // We cannot clone a main thread.
        return false;
    }

    void OnPluginDestruct( global_State *g, globalStatePluginOffset_t pluginOffset, unsigned int pluginId )
    {
        lua_State *L = globalStateFactory_t::RESOLVE_STRUCT <lua_State> ( g, pluginOffset );

        L->~lua_State();

        g->mainthread = NULL;
    }
};

static mainThreadLuaStatePluginInterface *_mainThreadPluginInterface = NULL;
static globalStatePluginOffset_t _mainThreadPluginOffset = 0;

LUAI_FUNC lua_State *lua_newstate (lua_Alloc f, void *ud)
{
    // Attempt to allocate a block of memory for bootstrapping.
    GeneralMemoryAllocator memAlloc( ud, f );

    const size_t allocDataMemSize = sizeof( GlobalStateAllocPluginData );

    void *allocDataMem = memAlloc.Allocate( allocDataMemSize );

    if ( !allocDataMem )
        return NULL;

    // Create the allocator meta-data.
    // Create the memory allocator for bootstrapping.
    // Create the factory that can produce and destroy global states.
    GlobalStateAllocPluginData *allocData = new (allocDataMem) GlobalStateAllocPluginData( f, ud );

    if ( !allocData )
        return NULL;

    // Initialize our library if it has not been initialized already.
    if ( _libraryReferenceCount++ == 0 )
    {
        // register global modules here!
        luaM_init();
        luaC_moduleinit();
        luaO_init();
        luaS_init();
        luaQ_init();
        luaH_init();
        luaF_init();
        luaT_moduleinit();
        luaJ_init();
        luaV_init();
        luaapi_init();

        // todo: add more

        // Initialize the plugin to hold the main lua thread state inside the global_State
        // as well as meta data.
        {
            mainThreadLuaStatePluginInterface *pluginInterface = new mainThreadLuaStatePluginInterface;

            if ( pluginInterface )
            {
                _mainThreadPluginOffset = globalStateFactory.RegisterPlugin(
                    sizeof( lua_State ), GLOBAL_STATE_PLUGIN_MAIN_STATE,
                    pluginInterface
                );
            }

            _mainThreadPluginInterface = pluginInterface;
        }
    }

    // Attempt to construct our global state.
    global_State *g = allocData->constructor->Construct();

    if ( g )
    {
        // Store allocation data into the struct.
        // This is done so we can destroy all plugins and deallocate ourselves.
        g->allocData = allocData;

        // Attempt to get the main thread from the global state.
        lua_State *L = globalStateFactory_t::RESOLVE_STRUCT <lua_State> ( g, _mainThreadPluginOffset );

        if ( L )
        {
            // Properly initialize the main state global object.
            g->frealloc = f;
            g->ud = ud;
            g->strt.size = 0;
            g->strt.nuse = 0;
            g->strt.hash = NULL;
            g->totalbytes = (lu_mem)globalStateFactory.GetClassSize();
            // Initialize the garbage collector
            luaC_init( g );
            preinit_state( L, g );
            luaE_newenvironment( L );   // create the first environment; threads will inherit it
            sethvalue(L, &g->l_registry, luaH_new(L, 0, 2));  /* registry */

            // To properly initialize the main state, we must call post-state initializators.
            luaS_stateinit( L );

            for ( unsigned int i = 0; i < LUA_NUM_EVENTS; i++ )
            {
                g->events[i] = NULL;
            }

            std::string errMsg;

            // Initialize GCthread
            luaC_initthread( g );

            bool luaInternalInitSuccess = ( luaD_rawrunprotected( L, f_luaopen, NULL, errMsg, NULL ) == 0 );

            if ( luaInternalInitSuccess )
            {
                luai_userstateopen(L);
                return L;
            }

            /* memory allocation error: free partial state */
            close_state( L );
        }

        dealloc_state( g );
    }

    // No success creating a Lua state.
    return NULL;
}

lu_mem lua_State::GetTypeSize( global_State *g ) const
{
    return sizeof( *this );
}

LUA_API void lua_close (lua_State *L)
{
    // The pointer passed to us must be a valid main state pointer.
    global_State *globalState = G(L);

    {
        L = G(L)->mainthread;  /* only the main thread can be closed */

        lua_lock(L);
        luaF_close(L, L->stack);  /* close all upvalues for this thread */

        // Collect all pending memory
        luaC_shutdown( G(L) );

        luai_userstateclose(L);

        close_state(L);
    }

    dealloc_state( globalState );

    // Clear global modules if necessary.
    if ( --_libraryReferenceCount == 0 )
    {
        // Shutdown our local plugins.
        if ( mainThreadLuaStatePluginInterface *pluginInterface = _mainThreadPluginInterface )
        {
            delete pluginInterface;

            _mainThreadPluginInterface = NULL;
        }

        // Shutdown global modules.
        luaapi_shutdown();
        luaV_shutdown();
        luaJ_shutdown();
        luaT_moduleshutdown();
        luaF_shutdown();
        luaH_shutdown();
        luaQ_shutdown();
        luaS_shutdown();
        luaO_shutdown();
        luaC_moduleshutdown();
        luaM_shutdown();

        // todo: add more.
    }

    // Finito!
}

// MTA specific
LUA_API lua_State* lua_getmainstate (lua_State *L) {
  return G(L)->mainthread;
}

