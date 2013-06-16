/*****************************************************************************
*
*  PROJECT:     Lua Interpreter
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        derived/CResource.cpp
*  PURPOSE:     Lua resource class
*  DEVELOPERS:  Martin Turski <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#include <StdInc.h>

CResource::CResource( CLuaMain& main, unsigned short id, const filePath& name, CFileTranslator& root ) : Resource( main, id, name, root )
{
}

CResource::~CResource()
{
}