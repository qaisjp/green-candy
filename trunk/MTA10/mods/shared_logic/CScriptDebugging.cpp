/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.0
*               (Shared logic for modifications)
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        mods/shared_logic/CScriptDebugging.cpp
*  PURPOSE:     Script debugging class
*  DEVELOPERS:  Jax <>
*               Kevin Whiteside <kevuwk@gmail.com>
*               Ed Lyons <eai@opencoding.net>
*               Derek Abdine <>
*               Chris McArthur <>
*               Christian Myhre Lundheim <>
*               The_GTA <quiret@gmx.de>
*
*****************************************************************************/

#include <StdInc.h>

#define MAX_STRING_LENGTH 2048

CScriptDebugging::CScriptDebugging( CLuaManager *pLuaManager )
{
    m_pLuaManager = pLuaManager;
    m_uiLogFileLevel = 0;
    m_file = NULL;
    m_bTriggeringOnClientDebugMessage = false;
}

#if 0   // Currently unused
void CScriptDebugging::OutputDebugInfo ( lua_State* luaVM, int iLevel, unsigned char ucRed, unsigned char ucGreen, unsigned char ucBlue )
{
    lua_Debug debugInfo;
    if ( lua_getstack ( luaVM, 1, &debugInfo ) )
    {
        lua_getinfo ( luaVM, "nlS", &debugInfo );
        
        // first version includes script path - makes much longer though
        //  strDebugDump = SString::Printf ( "Line: %d (%s + %d) %s", debugInfo.currentline, debugInfo.name, debugInfo.currentline - debugInfo.linedefined, debugInfo.short_src );
        SString strDebugDump ( "Line: %d (%s + %d)", debugInfo.currentline, debugInfo.name, debugInfo.currentline - debugInfo.linedefined );

        LogString ( strDebugDump, iLevel, ucRed, ucGreen, ucBlue );
    }
}
#endif

void CScriptDebugging::LogCustom( lua_State* luaVM, unsigned char ucRed, unsigned char ucGreen, unsigned char ucBlue, const char* szFormat, ... )
{
    // Compose the formatted message
    char szBuffer[MAX_STRING_LENGTH];
    va_list marker;
    va_start( marker, szFormat );
    VSNPRINTF( szBuffer, MAX_STRING_LENGTH, szFormat, marker );
    va_end( marker );

    LogString( "", luaVM, szBuffer, 0, ucRed, ucGreen, ucBlue );
}

void CScriptDebugging::LogInformation( lua_State* luaVM, const char* szFormat, ... )
{
    // Compose the formatted message
    char szBuffer[MAX_STRING_LENGTH];
    va_list marker;
    va_start( marker, szFormat );
    VSNPRINTF( szBuffer, MAX_STRING_LENGTH, szFormat, marker );
    va_end( marker );

    // Log it
    LogString( "INFO: ", luaVM, szBuffer, 3 );
}

void CScriptDebugging::LogWarning( lua_State* luaVM, const char* szFormat, ... )
{
    // Compose the formatted message
    char szBuffer[MAX_STRING_LENGTH];
    va_list marker;
    va_start( marker, szFormat );
    VSNPRINTF( szBuffer, MAX_STRING_LENGTH, szFormat, marker );
    va_end( marker );

    // Log it
    LogString( "WARNING: ", luaVM, szBuffer, 2 );
}

void CScriptDebugging::LogError( lua_State* luaVM, const char* szFormat, ... )
{
    // Compose the formatted message
    char szBuffer[MAX_STRING_LENGTH];
    va_list marker;
    va_start( marker, szFormat );
    VSNPRINTF( szBuffer, MAX_STRING_LENGTH, szFormat, marker );
    va_end( marker );

    // Log it
    LogString( "ERROR: ", luaVM, szBuffer, 1 );
}

void CScriptDebugging::NotifySystem( filePath& filename, int line, std::string& msg )
{
    if ( m_triggerCall )
        return;

    m_triggerCall = true;

    // Prepare onDebugMessage
    CLuaArguments Arguments;
    Arguments.PushString( msg.c_str() );
    Arguments.PushNumber( 1 );

    // Push the file name (if any)
    if ( !filename.empty() )
        Arguments.PushString( filename.c_str() );
    else
        Arguments.PushNil();

    // Push the line (if any)
    if ( line != -1 )
        Arguments.PushNumber( line );
    else
        Arguments.PushNil();
    
    // Call onDebugMessage
    g_pClientGame->GetRootEntity()->CallEvent( "onClientDebugMessage", Arguments, false );

    m_triggerCall = false;
}

