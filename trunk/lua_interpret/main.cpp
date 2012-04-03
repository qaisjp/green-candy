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
unsigned short fltCw;

int doubleprec( lua_State *lua )
{
    luaL_checktype( lua, 1, LUA_TBOOLEAN );

    if ( lua_toboolean( lua, 1 ) )
    {
        __asm
        {
            fstcw fltCw
            mov ax, fltCw
            or ax, 0x200
            mov fltCw,ax
            fldcw fltCw
        }
    }
    else
    {
        __asm
        {
            fstcw fltCw
            mov ax, fltCw
            and ax, 0xFDFF
            mov fltCw,ax
            fldcw fltCw
        }
    }

    lua_pushboolean( lua, true );
    return 1;
}

void lua_exec( const std::string& cmd )
{
    int top = lua_gettop( state );

    if ( luaL_loadstring( state, cmd.c_str() ) != 0 )
    {
        const char *err = lua_tostring( state, -1 );
        lua_pop( state, 1 );

        throw lua_exception( LUA_ERRSYNTAX, err );
    }

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

    lua_register( state, "doubleprec", doubleprec );

    try
    {
        while ( getline( cin, script ) )
        {
            try
            {
                std::string retCmd = "return ";
                retCmd += script;

                lua_exec( retCmd );
            }
            catch( lua_exception& )
            {
                try
                {
                    lua_exec( script );
                }
                catch( lua_exception& e )
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