/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        Shared/logic/ScriptDebugging.cpp
*  PURPOSE:     Script debugging base
*  DEVELOPERS:  Jax <>
*               Kevin Whiteside <kevuwk@gmail.com>
*               Ed Lyons <eai@opencoding.net>
*               Derek Abdine <>
*               Chris McArthur <>
*               Christian Myhre Lundheim <>
*               The_GTA <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#include <StdInc.h>

#define MAX_STRING_LENGTH 2048

ScriptDebugging::ScriptDebugging()
{
    m_fileLevel = 0;
    m_file = NULL;
}

void ScriptDebugging::LogCustom( lua_State *lua, unsigned char red, unsigned char green, unsigned char blue, const char *fmt, ... )
{
    char szBuffer[MAX_STRING_LENGTH];
    va_list marker;
    va_start( marker, fmt );
    VSNPRINTF( szBuffer, MAX_STRING_LENGTH, fmt, marker );
    va_end( marker );

    LogString( "", lua, szBuffer, 0, red, green, blue );
}

void ScriptDebugging::LogInformation( lua_State *lua, const char *fmt, ... )
{
    char szBuffer[MAX_STRING_LENGTH];
    va_list marker;
    va_start( marker, fmt );
    VSNPRINTF( szBuffer, MAX_STRING_LENGTH, fmt, marker );
    va_end( marker );

    LogString( "INFO: ", lua, szBuffer, 3 );
}

void ScriptDebugging::LogWarning( lua_State *lua, const char *fmt, ... )
{
    char szBuffer[MAX_STRING_LENGTH];
    va_list marker;
    va_start( marker, fmt );
    VSNPRINTF( szBuffer, MAX_STRING_LENGTH, fmt, marker );
    va_end( marker );

    LogString( "WARNING: ", lua, szBuffer, 2 );
}

void ScriptDebugging::LogError( lua_State *lua, const char *fmt, ... )
{
    // Compose the formatted message
    char szBuffer[MAX_STRING_LENGTH];
    va_list marker;
    va_start( marker, fmt );
    VSNPRINTF( szBuffer, MAX_STRING_LENGTH, fmt, marker );
    va_end( marker );

    // Log it
    LogString( "ERROR: ", lua, szBuffer, 1 );
}

void ScriptDebugging::LogBadPointer( lua_State *lua, const char *func, const char *argType, unsigned int argID )
{
    LogWarning( lua, "Bad '%s' pointer @ '%s'(%u)", argType, func, argID );
}

void ScriptDebugging::LogBadType( lua_State *lua, const char *func )
{
    LogWarning( lua, "Bad argument @ '%s'", func );
}

void ScriptDebugging::LogCustom( lua_State *lua, const char *msg )
{
    LogWarning( lua, "%s", msg );
}

void ScriptDebugging::LogBadLevel( lua_State *lua, const char *func, unsigned int level )
{
    LogWarning( lua, "Requires level '%d' @ '%s", level, func );
}

void ScriptDebugging::LogString( const char *pre, lua_State *lua, const char *msg, unsigned int minLevel, unsigned char red, unsigned char green, unsigned char blue )
{
    SString strText;
    lua_Debug debugInfo;

    // Initialize values for onClientDebugMessage
    SString strMsg = msg;
    SString strFile;
    int iLine = -1;

    if ( lua && lua_getstack( lua, 1, &debugInfo ) )
    {
        lua_getinfo( lua, "nlS", &debugInfo );

        // Make sure this function isn't defined in a string (eg: from runcode)
        if ( debugInfo.source[0] == '@' )
        {
            // Get and store the location of the debug message
            strFile = PathRelative( debugInfo.source + 1 );
            iLine = debugInfo.currentline;

            // Populate a message to print/send (unless "info" type)
            if ( minLevel < 3 )
                strText = SString( "%s%s:%d: %s", pre, strFile.c_str(), debugInfo.currentline, msg );
        }
        else
        {
            strFile = debugInfo.short_src;

            if ( minLevel < 3 )
                strText = SString( "%s%s %s", pre, msg, strFile.c_str() );
        }

        // Create a different message if type is "INFO"
        if ( minLevel > 2 )
            strText = SString( "%s%s", pre, msg );
    }
    else
        strText = SString( "%s%s", pre, msg );

    NotifySystem( strFile, lua, iLine, strMsg, red, green, blue );

    // Log it to the file if enough level
    if ( m_fileLevel >= minLevel )
        PrintLog( strText );
}

void ScriptDebugging::PrintLog( const char* szText )
{
    // Got a logfile?
    if ( !m_file )
        return;

    // Log it, timestamped
    char szBuffer[64];
    time_t timeNow;
    time( &timeNow );
    strftime( szBuffer, 32, "[%Y-%m-%d %H:%M:%S]", localtime( &timeNow ) );

    m_file->Printf( "%s %s\n", szBuffer, szText );
    m_file->Flush();
}
