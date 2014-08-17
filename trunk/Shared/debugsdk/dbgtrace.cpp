/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        debugsdk/dbgtrace.cpp
*  PURPOSE:     Win32 exception tracing tool for error isolation
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#include <StdInc.h>

// Include vendor libraries.
#include "dbgtrace.vendor.hwbrk.hxx"

namespace DbgTrace
{
    static const std::string GetValidDebugSymbolPath( void )
    {
        // TODO: allow the debuggee to set this path using a FileSearch dialog.
        return "E:\\Users\\The_GTA\\Desktop\\mta_green\\symbols";
    }

    struct Win32DebugManager
    {
        volatile bool isInitialized;
        volatile bool isInsideDebugPhase;
        CRITICAL_SECTION debugLock;
        volatile HANDLE contextProcess;

        AINLINE Win32DebugManager( void )
        {
            isInitialized = false;
            isInsideDebugPhase = false;
            contextProcess = NULL;

            InitializeCriticalSection( &debugLock );
        }

        AINLINE ~Win32DebugManager( void )
        {
            Shutdown();

            DeleteCriticalSection( &debugLock );
        }

        AINLINE bool AttemptInitialize( void )
        {
            if ( isInitialized )
                return true;

            bool successful = false;

            contextProcess = GetCurrentProcess();

            BOOL initializeSuccessful =
                SymInitialize( contextProcess, GetValidDebugSymbolPath().c_str(), true );

            successful = ( initializeSuccessful == TRUE );

            isInitialized = successful;

            return successful;
        }

        AINLINE void Shutdown( void )
        {
            assert( isInsideDebugPhase == false );

            if ( !isInitialized )
                return;

            SymCleanup( contextProcess );

            isInitialized = false;
        }

        AINLINE bool Begin( void )
        {
            // DbgHelp.dll is single-threaded, hence we must get a lock.
            EnterCriticalSection( &debugLock );

            assert( isInsideDebugPhase == false );

            bool isInitialized = AttemptInitialize();

            if ( !isInitialized )
            {
                LeaveCriticalSection( &debugLock );
            }
            else
            {
                // If we successfully initialized the debug library, set it up properly.
                SymSetOptions(
                    SYMOPT_DEFERRED_LOADS |
                    SYMOPT_LOAD_LINES |
                    SYMOPT_UNDNAME
                );

                isInsideDebugPhase = true;
            }

            return isInitialized;
        }

        AINLINE void End( void )
        {
            assert( isInsideDebugPhase == true );

            LeaveCriticalSection( &debugLock );

            isInsideDebugPhase = false;
        }

        struct InternalSymbolInfo : public SYMBOL_INFO
        {
            TCHAR Name_extended[255];
        };

        AINLINE void GetDebugInfoForAddress( void *addrPtr, CallStackEntry& csInfo )
        {
            const DWORD addrAsOffset = (DWORD)addrPtr;

            {
                // Get Information about the stack frame contents.
                InternalSymbolInfo addrSymbolInfo;
                addrSymbolInfo.SizeOfStruct = sizeof( InternalSymbolInfo );
                addrSymbolInfo.MaxNameLen = NUMELMS( addrSymbolInfo.Name_extended ) + NUMELMS( addrSymbolInfo.Name );

                DWORD64 displacementPtr;
                
                BOOL symbolFetchResult = SymFromAddr( contextProcess, addrAsOffset, &displacementPtr, &addrSymbolInfo );

                if ( symbolFetchResult == TRUE )
                {
                    csInfo.symbolName = addrSymbolInfo.Name;
                }
            }

            {
                // Get Information about the line and the file of the context.
                IMAGEHLP_LINE symbolLineInfo;
                symbolLineInfo.SizeOfStruct = sizeof( IMAGEHLP_LINE );

                DWORD displacementPtr;

                BOOL symbolLineFetchResult = SymGetLineFromAddr( contextProcess, addrAsOffset, &displacementPtr, &symbolLineInfo );

                if ( symbolLineFetchResult == TRUE )
                {
                    csInfo.symbolFile = symbolLineInfo.FileName;
                    csInfo.symbolFileLine = symbolLineInfo.LineNumber;
                }
            }
        }
    };

    static Win32DebugManager debugMan;

