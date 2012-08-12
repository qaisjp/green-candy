/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        Shared/logic/lua/LuaManager.cpp
*  PURPOSE:     Management of Lua hyperstructures
*       Every lua hyperstructure is a table in a global lua environment
*       upkept by the LuaManager entity. LuaManager is supposed to secure
*       the connections between hyperstructures and provide the basic
*       framework for their runtime.
*  DEVELOPERS:  Ed Lyons <>
*               Chris McArthur <>
*               Jax <>
*               Cecill Etheredge <>
*               Christian Myhre Lundheim <>
*               Oliver Brown <>
*               Kevin Whiteside <>
*               lil_Toady <>
*               The_GTA <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#include <StdInc.h>

#define HOOK_INSTRUCTION_COUNT 1000000
#define HOOK_MAXIMUM_TIME 5000

ResourceManager* LuaManager::m_resMan = NULL;

static LuaManager* lua_readmanager( lua_State *L )
{
    return lua_readuserdata <LuaManager, LUA_REGISTRYINDEX, 1> ( L );
}

static int lua_cocreatethread( lua_State *L )
{
    // Inherit previous settings
    lua_State *thread = lua_tothread( L, 1 );
    lua_rawgeti( L, LUA_STORAGEINDEX, 1 );
    lua_rawgeti( L, LUA_STORAGEINDEX, 2 );
    lua_rawgeti( L, LUA_STORAGEINDEX, 3 );

    lua_xmove( L, thread, 3 );

    lua_rawseti( L, LUA_STORAGEINDEX, 3 );
    lua_rawseti( L, LUA_STORAGEINDEX, 2 );
    lua_rawseti( L, LUA_STORAGEINDEX, 1 );
    return 0;
}

static int lua_pushstackthread( lua_State *lua )
{
    lua_readmanager( lua )->PushThread( lua_tothread( lua, -1 ) );
    return 0;
}

static int lua_popstackthread( lua_State *lua )
{
    lua_readmanager( lua )->PopThread();
    return 0;
}

static void InstructionCountHook( lua_State *lua, lua_Debug *debug )
{
    lua_readmanager( lua )->InstructionCountHook( lua, debug );
}

static int mainaccess_global( lua_State *lua )
{
    return lua_readmanager( lua )->AccessGlobalTable();
}

static const luaL_Reg interface_access[] =
{
    { "_G", mainaccess_global },
    { NULL, NULL }
};

static int luaglobal_index( lua_State *lua )
{
    return lua_readmanager( lua )->AccessGlobal();
}

static int luaglobal_newindex( lua_State *lua )
{
    //TODO: Register process!
}

static int luamain_constructor( lua_State *lua )
{
    lua_pushlstring( lua, "main", 4 );
    lua_setfield( lua, LUA_ENVIRONINDEX, "__type" );
    return 0;
}

LuaManager::LuaManager( Events& events, ScriptDebugging& debug ) :
    m_events( events ),
    m_debug( debug )
{
    // Setup the virtual machine
    m_lua = luaL_newstate();

    // Setup callbacks
    lua_setevent( m_lua, LUA_EVENT_THREAD_CO_CREATE, lua_cocreatethread );
    lua_setevent( m_lua, LUA_EVENT_THREAD_CONTEXT_PUSH, lua_pushstackthread );
    lua_setevent( m_lua, LUA_EVENT_THREAD_CONTEXT_POP, lua_popstackthread );

    // Cache the lua manager in the VM
    lua_pushlightuserdata( m_lua, this );
    luaL_ref( m_lua, LUA_REGISTRYINDEX );   // first in registry!

    // Setup the global
    lua_newtable( m_lua );
    lua_pushvalue( m_lua, 1 );
    luaL_ref( m_lua, LUA_REGISTRYINDEX );   // second in registry!

    // Fast access global interface
    lua_newtable( m_lua );
    luaL_openlib( m_lua, NULL, interface_access, 0 );
    luaL_ref( m_lua, LUA_REGISTRYINDEX );   // third in registry!

    // Initialize back hashmaps
    LuaCFunctions::InitializeHashMaps();
}

void LuaManager::Shutdown()
{
    if ( !m_lua )
        return;

    // Destroy lua environment
    lua_close( m_lua );

    m_lua = NULL;
}

LuaManager::~LuaManager()
{
    LuaCFunctions::RemoveAllFunctions();

    // TODO: analyze that every virtual machine really gets destroyed

    Shutdown();
}

