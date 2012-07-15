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

#if MTA_DEBUG
    // Tight allocation in debug to find trouble.
    #define LUA_CHECKSTACK(vm,space) lua_checkstack(vm, (space) )
#else
    // Extra room in release to avoid trouble.
    #define LUA_CHECKSTACK(vm,space) lua_checkstack(vm, (space)*2 )
#endif


class LuaArguments
{
public:
                                                        LuaArguments();
                                                        LuaArguments( const LuaArguments& args );
    virtual                                             ~LuaArguments();

    void                                                CopyRecursive( const LuaArguments& args );

    const LuaArguments&                                 operator = ( const LuaArguments& args );
    LuaArgument*                                        operator [] ( unsigned int pos ) const;

    virtual void                                        ReadArgument( lua_State *lua, signed int index ) = 0;
    void                                                ReadArguments( lua_State *lua, signed int indexStart = 1 );
    void                                                PushArguments( lua_State *lua ) const;
    void                                                PushArguments( LuaArguments& args );
    virtual bool                                        Call( class LuaMain *lua, const LuaFunctionRef& ref, LuaArguments *res = NULL ) const;

    void                                                ReadTable( lua_State *L, int indexStart );
    void                                                PushAsTable( lua_State *L );

    bool                                                IsIndexedArray();

    virtual LuaArgument*                                PushNil() = 0;
    virtual LuaArgument*                                PushBoolean( bool b ) = 0;
    virtual LuaArgument*                                PushNumber( double num ) = 0;
    virtual LuaArgument*                                PushString( const std::string& str ) = 0;
    virtual LuaArgument*                                PushUserData( void *data ) = 0;
    virtual LuaArgument*                                PushArgument( const LuaArgument& argument ) = 0;
    virtual LuaArgument*                                PushTable( const LuaArguments& table ) = 0;

    void                                                DeleteArguments();
    void                                                ValidateTableKeys();

    unsigned int                                        Count() const          { return static_cast < unsigned int > ( m_args.size() ); };
    std::vector <LuaArgument*> ::const_iterator         IterBegin()                { return m_args.begin(); };
    std::vector <LuaArgument*> ::const_iterator         IterEnd()                { return m_args.end(); };

#ifndef _KILLFRENZY
    bool                                                WriteToBitStream( NetBitStreamInterface& bitStream ) const;
#endif

protected:
    void                                                SetParent( LuaArguments *parent );

    typedef std::vector <LuaArgument*> argList_t;

    argList_t                           m_args;
    std::vector <LuaArguments*>         m_cachedTables;
    LuaArguments*                       m_parent;   // LuaArguments is a table!
};

#endif //_BASE_LUA_ARGUMENTS_