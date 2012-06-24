#include "StdInc.h"

// Return the scene/town id by position
unsigned char	World_GetTownAtPosition(vec3_t vecPos)
{
	DWORD dwFunc=FUNC_GetTownAtPos;
	unsigned char	ucSceneID;

	__asm
	{
		push [vecPos]
		call dwFunc
		pop ecx
		mov ucSceneID,al
	}
	return ucSceneID;
}

// Handle the scene switch, checks whether it can load a new scene
bool		Hook_CanLoadNewScene()
{
	return true;
}

void		World_Init()
{
	// At Multiplayer, we have the scene switch all the time
	DetourFunction((PBYTE)FUNC_CanLoadNewScene, (PBYTE)Hook_CanLoadNewScene);
}