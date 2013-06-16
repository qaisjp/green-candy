/*****************************************************************************
*
*  PROJECT:     Lua Interpreter
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        win32/dialogs/win32_dlg.h
*  PURPOSE:     Win32 MTA:Lua dialog logic
*  DEVELOPERS:  Martin Turski <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#ifndef _WIN32_DIALOGS_
#define _WIN32_DIALOGS_

#define LUACLASS_WIN32DIALOG    40

struct Win32Dialog : public LuaClass
{
    Win32Dialog( lua_State *L, unsigned int width, unsigned int height );
    ~Win32Dialog( void );

    bool isPainting;
    PAINTSTRUCT ps;
    HDC deviceContext;
    HWND handle;

    typedef LRESULT (__stdcall*wndProc_t)( HWND wnd, UINT msg, WPARAM wparam, LPARAM lparam );

    wndProc_t defaultWndProc;
};

void luawin32_extendDialogs( lua_State *L );
void luawin32_updateWindows( void );

#endif //_WIN32_DIALOGS_