/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.0
*               (Shared logic for modifications)
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        mods/shared_logic/lua/CLuaFunctionDefs.Ped.cpp
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
    LUA_DECLARE( getPedVoice )
    {
        // Right type?
        if ( lua_istype ( L, 1, LUA_TLIGHTUSERDATA ) )
        {
            CClientPed* pPed = lua_toped ( L, 1 );
            if ( pPed )
            {
                if ( !pPed->IsSpeechEnabled () )
                {
                    lua_pushstring ( L, "PED_TYPE_DISABLED" );
                    return 1;
                }
                else
                {
                    const char* szVoiceType = 0;
                    const char* szVoiceBank = 0;
                    pPed->GetVoice ( &szVoiceType, &szVoiceBank );
                    if ( szVoiceType && szVoiceBank )
                    {
                        lua_pushstring ( L, szVoiceType );
                        lua_pushstring ( L, szVoiceBank );
                        return 2;
                    }
                }
            }
            else
                m_pScriptDebugging->LogBadPointer( "getPedVoice", "ped", 1 );
        }
        else
            m_pScriptDebugging->LogBadType( "getPedVoice" );

        // Failed
        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( setPedVoice )
    {
        // Right type?
        if ( lua_istype ( L, 1, LUA_TLIGHTUSERDATA ) && 
            lua_istype ( L, 2, LUA_TSTRING ) )
        {
            CClientPed* pPed = lua_toped ( L, 1 );
            const char* szVoiceType = lua_tostring ( L, 2 );
            const char* szVoiceBank = NULL;
            if ( lua_istype ( L, 3, LUA_TSTRING ) ) szVoiceBank = lua_tostring ( L, 3 );        

            if ( pPed && szVoiceType )
            {
                if ( !stricmp ( szVoiceType, "PED_TYPE_DISABLED" ) ) pPed->SetSpeechEnabled ( false );
                else if ( szVoiceBank )
                {
                    pPed->SetSpeechEnabled ( true );
                    pPed->SetVoice ( szVoiceType, szVoiceBank );
                }

                lua_pushboolean ( L, true );
                return 1;
            }
            else if ( !pPed )
                m_pScriptDebugging->LogBadPointer( "setPedVoice", "ped", 1 );
            else if ( !szVoiceType )
                m_pScriptDebugging->LogBadPointer( "setPedVoice", "voicetype", 1 );
            else if ( !szVoiceBank )
                m_pScriptDebugging->LogBadPointer( "setPedVoice", "voicebank", 1 );

        }
        else
            m_pScriptDebugging->LogBadType( "setPedVoice" );

        // Failed
        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( getPedWeapon )
    {
        // Right type?
        if ( lua_istype ( L, 1, LUA_TLIGHTUSERDATA ) )
        {
            // Grab the ped
            CClientPed* pPed = lua_toped ( L, 1 );
            if ( pPed )
            {
                // Grab the slot if specified
                unsigned char ucSlot = 0xFF;
                if ( lua_istype ( L, 2, LUA_TNUMBER ) || lua_istype ( L, 2, LUA_TSTRING ) )
                    ucSlot = ( unsigned char ) lua_tonumber ( L, 2 );

                if ( ucSlot == 0xFF )
                    ucSlot = pPed->GetCurrentWeaponSlot ();

                CWeapon* pWeapon = pPed->GetWeapon ( ( eWeaponSlot ) ucSlot );
                if ( pWeapon )
                {
                    unsigned char ucWeapon = pWeapon->GetType ();
                    lua_pushnumber ( L, ucWeapon );
                    return 1;
                }
            }
            else
                m_pScriptDebugging->LogBadPointer( "getPedWeapon", "ped", 1 );
        }
        else
            m_pScriptDebugging->LogBadType( "getPedWeapon" );

        // Failed
        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( getPedWeaponSlot )
    {
        // Right type?
        if ( lua_istype ( L, 1, LUA_TLIGHTUSERDATA ) )
        {
            // Grab the ped
            CClientPed* pPed = lua_toped ( L, 1 );
            if ( pPed )
            {
                // Grab his current slot
                int iSlot = pPed->GetCurrentWeaponSlot ();
                lua_pushnumber ( L, iSlot );
                return 1;
            }
            else
                m_pScriptDebugging->LogBadPointer( "getPedWeaponSlot", "ped", 1 );
        }
        else
            m_pScriptDebugging->LogBadType( "getPedWeaponSlot" );

        // Failed
        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( getPedAmmoInClip )
    {
        // Right types?
        if ( lua_istype ( L, 1, LUA_TLIGHTUSERDATA ) )
        {
            // Grab the ped and the slot provided
            CClientPed* pPed = lua_toped ( L, 1 );

            // Got a ped
            if ( pPed )
            {
                // Got a second argument too (slot)?
                unsigned char ucSlot = pPed->GetCurrentWeaponSlot ();
                int iArgument2 = lua_type ( L, 2 );
                if ( iArgument2 == LUA_TSTRING || iArgument2 == LUA_TNUMBER )
                {
                    ucSlot = static_cast < unsigned char > ( lua_tonumber ( L, 2 ) );
                }

                CWeapon* pWeapon = pPed->GetWeapon ( (eWeaponSlot) ucSlot );
                if ( pWeapon )
                {
                    unsigned short usAmmo = static_cast < unsigned short > ( pWeapon->GetAmmoInClip () );
                    lua_pushnumber ( L, usAmmo );
                    return 1;
                }
            }
            else
                m_pScriptDebugging->LogBadPointer( "getPedAmmoInClip", "ped", 1 );
        }
        else
            m_pScriptDebugging->LogBadType( "getPedAmmoInClip" );

        // Failed
        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( getPedTotalAmmo )
    {
        // Right types?
        if ( lua_istype ( L, 1, LUA_TLIGHTUSERDATA ) )
        {
            // Grab the ped and the slot
            CClientPed* pPed = lua_toped ( L, 1 );

            // Got the ped?
            if ( pPed )
            {
                // Got a slot argument too?
                unsigned char ucSlot = pPed->GetCurrentWeaponSlot ();
                int iArgument2 = lua_type ( L, 2 );
                if ( iArgument2 == LUA_TSTRING || iArgument2 == LUA_TNUMBER )
                {
                    ucSlot = static_cast < unsigned char > ( lua_tonumber ( L, 2 ) );
                }

                // Grab the ammo and return
                CWeapon* pWeapon = pPed->GetWeapon ( (eWeaponSlot) ucSlot );
                if ( pWeapon )
                {
                    unsigned short usAmmo = static_cast < unsigned short > ( pWeapon->GetAmmoTotal () );
                    lua_pushnumber ( L, usAmmo );
                    return 1;
                }
            }
            else
                m_pScriptDebugging->LogBadPointer( "getPedTotalAmmo", "ped", 1 );
        }
        else
            m_pScriptDebugging->LogBadType( "getPedTotalAmmo" );

        // Failed
        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( getPedWeaponMuzzlePosition )
    {
        if ( lua_type ( L, 1 ) == LUA_TLIGHTUSERDATA )
        {
            CClientPed* pPed = lua_toped ( L, 1 );
            if ( pPed )
            {
                CVector vecMuzzlePos;
                if ( CStaticFunctionDefinitions::GetPedWeaponMuzzlePosition ( *pPed, vecMuzzlePos ) )
                {
                    lua_pushnumber ( L, vecMuzzlePos.fX );
                    lua_pushnumber ( L, vecMuzzlePos.fY );
                    lua_pushnumber ( L, vecMuzzlePos.fZ );
                    return 3;
                }
            }
            else
                m_pScriptDebugging->LogBadPointer( "getPedWeaponMuzzlePosition", "ped", 1 );
        }
        else
            m_pScriptDebugging->LogBadType( "getPedWeaponMuzzlePosition" );

        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( getPedOccupiedVehicle )
    {
        // Correct type?
        if ( lua_istype ( L, 1, LUA_TLIGHTUSERDATA ) )
        {
            // Grab the ped
            CClientPed* pPed = lua_toped ( L, 1 );
            if ( pPed )
            {
                // Grab his occupied vehicle
                CClientVehicle* pVehicle = pPed->GetOccupiedVehicle ();
                if ( pVehicle )
                {                
                    lua_pushelement ( L, pVehicle );
                    return 1;
                }
            }
            else
                m_pScriptDebugging->LogBadPointer( "getPedOccupiedVehicle", "ped", 1 );
        }

        // Failed
        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( getPedTask )
    {
        // Check types
        int iArgument3 = lua_type ( L, 3 );
        if ( lua_istype ( L, 1, LUA_TLIGHTUSERDATA ) &&
            lua_istype ( L, 2, LUA_TSTRING ) &&
            ( iArgument3 == LUA_TSTRING || iArgument3 == LUA_TNUMBER ) )
        {
            // Grab the ped, the priority and the task type
            CClientPed* pPed = lua_toped ( L, 1 );
            const char* szPriority = lua_tostring ( L, 2 );
            unsigned int uiTaskType = static_cast < unsigned int > ( lua_tonumber ( L, 3 ) );

            // Valid ped?
            if ( pPed )
            {
                // Any priority specified?
                if ( szPriority && szPriority [ 0 ] )
                {
                    // Primary or secondary task grabbed?
                    bool bPrimary = false;
                    if ( ( bPrimary = !stricmp ( szPriority, "primary" ) ) ||
                        ( !stricmp ( szPriority, "secondary" ) ) )
                    {
                        // Grab the taskname list and return it
                        std::vector < SString > taskHierarchy;
                        if ( CStaticFunctionDefinitions::GetPedTask ( *pPed, bPrimary, uiTaskType, taskHierarchy ) )
                        {
                            for ( uint i = 0 ; i < taskHierarchy.size () ; i++ )
                                lua_pushstring ( L, taskHierarchy[i] );
                            return taskHierarchy.size ();
                        }
                    }
                }
            }
            else
                m_pScriptDebugging->LogBadPointer( "getPedTask", "ped", 1 );
        }
        else
            m_pScriptDebugging->LogBadType( "getPedTask" );

        // Failed
        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( getPedSimplestTask )
    {
        // Check types
        if ( lua_istype ( L, 1, LUA_TLIGHTUSERDATA ) )
        {
            // Grab the ped
            CClientPed* pPed = lua_toped ( L, 1 );
            if ( pPed )
            {
                // Grab his simplest task and return it
                char* szTaskName = CStaticFunctionDefinitions::GetPedSimplestTask ( *pPed );
                if ( szTaskName )
                {
                    lua_pushstring ( L, szTaskName );
                    return 1;
                }
            }
            else
                m_pScriptDebugging->LogBadPointer( "getPedSimplestTask", "ped", 1 );
        }
        else
            m_pScriptDebugging->LogBadType( "getPedSimplestTask" );

        // Failed
        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( isPedDoingTask )
    {
        // Check types
        if ( lua_istype ( L, 1, LUA_TLIGHTUSERDATA ) &&
            lua_istype ( L, 2, LUA_TSTRING ) )
        {
            // Grab the ped and the task name to check
            CClientPed* pPed = lua_toped ( L, 1 );
            const char* szTaskName = lua_tostring ( L, 2 );

            // Check ped
            if ( pPed )
            {
                // Check whether he's doing that task or not
                bool bIsDoingTask;
                if ( CStaticFunctionDefinitions::IsPedDoingTask ( *pPed, szTaskName, bIsDoingTask ) )
                {
                    lua_pushboolean ( L, bIsDoingTask );
                    return 1;
                }
            }
            else
                m_pScriptDebugging->LogBadPointer( "isPedDoingTask", "ped", 1 );
        }
        else
            m_pScriptDebugging->LogBadType( "isPedDoingTask" );

        // Failed
        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( getPedTarget )
    {
        // Check types
        if ( lua_istype ( L, 1, LUA_TLIGHTUSERDATA ) )
        {
            // Grab the ped
            CClientPed* pPed = lua_toped ( L, 1 );
            if ( pPed )
            {
                // Grab his target element
                CClientEntity* pEntity = CStaticFunctionDefinitions::GetPedTarget ( *pPed );
                if ( pEntity )
                {
                    lua_pushelement ( L, pEntity );
                    return 1;
                }
            }
            else
                m_pScriptDebugging->LogBadPointer( "getPedTarget", "ped", 1 );
        }
        else
            m_pScriptDebugging->LogBadType( "getPedTarget" );

        // Failed
        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( getPedTargetStart )
    {
        // Check type
        if ( lua_istype ( L, 1, LUA_TLIGHTUSERDATA ) )
        {
            // Grab the ped
            CClientPed* pPed = lua_toped ( L, 1 );
            if ( pPed )
            {
                // Grab his start aim position and return it
                CVector vecStart;
                pPed->GetShotData ( &vecStart );

                lua_pushnumber ( L, vecStart.fX );
                lua_pushnumber ( L, vecStart.fY );
                lua_pushnumber ( L, vecStart.fZ );
                return 3;
            }
            else
                m_pScriptDebugging->LogBadPointer( "getPedTargetStart", "ped", 1 );
        }
        else
            m_pScriptDebugging->LogBadType( "getPedTargetStart" );

        // Failed
        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( getPedTargetEnd )
    {
        // Check types
        if ( lua_istype ( L, 1, LUA_TLIGHTUSERDATA ) )
        {
            // Grab the ped
            CClientPed* pPed = lua_toped ( L, 1 );
            if ( pPed )
            {
                // Grab the ped end target position and return it
                CVector vecEnd;
                pPed->GetShotData ( NULL, &vecEnd );

                lua_pushnumber ( L, vecEnd.fX );
                lua_pushnumber ( L, vecEnd.fY );
                lua_pushnumber ( L, vecEnd.fZ );
                return 3;
            }
            else
                m_pScriptDebugging->LogBadPointer( "getPedTargetEnd", "ped", 1 );
        }
        else
            m_pScriptDebugging->LogBadType( "getPedTargetEnd" );

        // Failed
        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( getPedTargetRange )
    {
        // Check type
        if ( lua_istype ( L, 1, LUA_TLIGHTUSERDATA ) )
        {
            // Grab the ped
            CClientPed* pPed = lua_toped ( L, 1 );
            if ( pPed )
            {
                // TODO: getPedTargetRange
            }
            else
                m_pScriptDebugging->LogBadPointer( "getPedTargetRange", "ped", 1 );
        }
        else
            m_pScriptDebugging->LogBadType( "getPedTargetRange" );

        // Failed
        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( getPedTargetCollision )
    {
        // Check type
        if ( lua_istype ( L, 1, LUA_TLIGHTUSERDATA ) )
        {
            // Grab the ped
            CClientPed* pPed = lua_toped ( L, 1 );
            if ( pPed )
            {
                // Grab his target collision and return it
                CVector vecCollision;
                if ( CStaticFunctionDefinitions::GetPedTargetCollision ( *pPed, vecCollision ) )
                {
                    lua_pushnumber ( L, vecCollision.fX );
                    lua_pushnumber ( L, vecCollision.fY );
                    lua_pushnumber ( L, vecCollision.fZ );
                    return 3;
                }
            }
            else
                m_pScriptDebugging->LogBadPointer( "getPedTargetCollision", "ped", 1 );
        }
        else
            m_pScriptDebugging->LogBadType( "getPedTargetCollision" );

        // Failed
        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( getPedArmor )
    {
        // Check type
        if ( lua_istype ( L, 1, LUA_TLIGHTUSERDATA ) )
        {
            // Grab the ped
            CClientPed* pPed = lua_toped ( L, 1 );
            if ( pPed )
            {
                // Grab the armor and return it
                float fArmor = pPed->GetArmor ();
                lua_pushnumber ( L, fArmor );
                return 1;
            }
            else
                m_pScriptDebugging->LogBadPointer( "getPedArmor", "ped", 1 );
        }
        else
            m_pScriptDebugging->LogBadType( "getPedArmor" );

        // Failed
        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( getPedStat )
    {
        // Check the types
        int iArgument1 = lua_type ( L, 1 );
        int iArgument2 = lua_type ( L, 2 );
        if ( ( iArgument1 == LUA_TLIGHTUSERDATA ) &&
            ( iArgument2 == LUA_TSTRING || iArgument2 == LUA_TNUMBER ) )
        {
            // Grab the ped and the stat argument
            CClientPed* pPed = lua_toped ( L, 1 );
            unsigned short usStat = static_cast < unsigned short > ( lua_tonumber ( L, 2 ) );

            // Valid ped?
            if ( pPed )
            {
                // Check the stat
                if ( usStat < NUM_PLAYER_STATS )
                {
                    float fValue = pPed->GetStat ( usStat );
                    lua_pushnumber ( L, fValue );
                    return 1;
                }
            }
            else
                m_pScriptDebugging->LogBadPointer( "getPedStat", "ped", 1 );
        }
        else
            m_pScriptDebugging->LogBadType( "getPedStat" );

        // Failed
        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( isPedChoking )
    {
        // Check type
        if ( lua_istype ( L, 1, LUA_TLIGHTUSERDATA ) )
        {
            // Grab the ped and verify him
            CClientPed* pPed = lua_toped ( L, 1 );
            if ( pPed )
            {
                // Return whether he's choking or not
                lua_pushboolean ( L, pPed->IsChoking () );
                return 1;
            }
            else
                m_pScriptDebugging->LogBadPointer( "isPedChoking", "ped", 1 );
        }
        else
            m_pScriptDebugging->LogBadType( "isPedChoking" );

        // Failed
        lua_pushnil ( L );
        return 1;
    }

    LUA_DECLARE( isPedDucked )
    {
        // Check type
        if ( lua_istype ( L, 1, LUA_TLIGHTUSERDATA ) )
        {
            // Grab the ped
            CClientPed* pPed = lua_toped ( L, 1 );
            if ( pPed )
            {
                // Grab his ducked state
                bool bDucked = pPed->IsDucked ();
                lua_pushboolean ( L, bDucked );
                return 1;
            }
            else
                m_pScriptDebugging->LogBadPointer( "isPedDucked", "ped", 1 );
        }
        else
            m_pScriptDebugging->LogBadType( "isPedDucked" );

        // Failed
        lua_pushnil ( L );
        return 1;
    }

    LUA_DECLARE( isPedInVehicle )
    {
        // Check type
        if ( lua_istype ( L, 1, LUA_TLIGHTUSERDATA ) )
        {
            // Grab the ped
            CClientPed* pPed = lua_toped ( L, 1 );
            if ( pPed )
            {
                // Find out whether he's in a vehicle or not
                bool bInVehicle;
                if ( CStaticFunctionDefinitions::IsPedInVehicle ( *pPed, bInVehicle ) )
                {
                    // Return that state
                    lua_pushboolean ( L, bInVehicle );
                    return 1;
                }
            }
            else
                m_pScriptDebugging->LogBadPointer( "isPedInVehicle", "ped", 1 );
        }
        else
            m_pScriptDebugging->LogBadType( "isPedInVehicle" );

        // Failed
        lua_pushnil ( L );
        return 1;
    }

    LUA_DECLARE( doesPedHaveJetPack )
    {
        // Check type
        if ( lua_istype ( L, 1, LUA_TLIGHTUSERDATA ) )
        {
            // Grab the ped
            CClientPed* pPed = lua_toped ( L, 1 );
            if ( pPed )
            {
                // Find out whether he has a jetpack or not and return it
                bool bHasJetPack = pPed->HasJetPack ();
                lua_pushboolean ( L, bHasJetPack );
                return 1;
            }
            else
                m_pScriptDebugging->LogBadPointer( "doesPedHaveJetPack", "ped", 1 );
        }
        else
            m_pScriptDebugging->LogBadType( "doesPedHaveJetPack" );

        // Failed
        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( isPedOnGround )
    {
        // Check type
        if ( lua_istype ( L, 1, LUA_TLIGHTUSERDATA ) )
        {
            // Grab the ped
            CClientPed* pPed = lua_toped ( L, 1 );
            if ( pPed )
            {
                // Find out whether he's on the ground or not and return it
                bool bOnGround = pPed->IsOnGround ();
                lua_pushboolean ( L, bOnGround );
                return 1;
            }
            else
                m_pScriptDebugging->LogBadPointer( "isPedOnGround", "ped", 1 );
        }
        else
            m_pScriptDebugging->LogBadType( "isPedOnGround" );

        // Failed
        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( getPedContactElement )
    {
        int iArgument1 = lua_type ( L, 1 );
        if ( ( iArgument1 == LUA_TLIGHTUSERDATA ) )
        {
            CClientPed* pPed = lua_toped ( L, 1 );
            if ( pPed )
            {
                CClientEntity* pEntity = pPed->GetContactEntity ();
                if ( pEntity )
                {
                    lua_pushelement ( L, pEntity );
                    return 1;
                }
            }
            else
                m_pScriptDebugging->LogBadPointer( "getPedContactElement", "ped", 1 );
        }
        else
            m_pScriptDebugging->LogBadType( "getPedContactElement" );

        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( getPedRotation )
    {
        int iArgument1 = lua_type ( L, 1 );
        if ( ( iArgument1 == LUA_TLIGHTUSERDATA ) )
        {
            CClientPed* pPed = lua_toped ( L, 1 );
            if ( pPed )
            {
                float fRotation = ConvertRadiansToDegrees ( pPed->GetCurrentRotation () );
                lua_pushnumber ( L, fRotation );
                return 1;
            }
            else
                m_pScriptDebugging->LogBadPointer( "getPedRotation", "ped", 1 );
        }
        else
            m_pScriptDebugging->LogBadType( "getPedRotation" );

        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( canPedBeKnockedOffBike )
    {
        int iArgument1 = lua_type ( L, 1 );
        if ( ( iArgument1 == LUA_TLIGHTUSERDATA ) )
        {
            CClientPed* pPed = lua_toped ( L, 1 );
            if ( pPed )
            {
                bool bCanBeKnockedOffBike = pPed->GetCanBeKnockedOffBike ();
                lua_pushboolean ( L, bCanBeKnockedOffBike );
                return 1;
            }
            else
                m_pScriptDebugging->LogBadPointer( "canPedBeKnockedOffBike", "ped", 1 );
        }
        else
            m_pScriptDebugging->LogBadType( "canPedBeKnockedOffBike" );

        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( getPedBonePosition )
    {
        int iArgument2 = lua_type ( L, 2 );
        if ( lua_type ( L, 1 ) == LUA_TLIGHTUSERDATA &&
            ( iArgument2 == LUA_TNUMBER || iArgument2 == LUA_TSTRING ) )
        {
            CClientPed* pPed = lua_toped ( L, 1 );
            if ( pPed )
            {
                unsigned char ucBone = static_cast < unsigned char > ( lua_tonumber ( L, 2 ) );
                if ( ucBone <= BONE_RIGHTFOOT )
                {
                    eBone bone = ( eBone ) ucBone;
                    CVector vecPosition;
                    if ( CStaticFunctionDefinitions::GetPedBonePosition ( *pPed, bone, vecPosition ) )
                    {
                        lua_pushnumber ( L, vecPosition.fX );
                        lua_pushnumber ( L, vecPosition.fY );
                        lua_pushnumber ( L, vecPosition.fZ );
                        return 3;
                    }
                }
            }
            else
                m_pScriptDebugging->LogBadPointer( "getPedBonePosition", "ped", 1 );
        }
        else
            m_pScriptDebugging->LogBadType( "getPedBonePosition" );

        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( setPedWeaponSlot )
    {
        // Check type
        if ( lua_istype ( L, 1, LUA_TLIGHTUSERDATA ) &&
            lua_istype ( L, 2, LUA_TNUMBER ) )
        {
            // Grab the arguments
            CClientEntity* pElement = lua_toelement ( L, 1 );
            int iSlot = static_cast < int > ( lua_tonumber ( L, 2 ) );

            // Valid ped?
            if ( pElement )
            {
                // Valid slot?
                if ( iSlot >= 0 )
                {
                    // Set his slot
                    if ( CStaticFunctionDefinitions::SetPedWeaponSlot ( *pElement, iSlot ) )
                    {
                        lua_pushboolean ( L, true );
                        return 1;
                    }
                }
            }
            else
                m_pScriptDebugging->LogBadPointer( "setPedWeaponSlot", "ped", 1 );
        }
        else
            m_pScriptDebugging->LogBadType( "setPedWeaponSlot" );

        // Failed
        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( getPedClothes )
    {
        int iArgument2 = lua_type ( L, 2 );
        if ( ( lua_type ( L, 1 ) == LUA_TLIGHTUSERDATA ) &&
            ( iArgument2 == LUA_TNUMBER || iArgument2 == LUA_TSTRING ) )
        {
            CClientPed * pPed = lua_toped ( L, 1 );
            unsigned char ucType = static_cast < unsigned char > ( lua_tonumber ( L, 2 ) );
            if ( pPed )
            {
                char szTexture [ 128 ], szModel [ 128 ];
                if ( CStaticFunctionDefinitions::GetPedClothes ( *pPed, ucType, szTexture, szModel ) )
                {
                    lua_pushstring ( L, szTexture );
                    lua_pushstring ( L, szModel );
                    return 2;
                }
            }
            else
                m_pScriptDebugging->LogBadPointer( "getPedClothes", "player", 1 );
        }
        else
            m_pScriptDebugging->LogBadType( "getPedClothes" );

        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( getPedControlState )
    {
        if ( ( lua_type ( L, 1 ) == LUA_TLIGHTUSERDATA ) &&
            ( lua_type ( L, 2 ) == LUA_TSTRING ) )
        {
            CClientPed * pPed = lua_toped ( L, 1 );
            const char * szControl = lua_tostring ( L, 2 );
            if ( pPed )
            {
                bool bState;
                if ( CStaticFunctionDefinitions::GetPedControlState ( *pPed, szControl, bState ) )
                {
                    lua_pushboolean ( L, bState );
                    return 1;
                }
            }
            else
                m_pScriptDebugging->LogBadPointer( "getPedControlState", "ped", 1 );
        }
        else
            m_pScriptDebugging->LogBadType( "getPedControlState" );

        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( isPedDoingGangDriveby )
    {
        if ( ( lua_type ( L, 1 ) == LUA_TLIGHTUSERDATA ) )
        {
            CClientPed * pPed = lua_toped ( L, 1 );
            if ( pPed )
            {
                bool bDoingGangDriveby;
                if ( CStaticFunctionDefinitions::IsPedDoingGangDriveby ( *pPed, bDoingGangDriveby ) )
                {
                    lua_pushboolean ( L, bDoingGangDriveby );
                    return 1;
                }
            }
            else
                m_pScriptDebugging->LogBadPointer( "isPedDoingGangDriveby", "ped", 1 );
        }
        else
            m_pScriptDebugging->LogBadType( "isPedDoingGangDriveby" );

        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( getPedAnimation )
    {
        if ( ( lua_type ( L, 1 ) == LUA_TLIGHTUSERDATA ) )
        {
            CClientPed * pPed = lua_toped ( L, 1 );
            if ( pPed )
            {
                char szBlockName [ 128 ], szAnimName [ 128 ];
                if ( CStaticFunctionDefinitions::GetPedAnimation ( *pPed, szBlockName, szAnimName, 128 ))
                {
                    lua_pushstring ( L, szBlockName );
                    lua_pushstring ( L, szAnimName );
                    return 2;
                }
            }
            else
                m_pScriptDebugging->LogBadPointer( "getPedAnimation", "ped", 1 );
        }
        else
            m_pScriptDebugging->LogBadType( "getPedAnimation" );

        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( getPedMoveState )
    {
        if ( ( lua_type ( L, 1 ) == LUA_TLIGHTUSERDATA ) )
        {
            CClientPed * pPed = lua_toped ( L, 1 );
            if ( pPed )
            {
                std::string strMoveState;
                if ( CStaticFunctionDefinitions::GetPedMoveState ( *pPed, strMoveState ) )
                {
                    lua_pushstring ( L, strMoveState.c_str() );
                    return 1;
                }
            }
            else
                m_pScriptDebugging->LogBadPointer( "GetPedMoveState", "ped", 1 );
        }
        else
            m_pScriptDebugging->LogBadType( "GetPedMoveState" );

        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( getPedMoveAnim )
    {
        if ( ( lua_type ( L, 1 ) == LUA_TLIGHTUSERDATA ) )
        {
            CClientPed * pPed = lua_toped ( L, 1 );
            if ( pPed )
            {
                unsigned int iMoveAnim;
                if ( CStaticFunctionDefinitions::GetPedMoveAnim ( *pPed, iMoveAnim ) )
                {
                    lua_pushnumber ( L, iMoveAnim );
                    return 1;
                }
            }
            else
                m_pScriptDebugging->LogBadPointer( "getPedWalkingStyle", "ped", 1 );
        }
        else
            m_pScriptDebugging->LogBadType( "getPedWalkingStyle" );

        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( isPedHeadless )
    {
        if ( ( lua_type ( L, 1 ) == LUA_TLIGHTUSERDATA ) )
        {
            CClientPed * pPed = lua_toped ( L, 1 );
            if ( pPed )
            {
                bool bHeadless;
                if ( CStaticFunctionDefinitions::IsPedHeadless ( *pPed, bHeadless ) )
                {
                    lua_pushboolean ( L, bHeadless );
                    return 1;
                }
            }
            else
                m_pScriptDebugging->LogBadPointer( "isPedHeadless", "ped", 1 );
        }
        else
            m_pScriptDebugging->LogBadType( "isPedHeadless" );

        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( isPedFrozen )
    {
        if ( ( lua_type ( L, 1 ) == LUA_TLIGHTUSERDATA ) )
        {
            CClientPed * pPed = lua_toped ( L, 1 );
            if ( pPed )
            {
                bool bFrozen;
                if ( CStaticFunctionDefinitions::IsPedFrozen ( *pPed, bFrozen ) )
                {
                    lua_pushboolean ( L, bFrozen );
                    return 1;
                }
            }
            else
                m_pScriptDebugging->LogBadPointer( "isPedFrozen", "ped", 1 );
        }
        else
            m_pScriptDebugging->LogBadType( "isPedFrozen" );

        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( isPedFootBloodEnabled )
    {
        if ( ( lua_type ( L, 1 ) == LUA_TLIGHTUSERDATA ) )
        {
            CClientPed * pPed = lua_toped ( L, 1 );
            if ( pPed )
            {
                bool bHasFootBlood = false;
                if ( CStaticFunctionDefinitions::IsPedFootBloodEnabled ( *pPed, bHasFootBlood ) )
                {
                    lua_pushboolean ( L, bHasFootBlood );
                    return 1;
                }
            }
            else
                m_pScriptDebugging->LogBadPointer( "isPedFootBloodEnabled", "ped", 1 );
        }
        else
            m_pScriptDebugging->LogBadType( "isPedFootBloodEnabled" );

        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( getPedCameraRotation )
    {
        if ( ( lua_type ( L, 1 ) == LUA_TLIGHTUSERDATA ) )
        {
            CClientPed * pPed = lua_toped ( L, 1 );
            if ( pPed )
            {
                float fRotation = 0.0f;
                if ( CStaticFunctionDefinitions::GetPedCameraRotation ( *pPed, fRotation ) )
                {
                    lua_pushnumber ( L, fRotation );
                    return 1;
                }
            }
            else
                m_pScriptDebugging->LogBadPointer( "getPedCameraRotation", "ped", 1 );
        }
        else
            m_pScriptDebugging->LogBadType( "getPedCameraRotation" );

        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( isPedOnFire )
    {
        if ( ( lua_type ( L, 1 ) == LUA_TLIGHTUSERDATA ) )
        {
            CClientPed * pPed = lua_toped ( L, 1 );
            if ( pPed )
            {
                bool bOnFire;
                if ( CStaticFunctionDefinitions::IsPedOnFire ( *pPed, bOnFire ) )
                {
                    lua_pushboolean ( L, bOnFire );
                    return 1;
                }
            }
            else
                m_pScriptDebugging->LogBadPointer( "isPedOnFire", "ped", 1 );
        }
        else
            m_pScriptDebugging->LogBadType( "isPedOnFire" );

        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( setPedOnFire )
    {
        if ( ( lua_type ( L, 1 ) == LUA_TLIGHTUSERDATA ) )
        {
            CClientEntity* pEntity = lua_toelement ( L, 1 );
            bool bOnFire = ( ( lua_toboolean ( L, 2 ) ) ? true:false );

            if ( pEntity )
            {
                if ( CStaticFunctionDefinitions::SetPedOnFire ( *pEntity, bOnFire ) )
                {
                    lua_pushboolean ( L, true );
                    return 1;
                }
            }
            else
                m_pScriptDebugging->LogBadPointer( "setPedOnFire", "ped", 1 );
        }
        else
            m_pScriptDebugging->LogBadType( "setPedOnFire" );

        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( addPedClothes )
    {
        int iArgument4 = lua_type ( L, 4 );
        if ( ( lua_type ( L, 1 ) == LUA_TLIGHTUSERDATA ) &&
            ( lua_type ( L, 2 ) == LUA_TSTRING ) &&
            ( lua_type ( L, 3 ) == LUA_TSTRING ) &&
            ( iArgument4 == LUA_TNUMBER || iArgument4 == LUA_TSTRING ) )
        {
            CClientEntity* pEntity = lua_toelement ( L, 1 );
            const char* szTexture = lua_tostring ( L, 2 );
            const char* szModel = lua_tostring ( L, 3 );
            unsigned char ucType = static_cast < unsigned char > ( lua_tonumber ( L, 4 ) );

            if ( pEntity )
            {
                if ( CStaticFunctionDefinitions::AddPedClothes ( *pEntity, const_cast < char* > ( szTexture ), const_cast < char* > ( szModel ), ucType ) )
                {
                    lua_pushboolean ( L, true );
                    return 1;
                }
            }
            else
                m_pScriptDebugging->LogBadPointer( "addPedClothes", "ped", 1 );
        }
        else
            m_pScriptDebugging->LogBadType( "addPedClothes" );

        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( removePedClothes )
    {
        int iArgument2 = lua_type ( L, 2 );
        if ( ( lua_type ( L, 1 ) == LUA_TLIGHTUSERDATA ) &&
            ( iArgument2 == LUA_TNUMBER || iArgument2 == LUA_TSTRING ) )
        {
            CClientEntity* pEntity = lua_toelement ( L, 1 );
            unsigned char ucType = static_cast < unsigned char > ( lua_tonumber ( L, 2 ) );

            if ( pEntity )
            {
                if ( CStaticFunctionDefinitions::RemovePedClothes ( *pEntity, ucType ) )
                {
                    lua_pushboolean ( L, true );
                    return 1;
                }
            }
            else
                m_pScriptDebugging->LogBadPointer( "removePedClothes", "ped", 1 );
        }
        else
            m_pScriptDebugging->LogBadType( "removePedClothes" );

        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( setPedControlState )
    {
        if ( ( lua_type ( L, 1 ) == LUA_TLIGHTUSERDATA ) &&
            ( lua_type ( L, 2 ) == LUA_TSTRING ) &&
            ( lua_type ( L, 3 ) == LUA_TBOOLEAN ) )
        {
            CClientEntity* pEntity = lua_toelement ( L, 1 );
            const char * szControl = lua_tostring ( L, 2 );
            bool bState = ( lua_toboolean ( L, 3 ) ) ? true:false;
            if ( pEntity )
            {
                if ( CStaticFunctionDefinitions::SetPedControlState ( *pEntity, szControl, bState ) )
                {
                    lua_pushboolean ( L, true );
                    return 1;
                }
            }
            else
                m_pScriptDebugging->LogBadPointer( "setPedControlState", "ped", 1 );
        }
        else
            m_pScriptDebugging->LogBadType( "setPedControlState" );

        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( setPedDoingGangDriveby )
    {
        if ( ( lua_type ( L, 1 ) == LUA_TLIGHTUSERDATA ) &&
            ( lua_type ( L, 2 ) == LUA_TBOOLEAN ) )
        {
            CClientEntity* pEntity = lua_toelement ( L, 1 );
            bool bDoingGangDriveby = ( lua_toboolean ( L, 2 ) ) ? true:false;
            if ( pEntity )
            {
                if ( CStaticFunctionDefinitions::SetPedDoingGangDriveby ( *pEntity, bDoingGangDriveby ) )
                {
                    lua_pushboolean ( L, true );
                    return 1;
                }
            }
            else
                m_pScriptDebugging->LogBadPointer( "setPedDoingGangDriveby", "ped", 1 );
        }
        else
            m_pScriptDebugging->LogBadType( "setPedDoingGangDriveby" );

        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( setPedLookAt )
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
            CVector vecPosition;
            vecPosition.fX = static_cast < float > ( lua_tonumber ( L, 2 ) );
            vecPosition.fY = static_cast < float > ( lua_tonumber ( L, 3 ) );
            vecPosition.fZ = static_cast < float > ( lua_tonumber ( L, 4 ) );
            int iTime = 3000;
            int iBlend = 1000;
            CClientEntity * pTarget = NULL;

            int iArgument5 = lua_type ( L, 5 );
            int iArgument6 = lua_type ( L, 6 );
            if ( iArgument5 == LUA_TNUMBER || iArgument5 == LUA_TSTRING )
            {
                iTime = static_cast < int > ( lua_tonumber ( L, 5 ) );
                if ( lua_type ( L, 6 ) == LUA_TLIGHTUSERDATA )
                    pTarget = lua_toelement ( L, 6 );
                else if ( iArgument6 == LUA_TNUMBER || iArgument6 == LUA_TSTRING )
                {
                    iBlend = static_cast < int > ( lua_tonumber ( L, 6 ) );
                    if ( lua_type ( L, 7 ) == LUA_TLIGHTUSERDATA )
                        pTarget = lua_toelement ( L, 7 );
                }
            }

            if ( pEntity )
            {
                if ( CStaticFunctionDefinitions::SetPedLookAt ( *pEntity, vecPosition, iTime, iBlend, pTarget ) )
                {
                    lua_pushboolean ( L, true );
                    return 1;
                }
            }
            else
                m_pScriptDebugging->LogBadPointer( "setPedLookAt", "ped", 1 );
        }
        else
            m_pScriptDebugging->LogBadType( "setPedLookAt" );

        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( setPedHeadless )
    {
        if ( ( lua_type ( L, 1 ) == LUA_TLIGHTUSERDATA ) &&
            ( lua_type ( L, 2 ) == LUA_TBOOLEAN ) )
        {
            CClientEntity* pEntity = lua_toelement ( L, 1 );
            bool bHeadless = ( lua_toboolean ( L, 2 ) ) ? true:false;
            if ( pEntity )
            {
                if ( CStaticFunctionDefinitions::SetPedHeadless ( *pEntity, bHeadless ) )
                {
                    lua_pushboolean ( L, true );
                    return 1;
                }
            }
            else
                m_pScriptDebugging->LogBadPointer( "setPedHeadless", "ped", 1 );
        }
        else
            m_pScriptDebugging->LogBadType( "setPedHeadless" );

        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( setPedFrozen )
    {
        if ( ( lua_type ( L, 1 ) == LUA_TLIGHTUSERDATA ) &&
            ( lua_type ( L, 2 ) == LUA_TBOOLEAN ) )
        {
            CClientEntity* pEntity = lua_toelement ( L, 1 );
            bool bFrozen = ( lua_toboolean ( L, 2 ) ) ? true:false;
            if ( pEntity )
            {
                if ( CStaticFunctionDefinitions::SetPedFrozen ( *pEntity, bFrozen ) )
                {
                    lua_pushboolean ( L, true );
                    return 1;
                }
            }
            else
                m_pScriptDebugging->LogBadPointer( "setPedFrozen", "ped", 1 );
        }
        else
            m_pScriptDebugging->LogBadType( "setPedFrozen" );

        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( setPedFootBloodEnabled )
    {
        if ( ( lua_type ( L, 1 ) == LUA_TLIGHTUSERDATA ) &&
             ( lua_type ( L, 2 ) == LUA_TBOOLEAN ) )
        {
            CClientEntity* pEntity = lua_toelement ( L, 1 );
            if ( pEntity )
            {
                bool bHasFootBlood = lua_toboolean ( L, 2 ) ? true : false;
                if ( CStaticFunctionDefinitions::SetPedFootBloodEnabled ( *pEntity, bHasFootBlood ) )
                {
                    lua_pushboolean ( L, true );
                    return 1;
                }
            }
            else
                m_pScriptDebugging->LogBadPointer( "setPedFootBloodEnabled", "ped", 1 );
        }
        else
            m_pScriptDebugging->LogBadType( "setPedFootBloodEnabled" );

        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( setPedCameraRotation )
    {
        int iArgument2 = lua_type ( L, 2 );
        if ( ( lua_type ( L, 1 ) == LUA_TLIGHTUSERDATA ) &&
            ( iArgument2 == LUA_TNUMBER || iArgument2 == LUA_TSTRING ) )
        {
            CClientEntity* pEntity = lua_toelement ( L, 1 );
            float fRotation = static_cast < float > ( lua_tonumber ( L, 2 ) );
            if ( pEntity )
            {
                if ( CStaticFunctionDefinitions::SetPedCameraRotation ( *pEntity, fRotation ) )
                {
                    lua_pushboolean ( L, true );
                    return 1;
                }
            }
            else
                m_pScriptDebugging->LogBadPointer( "setPedCameraRotation", "ped", 1 );
        }
        else
            m_pScriptDebugging->LogBadType( "setPedCameraRotation" );

        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( setPedAimTarget )
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
                CVector vecTarget;
                vecTarget.fX = static_cast < float > ( lua_tonumber ( L, 2 ) );
                vecTarget.fY = static_cast < float > ( lua_tonumber ( L, 3 ) );
                vecTarget.fZ = static_cast < float > ( lua_tonumber ( L, 4 ) );

                if ( CStaticFunctionDefinitions::SetPedAimTarget ( *pEntity, vecTarget ) )
                {
                    lua_pushboolean ( L, true );
                    return 1;
                }
            }
            else
                m_pScriptDebugging->LogBadPointer( "setPedAimTarget", "ped", 1 );
        }
        else
            m_pScriptDebugging->LogBadType( "setPedAimTarget" );

        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( setPedRotation )
    {
        // Check types
        if ( lua_istype ( L, 1, LUA_TLIGHTUSERDATA ) &&
            lua_istype ( L, 2, LUA_TNUMBER ) )
        {
            // Grab the entity
            CClientEntity* pEntity = lua_toelement ( L, 1 );
            float fRotation = static_cast < float > ( lua_tonumber ( L, 2 ) );

            // Valid element?
            if ( pEntity )
            {
                // Set the new rotation
                if ( CStaticFunctionDefinitions::SetPedRotation ( *pEntity, fRotation ) )
                {
                    lua_pushboolean ( L, true );
                    return 1;
                }
            }
            else
                m_pScriptDebugging->LogBadPointer( "setPedRotation", "ped", 1 );
        }
        else
            m_pScriptDebugging->LogBadType( "setPedRotation" );

        // Failed
        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( setPedCanBeKnockedOffBike )
    {
        // Check types
        if ( lua_istype ( L, 1, LUA_TLIGHTUSERDATA ) &&
            lua_istype ( L, 2, LUA_TBOOLEAN ) )
        {
            // Grab the entity
            CClientEntity* pEntity = lua_toelement ( L, 1 );
            bool bCanBeKnockedOffBike = ( lua_toboolean ( L, 2 ) ) ? true:false;

            // Valid element?
            if ( pEntity )
            {
                // Set the new rotation
                if ( CStaticFunctionDefinitions::SetPedCanBeKnockedOffBike ( *pEntity, bCanBeKnockedOffBike ) )
                {
                    lua_pushboolean ( L, true );
                    return 1;
                }
            }
            else
                m_pScriptDebugging->LogBadPointer( "setPedCanBeKnockedOffBike", "element", 1 );
        }
        else
            m_pScriptDebugging->LogBadType( "setPedCanBeKnockedOffBike" );

        // Failed
        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( setPedAnimation )
    {
        // Check types
        if ( lua_istype ( L, 1, LUA_TLIGHTUSERDATA ) )
        {
            // Grab the element
            CClientEntity* pEntity = lua_toelement ( L, 1 );
            if ( pEntity )
            {
                const char * szBlockName = NULL;
                const char * szAnimName = NULL;
                int iTime = -1;
                bool bLoop = true;
                bool bUpdatePosition = true;
                bool bInterruptable = true;
                bool bFreezeLastFrame = true;
                if ( lua_type ( L, 2 ) == LUA_TSTRING ) szBlockName = lua_tostring ( L, 2 );
                if ( lua_type ( L, 3 ) == LUA_TSTRING ) szAnimName = lua_tostring ( L, 3 );
                int iArgument4 = lua_type ( L, 4 );
                if ( iArgument4 == LUA_TNUMBER || iArgument4 == LUA_TSTRING )
                    iTime = static_cast < int > ( lua_tonumber ( L, 4 ) );
                if ( lua_type ( L, 5 ) == LUA_TBOOLEAN )
                    bLoop = ( lua_toboolean ( L, 5 ) ) ? true:false;
                if ( lua_type ( L, 6 ) == LUA_TBOOLEAN )
                    bUpdatePosition = ( lua_toboolean ( L, 6 ) ) ? true:false;
                if ( lua_type ( L, 7 ) == LUA_TBOOLEAN )
                    bInterruptable = ( lua_toboolean ( L, 7 ) ) ? true:false;
                if ( lua_type ( L, 8 ) == LUA_TBOOLEAN )
                    bFreezeLastFrame = ( lua_toboolean ( L, 8 ) ) ? true:false;

                if ( CStaticFunctionDefinitions::SetPedAnimation ( *pEntity, szBlockName, szAnimName, iTime, bLoop, bUpdatePosition, bInterruptable, bFreezeLastFrame ) )
                {
                    lua_pushboolean ( L, true );
                    return 1;
                }
            }
            else
                m_pScriptDebugging->LogBadPointer( "setPedAnimation", "element", 1 );
        }
        else
            m_pScriptDebugging->LogBadType( "setPedAnimation" );

        // Failed
        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( setPedAnimationProgress )
    {
        // Check types
        if ( lua_istype ( L, 1, LUA_TLIGHTUSERDATA ) )
        {
            // Grab the element
            CClientEntity* pEntity = lua_toelement ( L, 1 );
            if ( pEntity )
            {
                const char * szAnimName = NULL;
                float fProgress = 0.0f;
                if ( lua_type ( L, 2 ) == LUA_TSTRING ) szAnimName = lua_tostring ( L, 2 );
                if ( lua_type ( L, 3 ) == LUA_TNUMBER ) fProgress = static_cast < float > ( lua_tonumber ( L, 3 ) );

                if ( CStaticFunctionDefinitions::SetPedAnimationProgress ( *pEntity, szAnimName, fProgress ) )
                {
                    lua_pushboolean ( L, true );
                    return 1;
                }
            }
            else
                m_pScriptDebugging->LogBadPointer( "setPedAnimationProgress", "element", 1 );
        }
        else
            m_pScriptDebugging->LogBadType( "setPedAnimationProgress" );

        // Failed
        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( setPedMoveAnim )
    {
        // Check types
        if ( ( lua_type ( L, 1 ) == LUA_TLIGHTUSERDATA ) &&
            ( lua_type ( L, 2 ) == LUA_TNUMBER ) )
        {
            // Grab the element
            CClientEntity* pEntity = lua_toelement ( L, 1 );
            if ( pEntity )
            {
                unsigned int iMoveAnim = static_cast < unsigned int > ( lua_tonumber ( L, 2 ) );

                if ( CStaticFunctionDefinitions::SetPedMoveAnim ( *pEntity, iMoveAnim ) )
                {
                    lua_pushboolean ( L, true );
                    return 1;
                }
            }
            else
                m_pScriptDebugging->LogBadPointer( "setPedWalkingStyle", "element", 1 );
        }
        else
            m_pScriptDebugging->LogBadType( "setPedWalkingStyle" );

        // Failed
        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( createPed )
    {
        // Valid types?
        if ( ( lua_type ( L, 1 ) == LUA_TNUMBER || lua_type ( L, 1 ) == LUA_TSTRING ) &&
            ( lua_type ( L, 2 ) == LUA_TNUMBER || lua_type ( L, 2 ) == LUA_TSTRING ) &&
            ( lua_type ( L, 3 ) == LUA_TNUMBER || lua_type ( L, 3 ) == LUA_TSTRING ) &&
            ( lua_type ( L, 4 ) == LUA_TNUMBER || lua_type ( L, 4 ) == LUA_TSTRING ) )
        {
            // Grab arguments
            CVector vecPosition;
            unsigned long ulModel = static_cast < unsigned long > ( lua_tonumber ( L, 1 ) );
            vecPosition.fX = static_cast < float > ( lua_tonumber ( L, 2 ) );
            vecPosition.fY = static_cast < float > ( lua_tonumber ( L, 3 ) );
            vecPosition.fZ = static_cast < float > ( lua_tonumber ( L, 4 ) );

            float fRotation = 0.0f;
            if ( lua_type ( L, 5 ) == LUA_TNUMBER || lua_type ( L, 5 ) == LUA_TSTRING )
            {
                fRotation = static_cast < float > ( lua_tonumber ( L, 5 ) );
            }

            CLuaMain* pLuaMain = lua_readcontext( L );
            CResource * pResource = pLuaMain->GetResource();
            // Create it
            CClientPed* pPed = CStaticFunctionDefinitions::CreatePed ( *pResource, ulModel, vecPosition, fRotation );
            if ( pPed )
            {
                // Return it
                lua_pushelement ( L, pPed );
                return 1;
            }
        }

        // Failed
        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( detonateSatchels )
    {
        if ( CStaticFunctionDefinitions::DetonateSatchels() )
        {
            lua_pushboolean( L, true );
            return 1;
        }
        lua_pushboolean ( L, false );
        return 1;
    }
}