void CScriptDebugging::LogError( SString strFile, int iLine, SString strMsg )
{
    SString strText = SString( "ERROR: %s:%d: %s", strFile.c_str (), iLine, strMsg.c_str () );

    NotifySystem( strFile, iLine, strMsg );

    // Log it to the file if enough level
    if ( m_uiLogFileLevel >= 1 )
        PrintLog( strText );

    // Log to console
    g_pCore->DebugEchoColor( strText, 255, 0, 0 );
}

void CScriptDebugging::LogBadPointer ( lua_State* luaVM, const char* szFunction, const char* szArgumentType, unsigned int uiArgument )
{
    // Populate a message to print/send
    LogWarning ( luaVM, "Bad '%s' pointer @ '%s'(%u)", szArgumentType, szFunction, uiArgument );
}

void CScriptDebugging::LogBadType ( lua_State* luaVM, const char* szFunction )
{
    // Populate a message to print/send
    LogWarning ( luaVM, "Bad argument @ '%s'", szFunction );
}

void CScriptDebugging::LogCustom ( lua_State* luaVM, const char* szMessage )
{
    // Populate a message to print/send
    LogWarning ( luaVM, "%s", szMessage );
}

void CScriptDebugging::LogBadLevel ( lua_State* luaVM, const char* szFunction, unsigned int uiRequiredLevel )
{
    // Populate a message to print/send
    LogWarning ( luaVM, "Requires level '%d' @ '%s", uiRequiredLevel, szFunction );
}

bool CScriptDebugging::SetLogfile( const char *filename, unsigned int level )
{
    // Try to load the new file
    CFile *file = modFileRoot->Open( filename, "a+" );

    if ( !file )
        return false;

    // Close the previously loaded file
    if ( m_file )
    {
        m_file->Printf( "INFO: Logging to this file ended\n" );
        
        delete m_file;
    }

    // Set the new pointer and level and return true
    m_uiLogFileLevel = level;
    m_file = file;
    return true;
}

void CScriptDebugging::LogString( const char* szPrePend, lua_State* luaVM, const char* szMessage, unsigned int uiMinimumDebugLevel, unsigned char ucRed, unsigned char ucGreen, unsigned char ucBlue )
{
    SString strText;
    lua_Debug debugInfo;

    // Initialize values for onClientDebugMessage
    SString strMsg  = szMessage;
    SString strFile = "";
    int     iLine   = -1;

    if ( luaVM && lua_getstack( luaVM, 1, &debugInfo ) )
    {
        lua_getinfo( luaVM, "nlS", &debugInfo );

        // Make sure this function isn't defined in a string (eg: from runcode)
        if ( debugInfo.source[0] == '@' )
        {
            // Get and store the location of the debug message
            strFile = ConformResourcePath( debugInfo.source );
            iLine   = debugInfo.currentline;

            // Populate a message to print/send (unless "info" type)
            if ( uiMinimumDebugLevel < 3 )
                strText = SString( "%s%s:%d: %s", szPrePend, strFile.c_str(), debugInfo.currentline, szMessage );
        }
        else
        {
            strFile = debugInfo.short_src;

            if ( uiMinimumDebugLevel < 3 )
                strText = SString( "%s%s %s", szPrePend, szMessage, strFile.c_str() );
        }
    }
    else
        strText = SString( "%s%s", szPrePend, szMessage );

    // Create a different message if type is "INFO"
    if ( uiMinimumDebugLevel > 2 )
        strText = SString( "%s%s", szPrePend, szMessage );

    NotifySystem( strFile, iLine, strMsg );

    // Log it to the file if enough level
    if ( m_uiLogFileLevel >= uiMinimumDebugLevel )
        PrintLog( strText );

    switch( uiMinimumDebugLevel )
    {
    case 1:
        ucRed = 255, ucGreen = 0, ucBlue = 0;
        break;
    case 2:
        ucRed = 255, ucGreen = 128, ucBlue = 0;
        break;
    case 3:
        ucRed = 0, ucGreen = 255, ucBlue = 0;
        break;
    }

#ifdef MTA_DEBUG
    if ( !g_pCore->IsDebugVisible() )
        return;
#endif

    g_pCore->DebugEchoColor( strText, ucRed, ucGreen, ucBlue );
}

void CScriptDebugging::PrintLog( const char* szText )
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
