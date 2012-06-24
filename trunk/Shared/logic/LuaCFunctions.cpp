/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        Shared/logic/LuaCFunctions.cpp
*  PURPOSE:     Shared Lua functions
*  DEVELOPERS:  The_GTA <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#include <StdInc.h>

using namespace google;

typedef dense_hash_map <lua_CFunction, LuaCFunction*> funcList;
static funcList luaFunctions;
typedef dense_hash_map <std::string, LuaCFunction*> protoNameList;
static protoNameList luaProtosByName;
static bool mapsInitialized = false;

LuaCFunction::LuaCFunction( const char *name, lua_CFunction proto, bool restrict )
{
    if ( name )
        m_name = name;

    m_proto = proto;
    m_restrict = restrict;
}

namespace LuaCFunctions
{
    void Init()
    {
    }

    void Destroy()
    {
        RemoveAllFunctions();
    }

    void InitializeHashMaps ( )
    {
        if ( !mapsInitialized )
        {
            mapsInitialized = true;

            luaFunctions.set_empty_key( (lua_CFunction)0x00000000 );
            luaFunctions.set_deleted_key( (lua_CFunction)0xFFFFFFFF );
            luaProtosByName.set_empty_key( std::string( "" ) );
            luaProtosByName.set_deleted_key( std::string( "\xFF" ) );
        }
    }

    LuaCFunction* AddFunction( const char *name, lua_CFunction cProto, bool restrict )
    {
        // Already have a function by this name?
        LuaCFunction *proto = GetFunction( name );

        if ( proto )
            return proto;

        // Already have a function by this address?
        proto = GetFunction( cProto );

        if ( !proto )
        {
            luaFunctions[ cProto ] = proto;
        }
        luaProtosByName[ name ] = proto;
        return proto;
    }

    LuaCFunction* GetFunction( lua_CFunction proto )
    {
        funcList::iterator it = luaFunctions.find( proto );

        if ( it == luaFunctions.end() )
            return NULL;

        return it->second;
    }

    LuaCFunction* GetFunction( const char *name )
    {
        protoNameList::iterator it = luaProtosByName.find( name );

        if ( it == luaProtosByName.end() )
            return NULL;

        return it->second;
    }

    void RegisterFunctionsWithVM( lua_State *lua )
    {
        // Register all our functions to a lua VM
        protoNameList::iterator it;

        for ( it = luaProtosByName.begin(); it != luaProtosByName.end(); it++ )
            lua_register( lua, it->first.c_str(), it->second->GetAddress() );
    }

    void RemoveAllFunctions()
    {
        // Delete all functions
        funcList::iterator it;

        for ( it = luaFunctions.begin(); it != luaFunctions.end(); it++ )
            delete it->second;

        luaFunctions.clear();
        luaProtosByName.clear();
    }
}