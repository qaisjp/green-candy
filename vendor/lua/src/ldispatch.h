/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        vendor/lua/src/ldispatch.h
*  PURPOSE:     Lua dispatch extension
*  DEVELOPERS:  Martin Turski <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#ifndef _LUA_DISPATCHER_
#define _LUA_DISPATCHER_

ClassEnvDispatch*       luaQ_newclassenv( lua_State *L, Class *j );
ClassOutEnvDispatch*    luaQ_newclassoutenv( lua_State *L, Class *j );
ClassMethodDispatch*    luaQ_newclassmethodenv( lua_State *L, Class *j, GCObject *prevEnv );

#endif //_LUA_DISPATCHER_