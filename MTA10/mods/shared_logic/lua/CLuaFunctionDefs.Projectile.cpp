/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.0
*               (Shared logic for modifications)
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        mods/shared_logic/lua/CLuaFunctionDefs.Projectile.cpp
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
    LUA_DECLARE( createProjectile )
    {
        if ( ( lua_istype ( L, 1, LUA_TLIGHTUSERDATA ) ) &&
            ( lua_istype ( L, 2, LUA_TNUMBER ) || lua_istype ( L, 2, LUA_TSTRING ) ) )
        {
            CClientEntity* pCreator = lua_toelement ( L, 1 );
            if ( pCreator )
            {
                unsigned char ucWeaponType = static_cast < unsigned char > ( lua_tonumber ( L, 2 ) );
                CVector vecOrigin;
                pCreator->GetPosition ( vecOrigin );
                float fForce = 1.0f;
                CClientEntity* pTarget = NULL;
                CVector *pvecRotation = NULL, *pvecMoveSpeed = NULL;
                unsigned short usModel = 0;
                if ( ( lua_istype ( L, 3, LUA_TNUMBER ) || lua_istype ( L, 3, LUA_TSTRING ) ) &&
                    ( lua_istype ( L, 4, LUA_TNUMBER ) || lua_istype ( L, 4, LUA_TSTRING ) ) &&
                    ( lua_istype ( L, 5, LUA_TNUMBER ) || lua_istype ( L, 5, LUA_TSTRING ) ) )
                {
                    vecOrigin = CVector ( static_cast < float > ( lua_tonumber ( L, 3 ) ),
                        static_cast < float > ( lua_tonumber ( L, 4 ) ),
                        static_cast < float > ( lua_tonumber ( L, 5 ) ) );

                    if ( lua_istype ( L, 6, LUA_TNUMBER ) || lua_istype ( L, 6, LUA_TSTRING ) )
                    {
                        fForce = static_cast < float > ( lua_tonumber ( L, 6 ) );

                        if ( lua_istype ( L, 7, LUA_TLIGHTUSERDATA ) )
                        {
                            CClientEntity* pTemp = lua_toelement ( L, 7 );
                            if ( pTemp )
                            {
                                pTarget = pTemp;
                            }
                            else
                                m_pScriptDebugging->LogBadPointer( "createProjectile", "element", 7 );
                        }

                        int iArgument8 = lua_type ( L, 8 );
                        int iArgument9 = lua_type ( L, 9 );
                        int iArgument10 = lua_type ( L, 10 );
                        if ( ( iArgument8 == LUA_TSTRING || iArgument8 == LUA_TNUMBER ) &&
                            ( iArgument9 == LUA_TSTRING || iArgument9 == LUA_TNUMBER ) &&
                            ( iArgument10 == LUA_TSTRING || iArgument10 == LUA_TNUMBER ) )
                        {
                            pvecRotation = new CVector ( static_cast < float > ( lua_tonumber ( L, 8 ) ),
                                static_cast < float > ( lua_tonumber ( L, 9 ) ),
                                static_cast < float > ( lua_tonumber ( L, 10 ) ) );
                        }
                        int iArgument11 = lua_type ( L, 11 );
                        int iArgument12 = lua_type ( L, 12 );
                        int iArgument13 = lua_type ( L, 13 );
                        if ( ( iArgument11 == LUA_TSTRING || iArgument11 == LUA_TNUMBER ) &&
                            ( iArgument12 == LUA_TSTRING || iArgument12 == LUA_TNUMBER ) &&
                            ( iArgument13 == LUA_TSTRING || iArgument13 == LUA_TNUMBER ) )
                        {
                            pvecMoveSpeed = new CVector ( static_cast < float > ( lua_tonumber ( L, 11 ) ),
                                static_cast < float > ( lua_tonumber ( L, 12 ) ),
                                static_cast < float > ( lua_tonumber ( L, 13 ) ) );

                            int iArgument14 = lua_type ( L, 14 );
                            if ( iArgument14 == LUA_TSTRING || iArgument14 == LUA_TNUMBER )
                            {
                                usModel = static_cast < unsigned short > ( lua_tonumber ( L, 14 ) );
                            }
                        }
                    }
                }

                CLuaMain* pLuaMain = lua_readcontext( L );
                CResource * pResource = pLuaMain->GetResource();
                CClientProjectile * pProjectile = CStaticFunctionDefinitions::CreateProjectile ( *pResource, *pCreator, ucWeaponType, vecOrigin, fForce, pTarget, pvecRotation, pvecMoveSpeed, usModel );
                if ( pProjectile )
                {
                    CElementGroup * pGroup = pResource->GetElementGroup();
                    if ( pGroup )
                    {
                        pGroup->Add ( ( CClientEntity* ) pProjectile );
                    }

                    if ( pvecRotation )
                    {
                        delete pvecRotation;
                        pvecRotation = NULL;
                    }
                    if ( pvecMoveSpeed )
                    {
                        delete pvecMoveSpeed;
                        pvecMoveSpeed = NULL;
                    }

                    lua_pushelement ( L, pProjectile );
                    return 1;
                }
                if ( pvecRotation )
                    delete pvecRotation;

                if ( pvecMoveSpeed )
                    delete pvecMoveSpeed;
            }
            else
                m_pScriptDebugging->LogBadPointer( "createProjectile", "element", 1 );
        }
        else
            m_pScriptDebugging->LogBadType( "createProjectile" );

        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( getProjectileType )
    {
        // Verify the argument
        if ( lua_type ( L, 1 ) == LUA_TLIGHTUSERDATA )
        {
            CClientProjectile* pProjectile = lua_toprojectile ( L, 1 );
            if ( pProjectile )
            {
                unsigned char ucWeapon = pProjectile->GetWeaponType();
                lua_pushnumber ( L, static_cast < lua_Number > ( ucWeapon ) );
                return 1;
            }
            else
                m_pScriptDebugging->LogBadPointer( "getProjectileType", "projectile", 1 );
        }
        else
            m_pScriptDebugging->LogBadType( "getProjectileType" );

        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( getProjectileTarget )
    {
        // Verify the argument
        if ( lua_type ( L, 1 ) == LUA_TLIGHTUSERDATA )
        {
            CClientProjectile* pProjectile = lua_toprojectile ( L, 1 );
            if ( pProjectile )
            {
                unsigned char ucWeapon = pProjectile->GetWeaponType();
			    if (ucWeapon == WEAPONTYPE_ROCKET_HS)
			    {
				    lua_pushelement ( L, pProjectile->GetTargetEntity() );
                    return 1;
			    }
            }
            else
                m_pScriptDebugging->LogBadPointer( "getProjectileTarget", "projectile", 1 );
        }
        else
            m_pScriptDebugging->LogBadType( "getProjectileTarget" );

        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( getProjectileCreator )
    {
        // Verify the argument
        if ( lua_type ( L, 1 ) == LUA_TLIGHTUSERDATA )
        {
            CClientProjectile* pProjectile = lua_toprojectile ( L, 1 );
            if ( pProjectile )
            {
			    lua_pushelement ( L, pProjectile->GetCreator() );
                return 1;
            }
            else
                m_pScriptDebugging->LogBadPointer( "getProjectileCreator", "projectile", 1 );
        }
        else
            m_pScriptDebugging->LogBadType( "getProjectileCreator" );

        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( getProjectileForce )
    {
        // Verify the argument
        if ( lua_type ( L, 1 ) == LUA_TLIGHTUSERDATA )
        {
            CClientProjectile* pProjectile = lua_toprojectile ( L, 1 );
            if ( pProjectile )
            {
			    lua_pushnumber ( L, static_cast < lua_Number > ( pProjectile->GetForce() ) );
                return 1;
            }
            else
                m_pScriptDebugging->LogBadPointer( "getProjectileForce", "projectile", 1 );
        }
        else
            m_pScriptDebugging->LogBadType( "getProjectileForce" );

        lua_pushboolean ( L, false );
        return 1;
    }
}