#include "StdInc.h"

bool m_bIsSinglePlayer=true;
bool m_bIsSPLoaded=false;
bool m_bRequestStartGame=true;
bool m_bDisableHUD=false;
typedef void (*pfnDrawHUD)();
pfnDrawHUD m_pfnDrawHUD;

void		Hook_DrawHUD()
{
	/*char buff[0xff];

	Draw_SetColor(0xAA0000FF);
	AnsiToWide("Hello World", buff);
	Draw_String(500, 500, buff);*/

	if (!m_bDisableHUD)
		m_pfnDrawHUD();
}

CGame::CGame ( )
{
	Pool_Init();

	// Disable frame recorder
	memset((void*)MEM_RecordFrameCall, 0x90, 5);
	
	// No vehicle traffic nor script
	memset((void*)MEM_ProcessVehicleTraffic, 0x90, 15);

	// Stop HandlePopulation after collision detection
	*(DWORD*)(0x004F3A6A) = 0xC310C483;

	// Random testing
	memset((void*)0x0048CA40, 0x90, 5);
	memset((void*)0x0048CA36, 0x90, 5);
	//memset((void*)0x0048CA2C, 0x90, 5);	// Disable effects
	memset((void*)0x0048CA27, 0x90, 5);
	memset((void*)0x0048CA22, 0x90, 5);
	memset((void*)0x0048CA1D, 0x90, 5);
	memset((void*)0x0048CA18, 0x90, 5);
	//memset((void*)0x0048CA13, 0x90, 5);	// Disable tire paints
	memset((void*)0x0048CA0E, 0x90, 5);
	memset((void*)0x0048CA09, 0x90, 5);
	memset((void*)0x0048CA04, 0x90, 5);
	memset((void*)0x0048C9FF, 0x90, 5);
	//memset((void*)0x0048C9FA, 0x90, 5);
	memset((void*)0x0048C9F0, 0x90, 5);
	memset((void*)0x0048C9EB, 0x90, 5);
	memset((void*)0x0040B4CD, 0x90, 5);
	memset((void*)0x0040B3E5, 0x90, 5);
	memset((void*)0x0048C9F0, 0x90, 5);
	memset((void*)0x0048C9EB, 0x90, 5);
	memset((void*)0x0048C9E6, 0x90, 5);
	memset((void*)0x0048C9E1, 0x90, 5);
	memset((void*)0x0048C9DC, 0x90, 5);
	//memset((void*)0x0048C9D7, 0x90, 5);	// Disable text renderer
	//memset((void*)0x0048C9D2, 0x90, 5);	// Disable clock
	memset((void*)0x0048C9CD, 0x90, 5);
	//memset((void*)0x0048C9C8, 0x90, 5);	// Disable core player update
	//memset((void*)0x0048C9A9, 0x90, 5);	// Disable player world model procession
	//memset((void*)0x0048C950, 0x90, 5);	// Disable control management

	//memset((void*)0x0048CAA7, 0x90, 5);	// Disable camera
	memset((void*)0x0048CA61, 0x90, 5);
	memset((void*)0x0048CA66, 0x90, 5);
	memset((void*)0x0048CA6B, 0x90, 5);
	//memset((void*)0x0048CA70, 0x90, 5);	// Disable zone progression
	memset((void*)0x0048CA75, 0x90, 5);

	memset((void*)0x0048CA84, 0x90, 5);
	memset((void*)0x0048CA89, 0x90, 5);
	memset((void*)0x0048CA8E, 0x90, 5);
	memset((void*)0x0048CA93, 0x90, 5);
	memset((void*)0x0048CA98, 0x90, 5);
	memset((void*)0x0048CA9D, 0x90, 5);

	//memset((void*)0x0048CAB5, 0x90, 5);	// Disable camera updates
	memset((void*)0x0048CABA, 0x90, 5);
	//memset((void*)0x0048CACD, 0x90, 25);
	memset((void*)0x0048CAEB, 0x90, 5);

	//memset((void*)0x0048E565, 0x90, 5);
	//memset((void*)0x0057A5F3, 0x90, 5);
	//memset((void*)0x0057A604, 0x90, 5);
	//memset((void*)0x0057A612, 0x90, 5);
	//memset((void*)0x0057A639, 0x90, 5);
	//memset((void*)0x0057A640, 0x90, 5);
	//memset((void*)0x0057A64A, 0x90, 5);

	// Sound
	memset((void*)0x0057AE58, 0x90, 5);
	//memset((void*)0x0057AE5F, 0x90, 5);
	//memset((void*)0x0057AE66, 0x90, 5);
	//memset((void*)0x0057AE6D, 0x90, 5);
	//memset((void*)0x0057AE78, 0x90, 5);

	// Game Sound
	//memset((void*)0x00569B3C, 0x90, 5);	// Crash
	//memset((void*)0x00569B48, 0x90, 5);	// Crash
	//memset((void*)0x00569B54, 0x90, 5);	// Crash
	memset((void*)0x00569B6E, 0x90, 5);
	//memset((void*)0x00569B7A, 0x90, 5);	// Crash
	//memset((void*)0x00569B7F, 0x90, 5);	// Crash
	//memset((void*)0x00569BAB, 0x90, 5);

	//memset((void*)MEM_DrawHealthCall, 0x90, 5);

	// No vehicle path initialisation
//	*(BYTE*)(FUNC_InitTrainTracks) = 0xC3;
//	*(BYTE*)(FUNC_InitFlightTracks) = 0xC3;

	// No Intro Movies
	*(BYTE*)(MEM_SetInitialGameStatus+0x06)=GS_INIT_ONCE;

	// When the game is inited 'once', we start it immediatly, for now
//	*(BYTE*)(0x005830F9+0x06)=GS_INIT_PLAYING_GAME;

	// Try to disable scene loadíng, or limit it to things needed
	//memset((void*)0x0040B7F6, 0x90, 5);
	//memset((void*)0x0040B81F, 0x90, 5);
	//memset((void*)0x0040B827, 0x90, 5);
	//memset((void*)0x0040B82F, 0x90, 5);
	//memset((void*)0x0040B86F, 0x90, 5);
	//memset((void*)0x0040B837, 0x90, 5);
	//memset((void*)0x0040B83F, 0x90, 5);
	//memset((void*)0x0040B847, 0x90, 5);

	// Do not let the game load the default world file
	//memset((void*)0x0048C123, 0x90, 0x12);

	// Detours
	m_pfnDrawHUD = (pfnDrawHUD)DetourFunction((PBYTE)FUNC_DrawHUD, (PBYTE)Hook_DrawHUD);
}

