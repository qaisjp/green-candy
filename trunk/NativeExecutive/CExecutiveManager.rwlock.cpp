/*****************************************************************************
*
*  PROJECT:     Native Executive
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        NativeExecutive/CExecutiveManager.rwlock.cpp
*  PURPOSE:     Read/Write lock synchronization object
*  DEVELOPERS:  Martin Turski <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#include "StdInc.h"

BEGIN_NATIVE_EXECUTIVE

void CReadWriteLock::EnterCriticalReadRegion( void )
{
    ((CReadWriteLockNative*)this)->EnterCriticalReadRegionNative();
}

void CReadWriteLock::LeaveCriticalReadRegion( void )
{
    ((CReadWriteLockNative*)this)->LeaveCriticalReadRegionNative();
}

void CReadWriteLock::EnterCriticalWriteRegion( void )
{
    ((CReadWriteLockNative*)this)->EnterCriticalWriteRegionNative();
}

void CReadWriteLock::LeaveCriticalWriteRegion( void )
{
    ((CReadWriteLockNative*)this)->LeaveCriticalWriteRegionNative();
}

bool CReadWriteLock::TryEnterCriticalReadRegion( void )
{
    return ((CReadWriteLockNative*)this)->TryEnterCriticalReadRegionNative();
}

bool CReadWriteLock::TryEnterCriticalWriteRegion( void )
{
    return ((CReadWriteLockNative*)this)->TryEnterCriticalWriteRegionNative();
}

// Executive manager API.
CReadWriteLock* CExecutiveManager::CreateReadWriteLock( void )
{
    return new CReadWriteLockNative();
}

void CExecutiveManager::CloseReadWriteLock( CReadWriteLock *theLock )
{
    CReadWriteLockNative *lockImpl = (CReadWriteLockNative*)theLock;

    delete lockImpl;
}

size_t CExecutiveManager::GetReadWriteLockStructSize( void )
{
    return sizeof( CReadWriteLockNative );
}

CReadWriteLock* CExecutiveManager::CreatePlacedReadWriteLock( void *mem )
{
    return new (mem) CReadWriteLockNative();
}

void CExecutiveManager::ClosePlacedReadWriteLock( CReadWriteLock *placedLock )
{
    CReadWriteLockNative *lockImpl = (CReadWriteLockNative*)placedLock;

    lockImpl->~CReadWriteLockNative();
}

END_NATIVE_EXECUTIVE