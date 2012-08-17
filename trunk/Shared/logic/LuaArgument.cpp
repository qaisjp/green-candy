/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        Shared/logic/LuaArgument.cpp
*  PURPOSE:     Lua argument class
*  DEVELOPERS:  Ed Lyons <eai@opencoding.net>
*               Jax <>
*               Oliver Brown <>
*               Kevin Whiteside <kevuwk@gmail.com>
*               Christian Myhre Lundheim <>
*               The_GTA <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#include "StdInc.h"

// Prevent the warning issued when doing unsigned short -> void*
#pragma warning(disable:4312)


LuaArgument::LuaArgument()
{
    m_table = NULL;
    ReadNil();
}

LuaArgument::LuaArgument( bool b )
{
    m_table = NULL;
    Read( b );
}

LuaArgument::LuaArgument( double d )
{
    m_table = NULL;
    Read( d );
}

LuaArgument::LuaArgument( const std::string& str )
{
    m_table = NULL;
    Read( str );
}

LuaArgument::LuaArgument( void *ud )
{
    m_table = NULL;
    ReadUserData( ud );
}

LuaArgument::LuaArgument( const LuaArgument& arg )
{
    // Initialize and call our = on the argument
    m_table = NULL;
    CopyRecursive( arg );
}

#ifndef _KILLFRENZY
LuaArgument::LuaArgument( NetBitStreamInterface& bitStream )
{
    m_table = NULL;
    ReadFromBitStream( bitStream );
}
#endif //_KILLFRENZY

LuaArgument::LuaArgument( lua_State *lua, int idx )
{
    // Read the argument out of the lua VM
    m_table = NULL;
    Read( lua, idx );
}

LuaArgument::~LuaArgument()
{
    // Eventually destroy our table
    DeleteTableData();
}

void LuaArgument::CopyRecursive( const LuaArgument& arg )
{
    // Clear the string
    m_string.clear();

    // Destroy our old tabledata if neccessary
    DeleteTableData();

    // Copy over line and filename too
    m_path = arg.m_path;
    m_line = arg.m_line;

    // Set our variable equally to the copy class
    m_type = arg.m_type;

    switch( m_type )
    {
    case LUA_TBOOLEAN:
        Read( arg.GetBoolean() );
        return;

    case LUA_TLIGHTUSERDATA:
        ReadUserData( arg.GetLightUserData() );
        return;

    case LUA_TNUMBER:
        Read( arg.GetNumber() );
        return;

    case LUA_TTABLE:
#ifdef _TODO
        if ( pKnownTables && pKnownTables->find ( Argument.m_pTableData ) != pKnownTables->end () )
        {
            m_pTableData = pKnownTables->find ( Argument.m_pTableData )->second;
            m_bWeakTableRef = true;
        }
        else
        {
            m_pTableData = new CLuaArguments ( *Argument.m_pTableData, pKnownTables );
            m_bWeakTableRef = false;
        }
#endif //_TODO
        return;

    case LUA_TSTRING:
        Read( arg.m_string );
        return;
    }

    ReadNil();
}

const LuaArgument& LuaArgument::operator = ( const LuaArgument& arg )
{
    CopyRecursive( arg );

    // Return the given class allowing for chaining
    return arg;
}

bool LuaArgument::operator == ( const LuaArgument& arg )
{
    return CompareRecursive( arg );
}

bool LuaArgument::operator != ( const LuaArgument& arg )
{
    return !CompareRecursive( arg );
}

bool LuaArgument::CompareRecursive( const LuaArgument& arg )
{
    // If the types differ, they're not matching
    if ( arg.m_type == m_type )
        return false;

    // Compare the variables depending on the type
    switch( m_type )
    {
    case LUA_TBOOLEAN:
        return m_bool == arg.GetBoolean();

    case LUA_TLIGHTUSERDATA:
        return m_lightUD == arg.GetLightUserData();

    case LUA_TNUMBER:
        return m_num == arg.GetNumber();

    case LUA_TTABLE:
#ifdef _TODO
        if ( m_pTableData->Count () != Argument.m_pTableData->Count () )
            return false;

        vector < CLuaArgument * > ::const_iterator iter = m_pTableData->IterBegin ();
        vector < CLuaArgument * > ::const_iterator iterCompare = Argument.m_pTableData->IterBegin ();
        while ( iter != m_pTableData->IterEnd () && iterCompare != Argument.m_pTableData->IterEnd () )
        {
            if ( pKnownTables->find ( m_pTableData ) == pKnownTables->end () )
            {
                pKnownTables->insert ( m_pTableData );
                if ( *iter != *iterCompare )
                    return false;
            }
        
            iter++;
            iterCompare++;
        }
        return true;
#endif //_TODO
        return false;
    case LUA_TSTRING:
        return m_string == arg.m_string;
    }

    return false;
}

