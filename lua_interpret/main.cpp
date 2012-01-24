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

#include "main.h"

using namespace std;

lua_State *state;

void lua_exec( const std::string& cmd )
{
    int top = lua_gettop( state );

    luaL_loadstring( state, cmd.c_str() );
    if ( lua_pcall( state, 0, LUA_MULTRET, 0 ) != 0 )
    {
        const char *err = lua_tostring( state, -1 );
        lua_pop( state, 1 );

        throw lua_exception( LUA_ERRRUN, err );
    }

    int now = lua_gettop( state );

    if ( now == top )
        return;

    cout << "> ";

    int n;

    for ( n=top; n != now; n++ )
    {
        const char *type = lua_typename( state, lua_type( state, n + 1 ) );
        const char *strRep = lua_tostring( state, n + 1 );

        if ( !strRep )
        {
            cout << type;
            cout << ", ";
        }
        else
        {
            cout << strRep;
            cout << " [";
            cout << type;
            cout << "], ";
        }
    }

    cout << "\n";

    lua_settop( state, top );
}

int main( int argc, char *argv[] )
{
    std::string script;

    state = lua_open();

    luaL_openlibs( state );

    try
    {
        while ( getline( cin, script ) )
        {
            try
            {
                lua_exec( script );
            }
            catch( lua_exception& )
            {
                try
                {
                    std::string retCmd = "return ";
                    retCmd += script;

                    lua_exec( retCmd );
                }
                catch( lua_exception& e )
                {
                    cout << "error\n";
                    cout << e.what();
                    cout << "\n";
                }
            }
        }
    }
    catch( ... )
    {
        cout << "terminated";
    }

    lua_close( state );
    return EXIT_SUCCESS;
}