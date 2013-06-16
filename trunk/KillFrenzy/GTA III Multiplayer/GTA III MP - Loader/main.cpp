// KillFrenzy!
#include "main.h"
#include <wchar.h>
#include <string>
#include <map>

const char* GetLibraryPath()
{
#ifdef _DEBUG
	return "\\kf\\gtamod_d.dll";
#else
	return "\\kf\\gtamod.dll";
#endif
}

struct EnvironmentBlock
{
    EnvironmentBlock( wchar_t *envBlock )
    {
        while ( size_t len = wcslen( envBlock ) )
        {
            wchar_t *seperator = envBlock;

            while ( *seperator && *seperator != L'=' )
                seperator++;

            variables[std::wstring( envBlock, seperator )] = std::wstring( seperator + 1, envBlock + len );

            envBlock += len + 1;

            if ( *envBlock == L'\0' )
                break;
        }
    }

    std::wstring Get( const std::wstring& key )
    {
        variables_t::const_iterator iter = variables.find( key );

        if ( iter == variables.end() )
            return L"";

        return iter->second;
    }

    wchar_t* MakeTraverse( void )
    {
        size_t wtotallen = 1;

        for ( variables_t::const_iterator iter = variables.begin(); iter != variables.end(); iter++ )
        {
            wtotallen += iter->first.size() + iter->second.size() + 2;
        }

        wchar_t *envBlock = new wchar_t[wtotallen];
        wchar_t *cursor = envBlock;

        for ( variables_t::const_iterator iter = variables.begin(); iter != variables.end(); iter++ )
        {
            size_t len = iter->first.size();
            wcsncpy( cursor, iter->first.c_str(), len );
            cursor += len;

            *cursor++ = L'=';

            len = iter->second.size();
            wcsncpy( cursor, iter->second.c_str(), len );
            cursor += len;

            *cursor++ = L'\0';
        }

        *cursor++ = L'\0';

        return envBlock;
    }

    typedef std::map <std::wstring, std::wstring> variables_t;
    variables_t variables;
};

static char *environPointers = NULL;

inline std::wstring Convert( const char *str )
{
    size_t len = strlen( str );
    size_t wlen = MultiByteToWideChar( CP_UTF8, 0, str, (int)len, NULL, 0 );

    std::wstring wstr( wlen, L'0' );

    MultiByteToWideChar( CP_UTF8, 0, str, (int)len, (LPWSTR)wstr.c_str(), wlen );
    return wstr;
}

wchar_t* MakeEnvironment( void )
{
	// Grab newest environment variables
	wchar_t *envBlockPointers;
	HANDLE myToken = NULL;

	OpenProcessToken( GetCurrentProcess(), TOKEN_ALL_ACCESS, &myToken );

	CreateEnvironmentBlock( (LPVOID*)&envBlockPointers, myToken, 0 );

    EnvironmentBlock block( envBlockPointers );

    char myDir[1024];
    GetCurrentDirectory( sizeof(myDir), myDir );

    std::wstring dirEnv = Convert( myDir );

    std::wstring pathVar = block.Get( L"Path" );
    pathVar += dirEnv + L";";
    block.variables[L"Path"] = pathVar;

	CloseHandle( myToken );
	DestroyEnvironmentBlock( envBlockPointers );

    return block.MakeTraverse();
}

int	WINAPI WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow )
{
	STARTUPINFO stInfo;
	PROCESS_INFORMATION procInfo;
	HMODULE hKernel32=GetModuleHandle("Kernel32");
	const char *libPath=GetLibraryPath();
	void *pLibPathRemote;
	void *hRemoteThread;
	DWORD exitCode;
	SIZE_T numBytesWritten=0;

	ZeroMemory(&stInfo, sizeof(stInfo));
	ZeroMemory(&procInfo, sizeof(procInfo));

    wchar_t *envBlockPointers = MakeEnvironment();

	// Create a GTAIII process and attach our library to it
	bool success = CreateProcess( "D:/Programme/Rockstar Games/GTAIII/gta3.exe", NULL, NULL, NULL, 1, CREATE_SUSPENDED | CREATE_UNICODE_ENVIRONMENT, (LPVOID)envBlockPointers, NULL, &stInfo, &procInfo ) != FALSE;

	if (!success)
	{
		MessageBox(NULL, "Failed to create a GTAIII process!", "Error", MB_OK);
		return EXIT_FAILURE;
	}

    // Create absolute directory to library
    char pathBuf[1024];
    GetCurrentDirectory( sizeof(pathBuf), pathBuf );

    strcat( pathBuf, libPath );

    size_t pathLen = strlen( pathBuf );

	// Get our library path
	pLibPathRemote=VirtualAllocEx(procInfo.hProcess, NULL, pathLen+1, MEM_COMMIT, PAGE_READWRITE);
	if (!pLibPathRemote)
	{
		MessageBox(NULL, "Failed to allocate memory!", "Error", MB_OK);
		TerminateProcess(procInfo.hProcess, EXIT_FAILURE);
		return EXIT_FAILURE;
	}
	WriteProcessMemory(procInfo.hProcess, pLibPathRemote, pathBuf, pathLen+1, &numBytesWritten);
	if (numBytesWritten!=pathLen+1)
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
	VirtualFreeEx(procInfo.hProcess, pLibPathRemote, pathLen+1, MEM_RELEASE);

	// Resume execution
  	ResumeThread(procInfo.hThread);
	CloseHandle(procInfo.hProcess);
	CloseHandle(procInfo.hThread);
	CloseHandle(hRemoteThread);

	return EXIT_SUCCESS;
}