CGame::~CGame ( )
{

}

// Hook of load environment
void	CGame::Hook_LoadEnvironment ( )
{
	if ( m_bIsSinglePlayer )
	{
		DWORD dwFunc;
		char *pScene="loadsc0";
		char *pData="DATA\\GTA3.DAT";
		__asm
		{
			push pScene
			mov dwFunc,FUNC_LoadScene
			call dwFunc
			push pData
			mov dwFunc,FUNC_LoadProgress
			call dwFunc
		}
	}
}

// Starts the game
void	CGame::StartGame ( bool bSinglePlayer )
{
	if (*(bool*)(MEM_bGameNeedsLoad)==true)
	{
		DWORD dwStart=FUNC_LoadEnvironment;

		/*if (bSinglePlayer)
		{
			//DetourFunction ( (BYTE*)FUNC_LoadEnvironment, (BYTE*)CGame::Hook_LoadEnvironment );
		}
		*(BYTE*)(MEM_SHOWMENU) = 0;*/

		__asm
		{
			call dwStart
		}
		SetSystemState ( GS_PLAYING_GAME );
		*(bool*)(MEM_bGameNeedsLoad)=false;
		*(bool*)(0x00885B38)=false;
		//*(bool*)(MEM_SHOWMENU)=false;
	}
}

// Retrive the system state
eSystemState	CGame::GetSystemState ( )
{
	return *(eSystemState*)(MEM_SYSTEM_STATE);
}

// Sets the system state
void	CGame::SetSystemState ( eSystemState eState )
{
	*(eSystemState*)(MEM_SYSTEM_STATE) = eState;
}

bool	CGame::FadeCamera ( float fTime, bool bFadeIn )
{
	DWORD dwFunc=FUNC_FadeCamera;
	float m_fTime=fTime;
	DWORD m_bFadeIn=bFadeIn;
	__asm
	{
		push m_fTime
		push m_bFadeIn
		call dwFunc
		//add esp,8
	}
	return true;
}

char*	CGame::GetGameDrive ()
{
	return (char*)(MEM_gameDrive);
}

unsigned int	CGame::GetGameTime ()
{
	return *(DWORD*)(MEM_gameData+0x10C);
}

/*======================================================
	Pool
======================================================*/

// Retrive interface from ped pool
/*CPed*	CGame::GetPedRef ( unsigned int uiRef )
{
	if (*m_ppPedPool==NULL)
		return NULL;
	return (CPed*)((int)(*m_ppPedPool)->m_pPoolHeap+uiRef*sizeof(CPed));
}*/

// Sets the draw color

void		Draw_SetColor(DWORD dwColorID)
{
	DWORD dwColor=FUNC_DrawSetColor;
	DWORD dwColorIDmem=dwColorID;

	__asm
	{
		mov ecx,0x006FACF8
		lea eax,dwColorID
		push eax
		call dwColor
		add esp,4
	}
}

// Transforms ANSI to WIDE
void		AnsiToWide(char *pIn, char *pOut)
{
	DWORD dwConvert=FUNC_AnsiToWide;

	__asm
	{
		mov ecx,0x006FACF8
		push [pOut]
		push [pIn]
		call dwConvert
		add esp,8
	}
}

// Draw String
void		Draw_String(float x, float y, char *pIn)
{
	DWORD dwDraw=FUNC_DrawString;

	__asm
	{
		mov ecx,0x006FACF8
		push [pIn]
		push [y]
		push [x]
		call dwDraw
		add esp,12
	}
}