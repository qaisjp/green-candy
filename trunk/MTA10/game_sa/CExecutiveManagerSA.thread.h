/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        game_sa/CExecutiveManagerSA.thread.h
*  PURPOSE:     Thread abstraction layer for MTA
*  DEVELOPERS:  Martin Turski <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#ifndef _EXECUTIVE_MANAGER_THREADS_
#define _EXECUTIVE_MANAGER_THREADS_

#define THREAD_PLUGIN_NATIVE        0x00000000  // plugin for OS implementation

enum eThreadStatus
{
    THREAD_SUSPENDED,
    THREAD_RUNNING,
    THREAD_TERMINATED
};

class CExecThreadSA
{
public:
    typedef ExecutiveManager::threadPluginContainer_t threadPluginContainer_t;

    friend class CExecutiveManagerSA;
    friend class threadPluginContainer_t;

    typedef void (__stdcall*threadEntryPoint_t)( CExecThreadSA *thisThread, void *userdata );

    CExecThreadSA( CExecutiveManagerSA *manager, bool isRemoteThread );
    ~CExecThreadSA( void );

    eThreadStatus GetStatus( void ) const;

    bool Terminate( void );
    
    bool Suspend( void );
    bool Resume( void );

    threadEntryPoint_t entryPoint;
    void *userdata;
    size_t stackSize;

    bool isRemoteThread;

private:
    CExecutiveManagerSA *manager;

    RwListEntry <CExecThreadSA> managerNode;

    // Make available some memory for plugins.
    // This is to abstract away implementation defined structures from this header.
    typedef StaticMemoryAllocator <256> pluginMemory_t;

    pluginMemory_t pluginMemory;

    ExecutiveManager::threadPluginContainer_t::pluginSentry_t pluginSentry;
};

#endif //_EXECUTIVE_MANAGER_THREADS_