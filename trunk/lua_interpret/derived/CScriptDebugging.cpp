/*****************************************************************************
*
*  PROJECT:     Lua Interpreter
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        derived/CScriptDebugging.cpp
*  PURPOSE:     Lua script debugging system
*  DEVELOPERS:  Martin Turski <quiret@gmx.de>
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
    printf( "%s\n", msg.c_str() );
}

void CScriptDebugging::PathRelative( const char *in, filePath& out )
{
    fileRoot->GetRelativePath( in, true, out );
}

bool CScriptDebugging::SetLogfile( const char *filename, unsigned int level )
{
    // Close the previously loaded file
    CloseLogFile();

    if ( !filename || !*filename )
        return false;

    CFile *file = fileRoot->Open( filename, "a+" );

    if ( !file )
        return false;

    // Set the new pointer and level and return true
    m_fileLevel = level;
    m_file = file;
    return true;
}