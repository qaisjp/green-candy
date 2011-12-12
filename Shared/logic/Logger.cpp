/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.0
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        Shared/logic/Logger.cpp
*  PURPOSE:     Logger base class
*  DEVELOPERS:  Christian Myhre Lundheim <>
*               The_GTA <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#include <StdInc.h>

using namespace std;

FILE* Logger::m_file = NULL;

#define MAX_STRING_LENGTH 2048

void Logger::LogPrintf( const char* szFormat, ... )
{
    // Compose the formatted message
    char szBuffer[MAX_STRING_LENGTH];
    va_list marker;
    va_start( marker, szFormat );
    VSNPRINTF( szBuffer, MAX_STRING_LENGTH, szFormat, marker );
    va_end( marker );

    LogPrint( szBuffer );
}

void Logger::LogPrint( const char* szText )
{
    // Timestamp and send to the logfile
    HandleLogPrint( true, "", szText );
}

void Logger::LogPrintfNoStamp( const char* szFormat, ... )
{
    // Compose the formatted message
    char szBuffer[MAX_STRING_LENGTH];
    va_list marker;
    va_start( marker, szFormat );
    VSNPRINTF( szBuffer, MAX_STRING_LENGTH, szFormat, marker );
    va_end( marker );

    LogPrintNoStamp( szBuffer );
}

void Logger::LogPrintNoStamp( const char* szText )
{
    // Send to the console and logfile
    HandleLogPrint( false, "", szText );
}

void Logger::ErrorPrintf( const char* szFormat, ... )
{
    // Compose the formatted message
    char szBuffer[MAX_STRING_LENGTH];
    va_list marker;
    va_start( marker, szFormat );
    VSNPRINTF( szBuffer, MAX_STRING_LENGTH, szFormat, marker );
    va_end( marker );

    // Timestamp and send to the console and logfile
    HandleLogPrint( true, "ERROR: ", szBuffer );
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
    HandleLogPrint( true, "DEBUG: ", szBuffer );
#endif
}

void CLogger::SetLogFile( const char *filename )
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

void Logger::BuildEntry( bool time, const char *pre, const char *msg, std::string *shrt, std::string *lng )
{
    if ( time )
    {
        char szBuffer[MAX_STRING_LENGTH];
        time_t timeNow;
        time( &timeNow );
        tm *pCurrentTime = localtime( &timeNow );

        if ( shrt )
        {
            if ( !strftime( szBuffer, MAX_STRING_LENGTH - 1, "[%H:%M:%S] ", pCurrentTime ) )
                szBuffer[0] = 0;

            *shrt = szBuffer;
        }

        if ( lng )
        {
            if ( !strftime( szBuffer, MAX_STRING_LENGTH - 1, "[%Y-%m-%d %H:%M:%S] ", pCurrentTime ) )
                szBuffer[0] = 0;

            *lng = szBuffer;
        }
    }

    // Build the final string
    if ( shrt )
        *shrt += pre + msg;

    if ( lng )
        *lng += pre + msg;
}

// Handle where to send the message
void Logger::HandleLogPrint( bool time, const char *pre, const char *msg )
{
    if ( !m_file )
        return;

    // Put the timestamp at the beginning of the string
    string outLong;

    BuildEntry( time, pre, msg, NULL, &outLong );

    m_file->Printf( "%s", strOutputLong.c_str () );
    m_file->Flush();
}
