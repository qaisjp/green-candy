/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.0
*               (Shared logic for modifications)
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        mods/shared_logic/lua/CLuaArgument.cpp
*  PURPOSE:     Lua argument class
*  DEVELOPERS:  Ed Lyons <eai@opencoding.net>
*               Jax <>
*               Oliver Brown <>
*               Kevin Whiteside <kevuwk@gmail.com>
*               Christian Myhre Lundheim <>
*               The_GTA <quiret@gmx.de>
*
*****************************************************************************/

#include "StdInc.h"
#include "net/SyncStructures.h"

extern CClientGame* g_pClientGame;

// Prevent the warning issued when doing unsigned short -> void*
#pragma warning(disable:4312)

CLuaArgument::CLuaArgument( CClientEntity *element )
{
    m_table = NULL;
    ReadEntity( element );
}

CLuaArgument::~CLuaArgument()
{
}

LuaArgument* CLuaArgument::Clone() const
{
    // TODO: improve if we want to use this class
    return new CLuaArgument( *this );
}

void CLuaArgument::ReadEntity( CClientEntity *element )
{
    DeleteTableData();

    if ( !element )
    {   
        ReadNil();
        return;
    }

    m_string.clear();

    m_type = LUA_TLIGHTUSERDATA;
    m_lightUD = (void*)element->GetID().Value();
}

CClientEntity* CLuaArgument::GetElement() const
{
    ElementID ID = TO_ELEMENTID( m_lightUD );
    return CElementIDs::GetElement( ID );
}

bool CLuaArgument::ReadTypeFromBitStream( NetBitStreamInterface& stream, int type )
{
    switch( type )
    {
    case LUA_TTABLE:
        m_table = new CLuaArguments( stream );
        m_table->AddCachedTable( m_table );
        m_type = LUA_TTABLE;
        m_table->ValidateTableKeys();
        return true;

    // Element type
    case LUA_TLIGHTUSERDATA:
        ElementID id;

        if ( !stream.Read( id ) )
            return false;

        ReadEntity( CElementIDs::GetElement( id ) );
        return true;
    }
    return LuaArgument::ReadTypeFromBitStream( stream, type );
}

bool CLuaArgument::WriteToBitStream( NetBitStreamInterface& stream, argRep_t *cached ) const
{
    SLuaTypeSync type;

    switch( m_type )
    {
    // Element packet
    case LUA_TLIGHTUSERDATA:
        {
            CClientEntity *element = (CClientEntity*)m_lightUD;

            if ( !element )
            {
                // Write a nil though so other side won't get out of sync
                type.data.ucType = LUA_TNIL;
                stream.Write( &type );
                return false;
            }

            // Clientside element?
            if ( element->IsLocalEntity() )
            {
                // Write a nil though so other side won't get out of sync
                type.data.ucType = LUA_TNIL;
                stream.Write( &type );
                return false;
            }

            type.data.ucType = LUA_TLIGHTUSERDATA;
            stream.Write( &type );
            stream.Write( element->GetID() );
        }
        return true;
    case LUA_TCLASS:
        assert( 0 );
    }

    return LuaArgument::WriteToBitStream( stream, cached );
}

void CLuaArgument::LogUnableToPacketize( const char *msg ) const
{
    if ( m_path.empty() )
    {
        g_pClientGame->GetScriptDebugging()->LogWarning( "Unknown: %s\n", msg );
        return;
    }

    g_pClientGame->GetScriptDebugging()->LogWarning( "%s:%d: %s\n", m_path.c_str(), m_line, msg );
}