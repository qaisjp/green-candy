#include "StdInc.h"
CPool**	m_ppPointerNodesPool = (CPool**)MEM_PTRNODES_POOL;
CPool**	m_ppEntryInfoStructsPool = (CPool**)MEM_ENTRYINFOSTRUCTS_POOL;
CPool**	m_ppPedPool = (CPool**)MEM_PEDPOOL;
CPool** m_ppVehPool = (CPool**)MEM_VEHPOOL;
CPool** m_ppBuildingsPool = (CPool**)MEM_BUILDINGSPOOL;
CPool** m_ppTreadablesPool = (CPool**)MEM_TREADABLESPOOL;
CPool** m_ppObjectsPool = (CPool**)MEM_OBJECTSPOOL;
CPool**	m_ppDummiesPool = (CPool**)MEM_DUMMIESPOOL;
CPool** m_ppShutdownPool = (CPool**)MEM_SHUTDOWNPOOL;

/*===========================
	Pool Functions
===========================*/

void	Hook_InitPools()
{
	DWORD dwPointerNodes=FUNC_InitPointerNodesPool;
	DWORD dwEntryInfo=FUNC_InitEntryInfoStructsPool;
	DWORD dwPed=FUNC_InitPedPool;
	DWORD dwVeh=FUNC_InitVehPool;
	DWORD dwBuilding=FUNC_InitBuildingsPool;
	DWORD dwTreadables=FUNC_InitTreadablesPool;
	DWORD dwObjects=FUNC_InitObjectsPool;
	DWORD dwDummies=FUNC_InitDummiesPool;
	DWORD dwShutdown=FUNC_InitShutdownPool;

	(*m_ppPointerNodesPool) = new CPool();
	__asm
	{
		mov ecx,eax
		push MAX_PTRNODES
		call dwPointerNodes
		mov ecx,m_ppPointerNodesPool
		mov [ecx],eax
	}
	(*m_ppEntryInfoStructsPool) = new CPool();
	__asm
	{
		mov ecx,eax
		push MAX_ENTRYINFOSTRUCTS
		call dwEntryInfo
		mov ecx,m_ppEntryInfoStructsPool
		mov [ecx],eax
	}
	(*m_ppPedPool) = new CPool();
	__asm
	{
		mov ecx,eax
		push MAX_PEDS
		call dwPed
		mov ecx,m_ppPedPool
		mov [ecx],eax
	}
	(*m_ppVehPool) = new CPool();
	__asm
	{
		mov ecx,eax
		push MAX_VEHICLES
		call dwVeh
		mov ecx,m_ppVehPool
		mov [ecx],eax
	}
	(*m_ppBuildingsPool) = new CPool();
	__asm
	{
		mov ecx,eax
		push MAX_BUILDINGS
		call dwBuilding
		mov ecx,m_ppBuildingsPool
		mov [ecx],eax
	}
	(*m_ppTreadablesPool) = new CPool();
	__asm
	{
		mov ecx,eax
		push MAX_TREADABLES
		call dwTreadables
		mov ecx,m_ppTreadablesPool
		mov [ecx],eax
	}
	(*m_ppObjectsPool) = new CPool();
	__asm
	{
		mov ecx,eax
		push MAX_OBJECTS
		call dwObjects
		mov ecx,m_ppObjectsPool
		mov [ecx],eax
	}
	(*m_ppDummiesPool) = new CPool();
	__asm
	{
		mov ecx,eax
		push MAX_DUMMIES
		call dwDummies
		mov ecx,m_ppDummiesPool
		mov [ecx],eax
	}
	(*m_ppShutdownPool) = new CPool();
	__asm
	{
		mov ecx,eax
		push MAX_SHUTDOWN
		call dwShutdown
		mov ecx,m_ppShutdownPool
		mov [ecx],eax
	}
}

// Initializes the pools, extend them
void	Pool_Init()
{
	// Hook the pool initializement function
	DetourFunction((PBYTE)0x004A1860, (PBYTE)Hook_InitPools);
}