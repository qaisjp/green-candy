/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.0
*               (Shared logic for modifications)
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        mods/shared_logic/lua/CLuaFunctionDefs.cpp
*  PURPOSE:     Lua function definitions class
*  DEVELOPERS:  Ed Lyons <eai@opencoding.net>
*               Jax <>
*               Cecill Etheredge <ijsf@gmx.net>
*               Kevin Whiteside <kevuwk@gmail.com>
*               Chris McArthur <>
*               Derek Abdine <>
*               Christian Myhre Lundheim <>
*               Stanislav Bobrov <lil_toady@hotmail.com>
*               Alberto Alonso <rydencillo@gmail.com>
*
*****************************************************************************/

#include "StdInc.h"

namespace CLuaFunctionDefs
{
    CLuaManager*                 m_pLuaManager;
    CScriptDebugging*            m_pScriptDebugging;
    CClientGame*                 m_pClientGame;
    CClientManager*              m_pManager;
    CClientEntity*               m_pRootEntity;
    CClientGUIManager*           m_pGUIManager;
    CClientPedManager*           m_pPedManager;
    CClientPlayerManager*        m_pPlayerManager;
    CClientRadarMarkerManager*   m_pRadarMarkerManager;
    CResourceManager*            m_pResourceManager;
    CClientVehicleManager*       m_pVehicleManager;
    CClientColManager*           m_pColManager;
    CClientObjectManager*        m_pObjectManager;
    CClientTeamManager*          m_pTeamManager;
    CRenderWare*                 m_pRenderWare;
    CClientMarkerManager*        m_pMarkerManager;
    CClientPickupManager*        m_pPickupManager;
    CClientDFFManager*           m_pDFFManager;
    CClientColModelManager*      m_pColModelManager;
    CRegisteredCommands*         m_pRegisteredCommands;

    void CLuaFunctionDefs::Initialize( CLuaManager* pLuaManager, CScriptDebugging* pScriptDebugging, CClientGame* pClientGame )
    {
        m_pLuaManager = pLuaManager;
        m_pScriptDebugging = pScriptDebugging;
        m_pClientGame = pClientGame;
        m_pManager = pClientGame->GetManager ();
        m_pRootEntity = m_pClientGame->GetRootEntity ();
        m_pGUIManager = m_pClientGame->GetGUIManager ();
        m_pPedManager = m_pClientGame->GetPedManager ();
        m_pPlayerManager = m_pClientGame->GetPlayerManager ();
        m_pRadarMarkerManager = m_pClientGame->GetRadarMarkerManager ();
        m_pResourceManager = m_pClientGame->GetResourceManager ();
        m_pColManager = m_pManager->GetColManager ();
        m_pVehicleManager = m_pManager->GetVehicleManager ();
        m_pObjectManager = m_pManager->GetObjectManager ();
        m_pTeamManager = m_pManager->GetTeamManager ();
        m_pRenderWare = g_pGame->GetRenderWare ();
        m_pMarkerManager = m_pManager->GetMarkerManager ();
        m_pPickupManager = m_pManager->GetPickupManager ();
        m_pDFFManager = m_pManager->GetDFFManager ();
        m_pColModelManager = m_pManager->GetColModelManager ();
        m_pRegisteredCommands = m_pClientGame->GetRegisteredCommands ();
    }
}