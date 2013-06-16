/*****************************************************************************
*
*  PROJECT:     Lua Interpreter
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        win32/win32_main.h
*  PURPOSE:     Win32 MTA:Lua main logic
*  DEVELOPERS:  Martin Turski <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#ifndef _WIN32_MAIN_HEADER_
#define _WIN32_MAIN_HEADER_

void luawin32_open( lua_State *L );
void luawin32_pulse();

#endif //_WIN32_MAIN_HEADER_