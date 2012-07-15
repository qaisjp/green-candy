// Do the main thing here
#include "StdInc.h"
#include <SharedUtil.hpp>
unsigned int isCoreLoaded=0;
CGame	*m_pGame=0;
bool m_bShowInternalDebug=false;
CFileTranslator *modFileRoot;

extern DWORD *m_pScriptBase;

CGamePlayer *g_player;

/*=================================================
	Initialize
=================================================*/
CVehicle*		m_pTmpVeh;
CPed*			m_pTmpPed;

void Hook_PrintDebugf( const char *fmt, ... )
{
    if ( !m_bShowInternalDebug )
        return;

    char buffer[2048];
    va_list args;
    va_start( args, fmt );

    vsnprintf( buffer, 2047, fmt, args );

    va_end( args );

    Console_Printf( buffer, 0xffffffff );
}

// Init the core
void	Core_Init()
{
	// Before everything, we free whole code of gtaIII
	DWORD blah;
	Core_SetModuleAccessLevel ( GetModuleHandle ("gta3.exe"), PAGE_EXECUTE_READWRITE, &blah );

    // Hook the debug output
    DetourFunction( (PBYTE)FUNC_DebugPrintf, (PBYTE)Hook_PrintDebugf );

    // Initialize the fileSystem
	new CFileSystem();
    modFileRoot = fileRoot;

	D3D8_Init();
	Input_Init();
    Lua_Init();
	Sound_Init();
	Model_Init();
	SCM_Init();
	World_Init();
	Entity_Init();
	Ped_Init();
	Vehicle_Init();
	
	// Create all mandatory classes
	m_pGame = new CGame ();

	//isCoreLoaded=1;
}

void    Core_InitGame()
{
    // Create our player
	g_player = new CGamePlayer( lua_manager->GetVirtualMachine(), *Player_Create( 0, 19, 72, 16.5 ) );

    Lua_Start();
}

// Free all memory and close all other activities
void	Core_Destroy()
{
	delete m_pGame;
	delete fileSystem;

	Sound_Destroy();
    Lua_Destroy();
}

// Set a whole module to a virtual access level
bool	Core_SetModuleAccessLevel ( HMODULE pModule, DWORD dwAccessLevel, DWORD *pOldProt )
{
	char cModulePath[MAX_PATH];
	struct stat fileStat;
	FILE *pFile;

	if (!GetModuleFileName ( pModule, cModulePath, MAX_PATH ))
		return false;

	pFile=fopen(cModulePath, "rb");
	fstat(fileno(pFile), &fileStat);
	fclose(pFile);

	VirtualProtect ( (void*)pModule, fileStat.st_size, dwAccessLevel, pOldProt );
	return true;
}

