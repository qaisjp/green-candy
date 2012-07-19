/*****************************************************************************
*
*  PROJECT:     KillFrenzy! 1.0
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        derived/CResource.cpp
*  PURPOSE:     Lua Resource environment
*  DEVELOPERS:  The_GTA <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#include <StdInc.h>

CResource::CResource( LuaMain& main, unsigned short id, const filePath& name, CFileTranslator& root ) : Resource( main, id, name, root )
{
}

CResource::~CResource()
{
}