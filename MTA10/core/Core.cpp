/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.0
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        core/Core.cpp
*  PURPOSE:     Core library entry point
*  DEVELOPERS:  Derek Abdine <>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#include "StdInc.h"

using std::string;

CCore* g_pCore = NULL;
bool IsRealDeal ( void );

int WINAPI DllMain(HINSTANCE hModule, DWORD dwReason, PVOID pvNothing)
{
    if ( dwReason == DLL_PROCESS_ATTACH )
    {
        if ( IsRealDeal () )
        {
            // Search our path for .dll instead of WINDOWS
            SetDllDirectory( GetMTASABaseDir() + "mta/" );

            g_pCore = new CCore;
        }
    } 
    else if (dwReason == DLL_PROCESS_DETACH)
    {
        if ( IsRealDeal () )
        {
            // For now, TerminateProcess if any destruction is attempted (or we'll crash)
            TerminateProcess ( GetCurrentProcess (), 0 );

            if ( g_pCore )
            {
                delete g_pCore;
                g_pCore = NULL;
            }
        }
    }

    return TRUE;
}


//
// Returns true if dll has been loaded with GTA.
//
bool IsRealDeal ( void )
{
    static bool bResult = false;
    static bool bDone = false;
    if ( !bDone )
    {
        MEMORY_BASIC_INFORMATION info;
        VirtualQuery( (void*)0x0401000, &info, sizeof(MEMORY_BASIC_INFORMATION) );
        bResult = info.RegionSize > 0x0401000;
        bDone = true;
    }
    return bResult;
}
