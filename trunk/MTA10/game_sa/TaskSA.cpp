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

CTaskSAInterface* CTaskSAInterface::Clone()
{
    return NULL;
}

CTaskSAInterface* CTaskSAInterface::GetSubTask()
{
    return NULL;
}

bool CTaskSAInterface::IsSimpleTask()
{
    return false;
}

int CTaskSAInterface::GetTaskType()
{
    return -1;
}

void CTaskSAInterface::StopTimer( CEventSAInterface *evt )
{

}

bool CTaskSAInterface::MakeAbortable( CPedSAInterface *ped, int priority, CEventSAInterface *evt )
{
    return false;
}

CTaskSA::CTaskSA()
{
    DEBUG_TRACE("CTaskSA::CTaskSA()");

    m_parent = NULL;;
    m_interface = NULL;
    dwTasksCreatedTotal ++;
    dwTasksAlive++;
    m_beingDestroyed = false;
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

    m_interface = new CTaskSAInterface(); // :3
    m_parent = NULL;
}

CTask * CTaskSA::Clone() 
{
    // This will crash! >:D
    return (CTask *)m_interface->Clone();
}

void CTaskSA::SetParent( CTask* pParent ) 
{
    CTaskSA *parent = dynamic_cast < CTaskSA* > ( pParent );

    if ( !parent )
        return;

    m_interface->m_pParent = parent->m_interface;
    m_parent = parent;
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
bool CTaskSA::MakeAbortable( CPed* pPed, int iPriority, CEventSAInterface* pEvent ) 
{
    DEBUG_TRACE("bool CTaskSA::MakeAbortable(CPed* pPed, const int iPriority, const CEvent* pEvent)");

    return m_interface->MakeAbortable( ((CPedSA*)pPed)->GetPedInterface(), iPriority, pEvent );
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

    if ( m_beingDestroyed ) // we want to make sure we don't delete this twice or we get crashes :)
        return;              // our hook in CTaskManagementSystem will try to delete this otherwise

    m_beingDestroyed = true;
    delete this;
}

void CTaskSA::SetAsPedTask ( CPed * pPed, int iTaskPriority, bool bForceNewTask )
{
    ((CTaskManagerSA*)pPed->GetPedIntelligence()->GetTaskManager())->SetTask ( this, (eTaskPriority)iTaskPriority, bForceNewTask );
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

    return GetInterface()->ProcessPed( ((CPedSA*)pPed)->GetPedInterface() );
}

bool CTaskSimpleSA::SetPedPosition ( CPed* pPed )
{
    DEBUG_TRACE("bool CTaskSimpleSA::SetPedPosition(CPed* pPed)");

    return GetInterface()->SetPedPosition( ((CPedSA*)pPed)->GetPedInterface() );
}

// ####################################################################
// ## CTaskComplex Functions
// ####################################################################

void CTaskComplexSA::SetSubTask ( CTask* pSubTask )
{
    DEBUG_TRACE("void CTaskComplexSA::SetSubTask(CTask* pSubTask)");

    return GetInterface()->SetSubTask( dynamic_cast < CTaskSA* > ( pSubTask )->GetInterface() );
}

CTask * CTaskComplexSA::CreateNextSubTask ( CPed* pPed )
{
    DEBUG_TRACE("CTask * CTaskComplexSA::CreateNextSubTask(CPed* pPed)");

    return ((CTaskManagementSystemSA*)pGame->GetTaskManagementSystem())->GetTask ( GetInterface()->CreateNextSubTask( dynamic_cast <CPedSA*> ( pPed )->GetPedInterface() ) );
}

CTask * CTaskComplexSA::CreateFirstSubTask ( CPed* pPed )
{
    DEBUG_TRACE("CTask * CTaskComplexSA::CreateFirstSubTask(CPed* pPed)");

    return ((CTaskManagementSystemSA*)pGame->GetTaskManagementSystem())->GetTask ( GetInterface()->CreateFirstSubTask( dynamic_cast <CPedSA*> ( pPed )->GetPedInterface() ) );
}

CTask * CTaskComplexSA::ControlSubTask ( CPed* pPed )
{
    DEBUG_TRACE("CTask * CTaskComplexSA::ControlSubTask(CPed* pPed)");

    return ((CTaskManagementSystemSA*)pGame->GetTaskManagementSystem())->GetTask( GetInterface()->ControlSubTask( dynamic_cast <CPedSA*> ( pPed )->GetPedInterface() ) );
}

