/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        Shared/logic/LuaMain.h
*  PURPOSE:     Lua virtual machine extension
*  DEVELOPERS:  The_GTA <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#ifndef _BASE_LUA_MAIN_
#define _BASE_LUA_MAIN_

#include "CLuaTimerManager.h"

#include "CLuaFunctionDefs.h"

#include <xml/CXMLFile.h>

#define MAX_SCRIPTNAME_LENGTH 64

#include <list>

struct CRefInfo
{
    unsigned long ulUseCount;
    int iFunction;
};

class LuaMain
{
public:
    enum
    {
        OWNER_SERVER,
        OWNER_MAP,
    };

public:
                                    LuaMain( class CLuaManager* pLuaManager, CResource* pResourceOwner );
                                    ~LuaMain();

    bool                            BeingDeleted();

    bool                            LoadScriptFromFile( const char* szLUAScript );
    bool                            LoadScriptFromBuffer( const char* cpBuffer, unsigned int uiSize, const char* szFileName, bool bUTF8 );
    bool                            LoadScript( const char* szLUAScript );
    void                            UnloadScript();

    void                            DoPulse();

    const std::string&              GetName() const                     { return m_name; };
    void                            SetName( const char *name )         { m_name = name; };

    inline CLuaTimerManager*        GetTimerManager() const                  { return m_pLuaTimerManager; };
    inline lua_State*               GetVirtualMachine() const                  { return m_luaVM; };

    void                            ResetInstructionCount();

    inline class CResource*         GetResource()                        { return m_pResource; }

    virtual CXMLFile*               CreateXML( const char *path ) = 0;
    virtual void                    DestroyXML( CXMLFile *file ) = 0;
    virtual void                    DestroyXML( CXMLNode *root ) = 0;
    virtual void                    SaveXML( CXMLNode *root ) = 0;
    unsigned long                   GetXMLFileCount() const                  { return m_XMLFiles.size(); };
    unsigned long                   GetTimerCount() const                  { return m_pLuaTimerManager ? m_pLuaTimerManager->GetTimerCount() : 0; };

    void                            InitVM();
    const SString&                  GetFunctionTag( int ref );

private:
    void                            InitSecurity();

    std::string                     m_name;
    int                             m_iOwner;

    lua_State*                      m_lua;
    class LuaManager*               m_system;
    LuaTimerManager*               m_pLuaTimerManager;

    bool                            m_bBeingDeleted; // prevent it being deleted twice

    unsigned long                   m_ulFunctionEnterTime;

    class CResource*                m_pResource;

    std::list <CXMLFile*>           m_XMLFiles;
public:
    std::map <const void*, CRefInfo>    m_CallbackTable;
    std::map <int, SString>             m_FunctionTagMap;
};

#endif //_BASE_LUA_MAIN_