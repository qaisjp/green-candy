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

// TODO: create a global header that is shared between codebases and has common definitions in it.
template <typename dataType, unsigned int pulseCount, unsigned int allocFlags, typename arrayMan, typename countType>
struct growableArray
{
    typedef dataType dataType_t;

    static AINLINE void* _memAlloc( size_t memSize, unsigned int flags )
    {
#ifndef _USE_ROCKSTAR_MEMORY_FUNCTIONS
        return malloc( memSize );
#else
        return rwInterface->m_memory.m_malloc( memSize, flags );
#endif //_USE_ROCKSTAR_MEMORY_FUNCTIONS
    }

    static AINLINE void* _memRealloc( void *memPtr, size_t memSize, unsigned int flags )
    {
#ifndef _USE_ROCKSTAR_MEMORY_FUNCTIONS
        return realloc( memPtr, memSize );
#else
        return rwInterface->m_memory.m_realloc( memPtr, memSize, flags );
#endif //_USE_ROCKSTAR_MEMORY_FUNCTIONS
    }

    static AINLINE void _memFree( void *memPtr )
    {
#ifndef _USE_ROCKSTAR_MEMORY_FUNCTIONS
        free( memPtr );
#else
        rwInterface->m_memory.m_free( memPtr );
#endif //_USE_ROCKSTAR_MEMORY_FUNCTIONS
    }

    AINLINE growableArray( void )
    {
        data = NULL;
        numActiveEntries = 0;
        sizeCount = 0;
    }

    AINLINE growableArray( const growableArray& right )
    {
        operator = ( right );
    }

    AINLINE void operator = ( const growableArray& right )
    {
        SetSizeCount( right.GetSizeCount() );

        // Copy all data over.
        if ( sizeCount != 0 )
        {
            std::copy( right.data, right.data + sizeCount, data );
        }

        // Set the number of active entries.
        numActiveEntries = right.numActiveEntries;
    }

    AINLINE ~growableArray( void )
    {
        Shutdown();
    }

    AINLINE void Init( void )
    { }

    AINLINE void Shutdown( void )
    {
        if ( data )
            SetSizeCount( 0 );

        numActiveEntries = 0;
        sizeCount = 0;
    }

    AINLINE void SetSizeCount( countType index )
    {
        if ( index != sizeCount )
        {
            countType oldCount = sizeCount;

            sizeCount = index;

            if ( data )
            {
                // Destroy any structures that got removed.
                for ( countType n = index; n < oldCount; n++ )
                {
                    data[n].~dataType();
                }
            }

            RwInterface *rwInterface = RenderWare::GetInterface();

            if ( index == 0 )
            {
                // Handle clearance requests.
                if ( data )
                {
                    _memFree( data );

                    data = NULL;
                }
            }
            else
            {
                size_t newArraySize = sizeCount * sizeof( dataType );

                if ( !data )
                    data = (dataType*)_memAlloc( newArraySize, allocFlags );
                else
                    data = (dataType*)_memRealloc( data, newArraySize, allocFlags );
            }

            if ( data )
            {
                // Fill the gap.
                for ( countType n = oldCount; n < index; n++ )
                {
                    new (&data[n]) dataType;

                    manager.InitField( data[n] );
                }
            }
            else
                sizeCount = 0;
        }
    }

    AINLINE void AllocateToIndex( countType index )
    {
        if ( index >= sizeCount )
        {
            SetSizeCount( index + ( pulseCount + 1 ) );
        }
    }

    AINLINE void SetItem( const dataType& dataField, countType index )
    {
        AllocateToIndex( index );

        data[index] = dataField;
    }

    AINLINE void SetFast( const dataType& dataField, countType index )
    {
        // God mercy the coder knows why and how he is using this.
        data[index] = dataField;
    }

    AINLINE dataType& GetFast( countType index )
    {
        return data[index];
    }

    AINLINE void AddItem( const dataType& data )
    {
        SetItem( data, numActiveEntries );

        numActiveEntries++;
    }

    AINLINE dataType& ObtainItem( countType obtainIndex )
    {
        AllocateToIndex( obtainIndex );

        return data[obtainIndex];
    }

    AINLINE dataType& ObtainItem( void )
    {
        return ObtainItem( numActiveEntries++ );
    }

    AINLINE countType GetCount( void ) const
    {
        return numActiveEntries;
    }

    AINLINE countType GetSizeCount( void ) const
    {
        return sizeCount;
    }

    AINLINE dataType& Get( countType index )
    {
        assert( index < sizeCount );

        return data[index];
    }

    AINLINE const dataType& Get( countType index ) const
    {
        assert( index < sizeCount );

        return data[index];
    }

