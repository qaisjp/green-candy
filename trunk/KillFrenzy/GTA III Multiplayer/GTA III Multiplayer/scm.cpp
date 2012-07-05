// SCM Interpreter Functions
#include "StdInc.h"
scmThread_t	customThread;
DWORD* m_pScriptBase=(DWORD*)0x0074B248;

/*==================================
	SCM Functions
==================================*/

void	SCM_GameInit()
{
    Core_InitGame();
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

scmThread::scmThread( unsigned short opCode )
{
    m_cursor = 2;
    m_varCursor = 0;

    *(unsigned short*)m_scriptBuffer = opCode;
}

scmThread::~scmThread()
{
    // Write back the original vars
    unsigned int n;

    for ( n=0; n<m_varCursor; n++ )
        *(int*)( m_pScriptBase + n ) = m_origVarBuffer[n];
}

void scmThread::PushInt( int num )
{
	if ( (num & 0xFF) == num )
	{
		*(short*)((int)m_scriptBuffer + m_cursor) = (SCMCAST_CHAR | (char)num << 8);
		m_cursor += 2;
        return;
	}
	else if ( (num & 0xFFFF) == num )
	{
        m_scriptBuffer[m_cursor++] = SCMCAST_CHAR;
		*(short*)((int)m_scriptBuffer + m_cursor) = ((short)num << 8);
		m_cursor += sizeof(short);
        return;
	}

	m_scriptBuffer[ m_cursor++ ] = SCMCAST_INT;
	*(int*)((int)m_scriptBuffer + m_cursor) = num;
	m_cursor += sizeof(int);
}

void scmThread::PushString( const std::string& string )
{
    unsigned int n;
    size_t len = min(8, string.size());

    for ( n=0; n<len; n++ )
        m_scriptBuffer[ m_cursor++ ] = string[n];

    for ( n; n<8; n++ )
        m_scriptBuffer[ m_cursor++ ] = 0;
}

void scmThread::PushFloat( float num )
{
	m_scriptBuffer[ m_cursor++ ] = SCMCAST_FLOAT;

	*(short*)((int)m_scriptBuffer + m_cursor ) = (short)( num * 0x10 );
	m_cursor += sizeof(short);
}

void scmThread::PrepareVariable()
{
    if ( m_varCursor == 4 )
        return;

    unsigned int *varPos = (unsigned int*)( m_pScriptBase + m_varCursor );
    unsigned int *varBase = (unsigned int*)( &m_origVarBuffer + m_varCursor );

	m_origVarBuffer[ m_varCursor ] = *varPos;
    *varPos = *varBase;

	// Set script information
    m_scriptBuffer[m_cursor++] = SCMCAST_VAR;
    *(unsigned short*)((int)m_scriptBuffer + m_cursor) = m_varCursor++ * 4;
	m_cursor += sizeof(unsigned short);
}

bool scmThread::Process()
{
	DWORD dwFunc = FUNC_ProcessSCMCommand;

	customThread.dwScriptIP = (int)m_scriptBuffer - (int)m_pScriptBase;

	__asm
	{
		lea ecx,customThread
		call dwFunc
	}
	return customThread.bJumpFlag == 1;
}

int scmThread::GetResult( unsigned int id )
{
    if ( id > 3 )
        return 0;

    return *(int*)( m_pScriptBase + id );
}