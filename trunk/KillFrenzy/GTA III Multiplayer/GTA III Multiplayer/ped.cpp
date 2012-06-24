#include "StdInc.h"

// Handle ped bombers
void		Ped_HandleBomber ( CPed* pPed )
{

}

void	__declspec(naked)	Hook_HandleBomber()
{
	__asm
	{
		push ecx
		call Ped_HandleBomber
		pop ecx
		ret
	}
}

void		Ped_Init()
{
	// Do some hooks
	//DetourFunction((PBYTE)FUNC_CPed_HandleBomber, (PBYTE)Hook_HandleBomber);

	// Disable population management
	//memset((void*)MEM_HandlePopulationCall, 0x90, 5);
}

CPed*		Player_Create(unsigned int uiSlot, float fPosX, float fPosY, float fPosZ)
{
	DWORD dwFreeSlot=FUNC_GetFreePedSlot;
	DWORD dwContructor=FUNC_CPlayer_Create;
	CPed *pPlayer;
	CPlayerInfoStruct *pInfo;

	if ( uiSlot > NUM_PLAYERS )
		return NULL;

	Model_RequestSpecialInternal( 0, "player", 5 );

	while ( !Model_IsLoaded( 0 ) )
		Model_LoadRequestedModels();

	__asm
	{
		call dwFreeSlot
		mov pPlayer,eax
	}
	if (!pPlayer)
	{
		MessageBox(NULL, "Could not create player instance!", "System Error", MB_OK);
		TerminateProcess( GetCurrentProcess(), EXIT_FAILURE );
	}
	__asm
	{
		mov ecx,pPlayer
		//int 3
		call dwContructor
	}
	//Streamer_RemoveEntity(pPlayer);
	pInfo=(CPlayerInfoStruct*)(MEM_players)+uiSlot;
	pInfo->m_pPed=pPlayer;
	pPlayer->m_ucUnknown12=2;

	//pPlayer->Teleport(0, 0, 0);
	pPlayer->SetPosition(fPosX, fPosY, fPosZ);
	Streamer_AddEntity(pPlayer);
	pPlayer->m_cGravity=100;
	return pPlayer;
}

CPed*		Ped_Create( unsigned short usModelID, float fPosX, float fPosY, float fPosZ )
{
	CPed *pPed;
	DWORD dwSlot=FUNC_GetFreePedSlot;
	DWORD dwConstruct=FUNC_CPed_Constructor;

	if (!Model_IsLoaded(usModelID))
	{
		Model_RequestInternal(usModelID);
		Model_LoadRequestedModels();
	}

	if (m_pPedPool->m_uiActiveItems==m_pPedPool->m_uiMaxItems)
	{
		Console_Printf("Warning: Maximum peds reached\n", 0xffffffff);
		return NULL;
	}
	__asm
	{
		call dwSlot
		mov pPed,eax
	}
	if (!pPed)
	{
		Console_Printf("Failed to allocate ped slot\n", 0xffffffff);
		return NULL;
	}
	__asm
	{
		mov ecx,pPed
		lea eax,usModelID
		push [eax]
		push 0h
		call dwConstruct
	}
	pPed->m_ucUnknown12=2;
	
	pPed->m_ucUnknown13 &= 0xFD;
	pPed->m_ucUnknown14 &= 0xFD;

	pPed->SetPosition(fPosX, fPosY, fPosZ);
	Streamer_AddEntity(pPed);
	pPed->m_ucSceneID = World_GetTownAtPosition(pPed->m_matrix.m_vecPos);
	return pPed;
}

CPed*		Player_GetBySlot( unsigned int uiSlot )
{
	CPlayerInfoStruct *pInfo;

	if ( uiSlot > NUM_PLAYERS )
		return NULL;
	
	pInfo = (CPlayerInfoStruct*)(MEM_players);
	return pInfo->m_pPed;
}

/*============================================
	Ped Class
============================================*/

void			CPed::Destroy()
{
	DWORD dwUnkDestroy = FUNC_CVehicleManager_UnknownDestroy;

	if ( m_bInVehicle )
	{
		CVehicle *pVehicle = m_pVehicle;

		if (pVehicle->m_ppPassengers[0] == this)
		{
			pVehicle->RemoveControlEntity();

			if ( pVehicle->m_uiUnknownType != 5 )
				pVehicle->m_uiUnknownType = 1;

			// Police vehicle logic, lets leave that out
			/*if (pVehicle->m_uiControlFlag==6)
			{

			}*/
		}
		else
			pVehicle->RemovePassengerEntity( this );
	}
	__asm
	{
		push this
		call dwUnkDestroy
		pop ecx
	}

    delete this;
}

CVehicle*		CPed::GetOccupiedVehicle()
{
	if ( !m_bInVehicle )
        return NULL;

	return m_pVehicle;
}