void LuaManager::PushStatus( const LuaMain& main )
{
    if ( !m_envStack.empty() )
        m_envStack.back().Finalize();

    m_envStack.push_back( env_status( *m_lua, main ) );
}

const LuaMain* LuaManager::GetStatus( int *line, std::string *src, std::string *proto_name )
{
    if ( m_envStack.empty() )
        return NULL;

    return &m_envStack.back().Get( line, src, proto_name );
}

unsigned long LuaManager::GetRuntime()
{
    if ( m_envStack.empty() )
        return 0;

    return m_envStack.back().GetRuntime();
}

const LuaMain* LuaManager::PopStatus( int *line, std::string *src, std::string *proto_name )
{
    if ( m_envStack.empty() )
        return NULL;

    const LuaMain& context = m_envStack.back().Get( line, src, proto_name );

    m_envStack.pop_back();

    if ( !m_envStack.empty() )
        m_envStack.back().Resume();

    return &context;
}

void LuaManager::InstructionCountHook( lua_State *L, lua_Debug *ar )
{
    if ( GetRuntime() < HOOK_MAXIMUM_TIME )
        return;

    const LuaMain *env = GetStatus();

    // Print it in the console
    Logger::ErrorPrintf( "Infinite/too long execution (%s)", env->GetName().c_str() );

    // Error out
    throw lua_exception( L, LUA_ERRRUN, "Aborting; infinite running script" );
}

void LuaManager::CallStack( int args )
{
    lua_State *L = GetThread();

    lua_call( L, args, LUA_MULTRET );
}

void LuaManager::CallStackVoid( int args )
{
    lua_State *L = GetThread();

    lua_call( L, args, 0 );
}

void LuaManager::CallStackResult( int argc, LuaArguments& args )
{
    lua_State *L = GetThread();
    int top = lua_gettop( L );

    CallStack( argc );

    args.ReadArguments( L, top );

    lua_settop( L, top - argc - 1 );
}

static inline bool lua_protectedcall( LuaManager& man, lua_State *L, int argc, int retc )
{
    if ( lua_type( L, -argc - 1 ) != LUA_TFUNCTION )
        throw lua_exception( L, LUA_ERRSYNTAX, "expected function at LuaManager::CallStack" );

    lua_Debug debug;

    // We could theoretically protect it ourselves
    switch( int ret = lua_pcallex( L, argc, retc, 0, &debug ) )
    {
    case LUA_ERRRUN:
    case LUA_ERRMEM:
        if ( debug.currentline == -1 )
            man.GetDebug().LogError( "%s", lua_tostring( L, -1 ) );
        else
        {
            if ( debug.source[0] == '@' )
                man.GetDebug().LogError( "%s:%u: %s", debug.source + 1, debug.currentline, lua_tostring( L, -1 ) );
            else
                man.GetDebug().LogError( "%s:%u: %s", debug.short_src, debug.currentline, lua_tostring( L, -1 ) );
        }

        // Clean the stack
        lua_settop( L, -2 );
        return false;
    }

    return true;
}

bool LuaManager::PCallStack( int argc )
{
    return lua_protectedcall( *this, GetThread(), argc, LUA_MULTRET );
}

bool LuaManager::PCallStackVoid( int argc )
{
    return lua_protectedcall( *this, GetThread(), argc, 0 );
}

bool LuaManager::PCallStackResult( int argc, LuaArguments& args )
{
    lua_State *L = GetThread();
    int top = lua_gettop( L );

    if ( !lua_protectedcall( *this, L, argc, LUA_MULTRET ) )
        return false;

    args.ReadArguments( L, top );
    return true;
}

static int luamain_index( lua_State *lua )
{
    lua_gettable( lua, 1 );

    if ( !lua_isnil( lua, -1 ) )
        return 1;

    lua_pop( lua, 1 );
    return lua_readmanager( lua )->AccessGlobal();
}

// ARGS: table, key
int LuaManager::AccessGlobal()
{
    assert( 0 );

    lua_pushvalue( m_lua, -2 );
    lua_gettable( m_lua, -2 );

    if ( lua_isnil( m_lua, -1 ) )
    {
        lua_pop( m_lua, 1 );
        lua_gettable( m_lua, LUA_GLOBALSINDEX );
        return 1;
    }

    int top = lua_gettop( m_lua );

    try
    {
        CallStack( 1 );

        return lua_gettop( m_lua ) - top;
    }
    catch( lua_exception& e )
    {
        m_debug.LogError( e.what() );

        lua_settop( m_lua, top );
    }

    return 0;
}

