/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        Shared/logic/LuaCommon.h
*  PURPOSE:     Master lua header
*  DEVELOPERS:  Christian Myhre Lundheim <>
*               Jax <>
*               The_GTA
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#ifndef _BASE_LUA_COMMON_
#define _BASE_LUA_COMMON_

extern "C"
{
    #include "lua.h"
    #include "lualib.h"
    #include "lauxlib.h"
}

CLuaFunctionRef         luaM_toref              ( lua_State *luaVM, int iArgument );

#define TO_ELEMENTID(x) ((ElementID) reinterpret_cast < unsigned long > (x) )

#endif //_BASE_LUA_COMMON