/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.0
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        game_sa/CTaskManagerSA.cpp
*  PURPOSE:     Task manager
*  DEVELOPERS:  Ed Lyons <eai@opencoding.net>
*               Christian Myhre Lundheim <>
*               Cecill Etheredge <ijsf@gmx.net>
*               Jax <>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#include "StdInc.h"

CTaskManagerSA::CTaskManagerSA( CTaskManagerSAInterface *taskManager, CPed *ped )
{
    DEBUG_TRACE("CTaskManagerSA::CTaskManagerSA(CTaskManagerSAInterface * taskManagerInterface, CPed * ped)");

    m_ped = ped;
    m_interface = taskManager;
    m_pTaskManagementSystem = (CTaskManagementSystemSA *)(pGame->GetTaskManagementSystem());
}

void CTaskManagerSA::RemoveTask( eTaskPriority priority )
{
    if ( priority == TASK_PRIORITY_DEFAULT ) // TASK_PRIORITY_DEFAULT removed = crash
        return;

    SetTask( NULL, priority );
}

void CTaskManagerSA::SetTask( CTaskSA* pTaskPrimary, eTaskPriority priority, bool bForceNewTask )
{
    DEBUG_TRACE("void CTaskManagerSA::SetTask(CTask* pTaskPrimary, const int iTaskPriority, const bool bForceNewTask)");
    
    DWORD dwFunc = FUNC_SetTask;
    DWORD dwInterface = (DWORD)m_interface;
    CTaskSAInterface *task = pTaskPrimary ? pTaskPrimary->GetInterface() : NULL;

    _asm
    {
        xor     eax, eax
        movzx   eax, bForceNewTask
        push    eax
        push    priority
        push    task
        mov     ecx, dwInterface
        call    dwFunc
    }    
}

CTask* CTaskManagerSA::GetTask( eTaskPriority priority )
{
    DEBUG_TRACE("CTask* CTaskManagerSA::GetTask(const int iTaskPriority)");

    return m_pTaskManagementSystem->GetTask ( m_interface->m_tasks[ priority ] );
}

CTaskSA* CTaskManagerSA::GetActiveTask()
{
    DEBUG_TRACE("CTask* CTaskManagerSA::GetActiveTask()");

    DWORD dwFunc = FUNC_GetActiveTask;
    DWORD dwThis = (DWORD)m_interface;
    CTaskSAInterface *task;

    _asm
    {
        mov     ecx, dwThis
        call    dwFunc
        mov     task, eax
    }

    return task ? m_pTaskManagementSystem->GetTask ( task ) : NULL;
}

CTaskSA* CTaskManagerSA::GetSimplestActiveTask()
{
    DEBUG_TRACE("CTask* CTaskManagerSA::GetSimplestActiveTask()");

    DWORD dwFunc = FUNC_GetSimplestActiveTask;
    DWORD dwThis = (DWORD)m_interface;
    CTaskSAInterface *task;

    _asm
    {
        mov     ecx, dwThis
        call    dwFunc
        mov     task, eax
    }

    return task ? m_pTaskManagementSystem->GetTask ( task ) : NULL;
}

CTask* CTaskManagerSA::GetSimplestTask( eTaskPriority priority )
{
    DEBUG_TRACE("CTask* CTaskManagerSA::GetSimplestTask(const int iPriority)");

    DWORD dwFunc = FUNC_GetSimplestTask;
    DWORD dwThis = (DWORD)m_interface;
    CTaskSAInterface *task;

    _asm
    {
        mov     ecx, dwThis
        push    priority
        call    dwFunc
        mov     task, eax
    }

    return task ? m_pTaskManagementSystem->GetTask ( task ) : NULL;
}

CTaskSA* CTaskManagerSA::FindActiveTaskByType( int iTaskType )
{
    DEBUG_TRACE("CTask* CTaskManagerSA::FindActiveTaskByType(const int iTaskType)");

    DWORD dwFunc = FUNC_FindActiveTaskByType;
    DWORD dwThis = (DWORD)m_interface;
    CTaskSAInterface *task;

    _asm
    {
        mov     ecx, dwThis
        push    iTaskType
        call    dwFunc
        mov     task, eax
    }

    return task ? m_pTaskManagementSystem->GetTask ( task ) : NULL;
}

CTask* CTaskManagerSA::FindTaskByType( eTaskPriority priority, int iTaskType )
{
    DEBUG_TRACE("CTask* CTaskManagerSA::FindTaskByType(const int iPriority, const int iTaskType)");

    DWORD dwFunc = FUNC_FindTaskByType;
    DWORD dwThis = (DWORD)m_interface;
    CTaskSAInterface *task;

    _asm
    {
        mov     ecx, dwThis
        push    iTaskType
        push    priority
        call    dwFunc
        mov     task, eax
    }

    return task ? m_pTaskManagementSystem->GetTask ( task ) : NULL;
}

void CTaskManagerSA::RemoveTaskSecondary( eTaskPriority priority )
{
    SetTaskSecondary( NULL, priority );
}

void CTaskManagerSA::SetTaskSecondary( CTaskSA* pTask, int iType )
{
    DEBUG_TRACE("void CTaskManagerSA::SetTaskSecondary(CTask* pTaskSecondary, const int iType)");

    DWORD dwFunc = FUNC_SetTaskSecondary;
    DWORD dwInterface = (DWORD)m_interface;
    CTaskSAInterface *task = pTask ? pTask->GetInterface() : NULL;

    _asm
    {
        push    iType
        push    taskInterface
        mov     ecx, dwInterface
        call    dwFunc
    }
}

/**
 * \todo Convert to our tasks not gta's (same above too)
 */
CTaskSA* CTaskManagerSA::GetTaskSecondary( int iType )
{
    DEBUG_TRACE("CTask* CTaskManagerSA::GetTaskSecondary(const int iType)");

    return iType < TASK_SECONDARY_MAX ? m_pTaskManagementSystem->GetTask ( m_interface->m_tasksSecondary[ iType ] ) ? NULL;
}

bool CTaskManagerSA::HasTaskSecondary( const CTask *pTaskSecondary )
{
    _asm int 3

    DEBUG_TRACE("bool CTaskManagerSA::HasTaskSecondary(const CTask* pTaskSecondary)");

    DWORD dwFunc = FUNC_HasTaskSecondary;
    bool bReturn;

    _asm
    {
        push    pTaskSecondary
        call    dwFunc
        mov     bReturn, al
    }
    return bReturn;
}

void CTaskManagerSA::ClearTaskEventResponse()
{
    DEBUG_TRACE("void CTaskManagerSA::ClearTaskEventResponse()");

    DWORD dwFunc = FUNC_ClearTaskEventResponse;

    _asm
    {
        call    dwFunc
    }
}

void CTaskManagerSA::Flush( eTaskPriority priority )
{
    unsigned int i;

    DEBUG_TRACE("void CTaskManagerSA::Flush(bool bImmediately)");

    for (i=0; i < TASK_PRIORITY_MAX; i++)
    {
        CTaskSA *task = GetTask ( i );

        if ( !task )
            continue;

        task->MakeAbortable( m_ped, priority, NULL );
    }

    for (i=0; i < TASK_SECONDARY_MAX; i++)
    {
        CTaskSA *task = GetTaskSecondary ( i );

        if ( !task )
            continue;

        task->MakeAbortable( m_ped, priority, NULL );
    }
}
