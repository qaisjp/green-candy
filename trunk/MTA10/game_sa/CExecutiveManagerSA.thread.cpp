/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        game_sa/CExecutiveManagerSA.thread.cpp
*  PURPOSE:     Thread abstraction layer for MTA
*  DEVELOPERS:  Martin Turski <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#include "StdInc.h"

#ifdef _WIN32

struct nativeThreadPlugin
{
    struct nativeThreadPluginInterface *manager;
    CExecThreadSA *self;
    HANDLE hThread;
    mutable CRITICAL_SECTION threadLock;
    volatile eThreadStatus status;
    Fiber *terminationReturn;   // if not NULL, the thread yields to this state when it successfully terminated.

    RwListEntry <nativeThreadPlugin> node;
};

// Safe critical sections.
namespace LockSafety
{
    // When something horribly dangerous to the runtime happens (hacks, etc),
    // the attackers must contend this lock. It is used when actions happen that
    // must not be interrupted at any cost!
    // Ideally, we would control all of the possible OS locks through wrappers,
    // so they would be secure.
    static CRITICAL_SECTION lock_section;

    static void Init( void )
    {
        InitializeCriticalSection( &lock_section );
    }

    static void Shutdown( void )
    {
        DeleteCriticalSection( &lock_section );
    }

    static void Lock( void )
    {
        EnterCriticalSection( &lock_section );
    }

    static void Unlock( void )
    {
        LeaveCriticalSection( &lock_section );
    }
        
    static AINLINE void EnterLockSafely( CRITICAL_SECTION& theSection )
    {
        // Lock all things that do critical activity!
        Lock();

        // Now enter.
        {
            EnterCriticalSection( &theSection );
        }

        // Leave the very important space.
        Unlock();
    }

    static AINLINE void LeaveLockSafely( CRITICAL_SECTION& theSection )
    {
        // Make sure we are safe in doing stuff.
        Lock();

        // Now leave.
        {
            LeaveCriticalSection( &theSection );
        }

        // Leave the very important space, again.
        Unlock();
    }
}

// Struct for exception safety.
struct nativeLock
{
    CRITICAL_SECTION& critical_section;

    AINLINE nativeLock( CRITICAL_SECTION& theSection ) : critical_section( theSection )
    {
        LockSafety::EnterLockSafely( critical_section );
    }

    AINLINE ~nativeLock( void )
    {
        LockSafety::LeaveLockSafely( critical_section );
    }
};

struct nativeThreadPluginInterface : public ExecutiveManager::threadPluginContainer_t::pluginInterface
{
    RwList <nativeThreadPlugin> runningThreads;
    mutable CRITICAL_SECTION runningThreadListLock;

    volatile DWORD tlsCurrentThreadStruct;

    bool isTerminating;

    inline nativeThreadPluginInterface( void )
    {
        LIST_CLEAR( runningThreads.root );

        tlsCurrentThreadStruct = TlsAlloc();

        isTerminating = false;

        InitializeCriticalSection( &runningThreadListLock );
    }

    inline ~nativeThreadPluginInterface( void )
    {
        DeleteCriticalSection( &runningThreadListLock );

        if ( tlsCurrentThreadStruct != TLS_OUT_OF_INDEXES )
        {
            TlsFree( tlsCurrentThreadStruct );
        }
    }

    inline void TlsSetCurrentThreadInfo( nativeThreadPlugin *info )
    {
        DWORD currentThreadSlot = tlsCurrentThreadStruct;

        if ( currentThreadSlot != TLS_OUT_OF_INDEXES )
        {
            TlsSetValue( currentThreadSlot, info );
        }
    }

    inline nativeThreadPlugin* TlsGetCurrentThreadInfo( void )
    {
        nativeThreadPlugin *plugin = NULL;

        {
            DWORD currentThreadSlot = tlsCurrentThreadStruct;

            if ( currentThreadSlot != TLS_OUT_OF_INDEXES )
            {
                plugin = (nativeThreadPlugin*)TlsGetValue( currentThreadSlot );
            }
        }

        return plugin;
    }

