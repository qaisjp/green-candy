// Do the main thing here
#include "StdInc.h"
unsigned int isCoreLoaded=0;
CGame	*m_pGame=0;
bool m_bShowInternalDebug=false;

extern DWORD *m_pScriptBase;

/*=================================================
	Initialize
=================================================*/
CVehicle*		m_pTmpVeh;
CPed*			m_pTmpPed;

// Init the core
void	Core_Init()
{
	// Before everything, we free whole code of gtaIII
	DWORD blah;
	Core_SetModuleAccessLevel ( GetModuleHandle ("gta3.exe"), PAGE_EXECUTE_READWRITE, &blah );

	D3D8_Init();
	Input_Init();
	Sound_Init();
	Model_Init();
	SCM_Init();
	World_Init();
	Entity_Init();
	Ped_Init();
	Vehicle_Init();
	
	// Create all mandatory classes
	m_pGame = new CGame ();
	new CFileSystem();

	//isCoreLoaded=1;
}

// Free all memory and close all other activities
void	Core_Destroy()
{
	delete m_pGame;
	delete fileSystem;

	Sound_Destroy();
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
bool	Core_ProcessCommand ( char *cmdName, int iArgc, char **cArgv )
{
	if (strcmp(cmdName,"test")==0)
	{
		if (iArgc>0)
		{
			m_pTmpPed->m_uiTaskID=atoi(cArgv[0]);
			m_pTmpPed->m_pTaskEntity=m_pTmpVeh;
		}
		return true;
	}
	else if (strcmp(cmdName,"vel")==0)
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
	else if (strcmp(cmdName,"fade")==0)
	{
		m_pGame->FadeCamera ( 1, atoi(cArgv[0]) == 1 );
		//SCM_ProcessCommand(&fade, 1000, atoi(cArgv[0]));
		return true;
	}
	else if (strcmp(cmdName,"cmd")==0)
	{
		CPed *pPlayer = Player_GetBySlot(0);
		DWORD dwVar;
		vec3_t vecPos;
		pPlayer->GetPosition(vecPos);

		__asm int 3
		SCM_ProcessCommand(&create_actor, 6, 1, vecPos[0], vecPos[1]+5, vecPos[2]+5, &dwVar);
		return true;
	}
	else if (strcmp(cmdName,"aval")==0)
	{
		if (iArgc<1)
			return false;

		Console_Printf("Available: %i\n", 0xffffffff, Model_IsLoaded(atoi(cArgv[0])));
		return true;
	}
	else if (strcmp(cmdName,"ps")==0)
	{
		FMOD::Sound *pSound;

		if (iArgc<1)
			return false;

		pSound = Sound_LoadFile(cArgv[0]);
		if (!pSound)
		{
			Console_Printf("Failed to load\n", 0xffffffff);
			return false;
		}
		Music_Play(pSound, -1, false);
		return true;
	}
	else if (strcmp(cmdName,"s")==0)
	{
		//CPed *pPlayer = m_pGame->GetPedRef(1);

		SCM_ProcessCommand(&play_music, atoi(cArgv[0]));
	}
	else if (strcmp(cmdName,"ss")==0)
	{
		if (iArgc<1)
			return false;

		Sound_PlayInternal2D(atoi(cArgv[0]), CHANNEL_FREE);
		return true;
	}
	else if (strcmp(cmdName,"sounddump")==0)
	{
		Sound_Dump();
	}
	else if (strcmp(cmdName,"w")==0)
	{
		SCM_ProcessCommand(&set_weather, atoi(cArgv[0]));
	}
	else if (strcmp(cmdName,"getstate")==0)
	{
		Console_Printf ( "System State: %u\n", 0xffffffff, (unsigned int)m_pGame->GetSystemState() );
		return true;
	}
	else if (strcmp(cmdName,"setstate")==0)
	{
		if (iArgc>0)
		{
			m_pGame->SetSystemState ( (eSystemState)atoi(cArgv[0]) );
			return true;
		}
	}
	else if (strcmp(cmdName,"getpedpool")==0)
	{
		if (iArgc>0)
		{
			Console_Printf("Mem: %.8x\n", 0xffffffff, (int)(*m_ppPedPool)->m_pPoolHeap+atoi(cArgv[0])*sizeof(CPed));
			//Console_Printf("sizeof(CPed) = %u\n", sizeof(class CPed));
			CPed *m_pPed = (CPed*)((int)(*m_ppPedPool)->m_pPoolHeap+atoi(cArgv[0])*sizeof(CPed));
			float fHealth = *(float*)((int)m_pPed+704);
			return true;
		}
	}
	else if (strcmp(cmdName,"setflags")==0)
	{
		if (iArgc>0)
		{
			CPed *pPlayer = Player_GetBySlot(0);
			pPlayer->m_cEntityFlags=(char)atoi(cArgv[0]);
			return true;
		}
	}
	else if (strcmp(cmdName,"setallpedhealth")==0)
	{
		if (iArgc>0)
		{
			float fHealth=(float)atof(cArgv[0]);

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
	}
	else if (strcmp(cmdName,"killallpeds")==0)
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
	else if (strcmp(cmdName,"showmenu")==0)
	{
		if (iArgc>0)
			*(BYTE*)(MEM_SHOWMENU) = atoi(cArgv[0]) % 2;
		return true;
	}
	else if (strcmp(cmdName,"pause")==0)
	{
		if (iArgc>0)
			*(BYTE*)(0x008F5BAD) = atoi(cArgv[0]) % 2;
		return true;
	}
	else if (strcmp(cmdName,"tp")==0)
	{
		if (iArgc>2)
		{
			CPed *player = Player_GetBySlot(0);
			float fPosX=(float)atof(cArgv[0]);
			float fPosY=(float)atof(cArgv[1]);
			float fPosZ=(float)atof(cArgv[2]);
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
		}
		return true;
	}
	else if (strcmp(cmdName,"getpos")==0)
	{
		CPed *player = Player_GetBySlot(0);
		vec3_t vecPos;
		player->GetPosition(vecPos);
		Console_Printf ( "%f, %f, %f\n", 0xffffffff, vecPos[0], vecPos[1], vecPos[2] );
		return true;
	}
	else if (strcmp(cmdName,"getrot")==0)
	{
		CPed *pPlayer = Player_GetBySlot(0);
		CEntity *pEntity = pPlayer->GetOccupiedVehicle();
		vec3_t vecRot;

		if (!pEntity)
			pEntity=pPlayer;
		pEntity->m_matrix.GetRotation(vecRot);

		Console_Printf("X: %f Y: %f Z: %f\n", 0xffffffff, vecRot[0], vecRot[1], vecRot[2]);
	}
	else if (strcmp(cmdName,"freem")==0)
	{
		if (iArgc>0)
		{
			Model_Free(atoi(cArgv[0]));
		}
	}
	else if (strcmp(cmdName,"psvc")==0)
	{
		if (iArgc>0)
		{
			CPed *player = Player_GetBySlot(0);
			vec3_t vecPos;
			unsigned short usModelID=atoi(cArgv[0]);

			if (player->GetOccupiedVehicle())
			{
				CVehicle *pVehicle = player->GetOccupiedVehicle();

				if (iArgc<2)
					pVehicle->m_matrix.GetOffset(0, 7.5, 2.5, vecPos);
				else
					pVehicle->m_matrix.GetOffset(0, 7.5, (float)atof(cArgv[1]), vecPos);

				m_pTmpVeh = Vehicle_Create ( usModelID, vecPos[0], vecPos[1], vecPos[2] );
			}
			else
			{
				if (iArgc<2)
					player->m_matrix.GetOffset(0, 5, 2.5, vecPos);
				else
					player->m_matrix.GetOffset(0, 5, (float)atof(cArgv[1]), vecPos);

				m_pTmpVeh = Vehicle_Create ( usModelID, vecPos[0], vecPos[1], vecPos[2] );
			}
		}
		return true;
	}
	else if (strcmp(cmdName,"psp")==0)
	{
		if (iArgc>0)
		{
			CPed *pPlayer = Player_GetBySlot(0);
			vec3_t vecPos;
			pPlayer->m_matrix.GetOffset(0, 3, 1, vecPos);

			m_pTmpPed = Ped_Create(atoi(cArgv[0]), vecPos[0], vecPos[1], vecPos[2]);
		}
		return true;
	}
	else if (strcmp(cmdName,"makeplayer")==0)
	{
		CPed *pPed=Player_GetBySlot(0);
		m_pTmpPed=pPed;
		CPed *pPlayer=Player_Create(0, pPed->m_matrix.m_vecPos[0], pPed->m_matrix.m_vecPos[1], pPed->m_matrix.m_vecPos[2]);
	}
	else if (strcmp(cmdName,"getveh")==0)
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
	else if (strcmp(cmdName,"dstr")==0)
	{
		delete m_pTmpVeh;
	}
	else if (strcmp(cmdName,"dp")==0)
	{
		m_pTmpPed->Destroy();
	}
	else if (strcmp(cmdName,"tflag")==0)
	{
		CPed *pPlayer = Player_GetBySlot(0);

		if (iArgc>0)
		{
			pPlayer->m_ucUnknown12 = atoi(cArgv[0]);
		}
		return true;
	}
	else if (strcmp(cmdName,"gspname")==0)
	{
		CSpecialPedModelInfo *pInfo = m_pSpecialPedModelPool->m_pData + atoi(cArgv[0]);

		if (iArgc>0)
		{
			Console_Printf("Name: %s\n", 0xffffffff, pInfo->m_modelType);
		}
		return true;
	}
	else if (strcmp(cmdName,"testlag")==0)
	{
		CPed *player = Player_GetBySlot(0);
		float fPosX=player->m_matrix.m_vecPos[0];
		float fPosY=player->m_matrix.m_vecPos[1]+15;
		float fPosZ=player->m_matrix.m_vecPos[2]+5;
		unsigned int n;

		/*__asm int 3
		SCM_ProcessCommand ( &request_model, usModelID );
		SCM_ProcessCommand ( &load_requested_models );

		while (!Model_IsLoaded(usModelID))
		{
			SCM_ProcessCommand ( &request_model, usModelID );
			SCM_ProcessCommand ( &load_requested_models );

			Sleep(10);
		}*/

		for (n=0; n<100; n++)
			Vehicle_Create ( 111, fPosX, fPosY, fPosZ+n*3 );
	}
	else if (strcmp(cmdName,"test2")==0)
	{
		CPed *pPed=(CPed*)((*m_ppPedPool)->m_pPoolHeap)+1;
		CVehicle *pVehicle=(CVehicle*)((*m_ppVehPool)->m_pPoolHeap)+1;

		Console_Printf("Ped: %i\n", 0xffffffff, *(BYTE*)(pPed+0x50));
		Console_Printf("Vehicle: %i\n", 0xffffffff, *(BYTE*)(pVehicle+0x50));
	}
	else if (strcmp(cmdName,"mi")==0)
	{
		if (iArgc>0)
		{
			CModelInfo* pModelInfo = Model_GetEntry(atoi(cArgv[0]));

			if (!pModelInfo)
			{
				Console_Printf ( "Not found\n", 0xffffffff );
				return true;
			}
			Console_Printf( "Model ID: '%s'\n", 0xffffffff, pModelInfo->m_cModelName );
			Console_Printf( "Instance: '%s'\n", 0xffffffff, pModelInfo->m_cInstanceName );
			Console_Printf( "Model Type: %i\n", 0xffffffff, pModelInfo->m_cModelType );
		}
		return true;
	}
	else if (strcmp(cmdName,"startgame")==0)
	{
		m_pGame->StartGame ( true );
		return true;
	}
	else if (strcmp(cmdName,"showdebug")==0)
	{
		if (iArgc>0)
		{
			if (strcmp(cArgv[0],"1")==0)
				m_bShowInternalDebug=true;
			else
				m_bShowInternalDebug=false;
		}
		return true;
	}
	else if (strcmp(cmdName,"nogui")==0)
	{
		m_bDisableHUD=!m_bDisableHUD;
	}
	return false;
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
	if (m_bIsFocused && !m_bConsoleIsTyping && !*(BYTE*)(MEM_SHOWMENU) && m_pGame->GetSystemState ()==GS_PLAYING_GAME)
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