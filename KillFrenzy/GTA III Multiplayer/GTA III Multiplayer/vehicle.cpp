#include "StdInc.h"

/*==============================
	Vehicle Functions
==============================*/
float fMod2=9.0f;

// Function to spawn a vehicle
CVehicle*	Vehicle_Create(unsigned short usModelID, float fPosX, float fPosY, float fPosZ)
{
	DWORD dwFreeSlot=FUNC_CVehiclePool_GetFreeSlot;
	DWORD dwFunc=FUNC_CVehicle_Constructor;
	DWORD dwUnk=0x004B6A00;
	//DWORD dwUnk2=0x00454060;
	//DWORD dwUnk3=0x0041F820;
	unsigned int uiModel=usModelID;
	CVehicle *pVehicle;

	if (!Model_IsLoaded(usModelID))
	{
		Model_RequestInternal(usModelID);
		Model_LoadRequestedModels();
	}

	if ((*m_ppVehPool)->m_uiActiveItems == (*m_ppVehPool)->m_uiMaxItems)
	{
		Console_Printf("Warning: Vehicle pool full\n", 0xffffffff);
		return false;
	}

	//__asm int 3
	__asm
	{
		//push 0x5a8
		call dwFreeSlot
		mov pVehicle,eax
	}
	if (!pVehicle)
	{
		Console_Printf("Warning: Failed to alloc vehicle buffer\n", 0xffffffff);
		return false;
	}

	__asm
	{
		mov ecx,pVehicle
		push 4
		push uiModel
		call dwFunc
	}
	//pVehicle->Teleport(0, 0, 0);
	pVehicle->m_matrix.m_vecPos[0]=fPosX;
	pVehicle->m_matrix.m_vecPos[1]=fPosY;
	pVehicle->m_matrix.m_vecPos[2]=fPosZ;
		
	// Update flags
	pVehicle->m_unkFlags = (pVehicle->m_unkFlags & 0x07) | 0x20;
	pVehicle->m_unkFlags2 = (pVehicle->m_unkFlags & 0xF7) | 0x08;
	__asm
	{
		//
		//push pVehicle
		//push dwPtrMod
		//call dwUnk2
		//add esp,8

		//push pVehicle
		//call dwUnk3
		//add esp,4
		mov ecx,pVehicle
	}
	pVehicle->m_ucUnknown2=0;
	pVehicle->m_ucUnknown1=0;
	pVehicle->m_ucUnknown3=0;
	pVehicle->m_fUnknown9=fMod2;
	pVehicle->m_ucUnknown10=0;
	pVehicle->m_ucUnknown11=0;
	pVehicle->m_ucUnknown4=0;

	// Set the corresponding scene ID
	pVehicle->m_ucSceneID = World_GetTownAtPosition(pVehicle->m_matrix.m_vecPos);
	__asm
	{
		mov al,[ecx+0x1F5]
		and al,0xEF
		mov [ecx+0x1F5],al
		//
		//pop ecx
		mov ecx,pVehicle
		mov al,[ecx+0x1F7]
		and al,0xFB
		or al,4
		mov [ecx+0x1F7],al
		mov word ptr [ecx+290h],0
	}
	Streamer_AddEntity ( pVehicle );
	return pVehicle;
}

void		CVehicle::Destroy()
{
	DWORD dwVehicleInterface=(DWORD)this;
	DWORD dwUnknownDestroy=FUNC_CVehicleManager_UnknownDestroy;

	Streamer_RemoveEntity( this );
	__asm
	{
		push dwVehicleInterface
		call dwUnknownDestroy
		pop ecx
	}

    delete this;
}

#if 0
// Callback when model loaded
void		Vehicle_OnModelLoaded ( CEntityMP *pEntity )
{
	CVehicleMP *pVehicle = (CVehicleMP*)pEntity;

	pVehicle->Create();
}

CVehicleMP::CVehicleMP ( unsigned short usModelID, float fPosX, float fPosY, float fPosZ )
{
	m_usModelID=usModelID;
	m_vecPos[0]=fPosX;
	m_vecPos[1]=fPosY;
	m_vecPos[2]=fPosZ;
	m_pVehicle=NULL;

	// Request creation
	Model_Request(usModelID, (CEntityMP*)this, Vehicle_OnModelLoaded);
}

CVehicleMP::~CVehicleMP ()
{
	if (m_pVehicle)
	{
		Destroy();
	}
}
#endif

void		Vehicle_Init()
{
	// Disable vehicle traffic
	//*(BYTE*)(FUNC_CVehicleTraffic_Frame) = 0xC3;
}