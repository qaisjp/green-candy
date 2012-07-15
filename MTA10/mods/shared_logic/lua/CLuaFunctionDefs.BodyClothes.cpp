/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.0
*               (Shared logic for modifications)
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        mods/shared_logic/lua/CLuaFunctionDefs.BodyClothes.cpp
*  PURPOSE:     Lua function definitions class
*  DEVELOPERS:  Ed Lyons <eai@opencoding.net>
*               Jax <>
*               Cecill Etheredge <ijsf@gmx.net>
*               Kevin Whiteside <kevuwk@gmail.com>
*               Chris McArthur <>
*               Derek Abdine <>
*               Christian Myhre Lundheim <>
*               Stanislav Bobrov <lil_toady@hotmail.com>
*               Alberto Alonso <rydencillo@gmail.com>
*
*****************************************************************************/

#include "StdInc.h"

namespace CLuaFunctionDefs
{
    LUA_DECLARE( getBodyPartName )
    {
        int iArgument1 = lua_type ( L, 1 );
        if ( iArgument1 == LUA_TNUMBER || iArgument1 == LUA_TSTRING )
        {
            unsigned char ucID = static_cast < unsigned char > ( lua_tonumber ( L, 1 ) );

            char szBuffer [256];
            if ( CStaticFunctionDefinitions::GetBodyPartName ( ucID, szBuffer ) )
            {
                lua_pushstring ( L, szBuffer );
                return 1;
            }
        }
        else
            m_pScriptDebugging->LogBadType( "getBodyPartName" );

        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( getClothesByTypeIndex )
    {
        int iArgument1 = lua_type ( L, 1 );
        int iArgument2 = lua_type ( L, 2 );
        if ( ( iArgument1 == LUA_TNUMBER || iArgument1 == LUA_TSTRING ) &&
            ( iArgument2 == LUA_TNUMBER || iArgument2 == LUA_TSTRING ) )
        {
            unsigned char ucType = static_cast < unsigned char > ( lua_tonumber ( L, 1 ) );
            unsigned char ucIndex = static_cast < unsigned char > ( lua_tonumber ( L, 2 ) );

            char szTexture [ 128 ], szModel [ 128 ];
            if ( CStaticFunctionDefinitions::GetClothesByTypeIndex ( ucType, ucIndex, szTexture, szModel ) )
            {
                lua_pushstring ( L, szTexture );
                lua_pushstring ( L, szModel );
                return 2;
            }
        }
        else
            m_pScriptDebugging->LogBadType( "getClothesByTypeIndex" );

        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( getTypeIndexFromClothes )
    {
        if ( lua_type ( L, 1 ) == LUA_TSTRING )
        {
            const char* szTexture = lua_tostring ( L, 1 );
            const char* szModel = NULL;
            if ( lua_type ( L, 2 ) == LUA_TSTRING )
                szModel = lua_tostring ( L, 2 );

            unsigned char ucType, ucIndex;
            if ( CStaticFunctionDefinitions::GetTypeIndexFromClothes ( const_cast < char* > ( szTexture ), const_cast < char* > ( szModel ), ucType, ucIndex ) )
            {
                lua_pushnumber ( L, ucType );
                lua_pushnumber ( L, ucIndex );
                return 2;
            }
        }
        else
            m_pScriptDebugging->LogBadType( "getTypeIndexFromClothes" );

        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( getClothesTypeName )
    {
        int iArgument1 = lua_type ( L, 1 );
        if ( iArgument1 == LUA_TNUMBER || iArgument1 == LUA_TSTRING )
        {
            unsigned char ucType = static_cast < unsigned char > ( lua_tonumber ( L, 1 ) );

            char szName [ 40 ];
            if ( CStaticFunctionDefinitions::GetClothesTypeName ( ucType, szName ) )
            {
                lua_pushstring ( L, szName );
                return 1;
            }
        }
        else
            m_pScriptDebugging->LogBadType( "getClothesTypeName" );

        lua_pushboolean ( L, false );
        return 1;
    }
}