int LuaManager::AccessGlobalTable()
{
    // Default access to luaGlobal is allowed
    lua_pushvalue( m_lua, LUA_GLOBALSINDEX );
    return 1;
}

#undef LUA_REGISTER
#define LUA_REGISTER( main, x ) main->RegisterFunction( #x, x )

static int LoadCFunctions( LuaMain *L )
{
    using namespace LuaFunctionDefs;

    LUA_REGISTER( L, fileOpen );
    LUA_REGISTER( L, fileExists );
    LUA_REGISTER( L, fileCreate );
    LUA_REGISTER( L, fileIsEOF );
    LUA_REGISTER( L, fileGetPos );
    LUA_REGISTER( L, fileSetPos );
    LUA_REGISTER( L, fileGetSize );
    LUA_REGISTER( L, fileRead );
    LUA_REGISTER( L, fileWrite );
    LUA_REGISTER( L, fileFlush );
    LUA_REGISTER( L, fileClose );
    LUA_REGISTER( L, fileDelete );
    LUA_REGISTER( L, fileRename );
    LUA_REGISTER( L, fileCopy );

#ifndef _KILLFRENZY
    // XML Functions
    LUA_REGISTER( L, xmlFindChild );
    LUA_REGISTER( L, xmlNodeGetChildren );
    LUA_REGISTER( L, xmlNodeGetValue );
    LUA_REGISTER( L, xmlNodeSetValue );
    LUA_REGISTER( L, xmlNodeGetAttributes );
    LUA_REGISTER( L, xmlNodeGetAttribute );
    LUA_REGISTER( L, xmlNodeSetAttribute );
    LUA_REGISTER( L, xmlNodeGetParent );
    LUA_REGISTER( L, xmlLoadFile );
    LUA_REGISTER( L, xmlCreateFile );
    LUA_REGISTER( L, xmlUnloadFile );
    LUA_REGISTER( L, xmlSaveFile );
    LUA_REGISTER( L, xmlCreateChild );
    LUA_REGISTER( L, xmlDestroyNode );
    LUA_REGISTER( L, xmlCopyFile );
    LUA_REGISTER( L, xmlNodeGetName );
    LUA_REGISTER( L, xmlNodeSetName );
#endif //_KILLFRENZY

    LUA_REGISTER( L, utfLen );
    LUA_REGISTER( L, utfSeek );
    LUA_REGISTER( L, utfSub );
    LUA_REGISTER( L, utfChar );
    LUA_REGISTER( L, utfCode );

    LUA_REGISTER( L, getTickCount );
    LUA_REGISTER( L, getCTime );
    LUA_REGISTER( L, setTimer );
    LUA_REGISTER( L, killTimer );
    LUA_REGISTER( L, resetTimer );
    LUA_REGISTER( L, isTimer );
    LUA_REGISTER( L, getTimers );
    LUA_REGISTER( L, getTimerDetails );
    LUA_REGISTER( L, print );

    LUA_REGISTER( L, getDistance2D );
    LUA_REGISTER( L, getDistance3D );

    LUA_REGISTER( L, getResources );

#if 0
    // JSON funcs
    lua_register( m_lua, "toJSON", toJSON );
    lua_register( m_lua, "fromJSON", fromJSON );

    // Utility
    lua_register( m_lua, "getDistanceBetweenPoints2D", GetDistanceBetweenPoints2D );
    lua_register( m_lua, "getDistanceBetweenPoints3D", GetDistanceBetweenPoints3D );
    lua_register( m_lua, "getEasingValue", GetEasingValue );
    lua_register( m_lua, "interpolateBetween", InterpolateBetween );

    lua_register( m_lua, "getTickCount", GetTickCount_ );
    lua_register( m_lua, "getRealTime", GetCTime );
    lua_register( m_lua, "split", Split );
    lua_register( m_lua, "gettok", GetTok );
    lua_register( m_lua, "setTimer", SetTimer );
    lua_register( m_lua, "killTimer", KillTimer );
    lua_register( m_lua, "resetTimer", ResetTimer );
    lua_register( m_lua, "getTimers", GetTimers );
    lua_register( m_lua, "isTimer", IsTimer );
    lua_register( m_lua, "getTimerDetails", GetTimerDetails );
    lua_register( m_lua, "getColorFromString", GetColorFromString );

    // UTF functions
    lua_register( m_lua, "utfLen", UtfLen );
    lua_register( m_lua, "utfSeek", UtfSeek );
    lua_register( m_lua, "utfSub", UtfSub );
    lua_register( m_lua, "utfChar", UtfChar );
    lua_register( m_lua, "utfCode", UtfCode );

    lua_register( m_lua, "md5", Md5 );
    lua_register( m_lua, "getVersion", LuaFunctionDefs::GetVersion );
#endif

    return 0;
}

