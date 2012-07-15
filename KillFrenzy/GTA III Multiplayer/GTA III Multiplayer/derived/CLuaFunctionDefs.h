/*****************************************************************************
*
*  PROJECT:     KillFrenzy! 1.0
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        derived/CLuaFunctionDefs.h
*  PURPOSE:     Lua client functions
*  DEVELOPERS:  The_GTA <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#ifndef _LUA_FUNCTIONS_
#define _LUA_FUNCTIONS_

namespace CLuaFunctionDefs
{
    void SetResourceManager( CResourceManager& man );
    void SetRegisteredCommands( CRegisteredCommands& cmds );
    void SetDebugging( CScriptDebugging& debug );

    // Command functions
    LUA_DECLARE( addCommandHandler );
    LUA_DECLARE( removeCommandHandler );

    // Audio functions
    LUA_DECLARE( playSoundFrontEnd );

    // SCM functions
    LUA_DECLARE( callSCM );

    // Vehicle functions
    LUA_DECLARE( createVehicle );

    // Utility functions
    LUA_DECLARE( print );
};

#endif //_LUA_FUNCTIONS_