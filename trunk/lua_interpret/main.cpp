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

#include "../Shared/logic/networking/NetworkStruct.h"

using namespace Networking;

static const NetworkDataType testStructDef[] =
{
    { NETWORK_BOOL, "testBool" },
    { NETWORK_BYTE, "testByte" },
    { NETWORK_WORD, "testWord" },
    { NETWORK_DWORD, "testDWord" },
    { NETWORK_FLOAT, "testFloat" },
    { NETWORK_DOUBLE, "testDouble" }
};

enum eTestStruct
{
    TEST_BOOL,
    TEST_BYTE,
    TEST_WORD,
    TEST_DWORD,
    TEST_FLOAT,
    TEST_DOUBLE
};

struct testStruct
{
    bool tbool;
    char tbyte;
    short tword;
    int tint;
    float tfloat;
    double tdouble;

#pragma warning(push)
#pragma warning(disable: 4800)
#pragma warning(disable: 4244)
    template <class type>
    inline type NetworkRead( const unsigned char id ) const
    {
        switch( id )
        {
        case TEST_BOOL:     return tbool;
        case TEST_BYTE:     return tbyte;
        case TEST_WORD:     return tword;
        case TEST_DWORD:    return tint;
        case TEST_FLOAT:    return tfloat;
        case TEST_DOUBLE:   return tdouble;
        }

        return 0;
    }

    template <>
    inline CVector NetworkRead <CVector> ( const unsigned char id ) const
    {
        return CVector();
    }

    template <class type>
    inline void NetworkWrite( const unsigned char id, const type val )
    {
        switch( id )
        {
        case TEST_BOOL:     tbool = val; return;
        case TEST_BYTE:     tbyte = val; return;
        case TEST_WORD:     tword = val; return;
        case TEST_DWORD:    tint = val; return;
        case TEST_FLOAT:    tfloat = val; return;
        case TEST_DOUBLE:   tdouble = val; return;
        }
    }

    template <>
    inline void NetworkWrite <CVector> ( const unsigned char id, const CVector val )
    {
        
    }
#pragma warning(pop)
};

typedef NetworkSyncStruct <testStruct, ETSIZE(testStructDef)> test_network;

struct bitshit
{
    bitshit()
    {
        a = false; b = false; c = false; d = false;
        e = false; f = false; g = false; h = false;
    }

    bool a : 1;
    bool b : 1;
    bool c : 1;
    bool d : 1;
    bool e : 1;
    bool f : 1;
    bool g : 1;
    bool h : 1;
};

#define FIELD_A     0x01
#define FIELD_B     0x02
#define FIELD_C     0x04
#define FIELD_D     0x08
#define FIELD_E     0x10
#define FIELD_F     0x20
#define FIELD_G     0x40
#define FIELD_H     0x80

int main( int argc, char *argv[] )
{
    bitshit z;
    size_t s = sizeof(z);

    z.a = 1;
    z.d = 1;
    z.e = 1;
    z.g = 1;
    z.h = 1;

    unsigned char u = *(unsigned char*)&z;
    bool o = ( u & FIELD_B ) != 0;

    test_network instance( testStructDef );
    testStruct a;
    testStruct b;

    test_network::streamType stream( testStructDef );

    instance.Set( b );

    a.tbool = true;
    b.tdouble = 5.0;

    instance.Write( b, stream );
    

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