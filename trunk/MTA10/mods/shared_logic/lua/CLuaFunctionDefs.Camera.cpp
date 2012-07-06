/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.0
*               (Shared logic for modifications)
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        mods/shared_logic/lua/CLuaFunctionDefs.Camera.cpp
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
    LUA_DECLARE( getCameraViewMode )
    {
        unsigned short ucMode;
        if ( CStaticFunctionDefinitions::GetCameraViewMode ( ucMode ) )
        {
            lua_pushnumber ( L, ucMode );
            return 1;
        }

        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( getCameraMatrix )
    {
        CVector vecPosition, vecLookAt;
        float fRoll, fFOV;
        if ( CStaticFunctionDefinitions::GetCameraMatrix ( vecPosition, vecLookAt, fRoll, fFOV ) )
        {
            lua_pushnumber ( L, vecPosition.fX );
            lua_pushnumber ( L, vecPosition.fY );
            lua_pushnumber ( L, vecPosition.fZ );
            lua_pushnumber ( L, vecLookAt.fX );
            lua_pushnumber ( L, vecLookAt.fY );
            lua_pushnumber ( L, vecLookAt.fZ );
            lua_pushnumber ( L, fRoll );
            lua_pushnumber ( L, fFOV );
            return 8;
        }

        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( getCameraTarget )
    {
        CClientEntity* pTarget = CStaticFunctionDefinitions::GetCameraTarget ();
        if ( pTarget )
        {
            lua_pushelement ( L, pTarget );
            return 1;
        }

        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( getCameraInterior )
    {
        unsigned char ucInterior;
        if ( CStaticFunctionDefinitions::GetCameraInterior ( ucInterior ) )
        {
            lua_pushnumber ( L, ucInterior );
            return 1;
        }

        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( getCameraGoggleEffect )
    {
        bool bNightVision   = g_pMultiplayer->IsNightVisionEnabled   ( );
        bool bThermalVision = g_pMultiplayer->IsThermalVisionEnabled ( );

        if ( bNightVision )
            lua_pushstring ( L, "nightvision" );
        else if ( bThermalVision )
            lua_pushstring ( L, "thermalvision" );
        else
            lua_pushstring ( L, "normal" );

        return 1;
    }

    LUA_DECLARE( setCameraMatrix )
    {
        // Verify the parameter types
        int iArgument1 = lua_type ( L, 1 );
        int iArgument2 = lua_type ( L, 2 );
        int iArgument3 = lua_type ( L, 3 );

        if ( ( iArgument1 == LUA_TNUMBER || iArgument1 == LUA_TSTRING ) &&
             ( iArgument2 == LUA_TNUMBER || iArgument2 == LUA_TSTRING ) &&
             ( iArgument3 == LUA_TNUMBER || iArgument3 == LUA_TSTRING ) )
        {
            // Grab the parameters
            CVector vecPosition;
            vecPosition.fX = static_cast < float > ( lua_tonumber ( L, 1 ) );
            vecPosition.fY = static_cast < float > ( lua_tonumber ( L, 2 ) );
            vecPosition.fZ = static_cast < float > ( lua_tonumber ( L, 3 ) );

            CVector vecLookAt;
            CVector * pvecLookAt = NULL;
            float fRoll = 0.0f;
            float fFOV = 70.0f;
            int iArgument4 = lua_type ( L, 4 );
            int iArgument5 = lua_type ( L, 5 );
            int iArgument6 = lua_type ( L, 6 );
            if ( ( iArgument4 == LUA_TNUMBER || iArgument4 == LUA_TSTRING ) &&
                 ( iArgument5 == LUA_TNUMBER || iArgument5 == LUA_TSTRING ) &&
                 ( iArgument6 == LUA_TNUMBER || iArgument6 == LUA_TSTRING ) )
            {
                vecLookAt.fX = static_cast < float > ( lua_tonumber ( L, 4 ) );
                vecLookAt.fY = static_cast < float > ( lua_tonumber ( L, 5 ) );
                vecLookAt.fZ = static_cast < float > ( lua_tonumber ( L, 6 ) );
                pvecLookAt = &vecLookAt;

                int iArgument7 = lua_type ( L, 7 );
                
                if ( iArgument7 == LUA_TNUMBER || iArgument7 == LUA_TSTRING )
                {
                    fRoll = static_cast < float > ( lua_tonumber ( L, 7 ) );
                    int iArgument8 = lua_type ( L, 8 );
                    if ( iArgument8 == LUA_TNUMBER || iArgument8 == LUA_TSTRING )
                    {
                        fFOV = static_cast < float > ( lua_tonumber ( L, 8 ) );
                        if ( fFOV <= 0.0f || fFOV >= 180.0f )
                            fFOV = 70.0f;
                    }
                }
            }

            if ( CStaticFunctionDefinitions::SetCameraMatrix ( vecPosition, pvecLookAt, fRoll, fFOV ) )
            {
                lua_pushboolean ( L, true );
                return 1;
            }
        }
        else
            m_pScriptDebugging->LogBadType ( L, "setCameraMatrix" );

        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( setCameraTarget )
    {
        if ( lua_type ( L, 1 ) == LUA_TLIGHTUSERDATA )
        {
            // Grab the first argument
            CClientEntity * pTarget = lua_toelement ( L, 1 );
            if ( pTarget )    
            {
                if ( CStaticFunctionDefinitions::SetCameraTarget ( pTarget ) )
                {
                    lua_pushboolean ( L, true );
                    return 1;
                }
            }
            else
                m_pScriptDebugging->LogBadPointer ( L, "setCameraTarget", "target", 1 );
        }
        else
            m_pScriptDebugging->LogBadType ( L, "setCameraTarget" );

        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( setCameraInterior )
    {
        int iArgument1 = lua_type ( L, 1 );
        if ( iArgument1 == LUA_TNUMBER || iArgument1 == LUA_TSTRING )
        {
            unsigned char ucInterior = static_cast < unsigned char > ( lua_tonumber ( L, 1 ) );

            if ( CStaticFunctionDefinitions::SetCameraInterior ( ucInterior ) )
            {
                lua_pushboolean ( L, true );
                return 1;
            }
        }
        else
            m_pScriptDebugging->LogBadType ( L, "setCameraInterior" );

        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( fadeCamera )
    {
        int iArgument1 = lua_type ( L, 1 ); // fadeIn
        int iArgument2 = lua_type ( L, 2 ); // fadeTime
        int iArgument3 = lua_type ( L, 3 ); // red
        int iArgument4 = lua_type ( L, 4 ); // green
        int iArgument5 = lua_type ( L, 5 ); // blue

        if ( iArgument1 == LUA_TBOOLEAN )
        {
            bool bFadeIn = lua_toboolean (L, 1 ) ? true:false;
            unsigned char ucRed = 0;
            unsigned char ucGreen = 0;
            unsigned char ucBlue = 0;
            float fFadeTime = 1.0f;

            if ( iArgument2 == LUA_TSTRING || iArgument2 == LUA_TNUMBER )
            {
                fFadeTime = static_cast < unsigned char > ( lua_tonumber ( L, 2 ) );

                if ( iArgument3 == LUA_TSTRING || iArgument3 == LUA_TNUMBER )
                {
                    ucRed = static_cast < unsigned char > ( lua_tonumber ( L, 3 ) );

                    if ( iArgument4 == LUA_TSTRING || iArgument4 == LUA_TNUMBER )
                    {
                        ucGreen = static_cast < unsigned char > ( lua_tonumber ( L, 4 ) );

                        if ( iArgument5 == LUA_TSTRING || iArgument5 == LUA_TNUMBER )
                        {
                            ucBlue = static_cast < unsigned char > ( lua_tonumber ( L, 5 ) );
                        }
                    }
                }
            }

            if ( CStaticFunctionDefinitions::FadeCamera ( bFadeIn, fFadeTime, ucRed, ucGreen, ucBlue ) )
            {
                lua_pushboolean ( L, true );
                return 1;
            }
        }
        else
            m_pScriptDebugging->LogBadType ( L, "fadeCamera" );

        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( setCameraClip )
    {
        bool bObjects   = lua_type ( L, 1 ) != LUA_TBOOLEAN ? true : lua_toboolean ( L, 1 ) ? true : false;
        bool bVehicles  = lua_type ( L, 2 ) != LUA_TBOOLEAN ? true : lua_toboolean ( L, 2 ) ? true : false;

        m_pManager->GetCamera ()->SetCameraClip ( bObjects, bVehicles );

        lua_pushboolean ( L, true );
        return 1;
    }

    LUA_DECLARE( setCameraViewMode )
    {
        if ( lua_type ( L, 1 ) == LUA_TNUMBER )
        {
            CStaticFunctionDefinitions::SetCameraViewMode ( static_cast <unsigned short> ( lua_tonumber ( L, 1 ) ) );

            lua_pushboolean ( L, true );
            return 1;
        }

        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( setCameraGoggleEffect )
    {
        if ( lua_type ( L, 1 ) == LUA_TSTRING )
        {
            bool    bSuccess = false;
            SString strMode  = lua_tostring ( L, 1 );

            if ( strMode.compare ( "nightvision" ) == 0 )
            {
                g_pMultiplayer->SetNightVisionEnabled   ( true );
                g_pMultiplayer->SetThermalVisionEnabled ( false );
                
                bSuccess = true;
            }
            else if ( strMode.compare ( "thermalvision" ) == 0 )
            {
                g_pMultiplayer->SetNightVisionEnabled   ( false );
                g_pMultiplayer->SetThermalVisionEnabled ( true );

                bSuccess = true;
            }
            else if ( strMode.compare ( "normal" ) == 0 )
            {
                g_pMultiplayer->SetNightVisionEnabled   ( false );
                g_pMultiplayer->SetThermalVisionEnabled ( false );

                bSuccess = true;
            }

            if ( bSuccess )
            {
                lua_pushboolean ( L, true );
                return 1;
            }
        }

        lua_pushboolean ( L, false );
        return 1;
    }
}