/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.0
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        game_sa/CPedIntelligenceSA.cpp
*  PURPOSE:     Ped entity AI logic
*  DEVELOPERS:  Ed Lyons <eai@opencoding.net>
*               Jax <>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#include "StdInc.h"

void* CPedIntelligenceSAInterface::operator new( size_t )
{
    return (*ppPedIntelligencePool)->Allocate();
}

void CPedIntelligenceSAInterface::operator delete( void *ptr )
{
    (*ppPedIntelligencePool)->Free( (CPedIntelligenceSAInterface*)ptr );
}

bool CPedIntelligenceSAInterface::TestForStealthKill( CPedSAInterface *ped, bool bUnk )
{
    bool bReturn;
    DWORD dwThis = (DWORD)this;
    DWORD dwPed = (DWORD)ped;
    DWORD dwFunc = FUNC_CPedIntelligence_TestForStealthKill;

    _asm
    {
        mov     ecx, dwThis
        push    bUnk
        push    dwPed
        call    dwFunc
        mov     bReturn, al
    }
    return bReturn;
}

CPedIntelligenceSA::CPedIntelligenceSA ( CPedIntelligenceSAInterface *intelligence, CPed *ped )
{
    m_interface = intelligence;
    m_ped = ped;
    m_taskManager = new CTaskManagerSA( intelligence->m_taskManager, ped );
    m_vehicleScanner = new CVehicleScannerSA( intelligence->m_vehicleScanner );
}

CPedIntelligenceSA::~CPedIntelligenceSA ()
{
    delete m_taskManager;
}

CTaskManager * CPedIntelligenceSA::GetTaskManager( void )
{
    DEBUG_TRACE("CTaskManager * CPedSA::GetTaskManager( void )");
    return m_taskManager;
}

CVehicleScanner * CPedIntelligenceSA::GetVehicleScanner( void )
{
    return m_vehicleScanner;
}

bool CPedIntelligenceSA::IsRespondingToEvent ( void )
{
    DWORD dwFunc = FUNC_IsRespondingToEvent;

        
    return false;
}

int CPedIntelligenceSA::GetCurrentEventType ( void )
{
    DWORD dwFunc = FUNC_GetCurrentEventType;
    DWORD dwRet = 0;
    DWORD dwThis = (DWORD)m_pInterface;

    _asm
    {
        mov     ecx, dwThis
        call    dwFunc
        mov     dwRet, eax
    }
    return dwRet;
}

CEvent * CPedIntelligenceSA::GetCurrentEvent ( void )
{
    return NULL;
}