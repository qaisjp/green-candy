#ifndef _WORLD_
#define _WORLD_

#define FUNC_GetTownAtPos				0x004B6A00
#define FUNC_CanLoadNewScene			0x00525F00
#define FUNC_ShowSceneTransition		0x004F62B0	//int iLastScene, int iNewScene, bool bUnk
#define MEM_uiCurrentScene				0x008F6408
#define FUNC_UnloadScene				0x00408780

unsigned char	World_GetTownAtPosition(const CVector& vecPos);
void			World_Init();

#endif //_WORLD_