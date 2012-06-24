#ifndef _GAME_
#define _GAME_

#define MEM_SHOWMENU					0x008F5B9d
#define	MEM_SYSTEM_STATE				0x008F58EC
#define MEM_CPEDPOOL					0x008F2D14
#define MEM_bGameNeedsLoad				0x008F5BA2
#define FUNC_LoadEnvironment			0x0048E8A0
#define FUNC_LoadScene					0x0048D870
#define FUNC_LoadProgress				0x0048BFC0
#define FUNC_GetPedRef					0x0043EB30
#define FUNC_FadeCamera					0x0046B370
#define FUNC_RecordFrame				0x00593560
#define MEM_RecordFrameCall				0x0048CA75
#define MEM_ProcessVehicleTraffic		0x0048CAF9
#define FUNC_GetCDDrive					0x005A0BE0
#define MEM_DrawHealthCall				0x005066BA
#define FUNC_AnsiToWide					0x00500AA0
#define FUNC_DrawSetColor				0x00501CB0
#define FUNC_DrawString					0x00501030
#define FUNC_DrawHUD					0x00505380
#define FUNC_InitTrainTracks			0x0054F140
#define FUNC_InitFlightTracks			0x0054B9A0
#define FUNC_PlayMovie					0x005826C0
#define MEM_PlayIntroMovie				0x00582DC4
#define MEM_PlayTitleMovie				0x00582F39
#define MEM_SetInitialGameStatus		0x00582DB5

enum eSystemState
{
	GS_START_UP,
	GS_INIT_LOGO_MPEG,
	GS_LOGO_MPEG,
	GS_INIT_INTRO_MPEG,
	GS_INTRO_MPEG,
	GS_INIT_ONCE,
	GS_INIT_FRONTEND,
	GS_FRONTEND,
	GS_INIT_PLAYING_GAME,
	GS_PLAYING_GAME
};

typedef float vec3_t[3];
typedef float vec4_t[4];

//#include "ped.h"
#define	MEM_gameDrive	0x008E2A1C
#define MEM_gameData	0x008E2D30

class CGame
{
public:
		CGame ();
	   ~CGame ();
	

	void			Hook_LoadEnvironment ();
	void			StartGame ( bool bSinglePlayer );
	eSystemState	GetSystemState ();
	void			SetSystemState ( eSystemState eState );
	//CPed*			GetPedRef ( unsigned int uiRef );
	bool			FadeCamera ( float fTime, bool bFadeIn );
	char*			GetGameDrive ();
	unsigned int	GetGameTime ();
private:
};

extern bool m_bRequestStartGame;
extern bool m_bDisableHUD;

// Draw calls
void		Draw_SetColor(DWORD dwColorID);
void		AnsiToWide(char *pIn, char *pOut);
void		Draw_String(float x, float y, char *pIn);

#endif //_GAME