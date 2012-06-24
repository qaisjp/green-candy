#ifndef __PED__
#define __PED__

#include "entity.h"

#define PED_STRUCT_SIZE						1520
#define MEM_PED_POOL						0x0A1ED192
#define FUNC_CPed_HandleBomber				0x004E5920
#define MEM_HandlePopulationCall			0x0048CA3B
#define FUNC_GetFreePedSlot					0x004C52C0
#define MEM_players							0x009414A8
#define NUM_PLAYERS							1
#define FUNC_CPlayer_Create					0x004EF890
#define FUNC_CPed_Constructor				0x004C0020

class CPed : public CEntity // +680
{
public:
	void				Destroy();
	CVehicle*			GetOccupiedVehicle();

	BYTE				pad[24];			//+680
	float				m_fHealth;			//+704
	int					m_iUnknown;			//+708
	float				m_fArmor;			//+712
	BYTE				pad3[68];			//+716
	CVehicle*			m_pVehicle;			//+784
	bool				m_bInVehicle;		//+788
	BYTE				pad4[387];			//+792
	char				m_cGravity;			//+1179
	BYTE				pad2[320];			//+792
};

class CPlayerInfoStruct
{
public:
	CPed*		m_pPed;
	BYTE		pad[312];
};

void		Ped_Init();
CPed*		Player_Create(unsigned int uiSlot, float fPosX, float fPosY, float fPosZ);
CPed*		Ped_Create(unsigned short usModelID, float fPosX, float fPosY, float fPosZ);
CPed*		Player_GetBySlot(unsigned int uiSlot);

#endif