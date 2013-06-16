/*****************************************************************************
*
*  PROJECT:     Lua Interpreter
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        dxElements/include.h
*  PURPOSE:     Lua dxElements GUI development environment
*  DEVELOPERS:  Martin Turski <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#ifndef _LUA_INCLUDE_DXELEMENTS_
#define _LUA_INCLUDE_DXELEMENTS_

#define LUA_DECLARE(x) int x( lua_State *L )
#define LUA_REGISTER( L, x ) lua_register( L, #x, x )

#include <CVector2D.h>
#include "lua_dxElements.h"
#include "lua_dxElements_dxRoot.h"
#include "lua_dxElements_master.h"

#endif //_LUA_INCLUDE_DXELEMENTS_