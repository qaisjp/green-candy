/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        game_sa/TaskSA.h
*  PURPOSE:     Base game task
*  DEVELOPERS:  Ed Lyons <eai@opencoding.net>
*               Christian Myhre Lundheim <>
*               Jax <>
*               The_GTA <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#ifndef __CGAMESA_TASK
#define __CGAMESA_TASK

#include <game/Task.h>
#include "TaskNamesSA.h"

class CTaskTimer
{
public:
    DWORD                           dwTimeStart; // ?
    DWORD                           dwTimeEnd; // ?
    bool                            bSet;
    bool                            bStopped;
};

// I see what you did there, R*
class __declspec(align(128)) CTaskSAInterface
{
public:
    virtual                                     ~CTaskSAInterface() {}

    virtual __thiscall CTaskSAInterface*        Clone() = 0;
    virtual __thiscall CTaskSAInterface*        GetSubTask() = 0;
    virtual __thiscall bool                     IsSimpleTask() = 0;
    virtual __thiscall int                      GetTaskType() = 0;
    virtual __thiscall void                     StopTimer( CEventSAInterface *evt ) = 0;
    virtual __thiscall void                     MakeAbortable() = 0;

    CTaskSAInterface*           m_pParent;
};

class CTaskSimpleSAInterface : public CTaskSAInterface
{
public:
    virtual __thiscall bool                     ProcessPed( CPedSAInterface *ped ) = 0;
    virtual __thiscall bool                     SetPedRotation( CPedSAInterface *ped ) = 0;
};

class CTaskComplexSAInterface : public CTaskSAInterface
{
public:
    virtual __thiscall void                     SetSubTask( CTaskSAInterface *task ) = 0;
    virtual __thiscall CTaskSAInterface*        CreateNextSubTask( CPedSAInterface *ped ) = 0;
    virtual __thiscall CTaskSAInterface*        CreateFirstSubTask( CPedSAInterface *ped ) = 0;
    virtual __thiscall CTaskSAInterface*        ControlSubTask( CPedSAInterface *ped ) = 0;

    CTask*                      m_pSubTask;
};

class CTaskSA : public virtual CTask
{
private:
    CTaskSAInterface*           m_interface;
    CTaskSA*                    m_parent;
    bool                        m_beingDestroyed;

public:
                        CTaskSA                 ( void );
                        ~CTaskSA                ( void );

    CTask*              Clone                   ( void );
    void                SetParent               ( CTask* pParent );
    CTask*              GetParent               ( void )                        { return Parent; }
    CTask*              GetSubTask              ( void );
    bool                IsSimpleTask            ( void );
    int                 GetTaskType             ( void );
    void                StopTimer               ( CEventSAInterface* pEvent );
    bool                MakeAbortable           ( CPed* pPed, const int iPriority, const CEvent* pEvent );
    char*               GetTaskName             ( void );

    // our function(s)
    void                SetInterface            ( CTaskSAInterface* pInterface ) { TaskInterface = pInterface; };
    CTaskSAInterface*   GetInterface            ( void )                         {return this->TaskInterface;}
    bool                IsValid                 ( void )                         { return this->GetInterface() != NULL; }

    void                CreateTaskInterface     ( size_t nSize );

    void                SetAsPedTask            ( CPed * pPed, const int iTaskPriority, const bool bForceNewTask = false );
    void                SetAsSecondaryPedTask   ( CPed * pPed, const int iType );
    void                Destroy                 ( void );
    void                DestroyJustThis         ( void );
};

class CTaskSimpleSA : public virtual CTaskSA, public virtual CTaskSimple
{
public:
    CTaskSimpleSA ( ) {};

    bool ProcessPed(CPed* pPed);
    bool SetPedPosition(CPed *pPed);
};

class CTaskComplexSA : public virtual CTaskSA, public virtual CTaskComplex
{
public:
    CTaskComplexSA() {};

    void SetSubTask(CTask* pSubTask);
    CTask* CreateNextSubTask(CPed* pPed);
    CTask* CreateFirstSubTask(CPed* pPed);
    CTask* ControlSubTask(CPed* pPed);
};

#endif
