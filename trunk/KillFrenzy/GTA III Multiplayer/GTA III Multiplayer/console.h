#ifndef _CONSOLE_
#define _CONSOLE_

#include "game.h"
#include "d3d8.h"
#define MAX_CONSOLE_HISTORY	256
#define MAX_CONSOLE_LINE	128
#define CONSOLE_FACENAME	"Arial.ttf"

extern int m_isConsoleActive;
extern bool m_bConsoleIsTyping;
extern char m_consoleInput[MAX_CONSOLE_LINE];

#define VK_BACKSPACE	0x08
#define VK_ENTER		0x0D
#define VK_SHIFTC		0x14
#define VK_SHIFT		0x10

// Init
void	Console_Init();
LRESULT CALLBACK	Console_WndProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );
// Functionality
void	Console_Draw( IDirect3DDevice8 *pDevice );
void	Console_Printf( const char *Text, D3DCOLOR Color, ... );

extern CGame	*m_pGame;

#endif //_CONSOLE_