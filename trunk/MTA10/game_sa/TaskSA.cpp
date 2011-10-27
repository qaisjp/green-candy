/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        game_sa/TaskSA.cpp
*  PURPOSE:     Base game task
*  DEVELOPERS:  Ed Lyons <eai@opencoding.net>
*               Christian Myhre Lundheim <>
*               Jax <>
*               Cecill Etheredge <ijsf@gmx.net>
*               The_GTA <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#include "StdInc.h"

DWORD dwTasksAlive = 0;
DWORD dwTasksCreatedTotal = 0;

void* CTaskSAInterface::operator new( size_t )
{
    return (*ppTaskPool)->Allocate();
}

void CTaskSAInterface::operator delete( void *ptr )
{
    (*ppTaskPool)->Free( (CTaskSAInterface*)ptr );
}

CTaskSA::CTaskSA()
{
    DEBUG_TRACE("CTaskSA::CTaskSA()");
    Parent = 0;
    TaskInterface = 0;
    dwTasksCreatedTotal ++;
    dwTasksAlive++;
    m_bBeingDestroyed = false;
}

CTaskSA::~CTaskSA()
{
    DEBUG_TRACE("CTaskSA::~CTaskSA()");
    dwTasksAlive--;
}


// allocate memory for the task (ammount nSize)
void CTaskSA::CreateTaskInterface()
{
    DEBUG_TRACE("void CTaskSA::CreateTaskInterface(size_t nSize)");

    TaskInterface = new CTaskSAInterface(); // :3
    Parent = 0;
}


CTask * CTaskSA::Clone() 
{
    // This will crash!
    return (CTask *)m_interface->Clone();
}

void CTaskSA::SetParent(CTask* pParent) 
{
    m_interface->m_pParent = ((CTaskSA*)pParent)->m_interface;
    m_parent = (CTaskSA*)pParent;
}

CTask * CTaskSA::GetSubTask() 
{
    DEBUG_TRACE("CTask * CTaskSA::GetSubTask()");

    return pGame->GetTaskManagementSystem()->GetTask ( m_interface->GetSubTask() );
}

bool CTaskSA::IsSimpleTask()
{
    DEBUG_TRACE("bool CTaskSA::IsSimpleTask()");
 
    return m_interface->IsSimpleTask();
}

int CTaskSA::GetTaskType() 
{
    DEBUG_TRACE("int CTaskSA::GetTaskType()");

    return m_interface->GetTaskType();
}

void CTaskSA::StopTimer(CEventSAInterface* pEvent) 
{
    DEBUG_TRACE("void CTaskSA::StopTimer(const CEvent* pEvent)");
    
    m_interface->StopTimer( pEvent );
}

/**
 * \todo Handle pEvent correctly to convert it
 */
bool CTaskSA::MakeAbortable(CPed* pPed, const int iPriority, CEventSAInterface* pEvent) 
{
    DEBUG_TRACE("bool CTaskSA::MakeAbortable(CPed* pPed, const int iPriority, const CEvent* pEvent)");

    return m_interface->MakeAbortable( pPed, iPriority, pEvent );
}

char * CTaskSA::GetTaskName()
{
    DEBUG_TRACE("char * CTaskSA::GetTaskName()");

    return TaskNames[ m_interface->GetTaskType() ].szName;
}

void CTaskSA::Destroy()
{
    DEBUG_TRACE("void CTaskSA::Destroy()");

    delete m_interface;

    delete this;
}

void CTaskSA::DestroyJustThis()
{
    DEBUG_TRACE("void CTaskSA::DestroyJustThis()");

    if ( m_bBeingDestroyed ) // we want to make sure we don't delete this twice or we get crashes :)
        return;              // our hook in CTaskManagementSystem will try to delete this otherwise

    m_bBeingDestroyed = true;
    delete this;
}

void CTaskSA::SetAsPedTask ( CPed * pPed, const int iTaskPriority, const bool bForceNewTask )
{
    ((CTaskManagerSA*)pPed->GetPedIntelligence()->GetTaskManager())->SetTask ( this, iTaskPriority, bForceNewTask );
}