// Processes a command
bool	Core_ProcessCommand( const std::string& cmdName, std::vector <std::string>& args )
{
    if ( cmdName == "vel" )
	{
		CEntity *pEntity = Player_GetBySlot(0);
		size_t sClass = sizeof(CEntity);
		size_t sDouble = sizeof(double);
		
		if (((CPed*)(pEntity))->GetOccupiedVehicle())
			pEntity=((CPed*)(pEntity))->GetOccupiedVehicle();

		Console_Printf("X: %f, Y: %f, Z: %f\n", 0xffffffff, pEntity->m_vecVelocity[0], pEntity->m_vecVelocity[1], pEntity->m_vecVelocity[2]);
	//	memcpy(&pEntity->m_fRollLeft, &pEntity->m_matrix->m_vecRight, sizeof(vec3_t));
	//	memcpy(&pEntity->m_fPitchLeft, &pEntity->m_matrix->m_vecFront, sizeof(vec3_t));
		return true;
	}
	else if ( cmdName == "fade" )
	{
		m_pGame->FadeCamera( 1, atoi( args[0].c_str() ) == 1 );
		//SCM_ProcessCommand(&fade, 1000, atoi(cArgv[0]));
		return true;
	}
	else if ( cmdName == "aval" )
	{
		if ( args.size() < 1)
			return false;

		Console_Printf("Available: %i\n", 0xffffffff, Model_IsLoaded(atoi( args[0].c_str() )));
		return true;
	}
	else if ( cmdName == "ps" )
	{
		FMOD::Sound *pSound;

		if ( args.size() < 1 )
			return false;

		pSound = Sound_LoadFile( args[0].c_str() );

		if (!pSound)
		{
			Console_Printf("Failed to load\n", 0xffffffff);
			return false;
		}

		Music_Play(pSound, -1, false);
		return true;
	}
	else if ( cmdName == "sounddump" )
	{
		Sound_Dump();
        return true;
	}
	else if ( cmdName == "getstate" )
	{
		Console_Printf( "System State: %u\n", 0xffffffff, (unsigned int)m_pGame->GetSystemState() );
		return true;
	}
	else if ( cmdName == "setstate" )
	{
        if ( args.size() < 1 )
            return false;

		m_pGame->SetSystemState( (eSystemState)atoi( args[0].c_str() ) );
		return true;
	}
	else if ( cmdName == "getpedpool" )
	{
        if ( args.size() < 1 )
            return false;

        unsigned int num = atoi( args[0].c_str() );

		Console_Printf("Mem: %.8x\n", 0xffffffff, (int)(*m_ppPedPool)->m_pPoolHeap+num*sizeof(CPed));
		//Console_Printf("sizeof(CPed) = %u\n", sizeof(class CPed));
		CPed *m_pPed = (CPed*)((int)(*m_ppPedPool)->m_pPoolHeap+num*sizeof(CPed));
		float fHealth = *(float*)((int)m_pPed+704);
		return true;
	}
	else if ( cmdName == "setflags" )
	{
		if ( args.size() < 1 )
            return false;

		CPed *pPlayer = Player_GetBySlot(0);
		pPlayer->m_cEntityFlags = (char)atoi( args[0].c_str() );
		return true;
	}
	else if ( cmdName == "setallpedhealth" )
	{
		if ( args.size() < 1 )
            return false;

		float fHealth = (float)atof( args[0].c_str() );
		unsigned int n;

		for (n=0; n<(*m_ppPedPool)->m_uiMaxItems; n++)
		{
			CPed *pPed = (CPed*)((int)(*m_ppPedPool)->m_pPoolHeap+sizeof(CPed)*n);

			if ((*m_ppPedPool)->m_pSlots[n] & 0x80)
				continue;

			pPed->m_fHealth = fHealth;
		}
		return true;
	}
	else if ( cmdName == "killallpeds" )
	{
		unsigned int n;

		for (n=2; n<(*m_ppPedPool)->m_uiMaxItems; n++)
		{
			CPed *pPed = (CPed*)((int)(*m_ppPedPool)->m_pPoolHeap+sizeof(CPed)*n);

			if ((*m_ppPedPool)->m_pSlots[n] & 0x80)
				continue;

			pPed->m_fHealth = 1;
		}
		return true;
	}
	else if ( cmdName == "showmenu" )
	{
		if ( args.size() < 1 )
            return false;

		*(BYTE*)(MEM_SHOWMENU) = atoi( args[0].c_str() ) % 2;
		return true;
	}
	else if ( cmdName == "pause" )
	{
		if ( args.size() < 1 )
            return false;

		*(BYTE*)(0x008F5BAD) = atoi( args[0].c_str() ) % 2;
		return true;
	}
	else if ( cmdName == "tp" )
	{
		if ( args.size() < 3 )
            return false;

		CPed *player = Player_GetBySlot(0);
		float fPosX=(float)atof( args[0].c_str() );
		float fPosY=(float)atof( args[1].c_str() );
		float fPosZ=(float)atof( args[2].c_str() );
		//DWORD dwFunc = FUNC_CEntity_Teleport;
		/*__asm
		{
			push fPosX
			push fPosY
			push fPosZ
			mov edi,player
			lea ecx,[edi+4]
			call dwFunc
		}*/
		player->SetPosition(fPosX, fPosY, fPosZ);
		return true;
	}
	else if ( cmdName == "freem" )
	{
		if ( args.size() < 1 )
            return false;

		Model_Free(atoi( args[0].c_str() ));
	}
	else if ( cmdName == "psp" )
	{
        if ( args.size() < 1 )
            return false;

		CPed *pPlayer = Player_GetBySlot(0);
		vec3_t vecPos;
		pPlayer->m_matrix.GetOffset( 0, 3, 1, vecPos );

		m_pTmpPed = Ped_Create(atoi( args[0].c_str() ), vecPos[0], vecPos[1], vecPos[2]);
		return true;
	}
	else if ( cmdName == "makeplayer" )
	{
		CPed *pPed=Player_GetBySlot(0);
		m_pTmpPed=pPed;
		CPed *pPlayer=Player_Create(0, pPed->m_matrix.m_vecPos[0], pPed->m_matrix.m_vecPos[1], pPed->m_matrix.m_vecPos[2]);
        return true;
	}
	else if ( cmdName == "getveh" )
	{
		CPed *pPlayer=Player_GetBySlot(0);
		CVehicle *pVehicle=pPlayer->GetOccupiedVehicle();

		if (pVehicle)
		{
			Console_Printf("True: %i (%x8)\n", 0xffffffff, pVehicle->m_usModelID, (DWORD)pVehicle);
		}
		else
		{
			Console_Printf("False\n", 0xffffffff);
		}
		return true;
	}
	else if ( cmdName == "dstr" )
	{
        if ( !m_pTmpVeh )
            return true;

		m_pTmpVeh->Destroy();
        return true;
	}
	else if ( cmdName == "dp" )
	{
		m_pTmpPed->Destroy();
	}
	else if ( cmdName == "gspname" )
	{
        if ( args.size() < 1 )
            return false;

		CSpecialPedModelInfo *pInfo = m_pSpecialPedModelPool->m_pData + atoi( args[0].c_str() );

		Console_Printf( "Name: %s\n", 0xffffffff, pInfo->m_modelType );
		return true;
	}
	else if ( cmdName == "test2" )
	{
		CPed *pPed=(CPed*)((*m_ppPedPool)->m_pPoolHeap)+1;
		CVehicle *pVehicle=(CVehicle*)((*m_ppVehPool)->m_pPoolHeap)+1;

		Console_Printf("Ped: %i\n", 0xffffffff, *(BYTE*)(pPed+0x50));
		Console_Printf("Vehicle: %i\n", 0xffffffff, *(BYTE*)(pVehicle+0x50));
        return true;
	}
	else if ( cmdName == "mi" )
	{
        if ( args.size() < 1 )
            return false;

		CModelInfo* pModelInfo = Model_GetEntry(atoi( args[0].c_str() ));

		if ( !pModelInfo )
		{
			Console_Printf ( "Not found\n", 0xffffffff );
			return true;
		}

		Console_Printf( "Model ID: '%s'\n", 0xffffffff, pModelInfo->m_cModelName );
		Console_Printf( "Instance: '%s'\n", 0xffffffff, pModelInfo->m_cInstanceName );
		Console_Printf( "Model Type: %i\n", 0xffffffff, pModelInfo->m_cModelType );
		return true;
	}
	else if ( cmdName == "startgame" )
	{
		m_pGame->StartGame ( true );
		return true;
	}
	else if ( cmdName == "nogui" )
	{
        m_bDisableHUD = !m_bDisableHUD;
        return true;
	}
    else if ( cmdName == "showdebug" )
    {
        m_bShowInternalDebug = !m_bShowInternalDebug;

        if ( m_bShowInternalDebug )
            Console_Printf( "Enabled GTAIII debug output\n", 0xffffffff );
        else
            Console_Printf( "Disabled GTAIII debug output\n", 0xffffffff );

        return true;
    }

	return Lua_ProcessCommand( cmdName, args );
}