void LuaArgument::Read( lua_State *lua, int idx )
{
    // Store debug data for later retrieval
    lua_Debug debugInfo;

    if ( lua_getstack( lua, 1, &debugInfo ) )
    {
        lua_getinfo( lua, "nlS", &debugInfo );

        m_path = debugInfo.source;
        m_line = debugInfo.currentline;
    }
    else
    {
        m_path.clear();
        m_line = 0;
    }

    // Eventually delete our previous string
    m_string.clear();

    DeleteTableData();

    // Read out the content depending on the type
    switch( lua_type( lua, idx ) )
    {
    case LUA_TNONE:
    case LUA_TNIL:
        ReadNil();
        return;

    case LUA_TBOOLEAN:
        Read( lua_toboolean( lua, idx ) ? true : false );
        return;

    case LUA_TTABLE:
#ifdef _TODO
        const void* pTable = lua_topointer ( luaVM, iArgument );
        if ( pKnownTables && pKnownTables->find ( pTable ) != pKnownTables->end () )
        {
            m_pTableData = pKnownTables->find ( pTable )->second;
            m_bWeakTableRef = true;
        }
        else
        {
            m_pTableData = new CLuaArguments ();
            m_pTableData->ReadTable ( luaVM, iArgument, pKnownTables );
            m_bWeakTableRef = false;
        }
#endif //_TODO
        return;

    case LUA_TLIGHTUSERDATA:
        ReadUserData( lua_touserdata( lua, idx ) );
        return;

    case LUA_TCLASS:
        {
            LuaElement *elem = lua_readclass <LuaElement> ( lua, idx, LUACLASS_ELEMENT );

            if ( elem )
                ReadUserData( elem );
            else
                Read( false );
        }
        return;

    case LUA_TNUMBER:
        Read( lua_tonumber( lua, idx ) );
        return;

    case LUA_TSTRING:
        Read( lua_getstring( lua, idx ) );
        return;

    case LUA_TFUNCTION:
        // TODO: add function reading (has to work inside tables too)
        ReadNil();
        return;
    }

    ReadNil();
}

void LuaArgument::ReadNil()
{
    m_string = std::string( "nil", 3 );
    m_type = LUA_TNIL;
    DeleteTableData();
}

void LuaArgument::Read( bool b )
{
    m_string = b ? "true" : "false";
    m_type = LUA_TBOOLEAN;
    DeleteTableData();
    m_bool = b;
}

void LuaArgument::Read( double d )
{
    std::stringstream stream;
    stream.precision( 16 );

    stream << d;

    m_string = stream.str();
    m_type = LUA_TNUMBER;
    DeleteTableData();
    m_num = d;
}

void LuaArgument::Read( const std::string& string )
{
    m_type = LUA_TSTRING;
    DeleteTableData();
    m_string = string;
}

void LuaArgument::ReadUserData( void *ud )
{
    m_string = "";
    DeleteTableData();
    m_type = LUA_TLIGHTUSERDATA;
    m_lightUD = ud;
}

void LuaArgument::Push( lua_State *lua ) const
{
    LUA_CHECKSTACK( lua, 1 );

    switch( m_type )
    {
    case LUA_TBOOLEAN:
        lua_pushboolean( lua, m_bool );
        return;

    case LUA_TLIGHTUSERDATA:
        lua_pushlightuserdata( lua, m_lightUD );
        return;

    case LUA_TNUMBER:
        lua_pushnumber( lua, m_num );
        return;

    case LUA_TTABLE:
#ifdef _TODO
        if ( pKnownTables && pKnownTables->find ( m_pTableData ) != pKnownTables->end () )
        {
            lua_getfield ( luaVM, LUA_REGISTRYINDEX, "cache" );
			lua_pushnumber ( luaVM, pKnownTables->find ( m_pTableData )->second );
			lua_gettable ( luaVM, -2 );
			lua_remove ( luaVM, -2 );
        }
        else
        {
            m_pTableData->PushAsTable ( luaVM, pKnownTables );
        }
#endif //_TODO
        return;

    case LUA_TSTRING:
        lua_pushlstring( lua, m_string.c_str(), m_string.size() );
        return;
    }

    lua_pushnil( lua );
}

