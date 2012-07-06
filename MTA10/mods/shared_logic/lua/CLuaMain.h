/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.0
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        mods/shared_logic/lua/CLuaMain.h
*  PURPOSE:     Lua virtual machine container class
*  DEVELOPERS:  Ed Lyons <eai@opencoding.net>
*               Christian Myhre Lundheim <>
*               Cecill Etheredge <ijsf@gmx.net>
*               Jax <>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

class CLuaMain;

#ifndef __CLUAMAIN_H
#define __CLUAMAIN_H

#include "CLuaTimerManager.h"
#include "CLuaFunctionDefs.h"

#include <list>

class CLuaMain
{
public:
    enum
    {
        OWNER_SERVER,
        OWNER_MAP,
    };

public:
                                    CLuaMain                ( class CLuaManager* pLuaManager, CResource* pResourceOwner /*,
                                                              CObjectManager* pObjectManager,
                                                              CPlayerManager* pPlayerManager,
                                                              CVehicleManager* pVehicleManager,
                                                              CBlipManager* pBlipManager,
                                                              CRadarAreaManager* pRadarAreaManager,
                                                              CMapManager* pMapManager*/ );
                                    ~CLuaMain               ();

    inline int                      GetOwner                ()                        { return m_iOwner; };
    inline void                     SetOwner                ( int iOwner )                  { m_iOwner = iOwner; };

    bool                            LoadScriptFromFile      ( const char* szLUAScript );
    bool                            LoadScriptFromBuffer    ( const char* cpBuffer, unsigned int uiSize, const char* szFileName, bool bUTF8 );
    bool                            LoadScript              ( const char* szLUAScript );
    void                            UnloadScript            ();

    void                            Start                   ();

    void                            DoPulse                 ();

    void                            GetScriptName           ( char* szLuaScript ) const     { strcpy ( szLuaScript, m_szScriptName ); };
    inline const char*              GetScriptNamePointer    () const                  { return m_szScriptName; };
    void                            SetScriptName           ( const char* szName )          { strncpy ( m_szScriptName, szName, MAX_SCRIPTNAME_LENGTH ); };

    void                            RegisterFunction        ( const char* szFunction, lua_CFunction function );

    inline lua_State*               GetVM                   ()                        { return m_luaVM; };
    inline CLuaTimerManager*        GetTimerManager         () const                  { return m_pLuaTimerManager; };

    bool                            BeingDeleted            ();
    inline lua_State *              GetVirtualMachine       () const                  { return m_luaVM; };

    void                            ResetInstructionCount   ();

    inline class CResource*         GetResource             ()                        { return m_pResource; }

    CXMLFile *                      CreateXML               ( const char* szFilename );
    void                            DestroyXML              ( CXMLFile* pFile );
    void                            DestroyXML              ( CXMLNode* pRootNode );
    void                            SaveXML                 ( CXMLNode * pRootNode );
    bool                            XMLExists               ( CXMLFile* pFile );
    unsigned long                   GetXMLFileCount         () const                  { return m_XMLFiles.size (); };
    unsigned long                   GetTimerCount           () const                  { return m_pLuaTimerManager ? m_pLuaTimerManager->GetTimerCount () : 0; };
    unsigned long                   GetElementCount         () const;

    void                            InitVM                  ();
    const SString&                  GetFunctionTag          ( int iLuaFunction );
private:
    void                            InitSecurity            ();

    static void                     InstructionCountHook    ( lua_State* luaVM, lua_Debug* pDebug );

    char                            m_szScriptName[MAX_SCRIPTNAME_LENGTH + 1];
    int                             m_iOwner;

    lua_State*                      m_luaVM;
    CLuaTimerManager*               m_pLuaTimerManager;

    bool                            m_bBeingDeleted; // prevent it being deleted twice

    unsigned long                   m_ulFunctionEnterTime;

    class CResource*                m_pResource;

    std::list <CXMLFile*>           m_XMLFiles;
public:
    std::map <const void*, CRefInfo>    m_CallbackTable;
    std::map <int, SString>             m_FunctionTagMap;
};

#endif
