/*****************************************************************************
*
*  PROJECT:     Lua Interpreter
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        luafile.cpp
*  PURPOSE:     Test environment for the filesystem
*  DEVELOPERS:  The_GTA <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#include <StdInc.h>
#include "luafile.Utils.hxx"

using namespace std;

static int luafile_onIndex( lua_State *lua )
{
    lua_pushvalue( lua, 2 );
    lua_gettable( lua, lua_upvalueindex( 1 ) );

    if ( lua_type( lua, 3 ) == LUA_TBOOLEAN && lua_toboolean( lua, 3 ) == false )
        return 0;

    lua_pop( lua, 1 );
    lua_gettable( lua, lua_upvalueindex( 2 ) );
    return 1;
}

static int luafile_onNewindex( lua_State *lua )
{
    // We do not allow any modification from outside
    return 0;
}

static int luafile_read( lua_State *lua )
{
    luaL_checktype( lua, 1, LUA_TNUMBER );

    CFile *file = (CFile*)lua_touserdata( lua, lua_upvalueindex( 1 ) );

    size_t bytesRead = (size_t)lua_tonumber( lua, 1 );

    if ( bytesRead == 0 )
    {
        lua_pushlstring( lua, "", 0 );
        return 1;
    }

    std::vector <char> buf( bytesRead );

#ifdef FU_CLASS
    bytesRead = file->Read( &buf[0], 1, bytesRead );

    if ( bytesRead == 0 )
    {
        lua_pushlstring( lua, "", 0 );
        return 1;
    }
#else
    bytesRead = file->Read( &buf[0], 1, bytesRead );
#endif

    lua_pushlstring( lua, &buf[0], bytesRead );
    return 1;
}

static int luafile_readShort( lua_State *lua )
{
    lua_pushnumber( lua, ((CFile*)lua_touserdata( lua, lua_upvalueindex( 1 ) ))->ReadShort() );
    return 1;
}

static int luafile_readInt( lua_State *lua )
{
    lua_pushnumber( lua, ((CFile*)lua_touserdata( lua, lua_upvalueindex( 1 ) ))->ReadInt() );
    return 1;
}

static int luafile_readFloat( lua_State *lua )
{
    lua_pushnumber( lua, ((CFile*)lua_touserdata( lua, lua_upvalueindex( 1 ) ))->ReadFloat() );
    return 1;
}

static int luafile_write( lua_State *L )
{
    luaL_checktype( L, 1, LUA_TSTRING );

    size_t len;
    const char *string = lua_tolstring( L, 1, &len );

    lua_pushnumber( L, ((CFile*)lua_touserdata( L, lua_upvalueindex( 1 ) ) )->Write( string, 1, len ) );
    return 1;
}

static int luafile_writeShort( lua_State *L )
{
    luaL_checktype( L, 1, LUA_TNUMBER );
    lua_pushnumber( L, ((CFile*)lua_touserdata( L, lua_upvalueindex( 1 ) ) )->WriteShort( (int)lua_tonumber( L, 1 ) ) );
    return 1;
}

static int luafile_writeInt( lua_State *L )
{
    luaL_checktype( L, 1, LUA_TNUMBER );
    lua_pushnumber( L, ((CFile*)lua_touserdata( L, lua_upvalueindex( 1 ) ) )->WriteInt( (int)lua_tonumber( L, 1 ) ) );
    return 1;
}

static int luafile_writeFloat( lua_State *L )
{
    luaL_checktype( L, 1, LUA_TNUMBER );
    lua_pushnumber( L, ((CFile*)lua_touserdata( L, lua_upvalueindex( 1 ) ) )->WriteFloat( (float)lua_tonumber( L, 1 ) ) );
    return 1;
}

static int luafile_size( lua_State *L )
{
    lua_pushnumber( L, ((CFile*)lua_touserdata( L, lua_upvalueindex( 1 ) ) )->GetSize() );
    return 1;
}

static int luafile_stat( lua_State *L )
{
    struct stat stats;
    
    if ( !((CFile*)lua_touserdata( L, lua_upvalueindex( 1 ) ))->Stat( &stats ) )
        return 0;

    luafile_pushStats( L, stats );
    return 1;
}

static int luafile_tell( lua_State *L )
{
    lua_pushnumber( L, ((CFile*)lua_touserdata( L, lua_upvalueindex( 1 ) ) )->Tell() );
    return 1;
}

static int luafile_seek( lua_State *L )
{
    luaL_checktype( L, 1, LUA_TNUMBER );

    int seekType;

    switch( lua_type( L, 2 ) )
    {
    case LUA_TNUMBER:
        if ( (seekType = (int)lua_tonumber( L, 2 )) < 0 || seekType > SEEK_END )
            goto defMethod;

        break;
    case LUA_TSTRING:
        {
            const char *type = lua_tostring( L, 2 );

            if ( strcmp( type, "cur" ) == 0 )
            {
                seekType = SEEK_CUR;
                break;
            }
            else if ( strcmp( type, "set" ) == 0 )
            {
                seekType = SEEK_SET;
                break;
            }
            else if ( strcmp( type, "end" ) == 0 )
            {
                seekType = SEEK_END;
                break;
            }
        }
    default:
defMethod:
        lua_pushstring( L, "unknown seekmode" );
        lua_error( L );
        return -1;
    }

    lua_pushnumber( L, ((CFile*)lua_touserdata( L, lua_upvalueindex( 1 ) ) )->Seek( (long)lua_tonumber( L, 1 ), seekType ) );
    return 1;
}

static int luafile_eof( lua_State *L )
{
    lua_pushboolean( L, ((CFile*)lua_touserdata( L, lua_upvalueindex( 1 ) ) )->IsEOF() );
    return 1;
}

static int luafile_flush( lua_State *L )
{
    ((CFile*)lua_touserdata( L, lua_upvalueindex( 1 ) ) )->Flush();
    lua_pushboolean( L, true );
    return 1;
}

static int luafile_destroy( lua_State *lua )
{
    delete (CFile*)lua_touserdata( lua, lua_upvalueindex( 1 ) );

    return 0;
}

static const luaL_Reg fileInterface[] =
{
    { "__newindex", luafile_onNewindex },
    { "read", luafile_read },
    { "readShort", luafile_readShort },
    { "readInt", luafile_readInt },
    { "readFloat", luafile_readFloat },
    { "write", luafile_write },
    { "writeShort", luafile_writeShort },
    { "writeInt", luafile_writeInt },
    { "writeFloat", luafile_writeFloat },
    { "size", luafile_size },
    { "stat", luafile_stat },
    { "tell", luafile_tell },
    { "seek", luafile_seek },
    { "eof", luafile_eof },
    { "flush", luafile_flush },
    { "destroy", luafile_destroy },
    { NULL, NULL }
};

int luaconstructor_file( lua_State *lua )
{
#ifndef FU_CLASS
    // Register as file
    ILuaClass *j = lua_refclass( lua, 1 );
    j->SetTransmit( LUACLASS_FILE );

    // Create the illegal access table
    lua_newtable( lua );
    lua_pushboolean( lua, false );
    lua_setfield( lua, 2, "__index" );
    lua_pushboolean( lua, false );
    lua_setfield( lua, 2, "__newindex" );

    // We need the class outer environment
    j->PushOuterEnvironment( lua );

    lua_pushcclosure( lua, luafile_onIndex, 2 );
    lua_setfield( lua, LUA_ENVIRONINDEX, "__index" );

    lua_pushvalue( lua, lua_upvalueindex( 1 ) );
    lua_setfield( lua, LUA_ENVIRONINDEX, "ioptr" );

    lua_basicextend( lua );
#endif

    lua_pushvalue( lua, LUA_ENVIRONINDEX );
    lua_pushvalue( lua, lua_upvalueindex( 1 ) );
    luaL_openlib( lua, NULL, fileInterface, 1 );

    lua_pushlstring( lua, "file", 4 );
    lua_setfield( lua, LUA_ENVIRONINDEX, "__type" );
    return 0;
}

void luafile_open( lua_State *lua )
{
}