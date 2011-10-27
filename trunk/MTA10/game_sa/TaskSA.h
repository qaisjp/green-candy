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

    virtual CTaskSAInterface* __thiscall        Clone() = 0;
    virtual CTaskSAInterface* __thiscall        GetSubTask() = 0;
    virtual bool __thiscall                     IsSimpleTask() = 0;
    virtual int __thiscall                      GetTaskType() = 0;
    virtual void __thiscall                     StopTimer( CEventSAInterface *evt ) = 0;
    virtual void __thiscall                     MakeAbortable( CPedSAInterface *ped, int priority, CEventSAInterface *evt ) = 0;

    void*   operator new( size_t );
    void    operator delete( void *ptr );

    CTaskSAInterface*           m_pParent;
};

class CTaskSimpleSAInterface : public CTaskSAInterface
{
public:
    virtual bool __thiscall                     ProcessPed( CPedSAInterface *ped ) = 0;
    virtual bool __thiscall                     SetPedRotation( CPedSAInterface *ped ) = 0;
};

class CTaskComplexSAInterface : public CTaskSAInterface
{
public:
    virtual void __thiscall                     SetSubTask( CTaskSAInterface *task ) = 0;
    virtual CTaskSAInterface* __thiscall        CreateNextSubTask( CPedSAInterface *ped ) = 0;
    virtual CTaskSAInterface* __thiscall        CreateFirstSubTask( CPedSAInterface *ped ) = 0;
    virtual CTaskSAInterface* __thiscall        ControlSubTask( CPedSAInterface *ped ) = 0;

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

    void                CreateTaskInterface     ();

    void                SetAsPedTask            ( CPed * pPed, const int iTaskPriority, const bool bForceNewTask = false );
    void                SetAsSecondaryPedTask   ( CPed * pPed, const int iType );
    void                Destroy                 ( void );
    void                DestroyJustThis         ( void );
};

class CTaskSimpleSA : public virtual CTaskSA, public virtual CTaskSimple
{
public:
    CTaskSimpleSA ( ) {};

    bool                        ProcessPed(CPed* pPed);
    bool                        SetPedPosition(CPed *pPed);

    CTaskSimpleSAInterface*     GetInterface() { return (CTaskSimpleSAInterface*)m_interface; }
};

class CTaskComplexSA : public virtual CTaskSA, public virtual CTaskComplex
{
public:
    CTaskComplexSA() {};

    void                        SetSubTask(CTask* pSubTask);
    CTask*                      CreateNextSubTask(CPed* pPed);
    CTask*                      CreateFirstSubTask(CPed* pPed);
    CTask*                      ControlSubTask(CPed* pPed);

    CTaskComplexSAInterface*    GetInterface() { return (CTaskComplexSAInterface*)m_interface; }
};

#endif
