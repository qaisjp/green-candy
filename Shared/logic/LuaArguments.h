/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*               (Shared logic for modifications)
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        mods/shared_logic/lua/CLuaArguments.h
*  PURPOSE:     Lua arguments manager class header
*  DEVELOPERS:  Ed Lyons <eai@opencoding.net>
*               Jax <>
*               Oliver Brown <>
*               Kevin Whiteside <kevuwk@gmail.com>
*               Christian Myhre Lundheim <>
*               Chris McArthur <>
*               Cecill Etheredge <ijsf@gmx.net>
*               The_GTA <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#ifndef _BASE_LUA_ARGUMENTS_
#define _BASE_LUA_ARGUMENTS_

class LuaArguments
{
public:
                                                        LuaArguments()                { }
                                                        LuaArguments( const LuaArguments& args, std::map <LuaArguments*, LuaArguments*> *knownTables = NULL );
                                                        LuaArguments( NetBitStreamInterface& stream, std::vector <LuaArguments*> *knownTables = NULL );
                                                        ~LuaArguments()                { DeleteArguments (); };

    void                                                CopyRecursive( const LuaArguments& args, std::map <LuaArguments*, LuaArguments*> *knownTables = NULL );

    const CLuaArguments&                                operator = ( const LuaArguments& args );
    CLuaArgument*                                       operator [] ( unsigned int pos ) const;

    void                                                ReadArgument( lua_State *lua, signed int index );
    void                                                ReadArguments( lua_State *lua, signed int indexStart = 1 );
    void                                                PushArguments( lua_State *lua ) const;
    void                                                PushArguments( LuaArguments& args );
    bool                                                Call( class LuaMain *lua, const LuaFunctionRef& ref, LuaArguments *ret = NULL ) const;
    bool                                                CallGlobal( class LuaMain* pLuaMain, const char *funcName, LuaArguments *ret = NULL ) const;

    void                                                ReadTable( lua_State* luaVM, int indexStart, std::map <const void*, LuaArguments*> *knownTables = NULL );
    void                                                PushAsTable( lua_State* luaVM, std::map <CLuaArguments*, int> *knownTables = NULL );

    LuaArgument*                                        PushNil();
    LuaArgument*                                        PushBoolean( bool b );
    LuaArgument*                                        PushNumber( double num );
    LuaArgument*                                        PushString( const std::string& str );
    LuaArgument*                                        PushUserData( void *data );
    LuaArgument*                                        PushArgument( const LuaArgument& argument );
    LuaArgument*                                        PushTable( LuaArguments *table );

    void                                                DeleteArguments();

    bool                                                ReadFromBitStream( NetBitStreamInterface& bitStream, std::vector <LuaArguments*> *knownTables = NULL );
    bool                                                WriteToBitStream( NetBitStreamInterface& bitStream, std::map <LuaArguments*, unsigned long> *knownTables = NULL ) const;
    void                                                ValidateTableKeys();

    unsigned int                                        Count() const          { return static_cast < unsigned int > ( m_args.size() ); };
    std::vector <LuaArgument*> ::const_iterator         IterBegin()                { return m_args.begin(); };
    std::vector <LuaArgument*> ::const_iterator         IterEnd()                { return m_args.end(); };

private:
    std::vector <LuaArgument*>                       m_args;
};

#endif //_BASE_LUA_ARGUMENTS_