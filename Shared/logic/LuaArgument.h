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

class LuaArgument abstract
{
    friend class LuaArguments;
public:
                                LuaArgument();
                                LuaArgument( bool b );
                                LuaArgument( double d );
                                LuaArgument( const std::string& str );
                                LuaArgument( void *ud );
                                LuaArgument( const LuaArgument& arg );
#ifndef _KILLFRENZY
                                LuaArgument( NetBitStreamInterface& bitStream );
#endif //_KILLFRENZY
                                LuaArgument( lua_State *lua, int idx );
    virtual                     ~LuaArgument();

    const LuaArgument&          operator = ( const LuaArgument& arg );
    bool                        operator == ( const LuaArgument& arg );
    bool                        operator != ( const LuaArgument& arg );

    virtual LuaArgument*        Clone() const = 0;

    void                        ReadNil();
    void                        Read( lua_State *lua, int idx );
    void                        Read( bool b );
    void                        Read( double d );
    void                        Read( const std::string& str );
    void                        Read( const class LuaArguments *table );
    void                        ReadUserData( void *ud );

    void                        Push( lua_State* lua ) const;

    inline int                  GetType() const      { return m_type; };

    inline bool                 GetBoolean() const      { return m_bool; };
    inline lua_Number           GetNumber() const      { return m_num; };
    inline const std::string&   GetString() const       { return m_string; }
    inline void*                GetLightUserData() const      { return m_lightUD; };

#ifndef _KILLFRENZY
    bool                        ReadFromBitStream( NetBitStreamInterface& bitStream );
    virtual bool                WriteToBitStream( NetBitStreamInterface& bitStream ) const;
#endif //_KILLFRENZY

protected:
#ifndef _KILLFRENZY
    virtual bool                ReadTypeFromBitStream( NetBitStreamInterface& stream, int type );
#endif //_KILLFRENZY

    virtual void                LogUnableToPacketize( const char *msg ) const = 0;
    void                        CopyRecursive( const LuaArgument& arg );
    bool                        CompareRecursive( const LuaArgument& arg );
    void                        DeleteTableData();

    LuaArguments*               m_table;

    int                         m_type;
    bool                        m_bool;
    lua_Number                  m_num;
    std::string                 m_string;
    void*                       m_lightUD;
    bool                        m_weakRef;

    filePath                    m_path;
    int                         m_line;
};

#endif //_BASE_LUA_ARGUMENT_