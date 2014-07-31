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

#include "CMemoryUtilsSA.h"

// Forward declarations.
class CExecThreadSA;
class CFiberSA;
class CExecTaskSA;

namespace ExecutiveManager
{
    // Function used by the system for performance measurements.
    AINLINE double GetPerformanceTimer( void )
    {
        LONGLONG counterFrequency, currentCount;

        QueryPerformanceFrequency( (LARGE_INTEGER*)&counterFrequency );
        QueryPerformanceCounter( (LARGE_INTEGER*)&currentCount );

        return (long double)currentCount / (long double)counterFrequency;
    }

    typedef CPluginContainerSA <CExecThreadSA> threadPluginContainer_t;
};

#include "CExecutiveManagerSA.thread.h"
#include "CExecutiveManagerSA.fiber.h"
#include "CExecutiveManagerSA.task.h"

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
    CExecutiveManagerSA                 ( void );
    ~CExecutiveManagerSA                ( void );

    CExecThreadSA*  CreateThread        ( CExecThreadSA::threadEntryPoint_t proc, void *userdata, size_t stackSize = 0 );
    void            TerminateThread     ( CExecThreadSA *thread );
    CExecThreadSA*  GetCurrentThread    ( void );
    void            CloseThread         ( CExecThreadSA *thread );

    void            InitThreads         ( void );
    void            ShutdownThreads     ( void );

    CFiberSA*       CreateFiber         ( CFiberSA::fiberexec_t proc, void *userdata, size_t stackSize = 0 );
    void            TerminateFiber      ( CFiberSA *fiber );
    void            CloseFiber          ( CFiberSA *fiber );

    void            PushFiber           ( CFiberSA *fiber );
    void            PopFiber            ( void );
    CFiberSA*       GetCurrentFiber     ( void );

    void            InitFibers          ( void );
    void            ShutdownFibers      ( void );

    CExecutiveGroupSA*  CreateGroup     ( void );

    void            DoPulse             ( void );

    void            InitializeTasks     ( void );
    void            ShutdownTasks       ( void );

    CExecTaskSA*    CreateTask          ( CExecTaskSA::taskexec_t proc, void *userdata, size_t stackSize = 0 );
    void            CloseTask           ( CExecTaskSA *task );

    ExecutiveManager::threadPluginContainer_t threadPlugins;

    CRITICAL_SECTION threadPluginsLock;

    RwList <CExecThreadSA> threads;
    RwList <CFiberSA> fibers;
    RwList <CExecTaskSA> tasks;
    RwList <CExecutiveGroupSA> groups;

    // Place for the native plugins to store runtime data.
    ExecutiveManager::threadPluginContainer_t::pluginInterface *threadNativePlugin;
    ExecutiveManager::threadPluginContainer_t::pluginInterface *threadFiberStackPlugin;

    CExecutiveGroupSA *defGroup;    // default group that all fibers are put into at the beginning.

    double frameTime;
    double frameDuration;

    double GetFrameDuration( void )
    {
        return frameDuration;
    }
};

// Exception that gets thrown by threads when they terminate.
struct threadTerminationException : public std::exception
{
    inline threadTerminationException( CExecThreadSA *theThread ) : std::exception( "thread termination" )
    {
        this->terminatedThread = theThread;

        theThread->Lock();
    }

    inline ~threadTerminationException( void )
    {
        this->terminatedThread->Unlock();
    }

    CExecThreadSA *terminatedThread;
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

        this->perfMultiplier = 1.0f;

        LIST_APPEND( manager->groups.root, managerNode );
    }

    ~CExecutiveGroupSA( void )
    {
        while ( !LIST_EMPTY( fibers.root ) )
        {
            CFiberSA *fiber = LIST_GETITEM( CFiberSA, fibers.root.next, groupNode );

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

    double perfMultiplier;

    void SetPerfMultiplier( double mult )
    {
        perfMultiplier = mult;
    }
};

#endif //_EXECUTIVE_MANAGER_