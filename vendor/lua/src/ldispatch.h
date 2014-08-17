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

LUAI_FUNC ClassEnvDispatch*     luaQ_newclassenv( lua_State *L, Class *j );
LUAI_FUNC ClassOutEnvDispatch*  luaQ_newclassoutenv( lua_State *L, Class *j );
LUAI_FUNC ClassMethodDispatch*  luaQ_newclassmethodenv( lua_State *L, Class *j, GCObject *prevEnv );
LUAI_FUNC void luaQ_free( lua_State *L, Dispatch *q );

// Module initialization.
LUAI_FUNC void luaQ_init( void );
LUAI_FUNC void luaQ_shutdown( void );

#endif //_LUA_DISPATCHER_