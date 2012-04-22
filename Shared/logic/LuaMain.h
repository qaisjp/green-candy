/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        Shared/logic/LuaMain.h
*  PURPOSE:     Lua hyperstructure
*  DEVELOPERS:  The_GTA <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#ifndef _BASE_LUA_MAIN_
#define _BASE_LUA_MAIN_

#include <xml/CXMLFile.h>

struct CRefInfo
{
    unsigned long refCount;
    int idx;
};

class LuaMain
{
    friend class LuaManager;

    // Manager only
                                    LuaMain( class LuaManager& manager );
    virtual                         ~LuaMain();

public:
    enum
    {
        OWNER_SERVER,
        OWNER_MAP
    };

    LuaFunctionRef                  CreateReference( int stack );
    void                            Reference( const LuaFunctionRef& ref );
    void                            Dereference( const LuaFunctionRef& ref );
    void                            PushReference( const LuaFunctionRef& ref );

    virtual void                    RegisterFunction( const char *name, lua_CFunction *proto );

    inline void                     CallStack( int args )           { m_system.AcquireContext( *this ).CallStack( args ); }
    inline void                     CallStackVoid( int args )       { m_system.AcquireContext( *this ).CallStackVoid( args ); }
    inline LuaArguments             CallStackResult( int args )     { return m_system.AcquireContext( *this ).CallStackResult( args ); }
    bool                            PCallStack( int args );
    bool                            PCallStackVoid( int args );
    LuaArguments                    PCallStackResult( int args, bool& excpt );
    //TODO: Function reference calling

    bool                            LoadScriptFromBuffer( const char *buf, size_t size, const char *path, bool bUTF8 );
    bool                            LoadScript( const char *buf );

    void                            DoPulse();

    const std::string&              GetName() const                     { return m_name; };
    void                            SetName( const char *name )         { m_name = name; };

    inline LuaTimerManager&         GetTimerManager() const                  { return m_timers; };
    inline lua_State*               GetVirtualMachine() const                  { return m_lua; };

    inline lua_State*               operator * () const                 { return GetVirtualMachine(); }

    virtual bool                    ParseRelative( const char *in, filePath& out ) = 0;

    virtual CXMLFile*               CreateXML( const char *path ) = 0;
    virtual void                    SaveXML( CXMLNode *root ) = 0;
    void                            DestroyXML( CXMLFile *file );
    void                            DestroyXML( CXMLNode *root );
    unsigned long                   GetXMLFileCount() const                  { return m_XMLFiles.size(); };
    unsigned long                   GetTimerCount() const                  { return m_timers.GetTimerCount(); };

    const SString&                  GetFunctionTag( int ref ) const;

private:
    void                            InitSecurity();

protected:
    void                            InitVM( int structure, int meta );

    std::string                     m_name;
    int                             m_global;
    int                             m_structure;
    int                             m_meta;

    lua_State*                      m_lua;
    class LuaManager&               m_system;
    LuaTimerManager                 m_timers;

    unsigned long                   m_functionEnter;

    std::list <CXMLFile*>           m_XMLFiles;
public:
    std::map <const void*, CRefInfo>    m_refStore;
    std::map <int, SString>             m_tagStore;
};

#endif //_BASE_LUA_MAIN_