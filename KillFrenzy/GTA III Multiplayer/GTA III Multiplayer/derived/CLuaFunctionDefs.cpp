/*****************************************************************************
*
*  PROJECT:     KillFrenzy! 1.0
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        derived/CLuaFunctionDefs.cpp
*  PURPOSE:     Lua client functions
*  DEVELOPERS:  The_GTA <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#include <StdInc.h>

static CScriptDebugging *debug;
static CRegisteredCommands *cmds;
static CResourceManager *resManager;

static inline CLuaMain& lua_readcontext( lua_State *L )
{
    return *lua_readuserdata <CLuaMain, LUA_STORAGEINDEX, 2> ( L );
}

namespace CLuaFunctionDefs
{
    void SetResourceManager( CResourceManager& man )
    {
        resManager = &man;
    }

    void SetRegisteredCommands( CRegisteredCommands& _cmds )
    {
        cmds = &_cmds;
    }
    
    void SetDebugging( CScriptDebugging& _debug )
    {
        debug = &_debug;
    }

    LUA_DECLARE( addCommandHandler )
    {
        luaL_checktype( L, 1, LUA_TSTRING );
        luaL_checktype( L, 2, LUA_TFUNCTION );

        CLuaMain& main = lua_readcontext( L );
        LuaFunctionRef& ref = main.CreateReference( 2 );

        if ( !cmds->Add( main, lua_getstring( L, 1 ), ref, true ) )
        {
            main.Dereference( ref );
            lua_pushboolean( L, false );
            return 1;
        }
        lua_pushboolean( L, true );
        return 1;
    }

    LUA_DECLARE( removeCommandHandler )
    {
        luaL_checktype( L, 1, LUA_TSTRING );

        std::string cmdName = lua_getstring( L, 1 );
        CLuaMain& main = lua_readcontext( L );

        if ( lua_type( L, 2 ) == LUA_TFUNCTION )
        {
            Command *cmd = cmds->Get( cmdName.c_str(), &main );
            LuaFunctionRef ref;
            
            if ( !cmd || main.GetReference( 2, ref ) && cmd->ref != ref )
            {
                lua_pushboolean( L, false );
                return 1;
            }
        }

        lua_pushboolean( L, cmds->Remove( &main, cmdName ) );
        return 1;
    }

    LUA_DECLARE( playSoundFrontEnd )
    {
        luaL_checktype( L, 1, LUA_TNUMBER );

        lua_pushboolean( L, Sound_PlayInternal2D( lua_tointeger( L, 1 ), Sound_GetFreeChannel() ) );
        return 1;
    }

    LUA_DECLARE( callSCM )
    {
        luaL_checktype( L, 1, LUA_TNUMBER );
        luaL_checktype( L, 2, LUA_TSTRING );
        luaL_checktype( L, 3, LUA_TNUMBER );

        scmThread thread( lua_tointeger( L, 1 ) );
        std::string argTypes = lua_getstring( L, 2 );
        size_t argLen = argTypes.size();

        if ( lua_gettop( L ) - 3 < (int)argLen )
        {
            lua_pushboolean( L, false );
            return 1;
        }

        int n = 0;

        for ( ; n != argLen; n++ )
        {
            switch( argTypes[n] )
            {
            case 'i':
                thread.PushInt( lua_tointeger( L, n + 4 ) );
                break;
            case 'f':
                thread.PushFloat( (float)lua_tonumber( L, n + 4 ) );
                break;
            case 's':
                thread.PushString( lua_getstring( L, n + 4 ) );
                break;
            default:
                lua_pushboolean( L, false );
                return 1;
            }
        }

        int numVars = lua_tointeger( L, 3 );

        for ( n=0; n<numVars; n++ )
            thread.PrepareVariable();

        thread.Process();

        for ( n=0; n<numVars; n++ )
            lua_pushnumber( L, thread.GetResult( n ) );

        return numVars;
    }

    LUA_DECLARE( createVehicle )
    {
        luaL_checktyperange( L, 1, LUA_TNUMBER, 4 );

        CVehicle *veh = Vehicle_Create( lua_tointeger( L, 1 ), (float)lua_tonumber( L, 2 ), (float)lua_tonumber( L, 3 ), (float)lua_tonumber( L, 4 ) );

        if ( !veh )
        {
            lua_pushboolean( L, false );
            return 1;
        }

        CGameVehicle& gameVeh = *new CGameVehicle( L, false, *veh );
        gameVeh.PushStack( L );
        return 1;
    }
}