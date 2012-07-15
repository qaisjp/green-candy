/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.0
*               (Shared logic for modifications)
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        mods/shared_logic/lua/CLuaFunctionDefs.Shape.cpp
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
    LUA_DECLARE( createColCircle )
    {
        // Verify the argument types
        int iArgument1 = lua_type ( L, 1 );
        int iArgument2 = lua_type ( L, 2 );
        int iArgument3 = lua_type ( L, 3 );
        if ( ( iArgument1 == LUA_TNUMBER || iArgument1 == LUA_TSTRING ) &&
            ( iArgument2 == LUA_TNUMBER || iArgument2 == LUA_TSTRING ) &&
            ( iArgument3 == LUA_TNUMBER || iArgument3 == LUA_TSTRING ) )
        {
            // Grab the values
            CVector vecPosition = CVector ( float ( lua_tonumber ( L, 1 ) ), float ( lua_tonumber ( L, 2 ) ), 0.0f );
            float fRadius = float ( lua_tonumber ( L, 3 ) );
            if ( fRadius < 0.0f ) fRadius = 0.1f;

            CLuaMain* pLuaMain = lua_readcontext( L );

            CResource* pResource = pLuaMain->GetResource();
            if ( pResource )
            {
                // Create it and return it
                CClientColCircle* pShape = CStaticFunctionDefinitions::CreateColCircle ( *pResource, vecPosition, fRadius );
                if ( pShape )
                {
                    CElementGroup * pGroup = pResource->GetElementGroup();
                    if ( pGroup )
                    {
                        pGroup->Add ( pShape );
                    }
                    lua_pushelement ( L, pShape );
                    return 1;
                }
            }
        }
        else
            m_pScriptDebugging->LogBadType( "createColCircle" );

        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( createColCuboid )
    {
        // Verify the argument types
        int iArgument1 = lua_type ( L, 1 );
        int iArgument2 = lua_type ( L, 2 );
        int iArgument3 = lua_type ( L, 3 );
        int iArgument4 = lua_type ( L, 4 );
        int iArgument5 = lua_type ( L, 5 );
        int iArgument6 = lua_type ( L, 6 );
        if ( ( iArgument1 == LUA_TNUMBER || iArgument1 == LUA_TSTRING ) &&
            ( iArgument2 == LUA_TNUMBER || iArgument2 == LUA_TSTRING ) &&
            ( iArgument3 == LUA_TNUMBER || iArgument3 == LUA_TSTRING ) &&
            ( iArgument4 == LUA_TNUMBER || iArgument4 == LUA_TSTRING ) &&
            ( iArgument5 == LUA_TNUMBER || iArgument5 == LUA_TSTRING ) &&
            ( iArgument6 == LUA_TNUMBER || iArgument6 == LUA_TSTRING ) )
        {
            // Grab the values
            CVector vecPosition = CVector ( float ( lua_tonumber ( L, 1 ) ), float ( lua_tonumber ( L, 2 ) ), float ( lua_tonumber ( L, 3 ) ) );
            CVector vecSize = CVector ( float ( lua_tonumber ( L, 4 ) ), float ( lua_tonumber ( L, 5 ) ), float ( lua_tonumber ( L, 6 ) ) );
            if ( vecSize.fX < 0.0f ) vecSize.fX = 0.1f;
            if ( vecSize.fY < 0.0f ) vecSize.fY = 0.1f;

            CLuaMain* pLuaMain = lua_readcontext( L );

            CResource* pResource = pLuaMain->GetResource ();
            if ( pResource )
            {
                // Create it and return it
                CClientColCuboid* pShape = CStaticFunctionDefinitions::CreateColCuboid ( *pResource, vecPosition, vecSize );
                if ( pShape )
                {
                    CElementGroup * pGroup = pResource->GetElementGroup();
                    if ( pGroup )
                    {
                        pGroup->Add ( ( CClientEntity* ) pShape );
                    }
                    lua_pushelement ( L, pShape );
                    return 1;
                }
            }
        }
        else
            m_pScriptDebugging->LogBadType( "createColCuboid" );

        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( createColSphere )
    {
        // Verify the argument types
        int iArgument1 = lua_type ( L, 1 );
        int iArgument2 = lua_type ( L, 2 );
        int iArgument3 = lua_type ( L, 3 );
        int iArgument4 = lua_type ( L, 4 );
        if ( ( iArgument1 == LUA_TNUMBER || iArgument1 == LUA_TSTRING ) &&
            ( iArgument2 == LUA_TNUMBER || iArgument2 == LUA_TSTRING ) &&
            ( iArgument3 == LUA_TNUMBER || iArgument3 == LUA_TSTRING ) &&
            ( iArgument4 == LUA_TNUMBER || iArgument4 == LUA_TSTRING ) )
        {
            // Grab the values
            CVector vecPosition = CVector ( float ( lua_tonumber ( L, 1 ) ), float ( lua_tonumber ( L, 2 ) ), float ( lua_tonumber ( L, 3 ) ) );
            float fRadius = float ( lua_tonumber ( L, 4 ) );
            if ( fRadius < 0.0f ) fRadius = 0.1f;

            CLuaMain* pLuaMain = lua_readcontext( L );

            CResource* pResource = pLuaMain->GetResource ();
            if ( pResource )
            {
                // Create it and return it
                CClientColSphere* pShape = CStaticFunctionDefinitions::CreateColSphere ( *pResource, vecPosition, fRadius );
                if ( pShape )
                {
                    CElementGroup * pGroup = pResource->GetElementGroup();
                    if ( pGroup )
                    {
                        pGroup->Add ( ( CClientEntity* ) pShape );
                    }
                    lua_pushelement ( L, pShape );
                    return 1;
                }
            }
        }
        else
            m_pScriptDebugging->LogBadType( "createColSphere" );

        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( createColRectangle )
    {
        // Verify the argument types
        int iArgument1 = lua_type ( L, 1 );
        int iArgument2 = lua_type ( L, 2 );
        int iArgument3 = lua_type ( L, 3 );
        int iArgument4 = lua_type ( L, 4 );
        if ( ( iArgument1 == LUA_TNUMBER || iArgument1 == LUA_TSTRING ) &&
            ( iArgument2 == LUA_TNUMBER || iArgument2 == LUA_TSTRING ) &&
            ( iArgument3 == LUA_TNUMBER || iArgument3 == LUA_TSTRING ) &&
            ( iArgument4 == LUA_TNUMBER || iArgument4 == LUA_TSTRING ) )
        {
            // Grab the values
            CVector vecPosition = CVector ( float ( lua_tonumber ( L, 1 ) ), float ( lua_tonumber ( L, 2 ) ), float ( 0 ) );
            CVector2D vecSize = CVector2D ( float ( lua_tonumber ( L, 3 ) ), float ( lua_tonumber ( L, 4 ) ) );
            if ( vecSize.fX < 0.0f ) vecSize.fX = 0.1f;
            if ( vecSize.fY < 0.0f ) vecSize.fY = 0.1f;

            CLuaMain* pLuaMain = lua_readcontext( L );

            CResource* pResource = pLuaMain->GetResource ();
            if ( pResource )
            {
                // Create it and return it
                CClientColRectangle* pShape = CStaticFunctionDefinitions::CreateColRectangle ( *pResource, vecPosition, vecSize );
                if ( pShape )
                {
                    CElementGroup * pGroup = pResource->GetElementGroup();
                    if ( pGroup )
                    {
                        pGroup->Add ( ( CClientEntity* ) pShape );
                    }
                    lua_pushelement ( L, pShape );
                    return 1;
                }
            }
        }
        else
            m_pScriptDebugging->LogBadType( "createColRectangle" );

        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( createColPolygon )
    { // Formerly createColSquare
        // Verify the argument types
        int iArgument1 = lua_type ( L, 1 );
        int iArgument2 = lua_type ( L, 2 );
        if ( ( iArgument1 == LUA_TNUMBER || iArgument1 == LUA_TSTRING ) &&
            ( iArgument2 == LUA_TNUMBER || iArgument2 == LUA_TSTRING ) )
        {
            // Grab the values
            CVector vecPosition = CVector ( ( float ) lua_tonumber ( L, 1 ), ( float ) lua_tonumber ( L, 2 ), 0.0f );

            CLuaMain* pLuaMain = lua_readcontext( L );
            CResource* pResource = pLuaMain->GetResource();
            if ( pResource )
            {
                // Create it and return it
                CClientColPolygon* pShape = CStaticFunctionDefinitions::CreateColPolygon ( *pResource, vecPosition );
                if ( pShape )
                {
                    // Get the points
                    int iArgument = 3;
                    int iArgumentX = lua_type ( L, iArgument++ );
                    int iArgumentY = lua_type ( L, iArgument++ );
                    while ( iArgumentX != LUA_TNONE && iArgumentY != LUA_TNONE )
                    {
                        if ( ( iArgumentX == LUA_TNUMBER || iArgumentX == LUA_TSTRING ) &&
                            ( iArgumentY == LUA_TNUMBER || iArgumentY == LUA_TSTRING ) )
                        {
                            pShape->AddPoint ( CVector2D ( ( float ) lua_tonumber ( L, iArgument - 2 ),
                                ( float ) lua_tonumber ( L, iArgument - 1 ) ) );

                            iArgumentX = lua_type ( L, iArgument++ );
                            iArgumentY = lua_type ( L, iArgument++ );
                        }
                        else
                        {
                            break;
                        }
                    }

                    CElementGroup * pGroup = pResource->GetElementGroup();
                    if ( pGroup )
                    {
                        pGroup->Add ( pShape );
                    }
                    lua_pushelement ( L, pShape );
                    return 1;
                }
            }
        }
        else
            m_pScriptDebugging->LogBadType( "createColPolygon" );

        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( createColTube )
    {
        // Verify the argument types
        int iArgument1 = lua_type ( L, 1 );
        int iArgument2 = lua_type ( L, 2 );
        int iArgument3 = lua_type ( L, 3 );
        int iArgument4 = lua_type ( L, 4 );
        int iArgument5 = lua_type ( L, 5 );
        if ( ( iArgument1 == LUA_TNUMBER || iArgument1 == LUA_TSTRING ) &&
            ( iArgument2 == LUA_TNUMBER || iArgument2 == LUA_TSTRING ) &&
            ( iArgument3 == LUA_TNUMBER || iArgument3 == LUA_TSTRING ) &&
            ( iArgument4 == LUA_TNUMBER || iArgument4 == LUA_TSTRING ) &&
            ( iArgument5 == LUA_TNUMBER || iArgument5 == LUA_TSTRING ) )
        {
            // Grab the values
            CVector vecPosition = CVector ( float ( lua_tonumber ( L, 1 ) ), float ( lua_tonumber ( L, 2 ) ), float ( lua_tonumber ( L, 3 ) ) );
            float fRadius = float ( lua_tonumber ( L, 4 ) );
            float fHeight = float ( lua_tonumber ( L, 5 ) );
            if ( fRadius < 0.0f ) fRadius = 0.1f;
            if ( fHeight < 0.0f ) fRadius = 0.1f;

            CLuaMain* pLuaMain = lua_readcontext( L );
            CResource* pResource = pLuaMain->GetResource ();
            if ( pResource )
            {
                // Create it and return it
                CClientColTube* pShape = CStaticFunctionDefinitions::CreateColTube ( *pResource, vecPosition, fRadius, fHeight );
                if ( pShape )
                {
                    CElementGroup * pGroup = pResource->GetElementGroup();
                    if ( pGroup )
                    {
                        pGroup->Add ( ( CClientEntity* ) pShape );
                    }
                    lua_pushelement ( L, pShape );
                    return 1;
                }
            }
        }
        else
            m_pScriptDebugging->LogBadType( "createColTube" );

        lua_pushboolean ( L, false );
        return 1;
    }
}