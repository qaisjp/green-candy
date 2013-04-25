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

#include <SharedUtil.hpp>

#define DATA_TEXTURE_BLOCK      20000
#define DATA_COLL_BLOCK         25000
#define DATA_IPL_BLOCK          25255
#define DATA_PATHFIND_BLOCK     25511
#define DATA_ANIM_BLOCK         25575
#define DATA_RECORD_BLOCK       25755

using namespace std;

lua_State *state;

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

    lua_Debug debug;
    e.getDebug( debug );

    if ( debug.currentline != -1 )
        cout << debug.short_src << ' ';

    cout << e.what();
    cout << "\n";
}

void signal_handler( int sig )
{
    lua_close( state );

    delete fileSystem;

    exit( EXIT_SUCCESS );
}

static int lua_newmd5hasher( lua_State *L )
{
    CMD5Hasher *hasher = new CMD5Hasher();
    hasher->Init();

    lua_createmd5hasher( L, hasher );
    return 1;
} 

// This is an optimization because the computer binary system works that way.
// Removes the need for additional clock cycles by avoiding conversion to integer.
#define usOffset( num, off )                ((unsigned short)( (unsigned short)(num) - (unsigned short)(off) ))
#define usRangeCheck( num, off, range )     (usOffset((num),(off)) < (unsigned short)(range))
#define usRangeCheckEx( num, range )        ((unsigned short)(num) < (unsigned short)(range))

#include "../MTA10/game_sa/CStreamingSA.utils.hxx"

unsigned short array_troll[500000];

struct VerifyIntegrityCheck : ModelCheckDispatch <true>
{
    unsigned char& trollCode;

    VerifyIntegrityCheck( unsigned char& code ) : trollCode( code )
    {
    }

    bool __forceinline DoBaseModel( unsigned short id )
    {
        if ( array_troll[id * 80] == 0xCACA )
            return true;

        array_troll[1] = 0xcafe;
        return false;
    }

    bool __forceinline DoTexDictionary( unsigned short id )
    {
        trollCode = id;
        return true;
    }
};

int main( int argc, char *argv[] )
{
    memset(array_troll, 0xFA, sizeof(array_troll));

    unsigned char trollCode = 0;

    if ( !DefaultDispatchExecute( argc, VerifyIntegrityCheck( trollCode ) ) )
        __asm nop

    if ( trollCode == 0xBB )
        __asm nop

    std::string script;

    state = lua_open();

    srand( (unsigned int)time( NULL ) );

    signal( SIGTERM, signal_handler );
    signal( SIGBREAK, signal_handler );

    new CFileSystem();

    luaL_openlibs( state );
    luafile_open( state );

    lua_register( state, "newmd5hasher", lua_newmd5hasher );

    lua_pushvalue( state, LUA_GLOBALSINDEX );
    luafilesystem_open( state );
    lua_setfield( state, -2, "file" );

    if ( fileRoot->Exists( "/luabench/" ) )
    {
        // Include everything from /luabench/
        cout << "starting luaBench files...\n";

        fileRoot->ScanDirectory( "/luabench/", "*.lua", false, NULL, loadBenchFile, NULL );

        cout << "\n";
    }

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