    static DWORD WINAPI _ThreadProc( LPVOID param )
    {
        // Get the thread plugin information.
        nativeThreadPlugin *info = (nativeThreadPlugin*)param;

        CExecThreadSA *threadInfo = info->self;

        // Put our executing thread information into our TLS value.
        info->manager->TlsSetCurrentThreadInfo( info );

        // Make sure we intercept termination requests!
        try
        {
            // Enter the routine.
            threadInfo->entryPoint( threadInfo, threadInfo->userdata );

            // Terminate our thread.
            threadInfo->Terminate();
        }
        catch( ... )
        {
            // We are terminated.
            info->status = THREAD_TERMINATED;

            // If we have a termination info, we want to leave into it.
            if ( Fiber *termContext = info->terminationReturn )
            {
                ExecutiveFiber::leave( termContext );

                // We do not reach this point anymore.
            }
        }

        return ERROR_SUCCESS;
    }

    static void __stdcall _ThreadTerminationProto( void )
    {
        CExecThreadSA *thisThread;

        __asm
        {
            mov thisThread,ebx
        }

        throw threadTerminationException( thisThread );
    }

    typedef struct _NT_TIB
    {
        PVOID ExceptionList;
        PVOID StackBase;
        PVOID StackLimit;
    } NT_TIB;

    void RtlTerminateThread( nativeThreadPlugin *threadInfo )
    {
        CExecThreadSA *theThread = threadInfo->self;

        assert( theThread->isRemoteThread == false );

        // If we are not the current thread, we must do certain precautions.
        bool isCurrentThread = theThread->IsCurrent();

        if ( !isCurrentThread )
        {
            // Make sure we are outside of critical code.
            LockSafety::Lock();

            // Suspend our thread.
            // We must not use locking functions for that.
            DWORD suspendCount = SuspendThread( threadInfo->hThread );

            // Now that we are "suspended", cannot continue even if the lock
            // gets lifted.
            LockSafety::Unlock();
        }

        // Set our status to terminating.
        threadInfo->status = THREAD_TERMINATING;

        // Depends on whether we are the current thread or not.
        if ( isCurrentThread )
        {
            // Just do the termination.
            throw threadTerminationException( theThread );
        }
        else
        {
            // NOTE: this is a fucking insecure hack that changes the executing thread of a thread runtime.
            // It is written with the hope that no code makes assumptions about a certain thread having to execute
            // termination logic. If this feature will be required, I will still have certain work-arounds!

            // It works, so do not fuck around with it.

            // Create a fiber that we have to walk down the thread context with.
            Fiber terminationFiber;
            Fiber returnFiber;

            // Get the complete WIN32 x86 context.
            CONTEXT currentContext;
            currentContext.ContextFlags = ( CONTEXT_INTEGER | CONTEXT_CONTROL | CONTEXT_SEGMENTS );

            BOOL threadContextGet = GetThreadContext( threadInfo->hThread, &currentContext );

            if ( threadContextGet == TRUE )
            {
                // Put our context into the fiber.
                terminationFiber.ebp = currentContext.Ebp;
                terminationFiber.edi = currentContext.Edi;
                terminationFiber.esi = currentContext.Esi;
                terminationFiber.esp = currentContext.Esp;

                // Grab exception information of that thread, too!
                // This especially is quite an ugly hack.
                {
                    LDT_ENTRY fsSegmentEntry;

                    BOOL selectorGet = GetThreadSelectorEntry( threadInfo->hThread, currentContext.SegFs, &fsSegmentEntry );
                    
                    assert( selectorGet == TRUE );

                    // Construct the real pointer to NT_TIB.
                    DWORD fsPointer =
                        ( fsSegmentEntry.BaseLow ) |
                        ( fsSegmentEntry.HighWord.Bytes.BaseMid << 16 ) |
                        ( fsSegmentEntry.HighWord.Bytes.BaseHi << 24 );

                    const NT_TIB *threadInfoBlock = (NT_TIB*)fsPointer;

                    terminationFiber.stack_base = (unsigned int*)threadInfoBlock->StackBase;
                    terminationFiber.stack_limit = (unsigned int*)threadInfoBlock->StackLimit;
                    terminationFiber.except_info = (void*)threadInfoBlock->ExceptionList;
                }

                // Modify it so we point to the termination routine.
                terminationFiber.ebx = (unsigned int)theThread;
                terminationFiber.eip = (unsigned int)_ThreadTerminationProto;
                    
                // Make the thread now that it should jump back here at termination.
                threadInfo->terminationReturn = &returnFiber;

                // Jump!!!
                ExecutiveFiber::eswitch( &returnFiber, &terminationFiber );

                // If we return here, the thread must be terminated.
                assert( threadInfo->status == THREAD_TERMINATED );
            }

            // We can now terminate the handle.
            TerminateThread( threadInfo->hThread, ERROR_SUCCESS );
        }

        // If we were the current thread, we cannot reach this point.
        assert( isCurrentThread == false );
    }

