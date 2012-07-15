/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.0
*               (Shared logic for modifications)
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        mods/shared_logic/lua/CLuaFunctionDefs.Vehicle.cpp
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
    LUA_DECLARE( getVehicleType )
    {
        unsigned long ucModel = 0;
        if ( lua_type ( L, 1 ) == LUA_TLIGHTUSERDATA )
        {
            CClientVehicle* pVehicle = lua_tovehicle ( L, 1 );
            if ( pVehicle )
                ucModel = pVehicle -> GetModel ( );
        }
        else if ( lua_type ( L, 1 ) == LUA_TNUMBER )
        {
            ucModel = static_cast < unsigned long > (lua_tonumber ( L, 1 ));
        }
        else
            m_pScriptDebugging->LogBadType( "getVehicleType" );

        if ( ucModel >= 400 && ucModel < 610 )
            lua_pushstring ( L, CVehicleNames::GetVehicleTypeName ( ucModel ) );
        else
            lua_pushboolean ( L, false );

        return 1;
    }

    LUA_DECLARE( isVehicleTaxiLightOn )
    {
        if ( lua_type ( L, 1 ) == LUA_TLIGHTUSERDATA )
        {
            CClientVehicle* pVehicle = lua_tovehicle ( L, 1 );
            if ( pVehicle )
            {
                if ( pVehicle->GetModel() == 438 || pVehicle->GetModel() == 420 )
                {
                    bool bLightState = pVehicle->IsTaxiLightOn ();
                    if ( bLightState )
                    {
                        lua_pushboolean ( L, bLightState );
                        return 1;
                    }
                }
            }
            else
                m_pScriptDebugging->LogBadPointer( "isVehicleTaxiLightOn", "vehicle", 1 );
        }
        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( setVehicleTaxiLightOn )
    {
        if ( lua_type ( L, 1 ) == LUA_TLIGHTUSERDATA && lua_type ( L, 2 ) == LUA_TBOOLEAN )
        {
            CClientVehicle* pVehicle = lua_tovehicle ( L, 1 );
            if ( pVehicle )
            {
                if ( pVehicle->GetModel() == 438 || pVehicle->GetModel() == 420 )
                {
                    bool bLightState = ( lua_toboolean ( L, 2 ) ? true : false );
                    pVehicle->SetTaxiLightOn ( bLightState );
                    lua_pushboolean ( L, true );
                    return 1;
                }
            }
            else
                m_pScriptDebugging->LogBadPointer( "setVehicleTaxiLightOn", "vehicle", 1 );
        }
        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( getVehicleColor )
    {
        if ( lua_type ( L, 1 ) == LUA_TLIGHTUSERDATA )
        {
            bool bRGB = false;
            if ( lua_type ( L, 2 ) == LUA_TBOOLEAN )
                bRGB = lua_toboolean ( L, 2 ) ? true : false;

            CClientVehicle* pVehicle = lua_tovehicle ( L, 1 );
            if ( pVehicle ) 
            {
                CVehicleColor& color = pVehicle->GetColor ();

                if ( bRGB )
                {
                    for ( uint i = 0 ; i < 4 ; i++ )
                    {
                        SColor RGBColor = color.GetRGBColor ( i );
                        lua_pushnumber ( L, RGBColor.R );
                        lua_pushnumber ( L, RGBColor.G );
                        lua_pushnumber ( L, RGBColor.B );
                    }
                    return 12;
                }
                else
                {
                    lua_pushnumber ( L, color.GetPaletteColor ( 0 ) );
                    lua_pushnumber ( L, color.GetPaletteColor ( 1 ) );
                    lua_pushnumber ( L, color.GetPaletteColor ( 2 ) );
                    lua_pushnumber ( L, color.GetPaletteColor ( 3 ) );
                    return 4;
                }
            }
            else
                m_pScriptDebugging->LogBadPointer( "getVehicleColor", "vehicle", 1 );
        }
        else
            m_pScriptDebugging->LogBadType( "getVehicleColor" );

        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( getVehicleModelFromName )
    {
        if ( lua_type ( L, 1 ) == LUA_TSTRING )
        {
            const char* szName = lua_tostring ( L, 1 );
            unsigned short usModel;
            if ( CStaticFunctionDefinitions::GetVehicleModelFromName ( szName, usModel ) )
            {
                lua_pushnumber ( L, static_cast < lua_Number > ( usModel ) );
                return 1;
            }
        }
        else
            m_pScriptDebugging->LogBadType( "getVehicleModelFromName" );

        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( getVehicleLandingGearDown )
    {
        // Verify the first argument
        int iArgument1 = lua_type ( L, 1 );
        if ( iArgument1 == LUA_TLIGHTUSERDATA )
        {
            // Grab the vehicle. Is it valid?
            CClientVehicle* pVehicle = lua_tovehicle ( L, 1 );
            if ( pVehicle )
            {
                // Does the vehicle have landing gears?
                if ( CClientVehicleManager::HasLandingGears ( pVehicle->GetModel () ) )
                {
                    // Return whether it has landing gears or not
                    bool bLandingGear = pVehicle->IsLandingGearDown ();
                    lua_pushboolean ( L, bLandingGear );
                    return 1;
                }
            }
            else
                m_pScriptDebugging->LogBadPointer( "getVehicleLandingGear", "vehicle", 1 );
        }
        else
            m_pScriptDebugging->LogBadType( "getVehicleLandingGearDown" );

        lua_pushnil ( L );
        return 1;
    }

    LUA_DECLARE( getVehicleMaxPassengers )
    {
        if ( lua_type ( L, 1 ) == LUA_TLIGHTUSERDATA || lua_type ( L, 1 ) == LUA_TNUMBER )
        {
            unsigned short model = 0;

            if ( lua_type ( L, 1 ) == LUA_TLIGHTUSERDATA)
            {
                CClientVehicle* pVehicle = lua_tovehicle ( L, 1 );
                if ( pVehicle )
                    model = pVehicle->GetModel();
                else
                {
                    m_pScriptDebugging->LogBadPointer( "getVehicleMaxPassengers", "vehicle", 1 );
                    lua_pushboolean ( L, false );
                    return 1;
                }
            }
            else
            {
                model = (unsigned short) lua_tonumber ( L, 1 );

                if (!CClientVehicleManager::IsValidModel(model))
                {
                    m_pScriptDebugging->LogBadType( "getVehicleMaxPassengers" );
                    lua_pushboolean ( L, false );
                    return 1;
                }
            }
            
            unsigned int uiMaxPassengers = CClientVehicleManager::GetMaxPassengerCount ( model );
            if ( uiMaxPassengers != 0xFF )
            {
                lua_pushnumber ( L, uiMaxPassengers );
                return 1;
            }
        }
        else
            m_pScriptDebugging->LogBadType( "getVehicleMaxPassengers" );

        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( getVehicleOccupant )
    {
        // Grab the seat argument if exists
        unsigned int uiSeat = 0;
        int iArgument2 = lua_type ( L, 2 );
        if ( iArgument2 == LUA_TNUMBER || iArgument2 == LUA_TSTRING )
        {
            uiSeat = static_cast < unsigned int > ( lua_tonumber ( L, 2 ) );
        }

        if ( lua_type ( L, 1 ) == LUA_TLIGHTUSERDATA )
        {
            CClientVehicle* pVehicle = lua_tovehicle ( L, 1 );
            if ( pVehicle )
            {
                CClientPed* pPed = pVehicle->GetOccupant ( uiSeat );
                if ( pPed )
                {
                    lua_pushelement ( L, pPed );
                    return 1;
                }
            }
            else
                m_pScriptDebugging->LogBadPointer( "getVehicleOccupant", "vehicle", 1 );
        }
        else
            m_pScriptDebugging->LogBadType( "getVehicleOccupant" );

        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( getVehicleOccupants )
    {
        if ( lua_type ( L, 1 ) == LUA_TLIGHTUSERDATA )
        {
            CClientVehicle* pVehicle = lua_tovehicle ( L, 1 );
            if ( pVehicle )
            {
                // Create a new table
                lua_newtable ( L );

                // Get the maximum amount of passengers
                unsigned char ucMaxPassengers = CClientVehicleManager::GetMaxPassengerCount ( pVehicle->GetModel ( ) );

                // Make sure that if the vehicle doesn't have any seats, the function returns false
                if ( ucMaxPassengers == 255 )
                {
                    lua_pushboolean ( L, false );
                    return 1;
                }

                // Add All Occupants
                for ( unsigned char ucSeat = 0; ucSeat <= ucMaxPassengers; ++ ucSeat )
                {
                    CClientPed* pPed = pVehicle->GetOccupant ( ucSeat );
                    if ( pPed )
                    {
                        lua_pushnumber ( L, ucSeat );
                        lua_pushelement ( L, pPed );
                        lua_settable ( L, -3 );
                    }
                }
                return 1;
            }
            else
                m_pScriptDebugging->LogBadPointer( "getVehicleOccupants", "vehicle", 1 );
        }
        else
            m_pScriptDebugging->LogBadType( "getVehicleOccupants" );
                
        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( getVehicleController )
    {
        if ( lua_type ( L, 1 ) == LUA_TLIGHTUSERDATA )
        {
            CClientVehicle* pVehicle = lua_tovehicle ( L, 1 );
            if ( pVehicle )
            {
                CClientPed* pPed = pVehicle->GetControllingPlayer ();
                if ( pPed )
                {
                    lua_pushelement ( L, pPed );
                    return 1;
                }
            }
            else
                m_pScriptDebugging->LogBadPointer( "getVehicleController", "vehicle", 1 );
        }
        else
            m_pScriptDebugging->LogBadType( "getVehicleController" );

        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( getVehicleSirensOn )
    {
        // Verify the argument type
        int iArgument1 = lua_type ( L, 1 );
        if ( iArgument1 == LUA_TLIGHTUSERDATA )
        {
            // Grab the vehicle pointer. Is it valid?
            CClientVehicle* pVehicle = lua_tovehicle ( L, 1 );
            if ( pVehicle )
            {
                // Does the vehicle have Sirens?
                if ( CClientVehicleManager::HasSirens ( pVehicle->GetModel () ) )
                {
                    // Return whether it has its Sirens on or not
                    bool bSirensOn = pVehicle->IsSirenOrAlarmActive ();
                    lua_pushboolean ( L, bSirensOn );
                    return 1;
                }
            }
            else
                m_pScriptDebugging->LogBadPointer( "getVehicleSirensOn", "vehicle", 1 );
        }
        else
            m_pScriptDebugging->LogBadType( "getVehicleSirensOn" );

        lua_pushnil ( L );
        return 1;
    }

    LUA_DECLARE( getVehicleTurnVelocity )
    {
        if ( lua_type ( L, 1 ) == LUA_TLIGHTUSERDATA )
        {
            CClientVehicle* pVehicle = lua_tovehicle ( L, 1 );
            if ( pVehicle )
            {
                CVector vecTurnVelocity;
                pVehicle->GetTurnSpeed ( vecTurnVelocity );

                lua_pushnumber ( L, vecTurnVelocity.fX );
                lua_pushnumber ( L, vecTurnVelocity.fY );
                lua_pushnumber ( L, vecTurnVelocity.fZ );
                return 3;
            }
            else
                m_pScriptDebugging->LogBadPointer( "getVehicleTurnVelocity", "vehicle", 1 );
        }
        else
            m_pScriptDebugging->LogBadType( "getVehicleTurnVelocity" );

        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( getVehicleTurretPosition )
    {
        if ( lua_type ( L, 1 ) == LUA_TLIGHTUSERDATA )
        {
            CClientVehicle* pVehicle = lua_tovehicle ( L, 1 );
            if ( pVehicle )
            {
                CVector2D vecPosition;
                pVehicle->GetTurretRotation ( vecPosition.fX, vecPosition.fY );

                lua_pushnumber ( L, vecPosition.fX );
                lua_pushnumber ( L, vecPosition.fY );
                return 2;
            }
            else
                m_pScriptDebugging->LogBadPointer( "getVehicleTurretPosition", "vehicle", 1 );
        }
        else
            m_pScriptDebugging->LogBadType( "getVehicleTurretPosition" );

        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( isVehicleLocked )
    {
        if ( lua_type ( L, 1 ) == LUA_TLIGHTUSERDATA )
        {
            CClientVehicle* pVehicle = lua_tovehicle ( L, 1 );
            if ( pVehicle )
            {
                bool bLocked = pVehicle->AreDoorsLocked ();
                lua_pushboolean ( L, bLocked );
                return 1;
            }
            else
                m_pScriptDebugging->LogBadPointer( "isVehicleLocked", "vehicle", 1 );
        }
        else
            m_pScriptDebugging->LogBadType( "isVehicleLocked" );

        lua_pushnil ( L );
        return 1;
    }

    LUA_DECLARE( getVehicleUpgradeOnSlot )
    {
        int iArgument1 = lua_type ( L, 1 );
        int iArgument2 = lua_type ( L, 2 );
        if ( iArgument1 == LUA_TLIGHTUSERDATA &&
            ( iArgument2 == LUA_TNUMBER || iArgument2 == LUA_TSTRING ) )
        {
            CClientVehicle* pVehicle = lua_tovehicle ( L, 1 );
            if ( pVehicle )
            {           
                unsigned char ucSlot = static_cast < unsigned char > ( lua_tonumber ( L, 2 ) );
                CVehicleUpgrades* pUpgrades = pVehicle->GetUpgrades ();
                if ( pUpgrades )
                {
                    unsigned short usUpgrade = pUpgrades->GetSlotState ( ucSlot );
                    lua_pushnumber ( L, usUpgrade );
                    return 1;
                }
            }
            else
                m_pScriptDebugging->LogBadPointer( "getVehicleUpgradeOnSlot", "vehicle", 1 );
        }
        else
            m_pScriptDebugging->LogBadType( "getVehicleUpgradeOnSlot" );

        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( getVehicleUpgrades )
    {
        int iArgument1 = lua_type ( L, 1 );
        if ( iArgument1 == LUA_TLIGHTUSERDATA )
        {
            CClientVehicle* pVehicle = lua_tovehicle ( L, 1 );
            // If the vehicle is valid
            if ( pVehicle )
            {           
                CVehicleUpgrades* pUpgrades = pVehicle->GetUpgrades ();
                if ( pUpgrades )
                {
                    // Create a new table
                    lua_newtable ( L );

                    // Add all the upgrades to the table
                    unsigned short* usSlotStates = pUpgrades->GetSlotStates ();

                    unsigned int uiIndex = 0;
                    unsigned char ucSlot = 0;
                    for ( ; ucSlot < VEHICLE_UPGRADE_SLOTS ; ucSlot++ )
                    {
                        if ( usSlotStates [ucSlot] != 0 )
                        {
                            lua_pushnumber ( L, ++uiIndex );
                            lua_pushnumber ( L, usSlotStates [ ucSlot ] );
                            lua_settable ( L, -3 );
                        }
                    }

                    return 1;
                }
            }
            else
                m_pScriptDebugging->LogBadPointer( "getVehicleUpgrades", "vehicle", 1 );
        }
        else
            m_pScriptDebugging->LogBadType( "getVehicleUpgrades" );

        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( getVehicleUpgradeSlotName )
    {
        int iArgument1 = lua_type ( L, 1 );
        if ( iArgument1 == LUA_TNUMBER || iArgument1 == LUA_TSTRING )
        {
            unsigned long ulNumber = static_cast < unsigned long > ( lua_tonumber ( L, 1 ) );

            if ( ulNumber < 17 )
            {
                char szUpgradeName [32];
                if ( CStaticFunctionDefinitions::GetVehicleUpgradeSlotName ( static_cast < unsigned char > ( ulNumber ), szUpgradeName, sizeof(szUpgradeName) ) )
                {
                    lua_pushstring ( L, szUpgradeName );
                    return 1;
                }
            }
            else if ( ulNumber >= 1000 && ulNumber <= 1193 )
            {
                char szUpgradeName [32];
                if ( CStaticFunctionDefinitions::GetVehicleUpgradeSlotName ( static_cast < unsigned short > ( ulNumber ), szUpgradeName, sizeof(szUpgradeName) ) )
                {
                    lua_pushstring ( L, szUpgradeName );
                    return 1;
                }
            }
            else
                m_pScriptDebugging->LogBadPointer( "getVehicleUpgradeSlotName", "slot/upgrade", 1 );
        }
        else
            m_pScriptDebugging->LogBadType( "getVehicleUpgradeSlotName" );

        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( getVehicleCompatibleUpgrades )
    {
        if ( lua_type ( L, 1 ) == LUA_TLIGHTUSERDATA )
        {
            CClientVehicle* pVehicle = lua_tovehicle ( L, 1 );
            unsigned char ucSlot = 0xFF;
            int iArgument2 = lua_type ( L, 2 );
            if ( iArgument2 == LUA_TNUMBER || iArgument2 == LUA_TSTRING )
                ucSlot = static_cast < unsigned char > ( lua_tonumber ( L, 2 ) );

            if ( pVehicle )
            {
                CVehicleUpgrades* pUpgrades = pVehicle->GetUpgrades ();
                if ( pUpgrades )
                {
                    // Create a new table
                    lua_newtable ( L );

                    unsigned int uiIndex = 0;
                    for ( unsigned short usUpgrade = 1000 ; usUpgrade <= 1193 ; usUpgrade++ )
                    {
                        if ( pUpgrades->IsUpgradeCompatible ( usUpgrade ) )
                        {
                            if ( ucSlot != 0xFF )
                            {
                                unsigned char ucUpgradeSlot;
                                if ( !pUpgrades->GetSlotFromUpgrade ( usUpgrade, ucUpgradeSlot ) )
                                    continue;

                                if ( ucUpgradeSlot != ucSlot )
                                    continue;
                            }

                            // Add this one to a list                        
                            lua_pushnumber ( L, ++uiIndex );
                            lua_pushnumber ( L, usUpgrade );
                            lua_settable ( L, -3 );
                        }
                    }
                    return 1;
                }
            }
            else
                m_pScriptDebugging->LogBadPointer( "getVehicleCompatibleUpgrades", "vehicle", 1 );
        }
        else
            m_pScriptDebugging->LogBadType( "getVehicleCompatibleUpgrades" );

        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( getVehicleWheelStates )
    {
        int iArgument1 = lua_type ( L, 1 );
        if ( iArgument1 == LUA_TLIGHTUSERDATA )
        {
            CClientVehicle* pVehicle = lua_tovehicle ( L, 1 );
            if ( pVehicle )
            {
                unsigned char ucFrontLeft = pVehicle->GetWheelStatus ( FRONT_LEFT_WHEEL );
                unsigned char ucRearLeft = pVehicle->GetWheelStatus ( REAR_LEFT_WHEEL );
                unsigned char ucFrontRight = pVehicle->GetWheelStatus ( FRONT_RIGHT_WHEEL );
                unsigned char ucRearRight = pVehicle->GetWheelStatus ( REAR_RIGHT_WHEEL );

                lua_pushnumber ( L, ucFrontLeft );
                lua_pushnumber ( L, ucRearLeft );
                lua_pushnumber ( L, ucFrontRight );
                lua_pushnumber ( L, ucRearRight );
                return 4;
            }
            else
                m_pScriptDebugging->LogBadPointer( "getVehicleWheelStates", "vehicle", 1 );
        }
        else
            m_pScriptDebugging->LogBadType( "getVehicleWheelStates" );

        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( getVehicleDoorState )
    {
        int iArgument1 = lua_type ( L, 1 );
        int iArgument2 = lua_type ( L, 2 );
        if ( iArgument1 == LUA_TLIGHTUSERDATA &&
            ( iArgument2 == LUA_TNUMBER || iArgument2 == LUA_TSTRING ) )
        {
            CClientVehicle* pVehicle = lua_tovehicle ( L, 1 );
            if ( pVehicle )
            {
                unsigned char ucDoor = static_cast < unsigned char > ( lua_tonumber ( L, 2 ) );
                unsigned char ucState = pVehicle->GetDoorStatus ( ucDoor );
                lua_pushnumber ( L, ucState );
                return 1;
            }
            else
                m_pScriptDebugging->LogBadPointer( "getVehicleDoorState", "vehicle", 1 );
        }
        else
            m_pScriptDebugging->LogBadType( "getVehicleDoorState" );

        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( getVehicleLightState )
    {
        int iArgument1 = lua_type ( L, 1 );
        int iArgument2 = lua_type ( L, 2 );
        if ( iArgument1 == LUA_TLIGHTUSERDATA &&
            ( iArgument2 == LUA_TNUMBER || iArgument2 == LUA_TSTRING ) )
        {
            CClientVehicle* pVehicle = lua_tovehicle ( L, 1 );
            if ( pVehicle )
            {
                unsigned char ucLight = static_cast < unsigned char > ( lua_tonumber ( L, 2 ) );
                unsigned char ucState = pVehicle->GetLightStatus ( ucLight );
                lua_pushnumber ( L, ucState );
                return 1;
            }
            else
                m_pScriptDebugging->LogBadPointer( "getVehicleLightState", "vehicle", 1 );
        }
        else
            m_pScriptDebugging->LogBadType( "getVehicleLightState" );

        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( getVehiclePanelState )
    {
        int iArgument1 = lua_type ( L, 1 );
        int iArgument2 = lua_type ( L, 2 );
        if ( iArgument1 == LUA_TLIGHTUSERDATA &&
            ( iArgument2 == LUA_TNUMBER || iArgument2 == LUA_TSTRING ) )
        {
            CClientVehicle* pVehicle = lua_tovehicle ( L, 1 );
            if ( pVehicle )
            {
                unsigned char ucPanel = static_cast < unsigned char > ( lua_tonumber ( L, 2 ) );
                unsigned char ucState = pVehicle->GetPanelStatus ( ucPanel );
                lua_pushnumber ( L, ucState );
                return 1;
            }
            else
                m_pScriptDebugging->LogBadPointer( "getVehiclePanelState", "vehicle", 1 );
        }
        else
            m_pScriptDebugging->LogBadType( "getVehiclePanelState" );

        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( getVehicleOverrideLights )
    {
        if ( lua_type ( L, 1 ) == LUA_TLIGHTUSERDATA )
        {
            CClientVehicle* pVehicle = lua_tovehicle ( L, 1 );
            if ( pVehicle )
            {
                unsigned char ucLights = pVehicle->GetOverrideLights ();
                lua_pushnumber ( L, ucLights );
                return 1;
            }
            else
                m_pScriptDebugging->LogBadPointer( "getVehicleOverrideLights", "vehicle", 1 );
        }
        else
            m_pScriptDebugging->LogBadType( "getVehicleOverrideLights" );

        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( getVehicleTowedByVehicle )
    {
        if ( lua_type ( L, 1 ) == LUA_TLIGHTUSERDATA )
        {
            CClientVehicle* pVehicle = lua_tovehicle ( L, 1 );
            if ( pVehicle )
            {
                CClientVehicle* pTowedVehicle = pVehicle->GetTowedVehicle ();
                if ( pTowedVehicle )
                {
                    lua_pushelement ( L, pTowedVehicle );
                    return 1;
                }
            }
            else
                m_pScriptDebugging->LogBadPointer( "getVehicleTowedByVehicle", "vehicle", 1 );
        }
        else
            m_pScriptDebugging->LogBadType( "getVehicleTowedByVehicle" );

        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( getVehicleTowingVehicle )
    {
        if ( lua_type ( L, 1 ) == LUA_TLIGHTUSERDATA )
        {
            CClientVehicle* pVehicle = lua_tovehicle ( L, 1 );
            if ( pVehicle )
            {
                CClientVehicle* pTowedByVehicle = pVehicle->GetTowedByVehicle ();
                if ( pTowedByVehicle )
                {
                    lua_pushelement ( L, pTowedByVehicle );
                    return 1;
                }
            }
            else
                m_pScriptDebugging->LogBadPointer( "getVehicleTowingVehicle", "vehicle", 1 );
        }
        else
            m_pScriptDebugging->LogBadType( "getVehicleTowingVehicle" );

        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( getVehiclePaintjob )
    {
        if ( lua_type ( L, 1 ) == LUA_TLIGHTUSERDATA )
        {
            CClientVehicle* pVehicle = lua_tovehicle ( L, 1 );
            if ( pVehicle )
            {
                unsigned char ucPaintjob = pVehicle->GetPaintjob ();
                lua_pushnumber ( L, ucPaintjob );
                return 1;
            }
            else
                m_pScriptDebugging->LogBadPointer( "getVehiclePaintjob", "vehicle", 1 );
        }
        else
            m_pScriptDebugging->LogBadType( "getVehiclePaintjob" );

        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( getVehiclePlateText )
    {
        if ( lua_type ( L, 1 ) == LUA_TLIGHTUSERDATA )
        {
            CClientVehicle* pVehicle = lua_tovehicle ( L, 1 );
            if ( pVehicle )
            {
                const char* szRegPlate = pVehicle->GetRegPlate ();
                if ( szRegPlate )
                {
                    lua_pushstring ( L, szRegPlate );
                    return 1;
                }

                return false;
            }
            else
                m_pScriptDebugging->LogBadPointer( "getVehiclePlateText", "vehicle", 1 );
        }
        else
            m_pScriptDebugging->LogBadType( "getVehiclePlateText" );

        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( isVehicleDamageProof )
    {
        if ( lua_type ( L, 1 ) == LUA_TLIGHTUSERDATA )
        {
            CClientVehicle* pVehicle = lua_tovehicle ( L, 1 );
            if ( pVehicle )
            {
                bool bDamageProof;
                if ( CStaticFunctionDefinitions::IsVehicleDamageProof ( *pVehicle, bDamageProof ) )
                {
                    lua_pushboolean ( L, bDamageProof );
                    return 1;
                }
            }
            else
                m_pScriptDebugging->LogBadPointer( "isVehicleDamageProof", "vehicle", 1 );
        }
        else
            m_pScriptDebugging->LogBadType( "isVehicleDamageProof" );

        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( isVehicleFuelTankExplodable )
    {
        if ( lua_type ( L, 1 ) == LUA_TLIGHTUSERDATA )
        {
            CClientVehicle* pVehicle = lua_tovehicle ( L, 1 );
            if ( pVehicle )
            {
                bool bExplodable = pVehicle->GetCanShootPetrolTank ();
                lua_pushboolean ( L, bExplodable );
                return 1;
            }
            else
                m_pScriptDebugging->LogBadPointer( "isVehicleFuelTankExplodable", "vehicle", 1 );
        }
        else
            m_pScriptDebugging->LogBadType( "isVehicleFuelTankExplodable" );

        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( isVehicleFrozen )
    {
        if ( lua_type ( L, 1 ) == LUA_TLIGHTUSERDATA )
        {
            CClientVehicle* pVehicle = lua_tovehicle ( L, 1 );
            if ( pVehicle )
            {
                bool bFrozen = pVehicle->IsFrozen ();
                lua_pushboolean ( L, bFrozen );
                return 1;
            }
            else
                m_pScriptDebugging->LogBadPointer( "isVehicleFrozen", "vehicle", 1 );
        }
        else
            m_pScriptDebugging->LogBadType( "isVehicleFrozen" );

        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( isVehicleOnGround )
    {
        if ( lua_type ( L, 1 ) == LUA_TLIGHTUSERDATA )
        {
            CClientVehicle* pVehicle = lua_tovehicle ( L, 1 );

            if ( pVehicle )
            {
                bool bOnGround = pVehicle->IsOnGround ();
                lua_pushboolean ( L, bOnGround );
                return 1;
            }
            else
                m_pScriptDebugging->LogBadPointer( "isVehicleOnGround", "vehicle", 1 );
        }
        else
            m_pScriptDebugging->LogBadType( "isVehicleOnGround" );

        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( getVehicleName )
    {
        if ( lua_type ( L, 1 ) == LUA_TLIGHTUSERDATA )
        {
            CClientVehicle* pVehicle = lua_tovehicle ( L, 1 );
            if ( pVehicle )
            {
                const char* szVehicleName = CVehicleNames::GetVehicleName ( pVehicle->GetModel () );
                if ( szVehicleName )
                {
                    lua_pushstring ( L, szVehicleName );
                    return 1;
                }
            }
            else
                m_pScriptDebugging->LogBadPointer( "getVehicleName", "vehicle", 1 );
        }
        else
            m_pScriptDebugging->LogBadType( "getVehicleName" );

        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( getVehicleAdjustableProperty )
    {
        if ( lua_type ( L, 1 ) == LUA_TLIGHTUSERDATA )
        {
            CClientVehicle* pVehicle = lua_tovehicle ( L, 1 );
            if ( pVehicle )
            {
                if ( pVehicle->HasAdjustableProperty () )
                {
                    unsigned short usAdjustableProperty = pVehicle->GetAdjustablePropertyValue ();
                    lua_pushnumber ( L, usAdjustableProperty );
                    return 1;
                }
            }
            else
                m_pScriptDebugging->LogBadPointer( "getVehicleAdjustableProperty", "vehicle", 1 );
        }
        else
            m_pScriptDebugging->LogBadType( "getVehicleAdjustableProperty" );

        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( getHelicopterRotorSpeed )
    {
        if ( lua_type ( L, 1 ) == LUA_TLIGHTUSERDATA )
        {
            CClientVehicle* pVehicle = lua_tovehicle ( L, 1 );
            if ( pVehicle )
            {
                float fSpeed;
                if ( CStaticFunctionDefinitions::GetHelicopterRotorSpeed ( *pVehicle, fSpeed ) )
                {
                    lua_pushnumber ( L, fSpeed );
                    return 1;
                }
            }
            else
                m_pScriptDebugging->LogBadPointer( "getHelicopterRotorSpeed", "vehicle", 1 );
        }
        else
            m_pScriptDebugging->LogBadType( "getHelicopterRotorSpeed" );

        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( isTrainDerailed )
    {
        if ( lua_type ( L, 1 ) == LUA_TLIGHTUSERDATA )
        {
            CClientVehicle* pVehicle = lua_tovehicle ( L, 1 );
            if ( pVehicle )
            {
                bool bDerailed;
                if ( CStaticFunctionDefinitions::IsTrainDerailed ( *pVehicle, bDerailed ) )
                {
                    lua_pushboolean ( L, bDerailed );
                    return 1;
                }
            }
            else
                m_pScriptDebugging->LogBadPointer( "isTrainDerailed", "vehicle", 1 );
        }
        else
            m_pScriptDebugging->LogBadType( "isTrainDerailed" );

        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( isTrainDerailable )
    {
        if ( lua_type ( L, 1 ) == LUA_TLIGHTUSERDATA )
        {
            CClientVehicle* pVehicle = lua_tovehicle ( L, 1 );
            if ( pVehicle )
            {
                bool bIsDerailable;
                if ( CStaticFunctionDefinitions::IsTrainDerailable ( *pVehicle, bIsDerailable ) )
                {
                    lua_pushboolean ( L, bIsDerailable );
                    return 1;
                }
            }
            else
                m_pScriptDebugging->LogBadPointer( "isTrainDerailable", "vehicle", 1 );
        }
        else
            m_pScriptDebugging->LogBadType( "isTrainDerailable" );

        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( getTrainDirection )
    {
        if ( lua_type ( L, 1 ) == LUA_TLIGHTUSERDATA )
        {
            CClientVehicle* pVehicle = lua_tovehicle ( L, 1 );
            if ( pVehicle )
            {
                bool bDirection;
                if ( CStaticFunctionDefinitions::GetTrainDirection ( *pVehicle, bDirection ) )
                {
                    lua_pushboolean ( L, bDirection );
                    return 1;
                }
            }
            else
                m_pScriptDebugging->LogBadPointer( "getTrainDirection", "vehicle", 1 );
        }
        else
            m_pScriptDebugging->LogBadType( "getTrainDirection" );

        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( getTrainSpeed )
    {
        if ( lua_type ( L, 1 ) == LUA_TLIGHTUSERDATA )
        {
            CClientVehicle* pVehicle = lua_tovehicle ( L, 1 );
            if ( pVehicle )
            {
                float fSpeed;
                if ( CStaticFunctionDefinitions::GetTrainSpeed ( *pVehicle, fSpeed ) )
                {
                    lua_pushnumber ( L, fSpeed );
                    return 1;
                }
            }
            else
                m_pScriptDebugging->LogBadPointer( "getTrainSpeed", "vehicle", 1 );
        }
        else
            m_pScriptDebugging->LogBadType( "getTrainSpeed" );

        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( getVehicleEngineState )
    {
        if ( lua_type ( L, 1 ) == LUA_TLIGHTUSERDATA )
        {
            CClientVehicle* pVehicle = lua_tovehicle ( L, 1 );
            if ( pVehicle )
            {
                bool bState;
                if ( CStaticFunctionDefinitions::GetVehicleEngineState ( *pVehicle, bState ) )
                {
                    lua_pushboolean ( L, bState );
                    return 1;
                }
            }
            else
                m_pScriptDebugging->LogBadPointer( "getVehicleEngineState", "vehicle", 1 );
        }
        else
            m_pScriptDebugging->LogBadType( "getVehicleEngineState" );

        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( getVehicleNameFromModel )
    {
        int iArgument1 = lua_type ( L, 1 );
        if ( iArgument1 == LUA_TNUMBER || iArgument1 == LUA_TSTRING )
        {
            unsigned short usModel = static_cast < unsigned short > ( lua_tonumber ( L, 1 ) );
            char szVehicleName [32];

            if ( CStaticFunctionDefinitions::GetVehicleNameFromModel ( usModel, szVehicleName, sizeof(szVehicleName) ) )
            {
                lua_pushstring ( L, szVehicleName );
                return 1;
            }
        }
        else
            m_pScriptDebugging->LogBadType( "getVehicleNameFromID" );

        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( createVehicle )
    {
        // Verify the parameters
        int iArgument1 = lua_type ( L, 1 );
        int iArgument2 = lua_type ( L, 2 );
        int iArgument3 = lua_type ( L, 3 );
        int iArgument4 = lua_type ( L, 4 );
        if ( ( iArgument1 == LUA_TNUMBER || iArgument1 == LUA_TSTRING ) &&
            ( iArgument2 == LUA_TNUMBER || iArgument2 == LUA_TSTRING ) &&
            ( iArgument3 == LUA_TNUMBER || iArgument3 == LUA_TSTRING ) &&
            ( iArgument4 == LUA_TNUMBER || iArgument4 == LUA_TSTRING ) )
        {
            // Grab the vehicle id parameter
            unsigned short usModel = static_cast < unsigned short > ( atoi ( lua_tostring ( L, 1 ) ) );

            // Grab the position parameters
            CVector vecPosition;
            vecPosition.fX = static_cast < float > ( atof ( lua_tostring ( L, 2 ) ) );
            vecPosition.fY = static_cast < float > ( atof ( lua_tostring ( L, 3 ) ) );
            vecPosition.fZ = static_cast < float > ( atof ( lua_tostring ( L, 4 ) ) );

            // Grab the rotation parameters
            CVector vecRotation;
            const char* szRegPlate = NULL;
            int iArgument5 = lua_type ( L, 5 );
            if ( iArgument5 == LUA_TNUMBER || iArgument5 == LUA_TSTRING )
            {
                vecRotation.fX = static_cast < float > ( atof ( lua_tostring ( L, 5 ) ) );

                int iArgument6 = lua_type ( L, 6 );
                if ( iArgument6 == LUA_TNUMBER || iArgument6 == LUA_TSTRING )
                {
                    vecRotation.fY = static_cast < float > ( atof ( lua_tostring ( L, 6 ) ) );

                    int iArgument7 = lua_type ( L, 7 );
                    if ( iArgument7 == LUA_TNUMBER || iArgument7 == LUA_TSTRING )
                    {
                        vecRotation.fZ = static_cast < float > ( atof ( lua_tostring ( L, 7 ) ) );

                        if ( lua_type ( L, 8 ) == LUA_TSTRING )
                        {
                            szRegPlate = lua_tostring ( L, 8 );
                        }
                    }
                }
            }

            CLuaMain* pLuaMain = lua_readcontext( L );
            if ( pLuaMain )
            {
                CResource * pResource = pLuaMain->GetResource();
                if ( pResource )
                {
                    // Create the vehicle and return its handle
                    CClientVehicle* pVehicle = CStaticFunctionDefinitions::CreateVehicle ( *pResource, usModel, vecPosition, vecRotation, szRegPlate );
                    if ( pVehicle )
                    {
                        CElementGroup * pGroup = pResource->GetElementGroup();
                        if ( pGroup )
                        {
                            pGroup->Add ( ( CClientEntity* ) pVehicle );
                        }

                        lua_pushelement ( L, pVehicle );
                        return 1;
                    }
                }
            }
        }
        else
            m_pScriptDebugging->LogBadType( "createVehicle" );

        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( fixVehicle )
    {
        if ( lua_type ( L, 1 ) == LUA_TLIGHTUSERDATA )
        {
            CClientEntity* pEntity = lua_toelement ( L, 1 );
            if ( pEntity )
            {
                if ( CStaticFunctionDefinitions::FixVehicle ( *pEntity ) )
                {
                    lua_pushboolean ( L, true );
                    return 1;
                }
            }
            else
                m_pScriptDebugging->LogBadPointer( "fixVehicle", "element", 1 );
        }
        else
            m_pScriptDebugging->LogBadType( "fixVehicle" );

        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( blowVehicle )
    {
        // Verify the element pointer argument
        if ( lua_type ( L, 1 ) == LUA_TLIGHTUSERDATA )
        {
            // Grab the element
            CClientEntity* pEntity = lua_toelement ( L, 1 );
            if ( pEntity )
            {
                if ( CStaticFunctionDefinitions::BlowVehicle ( *pEntity ) )
                {
                    lua_pushboolean ( L, true );
                    return 1;
                }
            }
            else
                m_pScriptDebugging->LogBadPointer( "blowVehicle", "element", 1 );
        }
        else
            m_pScriptDebugging->LogBadType( "blowVehicle" );

        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( isVehicleBlown )
    {
        if ( lua_type ( L, 1 ) == LUA_TLIGHTUSERDATA )
        {
            CClientVehicle* pVehicle = lua_tovehicle ( L, 1 );
            if ( pVehicle ) {
                bool bBlown;
                if ( CStaticFunctionDefinitions::IsVehicleBlown(*pVehicle, bBlown) ) {
                    lua_pushboolean ( L, bBlown );
                    return 1;
                }
            } 
            else
                m_pScriptDebugging->LogBadPointer( "isVehicleBlown", "vehicle", 1 );
        }
        else
            m_pScriptDebugging->LogBadType( "isVehicleBlown" );

        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( getVehicleHeadLightColor )
    {
        if ( lua_type ( L, 1 ) == LUA_TLIGHTUSERDATA )
        {
            CClientVehicle* pVehicle = lua_tovehicle ( L, 1 );
            if ( pVehicle )
            {
                SColor color;
                if ( CStaticFunctionDefinitions::GetVehicleHeadLightColor ( *pVehicle, color ) )
                {
                    lua_pushnumber ( L, color.R );
                    lua_pushnumber ( L, color.G );
                    lua_pushnumber ( L, color.B );
                    return 3;
                }
            }
            else
                m_pScriptDebugging->LogBadPointer( "getVehicleHeadLightColor", "vehicle", 1 );
        }
        else
            m_pScriptDebugging->LogBadType( "getVehicleHeadLightColor" );

        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( getVehicleCurrentGear )
    {
        if ( lua_type ( L, 1 ) == LUA_TLIGHTUSERDATA )
        {
            CClientVehicle* pVehicle = lua_tovehicle ( L, 1 );
            if ( pVehicle )
            {
                unsigned short currentGear;
                if ( CStaticFunctionDefinitions::GetVehicleCurrentGear ( *pVehicle, currentGear ) )
                {
                    lua_pushnumber ( L, currentGear );
                    return 1;
                }
            }
            else
                m_pScriptDebugging->LogBadPointer( "getVehicleCurrentGear", "vehicle", 1 );
        }
	    else
            m_pScriptDebugging->LogBadType( "getVehicleCurrentGear" );

        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( setVehicleTurnVelocity )
    {
        int iArgumentType1 = lua_type ( L, 1 );
        int iArgumentType2 = lua_type ( L, 2 );
        int iArgumentType3 = lua_type ( L, 3 );
        int iArgumentType4 = lua_type ( L, 4 );
        if ( ( iArgumentType1 == LUA_TLIGHTUSERDATA ) &&
            ( iArgumentType2 == LUA_TNUMBER || iArgumentType2 == LUA_TSTRING ) &&
            ( iArgumentType3 == LUA_TNUMBER || iArgumentType3 == LUA_TSTRING ) &&
            ( iArgumentType4 == LUA_TNUMBER || iArgumentType4 == LUA_TSTRING ) )
        {
            CClientEntity* pEntity = lua_toelement ( L, 1 );
            if ( pEntity )
            {
                CVector vecTurnVelocity;
                vecTurnVelocity.fX = static_cast < float > ( lua_tonumber ( L, 2 ) );
                vecTurnVelocity.fY = static_cast < float > ( lua_tonumber ( L, 3 ) );
                vecTurnVelocity.fZ = static_cast < float > ( lua_tonumber ( L, 4 ) );

                if ( CStaticFunctionDefinitions::SetVehicleTurnVelocity ( *pEntity, vecTurnVelocity ) )
                {
                    lua_pushboolean ( L, true );
                    return 1;
                }
            }
            else
                m_pScriptDebugging->LogBadPointer( "setVehicleTurnVelocity", "element", 1 );
        }
        else
            m_pScriptDebugging->LogBadType( "setVehicleTurnVelocity" );

        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( setVehicleColor )
    {
        // Count up number of args after the first one
        uchar ucParams[12] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
        int i;
        for ( i = 0 ; i < 12 ; i++ )
        {
            int iArgumentType = lua_type ( L, i + 2 );
            if ( iArgumentType == LUA_TNUMBER || iArgumentType == LUA_TSTRING )
            {
                ucParams[i] = static_cast < unsigned char > ( Clamp ( 0.0, lua_tonumber ( L, i + 2 ), 255.0 ) );
            }
            else
                break;
        }

        if  ( ( lua_type ( L, 1 ) == LUA_TLIGHTUSERDATA ) &&
              ( i == 3 || i == 4 || i == 6 || i == 9 || i == 12 ) )
        {
            CClientEntity* pEntity = lua_toelement ( L, 1 );
            if ( pEntity )
            {
                CVehicleColor color;

                if ( i == 4 )
                {
                    // 4 args mean palette colours
                    color.SetPaletteColors ( ucParams[0], ucParams[1], ucParams[2], ucParams[3] );
                }
                else
                {
                    // 3,6,9 or 12 args mean rgb colours
                    color.SetRGBColors ( SColorRGBA ( ucParams[0], ucParams[1], ucParams[2], 0 ),
                                         SColorRGBA ( ucParams[3], ucParams[4], ucParams[5], 0 ),
                                         SColorRGBA ( ucParams[6], ucParams[7], ucParams[8], 0 ),
                                         SColorRGBA ( ucParams[9], ucParams[10], ucParams[11], 0 ) );
                }

                if ( CStaticFunctionDefinitions::SetVehicleColor ( *pEntity, color ) )
                {
                    lua_pushboolean ( L, true );
                    return 1;
                }
            }
            else
                m_pScriptDebugging->LogBadPointer( "setVehicleColor", "element", 1 );
        }
        else
            m_pScriptDebugging->LogBadType( "setVehicleColor" );

        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( setVehicleLandingGearDown )
    {
        // Verify the two arguments
        int iArgument1 = lua_type ( L, 1 );
        int iArgument2 = lua_type ( L, 2 );
        if ( iArgument1 == LUA_TLIGHTUSERDATA && iArgument2 == LUA_TBOOLEAN )
        {
            // Grab the element and verify it
            CClientEntity* pEntity = lua_toelement ( L, 1 );
            bool bLandingGearDown = lua_toboolean ( L, 2 ) ? true:false;
            if ( pEntity )
            {
                CClientVehicle* pVehicle = static_cast < CClientVehicle* > ( pEntity );
                if ( CClientVehicleManager::HasLandingGears ( pVehicle->GetModel () ) )
                {
                    // Do it
                    if ( CStaticFunctionDefinitions::SetVehicleLandingGearDown ( *pEntity, bLandingGearDown ) )
                        lua_pushboolean ( L, true );
                    else
                        lua_pushboolean ( L, false );
                }
                else
                    lua_pushboolean ( L, false );

                return 1;
            }
            else
                m_pScriptDebugging->LogBadPointer( "setVehicleLandingGearDown", "element", 1 );
        }
        else
            m_pScriptDebugging->LogBadType( "setVehicleLandingGearDown" );

        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( setVehicleLocked )
    {
        if ( lua_type ( L, 1 ) == LUA_TLIGHTUSERDATA &&
            lua_type ( L, 2 ) == LUA_TBOOLEAN )
        {
            CClientEntity* pEntity = lua_toelement ( L, 1 );
            bool bLocked = ( lua_toboolean ( L, 2 ) ) ? true:false;

            if ( pEntity )
            {
                if ( CStaticFunctionDefinitions::SetVehicleLocked ( *pEntity, bLocked ) )
                {
                    lua_pushboolean ( L, true );
                    return 1;
                }
            }
            else
                m_pScriptDebugging->LogBadPointer( "setVehicleLocked", "vehicle", 1 );
        }
        else
            m_pScriptDebugging->LogBadType( "setVehicleLocked" );

        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( setVehicleDoorsUndamageable )
    {
        if ( lua_type ( L, 1 ) == LUA_TLIGHTUSERDATA &&
            lua_type ( L, 2 ) == LUA_TBOOLEAN )
        {
            CClientEntity* pEntity = lua_toelement ( L, 1 );
            bool bDoorsUndamageable = ( lua_toboolean ( L, 2 ) ) ? false:true;

            if ( pEntity )
            {
                if ( CStaticFunctionDefinitions::SetVehicleDoorsUndamageable ( *pEntity, bDoorsUndamageable ) )
                {
                    lua_pushboolean ( L, true );
                    return 1;
                }
            }
            else
                m_pScriptDebugging->LogBadPointer( "setVehicleDoorsUndamageable", "vehicle", 1 );
        }
        else
            m_pScriptDebugging->LogBadType( "setVehicleDoorsUndamageable" );

        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( setVehicleSirensOn )
    {
        // Verify the two arguments
        int iArgument1 = lua_type ( L, 1 );
        int iArgument2 = lua_type ( L, 2 );
        if ( iArgument1 == LUA_TLIGHTUSERDATA && iArgument2 == LUA_TBOOLEAN )
        {
            // Grab the element and verify it
            CClientEntity* pEntity = lua_toelement ( L, 1 );
            bool bSirensOn = lua_toboolean ( L, 2 ) ? true:false;
            if ( pEntity )
            {
                // Do it
                if ( CStaticFunctionDefinitions::SetVehicleSirensOn ( *pEntity, bSirensOn ) )
                {
                    lua_pushboolean ( L, true );
                    return 1;
                }
            }
            else
                m_pScriptDebugging->LogBadPointer( "setVehicleSirensOn", "element", 1 );
        }
        else
            m_pScriptDebugging->LogBadType( "setVehicleSirensOn" );

        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( addVehicleUpgrade )
    {
        // Verify the two arguments
        int iArgument1 = lua_type ( L, 1 );
        int iArgument2 = lua_type ( L, 2 );
        if ( iArgument1 == LUA_TLIGHTUSERDATA && 
            ( iArgument2 == LUA_TNUMBER || iArgument2 == LUA_TSTRING ) )
        {

            // Grab the element and verify it
            CClientEntity* pEntity = lua_toelement ( L, 1 );

            unsigned short usUpgrade = static_cast < unsigned short > ( lua_tonumber ( L, 2 ) );
            if ( pEntity )
            {
                if ( iArgument2 == LUA_TSTRING )
                {
                    const char* szUpgrade = lua_tostring ( L, 2 );
                    if ( strcmp ( szUpgrade, "all" ) == 0 )
                    {
                        if ( CStaticFunctionDefinitions::AddAllVehicleUpgrades ( *pEntity ) )
                        {
                            lua_pushboolean ( L, true );
                            return 1;
                        }
                    }
                }

                if ( CStaticFunctionDefinitions::AddVehicleUpgrade ( *pEntity, usUpgrade ) )
                {
                    lua_pushboolean ( L, true );
                    return 1;
                }
            }
            else
                m_pScriptDebugging->LogBadPointer( "addVehicleUpgrade", "element", 1 );
        }
        else
            m_pScriptDebugging->LogBadType( "addVehicleUpgrade" );

        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( removeVehicleUpgrade )
    {
        // Verify the two arguments
        int iArgument1 = lua_type ( L, 1 );
        int iArgument2 = lua_type ( L, 2 );
        if ( iArgument1 == LUA_TLIGHTUSERDATA && 
            ( iArgument2 == LUA_TNUMBER || iArgument2 == LUA_TSTRING ) )
        {

            // Grab the element and verify it
            CClientEntity* pEntity = lua_toelement ( L, 1 );
            unsigned short usUpgrade = static_cast < unsigned short > ( lua_tonumber ( L, 2 ) );
            if ( pEntity )
            {
                // Do it
                if ( CStaticFunctionDefinitions::RemoveVehicleUpgrade ( *pEntity, usUpgrade ) )
                {
                    lua_pushboolean ( L, true );
                    return 1;
                }
            }
            else
                m_pScriptDebugging->LogBadPointer( "removeVehicleUpgrade", "element", 1 );
        }
        else
            m_pScriptDebugging->LogBadType( "removeVehicleUpgrade" );

        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( setVehicleDoorState )
    {
        // Verify the three arguments
        int iArgument1 = lua_type ( L, 1 );
        int iArgument2 = lua_type ( L, 2 );
        int iArgument3 = lua_type ( L, 3 );
        if ( iArgument1 == LUA_TLIGHTUSERDATA &&
            ( iArgument2 == LUA_TNUMBER || iArgument2 == LUA_TSTRING ) &&
            ( iArgument3 == LUA_TNUMBER || iArgument3 == LUA_TSTRING ) )
        {
            // Grab the element and verify it
            CClientEntity* pEntity = lua_toelement ( L, 1 );
            unsigned char ucDoor = static_cast < unsigned char > ( lua_tonumber ( L, 2 ) );
            unsigned char ucState = static_cast < unsigned char > ( lua_tonumber ( L, 3 ) );
            if ( pEntity )
            {
                // Do it
                if ( CStaticFunctionDefinitions::SetVehicleDoorState ( *pEntity, ucDoor, ucState ) )
                {
                    lua_pushboolean ( L, true );
                    return 1;
                }
            }
            else
                m_pScriptDebugging->LogBadPointer( "setVehicleDoorState", "element", 1 );
        }
        else
            m_pScriptDebugging->LogBadType( "setVehicleDoorState" );

        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( setVehicleWheelStates )
    {
        // Verify the three arguments
        int iArgument2 = lua_type ( L, 2 );
        if ( ( lua_type ( L, 1 ) == LUA_TLIGHTUSERDATA ) &&
            ( iArgument2 == LUA_TSTRING || iArgument2 == LUA_TNUMBER ) )
        {
            int iFrontLeft = static_cast < int > ( lua_tonumber ( L, 2 ) );
            int iRearLeft = -1, iFrontRight = -1, iRearRight = -1;

            int iArgument3 = lua_type ( L, 3 ), iArgument4 = lua_type ( L, 4 ),
                iArgument5 = lua_type ( L, 5 );

            if ( iArgument3 == LUA_TSTRING || iArgument3 == LUA_TNUMBER )
                iRearLeft = static_cast < int > ( lua_tonumber ( L, 3 ) );

            if ( iArgument4 == LUA_TSTRING || iArgument4 == LUA_TNUMBER )
                iFrontRight = static_cast < int > ( lua_tonumber ( L, 4 ) );

            if ( iArgument5 == LUA_TSTRING || iArgument5 == LUA_TNUMBER )
                iRearRight = static_cast < int > ( lua_tonumber ( L, 5 ) );

            // Grab the element and verify it
            CClientEntity* pEntity = lua_toelement ( L, 1 );
            if ( pEntity )
            {
                if ( CStaticFunctionDefinitions::SetVehicleWheelStates ( *pEntity, iFrontLeft, iRearLeft, iFrontRight, iRearRight ) )
                {
                    lua_pushboolean ( L, true );
                    return 1;
                }
            }
            else
                m_pScriptDebugging->LogBadPointer( "setVehicleWheelStates", "element", 1 );
        }
        else
            m_pScriptDebugging->LogBadType( "setVehicleWheelStates" );

        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( setVehicleLightState )
    {
        // Verify the three arguments
        int iArgument1 = lua_type ( L, 1 );
        int iArgument2 = lua_type ( L, 2 );
        int iArgument3 = lua_type ( L, 3 );
        if ( iArgument1 == LUA_TLIGHTUSERDATA &&
            ( iArgument2 == LUA_TNUMBER || iArgument2 == LUA_TSTRING ) &&
            ( iArgument3 == LUA_TNUMBER || iArgument3 == LUA_TSTRING ) )
        {
            // Grab the element and verify it
            CClientEntity* pEntity = lua_toelement ( L, 1 );
            unsigned char ucLight = static_cast < unsigned char > ( lua_tonumber ( L, 2 ) );
            unsigned char ucState = static_cast < unsigned char > ( lua_tonumber ( L, 3 ) );
            if ( pEntity )
            {
                // Do it
                if ( CStaticFunctionDefinitions::SetVehicleLightState ( *pEntity, ucLight, ucState ) )
                {
                    lua_pushboolean ( L, true );
                    return 1;
                }
            }
            else
                m_pScriptDebugging->LogBadPointer( "setVehicleLightState", "element", 1 );
        }
        else
            m_pScriptDebugging->LogBadType( "setVehicleLightState" );

        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( setVehiclePanelState )
    {
        // Verify the three arguments
        int iArgument1 = lua_type ( L, 1 );
        int iArgument2 = lua_type ( L, 2 );
        int iArgument3 = lua_type ( L, 3 );
        if ( iArgument1 == LUA_TLIGHTUSERDATA &&
            ( iArgument2 == LUA_TNUMBER || iArgument2 == LUA_TSTRING ) &&
            ( iArgument3 == LUA_TNUMBER || iArgument3 == LUA_TSTRING ) )
        {
            // Grab the element and verify it
            CClientEntity* pEntity = lua_toelement ( L, 1 );
            unsigned char ucPanel = static_cast < unsigned char > ( lua_tonumber ( L, 2 ) );
            unsigned char ucState = static_cast < unsigned char > ( lua_tonumber ( L, 3 ) );
            if ( pEntity )
            {
                // Do it
                if ( CStaticFunctionDefinitions::SetVehiclePanelState ( *pEntity, ucPanel, ucState ) )
                {
                    lua_pushboolean ( L, true );
                    return 1;
                }
            }
            else
                m_pScriptDebugging->LogBadPointer( "setVehiclePanelState", "element", 1 );
        }
        else
            m_pScriptDebugging->LogBadType( "setVehiclePanelState" );

        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( setVehicleOverrideLights )
    {
        int iArgument2 = lua_type ( L, 2 );
        if ( ( lua_type ( L, 1 ) == LUA_TLIGHTUSERDATA ) &&
            ( iArgument2 == LUA_TNUMBER || iArgument2 == LUA_TSTRING ) )
        {
            CClientEntity* pEntity = lua_toelement ( L, 1 );
            unsigned char ucLights = static_cast < unsigned char > ( lua_tonumber ( L, 2 ) );

            if ( pEntity )
            {
                if ( ucLights <= 2 )
                {
                    if ( CStaticFunctionDefinitions::SetVehicleOverrideLights ( *pEntity, ucLights ) )
                    {
                        lua_pushboolean ( L, true );
                        return 1;
                    }
                }
            }
            else
                m_pScriptDebugging->LogBadPointer( "setVehicleOverrideLights", "element", 1 );
        }
        else
            m_pScriptDebugging->LogBadType( "setVehicleOverrideLights" );

        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( attachTrailerToVehicle )
    {
        if ( lua_type ( L, 1 ) == LUA_TLIGHTUSERDATA &&
            lua_type ( L, 2 ) == LUA_TLIGHTUSERDATA )
        {
            CClientVehicle* pVehicle = lua_tovehicle ( L, 1 );
            CClientVehicle* pTrailer = lua_tovehicle ( L, 2 );

            if ( pVehicle )
            {
                if ( pTrailer )
                {
                    if ( CStaticFunctionDefinitions::AttachTrailerToVehicle ( *pVehicle, *pTrailer ) )
                    {
                        lua_pushboolean ( L, true );
                        return 1;
                    }
                }
                else
                    m_pScriptDebugging->LogBadPointer( "attachTrailerToVehicle", "trailer", 2 );
            }
            else
                m_pScriptDebugging->LogBadPointer( "attachTrailerToVehicle", "vehicle", 1 );
        }
        else
            m_pScriptDebugging->LogBadType( "attachTrailerToVehicle" );

        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( detachTrailerFromVehicle )
    {
        if ( lua_type ( L, 1 ) == LUA_TLIGHTUSERDATA )
        {
            CClientVehicle* pVehicle = lua_tovehicle ( L, 1 );
            CClientVehicle* pTrailer = NULL;

            if ( lua_type ( L, 2 )  == LUA_TLIGHTUSERDATA )
                pTrailer = lua_tovehicle ( L, 2 );

            if ( pVehicle )
            {
                if ( CStaticFunctionDefinitions::DetachTrailerFromVehicle ( *pVehicle, pTrailer ) )
                {
                    lua_pushboolean ( L, true );
                    return 1;
                }
            }
            else
                m_pScriptDebugging->LogBadPointer( "detachTrailerFromVehicle", "vehicle", 1 );
        }
        else
            m_pScriptDebugging->LogBadType( "detachTrailerFromVehicle" );

        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( setVehicleEngineState )
    {
        if ( lua_type ( L, 1 ) == LUA_TLIGHTUSERDATA &&
            lua_type ( L, 2 ) == LUA_TBOOLEAN )
        {
            CClientEntity* pEntity = lua_toelement ( L, 1 );
            bool bState = ( lua_toboolean ( L, 2 ) ) ? true:false;

            if ( pEntity )
            {
                if ( CStaticFunctionDefinitions::SetVehicleEngineState ( *pEntity, bState ) )
                {
                    lua_pushboolean ( L, true );
                    return 1;
                }
            }
            else
                m_pScriptDebugging->LogBadPointer( "setVehicleEngineState", "vehicle", 1 );
        }
        else
            m_pScriptDebugging->LogBadType( "setVehicleEngineState" );

        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( setVehicleDirtLevel )
    {
        int iArgument2 = lua_type ( L, 2 );
        if ( ( lua_type ( L, 1 ) == LUA_TLIGHTUSERDATA ) &&
            ( /*iArgument2 == LUA_TSTRING ||*/ iArgument2 == LUA_TNUMBER ) )
        {
            CClientEntity* pEntity = lua_toelement ( L, 1 );
            float fDirtLevel = static_cast < float > ( lua_tonumber ( L, 2 ) ); /*( atof ( lua_tostring ( L, 2 ) ) );*/

            if ( pEntity )
            {
                if ( CStaticFunctionDefinitions::SetVehicleDirtLevel ( *pEntity, fDirtLevel ) )
                {
                    lua_pushboolean ( L, true );
                    return 1;
                }
            }
            else
                m_pScriptDebugging->LogBadPointer( "setVehicleDirtLevel", "vehicle", 1 );
        }
        else
            m_pScriptDebugging->LogBadType( "setVehicleDirtLevel" );

        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( setVehicleDamageProof )
    {
        if ( lua_type ( L, 1 ) == LUA_TLIGHTUSERDATA &&
            lua_type ( L, 2 ) == LUA_TBOOLEAN )
        {
            CClientEntity* pEntity = lua_toelement ( L, 1 );
            bool bDamageProof = ( lua_toboolean ( L, 2 ) ) ? true:false;

            if ( pEntity )
            {
                if ( CStaticFunctionDefinitions::SetVehicleDamageProof ( *pEntity, bDamageProof ) )
                {
                    lua_pushboolean ( L, true );
                    return 1;
                }
            }
            else
                m_pScriptDebugging->LogBadPointer( "setVehicleDamageProof", "vehicle", 1 );
        }
        else
            m_pScriptDebugging->LogBadType( "setVehicleDamageProof" );

        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( setVehiclePaintjob )
    {
        int iArgument2 = lua_type ( L, 2 );
        if ( ( lua_type ( L, 1 ) == LUA_TLIGHTUSERDATA ) &&
            ( iArgument2 == LUA_TNUMBER || iArgument2 == LUA_TSTRING ) )
        {
            CClientEntity* pEntity = lua_toelement ( L, 1 );
            if ( pEntity )
            {
                unsigned char ucPaintjob = static_cast < unsigned char > ( lua_tonumber ( L, 2 ) );
                if ( CStaticFunctionDefinitions::SetVehiclePaintjob ( *pEntity, ucPaintjob ) )
                {
                    lua_pushboolean ( L, true );
                    return 1;
                }
            }
            else
                m_pScriptDebugging->LogBadPointer( "setVehiclePaintjob", "vehicle", 1 );
        }
        else
            m_pScriptDebugging->LogBadType( "setVehiclePaintjob" );

        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( setVehicleFuelTankExplodable )
    {
        if ( lua_type ( L, 1 ) == LUA_TLIGHTUSERDATA &&
            lua_type ( L, 2 ) == LUA_TBOOLEAN )
        {
            CClientEntity* pEntity = lua_toelement ( L, 1 );
            if ( pEntity )
            {
                bool bExplodable = ( lua_toboolean ( L, 2 ) ) ? true:false;
                if ( CStaticFunctionDefinitions::SetVehicleFuelTankExplodable ( *pEntity, bExplodable ) )
                {
                    lua_pushboolean ( L, true );
                    return 1;
                }
            }
            else
                m_pScriptDebugging->LogBadPointer( "setVehicleFuelTankExplodable", "vehicle", 1 );
        }
        else
            m_pScriptDebugging->LogBadType( "setVehicleFuelTankExplodable" );

        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( setVehicleFrozen )
    {
        if ( lua_type ( L, 1 ) == LUA_TLIGHTUSERDATA )
        {
            CClientVehicle* pVehicle = lua_tovehicle ( L, 1 );
            if ( pVehicle )
            {
                if ( lua_type ( L, 2 ) == LUA_TBOOLEAN )
                {
                    if ( CStaticFunctionDefinitions::SetVehicleFrozen ( *pVehicle, lua_toboolean ( L, 2 ) ? true:false ) )
                    {
                        lua_pushboolean ( L, true );
                    }
                    return 1;

                }
                else
                    m_pScriptDebugging->LogBadType( "setVehicleFrozen" );
            }
            else
                m_pScriptDebugging->LogBadPointer( "setVehicleFrozen", "vehicle", 1 );
        }
        else
            m_pScriptDebugging->LogBadType( "setVehicleFrozen" );
        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( setVehicleAdjustableProperty )
    {
        int iArgument2 = lua_type ( L, 2 );
        if ( ( lua_type ( L, 1 ) == LUA_TLIGHTUSERDATA ) &&
            ( iArgument2 == LUA_TNUMBER || iArgument2 == LUA_TSTRING ) )
        {
            CClientEntity* pEntity = lua_toelement ( L, 1 );
            if ( pEntity )
            {
                unsigned short usAdjustableProperty = static_cast < unsigned short > ( lua_tonumber ( L, 2 ) );
                if ( CStaticFunctionDefinitions::SetVehicleAdjustableProperty ( *pEntity, usAdjustableProperty ) )
                {
                    lua_pushboolean ( L, true );
                    return 1;
                }
            }
            else
                m_pScriptDebugging->LogBadPointer( "setVehicleAdjustableProperty", "vehicle", 1 );
        }
        else
            m_pScriptDebugging->LogBadType( "setVehicleAdjustableProperty" );

        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( setHelicopterRotorSpeed )
    {
        int iArgument2 = lua_type ( L, 2 );
        if ( ( lua_type ( L, 1 ) == LUA_TLIGHTUSERDATA ) &&
            ( iArgument2 == LUA_TNUMBER || iArgument2 == LUA_TSTRING ) )
        {
            CClientVehicle* pVehicle = lua_tovehicle ( L, 1 );
            if ( pVehicle )
            {
                float fSpeed = static_cast < float > ( lua_tonumber ( L, 2 ) );
                if ( CStaticFunctionDefinitions::SetHelicopterRotorSpeed ( *pVehicle, fSpeed ) )
                {
                    lua_pushboolean ( L, true );
                    return 1;
                }
            }
            else
                m_pScriptDebugging->LogBadPointer( "setHelicopterRotorSpeed", "vehicle", 1 );
        }
        else
            m_pScriptDebugging->LogBadType( "setHelicopterRotorSpeed" );

        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( setTrainDerailed )
    {
        if ( ( lua_type ( L, 1 ) == LUA_TLIGHTUSERDATA ) &&
            ( lua_type ( L, 2 ) == LUA_TBOOLEAN ) )
        {
            CClientVehicle* pVehicle = lua_tovehicle ( L, 1 );
            if ( pVehicle )
            {
                bool bDerailed = ( lua_toboolean ( L, 2 ) ? true : false );
                if ( CStaticFunctionDefinitions::SetTrainDerailed ( *pVehicle, bDerailed ) )
                {
                    lua_pushboolean ( L, true );
                    return 1;
                }
            }
            else
                m_pScriptDebugging->LogBadPointer( "setTrainDerailed", "vehicle", 1 );
        }
        else
            m_pScriptDebugging->LogBadType( "setTrainDerailed" );

        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( setTrainDerailable )
    {
        if ( ( lua_type ( L, 1 ) == LUA_TLIGHTUSERDATA ) &&
            ( lua_type ( L, 2 ) == LUA_TBOOLEAN ) )
        {
            CClientVehicle* pVehicle = lua_tovehicle ( L, 1 );
            if ( pVehicle )
            {
                bool bDerailable = ( lua_toboolean ( L, 2 ) ? true : false );
                if ( CStaticFunctionDefinitions::SetTrainDerailable ( *pVehicle, bDerailable ) )
                {
                    lua_pushboolean ( L, true );
                    return 1;
                }
            }
            else
                m_pScriptDebugging->LogBadPointer( "setTrainDerailable", "vehicle", 1 );
        }
        else
            m_pScriptDebugging->LogBadType( "setTrainDerailable" );

        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( setTrainDirection )
    {
        if ( ( lua_type ( L, 1 ) == LUA_TLIGHTUSERDATA ) &&
            ( lua_type ( L, 2 ) == LUA_TBOOLEAN ) )
        {
            CClientVehicle* pVehicle = lua_tovehicle ( L, 1 );
            if ( pVehicle )
            {
                bool bDirection = lua_toboolean ( L, 2 ) ? true : false;
                if ( CStaticFunctionDefinitions::SetTrainDirection ( *pVehicle, bDirection ) )
                {
                    lua_pushboolean ( L, true );
                    return 1;
                }
            }
            else
                m_pScriptDebugging->LogBadPointer( "setTrainDirection", "vehicle", 1 );
        }
        else
            m_pScriptDebugging->LogBadType( "setTrainDirection" );

        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( setTrainSpeed )
    {
        int iArgument2 = lua_type ( L, 2 );
        if ( ( lua_type ( L, 1 ) == LUA_TLIGHTUSERDATA ) &&
            ( iArgument2 == LUA_TNUMBER || iArgument2 == LUA_TSTRING ) )
        {
            CClientVehicle* pVehicle = lua_tovehicle ( L, 1 );
            if ( pVehicle )
            {
                float fSpeed = static_cast < float > ( lua_tonumber ( L, 2 ) );
                if ( CStaticFunctionDefinitions::SetTrainSpeed ( *pVehicle, fSpeed ) )
                {
                    lua_pushboolean ( L, true );
                    return 1;
                }
            }
            else
                m_pScriptDebugging->LogBadPointer( "setTrainSpeed", "vehicle", 1 );
        }
        else
            m_pScriptDebugging->LogBadType( "setTrainSpeed" );

        lua_pushboolean ( L, false );
        return 1;
    }

    // Radio
    LUA_DECLARE( setRadioChannel )
    {
        if ( lua_type ( L, 1 ) == LUA_TNUMBER ||
            lua_type ( L, 1 ) == LUA_TSTRING )
        {
            unsigned char ucChannel = ( unsigned char ) lua_tonumber ( L, 1 );
            if ( CStaticFunctionDefinitions::SetRadioChannel ( ucChannel ) )
            {
                lua_pushboolean ( L, true );
                return 1;
            }
        }
        else
            m_pScriptDebugging->LogBadType( "setRadioChannel" );

        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( getRadioChannel )
    {
        unsigned char ucChannel = 0;
        if ( CStaticFunctionDefinitions::GetRadioChannel ( ucChannel ) )
        {
            lua_pushnumber ( L, ucChannel );
            return 1;
        }

        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( getRadioChannelName )
    {
        static const char* szRadioStations[] = { "Radio off", "Playback FM", "K-Rose", "K-DST",
            "Bounce FM", "SF-UR", "Radio Los Santos", "Radio X", "CSR 103.9", "K-Jah West",
            "Master Sounds 98.3", "WCTR", "User Track Player" };

        if ( lua_type ( L, 1 ) == LUA_TNUMBER )
        {
            int iChannel = static_cast < int > ( lua_tonumber ( L, 1 ) );
            if ( iChannel >= 0 && iChannel < sizeof(szRadioStations)/sizeof(char *) ) {
                lua_pushstring ( L, szRadioStations [ iChannel ] );
                return 1;
            }
        }
        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( getVehicleGravity )
    {
        if ( lua_type ( L, 1 ) == LUA_TLIGHTUSERDATA )
        {
            CClientVehicle* pVehicle = lua_tovehicle ( L, 1 );
            if ( pVehicle )
            {
                CVector vecGravity;
                pVehicle->GetGravity ( vecGravity );
                lua_pushnumber ( L, vecGravity.fX );
                lua_pushnumber ( L, vecGravity.fY );
                lua_pushnumber ( L, vecGravity.fZ );
                return 3;
            }
            else
                m_pScriptDebugging->LogBadPointer( "getVehicleGravity", "vehicle", 1 );
        }
        else
            m_pScriptDebugging->LogBadType( "getVehicleGravity" );

        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( setVehicleGravity )
    {
        int iArgument2 = lua_type ( L, 2 );
        int iArgument3 = lua_type ( L, 3 );
        int iArgument4 = lua_type ( L, 4 );
        if ( ( lua_type ( L, 1 ) == LUA_TLIGHTUSERDATA ) &&
             ( iArgument2 == LUA_TNUMBER || iArgument2 == LUA_TSTRING ) &&
             ( iArgument3 == LUA_TNUMBER || iArgument3 == LUA_TSTRING ) &&
             ( iArgument4 == LUA_TNUMBER || iArgument4 == LUA_TSTRING ) )
        {
            CClientVehicle* pVehicle = lua_tovehicle ( L, 1 );
            if ( pVehicle )
            {
                CVector vecGravity ( static_cast < float > ( lua_tonumber ( L, 2 ) ),
                                     static_cast < float > ( lua_tonumber ( L, 3 ) ),
                                     static_cast < float > ( lua_tonumber ( L, 4 ) ) );
                pVehicle->SetGravity ( vecGravity );
                lua_pushboolean ( L, true );
                return 1;
            }
            else
                m_pScriptDebugging->LogBadPointer( "setVehicleGravity", "vehicle", 1 );
        }
        else
            m_pScriptDebugging->LogBadType( "setVehicleGravity" );

        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( setVehicleHeadLightColor )
    {
        int iArgument2 = lua_type ( L, 2 );
        int iArgument3 = lua_type ( L, 3 );
        int iArgument4 = lua_type ( L, 4 );
        if ( ( lua_type ( L, 1 ) == LUA_TLIGHTUSERDATA ) &&
             ( iArgument2 == LUA_TNUMBER || iArgument2 == LUA_TSTRING ) &&
             ( iArgument3 == LUA_TNUMBER || iArgument3 == LUA_TSTRING ) &&
             ( iArgument4 == LUA_TNUMBER || iArgument4 == LUA_TSTRING ) )
        {
            CClientEntity* pEntity = lua_toelement ( L, 1 );
            if ( pEntity )
            {
                SColor color;
                color.R = static_cast < unsigned char > ( lua_tonumber ( L, 2 ) );
                color.G = static_cast < unsigned char > ( lua_tonumber ( L, 3 ) );
                color.B = static_cast < unsigned char > ( lua_tonumber ( L, 4 ) );
                color.A = 255;

                if ( CStaticFunctionDefinitions::SetVehicleHeadLightColor ( *pEntity, color ) )
                {
                    lua_pushboolean ( L, true );
                    return 1;
                }
            }
            else
                m_pScriptDebugging->LogBadPointer( "setVehicleHeadLightColor", "vehicle", 1 );
        }
        else
            m_pScriptDebugging->LogBadType( "setVehicleHeadLightColor" );

        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( setVehicleTurretPosition )
    {
        if ( lua_type ( L, 1 ) == LUA_TLIGHTUSERDATA
            && lua_type( L, 2 ) == LUA_TNUMBER
            && lua_type( L, 3 ) == LUA_TNUMBER )
        {
            CClientVehicle* pVehicle = lua_tovehicle ( L, 1 );
            if ( pVehicle )
            {
                float fHorizontal = ( float ) lua_tonumber ( L, 2 );
                float fVertical   = ( float ) lua_tonumber ( L, 3 );

                pVehicle->SetTurretRotation ( fHorizontal, fVertical );

                lua_pushboolean ( L, true );
                return 1;
            }
        }

        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( getVehicleHandling )
    {
        if ( lua_type ( L, 1 ) == LUA_TLIGHTUSERDATA )
        {
            CClientVehicle* pVehicle = lua_tovehicle ( L, 1 );
            if ( pVehicle )
            {
                CHandlingEntry* pEntry = pVehicle->GetHandlingData ();

                lua_newtable ( L );

                lua_pushnumber ( L, pEntry->GetMass() );
                lua_setfield ( L, -2, "mass" );

                lua_pushnumber ( L, pEntry->GetTurnMass() );
                lua_setfield ( L, -2, "turnMass" );

                lua_pushnumber ( L, pEntry->GetDragCoeff() );
                lua_setfield ( L, -2, "dragCoeff" );

                lua_createtable ( L, 3, 0 );
                CVector vecCenter = pEntry->GetCenterOfMass ();
                lua_pushnumber ( L, 1 );
                lua_pushnumber ( L, vecCenter.fX );
                lua_settable ( L, -3 );
                lua_pushnumber ( L, 2 );
                lua_pushnumber ( L, vecCenter.fY );
                lua_settable ( L, -3 );
                lua_pushnumber ( L, 3 );
                lua_pushnumber ( L, vecCenter.fZ );
                lua_settable ( L, -3 );
                lua_setfield ( L, -2, "centerOfMass" );

                lua_pushnumber ( L, pEntry->GetPercentSubmerged() );
                lua_setfield ( L, -2, "percentSubmerged" );

                lua_pushnumber ( L, pEntry->GetTractionMultiplier() );
                lua_setfield ( L, -2, "tractionMultiplier" );

                CHandlingEntry::eDriveType eDriveType = pEntry->GetCarDriveType ();
                if ( eDriveType == CHandlingEntry::FWD )
                    lua_pushstring ( L, "fwd" );
                else if ( eDriveType==CHandlingEntry::RWD )
                    lua_pushstring ( L, "rwd" );
                else if ( eDriveType==CHandlingEntry::FOURWHEEL )
                    lua_pushstring ( L, "awd" );
                else // What the ... (yeah, security)
                    lua_pushnil ( L );
                lua_setfield ( L, -2, "driveType" );
                CHandlingEntry::eEngineType eEngineType = pEntry->GetCarEngineType ();
                if ( eEngineType == CHandlingEntry::PETROL )
                    lua_pushstring ( L, "petrol" );
                else if ( eEngineType == CHandlingEntry::DIESEL )
                    lua_pushstring ( L, "diesel" );
                else if ( eEngineType == CHandlingEntry::ELECTRIC )
                    lua_pushstring ( L, "electric" );
                else
                    lua_pushnil ( L );
                lua_setfield ( L, -2, "engineType" );

                lua_pushnumber ( L, pEntry->GetNumberOfGears () );
                lua_setfield ( L, -2, "numberOfGears" );

                lua_pushnumber ( L, pEntry->GetEngineAcceleration () );
                lua_setfield ( L, -2, "engineAcceleration" );

                lua_pushnumber ( L, pEntry->GetEngineInertia () );
                lua_setfield ( L, -2, "engineInertia" );

                lua_pushnumber ( L, pEntry->GetMaxVelocity () );
                lua_setfield ( L, -2, "maxVelocity" );

                lua_pushnumber ( L, pEntry->GetBrakeDeceleration () );
                lua_setfield ( L, -2, "brakeDeceleration" );

                lua_pushnumber ( L, pEntry->GetBrakeBias () );
                lua_setfield ( L, -2, "brakeBias" );

                lua_pushboolean ( L, pEntry->GetABS () );
                lua_setfield ( L, -2, "ABS" );

                lua_pushnumber ( L, pEntry->GetSteeringLock () );
                lua_setfield ( L, -2, "steeringLock" );

                lua_pushnumber ( L, pEntry->GetTractionLoss () );
                lua_setfield ( L, -2, "tractionLoss" );

                lua_pushnumber ( L, pEntry->GetTractionBias () );
                lua_setfield ( L, -2, "tractionBias" );

                lua_pushnumber ( L, pEntry->GetSuspensionForceLevel () );
                lua_setfield ( L, -2, "suspensionForceLevel" );

                lua_pushnumber ( L, pEntry->GetSuspensionDamping () );
                lua_setfield ( L, -2, "suspensionDamping" );

                lua_pushnumber ( L, pEntry->GetSuspensionHighSpeedDamping () );
                lua_setfield ( L, -2, "suspensionHighSpeedDamping" );

                lua_pushnumber ( L, pEntry->GetSuspensionUpperLimit () );
                lua_setfield ( L, -2, "suspensionUpperLimit" );

                lua_pushnumber ( L, pEntry->GetSuspensionLowerLimit () );
                lua_setfield ( L, -2, "suspensionLowerLimit" );

                lua_pushnumber ( L, pEntry->GetSuspensionFrontRearBias () );
                lua_setfield ( L, -2, "suspensionFrontRearBias" );

                lua_pushnumber ( L, pEntry->GetSuspensionAntiDiveMultiplier () );
                lua_setfield ( L, -2, "suspensionAntiDiveMultiplier" );

                lua_pushnumber ( L, pEntry->GetCollisionDamageMultiplier () );
                lua_setfield ( L, -2, "collisionDamageMultiplier" );

                lua_pushnumber ( L, pEntry->GetSeatOffsetDistance () );
                lua_setfield ( L, -2, "seatOffsetDistance" );

                lua_pushnumber ( L, pEntry->GetHandlingFlags () );
                lua_setfield ( L, -2, "handlingFlags" );

                lua_pushnumber ( L, pEntry->GetModelFlags () );
                lua_setfield ( L, -2, "modelFlags" );

                lua_pushnumber ( L, pEntry->GetMonetary () );
                lua_setfield ( L, -2, "monetary" );

                CHandlingEntry::eLightType eHeadType = pEntry->GetHeadLight ();
                if ( eHeadType == CHandlingEntry::LONG )
                    lua_pushstring ( L, "long" );
                else if ( eHeadType == CHandlingEntry::SMALL )
                    lua_pushstring ( L, "small" );
                else if ( eHeadType == CHandlingEntry::BIG )
                    lua_pushstring ( L, "big" );
                else
                    lua_pushnil( L );
                lua_setfield ( L, -2, "headLight" );

                CHandlingEntry::eLightType eTailType = pEntry->GetTailLight ();
                if ( eTailType == CHandlingEntry::LONG )
                    lua_pushstring ( L, "long" );
                else if ( eTailType == CHandlingEntry::SMALL )
                    lua_pushstring ( L, "small" );
                else if ( eTailType == CHandlingEntry::BIG )
                    lua_pushstring ( L, "big" );
                else
                    lua_pushnil( L );
                lua_setfield ( L, -2, "tailLight" );

                lua_pushnumber ( L, pEntry->GetAnimGroup () );
                lua_setfield ( L, -2, "animGroup" );
                return 1;
            }
            else
                m_pScriptDebugging->LogBadPointer( "getVehicleHandling", "vehicle", 1 );
        }
        else
            m_pScriptDebugging->LogBadType( "getVehicleHandling" );

        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( getOriginalHandling )
    {
        if ( lua_type ( L, 1 ) == LUA_TNUMBER )
        {
            eVehicleTypes eModel = static_cast < eVehicleTypes > ( (int)lua_tonumber ( L, 1 ) );
            if ( eModel )
            {
                const CHandlingEntry* pEntry = g_pGame->GetHandlingManager()->GetOriginalHandlingData( eModel );

                lua_newtable ( L );
                lua_pushnumber ( L, pEntry->GetMass() );
                lua_setfield ( L, -2, "mass" );
                lua_pushnumber ( L, pEntry->GetTurnMass() );
                lua_setfield ( L, -2, "turnMass" );
                lua_pushnumber ( L, pEntry->GetDragCoeff() );
                lua_setfield ( L, -2, "dragCoeff" );
                lua_createtable ( L, 3, 0 );
                CVector vecCenter = pEntry->GetCenterOfMass ();
                lua_pushnumber ( L, 1 );
                lua_pushnumber ( L, vecCenter.fX );
                lua_settable ( L, -3 );
                lua_pushnumber ( L, 2 );
                lua_pushnumber ( L, vecCenter.fY );
                lua_settable ( L, -3 );
                lua_pushnumber ( L, 3 );
                lua_pushnumber ( L, vecCenter.fZ );
                lua_settable ( L, -3 );
                lua_setfield ( L, -2, "centerOfMass" );
                lua_pushnumber ( L, pEntry->GetPercentSubmerged() );
                lua_setfield ( L, -2, "percentSubmerged" );
                lua_pushnumber ( L, pEntry->GetTractionMultiplier() );
                lua_setfield ( L, -2, "tractionMultiplier" );
                CHandlingEntry::eDriveType eDriveType=pEntry->GetCarDriveType();
                if (eDriveType==CHandlingEntry::FWD)
                    lua_pushstring(L,"fwd");
                else if (eDriveType==CHandlingEntry::RWD)
                    lua_pushstring(L,"rwd");
                else if (eDriveType==CHandlingEntry::FOURWHEEL)
                    lua_pushstring(L,"awd");
                else // What the ... (yeah, security)
                    lua_pushnil ( L );
                lua_setfield ( L, -2, "driveType" );
                CHandlingEntry::eEngineType eEngineType=pEntry->GetCarEngineType();
                if (eEngineType==CHandlingEntry::PETROL)
                    lua_pushstring(L,"petrol");
                else if (eEngineType==CHandlingEntry::DIESEL)
                    lua_pushstring(L,"diesel");
                else if (eEngineType==CHandlingEntry::ELECTRIC)
                    lua_pushstring(L,"electric");
                else
                    lua_pushnil ( L );
                lua_setfield ( L, -2, "engineType" );
                lua_pushnumber ( L, pEntry->GetNumberOfGears() );
                lua_setfield ( L, -2, "numberOfGears" );
                lua_pushnumber ( L, pEntry->GetEngineAcceleration() );
                lua_setfield ( L, -2, "engineAcceleration" );
                lua_pushnumber ( L, pEntry->GetEngineInertia() );
                lua_setfield ( L, -2, "engineInertia" );
                lua_pushnumber ( L, pEntry->GetMaxVelocity() );
                lua_setfield ( L, -2, "maxVelocity" );
                lua_pushnumber ( L, pEntry->GetBrakeDeceleration() );
                lua_setfield ( L, -2, "brakeDeceleration" );
                lua_pushnumber ( L, pEntry->GetBrakeBias() );
                lua_setfield ( L, -2, "brakeBias" );
                lua_pushboolean ( L, pEntry->GetABS() );
                lua_setfield ( L, -2, "ABS" );
                lua_pushnumber ( L, pEntry->GetSteeringLock() );
                lua_setfield ( L, -2, "steeringLock" );
                lua_pushnumber ( L, pEntry->GetTractionLoss() );
                lua_setfield ( L, -2, "tractionLoss" );
                lua_pushnumber ( L, pEntry->GetTractionBias() );
                lua_setfield ( L, -2, "tractionBias" );
                lua_pushnumber ( L, pEntry->GetSuspensionForceLevel() );
                lua_setfield ( L, -2, "suspensionForceLevel" );
                lua_pushnumber ( L, pEntry->GetSuspensionDamping() );
                lua_setfield ( L, -2, "suspensionDamping" );
                lua_pushnumber ( L, pEntry->GetSuspensionHighSpeedDamping() );
                lua_setfield ( L, -2, "suspensionHighSpeedDamping" );
                lua_pushnumber ( L, pEntry->GetSuspensionUpperLimit() );
                lua_setfield ( L, -2, "suspensionUpperLimit" );
                lua_pushnumber ( L, pEntry->GetSuspensionLowerLimit() );
                lua_setfield ( L, -2, "suspensionLowerLimit" );
                lua_pushnumber ( L, pEntry->GetSuspensionFrontRearBias() );
                lua_setfield ( L, -2, "suspensionFrontRearBias" );
                lua_pushnumber ( L, pEntry->GetSuspensionAntiDiveMultiplier() );
                lua_setfield ( L, -2, "suspensionAntiDiveMultiplier" );
                lua_pushnumber ( L, pEntry->GetCollisionDamageMultiplier() );
                lua_setfield ( L, -2, "collisionDamageMultiplier" );
                lua_pushnumber ( L, pEntry->GetSeatOffsetDistance() );
                lua_setfield ( L, -2, "seatOffsetDistance" );
                lua_pushnumber ( L, pEntry->GetHandlingFlags() );
                lua_setfield ( L, -2, "handlingFlags" );
                lua_pushnumber ( L, pEntry->GetModelFlags() );
                lua_setfield ( L, -2, "modelFlags" );
                lua_pushnumber ( L, pEntry->GetMonetary() );
                lua_setfield ( L, -2, "monetary" );
                CHandlingEntry::eLightType eHeadType=pEntry->GetHeadLight();
                if (eHeadType==CHandlingEntry::LONG)
                    lua_pushstring(L,"long");
                else if (eHeadType==CHandlingEntry::SMALL)
                    lua_pushstring(L,"small");
                else if (eHeadType==CHandlingEntry::BIG)
                    lua_pushstring(L,"big");
                else
                    lua_pushnil( L );
                lua_setfield ( L, -2, "headLight" );
                CHandlingEntry::eLightType eTailType=pEntry->GetTailLight();
                if (eTailType==CHandlingEntry::LONG)
                    lua_pushstring(L,"long");
                else if (eTailType==CHandlingEntry::SMALL)
                    lua_pushstring(L,"small");
                else if (eTailType==CHandlingEntry::BIG)
                    lua_pushstring(L,"big");
                else
                    lua_pushnil( L );
                lua_setfield ( L, -2, "tailLight" );
                lua_pushnumber ( L, pEntry->GetAnimGroup() );
                lua_setfield ( L, -2, "animGroup" );
                return 1;
            }
            else
                m_pScriptDebugging->LogBadPointer( "getOriginalHandling", "model", 1 );
        }
        else
            m_pScriptDebugging->LogBadType( "getOriginalHandling" );

        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( setVehicleDoorOpenRatio )
    {
        if ( lua_type ( L, 1 ) == LUA_TLIGHTUSERDATA &&
             lua_type ( L, 2 ) == LUA_TNUMBER &&
             lua_type ( L, 3 ) == LUA_TNUMBER )
        {
            CClientEntity* pEntity = lua_toelement ( L, 1 );
            if ( pEntity )
            {
                unsigned char ucDoor = static_cast < unsigned char > ( lua_tonumber ( L, 2 ) );
                float fRatio = static_cast < float > ( lua_tonumber ( L, 3 ) );
                unsigned long ulTime = 0UL;

                if ( lua_type ( L, 4 ) == LUA_TNUMBER )
                    ulTime = static_cast < unsigned long > ( lua_tonumber ( L, 4 ) );

                if ( CStaticFunctionDefinitions::SetVehicleDoorOpenRatio ( *pEntity, ucDoor, fRatio, ulTime ) )
                {
                    lua_pushboolean ( L, true );
                    return 1;
                }
            }
            else
                m_pScriptDebugging->LogBadPointer( "setVehicleDoorOpenRatio", "vehicle", 1 );
        }
        else
            m_pScriptDebugging->LogBadType( "setVehicleDoorOpenRatio" );

        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( getVehicleDoorOpenRatio )
    {
        if ( lua_type ( L, 1 ) == LUA_TLIGHTUSERDATA &&
             lua_type ( L, 2 ) == LUA_TNUMBER )
        {
            CClientVehicle* pVehicle = lua_tovehicle ( L, 1 );
            if ( pVehicle )
            {
                unsigned char ucDoor = static_cast < unsigned char > ( lua_tonumber ( L, 2 ) );
                if ( ucDoor <= 5 )
                {
                    lua_pushnumber ( L, pVehicle->GetDoorOpenRatio ( ucDoor ) );
                    return 1;
                }
            }
            else
                m_pScriptDebugging->LogBadPointer( "getVehicleDoorOpenRatio", "vehicle", 1 );
        }
        else
            m_pScriptDebugging->LogBadType( "getVehicleDoorOpenRatio" );

        lua_pushboolean ( L, false );
        return 1;
    }
}