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

    RwListEntry <nativeThreadPlugin> node;
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

        // Enter the routine.
        threadInfo->entryPoint( threadInfo, threadInfo->userdata );

        // Terminate our thread.
        threadInfo->Terminate();

        return ERROR_SUCCESS;
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

        // We assume the thread is (always) running if its a remote thread.
        // Otherwise we know that it starts suspended.
        info->status = ( !thread->isRemoteThread ) ? THREAD_SUSPENDED : THREAD_RUNNING;

        EnterCriticalSection( &this->runningThreadListLock );
        {
            LIST_INSERT( runningThreads.root, info->node );
        }
        LeaveCriticalSection( &this->runningThreadListLock );

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

        EnterCriticalSection( &this->runningThreadListLock );
        {
            LIST_REMOVE( runningThreads.root );
        }
        LeaveCriticalSection( &this->runningThreadListLock );

        CloseHandle( info->hThread );
    }
};

#endif

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
        EnterCriticalSection( &info->threadLock );

        status = info->status;

        LeaveCriticalSection( &info->threadLock );
    }
#endif

    return status;
}

bool CExecThreadSA::Terminate( void )
{
    bool returnVal = false;

#ifdef _WIN32
    nativeThreadPlugin *info = (nativeThreadPlugin*)pluginSentry.GetPluginByID( THREAD_PLUGIN_NATIVE );

    if ( info && info->status != THREAD_TERMINATED )
    {
        EnterCriticalSection( &info->threadLock );

        if ( info->status != THREAD_TERMINATED )
        {
            BOOL success = TerminateThread( info->hThread, ERROR_SUCCESS );

            if ( success == TRUE )
            {
                info->status = THREAD_TERMINATED;

                returnVal = true;
            }
        }

        LeaveCriticalSection( &info->threadLock );
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
            EnterCriticalSection( &info->threadLock );

            if ( info->status == THREAD_RUNNING )
            {
                BOOL success = SuspendThread( info->hThread );

                if ( success == TRUE )
                {
                    info->status = THREAD_SUSPENDED;

                    returnVal = true;
                }
            }

            LeaveCriticalSection( &info->threadLock );
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
            EnterCriticalSection( &info->threadLock );

            if ( info->status == THREAD_SUSPENDED )
            {
                BOOL success = ResumeThread( info->hThread );

                if ( success == TRUE )
                {
                    info->status = THREAD_RUNNING;

                    returnVal = true;
                }
            }

            LeaveCriticalSection( &info->threadLock );
        }
    }
#endif

    return returnVal;
}

CExecThreadSA* CExecutiveManagerSA::CreateThread( CExecThreadSA::threadEntryPoint_t entryPoint, void *userdata, size_t stackSize )
{
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
    EnterCriticalSection( &threadPluginsLock );

    // Initialize the thread structure here.
    threadPlugins.OnPluginObjectCreate( threadInfo );

    LeaveCriticalSection( &threadPluginsLock );

    // Check that the native threading plugin got successfully initialized.
    if ( threadInfo->pluginSentry.GetPluginByID( THREAD_PLUGIN_NATIVE ) == NULL )
    {
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
                EnterCriticalSection( &nativeInterface->runningThreadListLock );

                // Else we have to go the slow way by checking every running thread information in existance.
                LIST_FOREACH_BEGIN( nativeThreadPlugin, nativeInterface->runningThreads.root, node )
                    if ( item->hThread == hRunningThread )
                    {
                        currentThread = item->self;
                        break;
                    }
                LIST_FOREACH_END

                LeaveCriticalSection( &nativeInterface->runningThreadListLock );
            }

            // If we have not found a thread handle representing this native thread, we should create one.
            if ( currentThread == NULL && nativeInterface->isTerminating == false )
            {
                CExecThreadSA *newThreadInfo = new CExecThreadSA( this, true );

                if ( newThreadInfo )
                {
                    EnterCriticalSection( &threadPluginsLock );

                    // Initialize plugin objects for this thread object.
                    threadPlugins.OnPluginObjectCreate( newThreadInfo );

                    LeaveCriticalSection( &threadPluginsLock );

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
                        EnterCriticalSection( &threadPluginsLock );

                        threadPlugins.OnPluginObjectDestroy( newThreadInfo );

                        LeaveCriticalSection( &threadPluginsLock );

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
    // Only allow this from a remote thread if we are the current thread.
    if ( GetCurrentThread() == thread )
    {
        if ( !thread->isRemoteThread )
            return;
    }

    EnterCriticalSection( &threadPluginsLock );

    threadPlugins.OnPluginObjectDestroy( thread );

    LeaveCriticalSection( &threadPluginsLock );

    delete thread;
}

void CExecutiveManagerSA::InitThreads( void )
{
    LIST_CLEAR( threads.root );

    InitializeCriticalSection( &threadPluginsLock );

#ifdef _WIN32
    nativeThreadPluginInterface *nativeInterface = new nativeThreadPluginInterface();

    if ( nativeInterface )
    {
        threadPlugins.RegisterPlugin( sizeof( nativeThreadPlugin ), THREAD_PLUGIN_NATIVE, nativeInterface );
    }

    this->threadNativePlugin = nativeInterface;
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

    DeleteCriticalSection( &threadPluginsLock );
}