    bool OnPluginConstruct( CExecThreadSA *thread, void *mem, unsigned int id )
    {
        nativeThreadPlugin *info = (nativeThreadPlugin*)mem;

        // If we are not a remote thread...
        HANDLE hOurThread = NULL;

        if ( !thread->isRemoteThread )
        {
            // ... create a local thread!
            DWORD threadIdOut;

            HANDLE hThread = ::CreateThread( NULL, (SIZE_T)thread->stackSize, _ThreadProc, info, CREATE_SUSPENDED, &threadIdOut );

            if ( hThread == NULL )
                return false;

            hOurThread = hThread;
        }
        info->hThread = hOurThread;

        // NOTE: we initialize remote threads in the GetCurrentThread routine!

        // Give ourselves a self reference pointer.
        info->self = thread;
        info->manager = this;

        // This field is used by the runtime dispatcher to execute a "controlled return"
        // from different threads.
        info->terminationReturn = NULL;

        // We assume the thread is (always) running if its a remote thread.
        // Otherwise we know that it starts suspended.
        info->status = ( !thread->isRemoteThread ) ? THREAD_SUSPENDED : THREAD_RUNNING;

        {
            nativeLock lock( this->runningThreadListLock );

            LIST_INSERT( runningThreads.root, info->node );
        }

        // Set up synchronization object.
        InitializeCriticalSection( &info->threadLock );
        return true;
    }

    void OnPluginDestroy( CExecThreadSA *thread, void *mem, unsigned int id )
    {
        nativeThreadPlugin *info = (nativeThreadPlugin*)mem;

        // Only terminate if we have not acquired a remote thread.
        if ( !thread->isRemoteThread )
        {
            // Terminate the thread.
            thread->Terminate();
        }

        // Delete synchronization object.
        DeleteCriticalSection( &info->threadLock );

        {
            nativeLock lock( this->runningThreadListLock );

            LIST_REMOVE( runningThreads.root );
        }

        CloseHandle( info->hThread );
    }
};

#endif

// todo: add other OSes too when it becomes necessary.

CExecThreadSA::CExecThreadSA( CExecutiveManagerSA *manager, bool isRemoteThread )
{
    this->manager = manager;
    this->isRemoteThread = isRemoteThread;

    LIST_INSERT( manager->threads.root, managerNode );
}

CExecThreadSA::~CExecThreadSA( void )
{
    LIST_REMOVE( managerNode );
}

eThreadStatus CExecThreadSA::GetStatus( void ) const
{
    eThreadStatus status = THREAD_TERMINATED;

#ifdef _WIN32
    nativeThreadPlugin *info = (nativeThreadPlugin*)pluginSentry.GetPluginByID( THREAD_PLUGIN_NATIVE );

    if ( info )
    {
        nativeLock lock( info->threadLock );

        status = info->status;
    }
#endif

    return status;
}

// WARNING: terminating threads in general is very naughty and causes shit to go haywire!
// No matter what thread state, this function guarrantees to terminate a thread cleanly according to
// C++ stack unwinding logic!
bool CExecThreadSA::Terminate( void )
{
    bool returnVal = false;

#ifdef _WIN32
    nativeThreadPlugin *info = (nativeThreadPlugin*)pluginSentry.GetPluginByID( THREAD_PLUGIN_NATIVE );

    if ( info && info->status != THREAD_TERMINATED )
    {
        nativeLock lock( info->threadLock );

        if ( info->status != THREAD_TERMINATED )
        {
            // Termination depends on what kind of thread we face.
            bool terminationSuccessful = false;

            if ( this->isRemoteThread )
            {
                // Remote threads must be killed just like that.
                BOOL success = TerminateThread( info->hThread, ERROR_SUCCESS );

                if ( success == TRUE )
                {
                    terminationSuccessful = true;
                }
            }
            else
            {
                // User-mode threads have to be cleanly terminated.
                // This means going down the exception stack.
                nativeThreadPluginInterface *nativeInterface = (nativeThreadPluginInterface*)manager->threadNativePlugin;

                nativeInterface->RtlTerminateThread( info );

                // We may not actually get here!
            }

            if ( terminationSuccessful )
            {
                // Put the status as terminated.
                info->status = THREAD_TERMINATED;

                // Return true.
                returnVal = true;
            }
        }
    }
#endif

    return returnVal;
}

