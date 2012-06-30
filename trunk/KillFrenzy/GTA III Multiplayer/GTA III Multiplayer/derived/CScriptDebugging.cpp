/*****************************************************************************
*
*  PROJECT:     KillFrenzy! 1.0
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        derived/CScriptDebugging.cpp
*  PURPOSE:     Game Script Debugging class
*  DEVELOPERS:  The_GTA <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#include <StdInc.h>

CScriptDebugging::CScriptDebugging( CLuaManager& manager ) : ScriptDebugging( manager )
{
}

void CScriptDebugging::NotifySystem( unsigned int level, const filePath& filename, int line, std::string& msg, unsigned char r, unsigned char g, unsigned char b )
{
    Console_Printf( "%s\n", 0xFFFFFFFF, msg.c_str() );
}

void CScriptDebugging::PathRelative( const char *in, filePath& out )
{
    modFileRoot->GetRelativePath( in, true, out );
}

bool CScriptDebugging::SetLogfile( const char *filename, unsigned int level )
{
    // Close the previously loaded file
    CloseLogFile();

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