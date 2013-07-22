/*****************************************************************************
*
*  PROJECT:     Lua Interpreter
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        LuaDefs/lua.resource.cpp
*  PURPOSE:     Lua resource function definitions
*  DEVELOPERS:  Martin Turski <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#include <StdInc.h>

namespace CLuaFunctionDefs
{
    LUA_DECLARE( newResource )
    {
        lua_String path;
        std::string name;

        LUA_ARGS_BEGIN;
        argStream.ReadString( path );
        argStream.ReadString( name );
        LUA_ARGS_END;

        filePath absRoot;
        LUA_CHECK( fileRoot->GetFullPath( path, false, absRoot ) );
        
        CResource *res = resMan->Create( absRoot, name );

        LUA_CHECK( res );

        res->PushStack( L );
        return 1;
    }
};