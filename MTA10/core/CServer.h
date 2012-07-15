/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        core/CServer.h
*  PURPOSE:     Header for server class
*  DEVELOPERS:  Christian Myhre Lundheim <>
*               Stanislav Bobrov <lil_toady@hotmail.com>
*               The_GTA <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#ifndef __CSERVER_H
#define __CSERVER_H

#include "CDynamicLibrary.h"

class CServer : public CServerInterface
{
public:
                                CServer();
                                ~CServer();

    void                        DoPulse();

    bool                        Start( const char* szConfig );
    bool                        Stop();
    bool                        IsStarted();
    inline bool                 IsRunning()                             { return m_library != NULL; };
    inline bool                 IsReady()                               { return m_ready; };

    int                         GetLastError()                          { return m_lastError; };

    bool                        Send( const char* szString );

private:
    static DWORD WINAPI         Thread_EntryPoint( LPVOID pThis );
    unsigned long               Thread_Run();

    CFileTranslator*            m_serverRoot;

    bool                        m_ready;
    HANDLE                      m_thread;
    CDynamicLibrary* volatile   m_lib;
    CCriticalSection            m_criticalSection;

    int                         m_lastError;

    static void                 AddServerOutput( const char* szOutput );
};

#endif
