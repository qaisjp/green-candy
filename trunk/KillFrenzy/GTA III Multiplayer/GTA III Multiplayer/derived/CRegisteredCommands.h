/*****************************************************************************
*
*  PROJECT:     KillFrenzy! 1.0
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        derived/CRegisteredCommands.h
*  PURPOSE:     Lua commands registry extension
*  DEVELOPERS:  The_GTA <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#ifndef _REGISTERED_COMMANDS_
#define _REGISTERED_COMMANDS_

class CRegisteredCommands : public RegisteredCommands
{
public:
                                CRegisteredCommands( class CLuaManager& man );
                                ~CRegisteredCommands();
};

#endif //_REGISTERED_COMMANDS_