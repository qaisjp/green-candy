// KillFrenzy!
#include "main.h"

const char* GetLibraryPath()
{
#ifdef _DEBUG
	return "kf/gtamod_d.dll";
#else
	return "kf/gtamod.dll";
#endif
}

int	WINAPI WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow )
{
	STARTUPINFO stInfo;
	PROCESS_INFORMATION procInfo;
	HMODULE hKernel32=GetModuleHandle("Kernel32");
	const char *libPath=GetLibraryPath();
	void *pLibPathRemote;
	void *hRemoteThread;
	unsigned int exitCode;
	unsigned int numBytesWritten=0;
	ZeroMemory(&stInfo, sizeof(stInfo));
	ZeroMemory(&procInfo, sizeof(procInfo));

	// Create a GTAIII process and attach our library to it
	if (!CreateProcess("gta3.exe", NULL, NULL, NULL, 0, CREATE_SUSPENDED, NULL, NULL, &stInfo, &procInfo))
	{
		MessageBox(NULL, "Failed to create a GTAIII process!", "Error", MB_OK);
		return EXIT_FAILURE;
	}
	// Get our library path
	pLibPathRemote=VirtualAllocEx(procInfo.hProcess, NULL, strlen(libPath)+1, MEM_COMMIT, PAGE_READWRITE);
	if (!pLibPathRemote)
	{
		MessageBox(NULL, "Failed to allocate memory!", "Error", MB_OK);
		TerminateProcess(procInfo.hProcess, EXIT_FAILURE);
		return EXIT_FAILURE;
	}
	WriteProcessMemory(procInfo.hProcess, pLibPathRemote, libPath, strlen(libPath)+1, &numBytesWritten);
	if (numBytesWritten!=strlen(libPath)+1)
	{
		MessageBox(NULL, "'WriteProcessMemory()' failed!", "Error", MB_OK);
		TerminateProcess(procInfo.hProcess, EXIT_FAILURE);
		return EXIT_FAILURE;
	}

	// Make the executable run our code
	hRemoteThread=CreateRemoteThread(procInfo.hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)GetProcAddress(hKernel32, "LoadLibraryA"), pLibPathRemote, 0, NULL);
	if (!hRemoteThread)
	{
		MessageBox(NULL, "Could not inject remote thread!", "Error", MB_OK);
		TerminateProcess(procInfo.hProcess, EXIT_FAILURE);
		return EXIT_FAILURE;
	}
	WaitForSingleObject(hRemoteThread, INFINITE);
	GetExitCodeThread(hRemoteThread, &exitCode);
	/*if (exitCode!=TRUE)
	{
		MessageBox(NULL, "Injection failed", "Error", MB_OK);
		TerminateProcess(procInfo.hProcess, EXIT_FAILURE);
		return EXIT_FAILURE;
	}*/
	// We dont need the memory allocated at the process anymore
	VirtualFreeEx(procInfo.hProcess, pLibPathRemote, strlen(libPath)+1, MEM_RELEASE);
    
	// Resume execution
  	ResumeThread(procInfo.hThread);
	CloseHandle(procInfo.hProcess);
	CloseHandle(procInfo.hThread);
	CloseHandle(hRemoteThread);

	return EXIT_SUCCESS;
}