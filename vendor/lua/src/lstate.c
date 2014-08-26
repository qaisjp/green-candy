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

#include "lpluginutil.hxx"

// Main state of the whole Lua VM.
struct lua_MainState : public lua_State
{
    inline lua_MainState( void )
    {
        return;
    }

    inline lua_MainState( void *construction_params )
    {
        return;
    }

    inline void Initialize( global_State *g )
    {
        // Initialize the main thread.
        this->l_G = g;

        g->currentwhite = bit2mask(WHITE0BIT, FIXEDBIT);    // set a special GC flag so we do not collect the main thread

        luaC_register( this, this, LUA_TTHREAD );

        set2bits(this->marked, FIXEDBIT, SFIXEDBIT);

        for ( unsigned int i = 0; i < NUM_TAGS; i++ )
        {
            this->mt[i] = NULL;
        }

        g->mainthread = this;
    }

    inline void Shutdown( global_State *g )
    {
        g->mainthread = NULL;
    }
};

// Type information holding plugin.
static namespacePluginOffset_t _globalStateMetaInfoPluginOffset = namespaceFactory_t::INVALID_PLUGIN_OFFSET;

struct globalStateMainStateFactoryMeta
{
    typedef globalStateFactory_t factoryType;

    lua_config *managerStruct;

    // Type information.
    LuaTypeSystem::typeInfoBase *luaStateTypeInfo;
    LuaTypeSystem::typeInfoBase *luaThreadTypeInfo;

    unsigned int refCount;

    // Required field.
    globalStatePluginOffset_t endingPointPluginOffset;

    inline void Initialize( lua_config *cfgStruct )
    {
        LuaTypeSystem& typeSys = cfgStruct->typeSys;

        luaStateTypeInfo = typeSys.RegisterStructType <lua_MainState> ( "main_state" );
        luaThreadTypeInfo = typeSys.RegisterStructType <lua_Thread> ( "coroutine" );

        // Set inheritance information.
        typeSys.SetTypeInfoInheritingClass(
            luaStateTypeInfo,
            cfgStruct->grayobjTypeInfo
        );
        typeSys.SetTypeInfoInheritingClass(
            luaThreadTypeInfo,
            cfgStruct->grayobjTypeInfo
        );

        refCount = 0;
        endingPointPluginOffset = globalStateFactory_t::INVALID_PLUGIN_OFFSET;

        managerStruct = cfgStruct;
    }

    inline void Shutdown( lua_config *cfgStruct )
    {
        if ( this->luaThreadTypeInfo != NULL )
        {
            cfgStruct->typeSys.DeleteType( this->luaThreadTypeInfo );
        }

        if ( this->luaStateTypeInfo != NULL )
        {
            cfgStruct->typeSys.DeleteType( this->luaStateTypeInfo );
        }
    }

    inline void Reference( void )
    {
        if ( this->refCount++ == 0 )
        {
            // Get the dynamic size of the main lua state.
            size_t mainStateSize = this->managerStruct->typeSys.GetTypeStructSize( this->luaStateTypeInfo, NULL );

            // Register the plugin to the global_State!
            this->endingPointPluginOffset = this->managerStruct->globalStateFactory.RegisterDependantStructPlugin <lua_MainState> (
                GLOBAL_STATE_PLUGIN_MAIN_STATE, mainStateSize
            );
        }
    }

    inline void Dereference( void )
    {
        if ( --this->refCount == 0 )
        {
            // Delete the plugin again if present.
            if ( globalStateFactory_t::IsOffsetValid( this->endingPointPluginOffset ) )
            {
                this->managerStruct->globalStateFactory.UnregisterPlugin( this->endingPointPluginOffset );
            }
        }
    }
};

static PluginConnectingBridge <lua_MainState, globalStateMainStateFactoryMeta, namespaceFactory_t> mainLuaStateConnectingBridge( namespaceFactory );


static void stack_init (lua_State *L1, lua_State *L) {
  /* initialize CallInfo array */
  L1->base_ci = luaM_newvector <CallInfo> (L, BASIC_CI_SIZE);
  L1->ci = L1->base_ci;
  L1->size_ci = BASIC_CI_SIZE;
  L1->end_ci = L1->base_ci + L1->size_ci - 1;
  /* initialize stack array */
  L1->stack = luaM_newvector <TValue> (L, BASIC_STACK_SIZE + EXTRA_STACK);
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
  luaM_freearray(L, L1->base_ci, L1->size_ci);
  luaM_freearray(L, L1->stack, L1->stacksize);
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
    L->openupval.Clear();
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

#ifdef _WIN32

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

#endif //_WIN32


lua_Thread::lua_Thread( void *construction_params )
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
    global_State *g = G(L);

    // Get the type information.
    globalStateMainStateFactoryMeta *typeInfo = mainLuaStateConnectingBridge.GetMetaStruct( g->config );

    // No point in continuing if we do not have type information.
    if ( !typeInfo )
        return NULL;

    lua_Thread *L1 = lua_new <lua_Thread> ( L, typeInfo->luaThreadTypeInfo );

    if ( L1 )
    {
        luaC_link(L, L1, LUA_TTHREAD);
        LIST_INSERT( g->threads.root, L1->threadNode );  /* we need to be aware of all threads */
        preinit_state(L1, g);
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
        lua_assert(L->openupval.IsEmpty() == true);

		// Threads clean their environments after themselves
		luaX_closefiber( L, env );

		L->fiber = NULL;
	}
}

