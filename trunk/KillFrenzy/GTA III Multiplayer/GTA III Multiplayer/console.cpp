// Console functionality
#include "StdInc.h"
int m_showConsole=1;
int m_isConsoleActive=0;
char m_consoleHistory[MAX_CONSOLE_HISTORY][MAX_CONSOLE_LINE];
char m_consoleInput[MAX_CONSOLE_LINE];
size_t m_sInputCursor=0;
size_t m_numberOfLines=0;
size_t m_sCurrentCursor=0;
int m_iCurWidth=0;
bool m_bConsoleIsTyping=false;
bool m_bIsShift=false;
bool m_bIsConstShift=false;
bool m_bDisableShowCursor=false;

/*=================================
	Initialisation
=================================*/

// Init the console environment
void	Console_Init ()
{
	m_showConsole=1;
	m_isConsoleActive=0;
	m_numberOfLines=0;
}

static inline bool stradjust( const char*& msg, char delim )
{
    if ( *msg == 0 )
        return false;

    while ( *msg != delim )
    {
        if ( *msg == 0 )
            return true;

        msg++;
    }

    return true;
}

static inline bool strgettok( const char*& msg, char delim, std::string& out )
{
    const char *dpass = msg;

    if ( !stradjust( msg, delim ) )
        return false;

    out = std::string( dpass, (size_t)( msg - dpass ) );

    if ( *msg != 0 )
        msg++;

    return true;
}

static inline void strsplit( const char *msg, std::vector <std::string>& out )
{
    std::string item;

    while ( strgettok( msg, ' ', item ) )
    {
        if ( item.size() == 0 )
            continue;

        out.push_back( item );
    }
}

// Processes the console input
bool	Console_ProcessInput( const char *input )
{
    if ( *input == 0 )
        return true;

	if ( input[0] == '/' )
	{
		input++;
	
		Console_Printf( "> %s\n", 0xffffffff, input );

        // Process as command
        std::string cmd;

        if ( !strgettok( input, ' ', cmd ) )
            return true;

		std::vector <std::string> args;
        strsplit( input, args );

		// Process the command
		Core_ProcessCommand( cmd, args );
	}
	else
	{
		Console_Printf("echo: %s\n", 0xffffffff, input);
	}

    return true;
}

// Processes the console input
LRESULT CALLBACK	Console_WndProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	if (uMsg == WM_CHAR)
	{
		unsigned char ucKey=tolower(wParam);

		if (!m_bConsoleIsTyping)
		{
			if (ucKey=='t')
			{
				ShowCursor ( true );
				m_bDisableShowCursor = true;
				m_bConsoleIsTyping = true;
				m_bInputEnabled = false;
				return 1;
			}
		}
		else if (ucKey == VK_ENTER)
		{
			// Process the input
			Console_ProcessInput ( m_consoleInput );
			m_consoleInput[0]=0;
			m_sInputCursor=0;
			m_bConsoleIsTyping = false;
			m_bDisableShowCursor = false;
			m_bInputEnabled = true;
			if (!*(BYTE*)(MEM_SHOWMENU))
				ShowCursor ( false );
			return 1;
		}
		else if (ucKey == VK_BACKSPACE)
		{
			if (m_sInputCursor!=0)
				m_sInputCursor--;
			m_consoleInput[m_sInputCursor]=0;
			return 1;
		}
		else
		{
			if (m_sInputCursor+1!=MAX_CONSOLE_LINE)
			{
				m_consoleInput[m_sInputCursor]=wParam;
				m_consoleInput[m_sInputCursor+1]=0;
				m_sInputCursor++;
			}
			return 1;
		}
	}
	else if (uMsg == WM_KEYDOWN)
	{
		if (wParam == VK_F12)
		{
			m_showConsole = !m_showConsole;
			return 1;
		}
	}
	return 0;
}

/*=================================
	Functionality
=================================*/

