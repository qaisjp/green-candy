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
    friend class LuaArgument;
public:
                                                        LuaArguments();
                                                        LuaArguments( const LuaArguments& args );
    virtual                                             ~LuaArguments();

    void                                                CopyRecursive( const LuaArguments& args );

    const LuaArguments&                                 operator = ( const LuaArguments& args );
    LuaArgument*                                        operator [] ( unsigned int pos ) const;

    virtual void                                        ReadArgument( lua_State *lua, signed int index, luaArgRep_t *cached ) = 0;
    void                                                ReadArguments( lua_State *lua, signed int indexStart = 1 );
    void                                                PushArguments( lua_State *lua ) const;
    void                                                PushArguments( LuaArguments& args );
    virtual bool                                        Call( class LuaMain *lua, const LuaFunctionRef& ref, LuaArguments *res = NULL ) const;

    void                                                ReadTable( lua_State *L, int indexStart, luaArgRep_t *cached );
    void                                                PushAsTable( lua_State *L );

    bool                                                IsIndexedArray();

    virtual LuaArgument*                                PushNil() = 0;
    virtual LuaArgument*                                PushBoolean( bool b ) = 0;
    virtual LuaArgument*                                PushNumber( double num ) = 0;
    virtual LuaArgument*                                PushString( const std::string& str ) = 0;
    virtual LuaArgument*                                PushTableRef( unsigned int idx ) = 0;
    virtual LuaArgument*                                PushUserData( void *data ) = 0;
    virtual LuaArgument*                                PushArgument( const LuaArgument& argument ) = 0;
#if 0
    virtual LuaArgument*                                PushTable( const LuaArguments& table ) = 0;
#endif

    unsigned int                                        AddCachedTable( LuaArguments *table );
    LuaArguments*                                       GetCachedTable( unsigned int idx );

    void                                                DeleteArguments();
    void                                                ValidateTableKeys();

    typedef std::vector <LuaArgument*> argList_t;

    unsigned int                                        Count() const               { return m_args.size(); };
    argList_t::const_iterator                           IterBegin() const           { return m_args.begin(); };
    argList_t::const_iterator                           IterEnd() const             { return m_args.end(); };

#ifndef _KILLFRENZY
    bool                                                WriteToBitStream( NetBitStreamInterface& bitStream, argRep_t *cached = NULL ) const;
#endif

protected:
    typedef std::vector <LuaArguments*> cached_t;

    argList_t                           m_args;
    cached_t                            m_cachedTables;

    LuaArguments*                       m_parent;   // LuaArguments is a table!
    unsigned int                        m_cachedID; // registered index in the root
};

#endif //_BASE_LUA_ARGUMENTS_