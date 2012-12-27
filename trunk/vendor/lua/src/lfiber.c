/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        vendor/lua/src/lfiber.c
*  PURPOSE:     Lua OS fiber implementation
*  DEVELOPERS:  The_GTA <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#include "lua.h"
#include "lfiber.h"
#include "lstate.h"
#include "lmem.h"

#ifdef _WIN32
#include <excpt.h>
#include <windows.h>

typedef struct _EXCEPTION_REGISTRATION
{
     struct _EXCEPTION_REGISTRATION* Next;
     void *Handler;
} EXCEPTION_REGISTRATION, *PEXCEPTION_REGISTRATION;

static EXCEPTION_DISPOSITION _defaultHandler( EXCEPTION_REGISTRATION *record, void *frame, CONTEXT *context, void *dispatcher )
{
    exit( 0x0DAB00B5 );
    return ExceptionContinueSearch;
}

static EXCEPTION_REGISTRATION _baseException =
{
    (EXCEPTION_REGISTRATION*)0xFFFFFFFF,
    _defaultHandler
};
#endif

#pragma warning(disable:4733)

static void __declspec(naked) _retHandler( lua_Thread *L )
{
	__asm
	{
		// Set thread status to terminated
		mov eax,[esp+4]
		mov [eax]lua_Thread.status,THREAD_TERMINATED

		mov ecx,eax

        // Apply registers
        mov eax,[eax]lua_Thread.callee
        mov ebx,[eax]Fiber.ebx
        mov edi,[eax]Fiber.edi
        mov esi,[eax]Fiber.esi
        mov esp,[eax]Fiber.esp
        mov ebp,[eax]Fiber.ebp

		push [eax+16]		// The return address
		push ecx			// Push thread as preparation

#ifdef _WIN32
        // Apply exception and stack info
        mov ecx,[eax]Fiber.stack_base
        mov edx,[eax]Fiber.stack_limit
        mov fs:[4],ecx
        mov fs:[8],edx
        mov ecx,[eax]Fiber.except_info
        mov fs:[0],ecx
#endif

		// Terminate our thread
		call luaE_terminate
		add esp,4

        ret
	}
}

Fiber* luaX_newfiber( lua_State *L, size_t stackSize, FiberProcedure proc )
{
    Fiber *env = (Fiber*)luaM_malloc( L, sizeof(Fiber) );

    // Allocate stack memory
    if ( stackSize == 0 )
        stackSize = 2 << 17;    // 128 kilobytes of stack space (is this enough?)

    unsigned int *stack = (unsigned int*)luaM_malloc( L, stackSize );
    env->stack_base = (unsigned int*)( (unsigned int)stack + stackSize );
    env->stack_limit = stack;

    stack = env->stack_base;

    // Once entering, the first argument should be the thread
    *--stack = (unsigned int)L;
    *--stack = (unsigned int)&exit;
    *--stack = (unsigned int)L;
    *--stack = (unsigned int)&_retHandler;
    env->esp = (unsigned int)stack;

    env->eip = (unsigned int)proc;
    
#ifdef _WIN32
    env->except_info = &_baseException;
#endif

    env->stackSize = stackSize;
    return env;
}

Fiber* luaX_makefiber( lua_State *L )
{
    Fiber *fiber = (Fiber*)luaM_malloc( L, sizeof(Fiber) );
    fiber->stackSize = 0;
    return fiber;
}

void luaX_closefiber( lua_State *L, Fiber *fiber )
{
    if ( fiber->stackSize )
        luaM_freemem( L, fiber->stack_limit, fiber->stackSize );

    luaM_free( L, fiber );
}

void __declspec(naked) luaX_switch( Fiber *from, Fiber *to )
{
    __asm
    {
        // Save current environment
        mov eax,[esp+4]
        mov [eax]Fiber.ebx,ebx
        mov [eax]Fiber.edi,edi
        mov [eax]Fiber.esi,esi
        add esp,4
        mov [eax]Fiber.esp,esp
        mov ebx,[esp-4]
        mov [eax]Fiber.eip,ebx
        mov [eax]Fiber.ebp,ebp

#ifdef _WIN32
        // Save exception and stack info
        mov ebx,fs:[0]
        mov ecx,fs:[4]
        mov edx,fs:[8]
        mov [eax]Fiber.stack_base,ecx
        mov [eax]Fiber.stack_limit,edx
        mov [eax]Fiber.except_info,ebx
#endif

        // Apply registers
        mov eax,[esp+4]
        mov ebx,[eax]Fiber.ebx
        mov edi,[eax]Fiber.edi
        mov esi,[eax]Fiber.esi
        mov esp,[eax]Fiber.esp
        mov ebp,[eax]Fiber.ebp

#ifdef _WIN32
        // Apply exception and stack info
        mov ecx,[eax]Fiber.stack_base
        mov edx,[eax]Fiber.stack_limit
        mov fs:[4],ecx
        mov fs:[8],edx
        mov ecx,[eax]Fiber.except_info
        mov fs:[0],ecx
#endif

        jmp dword ptr[eax+16]
    }
}

// For use with yielding
void __declspec(naked) luaX_qswitch( Fiber *from, Fiber *to )
{
    __asm
    {
        // Save current environment
        mov eax,[esp+4]
        mov [eax]Fiber.ebx,ebx
        mov [eax]Fiber.edi,edi
        mov [eax]Fiber.esi,esi
        add esp,4
        mov [eax]Fiber.esp,esp
        mov ebx,[esp-4]
        mov [eax]Fiber.eip,ebx
        mov [eax]Fiber.ebp,ebp

#ifdef _WIN32
        // Save exception info
        mov ebx,fs:[0]
        mov [eax]Fiber.except_info,ebx
#endif

        // Apply registers
        mov eax,[esp+4]
        mov ebx,[eax]Fiber.ebx
        mov edi,[eax]Fiber.edi
        mov esi,[eax]Fiber.esi
        mov esp,[eax]Fiber.esp
        mov ebp,[eax]Fiber.ebp

#ifdef _WIN32
        // Apply exception and stack info
        mov ecx,[eax]Fiber.stack_base
        mov edx,[eax]Fiber.stack_limit
        mov fs:[4],ecx
        mov fs:[8],edx
        mov ecx,[eax]Fiber.except_info
        mov fs:[0],ecx
#endif

        jmp dword ptr[eax+16]
    }
}