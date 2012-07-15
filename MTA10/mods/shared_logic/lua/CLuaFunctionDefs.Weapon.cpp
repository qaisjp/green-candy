/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.0
*               (Shared logic for modifications)
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        mods/shared_logic/lua/CLuaFunctionDefs.Weapon.cpp
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
    LUA_DECLARE( getWeaponNameFromID )
    {
        int iArgument1 = lua_type ( L, 1 );
        if ( iArgument1 == LUA_TNUMBER || iArgument1 == LUA_TSTRING )
        {
            unsigned char ucID = static_cast < unsigned char > ( lua_tonumber ( L, 1 ) );

            char szBuffer [256];
            if ( CStaticFunctionDefinitions::GetWeaponNameFromID ( ucID, szBuffer, sizeof(szBuffer) ) )
            {
                lua_pushstring ( L, szBuffer );
                return 1;
            }
        }
        else
            m_pScriptDebugging->LogBadType( "getWeaponNameFromID" );

        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( getSlotFromWeapon )
    {
        if ( lua_type ( L, 1 ) == LUA_TNUMBER || lua_type ( L, 1 ) == LUA_TSTRING )
        {
            unsigned char ucWeaponID = static_cast < unsigned char > ( lua_tonumber ( L, 1 ) );
            char cSlot = CWeaponNames::GetSlotFromWeapon ( ucWeaponID );
            if ( cSlot >= 0 )
                lua_pushnumber ( L, cSlot );
            else
                lua_pushboolean ( L, false );
            //lua_pushnumber ( L, CWeaponNames::GetSlotFromWeapon ( ucWeaponID ) );
            return 1;
        }
        else
            m_pScriptDebugging->LogBadType( "getSlotFromWeapon" );

        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( getWeaponIDFromName )
    {
        if ( lua_type ( L, 1 ) == LUA_TSTRING )
        {
            const char* szName = lua_tostring ( L, 1 );
            unsigned char ucID;

            if ( CStaticFunctionDefinitions::GetWeaponIDFromName ( szName, ucID ) )
            {
                lua_pushnumber ( L, ucID );
                return 1;
            }
        }
        else
            m_pScriptDebugging->LogBadType( "getWeaponIDFromName" );

        lua_pushboolean ( L, false );
        return 1;
    }
}