    struct Win32EnvSnapshot : public IEnvSnapshot
    {
        static BOOL CALLBACK MemoryReadFunction( HANDLE hProcess, DWORD lpBaseAddress, PVOID lpBuffer, DWORD nSize, LPDWORD lpNumberOfBytesRead )
        {
            assert( hProcess == NULL );

            __try
            {
                memmove( lpBuffer, (void*)lpBaseAddress, nSize );
            }
            __except( EXCEPTION_EXECUTE_HANDLER )
            {
                *lpNumberOfBytesRead = 0;
                return FALSE;
            }

            *lpNumberOfBytesRead = nSize;
            return TRUE;
        }

        static PVOID CALLBACK FunctionTableAccess( HANDLE hProcess, DWORD addrBase )
        {
            assert( hProcess == NULL );

            return SymFunctionTableAccess( GetCurrentProcess(), addrBase );
        }

        static DWORD CALLBACK GetModuleBaseFunction( HANDLE hProcess, DWORD Address )
        {
            assert( hProcess == NULL );

            return SymGetModuleBase( GetCurrentProcess(), Address );
        }

        AINLINE Win32EnvSnapshot( const CONTEXT& theContext )
        {
            // Save the context.
            runtimeContext = theContext;

            // Construct the call-stack using debug information.
            bool isDebugLibraryInitialized = debugMan.Begin();

            // Build the call stack.
            CONTEXT walkContext = theContext;
            
            // Walk through the call frames.
            STACKFRAME outputFrame;
            memset( &outputFrame, 0, sizeof( outputFrame ) );

            DWORD machineType;

#ifdef _M_IX86
            machineType = IMAGE_FILE_MACHINE_I386;

            outputFrame.AddrPC.Offset =     runtimeContext.Eip;
            outputFrame.AddrPC.Mode =       AddrModeFlat;
            outputFrame.AddrFrame.Offset =  runtimeContext.Ebp;
            outputFrame.AddrFrame.Mode =    AddrModeFlat;
            outputFrame.AddrStack.Offset =  runtimeContext.Esp;
            outputFrame.AddrStack.Mode =    AddrModeFlat;
#endif

            while (
                StackWalk( machineType, NULL, NULL, &outputFrame, &walkContext,
                    MemoryReadFunction,
                    FunctionTableAccess,
                    GetModuleBaseFunction,
                    NULL ) )
            {
                // Get the offset as pointer.
                void *offsetPtr = (void*)outputFrame.AddrPC.Offset;

                // Construct a call stack entry.
                CallStackEntry contextRuntimeInfo( offsetPtr );

                if ( isDebugLibraryInitialized )
                {
                    debugMan.GetDebugInfoForAddress( offsetPtr, contextRuntimeInfo );
                }

                callstack.push_back( contextRuntimeInfo );
            }

            // If we have been using the symbol runtime, free its resources.
            if ( isDebugLibraryInitialized )
            {
                debugMan.End();
            }
        }

        AINLINE ~Win32EnvSnapshot( void )
        {
            return;
        }

        Win32EnvSnapshot* Clone( void )
        {
            return new Win32EnvSnapshot( runtimeContext );
        }

        void RestoreTo( void )
        {
            SetThreadContext( GetCurrentThread(), &runtimeContext );
        }

        callStack_t GetCallStack( void )
        {
            return callstack;
        }

        std::string ToString( void )
        {
            std::string outputBuffer = "Call Frames:\n";

            int n = 1;

            // List all call-stack frames into the output buffer.
            for ( callStack_t::const_iterator iter = callstack.begin(); iter != callstack.end(); iter++ )
            {
                const CallStackEntry& csInfo = *iter;

                outputBuffer += NumberUtil::to_string( n++ );
                outputBuffer += "-- ";
                {
                    const std::string symbolName = csInfo.GetSymbolName();

                    if ( symbolName.size() == 0 )
                    {
                        outputBuffer += "[0x";
                        outputBuffer += NumberUtil::to_string_hex( (unsigned int)csInfo.codePtr );
                        outputBuffer += "]";
                    }
                    else
                    {
                        outputBuffer += csInfo.GetSymbolName();
                        outputBuffer += " at 0x";
                        outputBuffer += NumberUtil::to_string_hex( (unsigned int)csInfo.codePtr );
                    }
                }
                outputBuffer += " (";
                outputBuffer += csInfo.GetFileName();
                outputBuffer += ":";
                outputBuffer += NumberUtil::to_string( csInfo.GetLineNumber() );
                outputBuffer += ")\n";
            }

            return outputBuffer;
        }

