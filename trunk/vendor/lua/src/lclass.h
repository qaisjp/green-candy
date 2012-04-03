/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        vendor/lua/src/lclass.h
*  PURPOSE:     Lua class system extension
*  DEVELOPERS:  The_GTA <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#ifndef _LUA_CLASS_
#define _LUA_CLASS_

#include "lobject.h"

LUAI_FUNC Class* luaJ_new( lua_State *L, int nargs );
LUAI_FUNC void luaJ_construct( lua_State *L, int nargs );
LUAI_FUNC void luaJ_free( lua_State *L, Class *c );

#endif //_LUA_CLASS_