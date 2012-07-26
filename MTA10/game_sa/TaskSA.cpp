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

bool CTaskSAInterface::IsSimpleTask() const
{
    return false;
}

int CTaskSAInterface::GetTaskType() const
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
    DEBUG_TRACE("void CTaskSA::CreateTaskInterface()");

    m_interface = new CTaskSAInterface(); // :3
    m_parent = NULL;
}

CTask* CTaskSA::Clone() 
{
    // This will crash! >:D
    return (CTask*)m_interface->Clone();
}

void CTaskSA::SetParent( CTask* pParent ) 
{
    CTaskSA *parent = dynamic_cast < CTaskSA* > ( pParent );

    if ( !parent )
        return;

    m_interface->m_pParent = parent->m_interface;
    m_parent = parent;
}

CTask* CTaskSA::GetSubTask() 
{
    DEBUG_TRACE("CTask* CTaskSA::GetSubTask()");

    return pGame->GetTaskManagementSystem()->GetTask( m_interface->GetSubTask() );
}

bool CTaskSA::MakeAbortable( CPed *ped, int iPriority )
{
    return m_interface->MakeAbortable( dynamic_cast <CPedSA*> ( ped )->GetInterface(), iPriority, NULL );
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

void CTaskSA::SetAsPedTask( CPed *ped, int iTaskPriority, bool bForceNewTask )
{
    ((CTaskManagerSA*)ped->GetPedIntelligence()->GetTaskManager())->SetTask( this, (eTaskPriority)iTaskPriority, bForceNewTask );
}

void CTaskSA::SetAsSecondaryPedTask( CPed *ped, int iType )
{
    ((CTaskManagerSA*)ped->GetPedIntelligence()->GetTaskManager())->SetTaskSecondary( this, iType );
}


// ####################################################################
// ## CTaskSimple Functions
// ####################################################################

bool CTaskSimpleSA::ProcessPed( CPed* ped )
{
    DEBUG_TRACE("bool CTaskSimpleSA::ProcessPed( CPed* ped )");

    return GetInterface()->ProcessPed( dynamic_cast <CPedSA*> ( ped )->GetInterface() );
}

bool CTaskSimpleSA::SetPedPosition( CPed* ped )
{
    DEBUG_TRACE("bool CTaskSimpleSA::SetPedPosition( CPed* ped )");

    return GetInterface()->SetPedPosition( dynamic_cast <CPedSA*> ( ped )->GetInterface() );
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

    return ((CTaskManagementSystemSA*)pGame->GetTaskManagementSystem())->GetTask ( GetInterface()->CreateNextSubTask( dynamic_cast <CPedSA*> ( pPed )->GetInterface() ) );
}

CTask * CTaskComplexSA::CreateFirstSubTask ( CPed* pPed )
{
    DEBUG_TRACE("CTask * CTaskComplexSA::CreateFirstSubTask(CPed* pPed)");

    return ((CTaskManagementSystemSA*)pGame->GetTaskManagementSystem())->GetTask ( GetInterface()->CreateFirstSubTask( dynamic_cast <CPedSA*> ( pPed )->GetInterface() ) );
}

CTask * CTaskComplexSA::ControlSubTask ( CPed* pPed )
{
    DEBUG_TRACE("CTask * CTaskComplexSA::ControlSubTask(CPed* pPed)");

    return ((CTaskManagementSystemSA*)pGame->GetTaskManagementSystem())->GetTask( GetInterface()->ControlSubTask( dynamic_cast <CPedSA*> ( pPed )->GetInterface() ) );
}