#ifndef _KILLFRENZY
bool LuaArgument::ReadTypeFromBitStream( NetBitStreamInterface& stream, int type )
{
    switch( type )
    {
    case LUA_TNONE:
    case LUA_TNIL:
        ReadNil();
        return true;

    case LUA_TBOOLEAN:
        Read( stream.ReadBit() );
        return true;

    case LUA_TNUMBER:
        if ( stream.ReadBit() )
        {
            float fNum;

            if ( stream.Read( fNum ) )
                Read( (double)fNum );

            return true;
        }

        long lNum;

        if ( stream.ReadCompressed( lNum ) )
            Read( (double)lNum );

        return true;

    case LUA_TTABLE:
#ifdef _TODO
        m_pTableData = new CLuaArguments( stream, pKnownTables );
        m_bWeakTableRef = false;
        m_iType = LUA_TTABLE;
        m_pTableData->ValidateTableKeys();
        return true;
#endif //_TODO
        return false;

    // Table reference (self-referencing tables)
    case LUA_TTABLEREF:
#ifdef _TODO
        unsigned long ulTableRef;
        if ( bitStream.ReadCompressed ( ulTableRef ) )
        {
            if ( pKnownTables && ulTableRef < pKnownTables->size() )
            {
                m_pTableData = pKnownTables->at( ulTableRef );
                m_bWeakTableRef = true;
                m_iType = LUA_TTABLE;
            }
        }
        return true;
#endif //_TODO
        return false;

    case LUA_TSTRING:
        // Read out the string length
        std::string buf;

        if ( !stream.ReadStringCompressed( buf ) )
        {
            Read( std::string() );
            return true;
        }

        Read( buf );
        return true;
    }

    return false;
}

bool LuaArgument::ReadFromBitStream( NetBitStreamInterface& bitStream )
{
    DeleteTableData();

    SLuaTypeSync type;

    // Read out the type
    if ( !bitStream.Read( &type ) )
        return false;

    return ReadTypeFromBitStream( bitStream, type.data.ucType );
}

bool LuaArgument::WriteToBitStream( NetBitStreamInterface& bitStream ) const
{
    SLuaTypeSync type;

    switch( m_type )
    {
    case LUA_TNIL:
        type.data.ucType = LUA_TNIL;
        bitStream.Write( &type );
        return true;

    case LUA_TBOOLEAN:
        type.data.ucType = LUA_TBOOLEAN;
        bitStream.Write( &type );
        bitStream.WriteBit( GetBoolean() );
        return true;

    case LUA_TTABLE:
#ifdef _TODO
        if ( pKnownTables && pKnownTables->find ( m_pTableData ) != pKnownTables->end () )
        {
            // Self-referencing table
            type.data.ucType = LUA_TTABLEREF;
            bitStream.Write ( &type );
            bitStream.WriteCompressed ( pKnownTables->find ( m_pTableData )->second );
        }
        else
        {
            type.data.ucType = LUA_TTABLE;
            bitStream.Write ( &type );

            // Write the subtable to the bitstream
            m_pTableData->WriteToBitStream ( bitStream, pKnownTables );
        }
#endif //_TODO
        return true;

    case LUA_TNUMBER:
        {
            type.data.ucType = LUA_TNUMBER;
            bitStream.Write( &type );

            float fNumber = (float)GetNumber();
            long lNumber = (long)fNumber;
            float fNumberInteger = (float)lNumber;

            // Check if the number is an integer and can fit a long datatype
            if ( fabs(fNumber) > fabs(fNumberInteger + 1) || fabs(fNumber - fNumberInteger) >= FLOAT_EPSILON )
            {
                bitStream.WriteBit( true );
                bitStream.Write( fNumber );
                return true;
            }

            bitStream.WriteBit( false );
            bitStream.WriteCompressed( lNumber );
        }
        return true;

    case LUA_TSTRING:
        {
            size_t len = m_string.size();

            if ( len > 65535 )
            {
                // Too long string
                LogUnableToPacketize( "Couldn't packetize argument list. Invalid string specified, limit is 65535 characters." );

                // Write a nil though so other side won't get out of sync
                bitStream.Write( (unsigned char)LUA_TNIL );
                return false;
            }

            type.data.ucType = LUA_TSTRING;
            bitStream.Write( &type );
        }
        bitStream.WriteStringCompressed( m_string );
        return true;
    }
    LogUnableToPacketize( "Couldn't packetize argument list, unknown type specified." );

    // Write a nil though so other side won't get out of sync
    type.data.ucType = LUA_TNIL;
    bitStream.Write( &type );
    return false;
}
#endif //_KILLFRENZY

void LuaArgument::DeleteTableData()
{
    if ( !m_table )
        return;

    if ( !m_weakRef )
        delete m_table;

    m_table = NULL;
}
