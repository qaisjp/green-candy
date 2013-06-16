/*****************************************************************************
*
*  PROJECT:     Lua Interpreter
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        benchmarks/netbench.cpp
*  PURPOSE:     Network Structure Traversal benchmark utility
*  DEVELOPERS:  Martin Turski <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#include <StdInc.h>
#include "../Shared/logic/networking/NetworkStruct.h"

struct syncObject
{
    syncObject( void )
    {
        a = b = c = 0.0f;
        d = e = f = 0;
        g = h = i = false;
    }

    float a, b, c;
    int d, e, f;
    bool g, h, i;

    NETWORK_NULL_DATA

    template <>
    NET_FAST void NetworkRead( const unsigned int id, float& val ) const
    {
        if ( id == 0 )          val = a;
        else if ( id == 1 )     val = b;
        else if ( id == 2 )     val = c;
    }

    template <>
    NET_FAST void NetworkRead( const unsigned int id, int& val ) const
    {
        if ( id == 3 )          val = d;
        else if ( id == 4 )     val = e;
        else if ( id == 5 )     val = f;
    }

    template <>
    NET_FAST void NetworkRead( const unsigned int id, bool& val ) const
    {
        if ( id == 6 )          val = g;
        else if ( id == 7 )     val = h;
        else if ( id == 8 )     val = i;
    }

    template <>
    NET_FAST void NetworkWrite( const unsigned int id, const float& val )
    {
        if ( id == 0 )          a = val;
        else if ( id == 1 )     b = val;
        else if ( id == 2 )     c = val;
    }

    template <>
    NET_FAST void NetworkWrite( unsigned int id, const int& val )
    {
        if ( id == 3 )          d = val;
        else if ( id == 4 )     e = val;
        else if ( id == 5 )     f = val;
    }

    template <>
    NET_FAST void NetworkWrite( unsigned int id, const bool& val )
    {
        if ( id == 6 )          g = val;
        else if ( id == 7 )     h = val;
        else if ( id == 8 )     i = val;
    }
};

using namespace Networking;

static LUA_DECLARE( updateTest )
{
    // Network dispatcher benchmarking
    const Networking::NetworkDataType sync_descr[] =
    {
        { NETWORK_FLOAT, "a" },
        { NETWORK_FLOAT, "b" },
        { NETWORK_FLOAT, "c" },
        { NETWORK_DWORD, "d" },
        { NETWORK_DWORD, "e" },
        { NETWORK_DWORD, "f" },
        { NETWORK_BOOL, "g" },
        { NETWORK_BOOL, "h" },
        { NETWORK_BOOL, "i" }
    };

    typedef DefaultDataStreamer <ETSIZE(sync_descr)> myStreamer;
    myStreamer streamer1( sync_descr );
    myStreamer streamer2( sync_descr );

    typedef NetworkSyncStruct <syncObject, ETSIZE(sync_descr)> mySync;
    mySync sync1( sync_descr );
    mySync sync2( sync_descr );

    mySync::streamType stream1( sync_descr );

    for ( unsigned int n = 0; n < 1000000; n++ )
    {
        syncObject a;
        syncObject b;

        sync1.Set( sync_descr, a );

        a.a = 1.0;
        a.f = 70;

        stream1.Reset();
        sync1.Write( sync_descr, a, stream1 );
        stream1.Reset();
        sync1.Establish( sync_descr, b, stream1 );

        if ( b.a == 1.0f )
            __asm nop
    }
    LUA_SUCCESS;
}

static const luaL_Reg netbench_interface[] =
{
    LUA_METHOD( updateTest ),
    { NULL, NULL }
};

void luanetbench_open( lua_State *L )
{
    luaL_openlib( L, "netbench", netbench_interface, 0 );
}