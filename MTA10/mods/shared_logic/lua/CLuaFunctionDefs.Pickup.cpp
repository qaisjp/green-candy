/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.0
*               (Shared logic for modifications)
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        mods/shared_logic/lua/CLuaFunctionDefs.Pickup.cpp
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
    LUA_DECLARE( createPickup )
    {
        // Grab all the argument types
        int iArgument1 = lua_type ( L, 1 );
        int iArgument2 = lua_type ( L, 2 );
        int iArgument3 = lua_type ( L, 3 );
        int iArgument4 = lua_type ( L, 4 );
        int iArgument5 = lua_type ( L, 5 );
        int iArgument6 = lua_type ( L, 6 );
        int iArgument7 = lua_type ( L, 7 );

        // The first 6 are always numeric saying position, type and weapon/health/armor
        // TODO: Check argument 7 incase type is weapon
        if ( ( iArgument1 == LUA_TNUMBER || iArgument1 == LUA_TSTRING ) &&
            ( iArgument2 == LUA_TNUMBER || iArgument2 == LUA_TSTRING ) &&
            ( iArgument3 == LUA_TNUMBER || iArgument3 == LUA_TSTRING ) &&
            ( iArgument4 == LUA_TNUMBER || iArgument4 == LUA_TSTRING ) &&
            ( iArgument5 == LUA_TNUMBER || iArgument5 == LUA_TSTRING ) &&
            ( iArgument6 == LUA_TNUMBER || iArgument6 == LUA_TSTRING || iArgument6 == LUA_TNONE ) &&
            ( iArgument7 == LUA_TNUMBER || iArgument7 == LUA_TSTRING || iArgument7 == LUA_TNONE ) )
        {
            // Populate a position vector for it
            CVector vecPosition = CVector ( static_cast < float > ( lua_tonumber ( L, 1 ) ),
                static_cast < float > ( lua_tonumber ( L, 2 ) ),
                static_cast < float > ( lua_tonumber ( L, 3 ) ) );

            // Is the type health or armor?
            unsigned long ulRespawnInterval = 30000;
            double dblAmmo = 50.0;
            if ( iArgument6 == LUA_TNUMBER || iArgument6 == LUA_TSTRING )
                ulRespawnInterval = static_cast < unsigned long > ( lua_tonumber ( L, 6 ) );

            if ( iArgument7 == LUA_TNUMBER || iArgument7 == LUA_TSTRING )
                dblAmmo = lua_tonumber ( L, 7 );

            CLuaMain* pLuaMain = lua_readcontext( L );
            CResource* pResource = pLuaMain->GetResource ();
            CClientPickup* pPickup = CStaticFunctionDefinitions::CreatePickup( *pResource, vecPosition, (unsigned char)lua_tonumber ( L, 4 ), lua_tonumber ( L, 5 ), ulRespawnInterval, dblAmmo );
            if ( pPickup )
            {
                CElementGroup * pGroup = pResource->GetElementGroup();
                if ( pGroup )
                {
                    pGroup->Add ( ( CClientEntity* ) pPickup );
                }

                // Return the handle
                lua_pushelement ( L, pPickup );
                return 1;
            }
        }
        else
            m_pScriptDebugging->LogBadType( "createPickup" );

        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( getPickupType )
    {
        // Verify the argument
        if ( lua_type ( L, 1 ) == LUA_TLIGHTUSERDATA )
        {
            CClientPickup* pPickup = lua_topickup ( L, 1 );
            if ( pPickup )
            {
                unsigned char ucType = pPickup->m_ucType;
                lua_pushnumber ( L, ucType );
                return 1;
            }
            else
                m_pScriptDebugging->LogBadPointer( "getPickupType", "pickup", 1 );
        }
        else
            m_pScriptDebugging->LogBadType( "getPickupType" );

        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( getPickupWeapon )
    {
        // Verify the argument
        if ( lua_type ( L, 1 ) == LUA_TLIGHTUSERDATA )
        {
            CClientPickup* pPickup = lua_topickup ( L, 1 );
            if ( pPickup )
            {
                unsigned char ucWeapon = pPickup->m_ucWeaponType;
                lua_pushnumber ( L, static_cast < lua_Number > ( ucWeapon ) );
                return 1;
            }
            else
                m_pScriptDebugging->LogBadPointer( "getPickupWeapon", "pickup", 1 );
        }
        else
            m_pScriptDebugging->LogBadType( "getPickupWeapon" );

        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( getPickupAmount )
    {
        // Verify the argument
        if ( lua_type ( L, 1 ) == LUA_TLIGHTUSERDATA )
        {
            CClientPickup* pPickup = lua_topickup ( L, 1 );
            if ( pPickup )
            {
                float fAmount = pPickup->m_fAmount;
                lua_pushnumber ( L, fAmount );
                return 1;
            }
            else
                m_pScriptDebugging->LogBadPointer( "getPickupAmount", "pickup", 1 );
        }
        else
            m_pScriptDebugging->LogBadType( "getPickupAmount" );

        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( getPickupAmmo )
    {
        // Verify the argument
        if ( lua_type ( L, 1 ) == LUA_TLIGHTUSERDATA )
        {
            CClientPickup* pPickup = lua_topickup ( L, 1 );
            if ( pPickup )
            {
                unsigned short usAmmo = pPickup->m_usAmmo;
                lua_pushnumber ( L, static_cast < lua_Number > ( usAmmo ) );
                return 1;
            }
            else
                m_pScriptDebugging->LogBadPointer( "getPickupAmmo", "pickup", 1 );
        }
        else
            m_pScriptDebugging->LogBadType( "getPickupAmmo" );

        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( setPickupType )
    {
        // The first 2 are always numeric saying weapon/health/armor
        int iArgument1 = lua_type ( L, 1 );
        int iArgument2 = lua_type ( L, 2 );
        int iArgument3 = lua_type ( L, 3 );
        if ( ( iArgument1 == LUA_TLIGHTUSERDATA ) &&
            ( iArgument2 == LUA_TNUMBER || iArgument2 == LUA_TSTRING ) &&
            ( iArgument3 == LUA_TNUMBER || iArgument3 == LUA_TSTRING ) )
        {
            CClientEntity* pEntity = lua_toelement ( L, 1 );
            if ( pEntity )
            {
                // Do it
                if ( CStaticFunctionDefinitions::SetPickupType ( *pEntity, static_cast < unsigned char > ( lua_tonumber ( L, 2 ) ), lua_tonumber ( L, 3 ), lua_tonumber ( L, 4 ) ) )
                {
                    lua_pushboolean ( L, true );
                    return 1;
                }
            }
            else
                m_pScriptDebugging->LogBadPointer( "setPickupType", "element", 1 );
        }
        else
            m_pScriptDebugging->LogBadType( "setPickupType" );

        lua_pushboolean ( L, false );
        return 1;
    }
}