void LuaManager::Init( LuaMain *lua )
{
    lua_State *thread = lua_newthread( m_lua );
    ILuaState& api = lua_getstateapi( thread );
    api.SetMainThread( true );

    // Reference the thread so it stays alive
    lua->m_threadRef = luaL_ref( m_lua, LUA_REGISTRYINDEX );

    // Adjust for custom environment, so that resources do not collide
    lua_newenvironment( thread );

    // Prevent too long runtime
    lua_sethook( thread, ::InstructionCountHook, LUA_MASKCOUNT, HOOK_INSTRUCTION_COUNT );

    // We need threads for every environment
    lua->m_lua = thread;

    // Prepare special globals
    m_resMan->PushStack( thread );
    lua_setfield( thread, LUA_GLOBALSINDEX, "root" );

    // Setup libraries
    luaopen_base( thread );
    luaopen_math( thread );
    luaopen_string( thread );
    luaopen_table( thread );
#if 1 //_DEBUG
    luaopen_debug( thread ); // WARNING: CREATE OUR OWN DEBUG LIB!!!
#endif
    luaopen_class( thread );

    // Load our functions into the hyperstructure
    LoadCFunctions( lua );

#if 0
    // Setup the resource class
    lua_pushcclosure( thread, luamain_constructor, 0 );
    lua_newclass( thread );

    // if env.val == nil, check _G!
    lua_pushlstring( thread, "__index", 7 );
    lua_pushcclosure( thread, luamain_index, 0 );
    lua_settable( thread, 2 );

    lua_pushlstring( thread, "__metatable", 11 );
    lua_pushboolean( thread, true );
    lua_settable( thread, 2 );

    lua_setfield( thread, 1, "resMain" );

    // Add it to the global env
    lua_pushlstring( m_lua, lua->GetName().c_str(), lua->GetName().size() );
    lua_xmove( thread, m_lua, 1 );
    lua_settable( m_lua, LUA_GLOBALSINDEX );
#endif

    // Reference the manager and the context
    lua_pushlightuserdata( thread, lua );
    lua_pushlightuserdata( thread, this );

    luaL_ref( thread, LUA_STORAGEINDEX );
    luaL_ref( thread, LUA_STORAGEINDEX );

    // Notify the VM
    lua->InitVM( 1, 2 );
}

LuaMain* LuaManager::Get( lua_State *lua )
{
    // Grab the main virtual state because the one we've got passed might be a coroutine state
    // and only the main state is in our list.
    lua = lua_getmainstate( lua );

    // Find a matching VM in our list
    list <LuaMain*>::const_iterator iter = IterBegin();

    for ( ; iter != IterEnd(); iter++ )
    {
        if ( lua == (*iter)->GetVirtualMachine() )
            return *iter;
    }

    // Doesn't exist
    return NULL;
}

//TODO: Runtime unwinding
bool LuaManager::Remove( LuaMain *lua )
{
    // Remove all events registered by it
    m_events.RemoveAll( lua );

    // Unreference the threads
    luaL_unref( m_lua, LUA_REGISTRYINDEX, lua->m_threadRef );

    delete lua;

    // Remove it from our list
    m_structures.remove( lua );
    return true;
}

void LuaManager::PushReference( lua_State *L, const LuaFunctionRef& ref )
{
    lua_rawgeti( L, LUA_REGISTRYINDEX, ref.m_ref );
}

void LuaManager::DoPulse()
{
    std::list <LuaMain*>::const_iterator iter;

    for ( iter = IterBegin(); iter != IterEnd(); iter++ )
        (*iter)->DoPulse();
}

#ifndef _KILLFRENZY

bool LuaArguments::WriteToBitStream( NetBitStreamInterface& bitStream ) const
{
    bool success = true;
    std::vector <LuaArgument*>::const_iterator iter = m_args.begin();

    bitStream.WriteCompressed( (unsigned short)m_args.size() );

    for ( ; iter != m_args.end(); iter++ )
    {
        if ( !(*iter)->WriteToBitStream( bitStream ) )
            success = false;
    }

    return success;
}

#endif