/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        game_sa/CExecutiveManagerSA.h
*  PURPOSE:     MTA thread and fiber execution manager for workload smoothing
*  DEVELOPERS:  Martin Turski <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#ifndef _EXECUTIVE_MANAGER_
#define _EXECUTIVE_MANAGER_

#include "CExecutiveManagerSA.fiber.h"

#define DEFAULT_GROUP_MAX_EXEC_TIME     16

struct fiberTerminationException
{
    fiberTerminationException( CFiberSA *fiber )
    {
        this->fiber = fiber;
    }

    CFiberSA *fiber;
};

class CExecutiveGroupSA;

class CExecutiveManagerSA
{
    friend class CExecutiveGroupSA;
public:
    CExecutiveManagerSA             ( void );
    ~CExecutiveManagerSA            ( void );

    CFiberSA*   CreateFiber         ( CFiberSA::fiberexec_t proc, void *userdata );
    void        TerminateFiber      ( CFiberSA *fiber );
    void        CloseFiber          ( CFiberSA *fiber );

    CExecutiveGroupSA*  CreateGroup ( void );

    void        DoPulse             ( void );

    RwList <CFiberSA> fibers;
    RwList <CExecutiveGroupSA> groups;

    CExecutiveGroupSA *defGroup;    // default group that all fibers are put into at the beginning.
};

class CExecutiveGroupSA
{
    friend class CExecutiveManagerSA;

    inline void AddFiberNative( CFiberSA *fiber )
    {
        LIST_APPEND( fibers.root, fiber->groupNode );

        fiber->group = this;
    }

public:
    CExecutiveGroupSA( CExecutiveManagerSA *manager )
    {
        LIST_CLEAR( fibers.root );

        this->manager = manager;
        this->maximumExecutionTime = DEFAULT_GROUP_MAX_EXEC_TIME;
        this->totalFrameExecutionTime = 0;

        LIST_APPEND( manager->groups.root, managerNode );
    }

    ~CExecutiveGroupSA( void )
    {
        while ( !LIST_EMPTY( fibers.root ) )
        {
            CFiberSA *fiber = LIST_GETITEM( CFiberSA, fibers.root.next, groupNode );

            LIST_REMOVE( fiber->groupNode );

            manager->CloseFiber( fiber );
        }

        LIST_REMOVE( managerNode );
    }

    inline void AddFiber( CFiberSA *fiber )
    {
        LIST_REMOVE( fiber->groupNode );
        
        AddFiberNative( fiber );
    }

    inline void SetMaximumExecutionTime( double ms )        { maximumExecutionTime = ms; }
    inline double GetMaximumExecutionTime( void ) const     { return maximumExecutionTime; }

    void DoPulse( void )
    {
        totalFrameExecutionTime = 0;
    }

    CExecutiveManagerSA *manager;

    RwList <CFiberSA> fibers;                       // all fibers registered to this group.

    RwListEntry <CExecutiveGroupSA> managerNode;    // node in list of all groups.

    // Per frame settings
    // These times are given in milliseconds.
    double totalFrameExecutionTime;
    double maximumExecutionTime;
};

#endif //_EXECUTIVE_MANAGER_