    AINLINE bool Pop( dataType& item )
    {
        if ( numActiveEntries != 0 )
        {
            item = data[--numActiveEntries];
            return true;
        }

        return false;
    }

    AINLINE bool Pop( void )
    {
        if ( numActiveEntries != 0 )
        {
            --numActiveEntries;
            return true;
        }

        return false;
    }

    AINLINE void RemoveItem( countType foundSlot )
    {
        assert( foundSlot >= 0 && foundSlot < numActiveEntries );
        assert( numActiveEntries != 0 );

        countType moveCount = numActiveEntries - ( foundSlot + 1 );

        if ( moveCount != 0 )
            std::copy( data + foundSlot + 1, data + numActiveEntries, data + foundSlot );

        numActiveEntries--;
    }

    AINLINE bool RemoveItem( const dataType& item )
    {
        countType foundSlot = -1;
        
        if ( !Find( item, foundSlot ) )
            return false;

        RemoveItem( foundSlot );
        return true;
    }

    AINLINE bool Find( const dataType& inst, countType& indexOut ) const
    {
        for ( countType n = 0; n < numActiveEntries; n++ )
        {
            if ( data[n] == inst )
            {
                indexOut = n;
                return true;
            }
        }

        return false;
    }

    AINLINE unsigned int Count( const dataType& inst ) const
    {
        unsigned int count = 0;

        for ( countType n = 0; n < numActiveEntries; n++ )
        {
            if ( data[n] == inst )
                count++;
        }

        return count;
    }

    AINLINE void Clear( void )
    {
        numActiveEntries = 0;
    }

    AINLINE void TrimTo( countType indexTo )
    {
        if ( numActiveEntries > indexTo )
            numActiveEntries = indexTo;
    }

    AINLINE void SwapContents( growableArray& right )
    {
        dataType *myData = this->data;
        dataType *swapData = right.data;

        this->data = swapData;
        right.data = myData;

        countType myActiveCount = this->numActiveEntries;
        countType swapActiveCount = right.numActiveEntries;

        this->numActiveEntries = swapActiveCount;
        right.numActiveEntries = myActiveCount;

        countType mySizeCount = this->sizeCount;
        countType swapSizeCount = right.sizeCount;

        this->sizeCount = swapSizeCount;
        right.sizeCount = mySizeCount;
    }
    
    AINLINE void SetContents( growableArray& right )
    {
        right.SetSizeCount( numActiveEntries );

        for ( countType n = 0; n < numActiveEntries; n++ )
            right.data[n] = data[n];

        right.numActiveEntries = numActiveEntries;
    }

    dataType* data;
    countType numActiveEntries;
    countType sizeCount;
    arrayMan manager;
};

namespace ExecutiveManager
{
    // Function used by the system for performance measurements.
    __forceinline double GetPerformanceTimer( void )
    {
        LONGLONG counterFrequency, currentCount;

        QueryPerformanceFrequency( (LARGE_INTEGER*)&counterFrequency );
        QueryPerformanceCounter( (LARGE_INTEGER*)&currentCount );

        return (double)currentCount / (double)counterFrequency;
    }
};

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
    CExecutiveManagerSA             ( void );
    ~CExecutiveManagerSA            ( void );

    CFiberSA*   CreateFiber         ( CFiberSA::fiberexec_t proc, void *userdata );
    void        TerminateFiber      ( CFiberSA *fiber );
    void        CloseFiber          ( CFiberSA *fiber );

    void        PushFiber           ( CFiberSA *fiber );
    void        PopFiber            ( void );
    CFiberSA*   GetCurrentFiber     ( void );

    CExecutiveGroupSA*  CreateGroup ( void );

    void        DoPulse             ( void );

    void        InitializeTasks     ( void );
    void        ShutdownTasks       ( void );

    CExecTaskSA*    CreateTask      ( CExecTaskSA::taskexec_t proc, void *userdata );
    void        CloseTask           ( CExecTaskSA *task );

    RwList <CFiberSA> fibers;
    RwList <CExecTaskSA> tasks;
    RwList <CExecutiveGroupSA> groups;

    struct fiberArrayAllocManager
    {
        AINLINE void InitField( CFiberSA*& fiber )
        {
            fiber = NULL;
        }
    };
    typedef growableArray <CFiberSA*, 2, 0, fiberArrayAllocManager, unsigned int> fiberArray;

    // TODO: when we introduce multi-threading, we will need a fiber stack per thread!
    fiberArray fiberStack;

    CExecutiveGroupSA *defGroup;    // default group that all fibers are put into at the beginning.

    double frameTime;
    double frameDuration;

    double GetFrameDuration( void )
    {
        return frameDuration;
    }
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