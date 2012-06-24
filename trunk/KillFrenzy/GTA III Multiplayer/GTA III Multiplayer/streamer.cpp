#include "StdInc.h"

void	Streamer_Init()
{

}

bool	Streamer_AddEntity(CEntity *pEntity)
{
	DWORD dwInterface = (DWORD)pEntity;
	DWORD dwAdd = FUNC_AddEntityToWorld;

	__asm
	{
		push dwInterface
		call dwAdd
		pop ecx
	}
	return true;
}

void	Streamer_RemoveEntity(CEntity *pEntity)
{
	DWORD dwInterface = (DWORD)pEntity;
	DWORD dwRemove = FUNC_RemoveEntityFromWorld;

	__asm
	{
		push dwInterface
		call dwRemove
		pop ecx
	}
}