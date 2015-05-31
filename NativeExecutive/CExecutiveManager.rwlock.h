/*****************************************************************************
*
*  PROJECT:     Native Executive
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        NativeExecutive/CExecutiveManager.rwlock.h
*  PURPOSE:     Read/Write lock synchronization object
*  DEVELOPERS:  Martin Turski <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#ifndef _NATIVE_EXECUTIVE_READ_WRITE_LOCK_
#define _NATIVE_EXECUTIVE_READ_WRITE_LOCK_

BEGIN_NATIVE_EXECUTIVE

/*
    Synchronization object - the "Read/Write" lock

    Use this sync object if you have a data structure that requires consistency in a multi-threaded environment.
    Just like any other sync object it prevents instruction reordering where it changes the designed functionality.
    But the speciality of this object is that it allows two access modes.

    In typical data structure development, read operations do not change the state of an object. This allows
    multiple threads to run concurrently and still keep the logic of the data intact. This assumption is easily
    warded off if the data structure keeps shadow data for optimization purposes (mutable variables).

    Then there is the writing mode. In this mode threads want exclusive access to a data structure, as concurrent
    modification on a data structure is a daunting task and most often is impossible to solve fast and clean.

    By using this object to mark critical read and write regions in your code, you easily make it thread-safe.
    Thread-safety is the future, as silicon has reached its single-threaded performance peak.

    Please make sure that you use this object in an exception-safe way to prevent dead-locks! This structure does
    not support recursive acquisition, so be careful how you do things!
*/
struct CReadWriteLock abstract
{
    // Shared access to data structures.
    void EnterCriticalReadRegion( void );
    void LeaveCriticalReadRegion( void );

    // Exclusive access to data structures.
    void EnterCriticalWriteRegion( void );
    void LeaveCriticalWriteRegion( void );

    // Attempting to enter the lock while preventing a wait scenario.
    bool TryEnterCriticalReadRegion( void );
    bool TryEnterCriticalWriteRegion( void );
};

// Lock context helpers for exception safe and correct code region marking.
struct CReadWriteReadContext
{
    inline CReadWriteReadContext( CReadWriteLock *theLock )
    {
        theLock->EnterCriticalReadRegion();

        this->theLock = theLock;
    }

    inline ~CReadWriteReadContext( void )
    {
        this->theLock->LeaveCriticalReadRegion();
    }

    CReadWriteLock *theLock;
};

struct CReadWriteWriteContext
{
    inline CReadWriteWriteContext( CReadWriteLock *theLock )
    {
        theLock->EnterCriticalWriteRegion();

        this->theLock = theLock;
    }

    inline ~CReadWriteWriteContext( void )
    {
        this->theLock->LeaveCriticalWriteRegion();
    }

    CReadWriteLock *theLock;
};

// Variants of locking contexts that accept a NULL pointer.
struct CReadWriteReadContextSafe
{
    inline CReadWriteReadContextSafe( CReadWriteLock *theLock )
    {
        if ( theLock )
        {
            theLock->EnterCriticalReadRegion();
        }

        this->theLock = theLock;
    }

    inline ~CReadWriteReadContextSafe( void )
    {
        if ( CReadWriteLock *theLock = this->theLock )
        {
            theLock->LeaveCriticalReadRegion();
        }
    }

    CReadWriteLock *theLock;
};

struct CReadWriteWriteContextSafe
{
    inline CReadWriteWriteContextSafe( CReadWriteLock *theLock )
    {
        if ( theLock )
        {
            theLock->EnterCriticalWriteRegion();
        }
        
        this->theLock = theLock;
    }

    inline ~CReadWriteWriteContextSafe( void )
    {
        if ( CReadWriteLock *theLock = this->theLock )
        {
            theLock->LeaveCriticalWriteRegion();
        }
    }

    CReadWriteLock *theLock;
};

END_NATIVE_EXECUTIVE

#endif //_NATIVE_EXECUTIVE_READ_WRITE_LOCK_