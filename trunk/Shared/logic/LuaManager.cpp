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

static int lua_pushstackthread( lua_State *lua )
{
    lua_getmanager( lua )->PushThread( lua_tothread( lua, -1 ) );
    return 0;
}

static int lua_popstackthread( lua_State *lua )
{
    lua_getmanager( lua )->PopThread();
    return 0;
}

#define lua_getglobalproxy( L ) (lua_rawseti( L, LUA_REGISTRYINDEX, 2 ))
#define lua_getaccessinterface( L ) (lua_rawseti( L, LUA_REGISTRYINDEX, 3))

static void InstructionCountHook( lua_State *lua, lua_Debug *debug )
{
    lua_getmanager( lua )->InstructionCountHook();
}

static int mainaccess_global( lua_State *lua )
{
    return lua_getmanager( lua )->AccessGlobalTable();
}

static const luaL_Reg interface_access =
{
    { "_G", mainaccess_global },
    { 0, 0 }
};

static int luaglobal_index( lua_State *lua )
{
    return lua_getmanager( lua )->AccessGlobal();
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

LuaManager::LuaManager( RegisteredCommands& commands, Events& events, ScriptDebugging& debug )
{
    m_commands = commands;
    m_events = events;
    m_debug = debug;

    ResetInstructionCount();

    // Setup the virtual machine
    m_lua = luaL_newstate();
    lua_sethook( m_lua, InstructionCountHook, LUA_MASKCOUNT, HOOK_INSTRUCTION_COUNT );

    // Setup callbacks
    lua_setevent( m_lua, LUA_EVENT_THREAD_CONTEXT_PUSH, lua_pushstackthread );
    lua_setevent( m_lua, LUA_EVENT_THREAD_CONTEXT_POP, lua_popstackthread );

    // Cache the lua manager in the VM
    lua_pushlightuserdata( m_lua, this );
    luaL_ref( m_lua, LUA_REGISTRYINDEX );   // first in registry!

    // Setup the global
    lua_newtable( L );
    lua_pushvalue( L, 1 );
    luaL_ref( m_lua, LUA_REGISTRYINDEX );   // second in registry!

    // Setup libraries
    luaopen_base( m_lua );
    luaopen_math( m_lua );
    luaopen_string( m_lua );
    luaopen_table( m_lua );
    luaopen_class( m_lua );
    luaopen_debug( m_lua ); // WARNING: CREATE OUR OWN DEBUG LIB!!!

    // Fast access global interface
    lua_newtable( m_lua );
    luaL_openlib( m_lua, NULL, interface_access, 0 );
    luaL_ref( m_lua, LUA_REGISTRYINDEX );   // third in registry!

    // Initialize back hashmaps
    LuaCFunctions::InitializeHashMaps();
}

LuaManager::~LuaManager()
{
    LuaCFunctions::RemoveAllFunctions();

    std::list <LuaMain*>::iterator iter;

    for ( iter = IterBegin(); iter != IterEnd(); iter++ )
        delete *iter;

    // Destroy lua environment
    lua_close( m_lua );
}

void LuaManager::PushStatus( const LuaMain& main )
{
    if ( !m_envStack.empty() )
        (*m_envStack.end()).Finalize();

    m_envStack.push_back( env_status( *m_lua, main ) );
}

const LuaMain* LuaManager::GetStatus( int *line, std::string *src, std::string *proto_name )
{
    if ( m_envStack.empty() )
        return NULL;

    return &(*m_envStack.end()).Get( line, src, proto_name );
}

const LuaMain* LuaManager::PopStatus( int *line, std::string *src, std::stirng *proto_name )
{
    if ( m_envStack.empty() )
        return NULL;

    const LuaMain& context = (*m_envStack.end()).Get( line, src, proto_name );

    m_envStack.pop_back();

    if ( !m_envStack.empty() )
        (*m_envStack.end()).Resume();

    return &context;
}

void LuaManager::Throw( unsigned int id, const char *error )
{
    std::string msg = error;
    msg += '\n';

    int line;
    std::string src;
    std::string proto;

    while ( PopStatus( &line, &src, &proto ) )
    {
        if ( line == -1 )
        {
            msg += "unknown C proto\n";
            continue;
        }

        msg += '"';
        msg += src;
        msg += '":';
        msg += proto;
        msg += ':';
        msg += SString( "%u", line );
        msg += '\n';
    }

    throw lua_exception( m_lua, id, msg );
}

void LuaManager::InstructionCountHook()
{
    if ( timeGetTime() < m_functionEnter + HOOK_MAXIMUM_TIME )
        return;

    const LuaMain *env = GetStatus();

    // Print it in the console
    Logger::ErrorPrintf( "Infinite/too long execution (%s)", env->GetName().c_str() );

    // Error out
    Throw( LUA_ERRRUN, "Aborting; infinite running script" );
}

void LuaManager::CallStack( int args )
{
#ifdef LUA_PROTECT_DEFAULT
    if ( lua_type( m_lua, -args - 1 ) != LUA_TFUNCTION )
        Throw( LUA_ERRSYNTAX, "expected function at LuaManager::CallStack!" );

    // We could theoretically protect it ourselves
    switch( int ret = lua_pcall( m_lua, args, LUA_MULTRET, 0 ) )
    {
    case LUA_ERRRUN:
    case LUA_ERRMEM:
        size_t len;
        const char *err = lua_tolstring( m_lua, -1, &len );
        std::string msg( err, len );

        // Clean the stack
        lua_settop( m_lua, -3 );

        throw lua_exception( m_lua, ret, msg );
    }
#else
    lua_call( m_lua, args, LUA_MULTRET );
#endif //LUA_PROTECT_DEFAULT
}

void LuaManager::CallStackVoid( int args )
{
#ifdef LUA_PROTECT_DEFAULT
    int top = lua_gettop( m_lua );

    CallStack( args );

    lua_settop( m_lua, top - args );
#else
    lua_call( m_lua, args, 0 );
#endif //LUA_PROTECT_DEFAULT
}

LuaArguments LuaManager::CallStackResult( int argc )
{
    int top = lua_gettop( m_lua );

    CallStack( argc );

    LuaArguments args;
    int rettop = lua_gettop( m_lua );

    while ( rettop != top )
        args.ReadArgument( m_lua, rettop-- );

    lua_settop( m_lua, top - argc );
    return args;
}

bool LuaManager::PCallStack( int argc )
{
    int top = lua_gettop( m_lua );

    try
    {
        CallStack( argc );
    }
    catch( lua_exception& e )
    {
        lua_settop( m_lua, top );

        m_debug.LogError( "%s", e.what() );
        return false;
    }
    return true;
}

bool LuaManager::PCallStackVoid( int argc )
{
    int top = lua_gettop( m_lua );

    try
    {
        CallStackVoid( argc );
    }
    catch( lua_exception& e )
    {
        lua_settop( m_lua, top );

        m_debug.LogError( "%s", e.what() );
        return false;
    }
    return true;
}

LuaArguments LuaManager::PCallStackResult( int argc, bool& excpt )
{
    LuaArguments args;
    int top = lua_gettop( m_lua );

    if ( lua_pcall( m_lua, argc, LUA_MULTRET, 0 ) != 0 )
    {
        m_debug.LogError( "%s", lua_tostring( m_lua, top + 1 ) );
        excpt = true;
    }
    else
        excpt = false;

    args.ReadArguments( m_lua, top );
    return args;
}

static int luamain_index( lua_State *lua )
{
    lua_gettable( lua, 1 );

    if ( !lua_isnil( m_lua, -1 ) )
        return 1;

    lua_pop( m_lua, 1 );
    return lua_getmanager( lua )->AccessGlobal();
}

// ARGS: table, key
int LuaManager::AccessGlobal()
{
    lua_getaccessinterface( m_lua );
    lua_pushvalue( m_lua, -2 );
    lua_gettable( m_lua, -2 );

    if ( lua_isnil( m_lua, -1 ) )
    {
        lua_getglobal( m_lua, -2 );
        return 1;
    }

    try
    {
        int top = lua_gettop( m_lua );

        CallStack( 1 );

        return lua_gettop( m_lua ) - top;
    }
    catch( lua_exception& e )
        m_debug.LogError( m_lua, e.what() );

    return 0;
}

int LuaManager::AccessGlobalTable()
{
    // Default access to luaGlobal is allowed
    lua_pushvalue( m_lua, LUA_GLOBALSINDEX );
    return 1;
}

void LuaManager::Init( LuaMain *lua )
{
    lua_State *thread = lua_newthread( m_lua );

    // We need threads for every environment
    lua->m_lua = thread;

    // Create the hyperstructure
    lua_newtable( thread );

    // Load our functions into the VM
    LoadCFunctions();

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

    lua_setfield( L, 1, "resMain" );

    // Add it to the global env
    lua_pushlstring( m_lua, lua->GetName().c_str(), lua->GetName().size() );
    lua_setglobal( m_lua, 1 );

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
    // Remove all events registered by it and all commands added
    m_events.RemoveAllEvents( lua );
    m_commands.CleanUpForVM( lua );

    delete lua;

    // Remove it from our list
    m_structures.remove( lua );
    return true;
}

void LuaManager::DoPulse()
{
    std::list <LuaMain*>::iterator iter;

    for ( iter = IterBegin(); iter != IterEnd(); iter++ )
        (*iter)->DoPulse();
}

void LuaManager::LoadCFunctions()
{
    // JSON funcs
    lua_register( m_lua, "toJSON", LuaFunctionDefinitions::toJSON );
    lua_register( m_lua, "fromJSON", LuaFunctionDefinitions::fromJSON );

    // Utility
    lua_register( m_lua, "getDistanceBetweenPoints2D", LuaFunctionDefinitions::GetDistanceBetweenPoints2D );
    lua_register( m_lua, "getDistanceBetweenPoints3D", LuaFunctionDefinitions::GetDistanceBetweenPoints3D );
    lua_register( m_lua, "getEasingValue", LuaFunctionDefinitions::GetEasingValue );
    lua_register( m_lua, "interpolateBetween", LuaFunctionDefinitions::InterpolateBetween );

    lua_register( m_lua, "getTickCount", LuaFunctionDefinitions::GetTickCount_ );
    lua_register( m_lua, "getRealTime", LuaFunctionDefinitions::GetCTime );
    lua_register( m_lua, "split", LuaFunctionDefinitions::Split );
    lua_register( m_lua, "gettok", LuaFunctionDefinitions::GetTok );
    lua_register( m_lua, "setTimer", LuaFunctionDefinitions::SetTimer );
    lua_register( m_lua, "killTimer", LuaFunctionDefinitions::KillTimer );
    lua_register( m_lua, "resetTimer", LuaFunctionDefinitions::ResetTimer );
    lua_register( m_lua, "getTimers", LuaFunctionDefinitions::GetTimers );
    lua_register( m_lua, "isTimer", LuaFunctionDefinitions::IsTimer );
    lua_register( m_lua, "getTimerDetails", LuaFunctionDefinitions::GetTimerDetails );
    lua_register( m_lua, "getColorFromString", LuaFunctionDefinitions::GetColorFromString );

    // UTF functions
    lua_register( m_lua, "utfLen", LuaFunctionDefinitions::UtfLen );
    lua_register( m_lua, "utfSeek", LuaFunctionDefinitions::UtfSeek );
    lua_register( m_lua, "utfSub", LuaFunctionDefinitions::UtfSub );
    lua_register( m_lua, "utfChar", LuaFunctionDefinitions::UtfChar );
    lua_register( m_lua, "utfCode", LuaFunctionDefinitions::UtfCode );

    lua_register( m_lua, "md5", LuaFunctionDefinitions::Md5 );
    lua_register( m_lua, "getVersion", LuaFunctionDefinitions::GetVersion );
}

void LuaManager::ResetInstructionCount()
{
    m_functionEnter = timeGetTime();
}

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
