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
void CTaskSA::CreateTaskInterface(size_t nSize)
{
    DEBUG_TRACE("void CTaskSA::CreateTaskInterface(size_t nSize)");

    TaskInterface = new ((*ppTaskPool)->Allocate()) CTaskSAInterface(); // :3
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

/**
 * \todo Handle pEvent correctly to convert it
 */
void CTaskSA::StopTimer(CEventSAInterface* pEvent) 
{
    DEBUG_TRACE("void CTaskSA::StopTimer(const CEvent* pEvent)");
    
    m_interface->StopTimer( pEvent );
}

/**
 * \todo Handle pEvent correctly to convert it
 */
bool CTaskSA::MakeAbortable(CPed* pPed, const int iPriority, const CEvent* pEvent) 
{
    DEBUG_TRACE("bool CTaskSA::MakeAbortable(CPed* pPed, const int iPriority, const CEvent* pEvent)");

    CPedSA* pPedSA = dynamic_cast < CPedSA* > ( pPed );
    if ( !pPedSA ) return false;

    DWORD dwPedInterface = (DWORD) pPedSA->GetInterface ();
    DWORD dwThisInterface = (DWORD) this->GetInterface ();
    DWORD dwFunc = this->GetInterface ()->VTBL->MakeAbortable;
    bool bReturn = 0;
    if ( dwFunc != 0x82263A  && dwFunc ) // 82263A = purecall
    {
        _asm
        {
            mov     ecx, dwThisInterface
            push    pEvent
            push    iPriority
            push    dwPedInterface
            call    dwFunc
            mov     bReturn, al
        }
    }
    return bReturn;
}

char * CTaskSA::GetTaskName()
{
    DEBUG_TRACE("char * CTaskSA::GetTaskName()");
    int iTaskType = GetTaskType();
    if ( iTaskType != NO_TASK_TYPE )
        return TaskNames[iTaskType].szName;
    else
        return reinterpret_cast < char* > ( &sNoTaskName );
}

void CTaskSA::Destroy()
{
    DEBUG_TRACE("void CTaskSA::Destroy()");

    if ( m_bBeingDestroyed ) // we want to make sure we don't delete this twice or we get crashes :)
        return;              // our hook in CTaskManagementSystem will try to delete this otherwise
    m_bBeingDestroyed = true;

    DWORD dwThisInterface = (DWORD)this->GetInterface();
    DWORD dwFunc = this->GetInterface()->VTBL->DeletingDestructor;
    if ( dwFunc )
    {
        _asm
        {
            mov     ecx, dwThisInterface
            push    1           // delete the task too
            call    dwFunc
        }
    }

    /*dwFunc = FUNC_CTask__Operator_Delete;
    DWORD thisInterface = (DWORD)this->GetInterface();
    if ( thisInterface )
    {
        _asm
        {
            push    thisInterface
            call    dwFunc
            add     esp, 4
        }
    }*/

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
/**
 * \todo Implement subtask related stuff without adding variables (so sizeof(CTask) == sizeof(CTaskComplex))
 */
/*
CTaskComplexSA::CTaskComplexSA()
{
    DEBUG_TRACE("CTaskComplexSA::CTaskComplexSA()");
//  this->m_pSubTask = 0;
}
*/


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

    CPedSA* pPedSA = dynamic_cast < CPedSA* > ( pPed );
    if ( !pPedSA ) return NULL;

    DWORD dwPedInterface = (DWORD)pPedSA->GetInterface ();
    DWORD dwThisInterface = (DWORD)this->GetInterface ();
    DWORD dwFunc = ((TaskComplexVTBL *)this->GetInterface ()->VTBL)->CreateNextSubTask;
    DWORD dwReturn = 0;
    if ( dwFunc != 0x82263A && dwFunc )
    {
        _asm
        {
            mov     ecx, dwThisInterface
            push    dwPedInterface
            call    dwFunc
            mov     dwReturn, eax
        }
    }
    return ((CTaskManagementSystemSA*)pGame->GetTaskManagementSystem())->GetTask ( ( CTaskSAInterface * ) dwReturn );
}

CTask * CTaskComplexSA::CreateFirstSubTask ( CPed* pPed )
{
    DEBUG_TRACE("CTask * CTaskComplexSA::CreateFirstSubTask(CPed* pPed)");

    CPedSA* pPedSA = dynamic_cast < CPedSA* > ( pPed );
    if ( !pPedSA ) return NULL;

    DWORD dwPedInterface = (DWORD)pPedSA->GetInterface ();
    DWORD dwThisInterface = (DWORD)this->GetInterface ();
    DWORD dwFunc = ((TaskComplexVTBL *)this->GetInterface ()->VTBL)->CreateFirstSubTask;
    DWORD dwReturn = 0;
    if ( dwFunc != 0x82263A && dwFunc )
    {
        _asm
        {
            mov     ecx, dwThisInterface
            push    dwPedInterface
            call    dwFunc
            mov     dwReturn, eax
        }
    }
    return ((CTaskManagementSystemSA*)pGame->GetTaskManagementSystem())->GetTask ( ( CTaskSAInterface * ) dwReturn );
}

CTask * CTaskComplexSA::ControlSubTask ( CPed* pPed )
{
    DEBUG_TRACE("CTask * CTaskComplexSA::ControlSubTask(CPed* pPed)");

    CPedSA* pPedSA = dynamic_cast < CPedSA* > ( pPed );
    if ( !pPedSA ) return NULL;

    DWORD dwPedInterface = (DWORD)pPedSA->GetInterface ();
    DWORD dwThisInterface = (DWORD)this->GetInterface ();
    DWORD dwFunc = ((TaskComplexVTBL *)this->GetInterface ()->VTBL)->ControlSubTask;
    DWORD dwReturn = 0;
    if ( dwFunc != 0x82263A && dwFunc )
    {
        _asm
        {
            mov     ecx, dwThisInterface
            push    dwPedInterface
            call    dwFunc
            mov     dwReturn, eax
        }
    }
    return ((CTaskManagementSystemSA*)pGame->GetTaskManagementSystem())->GetTask ( ( CTaskSAInterface * ) dwReturn );
}

