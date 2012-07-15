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

class Resource;

class LuaMain abstract
{
    friend class LuaManager;
    friend class Resource;
protected:

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
    bool                            GetReference( int stack, LuaFunctionRef& ref );
    void                            Reference( const LuaFunctionRef& ref );
    void                            Dereference( const LuaFunctionRef& ref );
    void                            PushReference( const LuaFunctionRef& ref );

    virtual void                    RegisterFunction( const char *name, lua_CFunction proto );

    void							CallStack( int args );
    void							CallStackVoid( int args );
    void					        CallStackResult( int argc, LuaArguments& args );
    bool                            PCallStack( int args );
    bool                            PCallStackVoid( int args );
    bool                            PCallStackResult( int argc, LuaArguments& args );
    //TODO: Function reference calling

    bool                            LoadScriptFromBuffer( const char *buf, size_t size, const char *path, bool utf8 );
    bool                            LoadScript( const char *buf );

    void                            DoPulse();

    const std::string&              GetName() const                     { return m_name; };
    void                            SetName( const std::string& name )  { m_name = name; };

    inline LuaTimerManager&         GetTimerManager()                   { return m_timers; };
    inline lua_State*               GetVirtualMachine() const           { return m_lua; };
    inline Resource*                GetResource()                       { return m_resource; }

    inline lua_State*               operator * () const                 { return GetVirtualMachine(); }

    virtual bool                    ParseRelative( const char *in, filePath& out ) const = 0;

#ifndef _KILLFRENZY
    virtual CXMLFile*               CreateXML( const char *path ) = 0;
    virtual void                    SaveXML( CXMLNode *root ) = 0;
    void                            DestroyXML( CXMLFile *file );
    void                            DestroyXML( CXMLNode *root );
    unsigned long                   GetXMLFileCount() const             { return m_XMLFiles.size(); };
    unsigned long                   GetTimerCount() const               { return m_timers.GetTimerCount(); };
#endif //_KILLFRENZY

    const SString&                  GetFunctionTag( int ref );

private:
    void                            InitSecurity();

protected:
    virtual void                    InitVM( int structure, int meta );

    std::string                     m_name;
    int                             m_global;
    int                             m_structure;
    int                             m_meta;

    Resource*                       m_resource;

    lua_State*                      m_lua;
    class LuaManager&               m_system;
    LuaTimerManager                 m_timers;

#ifndef _KILLFRENZY
    typedef std::list <CXMLFile*> xmlFiles_t;

    xmlFiles_t                      m_XMLFiles;
#endif //_KILLFRENZY

public:
    std::map <const void*, CRefInfo>    m_refStore;
    std::map <int, SString>             m_tagStore;
};

#endif //_BASE_LUA_MAIN_