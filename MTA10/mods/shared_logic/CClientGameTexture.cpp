/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*               (Shared logic for modifications)
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        mods/shared_logic/CClientGameTexture.cpp
*  PURPOSE:     RenderWare texture entity
*  DEVELOPERS:  The_GTA <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#include <StdInc.h>

typedef std::vector <unsigned short> imports_t;

static int texture_getName( lua_State *L )
{
    CClientGameTexture *tex = (CClientGameTexture*)lua_touserdata( L, lua_upvalueindex( 1 ) );
    const std::string& name = tex->GetName();

    lua_pushlstring( L, name.c_str(), name.size() );
    return 1;
}

static int texture_getHash( lua_State *L )
{
    lua_pushnumber( L, ((CClientGameTexture*)lua_touserdata( L, lua_upvalueindex( 1 ) ))->GetHash() );
    return 1;
}

static const luaL_Reg texture_interface[] =
{
    { "getName", texture_getName },
    { "getHash", texture_getHash },
    { NULL, NULL }
};

static int luaconstructor_texture( lua_State *L )
{
    CClientGameTexture *tex = (CClientGameTexture*)lua_touserdata( L, lua_upvalueindex( 1 ) );

    ILuaClass& j = *lua_refclass( L, 1 );
    j.SetTransmit( LUACLASS_TEXTURE, tex );

    lua_pushvalue( L, LUA_ENVIRONINDEX );
    lua_pushvalue( L, lua_upvalueindex( 1 ) );
    luaL_openlib( L, NULL, texture_interface, 1 );

    lua_basicprotect( L );

    lua_pushlstring( L, "texture", 7 );
    lua_setfield( L, LUA_ENVIRONINDEX, "__type" );
    return 0;
}

CClientGameTexture::CClientGameTexture( LuaClass& root, CTexture& tex ) : LuaElement( root ), m_tex( tex )
{
    lua_State *L = root.GetVM();

    PushStack( L );
    lua_pushlightuserdata( L, this );
    lua_pushcclosure( L, luaconstructor_texture, 1 );
    luaJ_extend( L, -2, 0 );
}

CClientGameTexture::~CClientGameTexture()
{
    delete &m_tex;
}

bool CClientGameTexture::Import( unsigned short id )
{
    if ( !m_tex.Import( id ) )
        return false;

    g_pClientGame->GetManager()->Restream( id );
    return true;
}

bool CClientGameTexture::IsImported( unsigned short id ) const
{
    return m_tex.IsImported( id );
}

void CClientGameTexture::Remove( unsigned short id )
{
    if ( !m_tex.Remove( id ) )
        return;

    g_pClientGame->GetManager()->Restream( id );
}