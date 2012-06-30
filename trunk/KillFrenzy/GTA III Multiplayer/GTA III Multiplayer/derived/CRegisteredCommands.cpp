/*****************************************************************************
*
*  PROJECT:     KillFrenzy! 1.0
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        derived/CRegisteredCommands.cpp
*  PURPOSE:     Lua commands registry extension
*  DEVELOPERS:  The_GTA <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#include <StdInc.h>

CRegisteredCommands::CRegisteredCommands( CLuaManager& manager ) : RegisteredCommands( manager )
{
}

CRegisteredCommands::~CRegisteredCommands()
{
}