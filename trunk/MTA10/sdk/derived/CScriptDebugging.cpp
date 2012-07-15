/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        derived/CScriptDebugging.cpp
*  PURPOSE:     Script debugging
*  DEVELOPERS:  The_GTA <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#include <StdInc.h>

CScriptDebugging::CScriptDebugging() : ScriptDebugging( *g_pClientGame->GetLuaManager() )
{
    m_triggerCall = false;
}

void CScriptDebugging::NotifySystem( unsigned int level, const filePath& filename, int line, std::string& msg, unsigned char r, unsigned char g, unsigned char b )
{
    if ( !m_triggerCall )
    {
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

#ifdef MTA_DEBUG
    if ( !g_pCore->IsDebugVisible() )
        return;
#endif

    switch( level )
    {
    case 1:
        r = 255, g = 0, b = 0;
        break;
    case 2:
        r = 255, g = 128, b = 0;
        break;
    case 3:
        r = 0, g = 255, b = 0;
        break;
    default:
        r = 180, g = 180, b = 220;
        break;
    }

    g_pCore->DebugEchoColor( msg.c_str(), r, g, b );
}

void CScriptDebugging::PathRelative( const char *in, filePath& out )
{
    modFileRoot->GetRelativePath( in, true, out );
}

bool CScriptDebugging::SetLogfile( const char *filename, unsigned int level )
{
    // Close the previously loaded file
    if ( m_file )
    {
        m_file->Printf( "INFO: Logging to this file ended\n" );
        
        delete m_file;
        m_file = NULL;
    }

    if ( !filename || !*filename )
        return false;

    CFile *file = modFileRoot->Open( filename, "a+" );

    if ( !file )
        return false;

    // Set the new pointer and level and return true
    m_fileLevel = level;
    m_file = file;
    return true;
}