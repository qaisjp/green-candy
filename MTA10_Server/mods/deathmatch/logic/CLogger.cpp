/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.0
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        mods/deathmatch/logic/CLogger.cpp
*  PURPOSE:     Server logger class
*  DEVELOPERS:  Christian Myhre Lundheim <>
*               The_GTA <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#include "StdInc.h"

using namespace std;

CFile* CLogger::m_log = NULL;
CFile* CLogger::m_authLog = NULL;
eLogLevel CLogger::m_MinLogLevel = LOGLEVEL_LOW;
bool CLogger::m_bPrintingDots = false;
SString CLogger::m_strCaptureBuffer;
bool CLogger::m_bCaptureConsole = false;
CCriticalSection CLogger::m_CaptureBufferMutex;

#define MAX_STRING_LENGTH 2048
void CLogger::LogPrintf( const char* szFormat, ... )
{
    // Compose the formatted message
    char szBuffer[MAX_STRING_LENGTH];
    va_list marker;
    va_start( marker, szFormat );
    VSNPRINTF( szBuffer, MAX_STRING_LENGTH, szFormat, marker );
    va_end( marker );

    // Timestamp and send to the console and logfile
    HandleLogPrint( true, "", szBuffer, true, true, false );
}

void CLogger::LogPrint( const char* szText )
{
    // Timestamp and send to the console and logfile
    HandleLogPrint( true, "", szText, true, true, false );
}

// As above, but with a log level
void CLogger::LogPrintf( eLogLevel logLevel, const char* szFormat, ... )
{
    // Compose the formatted message
    char szBuffer[MAX_STRING_LENGTH];
    va_list marker;
    va_start( marker, szFormat );
    VSNPRINTF( szBuffer, MAX_STRING_LENGTH, szFormat, marker );
    va_end( marker );

    // Timestamp and send to the console and logfile
    HandleLogPrint( true, "", szBuffer, true, true, false, logLevel );
}

void CLogger::LogPrint( eLogLevel logLevel, const char* szText )
{
    // Timestamp and send to the console and logfile
    HandleLogPrint( true, "", szText, true, true, false, logLevel );
}

void CLogger::LogPrintfNoStamp( const char* szFormat, ... )
{
    // Compose the formatted message
    char szBuffer[MAX_STRING_LENGTH];
    va_list marker;
    va_start( marker, szFormat );
    VSNPRINTF( szBuffer, MAX_STRING_LENGTH, szFormat, marker );
    va_end( marker );

    // Send to the console and logfile
    HandleLogPrint( false, "", szBuffer, true, true, false );
}

void CLogger::LogPrintNoStamp( const char* szText )
{
    // Send to the console and logfile
    HandleLogPrint( false, "", szText, true, true, false );
}

void CLogger::ErrorPrintf( const char* szFormat, ... )
{
    // Compose the formatted message
    char szBuffer[MAX_STRING_LENGTH];
    va_list marker;
    va_start( marker, szFormat );
    VSNPRINTF( szBuffer, MAX_STRING_LENGTH, szFormat, marker );
    va_end( marker );

    // Timestamp and send to the console and logfile
    HandleLogPrint( true, "ERROR: ", szBuffer, true, true, false );
}

void CLogger::DebugPrintf( const char* szFormat, ... )
{
#ifdef MTA_DEBUG
    // Compose the formatted message
    char szBuffer[MAX_STRING_LENGTH];
    va_list marker;
    va_start( marker, szFormat );
    VSNPRINTF( szBuffer, MAX_STRING_LENGTH, szFormat, marker );
    va_end( marker );

    // Timestamp and send to the console and logfile
    HandleLogPrint( true, "DEBUG: ", szBuffer, true, true, false );
#endif
}

void CLogger::AuthPrintf( const char* szFormat, ... )
{
    // Compose the formatted message
    char szBuffer[MAX_STRING_LENGTH];
    va_list marker;
    va_start( marker, szFormat );
    VSNPRINTF( szBuffer, MAX_STRING_LENGTH, szFormat, marker );
    va_end( marker );

    // Timestamp and send to the console, logfile and authfile
    HandleLogPrint( true, "", szBuffer, true, true, true );
}