bool CExecThreadSA::Suspend( void )
{
    bool returnVal = false;

#ifdef _WIN32
    nativeThreadPlugin *info = (nativeThreadPlugin*)pluginSentry.GetPluginByID( THREAD_PLUGIN_NATIVE );

    // We cannot suspend a remote thread.
    if ( !isRemoteThread )
    {
        if ( info && info->status == THREAD_RUNNING )
        {
            nativeLock lock( info->threadLock );

            if ( info->status == THREAD_RUNNING )
            {
                BOOL success = SuspendThread( info->hThread );

                if ( success == TRUE )
                {
                    info->status = THREAD_SUSPENDED;

                    returnVal = true;
                }
            }
        }
    }
#endif

    return returnVal;
}

bool CExecThreadSA::Resume( void )
{
    bool returnVal = false;

#ifdef _WIN32
    nativeThreadPlugin *info = (nativeThreadPlugin*)pluginSentry.GetPluginByID( THREAD_PLUGIN_NATIVE );

    // We cannot resume a remote thread.
    if ( !isRemoteThread )
    {
        if ( info && info->status == THREAD_SUSPENDED )
        {
            nativeLock lock( info->threadLock );

            if ( info->status == THREAD_SUSPENDED )
            {
                BOOL success = ResumeThread( info->hThread );

                if ( success == TRUE )
                {
                    info->status = THREAD_RUNNING;

                    returnVal = true;
                }
            }
        }
    }
#endif

    return returnVal;
}

bool CExecThreadSA::IsCurrent( void )
{
    return ( this->manager->GetCurrentThread() == this );
}

void CExecThreadSA::Lock( void )
{
#ifdef _WIN32
    nativeThreadPlugin *info = (nativeThreadPlugin*)pluginSentry.GetPluginByID( THREAD_PLUGIN_NATIVE );

    if ( info )
    {
        LockSafety::EnterLockSafely( info->threadLock );
    }
#else
    assert( 0 );
#endif
}

void CExecThreadSA::Unlock( void )
{
#ifdef _WIN32
    nativeThreadPlugin *info = (nativeThreadPlugin*)pluginSentry.GetPluginByID( THREAD_PLUGIN_NATIVE );

    if ( info )
    {
        LockSafety::LeaveLockSafely( info->threadLock );
    }
#else
    assert( 0 );
#endif
}

CExecThreadSA* CExecutiveManagerSA::CreateThread( CExecThreadSA::threadEntryPoint_t entryPoint, void *userdata, size_t stackSize )
{
    // No point in creating threads if we have no native implementation.
    if ( !threadNativePlugin )
        return NULL;

    CExecThreadSA *threadInfo = new CExecThreadSA( this, false );

    if ( !threadInfo )
    {
        return NULL;
    }

    // Pass our parameters to our thread info.
    threadInfo->entryPoint = entryPoint;
    threadInfo->userdata = userdata;
    threadInfo->stackSize = stackSize;

    // Make sure we synchronize access to plugin containers!
    // This only has to happen when the API has to be thread-safe.
    {
        nativeLock lock( threadPluginsLock );

        // Initialize the thread structure here.
        threadPlugins.OnPluginObjectCreate( threadInfo );
    }

    // Check that the native threading plugin got successfully initialized.
    if ( threadInfo->pluginSentry.GetPluginByID( THREAD_PLUGIN_NATIVE ) == NULL )
    {
        // There is no point in allowing function-less threads, so terminate the struct and return nothing.
        CloseThread( threadInfo );
        return NULL;
    }

    // Return the thread.
    return threadInfo;
}

void CExecutiveManagerSA::TerminateThread( CExecThreadSA *thread )
{
    thread->Terminate();
}

