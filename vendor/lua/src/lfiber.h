/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        vendor/lua/src/lfiber.h
*  PURPOSE:     Lua OS fiber implementation
*  DEVELOPERS:  The_GTA <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#ifndef _l_fiber
#define _l_fiber

typedef void    (__stdcall*FiberProcedure) ( class lua_Thread *L );

struct Fiber
{
    // Preserve __cdecl
    unsigned int ebx;   // 0
    unsigned int edi;   // 4
    unsigned int esi;   // 8
    unsigned int esp;   // 12
    unsigned int eip;   // 16
    unsigned int ebp;   // 20
    
    unsigned int *stack_base, *stack_limit;
    void *except_info;

    size_t stackSize;
};

LUAI_FUNC Fiber* luaX_newfiber( lua_State *L, size_t stackSize, FiberProcedure proc );
LUAI_FUNC Fiber* luaX_makefiber( lua_State *L );
LUAI_FUNC void luaX_closefiber( lua_State *L, Fiber *env );
LUAI_FUNC void __cdecl luaX_switch( Fiber *from, Fiber *to );
LUAI_FUNC void __cdecl luaX_qswitch( Fiber *from, Fiber *to );

#endif //_l_fiber