/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.0
*               (Shared logic for modifications)
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        mods/shared_logic/lua/CLuaFunctionDefs.Resource.cpp
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
    LUA_DECLARE( call )
    {
        // Grab our VM
        CLuaMain* pLuaMain = lua_readcontext( L );

        // Grab this resource
        CResource* pThisResource = pLuaMain->GetResource ();
        if ( pThisResource )
        {
            // Typechecking
            if ( lua_istype ( L, 1, LUA_TLIGHTUSERDATA ) &&
                lua_istype ( L, 2, LUA_TSTRING ) )
            {
                // Grab the resource
                CResource* pResource = lua_toresource ( L, 1 );
                if ( pResource )
                {
                    //Get the target Lua VM
                    lua_State* targetL = *pResource->GetVM();

                    // The function name
                    const char* szFunctionName = lua_tostring ( L, 2 );

                    // Read out the vargs
                    CLuaArguments args;
                    args.ReadArguments ( L, 3 );
                    CLuaArguments returns;

                    //Lets grab the original hidden variables so we can restore them later
                    lua_getglobal ( targetL, "sourceResource" );
                    CLuaArgument OldResource ( L, -1 );
                    lua_pop( targetL, 1 );

                    lua_getglobal ( targetL, "sourceResourceRoot" );
                    CLuaArgument OldResourceRoot ( L, -1 );
                    lua_pop( targetL, 1 );

                    //Set the new values for the current sourceResource, and sourceResourceRoot
                    lua_pushresource ( targetL, pThisResource );
                    lua_setglobal ( targetL, "sourceResource" );

                    lua_pushelement ( targetL, pThisResource->GetResourceEntity() );
                    lua_setglobal ( targetL, "sourceResourceRoot" );

                    // Call the exported function with the given name and the args
                    if ( pResource->CallExportedFunction ( szFunctionName, args, returns, *pThisResource ) )
                    {
                        // Push return arguments
                        returns.PushArguments ( L );
                        //Restore the old variables
                        OldResource.Push ( targetL );
                        lua_setglobal ( targetL, "sourceResource" );

                        OldResourceRoot.Push ( targetL );
                        lua_setglobal ( targetL, "sourceResourceRoot" );

                        return returns.Count ();
                    }
                    else
                    {
                        //Restore the old variables
                        OldResource.Push ( targetL );
                        lua_setglobal ( targetL, "sourceResource" );

                        OldResourceRoot.Push ( targetL );
                        lua_setglobal ( targetL, "sourceResourceRoot" );
                        m_pScriptDebugging->LogError( "call: failed to call '%s:%s'", pResource->GetName (), szFunctionName );
                    }
                }
                else
                {
                    m_pScriptDebugging->LogBadPointer( "call", "resource", 1 );
                }
            }
            else
            {
                m_pScriptDebugging->LogBadType( "call" );
            }
        }

        // Failed
        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( getThisResource )
    {
        lua_readcontext( L )->GetResource()->PushStack( L );
        return 1;
    }

    LUA_DECLARE( getResourceConfig )
    {
        if ( lua_istype ( L, 1, LUA_TLIGHTUSERDATA ) )
            m_pScriptDebugging->LogCustom( "getResourceConfig may be using an outdated syntax. Please check and update." );

        // Resource and config name
        CResource* pResource = NULL;
        const char* szInput = NULL;
        filePath strAbsPath;
        const char *meta;

        if ( lua_istype ( L, 1, LUA_TSTRING ) )
        {
            // Grab our lua main
            CLuaMain* pLuaMain = lua_readcontext( L );

            // Grab resource and the config name from arg
            pResource = pLuaMain->GetResource ();
            szInput = lua_tostring ( L, 1 );
        }

        // We have both a resource file to grab the config from and a config name?
        if ( pResource && szInput )
        {
            if ( m_pResourceManager->ParseResourceFullPath( (Resource*&)pResource, szInput, meta, strAbsPath ) )
            {
                // Loop through the configs in that resource
                list < CResourceConfigItem* >::iterator iter = pResource->ConfigIterBegin ();
                for ( ; iter != pResource->ConfigIterEnd (); iter++ )
                {
                    // Matching name?
                    if ( strcmp ( (*iter)->GetShortName(), meta ) == 0 )
                    {
                        // Return it
                        CResourceConfigItem* pConfig = (CResourceConfigItem*) (*iter);
                        CXMLNode* pNode = pConfig->GetRoot ();
                        if ( pNode )
                        {
                            lua_pushxmlnode ( L, pNode );
                            return 1;
                        }
                    }
                }
            }
        }
        else
            m_pScriptDebugging->LogBadType( "getResourceConfig" );

        // Failed
        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( getResourceName )
    {
        // Verify arguments
        if ( CResource *res = lua_readclass( L, 1, LUACLASS_RESOURCE ) )
        {
            // Grab its name and return it
            lua_pushstring( L, pResource->GetName() );
            return 1;
        }
        else
            m_pScriptDebugging->LogBadPointer( "getResourceName", "resource", 1 );

        // Failed
        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( getResourceFromName )
    {
        // Verify arguments
        if ( lua_isstring( L, 1 ) )
        {
            // Grab the argument
            const char* szResource = lua_tostring ( L, 1 );

            // Try to find a resource with that name
            CResource* pResource = (CResource*)m_pResourceManager->Get( szResource );
            if ( pResource )
            {
                pResource->PushStack( L )
                return 1;
            }
        }
        else
            m_pScriptDebugging->LogBadType( "getResourceFromName" );

        // Failed
        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( getResourceRootElement )
    {
        // Resource given?
        CResource *pResource;
        if ( lua_istype( L, 1, LUA_TCLASS ) )
        {
            pResource = lua_readclass( L, 1, LUACLASS_RESOURCE );

            if ( !pResource )
                goto error;
        }

        // No resource given, get this resource's root
        else if ( lua_istype ( L, 1, LUA_TNONE ) )
        {
            // Find our vm and get the root
            CLuaMain* pLuaMain = lua_readcontext( L );
            pResource = pLuaMain->GetResource ();
        }
        else
        {
error:
            m_pScriptDebugging->LogBadPointer( __FUNCTION__, "resource", 1 );

            lua_pushboolean( L, false );
            return 1;
        }

        // Grab the root element of it and return it if it existed
        pResource->GetResourceEntity()->PushStack( L );
        return 1;
    }

    LUA_DECLARE( getResourceGUIElement )
    {
        // Resource given?
        CResource *pResource;
        if ( lua_istype( L, 1, LUA_TCLASS ) )
        {
            pResource = lua_readclass( L, 1, LUACLASS_RESOURCE );
        }

        // No resource given, get this resource's root
        else if ( lua_istype ( L, 1, LUA_TNONE ) )
        {
            // Find our vm and get the root
            CLuaMain* pLuaMain = lua_readcontext( L );
            pResource = pLuaMain->GetResource ();
        }
        else
        {
            m_pScriptDebugging->LogBadPointer( __FUNCTION__, "resource", 1 );
            lua_pushboolean( L, false );
            return 1;
        }

        pResource->GetResourceGUIEntity()->PushStack( L );
        return 1;
    }

    LUA_DECLARE( getResourceDynamicElementRoot )
    {
        if ( CResource *res = lua_readclass( L, 1, LUACLASS_RESOURCE ) )
        {
            CClientEntity *pEntity = res->GetResourceDynamicEntity();
            if ( pEntity )
            {
                pEntity->PushStack( L );
                return 1;
            }
            else
                m_pScriptDebugging->LogError( "getResourceDynamicElementRoot: Resource %s Is Not Currently Running", pResource->GetName() );
        }
        else
            m_pScriptDebugging->LogBadPointer( "getResourceDynamicElementRoot", "resource", 1 );

        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( getResourceExportedFunctions )
    {
        CResource *resource = NULL;
        
        // resource
        if ( lua_istype( L, 1, LUA_TCLASS ) )
            resource = lua_readclass( L, 1, LUACLASS_RESOURCE );
        else if ( lua_istype( L, 1, LUA_TNONE ) )
        {
            CLuaMain* pLuaMain = lua_readcontext( L );
            resource = pLuaMain->GetResource ();
        }

        if ( resource )
        {
            lua_newtable ( L );
            unsigned int uiIndex = 0;
            CResource::exports_t::iterator iterd = resource->IterBeginExportedFunctions();
            for ( ; iterd != resource->IterEndExportedFunctions(); iterd++ )
            {
                lua_pushnumber ( L, ++uiIndex );
                lua_pushstring ( L, (*iterd)->GetFunctionName().c_str() );
                lua_settable ( L, -3 );
            }
            return 1;
        }
        
        m_pScriptDebugging->LogBadType( "getResourceExportedFunctions" );
        lua_pushboolean ( L, false );
        return 1;
    }
}