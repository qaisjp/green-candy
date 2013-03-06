/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.0
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        gui/Main.cpp
*  PURPOSE:     Graphical User Interface entry
*  DEVELOPERS:  Christian Myhre Lundheim <>
*               The_GTA <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#include "StdInc.h"
#include "SharedUtil.hpp"

CGUI_Impl* g_pGUI = NULL;

int WINAPI DllMain ( HINSTANCE hModule, DWORD dwReason, PVOID pvNothing )
{
    return TRUE;
}

extern "C" _declspec(dllexport)
CGUI* InitGUIInterface ( IDirect3DDevice9* pDevice, CCoreInterface *coreInterface )
{
    // Create our GUI interface if not already done
    if ( !g_pGUI )
    {
        g_pGUI = new CGUI_Impl( pDevice, coreInterface );
    }

    // Return it
    return g_pGUI;
}

extern "C"
{
BOOL WINAPI _DllMainCRTStartup(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpReserved);
}

BOOL WINAPI _dllInit( HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpReserved )
{
    switch( fdwReason )
    {
    case DLL_PROCESS_ATTACH:
        DbgHeap_Init();
        break;
    }

    BOOL ret = _DllMainCRTStartup( hinstDLL, fdwReason, lpReserved );

    switch( fdwReason )
    {
    case DLL_PROCESS_DETACH:
        DbgHeap_Shutdown();
        break;
    }

    return ret;
}