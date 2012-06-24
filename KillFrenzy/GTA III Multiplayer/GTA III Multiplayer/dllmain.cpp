// Injection library of KillFrenzy!
#include "StdInc.h"

int WINAPI DllMain(HINSTANCE hModule, DWORD dwReason, PVOID pvNothing)
{
	if (dwReason==DLL_PROCESS_ATTACH)
	{
		// Init stuff here
		Core_Init();
	}
	else if (dwReason==DLL_PROCESS_DETACH)
	{
		// Don't know, it will crash if I do it before, since GTA SA uses the code within its threads
		Core_Destroy();

		TerminateProcess(GetCurrentProcess(), EXIT_SUCCESS);
	}
	return TRUE;
}