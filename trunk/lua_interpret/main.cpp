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

class proc_exit : public std::exception
{
public:
    proc_exit( const char *msg ) : std::exception( msg )
    {
    }
};

int exitproc( lua_State *L )
{
    throw proc_exit( ":D" );
}

void lua_exec( const std::string& cmd )
{
    int top = lua_gettop( state );

    if ( luaL_loadstring( state, cmd.c_str() ) != 0 )
    {
        const char *err = lua_tostring( state, -1 );
        lua_pop( state, 1 );

        throw lua_exception( state, LUA_ERRSYNTAX, err );
    }

    if ( lua_pcall( state, 0, LUA_MULTRET, 0 ) != 0 )
    {
        const char *err = lua_tostring( state, -1 );
        lua_pop( state, 1 );
        
        throw lua_exception( state, LUA_ERRRUN, err );
    }

    int now = lua_gettop( state );

    if ( now == top )
        return;

    cout << "> ";

    int n;

    for ( n=top; n != now; n++ )
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

void loadBenchFile( const filePath& path, void *ud )
{
    filePath relPath;
    fileRoot->GetRelativePath( path, true, relPath );

    std::vector <char> buff;
    fileRoot->ReadToBuffer( path, buff );

    // Zero terminate
    buff.push_back( 0 );

    if ( luaL_loadstring( state, &buff[0] ) != 0 )
    {
        cout << "failed to load library " << relPath << "\n";
        return;
    }

    if ( lua_pcall( state, 0, 0, 0 ) != 0 )
    {
        cout << "failed to run library " << relPath << "\n";
        cout << lua_tostring( state, -1 ) << "\n";

        lua_pop( state, 1 );
        return;
    }

    int top = lua_gettop( state );

    cout << "init: " << relPath << "\n";
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

    cout << e.what();
    cout << "\n";
}

void signal_handler( int sig )
{
    lua_close( state );

    delete fileSystem;

    exit( EXIT_SUCCESS );
}

int main( int argc, char *argv[] )
{
    std::string script;

    state = lua_open();

    srand( (unsigned int)time( NULL ) );

    signal( SIGTERM, signal_handler );
    signal( SIGBREAK, signal_handler );

    new CFileSystem();

    luaL_openlibs( state );
    luafile_open( state );

    lua_pushvalue( state, LUA_GLOBALSINDEX );
    luafilesystem_open( state );
    lua_setfield( state, -2, "file" );

    // Include everything from /luabench/
    cout << "starting luaBench files...\n";

    fileRoot->ScanDirectory( "/luabench/", "*.lua", false, NULL, loadBenchFile, NULL );

    cout << "\n";

    lua_register( state, "doubleprec", doubleprec );
    lua_register( state, "exit", exitproc );

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
            catch( lua_exception& _e )
            {
                if ( _e.status() != LUA_ERRSYNTAX )
                {
                    handleError( _e );
                    continue;
                }

                try
                {
                    lua_exec( script );
                }
                catch( lua_exception& e )
                {
                    handleError( e );
                }
            }
        }
    }
    catch( ... )
    {
        cout << "terminated";
    }

    lua_close( state );

    delete fileSystem;

    return EXIT_SUCCESS;
}