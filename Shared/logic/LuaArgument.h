/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        Shared/logic/LuaArgument.h
*  PURPOSE:     Lua argument class header
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

#ifndef _BASE_LUA_ARGUMENT_
#define _BASE_LUA_ARGUMENT_

class LuaArguments;

class LuaArgument
{
    friend class LuaArguments;
public:
                            CLuaArgument();
                            CLuaArgument( bool bBool );
                            CLuaArgument( double dNumber );
                            CLuaArgument( const std::string& strString );
                            CLuaArgument( void* pUserData );
                            CLuaArgument( const CLuaArgument& arg );
                            CLuaArgument( NetBitStreamInterface& bitStream );
                            CLuaArgument( lua_State* luaVM, int iArgument );
                            ~CLuaArgument();

    const CLuaArgument&     operator = ( const CLuaArgument& Argument );
    bool                    operator == ( const CLuaArgument& Argument );
    bool                    operator != ( const CLuaArgument& Argument );

    void                    Read( lua_State* luaVM, int iArgument );
    void                    Read( bool bBool );
    void                    Read( double dNumber );
    void                    Read( const std::string& strString );
    void                    Read( void* pUserData );
    void                    Read( CClientEntity* pElement );

    void                    Push( lua_State* luaVM ) const;

    void                    ReadElementID( ElementID ID );

    inline int              GetType() const      { return m_iType; };

    inline bool             GetBoolean() const      { return m_bBoolean; };
    inline lua_Number       GetNumber() const      { return m_Number; };
    const char*             GetString()            { return m_strString.c_str (); };
    inline void*            GetLightUserData() const      { return m_pLightUserData; };

    bool                    ReadFromBitStream( NetBitStreamInterface& bitStream );
    bool                    WriteToBitStream( NetBitStreamInterface& bitStream ) const;

private:
    void                    LogUnableToPacketize( const char* szMessage ) const;
    void                    CopyRecursive( const CLuaArgument& Argument );
    bool                    CompareRecursive( const CLuaArgument& Argument );
    void                    DeleteTableData();

    LuaArguments*           m_table;

    int                     m_iType;
    bool                    m_bBoolean;
    lua_Number              m_Number;
    std::string             m_strString;
    void*                   m_pLightUserData;
    bool                    m_bWeakTableRef;

    filePath                m_path;
    int                     m_iLine;
};

#endif //_BASE_LUA_ARGUMENT_