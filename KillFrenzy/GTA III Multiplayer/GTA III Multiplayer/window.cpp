// Window handling
#include "StdInc.h"
#include <windowsx.h>
WNDPROC	m_pOrigWindowFunc=0;
bool m_bIsFocused=0;

/*================================
	Window Hooks
================================*/
extern bool	m_bIsSinglePlayer;

// Process the messages
LRESULT	CALLBACK	Window_ProcessMessage ( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	if ( uMsg == WM_CLOSE )
	{
		// Clean up our environment
		Core_Destroy();

		TerminateProcess ( GetCurrentProcess(), EXIT_SUCCESS );
	}

	// Log states
	if ( uMsg == WM_KILLFOCUS || ( uMsg == WM_ACTIVATE && LOWORD(wParam) == WA_INACTIVE ) )
	{
		m_bIsFocused = false;
		return 1;
	}
	else if ( uMsg == WM_SHOWWINDOW || (uMsg == WM_SETFOCUS || ( uMsg == WM_ACTIVATE && LOWORD(wParam) == WA_INACTIVE) ) )
	{
		m_bIsFocused = true;
	}
	if ( isCoreLoaded )
	{
		if (!(Console_WndProc ( hWnd, uMsg, wParam, lParam )))
			if (!m_bConsoleIsTyping && !(uMsg==WM_KEYDOWN && wParam==VK_ESCAPE && !m_bIsSinglePlayer))
				return m_pOrigWindowFunc ( hWnd, uMsg, wParam, lParam );
	}
	return DefWindowProc ( hWnd, uMsg, wParam, lParam );
}

/*================================
	Initialize
================================*/

// Apply our hook on the main GTA window
void	Window_ApplyHook ()
{
	m_pOrigWindowFunc = SubclassWindow ( hGTAWindow, Window_ProcessMessage );
}