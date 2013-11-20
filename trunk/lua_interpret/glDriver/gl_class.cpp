/*****************************************************************************
*
*  PROJECT:     Lua Interpreter
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        glDriver/gl_class.cpp
*  PURPOSE:     OpenGL driver class impl. for every OpenGL class
*  DEVELOPERS:  Martin Turski <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#include <StdInc.h>
#include "gl_internal.h"

namespace gl_class
{
    static LUA_DECLARE( getDriver )
    {
        glClass *obj = (glClass*)lua_getmethodtrans( L );

        obj->m_driver->PushStack( L );
        return 1;
    }

    static const luaL_Reg _interface[] =
    {
        LUA_METHOD( getDriver ),
        { NULL, NULL }
    };

    static LUA_DECLARE( constructor )
    {
        glClass *obj = (glClass*)lua_touserdata( L, lua_upvalueindex( 1 ) );

        ILuaClass *j = lua_refclass( L, LUA_ENVIRONINDEX );
        j->SetTransmit( LUACLASS_OPENGL_CLASS, obj );

        j->RegisterInterfaceTrans( L, _interface, 0, LUACLASS_OPENGL_CLASS );

        lua_pushcstring( L, "opengl_class" );
        lua_setfield( L, LUA_ENVIRONINDEX, "__type" );
        return 0;
    }
};

glClass::glClass( lua_State *L, glDriver *driver ) : LuaInstance( L ), m_driver( driver )
{
    // Construct the Lua representation of every glClass.
    Extend( L, gl_class::constructor );
}

glClass::~glClass( void )
{
    // We cannot do anything here.
    return;
}