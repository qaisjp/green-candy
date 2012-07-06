/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*               (Shared logic for modifications)
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        mods/shared_logic/lua/CLuaFunctionDefs.Audio.cpp
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
*               The_GTA <quiret@gmx.de>
*
*****************************************************************************/

#include "StdInc.h"

namespace CLuaFunctionDefs
{
    LUA_DECLARE( playSound )
    {
        if ( lua_istype ( L, 1, LUA_TSTRING ) )
        {
            SString strSound = lua_tostring ( L, 1 );
            CLuaMain * luaMain = m_pLuaManager->GetVirtualMachine ( L );
            if ( luaMain )
            {
                CResource* pResource = luaMain->GetResource();
                if ( pResource )
                {
                    SString strFilename;
                    bool bIsURL = false;
                    if ( CResourceManager::ParseResourcePathInput( strSound, pResource, strFilename ) )
                        strSound = strFilename;
                    else
                        bIsURL = true;

                    bool bLoop = false;
                    if ( lua_istype ( L, 2, LUA_TBOOLEAN ) )
                    {
                        bLoop = ( lua_toboolean ( L, 2 ) ) ? true : false;
                    }

                    CClientSound* pSound = CStaticFunctionDefinitions::PlaySound ( pResource, strSound, bIsURL, bLoop );
                    if ( pSound )
                    {
                        lua_pushelement ( L, pSound );
                        return 1;
                    }
                }
            }
        }
        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( playSound3D )
    {
        int iArgument2 = lua_type ( L, 2 );
        int iArgument3 = lua_type ( L, 3 );
        int iArgument4 = lua_type ( L, 4 );
        if ( ( lua_istype ( L, 1, LUA_TSTRING ) ) &&
            ( iArgument2 == LUA_TNUMBER || iArgument2 == LUA_TSTRING ) &&
            ( iArgument3 == LUA_TNUMBER || iArgument3 == LUA_TSTRING ) &&
            ( iArgument4 == LUA_TNUMBER || iArgument4 == LUA_TSTRING ) )
        {
            CVector vecPosition ( static_cast < float > ( lua_tonumber ( L, 2 ) ),
                static_cast < float > ( lua_tonumber ( L, 3 ) ),
                static_cast < float > ( lua_tonumber ( L, 4 ) ) );

            SString strSound = lua_tostring ( L, 1 );

            CLuaMain * luaMain = m_pLuaManager->GetVirtualMachine ( L );
            if ( luaMain )
            {
                CResource* pResource = luaMain->GetResource();
                if ( pResource )
                {
                    SString strFilename;
                    bool bIsURL = false;
                    if ( CResourceManager::ParseResourcePathInput( strSound, pResource, strFilename ) )
                        strSound = strFilename;
                    else
                        bIsURL = true;

                    bool bLoop = false;
                    if ( lua_istype ( L, 5, LUA_TBOOLEAN ) )
                    {
                        bLoop = ( lua_toboolean ( L, 5 ) ) ? true : false;
                    }

                    CClientSound* pSound = CStaticFunctionDefinitions::PlaySound3D ( pResource, strSound, bIsURL, vecPosition, bLoop );
                    if ( pSound )
                    {
                        lua_pushelement ( L, pSound );
                        return 1;
                    }
                }
            }
        }
        lua_pushboolean ( L, false );
        return 1;
    }


    LUA_DECLARE( stopSound )
    {
        if ( lua_istype ( L, 1, LUA_TLIGHTUSERDATA ) )
        {
            CClientSound* pSound = lua_tosound ( L, 1 );
            if ( pSound )
            {
                if ( CStaticFunctionDefinitions::StopSound ( *pSound ) )
                {
                    lua_pushboolean ( L, true );
                    return 1;
                }
            }
        }
        lua_pushboolean ( L, false );
        return 1;
    }


    LUA_DECLARE( setSoundPosition )
    {
        if ( lua_istype ( L, 1, LUA_TLIGHTUSERDATA ) &&
            lua_istype ( L, 2, LUA_TNUMBER ) )
        {
            CClientSound* pSound = lua_tosound ( L, 1 );
            if ( pSound )
            {
                double dPosition = lua_tonumber ( L, 2 );
                if ( CStaticFunctionDefinitions::SetSoundPosition ( *pSound, dPosition ) )
                {
                    lua_pushboolean ( L, true );
                    return 1;
                }
            }
        }
        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( getSoundPosition )
    {
        if ( lua_istype ( L, 1, LUA_TLIGHTUSERDATA ) )
        {
            CClientSound* pSound = lua_tosound ( L, 1 );
            if ( pSound )
            {
                double dPosition = 0;
                if ( CStaticFunctionDefinitions::GetSoundPosition ( *pSound, dPosition ) )
                {
                    lua_pushnumber ( L, dPosition );
                    return 1;
                }
            }
        }
        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( getSoundLength )
    {
        if ( lua_istype ( L, 1, LUA_TLIGHTUSERDATA ) )
        {
            CClientSound* pSound = lua_tosound ( L, 1 );
            if ( pSound )
            {
                double dLength = 0;
                if ( CStaticFunctionDefinitions::GetSoundLength ( *pSound, dLength ) )
                {
                    lua_pushnumber ( L, dLength );
                    return 1;
                }
            }
        }
        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( setSoundPaused )
    {
        if ( lua_istype ( L, 1, LUA_TLIGHTUSERDATA ) &&
            lua_istype ( L, 2, LUA_TBOOLEAN ) )
        {
            CClientSound* pSound = lua_tosound ( L, 1 );
            if ( pSound )
            {
                bool bPaused = ( lua_toboolean ( L, 2 ) ) ? true : false;
                if ( CStaticFunctionDefinitions::SetSoundPaused ( *pSound, bPaused ) )
                {
                    lua_pushboolean ( L, true );
                    return 1;
                }
            }
        }
        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( isSoundPaused )
    {
        if ( lua_istype ( L, 1, LUA_TLIGHTUSERDATA ) )
        {
            CClientSound* pSound = lua_tosound ( L, 1 );
            if ( pSound )
            {
                bool bPaused = false;
                if ( CStaticFunctionDefinitions::IsSoundPaused ( *pSound, bPaused ) )
                {
                    lua_pushboolean ( L, bPaused );
                    return 1;
                }
            }
        }
        lua_pushboolean ( L, false );
        return 1;
    }


    LUA_DECLARE( setSoundVolume )
    {
        if ( lua_istype ( L, 1, LUA_TLIGHTUSERDATA ) &&
            lua_istype ( L, 2, LUA_TNUMBER ) )
        {
            CClientSound* pSound = lua_tosound ( L, 1 );
            if ( pSound )
            {
                float fVolume = ( float ) lua_tonumber ( L, 2 );
                if ( CStaticFunctionDefinitions::SetSoundVolume ( *pSound, fVolume ) )
                {
                    lua_pushboolean ( L, true );
                    return 1;
                }
            }
        }
        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( getSoundVolume )
    {
        if ( lua_istype ( L, 1, LUA_TLIGHTUSERDATA ) )
        {
            CClientSound* pSound = lua_tosound ( L, 1 );
            if ( pSound )
            {
                float fVolume = 0.0f;
                if ( CStaticFunctionDefinitions::GetSoundVolume ( *pSound, fVolume ) )
                {
                    lua_pushnumber ( L, fVolume );
                    return 1;
                }
            }
        }
        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( setSoundSpeed )
    {
        if ( lua_istype ( L, 1, LUA_TLIGHTUSERDATA ) &&
            lua_istype ( L, 2, LUA_TNUMBER ) )
        {
            CClientSound* pSound = lua_tosound ( L, 1 );
            if ( pSound )
            {
                float fSpeed = ( float ) lua_tonumber ( L, 2 );
                if ( CStaticFunctionDefinitions::SetSoundSpeed ( *pSound, fSpeed ) )
                {
                    lua_pushboolean ( L, true );
                    return 1;
                }
            }
        }
        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( getSoundSpeed )
    {
        if ( lua_istype ( L, 1, LUA_TLIGHTUSERDATA ) )
        {
            CClientSound* pSound = lua_tosound ( L, 1 );
            if ( pSound )
            {
                float fSpeed = 0.0f;
                if ( CStaticFunctionDefinitions::GetSoundSpeed ( *pSound, fSpeed ) )
                {
                    lua_pushnumber ( L, fSpeed );
                    return 1;
                }
            }
        }
        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( setSoundMinDistance )
    {
        if ( lua_istype ( L, 1, LUA_TLIGHTUSERDATA ) &&
            lua_istype ( L, 2, LUA_TNUMBER ) )
        {
            CClientSound* pSound = lua_tosound ( L, 1 );
            if ( pSound )
            {
                float fDistance = ( float ) lua_tonumber ( L, 2 );
                if ( CStaticFunctionDefinitions::SetSoundMinDistance ( *pSound, fDistance ) )
                {
                    lua_pushboolean ( L, true );
                    return 1;
                }
            }
        }
        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( getSoundMinDistance )
    {
        if ( lua_istype ( L, 1, LUA_TLIGHTUSERDATA ) )
        {
            CClientSound* pSound = lua_tosound ( L, 1 );
            if ( pSound )
            {
                float fDistance = 0.0f;
                if ( CStaticFunctionDefinitions::GetSoundMinDistance ( *pSound, fDistance ) )
                {
                    lua_pushnumber ( L, fDistance );
                    return 1;
                }
            }
        }
        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( setSoundMaxDistance )
    {
        if ( lua_istype ( L, 1, LUA_TLIGHTUSERDATA ) &&
            lua_istype ( L, 2, LUA_TNUMBER ) )
        {
            CClientSound* pSound = lua_tosound ( L, 1 );
            if ( pSound )
            {
                float fDistance = ( float ) lua_tonumber ( L, 2 );
                if ( CStaticFunctionDefinitions::SetSoundMaxDistance ( *pSound, fDistance ) )
                {
                    lua_pushboolean ( L, true );
                    return 1;
                }
            }
        }
        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( getSoundMaxDistance )
    {
        if ( lua_istype ( L, 1, LUA_TLIGHTUSERDATA ) )
        {
            CClientSound* pSound = lua_tosound ( L, 1 );
            if ( pSound )
            {
                float fDistance = 0.0f;
                if ( CStaticFunctionDefinitions::GetSoundMaxDistance ( *pSound, fDistance ) )
                {
                    lua_pushnumber ( L, fDistance );
                    return 1;
                }
            }
        }
        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( getSoundMetaTags )
    {
        if ( lua_istype ( L, 1, LUA_TLIGHTUSERDATA ) )
        {
            CClientSound* pSound = lua_tosound ( L, 1 );
            if ( pSound )
            {
                SString strMetaTags = "";
                if ( lua_istype ( L, 2, LUA_TSTRING ) )
                {
                    SString strFormat = lua_tostring ( L, 2 );
                    if ( CStaticFunctionDefinitions::GetSoundMetaTags ( *pSound, strFormat, strMetaTags ) )
                    {
                        if ( !strMetaTags.empty() )
                        {
                            lua_pushstring ( L, strMetaTags );
                            return 1;
                        }
                    }
                }
                else
                {
                    lua_newtable ( L );
                    CStaticFunctionDefinitions::GetSoundMetaTags ( *pSound, "%TITL", strMetaTags );
                    if ( !strMetaTags.empty() )
                    {
                        lua_pushstring ( L, strMetaTags );
                        lua_setfield ( L, -2, "title" );
                    }
                    CStaticFunctionDefinitions::GetSoundMetaTags ( *pSound, "%ARTI", strMetaTags );
                    if ( !strMetaTags.empty() )
                    {
                        lua_pushstring ( L, strMetaTags );
                        lua_setfield ( L, -2, "artist" );
                    }
                    CStaticFunctionDefinitions::GetSoundMetaTags ( *pSound, "%ALBM", strMetaTags );
                    if ( !strMetaTags.empty() )
                    {
                        lua_pushstring ( L, strMetaTags );
                        lua_setfield ( L, -2, "album" );
                    }
                    CStaticFunctionDefinitions::GetSoundMetaTags ( *pSound, "%GNRE", strMetaTags );
                    if ( !strMetaTags.empty() )
                    {
                        lua_pushstring ( L, strMetaTags );
                        lua_setfield ( L, -2, "genre" );
                    }
                    CStaticFunctionDefinitions::GetSoundMetaTags ( *pSound, "%YEAR", strMetaTags );
                    if ( !strMetaTags.empty() )
                    {
                        lua_pushstring ( L, strMetaTags );
                        lua_setfield ( L, -2, "year" );
                    }
                    CStaticFunctionDefinitions::GetSoundMetaTags ( *pSound, "%CMNT", strMetaTags );
                    if ( !strMetaTags.empty() )
                    {
                        lua_pushstring ( L, strMetaTags );
                        lua_setfield ( L, -2, "comment" );
                    }
                    CStaticFunctionDefinitions::GetSoundMetaTags ( *pSound, "%TRCK", strMetaTags );
                    if ( !strMetaTags.empty() )
                    {
                        lua_pushstring ( L, strMetaTags );
                        lua_setfield ( L, -2, "track" );
                    }
                    CStaticFunctionDefinitions::GetSoundMetaTags ( *pSound, "%COMP", strMetaTags );
                    if ( !strMetaTags.empty() )
                    {
                        lua_pushstring ( L, strMetaTags );
                        lua_setfield ( L, -2, "composer" );
                    }
                    CStaticFunctionDefinitions::GetSoundMetaTags ( *pSound, "%COPY", strMetaTags );
                    if ( !strMetaTags.empty() )
                    {
                        lua_pushstring ( L, strMetaTags );
                        lua_setfield ( L, -2, "copyright" );
                    }
                    CStaticFunctionDefinitions::GetSoundMetaTags ( *pSound, "%SUBT", strMetaTags );
                    if ( !strMetaTags.empty() )
                    {
                        lua_pushstring ( L, strMetaTags );
                        lua_setfield ( L, -2, "subtitle" );
                    }
                    CStaticFunctionDefinitions::GetSoundMetaTags ( *pSound, "%AART", strMetaTags );
                    if ( !strMetaTags.empty() )
                    {
                        lua_pushstring ( L, strMetaTags );
                        lua_setfield ( L, -2, "album_artist" );
                    }
                    CStaticFunctionDefinitions::GetSoundMetaTags ( *pSound, "streamName", strMetaTags );
                    if ( !strMetaTags.empty() )
                    {
                        lua_pushstring ( L, strMetaTags );
                        lua_setfield ( L, -2, "stream_name" );
                    }
                    CStaticFunctionDefinitions::GetSoundMetaTags ( *pSound, "streamTitle", strMetaTags );
                    if ( !strMetaTags.empty() )
                    {
                        lua_pushstring ( L, strMetaTags );
                        lua_setfield ( L, -2, "stream_title" );
                    }
                    return 1;
                }
            }
        }
        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( setSoundEffectEnabled )
    {
        if ( lua_istype ( L, 1, LUA_TLIGHTUSERDATA ) &&
            lua_istype ( L, 2, LUA_TSTRING ) )
        {
            CClientSound* pSound = lua_tosound ( L, 1 );
            if ( pSound )
            {
                SString strEffectName = lua_tostring ( L, 2 );
                bool bEnable = false;
                if ( lua_istype ( L, 3, LUA_TBOOLEAN ) )
                {
                    bEnable = ( lua_toboolean ( L, 3 ) ) ? true : false;
                }
                if ( CStaticFunctionDefinitions::SetSoundEffectEnabled ( *pSound, strEffectName, bEnable ) )
                {
                    lua_pushboolean ( L, true );
                    return 1;
                }
            }
        }
        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( getSoundEffects )
    {
        if ( lua_istype ( L, 1, LUA_TLIGHTUSERDATA ) )
        {
            CClientSound* pSound = lua_tosound ( L, 1 );
            if ( pSound )
            {
                std::map < std::string, int > iFxEffects = m_pManager->GetSoundManager()->GetFxEffects();
                lua_newtable ( L );
                for ( std::map < std::string, int >::const_iterator iter = iFxEffects.begin(); iter != iFxEffects.end(); ++iter )
                {
                    lua_pushboolean ( L, pSound->IsFxEffectEnabled ( (*iter).second ) );
                    lua_setfield ( L, -2, (*iter).first.c_str () );
                }
                return 1;
            }
        }
        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( playMissionAudio )
    {
        // Grab the argument types
        int iArgument1 = lua_type ( L, 1 );
        int iArgument2 = lua_type ( L, 2 );
        int iArgument3 = lua_type ( L, 3 );
        int iArgument4 = lua_type ( L, 4 );

        // Correct argument types?
        if ( ( iArgument1 == LUA_TNUMBER || iArgument1 == LUA_TSTRING ) &&
            ( iArgument2 == LUA_TNUMBER || iArgument2 == LUA_TSTRING ) &&
            ( iArgument3 == LUA_TNUMBER || iArgument3 == LUA_TSTRING ) &&
            ( iArgument4 == LUA_TNUMBER || iArgument4 == LUA_TSTRING ) )
        {
            // Grab the sound and the position
            CVector vecPosition = CVector ( float ( lua_tonumber ( L, 1 ) ), float ( lua_tonumber ( L, 2 ) ), float ( lua_tonumber ( L, 3 ) ) );
            unsigned short usSound = static_cast < short > ( lua_tonumber ( L, 4 ) );

            // Play the sound
            if ( CStaticFunctionDefinitions::PlayMissionAudio ( vecPosition, usSound ) )
            {
                lua_pushboolean ( L, true );
                return 1;
            }
        }
        else
            m_pScriptDebugging->LogBadType ( L, "playMissionAudio" );

        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( playSoundFrontEnd )
    {
        int iArgument1 = lua_type ( L, 1 );
        if ( iArgument1 == LUA_TNUMBER || iArgument1 == LUA_TSTRING )
        {
            unsigned char ucSound = static_cast < unsigned char > ( lua_tonumber ( L, 1 ) );
            if ( ucSound <= 101 )
            {
                if ( CStaticFunctionDefinitions::PlaySoundFrontEnd ( ucSound ) )
                {
                    lua_pushboolean ( L, true );
                    return 1;
                }
            }
            else
                m_pScriptDebugging->LogError ( L, "Invalid sound ID specified. Valid sound IDs are 0 - 101." );
        }
        else
            m_pScriptDebugging->LogBadType ( L, "playSoundFrontEnd" );

        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( preloadMissionAudio )
    {
        int iArgument1 = lua_type ( L, 1 );
        int iArgument2 = lua_type ( L, 2 );

        if ( ( iArgument1 == LUA_TNUMBER || iArgument1 == LUA_TSTRING ) &&
            ( iArgument2 == LUA_TNUMBER || iArgument2 == LUA_TSTRING ) )
        {
            unsigned short usSound = static_cast < unsigned short > ( lua_tonumber ( L, 1 ) );
            unsigned short usSlot = static_cast < unsigned short > ( lua_tonumber ( L, 2 ) );

            if ( CStaticFunctionDefinitions::PreloadMissionAudio ( usSound, usSlot ) )
            {
                lua_pushboolean ( L, true );
                return 1;
            }
        }
        else
            m_pScriptDebugging->LogBadType ( L, "preloadMissionAudio" );

        lua_pushboolean ( L, false );
        return 1;
    }


    LUA_DECLARE( setAmbientSoundEnabled )
    {
        eAmbientSoundType eType; bool bEnabled;

        CScriptArgReader argStream ( L );
        argStream.ReadEnumString ( eType, AMBIENT_SOUND_GENERAL );
        argStream.ReadBool ( bEnabled );

        if ( !argStream.HasErrors () )
        {
            if ( CStaticFunctionDefinitions::SetAmbientSoundEnabled ( eType, bEnabled ) )
            {
                lua_pushboolean ( L, true );
                return 1;
            }
        }
        else
            m_pScriptDebugging->LogCustom ( L, SString ( "Bad argument @ '%s' [%s]", "setAmbientSoundEnabled", *argStream.GetErrorMessage () ) );

        lua_pushboolean ( L, false );
        return 1;
    }


    LUA_DECLARE( isAmbientSoundEnabled )
    {
        eAmbientSoundType eType;

        CScriptArgReader argStream ( L );
        argStream.ReadEnumString ( eType );

        if ( !argStream.HasErrors () )
        {
            bool bResultEnabled;
            if ( CStaticFunctionDefinitions::IsAmbientSoundEnabled ( eType, bResultEnabled ) )
            {
                lua_pushboolean ( L, bResultEnabled );
                return 1;
            }
        }
        else
            m_pScriptDebugging->LogCustom ( L, SString ( "Bad argument @ '%s' [%s]", "isAmbientSoundEnabled", *argStream.GetErrorMessage () ) );

        lua_pushboolean ( L, false );
        return 1;
    }


    LUA_DECLARE( resetAmbientSounds )
    {
        if ( CStaticFunctionDefinitions::ResetAmbientSounds () )
        {
            lua_pushboolean ( L, true );
            return 1;
        }
        else
            m_pScriptDebugging->LogBadType ( L, "resetAmbientSounds" );

        lua_pushboolean ( L, false );
        return 1;
    }
}