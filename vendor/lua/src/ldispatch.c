/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        vendor/lua/src/ldispatch.c
*  PURPOSE:     Lua dispatch extension
*  DEVELOPERS:  Martin Turski <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#include "lua.h"
#include "lgc.h"
#include "lobject.h"

ClassEnvDispatch* luaQ_newclassenv( lua_State *L, Class *j )
{
    ClassEnvDispatch *q = new (L) ClassEnvDispatch;

    luaC_link( L, q, LUA_TDISPATCH );
    q->m_class = j;
    return q;
}

ClassOutEnvDispatch* luaQ_newclassoutenv( lua_State *L, Class *j )
{
    ClassOutEnvDispatch *q = new (L) ClassOutEnvDispatch;

    luaC_link( L, q, LUA_TDISPATCH );
    q->m_class = j;
    return q;
}

ClassMethodDispatch* luaQ_newclassmethodenv( lua_State *L, Class *j, GCObject *prevEnv )
{
    ClassOutEnvDispatch *q = new (L) ClassOutEnvDispatch;

    luaC_link( L, q, LUA_TDISPATCH );
    q->m_class = j;
    q->m_prevEnv = prevEnv;
    return q;
}