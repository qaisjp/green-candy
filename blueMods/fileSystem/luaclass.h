/*****************************************************************************
*
*  PROJECT:     Lua Interpreter
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        luaclass.h
*  PURPOSE:     Lua filesystem access
*  DEVELOPERS:  The_GTA <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#ifndef FU_CLASS
#define FU_CLASS

class unk abstract
{
public:
    virtual ~unk() = 0;
};

void lua_newclass( lua_State *L, unk *inter );
void* lua_getmethodtrans( lua_State *L );

#define LUA_CHECK( val ) if ( !(val) ) { lua_pushboolean( L, false ); return 1; }

#endif //FU_CLASS
