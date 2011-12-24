/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        Shared/logic/LuaFunctionRef.cpp
*  PURPOSE:     Lua function reference
*  DEVELOPERS:  The_GTA <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#include <StdInc.h>

LuaFunctionRef::LuaFunctionRef()
{
    m_lua = NULL;
    m_ref = LUA_REFNIL;
    m_call = NULL;
}

LuaFunctionRef::LuaFunctionRef( LuaMain *lua, int ref, const void *call )
{
    m_lua = lua;
    m_ref = ref;
    m_call = call;
}

LuaFunctionRef::LuaFunctionRef( const LuaFunctionRef& other )
{
    m_lua = other.m_lua;
    m_ref = other.m_ref;
    m_call = other.m_call;

    m_lua->Reference( *this );
}

LuaFunctionRef::~LuaFunctionRef()
{
    m_lua->Dereference( *this );
}

LuaFunctionRef& LuaFunctionRef::operator = ( const LuaFunctionRef& other )
{
    m_lua->Dereference( *this );

    m_luaVM = other.m_luaVM;
    m_iFunction = other.m_iFunction;
    m_pFuncPtr = other.m_pFuncPtr;

    m_lua->Reference( *this );
    return *this;
}

int LuaFunctionRef::ToInt() const
{
    return m_ref;
}

bool LuaFunctionRef::operator == ( const LuaFunctionRef& other ) const
{
    return m_lua == other.m_lua && m_ref == other.m_ref;
}

bool LuaFunctionRef::operator != ( const LuaFunctionRef& other ) const
{
    return !(operator == ( other ));
}
