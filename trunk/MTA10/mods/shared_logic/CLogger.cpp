/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.0
*               (Shared logic for modifications)
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        mods/shared_logic/CLogger.cpp
*  PURPOSE:     Logger class
*  DEVELOPERS:  Jax <>
*               Oliver Brown <>
*               Kevin Whiteside <kevuwk@gmail.com>
*
*****************************************************************************/

#include <StdInc.h>

using namespace std;

FILE* CLogger::m_pLogFile = NULL;

#define MAX_STRING_LENGTH 2048

void CLogger::LogPrintf( const char* szFormat, ... )
{
    // Compose the formatted message
    char szBuffer[MAX_STRING_LENGTH];
    va_list marker;
    va_start( marker, szFormat );
    VSNPRINTF( szBuffer, MAX_STRING_LENGTH, szFormat, marker );
    va_end( marker );

    // Timestamp and send to the logfile
    HandleLogPrint( true, "", szBuffer, false, true );
}


void CLogger::LogPrint( const char* szText )
{
    // Timestamp and send to the logfile
    HandleLogPrint( true, "", szText, false, true );
}

#if 0   // Currently unused
void CLogger::LogPrintfNoStamp( const char* szFormat, ... )
{
    // Compose the formatted message
    char szBuffer [MAX_STRING_LENGTH];
    va_list marker;
    va_start ( marker, szFormat );
    VSNPRINTF ( szBuffer, MAX_STRING_LENGTH, szFormat, marker );
    va_end ( marker );

    // Send to the console and logfile
    HandleLogPrint ( false, "", szBuffer, true, true );
}


void CLogger::LogPrintNoStamp ( const char* szText )
{
    // Send to the console and logfile
    HandleLogPrint ( false, "", szText, true, true );
}
#endif

void CLogger::ErrorPrintf( const char* szFormat, ... )
{
    // Compose the formatted message
    char szBuffer[MAX_STRING_LENGTH];
    va_list marker;
    va_start( marker, szFormat );
    VSNPRINTF( szBuffer, MAX_STRING_LENGTH, szFormat, marker );
    va_end( marker );

    // Timestamp and send to the console and logfile
    HandleLogPrint( true, "ERROR: ", szBuffer, true, true );
}

#if 0   // Currently unused
void CLogger::DebugPrintf( const char* szFormat, ... )
{
    #ifdef MTA_DEBUG
        // Compose the formatted message
        char szBuffer [MAX_STRING_LENGTH];
        va_list marker;
        va_start ( marker, szFormat );
        VSNPRINTF ( szBuffer, MAX_STRING_LENGTH, szFormat, marker );
        va_end ( marker );

        // Timestamp and send to the console and logfile
        HandleLogPrint ( true, "DEBUG: ", szBuffer, true, true );
    #endif
}
#endif

void CLogger::SetLogFile( const char *filename )
{
    // Eventually delete our current file
    if ( m_file )
    {
        delete m_file;
        m_file = NULL;
    }

    if ( !szLogFile )
        return;

    m_file = modFileRoot->Open( filename, "a+" );
}

// Handle where to send the message
void CLogger::HandleLogPrint( bool bTimeStamp, const char *szPrePend, const char *szMessage, bool console, bool logFile )
{
    // Put the timestamp at the beginning of the string
    string strOutputShort;
    string strOutputLong;

    if ( bTimeStamp )
    {
        char szBuffer[MAX_STRING_LENGTH] = { "\0" };
        time_t timeNow;
        time( &timeNow );
        tm* pCurrentTime = localtime( &timeNow );

#if 0
        if ( !strftime( szBuffer, MAX_STRING_LENGTH - 1, "[%H:%M:%S] ", pCurrentTime ) )
            szBuffer[0] = 0;

        strOutputShort = szBuffer;
#endif
        if ( !strftime( szBuffer, MAX_STRING_LENGTH - 1, "[%Y-%m-%d %H:%M:%S] ", pCurrentTime ) )
            szBuffer[0] = 0;

        strOutputLong = szBuffer;
    }

    // Build the final string
    strOutputShort = strOutputShort + szPrePend + szMessage;
    strOutputLong = strOutputLong + szPrePend + szMessage;

    if ( console )
        g_pCore->GetConsole()->Print ( strOutputShort.c_str () );

    // Note: m_pLogFile is never set, so this always does nothing
    if ( logFile && m_file )
    {
        file->Printf( "%s", strOutputLong.c_str () );
        file->Flush();
    }
}
