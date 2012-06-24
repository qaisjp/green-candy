// SCM Interpreter Functions
#include "StdInc.h"
scmThread_t	customThread;
DWORD* m_pScriptBase=(DWORD*)0x0074B248;
char m_scriptBuffer[MAX_SCRIPT_SIZE];

/*==================================
	SCM Functions
==================================*/

void	SCM_GameInit()
{
	DWORD *blah;

	// Just create some stupid player
	Player_Create(0, 19, 72, 16.5);
	//__asm int 3
	//SCM_ProcessCommand(&create_player, 0, 0, 0, 0, &blah);
}

// Initialize environment
void	SCM_Init ()
{
	// Prevent game from loading main.scm
	//memset((void*)MEM_SCM_STARTMAIN, 0x90, 17);
	//*(BYTE*)FUNC_STARTMAIN = 0xC3;

	// Prevent script procession
	memset((void*)MEM_ProcessScriptCall, 0x90, 5);
	//memset((void*)0x0048C424, 0x90, 5);
	//memset((void*)MEM_ForwardScriptCall, 0x90, 5);

	// Instead of the scm file, we bootup game definitions
	*(DWORD*)(0x0048C425) = (DWORD)SCM_GameInit - 0x0048C429;
	//memset((void*)0x004393D0, 0x90, 0x20);
}

// Execute our scriptbuffer
bool	SCM_ExecuteBuffer ( )
{
	DWORD dwFunc=FUNC_ProcessSCMCommand;

	customThread.dwScriptIP = (int)m_scriptBuffer - (int)m_pScriptBase;
	__asm
	{
		lea ecx, customThread
		call dwFunc
	}
	return customThread.bJumpFlag == 1;
}

// Interpret a single command
bool	SCM_ProcessCommand ( const struct SCRIPT_COMMAND *cmd,  ... )
{
	char *pDec=(char*)cmd->Params;
	va_list vArgs;
	int iPos=2;
	DWORD *pVariableBase=m_pScriptBase;
	DWORD variableBuffer[MAX_VARIABLES];
	int iVariablePos=0;
	bool bResult;

	*(WORD*)(m_scriptBuffer) = cmd->OpCode;

	va_start(vArgs, cmd);
	while (*pDec)
	{
		switch (*pDec)
		{
		case 'i':
			{
				unsigned int integer = va_arg(vArgs, unsigned int);

				if ((integer & 0xFF) == integer)
				{
					*(WORD*)((int)m_scriptBuffer+iPos) = (SCMCAST_CHAR | (char)integer << 8);
					iPos+=2;
				}
				else if ((integer & 0xFFFF) == integer)
				{
					*(WORD*)((int)m_scriptBuffer+iPos) = (SCMCAST_SHORT | (short)integer << 8);
					iPos+=3;
				}
				else
				{
					m_scriptBuffer[iPos]=SCMCAST_INT;
					*(DWORD*)((int)m_scriptBuffer+iPos+1) = integer;
					iPos+=5;
				}
				break;
			}
		case 'v':
			{
				// Retrive the original variable and write our own one
				DWORD* pVar=va_arg(vArgs, DWORD*);

				variableBuffer[iVariablePos]=*pVariableBase;
				*(DWORD**)pVariableBase=pVar;
				pVariableBase++;
				iVariablePos++;

				// Set script information
				*(DWORD*)((int)m_scriptBuffer+iPos) = (SCMCAST_VAR | (WORD)((int)pVariableBase - (int)m_pScriptBase) << 8);
				iPos+=3;
				break;
			}
		case 's':
			{
				char* pString = va_arg(vArgs, char*);
				__asm
				{
					lea eax,m_scriptBuffer
					mov ecx,iPos
					mov ebx,pString
					imul ecx,4
					add eax,ecx
					emms
					movq mm0,dword ptr[ebx]
					movq dword ptr[eax],mm0
				}
				iPos+=8;
				break;
			}
		case 'f':
			{
				float flt = (float)va_arg(vArgs, double);

				m_scriptBuffer[iPos]=SCMCAST_FLOAT;
				*(float*)((int)m_scriptBuffer+iPos+1) = flt;
				iPos+=5;
				break;
			}
		}
		pDec++;
	}
	bResult = SCM_ExecuteBuffer();

	// Write back the original variables, if there
	for (iPos=0; iPos<iVariablePos; iPos++)
		*(DWORD*)(m_pScriptBase + iPos) = variableBuffer[iPos];

	va_end(vArgs);
	return bResult;
}