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

#include "luacore.h"

#include "lgc.h"
#include "lobject.h"

ClassEnvDispatch* luaQ_newclassenv( lua_State *L, Class *j )
{
    ClassEnvDispatch *q = lua_new <ClassEnvDispatch> ( L );

    if ( q )
    {
        luaC_link( L, q, LUA_TDISPATCH );
        q->m_class = j;
    }
    return q;
}

ClassOutEnvDispatch* luaQ_newclassoutenv( lua_State *L, Class *j )
{
    ClassOutEnvDispatch *q = lua_new <ClassOutEnvDispatch> ( L );

    if ( q )
    {
        luaC_link( L, q, LUA_TDISPATCH );
        q->m_class = j;
    }
    return q;
}

ClassMethodDispatch* luaQ_newclassmethodenv( lua_State *L, Class *j, GCObject *prevEnv )
{
    ClassMethodDispatch *q = lua_new <ClassMethodDispatch> ( L );

    if ( q )
    {
        luaC_link( L, q, LUA_TDISPATCH );
        q->m_class = j;
        luaC_objbarrier( L, q, j );
        q->m_prevEnv = prevEnv;
        luaC_objbarrier( L, q, prevEnv );
    }
    return q;
}

void luaQ_free( lua_State *L, Dispatch *q )
{
    // Free using default operator.
    lua_delete <Dispatch> ( L, q );
}

// Module initialization.
void luaQ_init( void )
{
    return;
}

void luaQ_shutdown( void )
{
    return;
}