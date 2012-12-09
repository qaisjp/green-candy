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
#include <excpt.h>
#include <windows.h>

#ifdef _WIN32
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

EXCEPTION_DISPOSITION FinalExceptionHandler( EXCEPTION_REGISTRATION *record, void *frame, CONTEXT *context, void *dispatcher );

static EXCEPTION_REGISTRATION _finalHandler =
{
    (EXCEPTION_REGISTRATION*)0xFFFFFFFF,
    NULL
};

static EXCEPTION_REGISTRATION _baseException =
{
    &_finalHandler,
    _defaultHandler
};
#endif

static void __stdcall _retHandler( lua_Thread *L )
{
    // Yield back.
    L->yield();
}

Fiber* luaX_newfiber( lua_State *L, size_t stackSize, FiberProcedure proc )
{
    Fiber *env = (Fiber*)luaM_malloc( L, sizeof(Fiber) );

    // Allocate stack memory
    if ( stackSize == 0 )
        stackSize = 2 << 20;    // a megabyte of stack space

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

    _finalHandler.Handler = (void*)GetProcAddress( GetModuleHandle("ntdll.dll"), "FinalExceptionHandler" );
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
        mov [eax],ebx
        mov [eax+4],edi
        mov [eax+8],esi
        add esp,4
        mov [eax+12],esp
        mov ebx,[esp-4]
        mov [eax+16],ebx
        mov [eax+20],ebp

#ifdef _WIN32
        mov ebx,fs:[0]
        mov ecx,fs:[4]
        mov edx,fs:[8]
        mov [eax+24],ecx
        mov [eax+28],edx
        mov [eax+32],ebx
#endif

        // Apply registers
        mov eax,[esp+4]
        mov ebx,[eax]
        mov edi,[eax+4]
        mov esi,[eax+8]
        mov esp,[eax+12]
        mov ebp,[eax+20]

#ifdef _WIN32
        // Apply exception and stack info
        mov ecx,[eax+24]
        mov edx,[eax+28]
        mov fs:[4],ecx
        mov fs:[8],edx
        mov ecx,[eax+32]
        mov fs:[0],ecx
#endif

        jmp [eax+16]
    }
}