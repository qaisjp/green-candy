/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        Shared/logic/lua/LuaManager.h
*  PURPOSE:     Management of Lua hyperstructures
*       Every lua hyperstructure is a table in a global lua environment
*       upkept by the LuaManager entity. LuaManager is supposed to secure
*       the connections between hyperstructures and provide the basic
*       framework for their runtime.
*  DEVELOPERS:  Ed Lyons <>
*               Cecill Etheredge <>
*               Christian Myhre Lundheim <>
*               Oliver Brown <>
*               The_GTA <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#ifndef _BASE_LUA_MANAGER_
#define _BASE_LUA_MANAGER_

#include "LuaCFunctions.h"

enum eLuaTypeEx
{
    LUA_TGLOBALPROXY = LUA_TTHREAD + 1,
    LUA_TELEMENT
};

class LuaManager
{
    friend class LuaMain;
    friend class RegisteredCommands;
public:
                                    LuaManager( RegisteredCommands& commands, Events& events, ScriptDebugging& debug );
                                    ~LuaManager();

    inline ScriptDebugging&         GetDebug()                  { return m_debug; }
    inline int                      GetGlobalEnvironment()      { return LUA_GLOBALSINDEX; }

    // Security inheritances
    virtual int                     AccessGlobal();
    virtual int                     AccessGlobalTable();

    // Current execution information
    const LuaMain*                  GetStatus( int *line = NULL, std::string *src = NULL, std::string *proto_name = NULL );

    void                            Throw( unsigned int id, const char *error );

    // Provide your own creation function
    void                            Init( LuaMain *lua );
    LuaMain*                        Get( lua_State *lua );
    bool                            Remove( LuaMain *lua );

    void                            PushThread( lua_State *thread )     { m_threadStack.push_back( thread ); }
    lua_State*                      GetThread() const                   { return m_threadStack.empty() ? m_lua : *m_threadStack.end(); }
    lua_State*                      PopThread()
    {
        if ( m_threadStack.empty() )
            return NULL;

        lua_State *thread = *m_threadStack.end();

        m_threadStack.pop_back();
        return thread;
    }

    inline std::list <LuaMain*>::const_iterator IterBegin()     { return m_structures.begin(); }
    inline std::list <LuaMain*>::const_iterator IterEnd()       { return m_structures.end(); }

    void                            DoPulse();
    void                            ResetInstructionCount();
    void                            InstructionCountHook( lua_State *L, lua_Debug *ar );

private:
    void                            InitSecurity();

protected:
    void                            LoadCFunctions();
    void                            CallStack( int args );
    void                            CallStackVoid( int args );
    LuaArguments                    CallStackResult( int args );
    bool                            PCallStack( int args );
    bool                            PCallStackVoid( int args );
    LuaArguments                    PCallStackResult( int args, bool& excpt );

    class env_status
    {
        inline void Update( int *line, std::string *src, std::string *proto_name ) const
        {
            lua_Debug info;

            // Gather main information
            if ( lua_getstack( &m_lua, 1, &info ) && lua_getinfo( &m_lua, "nlS", &info ) )
            {
                if ( line )
                    *line = info.currentline;

                if ( src )
                    *src = info.short_src;

                if ( proto_name )
                    *proto_name = info.name;
            }
            else if ( line )
                *line = -1;
        }

    public:
        env_status( lua_State& lua, const LuaMain& context ) : m_lua( lua ), m_env( context )
        {
            m_frozen = false;
        }

        const LuaMain&    Get( int *line, std::string *src, std::string *proto_name ) const
        {
            if ( !m_frozen )
                Update( line, src, proto_name );
            else
            {
                if ( line )
                    *line = m_line;

                if ( src )
                    *src = m_src;

                if ( proto_name )
                    *proto_name = m_proto_name;
            }

            return m_env;
        }

        void    Finalize()
        {
            if ( m_frozen )
                return;

            Update( &m_line, &m_src, &m_proto_name );

            m_frozen = true;
        }

        void    Resume()
        {
            m_frozen = false;
        }
    private:
        int m_line;
        std::string m_src;
        std::string m_proto_name;
        const LuaMain& m_env;

        bool m_frozen;
        lua_State& m_lua;
    };

    // Inline stack utility
    class context
    {
        friend class LuaManager;

        context( LuaMain& context, LuaManager& system ) : m_context( context ), m_system( system )
        {
            m_lua = *m_context;

            system.PushStatus( context );
        }

        LuaMain& m_context;
        LuaManager& m_system;
        lua_State* m_lua;

    public:
        ~context()
        {
            m_system.PopStatus();
        }

        inline void CallStack( int args )
        {
            m_system.CallStack( args );
        }

        inline void CallStackVoid( int args )
        {
            m_system.CallStack( args );
        }

        inline LuaArguments CallStackResult( int args )
        {
            return m_system.CallStackResult( args );
        }

        inline bool PCallStack( int args )
        {
            return m_system.PCallStack( args );
        }

        inline bool PCallStackVoid( int args )
        {
            return m_system.PCallStackVoid( args );
        }

        inline LuaArguments PCallStackResult( int args, bool& excpt )
        {
            return m_system.PCallStackResult( args, excpt );
        }
    };

    friend class context;

    inline context                  AcquireContext( LuaMain& env )  { return context( env, *this ); }
    void                            PushStatus( const LuaMain& main );
    const LuaMain*                  PopStatus( int *line = NULL, std::string *src = NULL, std::string *proto = NULL );
    
    std::list <env_status>          m_envStack;

    lua_State*                      m_lua;
    unsigned long                   m_functionEnter;

    std::list <lua_State*>          m_threadStack;

    class RegisteredCommands&       m_commands;
    class Events&                   m_events;
    ScriptDebugging&                m_debug;
    std::list <LuaMain*>            m_structures;
};

// quick macros
#define lua_getmanager( L ) (lua_rawgeti( L, LUA_REGISTRYINDEX, 1 ))
#define lua_getcontext( L ) (lua_rawgeti( L, LUA_REGISTRYINDEX, 2 ))

#endif //_BASE_LUA_MANAGER_