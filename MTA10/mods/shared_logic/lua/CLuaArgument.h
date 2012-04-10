/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.0
*               (Shared logic for modifications)
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        mods/shared_logic/lua/CLuaArgument.h
*  PURPOSE:     Lua argument class header
*  DEVELOPERS:  Ed Lyons <eai@opencoding.net>
*               Jax <>
*               Oliver Brown <>
*               Kevin Whiteside <kevuwk@gmail.com>
*               Christian Myhre Lundheim <>
*
*****************************************************************************/

#ifndef __CLUAARGUMENT_H
#define __CLUAARGUMENT_H

#include <net/bitstream.h>
#include <string>

class CClientEntity;
class CLuaArguments;

#define LUA_TTABLEREF 9

class CLuaArgument
{
public:
                            CLuaArgument        ( void );
                            CLuaArgument        ( bool bBool );
                            CLuaArgument        ( double dNumber );
                            CLuaArgument        ( const std::string& strString );
                            CLuaArgument        ( void* pUserData );
                            CLuaArgument        ( CClientEntity* pElement );
                            CLuaArgument        ( const CLuaArgument& Argument, std::map < CLuaArguments*, CLuaArguments* > * pKnownTables = NULL );
                            CLuaArgument        ( NetBitStreamInterface& bitStream, std::vector < CLuaArguments* > * pKnownTables = NULL );
                            CLuaArgument        ( lua_State* luaVM, int iArgument, std::map < const void*, CLuaArguments* > * pKnownTables = NULL );
                            ~CLuaArgument       ( void );

    const CLuaArgument&     operator =          ( const CLuaArgument& Argument );
    bool                    operator ==         ( const CLuaArgument& Argument );
    bool                    operator !=         ( const CLuaArgument& Argument );

    void                    Read                ( lua_State* luaVM, int iArgument, std::map < const void*, CLuaArguments* > * pKnownTables = NULL );
    void                    Push                ( lua_State* luaVM, std::map < CLuaArguments*, int > * pKnownTables = NULL ) const;

    void                    Read                ( bool bBool );
    void                    Read                ( double dNumber );
    void                    Read                ( const std::string& strString );
    void                    Read                ( void* pUserData );
    void                    Read                ( CClientEntity* pElement );

    inline int              GetType             ( void ) const      { return m_iType; };

    inline bool             GetBoolean          ( void ) const      { return m_bBoolean; };
    inline lua_Number       GetNumber           ( void ) const      { return m_Number; };
    const char*             GetString           ( void )            { return m_strString.c_str (); };
    inline void*            GetLightUserData    ( void ) const      { return m_pLightUserData; };
    CClientEntity*          GetElement          ( void ) const;

    bool                    ReadFromBitStream   ( NetBitStreamInterface& bitStream, std::vector < CLuaArguments* > * pKnownTables = NULL );
    bool                    WriteToBitStream    ( NetBitStreamInterface& bitStream, std::map < CLuaArguments*, unsigned long > * pKnownTables = NULL ) const;
};

#endif
