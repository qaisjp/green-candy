/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*               (Shared logic for modifications)
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        Shared/logic/LuaArguments.h
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
                                                        LuaArguments();
                                                        LuaArguments( const LuaArguments& args );
                                                        ~LuaArguments();

    void                                                CopyRecursive( const LuaArguments& args );

    const LuaArguments&                                 operator = ( const LuaArguments& args );
    LuaArgument*                                        operator [] ( unsigned int pos ) const;

    void                                                ReadArgument( lua_State *lua, signed int index );
    void                                                ReadArguments( lua_State *lua, signed int indexStart = 1 );
    void                                                PushArguments( lua_State *lua ) const;
    void                                                PushArguments( LuaArguments& args );
    virtual bool                                        Call( class LuaMain *lua, const LuaFunctionRef& ref, LuaArguments *res = NULL );

    void                                                ReadTable( lua_State* luaVM, int indexStart );
    void                                                PushAsTable( lua_State* luaVM );

    bool                                                IsIndexedArray();

    LuaArgument*                                        PushNil();
    LuaArgument*                                        PushBoolean( bool b );
    LuaArgument*                                        PushNumber( double num );
    LuaArgument*                                        PushString( const std::string& str );
    LuaArgument*                                        PushUserData( void *data );
    LuaArgument*                                        PushArgument( const LuaArgument& argument );
    LuaArgument*                                        PushTable( const LuaArguments& table );

    void                                                DeleteArguments();
    void                                                ValidateTableKeys();

    unsigned int                                        Count() const          { return static_cast < unsigned int > ( m_args.size() ); };
    std::vector <LuaArgument*> ::const_iterator         IterBegin()                { return m_args.begin(); };
    std::vector <LuaArgument*> ::const_iterator         IterEnd()                { return m_args.end(); };

protected:
    void                                                SetParent( LuaArguments *parent );

    std::vector <LuaArgument*>          m_args;
    std::vector <LuaArguments*>         m_cachedTables;
    LuaArguments*                       m_parent;   // LuaArguments is a table!
};

#endif //_BASE_LUA_ARGUMENTS_