void CTaskSA::SetAsSecondaryPedTask ( CPed * pPed, const int iType )
{
    ((CTaskManagerSA*)pPed->GetPedIntelligence()->GetTaskManager())->SetTaskSecondary ( this, iType );
}


// ####################################################################
// ## CTaskSimple Functions
// ####################################################################

bool CTaskSimpleSA::ProcessPed ( CPed* pPed )
{
    DEBUG_TRACE("bool CTaskSimpleSA::ProcessPed(CPed* pPed)");

    CPedSA* pPedSA = dynamic_cast < CPedSA* > ( pPed );
    if ( !pPedSA ) return false;

    DWORD dwPedInterface = (DWORD)pPedSA->GetInterface ();
    DWORD dwThisInterface = (DWORD)this->GetInterface ();
    DWORD dwFunc = ((TaskSimpleVTBL *)this->GetInterface ()->VTBL)->ProcessPed;
    bool bReturn = 0;
    if ( dwFunc != 0x82263A && dwFunc )
    {
        _asm
        {
            mov     ecx, dwThisInterface
            push    dwPedInterface
            call    dwFunc
            mov     bReturn, al
        }
    }
    return bReturn;
}

bool CTaskSimpleSA::SetPedPosition ( CPed* pPed )
{
    DEBUG_TRACE("bool CTaskSimpleSA::SetPedPosition(CPed* pPed)");

    CPedSA* pPedSA = dynamic_cast < CPedSA* > ( pPed );
    if ( !pPedSA ) return false;

    DWORD dwPedInterface = (DWORD)pPedSA->GetInterface ();
    DWORD dwThisInterface = (DWORD)this->GetInterface ();
    DWORD dwFunc = ((TaskSimpleVTBL *)this->GetInterface ()->VTBL)->SetPedPosition;
    bool bReturn = 0;
    if ( dwFunc != 0x82263A && dwFunc )
    {
        _asm
        {
            mov     ecx, dwThisInterface
            push    dwPedInterface
            call    dwFunc
            mov     bReturn, al
        }
    }
    return bReturn;
}
// ####################################################################
// ## CTaskComplex Functions
// ####################################################################

void CTaskComplexSA::SetSubTask(CTask* pSubTask)
{
    DEBUG_TRACE("void CTaskComplexSA::SetSubTask(CTask* pSubTask)");
/*  if(this->m_pSubTask)
        delete this->m_pSubTask;
    else
        this->m_pSubTask = pSubTask;*/

    DWORD dwTaskInterface = (DWORD)pSubTask->GetInterface();
    DWORD dwThisInterface = (DWORD)this->GetInterface();
    DWORD dwFunc = ((TaskComplexVTBL *)this->GetInterface()->VTBL)->SetSubTask;
    if ( dwFunc != 0x82263A  && dwFunc )
    {
        _asm
        {
            mov     ecx, dwThisInterface
            push    dwTaskInterface
            call    dwFunc
        }
    }
}

CTask * CTaskComplexSA::CreateNextSubTask ( CPed* pPed )
{
    DEBUG_TRACE("CTask * CTaskComplexSA::CreateNextSubTask(CPed* pPed)");

    return ((CTaskManagementSystemSA*)pGame->GetTaskManagementSystem())->GetTask ( GetInterface()->CreateNextSubTask( (CPed );
}

CTask * CTaskComplexSA::CreateFirstSubTask ( CPed* pPed )
{
    DEBUG_TRACE("CTask * CTaskComplexSA::CreateFirstSubTask(CPed* pPed)");

    return ((CTaskManagementSystemSA*)pGame->GetTaskManagementSystem())->GetTask ( GetInterface()->CreateFirstSubTask( ((CPedSA*)pPed)->GetPedInterface() );
}

CTask * CTaskComplexSA::ControlSubTask ( CPed* pPed )
{
    DEBUG_TRACE("CTask * CTaskComplexSA::ControlSubTask(CPed* pPed)");

    return ((CTaskManagementSystemSA*)pGame->GetTaskManagementSystem())->GetTask( GetInterface()->ControlSubTask( ((CPedSA*)pPed)->GetPedInterface() );
}

