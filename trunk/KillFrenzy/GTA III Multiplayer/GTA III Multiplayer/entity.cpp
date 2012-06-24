#include "StdInc.h"

void		CEntity::SetPosition(float fPosX, float fPosY, float fPosZ)
{
	m_matrix.m_vecPos[0] = fPosX;
	m_matrix.m_vecPos[1] = fPosY;
	m_matrix.m_vecPos[2] = fPosZ;
}

void		CEntity::SetPosition(vec3_t vecPos)
{
	m_matrix.m_vecPos[0] = vecPos[0];
	m_matrix.m_vecPos[1] = vecPos[1];
	m_matrix.m_vecPos[2] = vecPos[2];
}

void		CEntity::GetPosition(vec3_t vecPos)
{
	vecPos[0] = m_matrix.m_vecPos[0];
	vecPos[1] = m_matrix.m_vecPos[1];
	vecPos[2] = m_matrix.m_vecPos[2];
}

void		CEntity::SetRotation(float fRotX, float fRotY, float fRotZ)
{

}

void		CEntity::SetRotation(vec3_t vecRot)
{

}

void		CEntity::GetRotation(vec3_t vecRot)
{

}

// Teleport a entity
void		CEntity::Teleport(float fPosX, float fPosY, float fPosZ)
{
	float fX=fPosX;
	float fY=fPosY;
	float fZ=fPosZ;
	DWORD dwTeleport=FUNC_CEntity_Teleport;

	__asm
	{
		lea ecx,[ecx+4]
		push fZ
		push fY
		push fX
		call dwTeleport
	}
}

void		CEntity::RemoveControlEntity()
{
	m_unkFlags = (m_unkFlags & 0x7) | 0x20;
	m_ppPassengers[0] = NULL;
}

void		CEntity::RemovePassengerEntity(CEntity *pEntity)
{
	unsigned int n;
	unsigned int maxPassengers = m_uiPassengerFlag == 2 ? 8 : m_ucMaxPassengers;

	for (n=0; n<maxPassengers; n++)
	{
		CEntity *pPassenger = m_ppPassengers[n+1];

		if (pPassenger!=pEntity)
			continue;
		m_ppPassengers[n+1]=NULL;
		m_ucNumPassengers--;
		return;
	}
}

void		Hook_PlayerWasted()
{
	CPed *pPlayer=Player_GetBySlot(0);
	pPlayer->m_fHealth=0;
}

DWORD hookFuncDam=(DWORD)Hook_PlayerWasted;
void		Entity_Init()
{
	// Install the damage manager hook
	/**(BYTE*)(MEM_CDamageManager_PlayerWasted) = 0xFF;
	*(BYTE*)(MEM_CDamageManager_PlayerWasted+1) = 0x15;
	*(DWORD*)(MEM_CDamageManager_PlayerWasted+2) = (DWORD)&hookFuncDam;
	*(BYTE*)(MEM_CDamageManager_PlayerWasted+6) = 0xEB;
	*(BYTE*)(MEM_CDamageManager_PlayerWasted+7) = 0x76;*/
}

// Process entities
void		Entity_Frame()
{

}