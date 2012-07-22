/*****************************************************************************
*
*  PROJECT:     KillFrenzy! 1.0
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        lua/luaroot.h
*  PURPOSE:     Game Lua root
*  DEVELOPERS:  The_GTA <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#ifndef _LUA_ROOT_
#define _LUA_ROOT_

#define LUACLASS_ROOT   16

class LuaRoot : public LuaClass
{
public:
                            LuaRoot( lua_State *L );
                            ~LuaRoot();
};

#endif //_LUA_ROOT_