CExecThreadSA* CExecutiveManagerSA::GetCurrentThread( void )
{
    CExecThreadSA *currentThread = NULL;

#ifdef _WIN32
    {
        // Get our native interface (if available).
        nativeThreadPluginInterface *nativeInterface = (nativeThreadPluginInterface*)this->threadNativePlugin;
        
        if ( nativeInterface )
        {
            HANDLE hRunningThread = ::GetCurrentThread();

            // If we have an accelerated TLS slot, try to get the handle from it.
            if ( nativeThreadPlugin *tlsInfo = nativeInterface->TlsGetCurrentThreadInfo() )
            {
                currentThread = tlsInfo->self;
            }
            else
            {
                nativeLock lock( nativeInterface->runningThreadListLock );

                // Else we have to go the slow way by checking every running thread information in existance.
                LIST_FOREACH_BEGIN( nativeThreadPlugin, nativeInterface->runningThreads.root, node )
                    if ( item->hThread == hRunningThread )
                    {
                        currentThread = item->self;
                        break;
                    }
                LIST_FOREACH_END
            }

            // If we have not found a thread handle representing this native thread, we should create one.
            if ( currentThread == NULL && nativeInterface->isTerminating == false )
            {
                CExecThreadSA *newThreadInfo = new CExecThreadSA( this, true );

                if ( newThreadInfo )
                {
                    {
                        // Make sure to lock plugin activity!
                        nativeLock lock( threadPluginsLock );

                        // Initialize plugin objects for this thread object.
                        threadPlugins.OnPluginObjectCreate( newThreadInfo );
                    }

                    bool successPluginCreation = false;

                    // Our plugin must have been successfully intialized to continue.
                    if ( nativeThreadPlugin *plugInfo = (nativeThreadPlugin*)newThreadInfo->pluginSentry.GetPluginByID( THREAD_PLUGIN_NATIVE ) )
                    {
                        // Open another thread handle and put it into our native plugin.
                        HANDLE newHandle = NULL;

                        BOOL successClone = DuplicateHandle(
                            GetCurrentProcess(), hRunningThread,
                            GetCurrentProcess(), &newHandle,
                            0, FALSE, DUPLICATE_SAME_ACCESS
                        );

                        if ( successClone == TRUE )
                        {
                            // Put the new handle into our plugin structure.
                            plugInfo->hThread = newHandle;

                            // Set our plugin information into our Tls slot (if available).
                            nativeInterface->TlsSetCurrentThreadInfo( plugInfo );

                            // Return it.
                            currentThread = newThreadInfo;

                            successPluginCreation = true;
                        }
                    }
                    
                    if ( successPluginCreation == false )
                    {
                        {
                            nativeLock lock( threadPluginsLock );

                            threadPlugins.OnPluginObjectDestroy( newThreadInfo );
                        }

                        delete newThreadInfo;
                    }
                }
            }
        }
    }
#endif

    return currentThread;
}

void CExecutiveManagerSA::CloseThread( CExecThreadSA *thread )
{
    // Only allow this from the current thread if we are a remote thread.
    if ( GetCurrentThread() == thread )
    {
        if ( !thread->isRemoteThread )
            return;
    }

    {
#ifdef _WIN32
        nativeLock lock( threadPluginsLock );
#endif

        threadPlugins.OnPluginObjectDestroy( thread );
    }

    delete thread;
}

void CExecutiveManagerSA::InitThreads( void )
{
    LIST_CLEAR( threads.root );

#ifdef _WIN32
    LockSafety::Init();

    InitializeCriticalSection( &threadPluginsLock );

    nativeThreadPluginInterface *nativeInterface = new nativeThreadPluginInterface();

    if ( nativeInterface )
    {
        threadPlugins.RegisterPlugin( sizeof( nativeThreadPlugin ), THREAD_PLUGIN_NATIVE, nativeInterface );
    }

    this->threadNativePlugin = nativeInterface;
#else
    this->threadNativePlugin = NULL;
#endif
}

void CExecutiveManagerSA::ShutdownThreads( void )
{
#ifdef _WIN32
    {
        // Notify ourselves that we are terminating.
        nativeThreadPluginInterface *nativeInterface = (nativeThreadPluginInterface*)this->threadNativePlugin;

        nativeInterface->isTerminating = true;
    }
#endif

    // Shutdown all currently yet active threads.
    while ( !LIST_EMPTY( threads.root ) )
    {
        CExecThreadSA *thread = LIST_GETITEM( CExecThreadSA, threads.root.next, managerNode );

        CloseThread( thread );
    }

    if ( this->threadNativePlugin )
    {
        delete this->threadNativePlugin;

        this->threadNativePlugin = NULL;
    }

#ifdef _WIN32
    DeleteCriticalSection( &threadPluginsLock );

    LockSafety::Shutdown();
#endif
}