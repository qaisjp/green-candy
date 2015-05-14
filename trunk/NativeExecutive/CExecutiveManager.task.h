/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        NativeExecutive/CExecutiveManager.task.h
*  PURPOSE:     Runtime for quick parallel execution sheduling
*  DEVELOPERS:  Martin Turski <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#ifndef _EXECUTIVE_MANAGER_TASKS_
#define _EXECUTIVE_MANAGER_TASKS_

// Implementation for very fast and synchronized data sheduling.
template <typename dataType>
struct SynchronizedHyperQueue
{
    struct queueItemArrayManager
    {
        AINLINE void InitField( dataType& field )
        {
            return;
        }
    };
    typedef growableArray <dataType, 32, 0, queueItemArrayManager, unsigned int> queueItemArray;

    AINLINE SynchronizedHyperQueue( unsigned int queueStartSize )
    {
        if ( queueStartSize == 0 )
        {
            queueStartSize = 256;
        }

        queueArray.SetSizeCount( queueStartSize );

        queueWrapSize = queueStartSize;

        // Initialize synchronization objects.
        InitializeCriticalSection( &sheduler_lock );
    }

    AINLINE ~SynchronizedHyperQueue( void )
    {
        // Delete synchronization objects.
        DeleteCriticalSection( &sheduler_lock );
    }

    AINLINE bool GetSheduledItem( dataType& item )
    {
        bool hasItem = false;
        
        EnterCriticalSection( &sheduler_lock );

        unsigned int currentIndex = queueProcessorIndex;

        if ( currentIndex != queueShedulerIndex )
        {
            unsigned int nextIndex = currentIndex + 1;

            if ( nextIndex == queueWrapSize )
            {
                nextIndex = 0;
            }

            item = queueArray.Get( currentIndex );

            hasItem = true;

            queueProcessorIndex = nextIndex;
        }

        LeaveCriticalSection( &sheduler_lock );

        return hasItem;
    }

    template <typename itemPositionerType>
    AINLINE void AddItemGeneric( itemPositionerType& cb )
    {
        unsigned int oldSize = queueWrapSize;

        unsigned int currentIndex = queueShedulerIndex;
        unsigned int nextIndex = currentIndex + 1;

        // Add our item.
        if ( nextIndex == oldSize )
        {
            nextIndex = 0;
        }

        // We must add the item here.
        cb.PutItem( queueArray.GetFast( currentIndex ) );

        queueShedulerIndex = nextIndex;

        // Ensure next task addition is safe.
        unsigned int currentProcessorIndex = queueProcessorIndex;

        if ( nextIndex == currentProcessorIndex )
        {
            // We need to expand the queue and shift items around.
            // For this a very expensive operation is required.
            // Ideally, this is not triggered often end-game.
            // Problems happen when the tasks cannot keep up with the sheduler thread.
            // Then this is triggered unnecessaringly often; the task creator is to blame.
            EnterCriticalSection( &sheduler_lock );

            // Skip this expensive operation if the sheduler has processed an item already.
            // This means that the next sheduling will be safe for sure.
            if ( currentProcessorIndex == queueProcessorIndex )
            {
                // Allocate new space.
                unsigned int maxSize = oldSize + 1;

                queueArray.AllocateToIndex( maxSize );

                // Increase the queue wrap size.
                queueWrapSize++;

                // Copy items to next spots.
                unsigned int copyCount = oldSize - ( currentIndex + 1 );

                if ( copyCount != 0 )
                {
                    dataType *startCopy = &queueArray.Get( currentIndex + 1 );
                    dataType *endCopy = startCopy + copyCount;
                    
                    std::copy_backward( startCopy, endCopy, endCopy + 1 );

                    // Initialize the new spot to empty.
                    *startCopy = dataType();
                }

                // Increase the processor index appropriately.
                queueProcessorIndex++;
            }

            LeaveCriticalSection( &sheduler_lock );
        }
    }

    struct DefaultItemPositioner
    {
        const dataType& theData;

        AINLINE DefaultItemPositioner( const dataType& theData ) : theData( theData )
        {
            return;
        }

        AINLINE void PutItem( dataType& theField )
        {
            theField = theData;
        }
    };

    AINLINE void AddItem( const dataType& theData )
    {
        DefaultItemPositioner positioner( theData );

        AddItemGeneric( positioner );
    }

    AINLINE dataType& GetLastSheduledItem( void )
    {
        unsigned int targetIndex = queueShedulerIndex;

        if ( targetIndex == 0 )
        {
            targetIndex = queueWrapSize - 1;
        }
        else
        {
            targetIndex--;
        }

        return queueArray.Get( targetIndex );
    }

    queueItemArray queueArray;

    unsigned int queueWrapSize;
    unsigned int queueProcessorIndex;
    unsigned int queueShedulerIndex;
    CRITICAL_SECTION sheduler_lock;
};

class CExecTask
{
public:
    friend class CExecutiveManager;
    friend class CExecutiveGroup;

    CFiber *runtimeFiber;

    typedef void (__stdcall*taskexec_t)( CExecTask *task, void *userdata );

    taskexec_t callback;

    RwListEntry <CExecTask> node;

    HANDLE finishEvent;

    bool isInitialized;
    bool isOnProcessedList;
    volatile LONG usageCount;

    CExecTask( CExecutiveManager *manager, CFiber *runtime )
    {
        this->runtimeFiber = runtime;

        this->manager = manager;

        // Event that is signaled when the task finsished execution.
        this->finishEvent = CreateEvent( NULL, true, true, NULL );
        this->isInitialized = false;
        this->isOnProcessedList = false;
        this->usageCount = 0;
    }

    ~CExecTask( void )
    {
        CloseHandle( finishEvent );
    }

    void    Execute( void );
    void    WaitForFinish( void );

    AINLINE bool IsFinished( void )
    {
        return ( usageCount == 0 );
    }

    CExecutiveManager *manager;
};

#endif //_EXECUTIVE_MANAGER_TASKS_