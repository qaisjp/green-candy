/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.0
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
*
*****************************************************************************/

#include "StdInc.h"

extern CGame *g_pGame;

namespace CLuaFunctionDefs
{
    LUA_DECLARE( engineLoadCOL )
    {
        CLuaMain* pLuaMain = lua_readcontext( L );
        CResource* pResource = pLuaMain->GetResource ();

        CFile *file = m_pResourceManager->OpenStream( pResource, lua_istype ( L, 1, LUA_TSTRING ) ? lua_tostring ( L, 1 ) : "" , "rb" );

        if ( file )
        {
            // TODO: plant the COL root

            // Create the col model
            CClientColModel *pCol = new CClientColModel( *pLuaMain->GetResource() );

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
        CLuaMain* pLuaMain = lua_readcontext( L );
        CResource* pResource = pLuaMain->GetResource ();
        SString strFile = ( lua_istype ( L, 1, LUA_TSTRING ) ? lua_tostring ( L, 1 ) : "" );
        
        filePath strPath;
        const char *meta;
        // Is this a legal filepath?
        if ( lua_istype ( L, 2, LUA_TNUMBER ) && m_pResourceManager->ParseResourceFullPath( (Resource*&)pResource, strFile, meta, strPath ) )
        {
            CModel *model = g_pGame->GetModelManager()->CreateModel( strPath.c_str(), lua_tointeger( L, 2 ) );

            if ( !model )
                goto error;

            // TODO: plant the DFF root

            // Create a DFF element
            CClientDFF *dff = new CClientDFF( *lua_readcontext( L )->GetResource(), *model );
            dff->PushStack( L );
            return 1;

error:
            m_pScriptDebugging->LogBadPointer( "engineLoadDFF", "number", 2 );
        }
        else
            m_pScriptDebugging->LogBadPointer( "engineLoadDFF", "string", 1 );

        // We failed
        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( engineLoadTXD )
    {
        CLuaMain* pLuaMain = lua_readcontext( L );
        CResource* pResource = pLuaMain->GetResource ();
        bool bFilteringEnabled = true;

        if ( lua_type ( L, 2 ) == LUA_TBOOLEAN )
            bFilteringEnabled = ( lua_toboolean ( L, 2 ) ) ? true:false;

        // Grab the filename
        SString strFile = ( lua_istype( L, 1, LUA_TSTRING ) ? lua_tostring( L, 1 ) : "" );
        
        filePath strPath;
        const char *meta;
        // Is this a legal filepath?
        if ( m_pResourceManager->ParseResourceFullPath( (Resource*&)pResource, strFile, meta, strPath ) )
        {
            CTexDictionary *dict = g_pGame->GetTextureManager()->CreateTxd( ExtractFilename( strPath ).c_str() );

            if ( !dict )
            {
                lua_pushboolean( L, false );
                return 1;
            }
            //TODO: plant TXDRoot

            // Create a TXD element
            CClientTXD *pTXD = new CClientTXD( *lua_readcontext( L )->GetResource(), *dict );

            // Try to load the TXD file
            if ( pTXD->LoadTXD( strPath, bFilteringEnabled ) )
            {
                // Return the TXD
                pTXD->PushStack( L );
                return 1;
            }

            // Delete it again
            delete pTXD;
        }
        else
            m_pScriptDebugging->LogBadPointer( "engineLoadTXD", "string", 1 );

        // We failed
        lua_pushboolean( L, false );
        return 1;
    }

    LUA_DECLARE( engineReplaceCOL )
    {
        // Grab the DFF and model ID
        CClientColModel* pCol = ( lua_istype ( L, 1, LUA_TLIGHTUSERDATA ) ? lua_tocolmodel ( L, 1 ) : NULL );
        unsigned short usModel = ( lua_istype ( L, 2, LUA_TNUMBER ) ? ( static_cast < unsigned short > ( lua_tonumber ( L, 2 ) ) ) : 0 );

        // Valid collision model?
        if ( pCol )
        {
            // Valid client DFF and model?
            if ( CClientColModelManager::IsReplacableModel ( usModel ) )
            {
                // Replace the colmodel
                if ( pCol->Replace ( usModel ) )
                {
                    lua_pushboolean ( L, true );
                    return 1;
                }
            }
            else
                m_pScriptDebugging->LogBadPointer( "engineReplaceCOL", "number", 2 );
        }
        else
            m_pScriptDebugging->LogBadPointer( "engineReplaceCOL", "col", 1 );

        // Failed
        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( engineRestoreCOL )
    {
        // Grab the model ID we're going to restore
        int iArgument1 = lua_type ( L, 1 );
        if ( iArgument1 == LUA_TNUMBER || iArgument1 == LUA_TSTRING )
        {
            unsigned short usModelID = static_cast < unsigned short > ( lua_tonumber ( L, 1 ) );

            if ( m_pColModelManager->RestoreModel ( usModelID ) )
            {
                // Success
                lua_pushboolean ( L, true );
                return 1;
            }  
        }
        else
            m_pScriptDebugging->LogBadType( "engineRestoreCOL" );

        // Failed.
        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( engineImportTXD )
    {
        if ( lua_type( L, 1 ) != LUA_TCLASS )
            goto typeError;

        ILuaClass& j = *lua_refclass( L, 1 );
        CClientTXD *pTXD;

        if ( !j.GetTransmit( LUACLASS_TXD, (void*&)pTXD ) )
            goto typeError;

        // Grab the TXD and the model ID
        unsigned short usModelID = lua_istype( L, 2, LUA_TNUMBER ) ? lua_tointeger( L, 2 ) : 0;

        // Try to import
        if ( pTXD->Import( usModelID ) )
        {
            // Success
            lua_pushboolean( L, true );
            return 1;
        }

        m_pScriptDebugging->LogBadPointer( "engineImportTXD", "number", 2 );
        goto error;

typeError:
        m_pScriptDebugging->LogBadPointer( "engineImportTXD", "txd", 1 );

error:
        // Failed
        lua_pushboolean( L, false );
        return 1;
    }

    LUA_DECLARE( engineReplaceModel )
    {
        if ( lua_type( L, 1 ) != LUA_TCLASS )
            goto typeError;

        ILuaClass& j = *lua_refclass( L, 1 );
        CClientDFF *dff;

        if ( !j.GetTransmit( LUACLASS_DFF, (void*&)dff ) )
            goto typeError;

        lua_pushboolean( L, dff->ReplaceModel( lua_istype( L, 2, LUA_TNUMBER ) ? lua_tointeger( L, 2 ) : 0 ) );
        return 1;

typeError:
        m_pScriptDebugging->LogBadPointer( "engineReplaceModel", "dff", 1 );

        // Failure
        lua_pushboolean( L, false );
        return 1;
    }

    LUA_DECLARE( engineRestoreModel )
    {
        // Restore the model
        if ( g_pGame->GetModelManager()->RestoreModel( lua_istype( L, 1, LUA_TNUMBER ) ? lua_tointeger( L, 1 ) : 0 ) )
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
        int iArgument1 = lua_type ( L, 1 );
        int iArgument2 = lua_type ( L, 2 );
        if ( ( iArgument1 == LUA_TNUMBER || iArgument2 == LUA_TSTRING ) &&
            ( iArgument2 == LUA_TNUMBER || iArgument2 == LUA_TSTRING ) )
        {
            unsigned short usModelID = static_cast < unsigned short > ( lua_tonumber ( L, 1 ) );
            float fDistance = static_cast < float > ( lua_tonumber ( L, 2 ) );
            CModelInfo* pModelInfo = g_pGame->GetModelInfo ( usModelID );
            if ( pModelInfo && fDistance > 0.0f )
            {
                pModelInfo->SetLODDistance ( fDistance );
                lua_pushboolean ( L, true );
                return 1;
            }
        }

        lua_pushboolean ( L, false );
        return 1;
    }

    LUA_DECLARE( engineSetAsynchronousLoading )
    {
        int iArgument1 = lua_type ( L, 1 );
        int iArgument2 = lua_type ( L, 2 );
        if ( ( iArgument1 == LUA_TBOOLEAN ) &&
            ( iArgument2 == LUA_TBOOLEAN || iArgument2 == LUA_TNONE ) )
        {
            bool bEnabled = lua_toboolean ( L, 1 ) ? true : false;
            bool bForced = iArgument2 == LUA_TBOOLEAN && lua_toboolean ( L, 2 );
            g_pGame->SetAsyncLoadingFromScript ( bEnabled, bForced );
            lua_pushboolean ( L, true );
            return 1;
        }

        lua_pushboolean ( L, false );
        return 1;
    }

    // TODO: int CLuaFunctionDefs::EngineReplaceMatchingAtomics ( lua_State* L )
    LUA_DECLARE( engineReplaceMatchingAtomics )
    {
        /*
        RpClump * pClump = ( lua_istype ( 1, LUA_TLIGHTUSERDATA ) ? reinterpret_cast < RpClump* > ( lua_touserdata ( L, 1 ) ) : NULL );
        CClientEntity* pEntity = ( lua_istype ( 2, LUA_TLIGHTUSERDATA ) ? reinterpret_cast < CClientEntity* > ( lua_touserdata ( L, 2 ) ) : NULL );
        RpAtomicContainer Atomics[MAX_ATOMICS_PER_CLUMP];
        RpClump * pEntityClump = NULL;
        unsigned int uiAtomics = 0;

        if ( pEntity ) {
        if ( IS_VEHICLE ( pEntity ) )
        pEntityClump = static_cast < CClientVehicle* > ( pEntity ) -> GetGameVehicle () -> GetRpClump ();
        else if ( IS_OBJECT ( pEntity ) )
        pEntityClump = static_cast < CClientObject* > ( pEntity ) -> GetGameObject () -> GetRpClump ();
        else {
        m_pScriptDebugging->LogWarning( "engineReplaceMatchingAtomics only supports vehicles and objects." );
        m_pScriptDebugging->LogBadType( "engineReplaceMatchingAtomics" );
        }
        }

        if ( pEntityClump && pClump ) {
        uiAtomics = m_pRenderWare->LoadAtomics ( pClump, &Atomics[0] );
        m_pRenderWare->ReplaceAllAtomicsInClump ( pEntityClump, &Atomics[0], uiAtomics );

        lua_pushboolean ( L, true );
        } else {
        m_pScriptDebugging->LogBadType( "engineReplaceMatchingAtomics" );
        lua_pushboolean ( L, false );
        }
        */

        lua_pushboolean ( L, false );
        return 1;
    }

    // TODO: int CLuaFunctionDefs::EngineReplaceWheelAtomics ( lua_State* L )
    LUA_DECLARE( engineReplaceWheelAtomics )
    {
        /*
        RpClump * pClump = ( lua_istype ( 1, LUA_TLIGHTUSERDATA ) ? reinterpret_cast < RpClump* > ( lua_touserdata ( L, 1 ) ) : NULL );
        CClientEntity* pEntity = ( lua_istype ( 2, LUA_TLIGHTUSERDATA ) ? reinterpret_cast < CClientEntity* > ( lua_touserdata ( L, 2 ) ) : NULL );
        const char * szWheel = ( lua_istype ( 3, LUA_TSTRING ) ? lua_tostring ( L, 3 ) : NULL );

        RpAtomicContainer Atomics[MAX_ATOMICS_PER_CLUMP];
        RpClump * pEntityClump = NULL;
        unsigned int uiAtomics = 0;

        if ( pEntity ) {
        if ( IS_VEHICLE ( pEntity ) )
        pEntityClump = static_cast < CClientVehicle* > ( pEntity ) -> GetGameVehicle () -> GetRpClump ();
        else if ( IS_OBJECT ( pEntity ) )
        pEntityClump = static_cast < CClientObject* > ( pEntity ) -> GetGameObject () -> GetRpClump ();
        else {
        m_pScriptDebugging->LogWarning( "engineReplaceWheelAtomics only supports vehicles and objects." );
        }
        }

        if ( pClump ) {
        uiAtomics = m_pRenderWare->LoadAtomics ( pClump, &Atomics[0] );
        m_pScriptDebugging->LogWarning( "engineReplaceWheelAtomics DFF argument was not valid." );
        }

        if ( pEntityClump && uiAtomics > 0 && szWheel ) {   
        m_pRenderWare->ReplaceWheels ( pEntityClump, &Atomics[0], uiAtomics, szWheel );

        lua_pushboolean ( L, true );
        } else {
        m_pScriptDebugging->LogBadType( "engineReplaceWheelAtomics" );
        lua_pushboolean ( L, false );
        }
        */

        lua_pushboolean ( L, false );
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
        argStream.ReadUserData ( pShader );
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
        argStream.ReadUserData ( pShader );
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
                g_pGame->GetRenderWare ()->GetModelTextureNames ( nameList, usModelID );

                lua_newtable ( L );
                for ( uint i = 0 ; i < nameList.size () ; i++ )
                {                
                    lua_pushnumber ( L, i + 1 );
                    lua_pushstring ( L, nameList [ i ] );
                    lua_settable ( L, -3 );
                }
                return 1;
            }
            m_pScriptDebugging->LogCustom( SString ( "Bad argument @ '%s' [%s]", "engineGetModelTextureNames", "Expected valid model ID or name at argument 1" ) );
        }
        else
            m_pScriptDebugging->LogCustom( SString ( "Bad argument @ '%s' [%s]", "engineGetModelTextureNames", *argStream.GetErrorMessage () ) );

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
            m_pScriptDebugging->LogCustom( SString ( "Bad argument @ '%s' [%s]", "engineGetVisibleTextureNames", "Expected valid model ID or name at argument 1" ) );
        }
        else
            m_pScriptDebugging->LogCustom( SString ( "Bad argument @ '%s' [%s]", "engineGetVisibleTextureNames", *argStream.GetErrorMessage () ) );

        // We failed
        lua_pushboolean ( L, false );
        return 1;
    }
}