bool CLogger::SetLogFile( const char *filename )
{
    // Eventually delete our current file
    if ( m_file )
    {
        delete m_file;
        m_file = NULL;
    }

    if ( !filename || !filename[0] )
        return true;

    m_file = modFileRoot->Open( filename, "a+" );
    return m_file != NULL;
}

bool CLogger::SetAuthFile( const char *filename )
{
    // Eventually delete our current file
    if ( m_authLog )
    {
        delete m_authLog;
        m_authLog = NULL;
    }

    if ( !filename || !filename[0] )
        return true;

    m_authLog = modFileRoot->Open( filename, "a+" );
    return m_authLog != NULL;
}

void CLogger::SetMinLogLevel( eLogLevel logLevel )
{
    m_MinLogLevel = logLevel;
}

void CLogger::ProgressDotsBegin()
{
    m_bPrintingDots = true;
}

void CLogger::ProgressDotsUpdate()
{
    if ( !m_bPrintingDots )
        return;

    HandleLogPrint( false, "", ".", true, true, false );
}

void CLogger::ProgressDotsEnd()
{
    if ( !m_bPrintingDots )
        return;

    m_bPrintingDots = false;
    HandleLogPrint( false, "", "\n", true, true, false );
}

void CLogger::BeginConsoleOutputCapture()
{
    m_CaptureBufferMutex.Lock();

    m_strCaptureBuffer.clear();
    m_bCaptureConsole = true;

    m_CaptureBufferMutex.Unlock();
}

SString CLogger::EndConsoleOutputCapture()
{
    m_CaptureBufferMutex.Lock();

    SString strTemp = m_strCaptureBuffer;
    m_bCaptureConsole = false;
    m_strCaptureBuffer.clear();

    m_CaptureBufferMutex.Unlock();
    return strTemp;
}

// Handle where to send the message
void CLogger::HandleLogPrint( bool bTimeStamp, const char* szPrePend, const char* szMessage, bool bToConsole, bool bToLogFile, bool bToAuthFile, eLogLevel logLevel )
{
    if ( logLevel < m_MinLogLevel )
        return;

    // Handle interruption of progress dots
    if ( m_bPrintingDots && ( bTimeStamp || strlen( szPrePend ) != 0 || strlen( szMessage ) > 1 || szMessage[0] != '.' ) )
        ProgressDotsEnd();

    // Put the timestamp at the beginning of the string
    string strOutputShort;
    string strOutputLong;

    if ( bTimeStamp )
    {
        char szBuffer[MAX_STRING_LENGTH] = { "\0" };
        time_t timeNow;
        time( &timeNow );
        tm* pCurrentTime = localtime( &timeNow );

        if ( !strftime( szBuffer, MAX_STRING_LENGTH - 1, "[%H:%M:%S] ", pCurrentTime ) )
            szBuffer[0] = 0;

        strOutputShort = szBuffer;

        if ( !strftime( szBuffer, MAX_STRING_LENGTH - 1, "[%Y-%m-%d %H:%M:%S] ", pCurrentTime ) )
            szBuffer[0] = 0;

        strOutputLong = szBuffer;
    }

    // Build the final string
    strOutputShort = strOutputShort + szPrePend + szMessage;
    strOutputLong = strOutputLong + szPrePend + szMessage;

    if ( bToConsole )
        g_pServerInterface->Printf( "%s", strOutputShort.c_str() );

    if ( bToConsole && m_bCaptureConsole )
    {
        m_CaptureBufferMutex.Lock();
        m_strCaptureBuffer += szPrePend;
        m_strCaptureBuffer += szMessage;

        if ( m_strCaptureBuffer.length() > 1000 )
            m_bCaptureConsole = false;

        m_CaptureBufferMutex.Unlock();
    }

    if ( bToLogFile && m_file )
    {
        m_file->Printf( "%s", strOutputLong.c_str() );
        m_file->Flush();
    }

    if ( bToAuthFile && m_authLog )
    {
        m_authLog->Printf( "%s", strOutputLong.c_str() );
        m_authLog->Flush();
    }
}
