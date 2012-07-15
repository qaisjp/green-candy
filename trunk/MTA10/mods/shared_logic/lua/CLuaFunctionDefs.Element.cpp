/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.0
*               (Shared logic for modifications)
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        mods/shared_logic/lua/CLuaFunctionDefs.Element.cpp
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
    LUA_DECLARE( getRootElement )
    {
        CClientEntity* pRoot = CStaticFunctionDefinitions::GetRootElement ();

        // Return the root element
        lua_pushelement ( L, pRoot );
        return 1;
    }

    LUA_DECLARE( isElement )
    {
        // Correct argument type?
        if ( lua_istype ( L, 1, LUA_TLIGHTUSERDATA ) )
        {
            // Grab the element. If it's valid, return true.
            CClientEntity* pEntity = lua_toelement ( L, 1 );
            if ( pEntity )
            {
                lua_pushboolean ( L, true );
                return 1;
            }
        }

        // Failed
        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( getElementID )
    {
        // Correct argument type specified?
        if ( lua_istype ( L, 1, LUA_TLIGHTUSERDATA ) )
        {
            // Grab the element
            CClientEntity* pEntity = lua_toelement ( L, 1 );
            if ( pEntity )
            {
                // Get its ID and return it
                const char* szName = pEntity->GetName ();
                if ( szName )
                {
                    lua_pushstring ( L, szName );
                    return 1;
                }
            }
            else
                m_pScriptDebugging->LogBadPointer( "getElementID", "element", 1 );
        }
        else
            m_pScriptDebugging->LogBadType( "getElementID" );

        // Failed
        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( getElementByID )
    {
        // Eventually read out the index
        int iArgument2 = lua_type ( L, 2 );
        unsigned int uiIndex = 0;
        if ( iArgument2 == LUA_TNUMBER || iArgument2 == LUA_TSTRING )
        {
            uiIndex = static_cast < unsigned int > ( lua_tonumber ( L, 2 ) );
        }

        // Check that we got the name in place
        if ( lua_istype ( L, 1, LUA_TSTRING ) )
        {
            // Grab the string
            const char* szID = lua_tostring ( L, 1 );

            // Try to find the element with that ID. Return it
            CClientEntity* pEntity = CStaticFunctionDefinitions::GetElementByID ( szID, uiIndex );
            if ( pEntity )
            {
                lua_pushelement ( L, pEntity );
                return 1;
            }
        }
        else
            m_pScriptDebugging->LogBadType( "getElementByID" );

        // Failed
        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( getElementByIndex )
    {
        // Verify element type
        int iArgument1 = lua_type ( L, 1 );
        int iArgument2 = lua_type ( L, 2 );
        if ( ( iArgument1 == LUA_TSTRING ) &&
            ( iArgument2 == LUA_TNUMBER || iArgument2 == LUA_TSTRING ) )
        {
            // Grab the element type name and the index
            const char* szType = lua_tostring ( L, 1 );
            unsigned int uiIndex = static_cast < unsigned int > ( lua_tonumber ( L, 2 ) );

            // Try to find it. Return it if we did.
            CClientEntity* pEntity = CStaticFunctionDefinitions::GetElementByIndex ( szType, uiIndex );
            if ( pEntity )
            {
                lua_pushelement ( L, pEntity );
                return 1;
            }
        }
        else
            m_pScriptDebugging->LogBadType( "getElementByIndex" );

        // Failed
        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( getElementData )
    {
    //  var getElementData ( element theElement, string key [, inherit = true] )
        CClientEntity* pEntity; SString strKey; bool bInherit;

        CScriptArgReader argStream ( L );
        argStream.ReadUserData ( pEntity );
        argStream.ReadString ( strKey );
        argStream.ReadBool ( bInherit, true );

        if ( !argStream.HasErrors () )
        {
            CLuaMain* pLuaMain = lua_readcontext( L );

            if ( strKey.length () > MAX_CUSTOMDATA_NAME_LENGTH )
            {
                // Warn and truncate if key is too long
                m_pScriptDebugging->LogCustom( SString ( "Truncated argument @ '%s' [%s]", "getElementData", *SString ( "string length reduced to %d characters at argument 2", MAX_CUSTOMDATA_NAME_LENGTH ) ) );
                strKey = strKey.Left ( MAX_CUSTOMDATA_NAME_LENGTH );
            }

            CLuaArgument* pVariable = pEntity->GetCustomData ( strKey, bInherit );
            if ( pVariable )
            {
                pVariable->Push ( L );
                return 1;
            }
        }
        else
            m_pScriptDebugging->LogCustom( SString ( "Bad argument @ '%s' [%s]", "getElementData", *argStream.GetErrorMessage () ) );

        // Failed
        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( getElementMatrix )
    {
        // Verify the argument
        if ( lua_istype ( L, 1, LUA_TLIGHTUSERDATA ) )
        {
            // Grab the element, verify it
            CClientEntity* pEntity = lua_toelement ( L, 1 );
            if ( pEntity )
            {
                // Grab the position
                CMatrix matrix;
                if ( CStaticFunctionDefinitions::GetElementMatrix ( *pEntity, matrix ) )
                {            
                    // Return it
                    lua_createtable ( L, 4, 0 );

                    // First row
                    lua_createtable ( L, 4, 0 );
                    lua_pushnumber ( L, matrix.vRight.fX );
                    lua_rawseti ( L, -2, 1 );
                    lua_pushnumber ( L, matrix.vRight.fY );
                    lua_rawseti ( L, -2, 2 );
                    lua_pushnumber ( L, matrix.vRight.fZ );
                    lua_rawseti ( L, -2, 3 );
                    lua_pushnumber ( L, 1.0f );
                    lua_rawseti ( L, -2, 4 );
                    lua_rawseti ( L, -2, 1 );

                    // Second row
                    lua_createtable ( L, 4, 0 );
                    lua_pushnumber ( L, matrix.vFront.fX );
                    lua_rawseti ( L, -2, 1 );
                    lua_pushnumber ( L, matrix.vFront.fY );
                    lua_rawseti ( L, -2, 2 );
                    lua_pushnumber ( L, matrix.vFront.fZ );
                    lua_rawseti ( L, -2, 3 );
                    lua_pushnumber ( L, 1.0f );
                    lua_rawseti ( L, -2, 4 );
                    lua_rawseti ( L, -2, 2 );

                    // Third row
                    lua_createtable ( L, 4, 0 );
                    lua_pushnumber ( L, matrix.vUp.fX );
                    lua_rawseti ( L, -2, 1 );
                    lua_pushnumber ( L, matrix.vUp.fY );
                    lua_rawseti ( L, -2, 2 );
                    lua_pushnumber ( L, matrix.vUp.fZ );
                    lua_rawseti ( L, -2, 3 );
                    lua_pushnumber ( L, 1.0f );
                    lua_rawseti ( L, -2, 4 );
                    lua_rawseti ( L, -2, 3 );

                    // Fourth row
                    lua_createtable ( L, 4, 0 );
                    lua_pushnumber ( L, matrix.vPos.fX );
                    lua_rawseti ( L, -2, 1 );
                    lua_pushnumber ( L, matrix.vPos.fY );
                    lua_rawseti ( L, -2, 2 );
                    lua_pushnumber ( L, matrix.vPos.fZ );
                    lua_rawseti ( L, -2, 3 );
                    lua_pushnumber ( L, 1.0f );
                    lua_rawseti ( L, -2, 4 );
                    lua_rawseti ( L, -2, 4 );

                    return 1;
                }
            }
            else
                m_pScriptDebugging->LogBadPointer( "getElementMatrix", "element", 1 );
        }
        else
            m_pScriptDebugging->LogBadType( "getElementMatrix" );

        // Failed
        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( getElementPosition )
    {
        // Verify the argument
        if ( lua_istype ( L, 1, LUA_TLIGHTUSERDATA ) )
        {
            // Grab the element, verify it
            CClientEntity* pEntity = lua_toelement ( L, 1 );
            if ( pEntity )
            {
                // Grab the position
                CVector vecPosition;
                if ( CStaticFunctionDefinitions::GetElementPosition ( *pEntity, vecPosition ) )
                {            
                    // Return it
                    lua_pushnumber ( L, vecPosition.fX );
                    lua_pushnumber ( L, vecPosition.fY );
                    lua_pushnumber ( L, vecPosition.fZ );
                    return 3;
                }
            }
            else
                m_pScriptDebugging->LogBadPointer( "getElementPosition", "element", 1 );
        }
        else
            m_pScriptDebugging->LogBadType( "getElementPosition" );

        // Failed
        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( getElementRotation )
    {
        // Verify the argument
        if ( lua_istype ( L, 1, LUA_TLIGHTUSERDATA ) )
        {
            // Grab the element, verify it
            CClientEntity* pEntity = lua_toelement ( L, 1 );

            const char* szRotationOrder = "default";
            if ( lua_type ( L, 2 ) == LUA_TSTRING ) 
            {
                szRotationOrder = lua_tostring ( L, 2 );
            }

            if ( pEntity )
            {
                // Grab the rotation
                CVector vecRotation;
                if ( CStaticFunctionDefinitions::GetElementRotation ( *pEntity, vecRotation, szRotationOrder ) )
                {            
                    // Return it
                    lua_pushnumber ( L, vecRotation.fX );
                    lua_pushnumber ( L, vecRotation.fY );
                    lua_pushnumber ( L, vecRotation.fZ );
                    return 3;
                }
            }
            else
                m_pScriptDebugging->LogBadPointer( "getElementRotation", "element", 1 );
        }
        else
            m_pScriptDebugging->LogBadType( "getElementRotation" );

        // Failed
        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( getElementVelocity )
    {
        // Verify the argument
        if ( lua_istype ( L, 1, LUA_TLIGHTUSERDATA ) )
        {
            // Grab the element, verify it
            CClientEntity* pEntity = lua_toelement ( L, 1 );
            if ( pEntity )
            {
                // Grab the velocity
                CVector vecVelocity;
                if ( CStaticFunctionDefinitions::GetElementVelocity ( *pEntity, vecVelocity ) )
                {
                    // Return it
                    lua_pushnumber ( L, vecVelocity.fX );
                    lua_pushnumber ( L, vecVelocity.fY );
                    lua_pushnumber ( L, vecVelocity.fZ );
                    return 3;
                }
            }
            else
                m_pScriptDebugging->LogBadPointer( "getElementVelocity", "element", 1 );
        }
        else
            m_pScriptDebugging->LogBadType( "getElementVelocity" );

        // Failed
        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( getElementType )
    {
        // Check the arg type
        if ( lua_istype ( L, 1, LUA_TLIGHTUSERDATA ) )
        {
            // Grab the element
            CClientEntity* pEntity = lua_toelement ( L, 1 );
            if ( pEntity )
            {
                // Get its typename
                const char* szTypeName = pEntity->GetTypeName ();
                if ( szTypeName )
                {
                    lua_pushstring ( L, szTypeName );
                    return 1;
                }
            }
            else
                m_pScriptDebugging->LogBadPointer( "getElementType", "element", 1 );
        }
        else
            m_pScriptDebugging->LogBadType( "getElementType" );

        // Failed
        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( getElementChildren )
    {
        // Find our VM
        CLuaMain* pLuaMain = lua_readcontext( L );

        // Correct arg type?
        if ( lua_istype ( L, 1, LUA_TLIGHTUSERDATA ) )
        {
            // Grab the argument
            CClientEntity* pEntity = lua_toelement ( L, 1 );
            if ( pEntity )
            {
                // Create a new table
                lua_newtable ( L );

                // Add all the elements with a matching type to it
                pEntity->GetChildren ( L );
                return 1;
            }
            else
                m_pScriptDebugging->LogBadPointer( "getElementChildren", "element", 1 );
        }
        else
            m_pScriptDebugging->LogBadType( "getElementChildren" );

        // Failed
        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( getElementChild )
    {
        // Verify argument types
        int iArgument1 = lua_type ( L, 1 );
        int iArgument2 = lua_type ( L, 2 );
        if ( ( iArgument1 == LUA_TLIGHTUSERDATA ) &&
            ( iArgument2 == LUA_TNUMBER || iArgument2 == LUA_TSTRING ) )
        {
            // Grab entity and index
            CClientEntity* pEntity = lua_toelement ( L, 1 );
            unsigned int uiIndex = static_cast < unsigned int > ( lua_tonumber ( L, 2 ) );
            if ( pEntity )
            {
                // Grab the child
                CClientEntity* pChild = CStaticFunctionDefinitions::GetElementChild ( *pEntity, uiIndex );
                if ( pChild )
                {
                    lua_pushelement ( L, pChild );
                    return 1;
                }
            }
            else
                m_pScriptDebugging->LogBadPointer( "getElementChild", "element", 1 );
        }
        else
            m_pScriptDebugging->LogBadType( "getElementChild" );

        // Failed
        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( getElementChildrenCount )
    {
        // Correct arg type?
        if ( lua_istype ( L, 1, LUA_TLIGHTUSERDATA ) )
        {
            // Grab the element
            CClientEntity* pEntity = lua_toelement ( L, 1 );
            if ( pEntity )
            {
                // Grab the count
                unsigned int uiCount = pEntity->CountChildren ();
                lua_pushnumber ( L, static_cast < lua_Number > ( uiCount ) );
                return 1;
            }
            else
                m_pScriptDebugging->LogBadPointer( "getElementChildrenCount", "element", 1 );
        }
        else
            m_pScriptDebugging->LogBadType( "getElementChildrenCount" );

        // Failed
        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( getElementParent )
    {
        // Correct arg type?
        if ( lua_istype ( L, 1, LUA_TLIGHTUSERDATA ) )
        {
            // Grab the entity
            CClientEntity* pEntity = lua_toelement ( L, 1 );
            if ( pEntity )
            {
                // Grab the parent and return it
                CClientEntity* pParent = pEntity->GetParent ();
                if ( pParent )
                {
                    lua_pushelement ( L, pParent );
                    return 1;
                }
            }
            else
                m_pScriptDebugging->LogBadPointer( "getElementParent", "element", 1 );
        }
        else
            m_pScriptDebugging->LogBadType( "getElementParent" );

        // Failed
        lua_pushboolean ( L, false );
        return 1;
    }


    LUA_DECLARE( getElementsByType )
    {
        // Find our VM
        CLuaMain* pLuaMain = lua_readcontext( L );

        // Check arg types
        int iArgument2 = lua_type ( L, 2 );
        if ( lua_type ( L, 1 ) == LUA_TSTRING && ( iArgument2 == LUA_TNONE || iArgument2 == LUA_TLIGHTUSERDATA ) )
        {
            // see if a root argument has been specified
            CClientEntity* startAt = m_pRootEntity;
            if ( iArgument2 == LUA_TLIGHTUSERDATA )
            {
                // if its valid, use that, otherwise, produce an error
                CClientEntity* pEntity = lua_toelement ( L, 2 );
                if ( pEntity )
                    startAt = pEntity;
                else
                {
                    m_pScriptDebugging->LogBadPointer( "getElementsByType", "element", 2 );
                    lua_pushboolean ( L, false );
                    return 1;
                }
            }

            bool bStreamedIn = false;
            if ( lua_type ( L, 3 ) == LUA_TBOOLEAN )
            {
                bStreamedIn = lua_toboolean ( L, 3 ) ? true : false;
            }

            // Grab the argument
            const char* szType = lua_tostring ( L, 1 );

            // Create a new table
            lua_newtable ( L );

            // Add all the elements with a matching type to it
            startAt->FindAllChildrenByType ( szType, L, bStreamedIn );
            return 1;
        }
        else
            m_pScriptDebugging->LogBadType( "getElementsByType" );

        // Failed
        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( getElementInterior )
    {
        // Check arg types
        if ( lua_istype ( L, 1, LUA_TLIGHTUSERDATA ) )
        {
            // Grab the element
            CClientEntity* pEntity = lua_toelement ( L, 1 );
            if ( pEntity )
            {
                // Grab the interior and return it
                unsigned char ucInterior;
                if ( CStaticFunctionDefinitions::GetElementInterior ( *pEntity, ucInterior ) )
                {
                    lua_pushnumber ( L, ucInterior );
                    return 1;
                }
            }
            else
                m_pScriptDebugging->LogBadPointer( "getElementInterior", "element", 1 );
        }
        else
            m_pScriptDebugging->LogBadType( "getElementInterior" );

        // Failed
        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( isElementWithinColShape )
    {
        // Check arg types
        if ( lua_istype ( L, 1, LUA_TLIGHTUSERDATA ) &&
            lua_istype ( L, 2, LUA_TLIGHTUSERDATA ) )
        {
            // Grab elements
            CClientEntity* pEntity = lua_toelement ( L, 1 );
            CClientColShape* pColShape = lua_tocolshape ( L, 2 );

            // Valid element?
            if ( pEntity )
            {
                // Valid colshape?
                if ( pColShape )
                {
                    // Check if it's within
                    bool bWithin = pEntity->CollisionExists ( pColShape );
                    lua_pushboolean ( L, bWithin );
                    return 1;
                }
                else
                    m_pScriptDebugging->LogBadPointer( "isElementWithinColShape", "colshape", 2 );
            }
            else
                m_pScriptDebugging->LogBadPointer( "isElementWithinColShape", "element", 1 );
        }
        else
            m_pScriptDebugging->LogBadType( "isElementWithinColShape" );

        // Failed
        lua_pushnil ( L );
        return 1;
    }

    LUA_DECLARE( isElementWithinMarker )
    {
        // Check arg types
        if ( lua_istype ( L, 1, LUA_TLIGHTUSERDATA ) &&
            lua_istype ( L, 2, LUA_TLIGHTUSERDATA ) )
        {
            // Grab elements
            CClientEntity* pEntity = lua_toelement ( L, 1 );
            CClientMarker* pMarker = lua_tomarker ( L, 2 );

            // Valid element?
            if ( pEntity )
            {
                // Valid colshape?
                if ( pMarker )
                {
                    // Check if it's within
                    bool bWithin = pEntity->CollisionExists ( pMarker->GetColShape() );
                    lua_pushboolean ( L, bWithin );
                    return 1;
                }
                else
                    m_pScriptDebugging->LogBadPointer( "isElementWithinMarker", "marker", 2 );
            }
            else
                m_pScriptDebugging->LogBadPointer( "isElementWithinMarker", "element", 1 );
        }
        else
            m_pScriptDebugging->LogBadType( "isElementWithinMarker" );

        // Failed
        lua_pushnil ( L );
        return 1;
    }

    LUA_DECLARE( getElementsWithinColShape )
    {
        // Grab our VM
        CLuaMain* pLuaMain = lua_readcontext( L );

        // Correct arg type?
        if ( lua_istype ( L, 1, LUA_TLIGHTUSERDATA ) )
        {
            // Grab optional type arg
            const char* szType = NULL;
            if ( lua_istype ( L, 2, LUA_TSTRING ) )
            {
                // Is it zero length? Don't use it
                szType = lua_tostring ( L, 2 );
                if ( szType [ 0 ] == 0 )
                    szType = NULL;
            }

            // Grab the colshape passed
            CClientColShape* pColShape = lua_tocolshape ( L, 1 );
            if ( pColShape )
            {
                // Create a new table
                lua_newtable ( L );

                // Add all the elements within the shape to it
                unsigned int uiIndex = 0;
                list < CClientEntity* > ::iterator iter = pColShape->CollidersBegin ();
                for ( ; iter != pColShape->CollidersEnd (); iter++ )
                {
                    if ( szType == NULL || strcmp ( (*iter)->GetTypeName (), szType ) == 0 )
                    {
                        lua_pushnumber ( L, ++uiIndex );
                        lua_pushelement ( L, *iter );
                        lua_settable ( L, -3 );
                    }
                }

                // We're returning 1 table
                return 1;
            }
            else
                m_pScriptDebugging->LogBadPointer( "getElementsWithinColShape", "colshape", 1 );
        }
        else
            m_pScriptDebugging->LogBadType( "getElementsWithinColShape" );

        // Failed
        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( getElementDimension )
    {
        // Correct type?
        if ( lua_istype ( L, 1, LUA_TLIGHTUSERDATA ) )
        {
            // Grab the element to check
            CClientEntity* pEntity = lua_toelement ( L, 1 );
            if ( pEntity )
            {
                // Grab the dimension
                unsigned short usDimension = pEntity->GetDimension ();
                lua_pushnumber ( L, usDimension );
                return 1;
            }
            else
                m_pScriptDebugging->LogBadPointer( "getElementDimension", "element", 1 );
        }
        else
            m_pScriptDebugging->LogBadType( "getElementDimension" );

        // Failed
        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( getElementBoundingBox )
    {
        // Correct type?
        if ( lua_istype ( L, 1, LUA_TLIGHTUSERDATA ) )
        {
            // Grab the element
            CClientEntity* pEntity = lua_toelement ( L, 1 );
            if ( pEntity )
            {
                // Grab the bounding box and return it
                CVector vecMin, vecMax;
                if ( CStaticFunctionDefinitions::GetElementBoundingBox ( *pEntity, vecMin, vecMax ) )
                {
                    lua_pushnumber ( L, vecMin.fX );
                    lua_pushnumber ( L, vecMin.fY );
                    lua_pushnumber ( L, vecMin.fZ );
                    lua_pushnumber ( L, vecMax.fX );
                    lua_pushnumber ( L, vecMax.fY );
                    lua_pushnumber ( L, vecMax.fZ );
                    return 6;
                }
            }
            else
                m_pScriptDebugging->LogBadPointer( "getElementBoundingBox", "element", 1 );
        }
        else
            m_pScriptDebugging->LogBadType( "getElementBoundingBox" );

        // Failed
        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( getElementRadius )
    {
        // Correct type?
        if ( lua_istype ( L, 1, LUA_TLIGHTUSERDATA ) )
        {
            // Grab the element
            CClientEntity* pEntity = lua_toelement ( L, 1 );
            if ( pEntity )
            {
                // Grab its radius and return it
                float fRadius = 0.0f;
                if ( CStaticFunctionDefinitions::GetElementRadius ( *pEntity, fRadius ) )
                {
                    lua_pushnumber ( L, fRadius );
                    return 1;
                }
            }
            else
                m_pScriptDebugging->LogBadPointer( "getElementRadius", "element", 1 );
        }
        else
            m_pScriptDebugging->LogBadType( "getElementRadius" );

        // Failed
        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( isElementAttached )
    {
        // Correct type?
        if ( lua_istype ( L, 1, LUA_TLIGHTUSERDATA ) )
        {
            // Grab the element
            CClientEntity* pEntity = lua_toelement ( L, 1 );
            if ( pEntity )
            {
                CClientEntity* pEntityAttachedTo = pEntity->GetAttachedTo();
                if ( pEntityAttachedTo )
                {
                    if ( pEntityAttachedTo->IsEntityAttached ( pEntity ) )
                    {
                        lua_pushboolean ( L, true );
                        return 1;
                    }
                }
            }
            else
                m_pScriptDebugging->LogBadPointer( "isElementAttached", "element", 1 );
        }
        else
            m_pScriptDebugging->LogBadType( "isElementAttached" );

        // Failed
        lua_pushnil ( L );
        return 1;
    }

    LUA_DECLARE( getElementAttachedTo )
    {
        // Check types
        if ( lua_istype ( L, 1, LUA_TLIGHTUSERDATA ) )
        {
            // Grab the element to check
            CClientEntity* pEntity = lua_toelement ( L, 1 );
            CClientEntity* pEntityAttachedTo = NULL;

            // Valid?
            if ( pEntity )
            {
                // Grab the enity attached to it
                CClientEntity* pEntityAttachedTo = CStaticFunctionDefinitions::GetElementAttachedTo ( *pEntity );
                if ( pEntityAttachedTo )
                {
                    lua_pushelement ( L, pEntityAttachedTo );
                    return 1;
                }
                else
                {
                    lua_pushboolean ( L, false );
                    return 1;
                }
            }
            else
                m_pScriptDebugging->LogBadPointer( "getElementAttachedTo", "element", 1 );
        }
        else
            m_pScriptDebugging->LogBadType( "getElementAttachedTo" );

        // Failed
        lua_pushnil ( L );
        return 1;
    }

    LUA_DECLARE( getAttachedElements )
    {
        // Grab our VM
        CLuaMain* pLuaMain = lua_readcontext( L );

        // Correct type?
        if ( lua_istype ( L, 1, LUA_TLIGHTUSERDATA ) )
        {
            // Grab the element
            CClientEntity* pEntity = lua_toelement ( L, 1 );
            if ( pEntity )
            {
                // Create a new table
                lua_newtable ( L );

                // Add All Attached Elements
                unsigned int uiIndex = 0;
                list < CClientEntity* > ::const_iterator iter = pEntity->AttachedEntitiesBegin ();
                for ( ; iter != pEntity->AttachedEntitiesEnd () ; iter++ )
                {
                    CClientEntity * pAttached = *iter;
                    if ( pAttached->GetAttachedTo () == pEntity )
                    {
                        lua_pushnumber ( L, ++uiIndex );
                        lua_pushelement ( L, *iter );
                        lua_settable ( L, -3 );
                    }
                }
                return 1;
            }
            else
                m_pScriptDebugging->LogBadPointer( "getAttachedElements", "element", 1 );
        }
        else
            m_pScriptDebugging->LogBadType( "getAttachedElements" );

        // Failed
        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( getElementDistanceFromCentreOfMassToBaseOfModel )
    {
        // Valid types?
        if ( lua_istype ( L, 1, LUA_TLIGHTUSERDATA ) )
        {
            // Grab the element to check
            CClientEntity* pEntity = lua_toelement ( L, 1 );
            if ( pEntity )
            {
                // Grab the distance and return it
                float fDistance;
                if ( CStaticFunctionDefinitions::GetElementDistanceFromCentreOfMassToBaseOfModel ( *pEntity, fDistance ) )
                {
                    lua_pushnumber ( L, fDistance );
                    return 1;
                }
            }
            else
                m_pScriptDebugging->LogBadPointer( "getElementDistanceFromCentreOfMassToBaseOfModel", "element", 1 );
        }
        else
            m_pScriptDebugging->LogBadType( "getElementDistanceFromCentreOfMassToBaseOfModel" );

        // Failed
        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( isElementLocal )
    {
        // Check types
        if ( lua_istype ( L, 1, LUA_TLIGHTUSERDATA ) )
        {
            // Grab the element
            CClientEntity* pEntity = lua_toelement ( L, 1 );
            if ( pEntity )
            {
                // Return whether it's local or not
                bool bLocal = pEntity->IsLocalEntity ();;
                lua_pushboolean ( L, bLocal );
                return 1;
            }
            else
                m_pScriptDebugging->LogBadPointer( "isElementLocal", "element", 1 );
        }
        else
            m_pScriptDebugging->LogBadType( "isElementLocal" );

        // Failed
        lua_pushnil ( L );
        return 1;
    }

    LUA_DECLARE( getElementAttachedOffsets )
    {
        if ( lua_type ( L, 1 ) == LUA_TLIGHTUSERDATA )
        {
            // Grab the attached element
            CClientEntity* pEntity = lua_toelement ( L, 1 );
            CVector vecPosition, vecRotation;

            // Valid element?
            if ( pEntity )
            {
                if ( CStaticFunctionDefinitions::GetElementAttachedOffsets ( *pEntity, vecPosition, vecRotation ) )
                {
                    lua_pushnumber( L, vecPosition.fX );
                    lua_pushnumber( L, vecPosition.fY );
                    lua_pushnumber( L, vecPosition.fZ );
                    lua_pushnumber( L, vecRotation.fX );
                    lua_pushnumber( L, vecRotation.fY );
                    lua_pushnumber( L, vecRotation.fZ );
                    return 6;
                }
            }
            else
                m_pScriptDebugging->LogBadPointer( "getElementAttachedOffsets", "element", 1 );
        }
        else
            m_pScriptDebugging->LogBadType( "getElementAttachedOffsets" );

        // Failed
        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( getElementAlpha )
    {
        // Valid type?
        if ( lua_istype ( L, 1, LUA_TLIGHTUSERDATA ) )
        {
            // Grab the element
            CClientEntity* pEntity = lua_toelement ( L, 1 );
            if ( pEntity )
            {
                // Grab its alpha level and return it
                unsigned char ucAlpha;
                if ( CStaticFunctionDefinitions::GetElementAlpha ( *pEntity, ucAlpha ) )
                {
                    lua_pushnumber ( L, ucAlpha );
                    return 1;
                }
            }
            else
                m_pScriptDebugging->LogBadPointer( "getElementAlpha", "element", 1 );
        }
        else
            m_pScriptDebugging->LogBadType( "getElementAlpha" );

        // Failed
        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( getElementHealth )
    {
        // Same type?
        if ( lua_istype ( L, 1, LUA_TLIGHTUSERDATA ) )
        {
            // Get the element
            CClientEntity* pEntity = lua_toelement ( L, 1 );
            if ( pEntity )
            {
                // Grab the health and return it
                float fHealth;
                if ( CStaticFunctionDefinitions::GetElementHealth ( *pEntity, fHealth ) )
                {
                    lua_pushnumber ( L, fHealth );
                    return 1;
                }
            }
            else
                m_pScriptDebugging->LogBadPointer( "getElementHealth", "element", 1 );
        }
        else
            m_pScriptDebugging->LogBadType( "getElementHealth" );

        // Failed
        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( getElementModel )
    {
        // Same type?
        if ( lua_istype ( L, 1, LUA_TLIGHTUSERDATA ) )
        {
            // Get the element
            CClientEntity* pEntity = lua_toelement ( L, 1 );
            if ( pEntity )
            {
                unsigned short usModel;
                if ( CStaticFunctionDefinitions::GetElementModel ( *pEntity, usModel ) )
                {
                    lua_pushnumber ( L, usModel );
                    return 1;
                }
            }
            else
                m_pScriptDebugging->LogBadPointer( "getElementModel", "element", 1 );
        }
        else
            m_pScriptDebugging->LogBadType( "getElementModel" );

        // Failed
        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( getElementColShape )
    {
        if ( lua_type ( L, 1 ) == LUA_TLIGHTUSERDATA )
        {
            // Get the element
            CClientEntity* pEntity = lua_toelement ( L, 1 );
            if ( pEntity )
            {
                CClientEntity* pColShape = static_cast < CClientEntity* > ( CStaticFunctionDefinitions::GetElementColShape ( pEntity ) );
                if ( pColShape )
                {
                    lua_pushelement ( L, pColShape );
                    return 1;
                }
            }
            else
                m_pScriptDebugging->LogBadPointer( "getElementColShape", "element", 1 );
        }
        else
            m_pScriptDebugging->LogBadType( "getElementColShape" );

        // Failed
        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( isElementInWater )
    {
        if ( lua_istype ( L, 1, LUA_TLIGHTUSERDATA ) )
        {
            // Grab the entity and verify it.
            CClientEntity* pEntity = lua_toelement ( L, 1 );
            if ( pEntity )
            {
                bool bInWater = false;
                if ( CStaticFunctionDefinitions::IsElementInWater ( *pEntity, bInWater ) )
                {
                    lua_pushboolean ( L, bInWater );
                    return 1;
                }
            }
        }
        else
            m_pScriptDebugging->LogBadType( "isElementInWater" );

        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( isElementSyncer )
    {
        if ( lua_istype ( L, 1, LUA_TLIGHTUSERDATA ) )
        {
            // Grab the entity and verify it.
            CClientEntity* pEntity = lua_toelement ( L, 1 );
            if ( pEntity )
            {
                bool bIsSyncer = false;
                if ( CStaticFunctionDefinitions::IsElementSyncer ( *pEntity, bIsSyncer ) )
                {
                    lua_pushboolean ( L, bIsSyncer );
                    return 1;
                }
            }
        }
        else
            m_pScriptDebugging->LogBadType( "isElementSyncer" );

        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( isElementCollidableWith )
    {
        if ( lua_istype ( L, 1, LUA_TLIGHTUSERDATA ) &&
             lua_istype ( L, 2, LUA_TLIGHTUSERDATA ) )
        {
            // Grab the entity and verify it.
            CClientEntity* pEntity = lua_toelement ( L, 1 );
            CClientEntity* pWithEntity = lua_toelement ( L, 2 );
            if ( pEntity && pWithEntity )
            {
                bool bCanCollide;
                if ( CStaticFunctionDefinitions::IsElementCollidableWith ( *pEntity, *pWithEntity, bCanCollide ) )
                {
                    lua_pushboolean ( L, bCanCollide );
                    return 1;
                }
            }
        }
        else
            m_pScriptDebugging->LogBadType( "isElementCollidableWith" );

        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( isElementDoubleSided )
    {
        if ( lua_istype ( L, 1, LUA_TLIGHTUSERDATA ) )
        {
            // Grab the entity and verify it.
            CClientEntity* pEntity = lua_toelement ( L, 1 );
            if ( pEntity )
            {
                lua_pushboolean ( L, pEntity->IsDoubleSided () );
                return 1;
            }
        }
        else
            m_pScriptDebugging->LogBadType( "isElementDoubleSided" );

        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( getElementCollisionsEnabled )
    {
        if ( lua_type ( L, 1 ) == LUA_TLIGHTUSERDATA )
        {
            CClientEntity* pEntity = lua_toelement ( L, 1 );
            if ( pEntity )
            {
                if ( CStaticFunctionDefinitions::GetElementCollisionsEnabled ( *pEntity ) )
                {
                    lua_pushboolean ( L, true );
                    return 1;
                }        
            }
            else
                m_pScriptDebugging->LogBadPointer( "getElementCollisionsEnabled", "element", 1 );
        }
        else
            m_pScriptDebugging->LogBadType( "getElementCollisionsEnabled" );

        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( isElementFrozen )
    {
        if ( lua_type ( L, 1 ) == LUA_TLIGHTUSERDATA )
        {
            CClientEntity* pEntity = lua_toelement ( L, 1 );
            if ( pEntity )
            {
                bool bFrozen;
                if ( CStaticFunctionDefinitions::IsElementFrozen ( *pEntity, bFrozen ) )
                {
                    lua_pushboolean ( L, bFrozen );
                    return 1;
                }        
            }
            else
                m_pScriptDebugging->LogBadPointer( "isElementFrozen", "element", 1 );
        }
        else
            m_pScriptDebugging->LogBadType( "isElementFrozen" );

        lua_pushboolean ( L, false );
        return 1;
    }


    LUA_DECLARE( isElementStreamedIn )
    {
        // We have a correct parameter 1?
        if ( lua_istype ( L, 1, LUA_TLIGHTUSERDATA ) )
        {
            // Grab the entity and verify it.
            CClientEntity* pEntity = lua_toelement ( L, 1 );
            if ( pEntity )
            {
                // Is this a streaming compatible class?
                if ( pEntity->IsStreamingCompatibleClass () )
                {
                    CClientStreamElement* pStreamElement = static_cast < CClientStreamElement* > ( pEntity );

                    // Return whether or not this class is streamed in
                    lua_pushboolean ( L, pStreamElement->IsStreamedIn () );
                    return 1;
                }
                else if ( pEntity->GetType() == CCLIENTSOUND )
                {
                    CClientSound* pSound = static_cast < CClientSound* > ( pEntity );
                    lua_pushboolean ( L, pSound->IsSoundStopped() ? false : true );
                    return 1;
                }
                else
                    m_pScriptDebugging->LogWarning( "isElementStreamedIn; element is not streaming compatible\n" );
            }
            else
                m_pScriptDebugging->LogBadPointer( "isElementStreamedIn", "element", 1 );
        }
        else
            m_pScriptDebugging->LogBadType( "isElementStreamedIn" );

        // We failed
        lua_pushnil ( L );
        return 1;
    }

    LUA_DECLARE( isElementStreamable )
    {
        // We have a correct parameter 1?
        if ( lua_istype ( L, 1, LUA_TLIGHTUSERDATA ) )
        {
            // Grab the entity and verify it.
            CClientEntity* pEntity = lua_toelement ( L, 1 );
            if ( pEntity )
            {
                // Is this a streaming compatible class?
                if ( pEntity->IsStreamingCompatibleClass () )
                {
                    CClientStreamElement* pStreamElement = static_cast < CClientStreamElement* > ( pEntity );

                    bool bStreamable = ( pStreamElement->GetTotalStreamReferences () == 0 );
                    // Return whether or not this element is set to never be streamed out
                    lua_pushboolean ( L, bStreamable );
                    return 1;
                }
                else
                    m_pScriptDebugging->LogWarning( "isElementStreamable; element is not streaming compatible\n" );

                // Return false, we're not streamable
                lua_pushboolean ( L, false );
                return 1;
            }
            else
                m_pScriptDebugging->LogBadPointer( "isElementStreamable", "element", 1 );
        }
        else
            m_pScriptDebugging->LogBadType( "isElementStreamable" );

        // We failed
        lua_pushnil ( L );
        return 1;
    }

    LUA_DECLARE( isElementOnScreen )
    {
        // Valid type?
        if ( lua_istype ( L, 1, LUA_TLIGHTUSERDATA ) )
        {
            // Grab the element
            CClientEntity* pEntity = lua_toelement ( L, 1 );
            if ( pEntity )
            {
                // Return whether we're on the screen or not
                bool bOnScreen;
                if ( CStaticFunctionDefinitions::IsElementOnScreen ( *pEntity, bOnScreen ) )
                {
                    lua_pushboolean ( L, bOnScreen );
                    return 1;
                }
            }
            else
                m_pScriptDebugging->LogBadPointer( "isElementOnScreen", "element", 1 );
        }
        else
            m_pScriptDebugging->LogBadType( "isElementOnScreen" );

        lua_pushnil ( L );
        return 1;
    }

    LUA_DECLARE( createElement )
    {
        CLuaMain* pLuaMain = lua_readcontext( L );
        CResource * pResource = pLuaMain->GetResource();

        // Grab the optional name argument
        char szDefaultID [] = "";
        const char* szID = szDefaultID;
        int iArgument2 = lua_type ( L, 2 );
        if ( iArgument2 == LUA_TSTRING )
        {
            szID = lua_tostring ( L, 2 );
        }
        else if ( iArgument2 != LUA_TNONE )
        {
            m_pScriptDebugging->LogBadType( "createElement" );
            lua_pushboolean ( L, false );
            return 1;
        }

        // Verify argument types
        if ( lua_istype ( L, 1, LUA_TSTRING ) )
        {
            // Grab the string
            const char* szTypeName = lua_tostring ( L, 1 );

            // Try to create
            CClientDummy* pDummy = CStaticFunctionDefinitions::CreateElement ( *pResource, szTypeName, szID );
            if ( pDummy )
            {
                // Add it to the element group
                // TODO: Get rid of element groups
                CElementGroup * pGroup = pResource->GetElementGroup();
                if ( pGroup )
                {
                    pGroup->Add ( ( CClientEntity* ) pDummy );
                }

                // Return it
                lua_pushelement ( L, pDummy );
                return 1;
            }
            else
                m_pScriptDebugging->LogError( "createElement; unable to create mor elements\n" );
        }
        else
            m_pScriptDebugging->LogBadType( "createElement" );

        // Failed
        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( destroyElement )
    {
        // Correct type?
        if ( lua_istype ( L, 1, LUA_TLIGHTUSERDATA ) )
        {
            // Grab the element
            CClientEntity* pEntity = lua_toelement ( L, 1 );
            if ( pEntity )
            {
                // Destroy it
                if ( CStaticFunctionDefinitions::DestroyElement ( *pEntity ) )
                {
                    lua_pushboolean ( L, true );
                    return 1;
                }
            }
            else
                m_pScriptDebugging->LogBadPointer( "destroyElement", "element", 1 );
        }
        else
            m_pScriptDebugging->LogBadType( "destroyElement" );

        // Failed
        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( setElementID )
    {
        // Correct type?
        if ( lua_istype ( L, 1, LUA_TLIGHTUSERDATA ) &&
             lua_istype ( L, 2, LUA_TSTRING ) )
        {
            // Grab the element
            CClientEntity* pEntity = lua_toelement ( L, 1 );
            if ( pEntity )
            {
                const char* szID = lua_tostring ( L, 2 );
                // It returns false if we tried to change ID of server-created element
                if ( CStaticFunctionDefinitions::SetElementID ( *pEntity, szID ) )
                {
                    lua_pushboolean ( L, true );
                    return 1;
                }
            }
            else
                m_pScriptDebugging->LogBadPointer( "setElementID", "element", 1 );
        }
        else
            m_pScriptDebugging->LogBadType( "setElementID" );

        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( setElementData )
    {
    //  bool setElementData ( element theElement, string key, var value, [bool synchronize = true] )
        CClientEntity* pEntity; SString strKey; CLuaArgument value; bool bSynchronize;

        CScriptArgReader argStream ( L );
        argStream.ReadUserData ( pEntity );
        argStream.ReadString ( strKey );
        argStream.ReadLuaArgument ( value );
        argStream.ReadBool ( bSynchronize, true );

        if ( !argStream.HasErrors () )
        {
            CLuaMain* pLuaMain = lua_readcontext( L );

            if ( strKey.length () > MAX_CUSTOMDATA_NAME_LENGTH )
            {
                // Warn and truncate if key is too long
                m_pScriptDebugging->LogCustom( SString ( "Truncated argument @ '%s' [%s]", "setElementData", *SString ( "string length reduced to %d characters at argument 2", MAX_CUSTOMDATA_NAME_LENGTH ) ) );
                strKey = strKey.Left ( MAX_CUSTOMDATA_NAME_LENGTH );
            }

            if ( CStaticFunctionDefinitions::SetElementData ( *pEntity, strKey, value, *pLuaMain, bSynchronize ) )
            {
                lua_pushboolean ( L, true );
                return 1;
            }
        }
        else
            m_pScriptDebugging->LogCustom( SString ( "Bad argument @ '%s' [%s]", "setElementData", *argStream.GetErrorMessage () ) );

        // Failed
        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( removeElementData )
    {
    //  bool removeElementData ( element theElement, string key )
        CClientEntity* pEntity; SString strKey;;

        CScriptArgReader argStream ( L );
        argStream.ReadUserData ( pEntity );
        argStream.ReadString ( strKey );

        if ( !argStream.HasErrors () )
        {
            CLuaMain* pLuaMain = lua_readcontext( L );

            if ( strKey.length () > MAX_CUSTOMDATA_NAME_LENGTH )
            {
                // Warn and truncate if key is too long
                m_pScriptDebugging->LogCustom( SString ( "Truncated argument @ '%s' [%s]", "removeElementData", *SString ( "string length reduced to %d characters at argument 2", MAX_CUSTOMDATA_NAME_LENGTH ) ) );
                strKey = strKey.Left ( MAX_CUSTOMDATA_NAME_LENGTH );
            }

            lua_pushboolean( L, CStaticFunctionDefinitions::RemoveElementData ( *pEntity, strKey ) );
            return 1;
        }
        else
            m_pScriptDebugging->LogCustom( SString ( "Bad argument @ '%s' [%s]", "removeElementData", *argStream.GetErrorMessage () ) );

        // Failed
        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( setElementPosition )
    {
        // Correct types?
        int iArgument2 = lua_type ( L, 2 );
        int iArgument3 = lua_type ( L, 3 );
        int iArgument4 = lua_type ( L, 4 );
        if ( ( lua_istype ( L, 1, LUA_TLIGHTUSERDATA ) ) &&
            ( iArgument2 == LUA_TNUMBER || iArgument2 == LUA_TSTRING ) &&
            ( iArgument3 == LUA_TNUMBER || iArgument3 == LUA_TSTRING ) &&
            ( iArgument4 == LUA_TNUMBER || iArgument4 == LUA_TSTRING ) )
        {
            // Grab the element and the position to change to
            CClientEntity* pEntity = lua_toelement ( L, 1 );
            CVector vecPosition ( static_cast < float > ( lua_tonumber ( L, 2 ) ),
                static_cast < float > ( lua_tonumber ( L, 3 ) ),
                static_cast < float > ( lua_tonumber ( L, 4 ) ) );

            bool bWarp = true;
            if ( lua_type ( L, 5 ) == LUA_TBOOLEAN )
                bWarp = lua_toboolean ( L, 5 ) ? true : false;

            // Valid?
            if ( pEntity )
            {
                // Try to set the position
                if ( CStaticFunctionDefinitions::SetElementPosition ( *pEntity, vecPosition, bWarp ) )
                {
                    lua_pushboolean ( L, true );
                    return 1;
                }
            }
            else
                m_pScriptDebugging->LogBadPointer( "setElementPosition", "element", 1 );
        }
        else
            m_pScriptDebugging->LogBadType( "setElementPosition" );

        // Error
        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( setElementRotation )
    {
        // Correct types?
        int iArgument2 = lua_type ( L, 2 );
        int iArgument3 = lua_type ( L, 3 );
        int iArgument4 = lua_type ( L, 4 );
        if ( ( lua_istype ( L, 1, LUA_TLIGHTUSERDATA ) ) &&
            ( iArgument2 == LUA_TNUMBER || iArgument2 == LUA_TSTRING ) &&
            ( iArgument3 == LUA_TNUMBER || iArgument3 == LUA_TSTRING ) &&
            ( iArgument4 == LUA_TNUMBER || iArgument4 == LUA_TSTRING ) )
        {
            const char* szRotationOrder = "default";
            if ( lua_type ( L, 5 ) == LUA_TSTRING ) 
            {
                szRotationOrder = lua_tostring ( L, 5 );
            }

            // Grab the element and the position to change to
            CClientEntity* pEntity = lua_toelement ( L, 1 );
            CVector vecRotation ( static_cast < float > ( lua_tonumber ( L, 2 ) ),
                static_cast < float > ( lua_tonumber ( L, 3 ) ),
                static_cast < float > ( lua_tonumber ( L, 4 ) ) );

            // Valid?
            if ( pEntity )
            {
                // Try to set the position
                if ( CStaticFunctionDefinitions::SetElementRotation ( *pEntity, vecRotation, szRotationOrder ) )
                {
                    lua_pushboolean ( L, true );
                    return 1;
                }
            }
            else
                m_pScriptDebugging->LogBadPointer( "setElementRotation", "element", 1 );
        }
        else
            m_pScriptDebugging->LogBadType( "setElementRotation" );

        // Error
        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( setElementVelocity )
    {
        // Verify the first argument
        if ( lua_istype ( L, 1, LUA_TLIGHTUSERDATA ) )
        {
            // Grab the element
            CClientEntity* pEntity = lua_toelement ( L, 1 );
            if ( pEntity )
            {
                // Check rest of the arguments. Don't mind the last Z argument if the element is 2D
                int iArgument2 = lua_type ( L, 2 );
                int iArgument3 = lua_type ( L, 3 );
                int iArgument4 = lua_type ( L, 4 );
                if ( ( iArgument2 == LUA_TNUMBER || iArgument2 == LUA_TSTRING ) &&
                    ( iArgument3 == LUA_TNUMBER || iArgument3 == LUA_TSTRING ) &&
                    ( iArgument4 == LUA_TNUMBER || iArgument4 == LUA_TSTRING || pEntity->GetType () == CClientGame::RADAR_AREA ) )
                {
                    // Grab the velocity
                    CVector vecVelocity = CVector ( static_cast < float > ( lua_tonumber ( L, 2 ) ),
                        static_cast < float > ( lua_tonumber ( L, 3 ) ),
                        static_cast < float > ( lua_tonumber ( L, 4 ) ) );
                    // Set the velocity
                    if ( CStaticFunctionDefinitions::SetElementVelocity ( *pEntity, vecVelocity ) )
                    {
                        lua_pushboolean ( L, true );
                        return 1;
                    }
                }
                else
                    m_pScriptDebugging->LogBadType( "setElementVelocity" );
            }
            else
                m_pScriptDebugging->LogBadPointer( "setElementVelocity", "element", 1 );
        }
        else
            m_pScriptDebugging->LogBadType( "setElementVelocity" );

        // Failed
        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( setElementParent )
    {
        // Correct args?
        if ( lua_istype ( L, 1, LUA_TLIGHTUSERDATA ) && lua_istype ( L, 2, LUA_TLIGHTUSERDATA ) )
        {
            // Grab entity 
            CClientEntity* pEntity = lua_toelement ( L, 1 );
            if ( pEntity )
            {
                // Grab new parent
                CClientEntity* pParent = lua_toelement ( L, 2 );
                if ( pParent )
                {
                    CLuaMain* pLuaMain = lua_readcontext( L );

                    // Change the parent
                    if ( CStaticFunctionDefinitions::SetElementParent ( *pEntity, *pParent, pLuaMain ) )
                    {
                        lua_pushboolean ( L, true );
                        return 1;
                    }
                }
                else
                    m_pScriptDebugging->LogBadPointer( "setElementParent", "element", 2 );
            }
            else
                m_pScriptDebugging->LogBadPointer( "setElementParent", "element", 1 );
        }
        else
            m_pScriptDebugging->LogBadType( "setElementParent" );

        // Failed
        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( setElementInterior )
    {
        // Correct args?
        int iArgument2 = lua_type ( L, 2 );
        if ( lua_istype ( L, 1, LUA_TLIGHTUSERDATA ) &&
            ( iArgument2 == LUA_TNUMBER || iArgument2 == LUA_TSTRING ) )
        {
            // Grab the element and the interior
            CClientEntity* pEntity = lua_toelement ( L, 1 );
            unsigned char ucInterior = static_cast < unsigned char > ( lua_tonumber ( L, 2 ) );

            // Grab position if supplied
            bool bSetPosition = false;
            CVector vecPosition;

            int iArgument3 = lua_type ( L, 3 );
            int iArgument4 = lua_type ( L, 4 );
            int iArgument5 = lua_type ( L, 5 );
            if ( ( iArgument3 == LUA_TNUMBER || iArgument3 == LUA_TSTRING ) &&
                ( iArgument4 == LUA_TNUMBER || iArgument4 == LUA_TSTRING ) &&
                ( iArgument5 == LUA_TNUMBER || iArgument5 == LUA_TSTRING ) )
            {
                vecPosition = CVector ( static_cast < float > ( lua_tonumber ( L, 3 ) ),
                    static_cast < float > ( lua_tonumber ( L, 4 ) ),
                    static_cast < float > ( lua_tonumber ( L, 5 ) ) );
                bSetPosition = true;
            }

            // Valid element?
            if ( pEntity )
            {
                // Set the interior
                if ( CStaticFunctionDefinitions::SetElementInterior ( *pEntity, ucInterior, bSetPosition, vecPosition ) )
                {
                    lua_pushboolean ( L, true );
                    return 1;
                }
            }
            else
                m_pScriptDebugging->LogBadPointer( "setElementInterior", "element", 1 );
        }
        else
            m_pScriptDebugging->LogBadType( "setElementInterior" );

        // Failed
        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( setElementDimension )
    {
        // Correct args?
        int iArgument2 = lua_type ( L, 2 );
        if ( lua_istype ( L, 1, LUA_TLIGHTUSERDATA ) &&
            ( iArgument2 == LUA_TNUMBER || iArgument2 == LUA_TSTRING ) )
        {
            // Grab the element and the dimension
            CClientEntity* pEntity = lua_toelement ( L, 1 );
            unsigned short usDimension = static_cast < unsigned int > ( lua_tonumber ( L, 2 ) );

            // Valid?
            if ( pEntity )
            {
                // Change the dimension
                if ( CStaticFunctionDefinitions::SetElementDimension ( *pEntity, usDimension ) )
                {
                    lua_pushboolean ( L, true );
                    return 1;
                }
            }
            else
                m_pScriptDebugging->LogBadPointer( "setElementDimension", "element", 1 );
        }
        else
            m_pScriptDebugging->LogBadType( "setElementDimension" );

        // Failed
        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( attachElements )
    {
        // Valid arguments?
        if ( lua_type ( L, 1 ) == LUA_TLIGHTUSERDATA &&
            lua_type ( L, 2 ) == LUA_TLIGHTUSERDATA )
        {
            // Grab the element to attach and the target
            CClientEntity* pEntity = lua_toelement ( L, 1 );
            CClientEntity* pAttachedToEntity = lua_toelement ( L, 2 );
            CVector vecPosition, vecRotation;

            // Grab the supplied arguments (pos: x y z, rot: x y z)
            int iArgument3 = lua_type ( L, 3 );
            if ( iArgument3 == LUA_TSTRING || iArgument3 == LUA_TNUMBER )
            {
                vecPosition.fX = static_cast < float > ( lua_tonumber ( L, 3 ) );

                int iArgument4 = lua_type ( L, 4 );
                if ( iArgument4 == LUA_TSTRING || iArgument4 == LUA_TNUMBER )
                {
                    vecPosition.fY = static_cast < float > ( lua_tonumber ( L, 4 ) );

                    int iArgument5 = lua_type ( L, 5 );
                    if ( iArgument5 == LUA_TSTRING || iArgument5 == LUA_TNUMBER )
                    {
                        vecPosition.fZ = static_cast < float > ( lua_tonumber ( L, 5 ) );

                        int iArgument6 = lua_type ( L, 6 );
                        if ( iArgument6 == LUA_TSTRING || iArgument6 == LUA_TNUMBER )
                        {
                            vecRotation.fX = static_cast < float > ( lua_tonumber ( L, 6 ) );

                            int iArgument7 = lua_type ( L, 7 );
                            if ( iArgument7 == LUA_TSTRING || iArgument7 == LUA_TNUMBER )
                            {
                                vecRotation.fY = static_cast < float > ( lua_tonumber ( L, 7 ) );

                                int iArgument8 = lua_type ( L, 8 );
                                if ( iArgument8 == LUA_TSTRING || iArgument8 == LUA_TNUMBER )
                                {
                                    vecRotation.fZ = static_cast < float > ( lua_tonumber ( L, 8 ) );
                                }
                            }
                        }
                    }
                }
            }

            // Valid element?
            if ( pEntity )
            {
                // Valid host element?
                if ( pAttachedToEntity )
                {
                    // Try to attach them
                    if ( CStaticFunctionDefinitions::AttachElements ( *pEntity, *pAttachedToEntity, vecPosition, vecRotation ) )
                    {
                        lua_pushboolean ( L, true );
                        return 1;
                    }
                }
                else
                    m_pScriptDebugging->LogBadPointer( "attachElements", "element", 2 );
            }
            else
                m_pScriptDebugging->LogBadPointer( "attachElements", "element", 1 );
        }
        else
            m_pScriptDebugging->LogBadType( "attachElements" );

        // Failed
        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( detachElements )
    {
        // Valid arg type?
        if ( lua_istype ( L, 1, LUA_TLIGHTUSERDATA ) )
        {
            // Grab the element that's attached to something
            CClientEntity* pEntity = lua_toelement ( L, 1 );

            // Grab the other element if supplied
            CClientEntity* pAttachedToEntity = NULL;
            if ( lua_istype ( L, 2, LUA_TLIGHTUSERDATA ) )
                pAttachedToEntity = lua_toelement ( L, 2 );

            // Valid?
            if ( pEntity )
            {
                // Try to detach
                if ( CStaticFunctionDefinitions::DetachElements ( *pEntity, pAttachedToEntity ) )
                {
                    lua_pushboolean ( L, true );
                    return 1;
                }
            }
            else
                m_pScriptDebugging->LogBadPointer( "detachElements", "element", 1 );
        }
        else
            m_pScriptDebugging->LogBadType( "detachElements" );

        // Failure
        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( setElementAttachedOffsets )
    {
        if ( lua_type ( L, 1 ) == LUA_TLIGHTUSERDATA )
        {
            // Grab the element to attach and the target
            CClientEntity* pEntity = lua_toelement ( L, 1 );
            CVector vecPosition, vecRotation;

            // Grab the supplied arguments (pos: x y z, rot: x y z)
            int iArgument2 = lua_type ( L, 2 );
            if ( iArgument2 == LUA_TSTRING || iArgument2 == LUA_TNUMBER )
            {
                vecPosition.fX = static_cast < float > ( lua_tonumber ( L, 2 ) );

                int iArgument3 = lua_type ( L, 3 );
                if ( iArgument3 == LUA_TSTRING || iArgument3 == LUA_TNUMBER )
                {
                    vecPosition.fY = static_cast < float > ( lua_tonumber ( L, 3 ) );

                    int iArgument4 = lua_type ( L, 4 );
                    if ( iArgument4 == LUA_TSTRING || iArgument4 == LUA_TNUMBER )
                    {
                        vecPosition.fZ = static_cast < float > ( lua_tonumber ( L, 4 ) );

                        int iArgument5 = lua_type ( L, 5 );
                        if ( iArgument5 == LUA_TSTRING || iArgument5 == LUA_TNUMBER )
                        {
                            vecRotation.fX = static_cast < float > ( lua_tonumber ( L, 5 ) );

                            int iArgument6 = lua_type ( L, 6 );
                            if ( iArgument6 == LUA_TSTRING || iArgument6 == LUA_TNUMBER )
                            {
                                vecRotation.fY = static_cast < float > ( lua_tonumber ( L, 6 ) );

                                int iArgument7 = lua_type ( L, 7 );
                                if ( iArgument7 == LUA_TSTRING || iArgument7 == LUA_TNUMBER )
                                {
                                    vecRotation.fZ = static_cast < float > ( lua_tonumber ( L, 7 ) );
                                }
                            }
                        }
                    }
                }
            }

            // Valid element?
            if ( pEntity )
            {
                if ( CStaticFunctionDefinitions::SetElementAttachedOffsets ( *pEntity, vecPosition, vecRotation ) )
                {
                    lua_pushboolean ( L, true );
                    return 1;
                }
            }
            else
                m_pScriptDebugging->LogBadPointer( "setElementAttachedOffsets", "element", 1 );
        }
        else
            m_pScriptDebugging->LogBadType( "setElementAttachedOffsets" );

        // Failed
        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( setElementCollisionsEnabled )
    {
        if ( lua_type ( L, 1 ) == LUA_TLIGHTUSERDATA &&
            lua_type ( L, 2 ) == LUA_TBOOLEAN )
        {
            CClientEntity* pEntity = lua_toelement ( L, 1 );
            if ( pEntity )
            {
                bool bEnabled = ( lua_toboolean ( L, 2 ) ) ? true:false;
                if ( CStaticFunctionDefinitions::SetElementCollisionsEnabled ( *pEntity, lua_toboolean ( L, 2 ) ? true:false ) )
                {
                    lua_pushboolean ( L, true );    
                    return 1;
                }        
            }
            else
                m_pScriptDebugging->LogBadPointer( "setElementCollisionsEnabled", "element", 1 );
        }
        else
            m_pScriptDebugging->LogBadType( "setElementCollisionsEnabled" );

        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( setElementCollidableWith )
    {
        if ( lua_type ( L, 1 ) == LUA_TLIGHTUSERDATA &&
             lua_type ( L, 2 ) == LUA_TLIGHTUSERDATA &&
             lua_type ( L, 3 ) == LUA_TBOOLEAN )
        {
            CClientEntity* pEntity = lua_toelement ( L, 1 );
            CClientEntity* pWithEntity = lua_toelement ( L, 2 );
            if ( pEntity && pWithEntity )
            {
                bool bCanCollide = ( lua_toboolean ( L, 3 ) ) ? true:false;
                if ( CStaticFunctionDefinitions::SetElementCollidableWith ( *pEntity, *pWithEntity, bCanCollide ) )
                {
                    lua_pushboolean ( L, true );    
                    return 1;
                }        
            }
            else
                m_pScriptDebugging->LogBadPointer( "setElementCollidableWith", "element", 1 );
        }
        else
            m_pScriptDebugging->LogBadType( "setElementCollidableWith" );

        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( setElementDoubleSided )
    {
        // Valid args?
        if ( lua_istype ( L, 1, LUA_TLIGHTUSERDATA ) &&
            lua_istype ( L, 2, LUA_TBOOLEAN ) )
        {
            // Grab the element to change
            CClientEntity* pEntity = lua_toelement ( L, 1 );
            if ( pEntity )
            {
                // Grab the chosen value and set it
                pEntity->SetDoubleSided ( lua_toboolean ( L, 2 ) ? true : false );
                lua_pushboolean ( L, true );
                return 1;
            }
            else
                m_pScriptDebugging->LogBadPointer( "setElementDoubleSided", "element", 1 );
        }
        else
            m_pScriptDebugging->LogBadType( "setElementDoubleSided" );

        // Failure
        lua_pushboolean ( L, false );
        return 1;
    }


    LUA_DECLARE( setElementFrozen )
    {
        if ( lua_istype ( L, 1, LUA_TLIGHTUSERDATA ) && lua_istype ( L, 2, LUA_TBOOLEAN ) )
        {
            CClientEntity* pEntity = lua_toelement ( L, 1 );
            if ( pEntity )
            {
                bool bFrozen = lua_toboolean ( L, 2 ) ? true : false;

                if ( CStaticFunctionDefinitions::SetElementFrozen ( *pEntity, bFrozen ) )
                {
                    lua_pushboolean ( L, true );
                    return 1;
                }
            }
            else
                m_pScriptDebugging->LogBadPointer( "setElementFrozen", "element", 1 );
        }
        else
            m_pScriptDebugging->LogBadType( "setElementFrozen" );

        // Failure
        lua_pushboolean ( L, false );
        return 1;
    }


    LUA_DECLARE( setElementAlpha )
    {
        // Valid args?
        int iArgument2 = lua_type ( L, 2 );
        if ( lua_istype ( L, 1, LUA_TLIGHTUSERDATA ) &&
            ( iArgument2 == LUA_TSTRING || iArgument2 == LUA_TNUMBER ) )
        {
            // Grab the element to change the alpha of
            CClientEntity* pEntity = lua_toelement ( L, 1 );
            if ( pEntity )
            {
                // Grab the chosen alpha and set it
                unsigned char ucAlpha = static_cast < unsigned char > ( lua_tonumber ( L, 2 ) );
                if ( CStaticFunctionDefinitions::SetElementAlpha ( *pEntity, ucAlpha ) )
                {
                    lua_pushboolean ( L, true );
                    return 1;
                }
            }
            else
                m_pScriptDebugging->LogBadPointer( "setElementAlpha", "element", 1 );
        }
        else
            m_pScriptDebugging->LogBadType( "setElementAlpha" );

        // Failure
        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( setElementHealth )
    {
        // Check the args
        int iArgument2 = lua_type ( L, 2 );
        if ( lua_istype ( L, 1, LUA_TLIGHTUSERDATA ) &&
            ( iArgument2 == LUA_TSTRING || iArgument2 == LUA_TNUMBER ) )
        {
            // Grab the element to change the health of
            CClientEntity* pEntity = lua_toelement ( L, 1 );
            if ( pEntity )
            {
                // Try to change the health
                float fHealth = static_cast < float > ( lua_tonumber ( L, 2 ) );
                if ( CStaticFunctionDefinitions::SetElementHealth ( *pEntity, fHealth ) )
                {
                    lua_pushboolean ( L, true );
                    return 1;
                }
            }
            else
                m_pScriptDebugging->LogBadPointer( "setElementHealth", "element", 1 );
        }
        else
            m_pScriptDebugging->LogBadType( "setElementHealth" );

        // Failed
        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( setElementModel )
    {
        // Check the args
        int iArgument2 = lua_type ( L, 2 );
        if ( lua_istype ( L, 1, LUA_TLIGHTUSERDATA ) &&
            ( iArgument2 == LUA_TSTRING || iArgument2 == LUA_TNUMBER ) )
        {
            // Grab the element to change the health of
            CClientEntity* pEntity = lua_toelement ( L, 1 );
            if ( pEntity )
            {
                // Try to change the health
                unsigned short usModel = static_cast < unsigned short > ( lua_tonumber ( L, 2 ) );
                if ( CStaticFunctionDefinitions::SetElementModel ( *pEntity, usModel ) )
                {
                    lua_pushboolean ( L, true );
                    return 1;
                }
            }
            else
                m_pScriptDebugging->LogBadPointer( "setElementModel", "element", 1 );
        }
        else
            m_pScriptDebugging->LogBadType( "setElementModel" );

        // Failed
        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( setElementStreamable )
    {
        // We have a correct parameter 1 and 2?
        if ( lua_istype ( L, 1, LUA_TLIGHTUSERDATA ) &&
            lua_istype ( L, 2, LUA_TBOOLEAN ) )
        {
            // Grab the entity and verify it.
            CClientEntity* pEntity = lua_toelement ( L, 1 );
            if ( pEntity )
            {
                // Is this a streaming compatible class?
                if ( pEntity->IsStreamingCompatibleClass () )
                {
                    CClientStreamElement* pStreamElement = static_cast < CClientStreamElement* > ( pEntity );

                    // Set this object's streamable state.
                    bool bStreamable = lua_toboolean ( L, 2 ) ? true : false;

                    // TODO: maybe use a better VM-based reference system (rather than a boolean one)
                    if ( bStreamable && ( pStreamElement->GetStreamReferences ( true ) > 0 ) )
                    {
                        pStreamElement->RemoveStreamReference ( true );
                    }
                    else if ( !bStreamable && ( pStreamElement->GetStreamReferences ( true ) == 0 ) )
                    {
                        pStreamElement->AddStreamReference ( true );
                    }
                    else
                    {
                        lua_pushboolean ( L, false );
                        return 1;
                    }

                    // Return success
                    lua_pushboolean ( L, true );
                    return 1;
                }
            }
            else
                m_pScriptDebugging->LogBadPointer( "setElementStreamable", "element", 1 );
        }
        else
            m_pScriptDebugging->LogBadType( "setElementStreamable" );

        // We failed
        lua_pushboolean ( L, false );
        return 1;
    }
}