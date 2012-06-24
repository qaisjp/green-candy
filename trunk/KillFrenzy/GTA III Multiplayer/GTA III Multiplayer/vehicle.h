#ifndef _VEHICLE_
#define _VEHICLE_

#define VEHICLE_STRUCT_SIZE					1448
#define FUNC_CVehiclePool_GetFreeSlot		0x00551260
#define FUNC_CVehicle_Constructor			0x0052C8F0
#define FUNC_CVehicleManager_UnknownDestroy	0x004B3CE0
#define FUNC_CVehicleTraffic_Frame			0x00416580

class CVehicle : public CEntity // + 680
{
public:
	void					Destroy();

	BYTE					pad2[132];				// +680
	unsigned int			m_uiControlFlag;		// +812
	BYTE					pad[632];				// +816
};	// Size: 1448

// Vehicle Functions
//void			Vehicle_OnModelLoaded ( CEntityMP *pEntity );
CVehicle*		Vehicle_Create(unsigned short usModelID, float fPosX, float fPosY, float fPosZ);
void			Vehicle_HandleBombs ( CVehicle* pVehicle);
void			Vehicle_Init ();

#endif //_VEHICLE_