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

static void InstructionCountHook( lua_State *lua, lua_Debug *debug )
{
    // Grab our lua VM
    lua_getmanager( lua )->InstructionCountHook();
}

static int mainaccess_global( lua_State *lua )
{
    return lua_getmanager( lua )->AccessGlobalTable();
}

static const luaL_Reg interface_access = {
    { "_G", mainaccess_global },
    { "_ENV", mainaccess_global },
    { 0, 0 }
};

LuaManager::LuaManager( RegisteredCommands& commands, Events& events, ScriptDebugging& debug )
{
    m_commands = commands;
    m_events = events;
    m_debug = debug;

    ResetInstructionCount();

    // Setup the virtual machine
    m_lua = luaL_newstate();
    lua_sethook( m_lua, InstructionCountHook, LUA_MASKCOUNT, HOOK_INSTRUCTION_COUNT );

    // Cache the lua manager in the VM
    lua_pushlightuserdata( m_lua, this );
    luaL_ref( m_lua, 1 );  // has to be first!

    // Setup libraries
    luaopen_base( m_lua );
    luaopen_math( m_lua );
    luaopen_string( m_lua );
    luaopen_table( m_lua );
    luaopen_debug( m_lua ); // WARNING: CREATE OUR OWN DEBUG LIB!!!

    // Fast access global interface
    lua_newtable( m_lua );
    luaL_openlib( m_lua, NULL, interface_access, 0 );

    // Load our C Functions into LUA and hook callback
    LuaCFunctions::InitializeHashMaps();
    LoadCFunctions();

    lua_registerPreCallHook( CLuaDefs::CanUseFunction );
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

    while ( LuaMain *env = PopStatus( &line, &src, &proto ) )
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

    throw lua_exception( id, msg );
}

void LuaManager::InstructionCountHook()
{
    if ( timeGetTime() < m_functionEnter + HOOK_MAXIMUM_TIME )
        return;

    LuaMain *env = GetStatus();

    // Print it in the console
    Logger::ErrorPrintf( "Infinite/too long execution (%s)", env->GetName().c_str() );

    // Error out
    Throw( LUA_ERRRUN, "Aborting; infinite running script" );
}

void LuaManager::CallStack( int args )
{
    if ( lua_type( m_lua, -1 ) != LUA_TFUNCTION )
        throw lua_exception( LUA_ERRSYNTAX, "expected function at LuaManager::CallStack!" );

    // We could theoretically protect it ourselves
    switch( int ret = lua_pcall( m_lua, args, LUA_MULTRET, 0 ) )
    {
    case LUA_ERRRUN:
    case LUA_ERRMEM:
        size_t len;
        const char *err = lua_tolstring( m_lua, -1, &len );
        std::string msg( err, len );

        // Clean the stack
        lua_settop( m_lua, -2 );

        throw lua_exception( ret, msg );
    }
}

void LuaManager::CallStackVoid( int args )
{
    int top = lua_gettop( m_lua );

    CallStack( args );

    lua_settop( m_lua, top - args );
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
    lua_rawgeti( m_lua, LUA_REGISTRYINDEX, m_access );
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
    // Create the hyperstructure
    lua_newtable( m_lua );
    int structure = lua_gettop( m_lua );

    // Create the common metatable
    lua_newtable( m_lua );
    int meta = lua_gettop( m_lua );
    
    // if env.val == nil, check _G!
    lua_pushlstring( m_lua, "__index", 7 );
    lua_pushcclosure( m_lua, luamain_index, 0 );
    lua_settable( m_lua, meta );

    lua_pushlstring( m_lua, "__metatable", 11 );
    lua_pushboolean( m_lua, true );
    lua_settable( m_lua, meta );

    lua_setmetatable( m_lua, structure );

    // Add it to the global env
    lua_pushlstring( m_lua, lua->GetName().c_str(), lua->GetName().size() );
    lua_setglobal( m_lua, structure );
    
    // Notify the VM
    lua->InitVM( structure, meta );
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

bool LuaManager::Remove( LuaMain *lua )
{
    if ( !lua )
        return false;

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