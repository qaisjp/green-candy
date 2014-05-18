/*****************************************************************************
*
*  PROJECT:     Lua Interpreter
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        main.cpp
*  PURPOSE:     Global header
*  DEVELOPERS:  The_GTA <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#include "StdInc.h"
#include <signal.h>

#include <SharedUtil.hpp>

using namespace std;

lua_State *userLuaState;

static lua_State *state;
static CEvents *events;
static CLuaManager *manager;
CResourceManager *resMan;

static CResource *benchResource = NULL;

CFileTranslator *modFileRoot;

__forceinline void lua_exec( LuaManager::context *context, const std::string& cmd )
{
    int top = lua_gettop( state );

    if ( luaL_loadbuffer( state, cmd.c_str(), cmd.size(), "@cmdline" ) != 0 )
    {
        const char *err = lua_tostring( state, -1 );
        lua_pop( state, 1 );

        throw lua_exception( state, LUA_ERRSYNTAX, err );
    }

    bool success = ( lua_pcall( state, 0, LUA_MULTRET, 0 ) == 0 );

    if ( !success )
    {
        const char *err = lua_tostring( state, -1 );
        lua_pop( state, 1 );
        
        throw lua_exception( state, LUA_ERRRUN, err );
    }

    int now = lua_gettop( state );

    if ( now == top )
        return;

    cout << "> ";

    for ( int n = top; n != now; n++ )
    {
        const char *strRep;
        int t = lua_type( state, n + 1 );
        const char *type = lua_typename( state, t );

        switch( t )
        {
        case LUA_TBOOLEAN:
            strRep = lua_toboolean( state, n + 1 ) ? "true" : "false";
            break;
        default:
            strRep = lua_tostring( state, n + 1 );
            break;
        }

        if ( !strRep )
        {
            cout << type;

            if ( strcmp( type, "class" ) == 0 )
            {
                lua_pushtype( state, n + 1 );
                
                const char *typeString = lua_tostring( state, -1 );

                cout << " [";

                if ( typeString )
                    cout << typeString;
                else
                    cout << "unknown";

                lua_pop( state, 1 );

                cout << "]";
            }
        }
        else
        {
            cout << strRep;
            cout << " [";
            cout << type;

            if ( strcmp( type, "class" ) == 0 )
            {
                lua_pushtype( state, n + 1 );
                
                const char *typeString = lua_tostring( state, -1 );

                cout << ", ";

                if ( typeString )
                    cout << typeString;
                else
                    cout << "unknown";

                lua_pop( state, 1 );
            }

            cout << "]";
        }

        if ( n + 1 != now )
            cout << ", ";
    }

    cout << "\n";

    lua_settop( state, top );
}

void handleError( const lua_exception& e )
{
    switch( e.status() )
    {
    case LUA_ERRRUN:
        cout << "error\n";
        break;
    case LUA_ERRSYNTAX:
        cout << "syntax_error\n";
        break;
    }

    lua_Debug debug;
    e.getDebug( debug );

    if ( debug.currentline != -1 )
        cout << debug.short_src << ' ';

    cout << e.what();
    cout << "\n";
}

static LuaManager::context *execContext = NULL;

static HANDLE threadHandle = NULL;
static HANDLE processCmdEvent = NULL;
static HANDLE consoleInputHandle = NULL;

__forceinline void shutdown_interpreter( void )
{
    TerminateThread( threadHandle, 0 );

    if ( benchResource )
    {
        benchResource->Destroy();
        benchResource->DecrementMethodStack();
    }

    if ( execContext )
        delete execContext;

    resMan->Delete();
    manager->Shutdown();
    delete manager;
    delete events;

    luagl_shutdownDrivers();

    delete fileSystem;
}

static bool do_runtime = true;

void signal_handler( int sig )
{
    do_runtime = false;
}

BOOL WINAPI ControlHandler( DWORD type )
{
    do_runtime = false;

    return 1;
}

static bool cmdLinePushed;
static std::string cmdLine;

static DWORD __stdcall HandleConsoleInput( LPVOID param )
{
    while ( getline( cin, cmdLine ) )
    {
        cmdLinePushed = true;

        WaitForSingleObject( processCmdEvent, INFINITE );
        ResetEvent( processCmdEvent );
    }
    return 0;
}

extern "C"
{
extern int mainCRTStartup( void );
}

static void __cdecl _DbgHeap_MemAllocWatch( void *memPtr, size_t memSize )
{
    if ( (DWORD)memPtr == 0x00154380 || (DWORD)memPtr == 0x001543B8 )
    {
        __asm int 3
    }
}

extern "C" int APIENTRY _MainInit( void )
{
    DbgHeap_Init();

    DbgTraceStackSpace stackSpace;  // reserved memory; must be always allocated.

    DbgTrace_Init( stackSpace );

    // Set up memory debugging routines.
    DbgHeap_SetMemoryAllocationWatch( _DbgHeap_MemAllocWatch );

    int ret = mainCRTStartup();

    DbgTrace_Shutdown();
    DbgHeap_Shutdown();
    return ret;
}

static LUA_DECLARE( quit )
{
    do_runtime = false;
    return 0;
}

bool lint_loadscript( lua_State *L, const char *script, const char *path )
{
    if ( luaL_loadbuffer( L, script, strlen( script ), ( std::string( "@" ) + path ).c_str() ) != 0 )
    {
        cout << "failed to load library " << path << "\n";
        cout << lua_tostring( L, -1 ) << "\n";
        lua_pop( L, 1 );
        return false;
    }

    if ( lua_pcall( L, 0, 0, 0 ) != 0 )
    {
        cout << "failed to run library " << path << "\n";
        cout << lua_tostring( L, -1 ) << "\n";

        lua_pop( L, 1 );
        return false;
    }

    return true;
}

struct testExceptionHandler : public DbgTrace::IExceptionHandler
{
    testExceptionHandler( void )
    {
        return;
    }

    ~testExceptionHandler( void )
    {
        return;
    }

    bool OnException( unsigned int error_code, DbgTrace::IEnvSnapshot *runtimeSnapShot )
    {
        if ( !IsDebuggerPresent() )
        {
            printf( "An irrecoverable exception has occured!\n%s", runtimeSnapShot->ToString().c_str() );
            
            getchar();
        }
        return false;
    }
};

int _main( int argc, char *argv[] )
{
    cout << "MTA:Lua Interpreter v1.0, by (c)Martin Turski (visit mtasa.com)\nCompiled on " __DATE__ "\n\n";

    events = new CEvents;
    manager = new CLuaManager( *events );
    resMan = new CResourceManager( *manager );
    userLuaState = state = manager->GetVirtualMachine();

    // Initialize components here
    luagl_initDrivers();

    srand( (unsigned int)time( NULL ) );

    signal( SIGTERM, signal_handler );
    signal( SIGBREAK, signal_handler );

    new CFileSystem();

    SetConsoleCtrlHandler( ControlHandler, true );

    ResourceManager::resFileRoot = fileRoot;
    LuaManager::m_resMan = resMan;
    LuaFunctionDefs::SetResourceManager( *resMan );
    LuaFunctionDefs::SetDebugging( manager->GetDebug() );

    LuaManager::context *useContext = NULL;

    if ( fileRoot->Exists( "/luabench/" ) )
    {
        // Include everything from /luabench/
        cout << "starting luaBench files...\n";

        modFileRoot = fileSystem->CreateTranslator( "/luabench/" );

        if ( !modFileRoot )
            modFileRoot = fileRoot;

        benchResource = resMan->Load( "luabench" );
        userLuaState = state = benchResource->GetVM().GetVirtualMachine();

        cout << "\nlogged in as 'luabench'\n";
    }
    else
    {
        modFileRoot = fileRoot;

        benchResource = resMan->Create( "/", "guest" );
        userLuaState = state = benchResource->GetVM().GetVirtualMachine();

        cout << "logged in as 'guest'\n";
    }

    // Reference our main resource.
    benchResource->IncrementMethodStack();

    // Create the execution context
    execContext = new LuaManager::context( manager->AcquireContext( benchResource->GetVM() ) );
    useContext = execContext;

    lua_register( userLuaState, "quit", quit );

    // Run "init.lua" in root if present
    {
        std::vector <char> buf;

        if ( fileRoot->ReadToBuffer( "init.lua", buf ) )
        {
            buf.push_back( 0 );

            if ( lint_loadscript( userLuaState, &buf[0], "init.lua" ) )
                std::cout << "executed init.lua\n";
        }
    }

    cout << '\n';

    // Get the console handlers
    consoleInputHandle = GetStdHandle( STD_INPUT_HANDLE );

    SetConsoleMode( consoleInputHandle, ENABLE_LINE_INPUT | ENABLE_ECHO_INPUT | ENABLE_PROCESSED_OUTPUT );

    // Create the input thread
    DWORD threadId;

    processCmdEvent = CreateEvent( NULL, true, false, "Process Command" );

    threadHandle = CreateThread( NULL, 0, HandleConsoleInput, NULL, 0, &threadId );

    // Reset stack offset
    int resetTop = lua_gettop( state );

    try
    {
        while ( do_runtime )
        {
            if ( cmdLinePushed )
            {
                std::string script = cmdLine;

                try
                {
                    std::string retCmd = "return ";
                    retCmd += script;

                    lua_exec( useContext, retCmd );
                }
                catch( lua_exception& _e )
                {
                    if ( _e.status() != LUA_ERRSYNTAX )
                    {
                        handleError( _e );
                    }
                    else
                    {
                        try
                        {
                            lua_exec( useContext, script );
                        }
                        catch( lua_exception& e )
                        {
                            handleError( e );
                        }
                    }
                }

                cmdLinePushed = false;
                SetEvent( processCmdEvent );
            }

            bool peeked = false;

            try
            {
                // Put manager logic here.
                luawin32_pulse();
                luagl_pulseDrivers( state );

                MSG msg;

                if ( !do_runtime )
                    break;

                // Proceed with the garbage collector
                lua_gc( g_L, LUA_GCSTEP, 2 );

                while ( PeekMessage( &msg, NULL, 0, 0, PM_REMOVE ) )
                {
                    peeked = true;

                    TranslateMessage( &msg );
                    DispatchMessage( &msg );
                }
            }
            catch( lua_exception& e )
            {
                cout << "runtime_";

                handleError( e );

                lua_settop( state, resetTop );
            }

            Sleep( peeked ? 0 : 1 );
        }
    }
    catch( ... )
    {
        cout << "terminated";
    }

    shutdown_interpreter();
    return EXIT_SUCCESS;
}

int main( int argc, char *argv[] )
{
    testExceptionHandler myHandler;

    DbgTrace::RegisterExceptionHandler( &myHandler );

    _main( argc, argv );

    DbgTrace::UnregisterExceptionHandler( &myHandler );
}