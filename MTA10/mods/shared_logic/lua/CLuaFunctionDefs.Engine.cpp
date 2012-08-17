/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*               (Shared logic for modifications)
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        mods/shared_logic/lua/CLuaFunctionDefs.Engine.cpp
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
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#include "StdInc.h"

extern CGame *g_pGame;

namespace CLuaFunctionDefs
{
    LUA_DECLARE( engineLoadCOL )
    {
        CResource *res = lua_readcontext( L )->GetResource();

        CFile *file = m_pResourceManager->OpenStream( res, lua_isstring( L, 1 ) ? lua_tostring( L, 1 ) : "" , "rb" );

        if ( file )
        {
            // Create the col model
            CClientColModel *pCol = new CClientColModel( *res->GetResourceCOLModelRoot() );

            // Attempt loading the file
            if ( pCol->LoadCol( file ) )
            {
                delete file;

                pCol->PushStack( L );
                return 1;
            }
            else
                pCol->Destroy();

            delete file;
        }
        else
            m_pScriptDebugging->LogBadType( "engineLoadCOL" );

        // We failed for some reason
        lua_pushboolean( L, false );
        return 1;
    }

    LUA_DECLARE( engineLoadDFF )
    {
        const char *path;
        unsigned short num;

        CScriptArgReader argStream( L );
        
        argStream.ReadString( path );
        argStream.ReadNumber( num );
        
        if ( !argStream.HasErrors() )
        {
            CResource *res = lua_readcontext( L )->GetResource();

            CFile *file = m_pResourceManager->OpenStream( res, path, "rb" );

            CModel *model = g_pGame->GetModelManager()->CreateModel( file, num );

            delete file;

            if ( model )
            {
                // Create a DFF element
                CClientDFF *dff = new CClientDFF( *res->GetResourceDFFRoot(), *model );
                dff->PushStack( L );
                return 1;
            }
            else
                m_pScriptDebugging->LogError( "failed to load .dff file" );
        }
        else
            m_pScriptDebugging->LogCustom( SString( "Bad argument @ '" __FUNCTION__ "' [%s]", *argStream.GetErrorMessage() ) );

        // We failed
        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( engineLoadTXD )
    {
        return 0;

        const char *path;
        bool filtering;

        CScriptArgReader argStream( L );

        argStream.ReadString( path );
        argStream.ReadBool( filtering, true );

        if ( !argStream.HasErrors() )
        {
            CResource *res = lua_readcontext( L )->GetResource();

            CTexDictionary *dict = g_pGame->GetTextureManager()->CreateTxd( ExtractFilename( path ).c_str() );

            if ( !dict )
            {
                lua_pushboolean( L, false );
                return 1;
            }

            // Create a TXD element
            CClientTXD *pTXD = new CClientTXD( *res->GetResourceTXDRoot(), *dict );

            // Open a new fileStream
            CFile *file = res->OpenStream( path, "rb" );

            // Try to load the TXD file
            bool success = pTXD->LoadTXD( file, filtering );

            delete file;

            if ( success )
            {
                // Return the TXD
                pTXD->PushStack( L );
                return 1;
            }

            // Delete it again
            pTXD->Delete();
        }
        else
            m_pScriptDebugging->LogCustom( SString( "Bad argument @ '" __FUNCTION__ "' [%s]", *argStream.GetErrorMessage() ) );

        // We failed
        lua_pushboolean( L, false );
        return 1;
    }

    LUA_DECLARE( engineReplaceCOL )
    {
        // Grab the COL and model ID
        CClientColModel* pCol = lua_readclass <CClientColModel> ( L, 1, LUACLASS_COLMODEL );
        unsigned short usModel = lua_tointeger( L, 2 );

        if ( pCol )
        {
            // Replace the colmodel
            lua_pushboolean( L, pCol->Replace( usModel ) );
            return 1;
        }
        else
            m_pScriptDebugging->LogBadPointer( "engineReplaceCOL", "col", 1 );

        // Failed
        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( engineRestoreCOL )
    {
        if ( lua_isnumber( L, 1 ) )
        {
            lua_pushboolean( L, g_pGame->GetModelManager()->RestoreCollision( (unsigned short)lua_tonumber( L, 1 ) ) );
            return 1;
        }
        else
            m_pScriptDebugging->LogBadType( "engineRestoreCOL" );

        // Failed.
        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( engineImportTXD )
    {
        return 0;

        CClientTXD *txd = lua_readclass <CClientTXD> ( L, 1, LUACLASS_TXD );
        unsigned short usModelID = lua_tointeger( L, 2 );

        if ( txd )
        {
            // Try to import
            if ( txd->Import( usModelID ) )
            {
                // Success
                lua_pushboolean( L, true );
                return 1;
            }
            else
                m_pScriptDebugging->LogBadPointer( "engineImportTXD", "number", 2 );
        }
        else
            m_pScriptDebugging->LogBadPointer( "engineImportTXD", "txd", 1 );

        // Failed
        lua_pushboolean( L, false );
        return 1;
    }

    LUA_DECLARE( engineReplaceModel )
    {
        CClientDFF *dff = lua_readclass <CClientDFF> ( L, 1, LUACLASS_DFF );

        if ( dff )
        {
            lua_pushboolean( L, dff->ReplaceModel( lua_tointeger( L, 2 ) ) );
            return 1;
        }
        else
            m_pScriptDebugging->LogBadPointer( "engineReplaceModel", "dff", 1 );

        // Failure
        lua_pushboolean( L, false );
        return 1;
    }

    LUA_DECLARE( engineRestoreModel )
    {
        // Restore the model
        if ( g_pGame->GetModelManager()->RestoreModel( lua_tointeger( L, 1 ) ) )
        {
            // Success
            lua_pushboolean( L, true );
            return 1;
        }

        m_pScriptDebugging->LogBadType( "engineRestoreModel" );

        // Failure
        lua_pushboolean( L, false );
        return 1;
    }

    LUA_DECLARE( engineSetModelLODDistance )
    {
        if ( lua_isnumber( L, 1 ) && lua_isnumber( L, 2 ) )
        {
            unsigned short usModelID = (unsigned short)lua_tonumber( L, 1 );
            float fDistance = (float)lua_tonumber( L, 2 );

            CModelInfo *pModelInfo = g_pGame->GetModelInfo( usModelID );

            if ( pModelInfo && fDistance > 0.0f )
            {
                pModelInfo->SetLODDistance( fDistance );
                lua_pushboolean( L, true );
                return 1;
            }
        }

        lua_pushboolean( L, false );
        return 1;
    }

    LUA_DECLARE( engineSetAsynchronousLoading )
    {
        if ( lua_type( L, 1 ) == LUA_TBOOLEAN )
        {
            bool bEnabled = lua_toboolean( L, 1 ) == 1;
            bool bForced = lua_toboolean( L, 2 ) == 1;

            g_pGame->SetAsyncLoadingFromScript( bEnabled, bForced );
            lua_pushboolean( L, true );
            return 1;
        }

        lua_pushboolean( L, false );
        return 1;
    }

    // TODO: int CLuaFunctionDefs::EnginePositionAtomic ( lua_State* L )
    LUA_DECLARE( enginePositionAtomic )
    {
        lua_pushboolean ( L, false );
        return 1;
    }

    // TODO: int CLuaFunctionDefs::EnginePositionSeats ( lua_State* L )
    LUA_DECLARE( enginePositionSeats )
    {
        lua_pushboolean ( L, false );
        return 1;
    }

    // TODO: int CLuaFunctionDefs::EngineAddAllAtomics ( lua_State* L )
    LUA_DECLARE( engineAddAllAtomics )
    {
        lua_pushboolean ( L, false );
        return 1;
    }

    // TODO: int CLuaFunctionDefs::EngineReplaceVehiclePart ( lua_State* L )
    LUA_DECLARE( engineReplaceVehiclePart )
    {
        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( engineApplyShaderToWorldTexture )
    {
    //  bool engineApplyShaderToWorldTexture ( element shader, string textureName, float orderPriority )
        CClientShader* pShader; SString strTextureNameMatch; float fOrderPriority;

        CScriptArgReader argStream ( L );
        argStream.ReadClass( pShader, LUACLASS_SHADER );
        argStream.ReadString ( strTextureNameMatch );
        argStream.ReadNumber ( fOrderPriority, 0 );

        if ( !argStream.HasErrors () )
        {
            bool bResult = g_pCore->GetGraphics ()->GetRenderItemManager ()->ApplyShaderItemToWorldTexture ( pShader->GetShaderItem (), strTextureNameMatch, fOrderPriority );
            lua_pushboolean ( L, bResult );
            return 1;
        }
        else
            m_pScriptDebugging->LogCustom( SString ( "Bad argument @ '%s' [%s]", "engineApplyShaderToWorldTexture", *argStream.GetErrorMessage () ) );

        // We failed
        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( engineRemoveShaderFromWorldTexture )
    {
    //  bool engineRemoveShaderFromWorldTexture ( element shader, string textureName )
        CClientShader* pShader; SString strTextureNameMatch;

        CScriptArgReader argStream ( L );
        argStream.ReadClass( pShader, LUACLASS_SHADER );
        argStream.ReadString ( strTextureNameMatch );

        if ( !argStream.HasErrors () )
        {
            bool bResult = g_pCore->GetGraphics ()->GetRenderItemManager ()->RemoveShaderItemFromWorldTexture ( pShader->GetShaderItem (), strTextureNameMatch );
            lua_pushboolean ( L, bResult );
            return 1;
        }
        else
            m_pScriptDebugging->LogCustom( SString ( "Bad argument @ '%s' [%s]", "engineRemoveShaderFromWorldTexture", *argStream.GetErrorMessage () ) );

        // We failed
        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( engineGetModelNameFromID )
    {
    //  string engineGetModelNameFromID ( int modelID )
        int iModelID;

        CScriptArgReader argStream ( L );
        argStream.ReadNumber ( iModelID );

        if ( !argStream.HasErrors () )
        {
            SString strModelName = CModelNames::GetModelName ( iModelID );
            if ( !strModelName.empty () )
            {
                lua_pushstring ( L, strModelName );
                return 1;
            }
            m_pScriptDebugging->LogCustom( SString ( "Bad argument @ '%s' [%s]", "engineGetModelNameFromID", "Expected valid model ID at argument 1" ) );
        }
        else
            m_pScriptDebugging->LogCustom( SString ( "Bad argument @ '%s' [%s]", "engineGetModelNameFromID", *argStream.GetErrorMessage () ) );

        // We failed
        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( engineGetModelIDFromName )
    {
        //  int engineGetModelIDFromName ( string modelName )
        SString strModelName;

        CScriptArgReader argStream ( L );
        argStream.ReadString ( strModelName );

        if ( !argStream.HasErrors () )
        {
            int iModelID = CModelNames::GetModelID ( strModelName );
            if ( iModelID )
            {
                lua_pushnumber ( L, iModelID );
                return 1;
            }
            m_pScriptDebugging->LogCustom( SString ( "Bad argument @ '%s' [%s]", "engineGetModelIDFromName", "Expected valid model name at argument 1" ) );
        }
        else
            m_pScriptDebugging->LogCustom( SString ( "Bad argument @ '%s' [%s]", "engineGetModelIDFromName", *argStream.GetErrorMessage () ) );

        // We failed
        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( engineGetModelTextureNames )
    {
    //  table engineGetModelTextureNames ( string modelName )
        SString strModelName;

        CScriptArgReader argStream ( L );
        argStream.ReadString ( strModelName, "" );

        if ( !argStream.HasErrors () )
        {
            ushort usModelID = CModelNames::ResolveModelID ( strModelName );
            if ( usModelID )
            {
                std::vector < SString > nameList;
                g_pGame->GetTextureManager()->GetModelTextureNames ( nameList, usModelID );

                lua_newtable ( L );
                for ( uint i = 0 ; i < nameList.size () ; i++ )
                {                
                    lua_pushnumber ( L, i + 1 );
                    lua_pushstring ( L, nameList [ i ] );
                    lua_settable ( L, -3 );
                }
                return 1;
            }
            m_pScriptDebugging->LogCustom( SString ( "Bad argument @ '%s' [%s]", __FUNCTION__, "Expected valid model ID or name at argument 1" ) );
        }
        else
            m_pScriptDebugging->LogCustom( SString ( "Bad argument @ '%s' [%s]", __FUNCTION__, *argStream.GetErrorMessage () ) );

        // We failed
        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( engineGetVisibleTextureNames )
    {
    //  table engineGetVisibleTextureNames ( string wildcardMatch = "*" [, string modelName )
        SString strTextureNameMatch; SString strModelName;

        CScriptArgReader argStream ( L );
        argStream.ReadString ( strTextureNameMatch, "*" );
        argStream.ReadString ( strModelName, "" );

        if ( !argStream.HasErrors () )
        {
            ushort usModelID = CModelNames::ResolveModelID ( strModelName );
            if ( usModelID || strModelName == "" )
            {
                std::vector < SString > nameList;
                g_pCore->GetGraphics ()->GetRenderItemManager ()->GetVisibleTextureNames ( nameList, strTextureNameMatch, usModelID );

                lua_newtable ( L );
                for ( uint i = 0 ; i < nameList.size () ; i++ )
                {                
                    lua_pushnumber ( L, i + 1 );
                    lua_pushstring ( L, nameList [ i ] );
                    lua_settable ( L, -3 );
                }
                return 1;
            }
            m_pScriptDebugging->LogCustom( SString ( "Bad argument @ '%s' [%s]", __FUNCTION__, "Expected valid model ID or name at argument 1" ) );
        }
        else
            m_pScriptDebugging->LogCustom( SString ( "Bad argument @ '%s' [%s]", __FUNCTION__, *argStream.GetErrorMessage () ) );

        // We failed
        lua_pushboolean ( L, false );
        return 1;
    }
}