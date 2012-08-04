/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.0
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        game_sa/TaskPhysicalResponseSA.h
*  PURPOSE:     Physical response game tasks
*  DEVELOPERS:  Christian Myhre Lundheim <>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#ifndef __CGAMESA_TASKPHYSICALRESPONSE
#define __CGAMESA_TASKPHYSICALRESPONSE

#include <game/TaskPhysicalResponse.h>
#include "TaskSA.h"

#define FUNC_CTaskSimpleChoking__Constructor                0x6202C0
#define FUNC_CTaskSimpleChoking__UpdateChoke                0x620660

class CTaskSimpleChokingSAInterface : public CTaskSimpleSAInterface
{
public:
    CPedSAInterface*            m_pAttacker;
    DWORD*                      m_pAnim;
    unsigned int                m_nTimeRemaining;
    unsigned int                m_nTimeStarted;
    unsigned char               m_bIsTeargas;
    bool                        m_bIsFinished;
};

class CTaskSimpleChokingSA : public virtual CTaskSimpleSA, public virtual CTaskSimpleChoking
{
public:
                        CTaskSimpleChokingSA    () {};
                        CTaskSimpleChokingSA    ( CPed* pAttacker, bool bIsTearGas );

    CPed*               GetAttacker             ();
    unsigned int        GetTimeRemaining        ();
    unsigned int        GetTimeStarted          ();
    bool                IsTeargas               ();
    bool                IsFinished              ();

    void                UpdateChoke             ( CPed* pPed, CPed* pAttacker, bool bIsTearGas );
};

#define MAX_PHYSICAL_TASK_SIZE ( sizeof(CTaskSimpleChokingSA) )

#endif
