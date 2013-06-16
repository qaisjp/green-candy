/*****************************************************************************
*
*  PROJECT:     Lua Interpreter
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        derived/CLuaMain.cpp
*  PURPOSE:     Lua hyperstructure class
*  DEVELOPERS:  Martin Turski <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#include <StdInc.h>

CLuaMain::CLuaMain( CLuaManager& man, CFileTranslator& root ) : LuaMain( man ), m_fileRoot( root )
{
}

CLuaMain::~CLuaMain()
{
}

bool CLuaMain::ParseRelative( const char *in, filePath& out ) const
{
    return m_fileRoot.GetRelativePath( in, true, out );
}