bool lua_Thread::AllocateRuntime()
{
    if ( fiber )
        return true;

    FiberProcedure fiberProc = NULL;

#ifdef _WIN32
    fiberProc = luaE_guardedThreadEntryPoint;
#else
    fiberProc = (FiberProcedure)luaE_threadEntryPoint;
#endif //_WIN32

    if ( fiberProc != NULL )
    {
        fiber = luaX_newfiber( this, 0, fiberProc );
        
        if ( fiber != NULL )
        {
            // initiate it
            resume();
            return true;
        }
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

// Entry point to the Lua library (old style).
#include "lstate.lowlevel.hxx"

void luaE_libraryinit( void )
{
#if 0
    _globalStateMetaInfoPluginOffset =
        namespaceFactory.RegisterDependantStructPlugin <mainStateTypeInfo> ( namespaceFactory_t::ANONYMOUS_PLUGIN_ID );

    mainLuaStateConnectingBridge.RegisterPluginStruct( namespaceFactory );
#endif
}

void luaE_libraryshutdown( void )
{
    return;
}

static void close_state (lua_State *L)
{
    global_State *g = G(L);

    luaF_close(L, L->stack);  /* close all upvalues for this thread */
    luaC_freeall(L);  /* collect all objects */
    lua_assert(g->strt.nuse == 0);
    luaM_freearray(L, G(L)->strt.hash, G(L)->strt.size);

    // Call post-state destructors.
    luaS_stateshutdown( L );

    freestack(L, L);

    // Close down type runtimes.
    luaQ_runtimeshutdown( g );
    luaF_runtimeshutdown( g );
}

static void dealloc_state( global_State *globalState )
{
    // Get the Lua VM configuration.
    lua_config *config = globalState->config;

    // Delete the plugin object and destroy the allocator info.
    GlobalStateAllocPluginData *allocData = (GlobalStateAllocPluginData*)config->allocData;

    // If this assert fails, then GC is flawed because it did not clean up everything.
    // A flawed GC can have many reasons, even can be caused by the application!
    lua_assert(globalState->totalbytes == config->globalStateFactory.GetClassSize());

    allocData->constructor->Destroy( globalState );

    // Dereference our meta struct.
    globalStateMainStateFactoryMeta *metaInfo = mainLuaStateConnectingBridge.GetMetaStruct( config );

    metaInfo->Dereference();

    // Get allocator information to destroy the last bit of data.
    GeneralMemoryAllocator memAlloc( allocData->_memAlloc );

    // Destroy configuration (if unique).
    if ( globalState->hasUniqueConfig )
    {
        // This should delete all remaining memory.
        lua_freeconfig( config );
    }
}

lua_State *lua_newstate (lua_Alloc f, void *ud)
{
    // Create a Lua configuration entity.
    lua_config *cfg = lua_newconfig( f, ud );

    if ( !cfg )
        return NULL;

    // Get the allocation data.
    GlobalStateAllocPluginData *allocData = (GlobalStateAllocPluginData*)cfg->allocData;

    // Get our environment inside of the configuration struct.
    globalStateMainStateFactoryMeta *metaInfo = mainLuaStateConnectingBridge.GetMetaStruct( cfg );

    if ( metaInfo )
    {
        // Reference it.
        metaInfo->Reference();

        // Construct it specially.
        struct globalStateConstructor
        {
            lua_config *config;

            inline global_State* Construct( void *mem )
            {
                global_State *g = new (mem) global_State;

                // Set the configuration.
                g->config = config;

                return g;
            }
        };

        // Attempt to construct our global state.
        globalStateConstructor constructor;
        constructor.config = cfg;

        global_State *g = allocData->constructor->ConstructTemplate( constructor );

        if ( g )
        {
            // Attempt to get the main thread from the global state.
            lua_MainState *L = mainLuaStateConnectingBridge.GetPluginStructFromMetaStruct( metaInfo, g );

            if ( L )
            {
                // Store whether we have a unique configuration.
                // If so, we will have to destroy it when we close this state.
                g->hasUniqueConfig = true;
                g->config = cfg;

                // Store local configuration values.
                // These values must not change across the runtime.
                g->isMultithreaded = cfg->isMultithreaded;

                // Properly initialize the main state global object.
                g->strt.size = 0;
                g->strt.nuse = 0;
                g->strt.hash = NULL;
                g->totalbytes = (lu_mem)cfg->globalStateFactory.GetClassSize();

                // Initialize type runtimes.
                luaF_runtimeinit( g );
                luaQ_runtimeinit( g );

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
    }

    // No success creating a Lua state.
    return NULL;
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

    // Finito!
}

// MTA specific
LUA_API lua_State* lua_getmainstate (lua_State *L) {
  return G(L)->mainthread;
}