// Draw the console
void	Console_Draw( IDirect3DDevice8 *pDevice )
{
	ID3DXFont *pFont;
	unsigned int uiCurOffset=0;

	if (!m_showConsole)
		return;

	//Console_Printf ( "Test\na", 0xffffffff );

	pFont=D3D_GetFont (	CONSOLE_FACENAME );
	//int charLen = D3D_GetCharWidth ( pFont, 'i', 1 );

	D3D_BeginDrawing();
	D3D_DrawRect ( D3D_GetAbsFromRel2D (0.05,0.05), D3D_GetAbsFromRel2D (0.4,0.3), 0x601525ff );
	if (m_bConsoleIsTyping)
		D3D_DrawRect ( D3D_GetAbsFromRel2D (0.05,0.35), (float)(m_iScreenWidth*0.4), D3D_FONTHEIGHT+10, 0x554515ff );
	pFont->Begin ( );

	// Draw the console history
	unsigned int uiCurLine=max((int)m_numberOfLines-((int)(m_iScreenHeight*0.3)-(int)(m_iScreenHeight*0.05))/D3D_FONTHEIGHT,0);

	for (uiCurLine; uiCurLine<m_numberOfLines; uiCurLine++)
	{
		RECT drawRect;
		drawRect.left = (int)(m_iScreenWidth*0.05)+15;
		drawRect.top = (int)(m_iScreenHeight*0.05)+10+(uiCurOffset)*D3D_FONTHEIGHT;
		drawRect.right = (int)(m_iScreenWidth*0.05)+(int)(m_iScreenWidth*0.4)-20;
		drawRect.bottom = (int)(m_iScreenWidth*0.05)+(uiCurOffset-1)*D3D_FONTHEIGHT;
		pFont->DrawTextA ( m_consoleHistory[uiCurLine], -1, &drawRect, 0, 0xffffffff );
		uiCurOffset++;
	}

	// Draw the console typebar
	if (m_bConsoleIsTyping)
	{
		RECT TypeRect;
		TypeRect.left = (int)(m_iScreenWidth*0.05)+12;
		TypeRect.top = (int)(m_iScreenHeight*0.05)+(int)(m_iScreenHeight*0.3)+5;
		TypeRect.right = TypeRect.left+40;
		TypeRect.bottom = 0;
		pFont->DrawTextA ("Type:", -1, &TypeRect, 0, 0xff20ff20);

		// Format the entry and post it
		size_t n;
		size_t sSize=strlen(m_consoleInput);

        if ( sSize )
        {
		    int iInputWidth=0;
		    char cOutputBuff[MAX_CONSOLE_LINE];
		    cOutputBuff[0]=0;
	
		    TypeRect.left = TypeRect.right;
		    TypeRect.right = (int)(m_iScreenWidth*0.05)+(int)(m_iScreenWidth*0.35)+18;
		    for (n=sSize-1; n!=0; n--)
		    {
			    int iCurWidth = D3D_GetCharWidth ( pFont, m_consoleInput[n], 1 );
			
			    if (iInputWidth+iCurWidth>TypeRect.right-TypeRect.left)
			    {
				    break;
			    }
			    else
				    iInputWidth+=iCurWidth;
		    }
		    memcpy (cOutputBuff, m_consoleInput+n, sSize-n+1);
		    cOutputBuff[sSize-n+1]=0;
		    pFont->DrawTextA (cOutputBuff, -1, &TypeRect, DT_NOPREFIX, 0xffffffff);
        }
	}

	pFont->End ( );
	D3D_EndDrawing();
}

// Print a line
void	Console_Printf( const char *Text, D3DCOLOR Color, ... )
{
	char line_buff[0xFFFF];
	va_list vl;
	ID3DXFont *pFont=D3D_GetFont ( CONSOLE_FACENAME );
	int iConsoleWidth = (int)(m_iScreenWidth*0.36);

	if (!pFont)
		return;

	va_start ( vl, Color );
	vsprintf ( line_buff, Text, vl );
	va_end ( vl );

	// Format the line
	size_t sCurBuff;
	size_t sBuff=strlen(line_buff);
	//size_t sLineCount=0;
	for (sCurBuff=0; sCurBuff<sBuff; sCurBuff++)
	{
		int iThisWidth = D3D_GetCharWidth ( pFont, line_buff[sCurBuff], 1 );
		switch (line_buff[sCurBuff])
		{
		case '\n':
			//strncpy ( m_consoleHistory, line_buff+sLastPos, min (sLineCount,MAX_CONSOLE_LINE-1) );
			//sLastPos=sCurBuff+1;
			if (m_numberOfLines==MAX_CONSOLE_HISTORY-1)
			{
				unsigned int n;
				
				for (n=0; n<m_numberOfLines; n++)
				{
					memcpy (m_consoleHistory[n], m_consoleHistory[n+1], MAX_CONSOLE_LINE);
				}
				m_numberOfLines--;
			}
			//m_consoleHistory[m_numberOfLines][m_sCurrentCursor]=0;
			m_sCurrentCursor=0;
			m_numberOfLines++;
			m_iCurWidth=0;
			break;
		default:
			if (m_sCurrentCursor==MAX_CONSOLE_LINE-1 || m_iCurWidth+iThisWidth>iConsoleWidth)
			{
				if (m_numberOfLines==MAX_CONSOLE_HISTORY-1)
				{
					unsigned int n;
					
					for (n=0; n<m_numberOfLines; n++)
					{
						memcpy (m_consoleHistory[n], m_consoleHistory[n+1], MAX_CONSOLE_LINE);
					}
					m_numberOfLines--;
				}
				//m_consoleHistory[m_numberOfLines][m_sCurrentCursor]=0;
				m_sCurrentCursor=0;
				m_numberOfLines++;
				m_iCurWidth=0;
			}
			m_consoleHistory[m_numberOfLines][m_sCurrentCursor]=line_buff[sCurBuff];
			m_consoleHistory[m_numberOfLines][m_sCurrentCursor+1]=0;
			m_iCurWidth+=D3D_GetCharWidth ( pFont, line_buff[sCurBuff], 1 );
			m_sCurrentCursor++;
			break;
		}
	}
	if (m_sCurrentCursor != 0 && m_numberOfLines==0)
		m_numberOfLines++;
}