/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.0
*               (Shared logic for modifications)
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        mods/shared_logic/lua/CLuaFunctionDefs.Blip.cpp
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
    LUA_DECLARE( createBlip )
    {
        // Position arguments in place?
        int iArgument1 = lua_type ( L, 1 );
        int iArgument2 = lua_type ( L, 2 );
        int iArgument3 = lua_type ( L, 3 );

        if ( ( iArgument1 == LUA_TNUMBER || iArgument1 == LUA_TSTRING ) &&
            ( iArgument2 == LUA_TNUMBER || iArgument2 == LUA_TSTRING ) &&
            ( iArgument3 == LUA_TNUMBER || iArgument3 == LUA_TSTRING ) )
        {
            // Grab the vector
            CVector vecPosition = CVector ( static_cast < float > ( lua_tonumber ( L, 1 ) ),
                static_cast < float > ( lua_tonumber ( L, 2 ) ),
                static_cast < float > ( lua_tonumber ( L, 3 ) ) );

            // Default colors and size
            unsigned char ucIcon = 0;
            unsigned char ucSize = 2;
            SColorRGBA color ( 255, 0, 0, 255 );
            short sOrdering = 0;
            unsigned short usVisibleDistance = 16383;

            // Read out the optional args
            int iArgument4 = lua_type ( L, 4 );
            if ( iArgument4 == LUA_TNUMBER || iArgument4 == LUA_TSTRING )
            {
                ucIcon = static_cast < unsigned char > ( lua_tonumber ( L, 4 ) );

                int iArgument5 = lua_type ( L, 5 );
                if ( iArgument5 == LUA_TNUMBER || iArgument5 == LUA_TSTRING )
                {
                    ucSize = static_cast < unsigned char > ( lua_tonumber ( L, 5 ) );

                    int iArgument6 = lua_type ( L, 6 );
                    if ( iArgument6 == LUA_TNUMBER || iArgument6 == LUA_TSTRING )
                    {
                        color.R = static_cast < unsigned char > ( lua_tonumber ( L, 6 ) );

                        int iArgument7 = lua_type ( L, 7 );
                        if ( iArgument7 == LUA_TNUMBER || iArgument7 == LUA_TSTRING )
                        {
                            color.G = static_cast < unsigned char > ( lua_tonumber ( L, 7 ) );

                            int iArgument8 = lua_type ( L, 8 );
                            if ( iArgument8 == LUA_TNUMBER || iArgument8 == LUA_TSTRING )
                            {
                                color.B = static_cast < unsigned char > ( lua_tonumber ( L, 8 ) );

                                int iArgument9 = lua_type ( L, 9 );
                                if ( iArgument9 == LUA_TNUMBER || iArgument9 == LUA_TSTRING )
                                {
                                    color.A = static_cast < unsigned char > ( lua_tonumber ( L, 9 ) );

                                    int iArgument10 = lua_type ( L, 10 );
                                    if ( iArgument10 == LUA_TNUMBER || iArgument10 == LUA_TSTRING )
                                    {
                                        sOrdering = static_cast < short > ( lua_tonumber ( L, 10 ) );
                                    
                                        int iArgument11 = lua_type ( L, 11 );
                                        if ( iArgument11 == LUA_TNUMBER || iArgument11 == LUA_TSTRING )
                                        {
                                            usVisibleDistance = static_cast < unsigned short > ( lua_tonumber ( L, 11 ) );
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }

            CLuaMain* pLuaMain = m_pLuaManager->GetVirtualMachine ( L );
            if ( pLuaMain )
            {
                CResource* pResource = pLuaMain->GetResource ();
                if ( pResource )
                {
                    // Create the blip
                    CClientRadarMarker* pMarker = CStaticFunctionDefinitions::CreateBlip ( *pResource, vecPosition, ucIcon, ucSize, color, sOrdering, usVisibleDistance );
                    if ( pMarker )
                    {
                        CElementGroup * pGroup = pResource->GetElementGroup();
                        if ( pGroup )
                        {
                            pGroup->Add ( pMarker );
                        }

                        lua_pushelement ( L, pMarker );
                        return 1;
                    }
                }
            }
        }
        else
            m_pScriptDebugging->LogBadType ( L, "createBlip" );

        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( createBlipAttachedTo )
    {
        // Element in place?
        if ( lua_type ( L, 1 ) == LUA_TLIGHTUSERDATA )
        {
            // Grab the element and verify it
            CClientEntity* pEntity = lua_toelement ( L, 1 );
            if ( pEntity )
            {
                // Default colors and size
                unsigned char ucIcon = 0;
                unsigned char ucSize = 2;
                SColorRGBA color ( 255, 0, 0, 255 );
                short sOrdering = 0;
                unsigned short usVisibleDistance = 16383;

                // Read out the optional args
                int iArgument2 = lua_type ( L, 2 );
                if ( iArgument2 == LUA_TNUMBER || iArgument2 == LUA_TSTRING )
                {
                    ucIcon = static_cast < unsigned char > ( lua_tonumber ( L, 2 ) );

                    int iArgument3 = lua_type ( L, 3 );
                    if ( iArgument3 == LUA_TNUMBER || iArgument3 == LUA_TSTRING )
                    {
                        ucSize = static_cast < unsigned char > ( lua_tonumber ( L, 3 ) );

                        int iArgument4 = lua_type ( L, 4 );
                        if ( iArgument4 == LUA_TNUMBER || iArgument4 == LUA_TSTRING )
                        {
                            color.R = static_cast < unsigned char > ( lua_tonumber ( L, 4 ) );

                            int iArgument5 = lua_type ( L, 5 );
                            if ( iArgument5 == LUA_TNUMBER || iArgument5 == LUA_TSTRING )
                            {
                                color.G = static_cast < unsigned char > ( lua_tonumber ( L, 5 ) );

                                int iArgument6 = lua_type ( L, 6 );
                                if ( iArgument6 == LUA_TNUMBER || iArgument6 == LUA_TSTRING )
                                {
                                    color.B = static_cast < unsigned char > ( lua_tonumber ( L, 6 ) );

                                    int iArgument7 = lua_type ( L, 7 );
                                    if ( iArgument7 == LUA_TNUMBER || iArgument7 == LUA_TSTRING )
                                    {
                                        color.A = static_cast < unsigned char > ( lua_tonumber ( L, 7 ) );

                                        int iArgument8 = lua_type ( L, 8 );
                                        if ( iArgument8 == LUA_TNUMBER || iArgument8 == LUA_TSTRING )
                                        {
                                            sOrdering = static_cast < short > ( lua_tonumber ( L, 8 ) );
                                        
                                            int iArgument9 = lua_type ( L, 9 );
                                            if ( iArgument9 == LUA_TNUMBER || iArgument9 == LUA_TSTRING )
                                            {
                                                usVisibleDistance = static_cast < unsigned short > ( lua_tonumber ( L, 9 ) );
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }

                CLuaMain* pLuaMain = m_pLuaManager->GetVirtualMachine ( L );
                if ( pLuaMain )
                {
                    CResource* pResource = pLuaMain->GetResource ();
                    if ( pResource )
                    {
                        // Create the blip
                        CClientRadarMarker* pMarker = CStaticFunctionDefinitions::CreateBlipAttachedTo ( *pResource, *pEntity, ucIcon, ucSize, color, sOrdering, usVisibleDistance );
                        if ( pMarker )
                        {
                            CElementGroup * pGroup = pResource->GetElementGroup();
                            if ( pGroup )
                            {
                                pGroup->Add ( pMarker );
                            }
                            lua_pushelement ( L, pMarker );
                            return 1;
                        }
                    }
                }
            }
            else
                m_pScriptDebugging->LogBadPointer ( L, "createBlipAttachedTo", "element", 1 );
        }
        else
            m_pScriptDebugging->LogBadType ( L, "createBlipAttachedTo" );

        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( getBlipIcon )
    {
        if ( lua_type ( L, 1 ) == LUA_TLIGHTUSERDATA )
        {
            CClientRadarMarker* pMarker = lua_toblip ( L, 1 );
            if ( pMarker )
            {
                unsigned char ucIcon = static_cast < unsigned char > ( pMarker->GetSprite () );
                lua_pushnumber ( L, ucIcon );
                return 1;
            }
            else
                m_pScriptDebugging->LogBadPointer ( L, "getBlipIcon", "blip", 1 );
        }
        else
            m_pScriptDebugging->LogBadType ( L, "getBlipIcon" );

        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( getBlipSize )
    {
        if ( lua_type ( L, 1 ) == LUA_TLIGHTUSERDATA )
        {
            CClientRadarMarker* pMarker = lua_toblip ( L, 1 );
            if ( pMarker )
            {
                unsigned char ucSize = static_cast < unsigned char > ( pMarker->GetScale () );
                lua_pushnumber ( L, ucSize );
                return 1;
            }
            else
                m_pScriptDebugging->LogBadPointer ( L, "getBlipSize", "blip", 1 );
        }
        else
            m_pScriptDebugging->LogBadType ( L, "getBlipSize" );

        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( getBlipColor )
    {
        if ( lua_type ( L, 1 ) == LUA_TLIGHTUSERDATA )
        {
            CClientRadarMarker* pMarker = lua_toblip ( L, 1 );
            if ( pMarker )
            {
                SColor color = pMarker->GetColor ();
                lua_pushnumber ( L, color.R );
                lua_pushnumber ( L, color.G );
                lua_pushnumber ( L, color.B );
                lua_pushnumber ( L, color.A );
                return 4;
            }
            else
                m_pScriptDebugging->LogBadPointer ( L, "getBlipColor", "blip", 1 );
        }
        else
            m_pScriptDebugging->LogBadType ( L, "getBlipColor" );

        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( getBlipOrdering )
    {
        if ( lua_type ( L, 1 ) == LUA_TLIGHTUSERDATA )
        {
            CClientRadarMarker* pMarker = lua_toblip ( L, 1 );
            if ( pMarker )
            {
                short sOrdering = pMarker->GetOrdering ();
                lua_pushnumber ( L, sOrdering );
                return 1;
            }
            else
                m_pScriptDebugging->LogBadPointer ( L, "getBlipOrdering", "blip", 1 );
        }
        else
            m_pScriptDebugging->LogBadType ( L, "getBlipOrdering" );

        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( getBlipVisibleDistance )
    {
        if ( lua_type ( L, 1 ) == LUA_TLIGHTUSERDATA )
        {
            CClientRadarMarker* pMarker = lua_toblip ( L, 1 );
            if ( pMarker )
            {
                unsigned short usVisibleDistance = pMarker->GetVisibleDistance ();
                lua_pushnumber ( L, usVisibleDistance );
                return 1;
            }
            else
                m_pScriptDebugging->LogBadPointer ( L, "getBlipVisibleDistance", "blip", 1 );
        }
        else
            m_pScriptDebugging->LogBadType ( L, "getBlipVisibleDistance" );

        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( setBlipIcon )
    {
        int iArgument1 = lua_type ( L, 1 );
        int iArgument2 = lua_type ( L, 2 );
        if ( ( iArgument1 == LUA_TLIGHTUSERDATA ) &&
            ( iArgument2 == LUA_TNUMBER || iArgument2 == LUA_TSTRING ) )
        {
            CClientEntity* pEntity = lua_toelement ( L, 1 );
            if ( pEntity )
            {
                unsigned char ucIcon = static_cast < unsigned char > ( lua_tonumber ( L, 2 ) );
                if ( CStaticFunctionDefinitions::SetBlipIcon ( *pEntity, ucIcon ) )
                {
                    lua_pushboolean ( L, true );
                    return 1;
                }
            }
            else
                m_pScriptDebugging->LogBadPointer ( L, "setBlipIcon", "element", 1 );
        }
        else
            m_pScriptDebugging->LogBadType ( L, "setBlipIcon" );

        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( setBlipSize )
    {
        int iArgument2 = lua_type ( L, 2 );
        if ( ( lua_type ( L, 1 ) == LUA_TLIGHTUSERDATA ) &&
            ( iArgument2 == LUA_TNUMBER || iArgument2 == LUA_TSTRING ) )
        {
            CClientEntity* pEntity = lua_toelement ( L, 1 );
            if ( pEntity )
            {
                unsigned char ucSize = static_cast < unsigned char > ( lua_tonumber ( L, 2 ) );
                if ( CStaticFunctionDefinitions::SetBlipSize ( *pEntity, ucSize ) )
                {
                    lua_pushboolean ( L, true );
                    return 1;
                }
            }
            else
                m_pScriptDebugging->LogBadPointer ( L, "setBlipSize", "element", 1 );
        }
        else
            m_pScriptDebugging->LogBadType ( L, "setBlipSize" );

        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( setBlipColor )
    {
        int iArgument1 = lua_type ( L, 1 );
        int iArgument2 = lua_type ( L, 2 );
        int iArgument3 = lua_type ( L, 3 );
        int iArgument4 = lua_type ( L, 4 );
        int iArgument5 = lua_type ( L, 5 );
        if ( ( iArgument1 == LUA_TLIGHTUSERDATA ) &&
            ( iArgument2 == LUA_TNUMBER || iArgument2 == LUA_TSTRING ) &&
            ( iArgument3 == LUA_TNUMBER || iArgument3 == LUA_TSTRING ) &&
            ( iArgument4 == LUA_TNUMBER || iArgument4 == LUA_TSTRING ) &&
            ( iArgument5 == LUA_TNUMBER || iArgument5 == LUA_TSTRING ) )
        {
            CClientEntity* pEntity = lua_toelement ( L, 1 );
            if ( pEntity )
            {
                SColor color;
                color.R = static_cast < unsigned char > ( lua_tonumber ( L, 2 ) );
                color.G = static_cast < unsigned char > ( lua_tonumber ( L, 3 ) );
                color.B = static_cast < unsigned char > ( lua_tonumber ( L, 4 ) );
                color.A = static_cast < unsigned char > ( lua_tonumber ( L, 5 ) );

                if ( CStaticFunctionDefinitions::SetBlipColor ( *pEntity, color ) )
                {
                    lua_pushboolean ( L, true );
                    return 1;
                }
            }
            else
                m_pScriptDebugging->LogBadPointer ( L, "setBlipColor", "element", 1 );
        }
        else
            m_pScriptDebugging->LogBadType ( L, "setBlipColor" );

        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( setBlipOrdering )
    {
        int iArgument1 = lua_type ( L, 1 );
        int iArgument2 = lua_type ( L, 2 );
        if ( ( iArgument1 == LUA_TLIGHTUSERDATA ) &&
            ( iArgument2 == LUA_TNUMBER || iArgument2 == LUA_TSTRING ) )
        {
            CClientEntity* pEntity = lua_toelement ( L, 1 );
            if ( pEntity )
            {
                short sOrdering = static_cast < short > ( lua_tonumber ( L, 2 ) );

                if ( CStaticFunctionDefinitions::SetBlipOrdering ( *pEntity, sOrdering ) )
                {
                    lua_pushboolean ( L, true );
                    return 1;
                }
            }
            else
                m_pScriptDebugging->LogBadPointer ( L, "setBlipOrdering", "element", 1 );
        }
        else
            m_pScriptDebugging->LogBadType ( L, "setBlipOrdering" );

        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( setBlipVisibleDistance )
    {
        int iArgument1 = lua_type ( L, 1 );
        int iArgument2 = lua_type ( L, 2 );
        if ( ( iArgument1 == LUA_TLIGHTUSERDATA ) &&
            ( iArgument2 == LUA_TNUMBER || iArgument2 == LUA_TSTRING ) )
        {
            CClientEntity* pEntity = lua_toelement ( L, 1 );
            if ( pEntity )
            {
                unsigned short usVisibleDistance = static_cast < unsigned short > ( lua_tonumber ( L, 2 ) );

                if ( CStaticFunctionDefinitions::SetBlipVisibleDistance ( *pEntity, usVisibleDistance ) )
                {
                    lua_pushboolean ( L, true );
                    return 1;
                }
            }
            else
                m_pScriptDebugging->LogBadPointer ( L, "setBlipVisibleDistance", "element", 1 );
        }
        else
            m_pScriptDebugging->LogBadType ( L, "setBlipVisibleDistance" );

        lua_pushboolean ( L, false );
        return 1;
    }
}