        CONTEXT runtimeContext;
        callStack_t callstack;
    };

    static bool CaptureRuntimeContext( CONTEXT& outputContext )
    {
        bool successful = false;

        CONTEXT runtimeContext;
        memset( &runtimeContext, 0, sizeof( runtimeContext ) );

        RtlCaptureContext( &runtimeContext );

        outputContext = runtimeContext;

        successful = true;

        return successful;
    }

    IEnvSnapshot* CreateEnvironmentSnapshot( void )
    {
        CONTEXT theContext;
#if 0
        memset( &theContext, 0, sizeof( theContext ) );

        theContext.ContextFlags = CONTEXT_CONTROL;
#endif

        bool gotContext = CaptureRuntimeContext( theContext );

        IEnvSnapshot *returnObj = NULL;

        if ( gotContext )
        {
            returnObj = new Win32EnvSnapshot( theContext );
        }

        return returnObj;
    }

    IEnvSnapshot* CreateEnvironmentSnapshotFromContext( const CONTEXT *const runtimeContext )
    {
        try
        {
            return new Win32EnvSnapshot( *runtimeContext );
        }
        catch( ... )
        {
            return NULL;
        }
    }

    // Thanks to NirSoft!
    typedef enum _EXCEPTION_DISPOSITION
    {
        ExceptionContinueExecution = 0,
        ExceptionContinueSearch = 1,
        ExceptionNestedException = 2,
        ExceptionCollidedUnwind = 3
    } EXCEPTION_DISPOSITION;

    typedef EXCEPTION_DISPOSITION (__cdecl*EXCEPTION_HANDLER)(
            EXCEPTION_RECORD*   ExceptionRecord,
            void*               EstablisherFrame,
            CONTEXT*            ContextRecord,
            void*               DispatcherContext );

    typedef struct _EXCEPTION_REGISTRATION_RECORD
    {
        _EXCEPTION_REGISTRATION_RECORD *Next;
        EXCEPTION_HANDLER Handler;
    } EXCEPTION_REGISTRATION_RECORD, *PEXCEPTION_REGISTRATION_RECORD;

    typedef struct _NT_TIB
    {
        PEXCEPTION_REGISTRATION_RECORD ExceptionList;
        PVOID StackBase;
        PVOID StackLimit;
        PVOID SubSystemTib;
        union
        {
            PVOID FiberData;
            ULONG Version;
        };
        PVOID ArbitraryUserPointer;
        PNT_TIB Self;

        static PEXCEPTION_REGISTRATION_RECORD GetInvalidExceptionRecord( void )
        {
            return (PEXCEPTION_REGISTRATION_RECORD)-1;
        }

        inline void PushExceptionRegistration( PEXCEPTION_REGISTRATION_RECORD recordEntry )
        {
            // Make sure we execute the exception record we had before registering this one after
            // the one that is being added now.
            recordEntry->Next = ExceptionList;

            // Set the given record as the current exception handler.
            ExceptionList = recordEntry;
        }

        inline PEXCEPTION_REGISTRATION_RECORD GetExceptionRegistrationTop( void )
        {
            return ExceptionList;
        }

        inline void PopExceptionRegistration( void )
        {
            // We just remove the top most handler, if there is one.
            if ( ExceptionList == GetInvalidExceptionRecord() )
                return;

            ExceptionList = ExceptionList->Next;
        }

    } NT_TIB, *PNT_TIB;

    static NT_TIB& GetThreadEnvironmentBlock( void )
    {
        NT_TIB *blockPtr = NULL;

        __asm
        {
            mov eax,dword ptr fs:[0x18]
            mov blockPtr,eax
        }

        return *blockPtr;
    }

#define EH_NONCONTINUABLE   0x01
#define EH_UNWINDING        0x02
#define EH_EXIT_UNWIND      0x04
#define EH_STACK_INVALID    0x08
#define EH_NESTED_CALL      0x10

    static DbgTraceStackSpace *stackSpace = NULL;

    struct Win32ExceptionRegistrationRecord : public _EXCEPTION_REGISTRATION_RECORD
    {
        typedef std::list <IExceptionHandler*> exceptions_t;

        exceptions_t registeredExceptions;