/*=================================================
	Frame functions
=================================================*/

// The first frame in game
void	Core_FirstFrame ()
{
	// Here we shall hook
	//DetourFunction ( (PBYTE)FUNC_DebugPrintf, (PBYTE)Hook_DebugPrintf );
	Console_Printf ("run();\n", 0xffffff);
	//m_pGame->StartGame ( true );
}

// Pre render frame
bool	Core_PreRender ()
{
	/*char buff[0xff];

	Draw_SetColor(0xff0000ff);
	AnsiToWide("Yo, hi", buff);
	Draw_String(250, 400, buff);*/

    Lua_Frame();

	Model_ManagerFrame();
	Sound_Frame();
	return true;
}

// At render
bool	Core_Render ( IDirect3DDevice8 *pD3D )
{
	return true;
}

// After render
bool	Core_PostRender ()
{
	// Cursor fix
	if ( m_bIsFocused && !m_bConsoleIsTyping && !*(BYTE*)(MEM_SHOWMENU) && m_pGame->GetSystemState() == GS_PLAYING_GAME )
	{
		RECT wndRect;

		if (GetWindowRect (hGTAWindow, &wndRect))
		{
			SetCursorPos ( wndRect.left+m_iScreenWidth/2, wndRect.top+m_iScreenHeight/2 );
			//m_pOrigDDevice->SetCursorPosition ( wndRect.left+m_iScreenWidth/2, wndRect.top+m_iScreenHeight/2, D3DCURSOR_IMMEDIATE_UPDATE );
		}
	}
	return true;
}