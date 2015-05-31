/*****************************************************************************
*
*  PROJECT:     Native Executive
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        NativeExecutive/CExecutiveManager.thread.h
*  PURPOSE:     Thread abstraction layer for MTA
*  DEVELOPERS:  Martin Turski <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#ifndef _EXECUTIVE_MANAGER_THREADS_
#define _EXECUTIVE_MANAGER_THREADS_

BEGIN_NATIVE_EXECUTIVE

#define THREAD_PLUGIN_NATIVE        0x00000000      // plugin id for OS implementation

enum eThreadStatus
{
    THREAD_SUSPENDED,
    THREAD_RUNNING,
    THREAD_TERMINATING,
    THREAD_TERMINATED
};

class CExecThread
{
public:
    typedef ExecutiveManager::threadPluginContainer_t threadPluginContainer_t;

    friend class CExecutiveManager;

    typedef void (__stdcall*threadEntryPoint_t)( CExecThread *thisThread, void *userdata );

    CExecThread( CExecutiveManager *manager, bool isRemoteThread, void *userdata, size_t stackSize, threadEntryPoint_t entryPoint );
    ~CExecThread( void );

    eThreadStatus GetStatus( void ) const;

    bool Terminate( void );
    
    bool Suspend( void );
    bool Resume( void );

    bool IsCurrent( void );

    void Lock( void );
    void Unlock( void );

    // These parameters are only valid if this thread is not a remote thread!
    threadEntryPoint_t entryPoint;
    void *userdata;
    size_t stackSize;

    bool isRemoteThread;

private:
    CExecutiveManager *manager;

public:
    RwListEntry <CExecThread> managerNode;
};

END_NATIVE_EXECUTIVE

#endif //_EXECUTIVE_MANAGER_THREADS_