        static EXCEPTION_DISPOSITION __cdecl StaticHandlerRoutine(
            EXCEPTION_RECORD*   ExceptionRecord,
            void*               EstablisherFrame,
            CONTEXT*            ContextRecord,
            void*               DispatcherContext )
        {
            Win32ExceptionRegistrationRecord *thisPtr = (Win32ExceptionRegistrationRecord*)EstablisherFrame;

            bool hasBeenHandled = false;

            if ( !( ExceptionRecord->ExceptionFlags & ( EH_UNWINDING | EH_EXIT_UNWIND ) ) )
            {
                IEnvSnapshot *envSnapShot = CreateEnvironmentSnapshotFromContext( ContextRecord );

                for ( exceptions_t::const_iterator iter = thisPtr->registeredExceptions.begin(); iter != thisPtr->registeredExceptions.end(); iter++ )
                {
                    IExceptionHandler *const userHandler = *iter;

                    bool userHandled = userHandler->OnException( ExceptionRecord->ExceptionCode, envSnapShot );

                    if ( userHandled )
                    {
                        hasBeenHandled = true;
                    }
                }
            }

            return ( hasBeenHandled ) ? ExceptionContinueExecution : ExceptionContinueSearch;
        }

        inline Win32ExceptionRegistrationRecord( void )
        {
            this->Handler = StaticHandlerRoutine;
            this->Next = NT_TIB::GetInvalidExceptionRecord();
        }

        inline ~Win32ExceptionRegistrationRecord( void )
        {
            return;
        }

        inline void PushRecord( void )
        {
            GetThreadEnvironmentBlock().PushExceptionRegistration( this );
        }

        inline void PopRecord( void )
        {
            NT_TIB& threadBlock = GetThreadEnvironmentBlock();

            assert( threadBlock.GetExceptionRegistrationTop() == this );

            threadBlock.PopExceptionRegistration();
        }

        inline bool IsUserHandlerRegistered( IExceptionHandler *theHandler )
        {
            return std::find( registeredExceptions.begin(), registeredExceptions.end(), theHandler ) != registeredExceptions.end();
        }

        inline void RegisterUserHandler( IExceptionHandler *theHandler )
        {
            if ( !IsUserHandlerRegistered( theHandler ) )
            {
                registeredExceptions.push_back( theHandler );
            }
        }

        inline void UnregisterUserHandler( IExceptionHandler *theHandler )
        {
            exceptions_t::const_iterator iter = std::find( registeredExceptions.begin(), registeredExceptions.end(), theHandler );

            if ( iter != registeredExceptions.end() )
            {
                registeredExceptions.erase( iter );
            }
        }

        // Construction is very special for this container: it must be allocated on stack space on Win32.
        // Otherwise SEH will fault for us.
        inline void* operator new( size_t memSize )
        {
            assert( memSize < sizeof( DbgTraceStackSpace ) );

            return stackSpace;
        }

        inline void operator delete( void *memPtr )
        {
            return;
        }
    };

    static Win32ExceptionRegistrationRecord *runtimeRecord;

    void __declspec(nothrow) InitializeExceptionSystem( void )
    {
        runtimeRecord = new Win32ExceptionRegistrationRecord();

        runtimeRecord->PushRecord();
    }

    void __declspec(nothrow) ShutdownExceptionSystem( void )
    {
        runtimeRecord->PopRecord();

        delete runtimeRecord;
    }

    void RegisterExceptionHandler( IExceptionHandler *handler )
    {
        runtimeRecord->RegisterUserHandler( handler );
    }

    void UnregisterExceptionHandler( IExceptionHandler *handler )
    {
        runtimeRecord->UnregisterUserHandler( handler );
    }
};

void DbgTrace_Init( DbgTraceStackSpace& stackSpace )
{
    // Set a private pointer that will keep pointing to that allocated stack space.
    DbgTrace::stackSpace = &stackSpace;

    DbgTrace::InitializeExceptionSystem();
}

void DbgTrace_InitializeGlobalDebug( void )
{
    // Put special debug code here.
    //SetHardwareBreakpoint( GetCurrentThread(), HWBRK_TYPE_WRITE, HWBRK_SIZE_4, (void*)0x001548E0 );
}

void DbgTrace_Shutdown( void )
{
    DbgTrace::ShutdownExceptionSystem();
}