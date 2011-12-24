/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        Shared/logic/LuaMain.cpp
*  PURPOSE:     Lua hyperstructure
*       This is a child environment of the LuaManager virtual machine.
*  DEVELOPERS:  Kevin Whiteside <kevuwk@gmail.com>
*               Cecill Etheredge <ijsf@gmx.net>
*               Derek Abdine <>
*               Jax <>
*               Ed Lyons <eai@opencoding.net>
*               The_GTA <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#include <StdInc.h>

LuaMain::LuaMain( LuaManager& manager )
{
    m_system = manager;
    m_lua = manager.m_lua;
}

LuaMain::~LuaMain()
{
    // TODO: Kill hyperstructure out of management VM
    luaL_unref( m_lua, LUA_REGISTRYINDEX, m_meta );
}

// Custom Lua stack argument->reference routine
CLuaFunctionRef LuaMain::CreateReference( int stack )
{
    const void *ptr = lua_topointer( m_lua, stack );

    if ( CRefInfo *info = m_refStore[ptr] )
    {
        // Re-use the lua ref we already have
        info->refCount++;
        return LuaFunctionRef( this, info->idx, ptr );
    }

    // Get a lua ref
    lua_settop( m_lua, stack );
    int ref = lua_ref( m_lua, 1 );

    // Save ref info
    CRefInfo info;
    info.refCount = 1;
    info.idx = ref;
    m_refStore[ptr] = info;

    return LuaFunctionRef( this, ref, ptr );
}

void LuaMain::Reference( const LuaFunctionRef& ref )
{
    CRefInfo *iref = MapFind( m_refStore, ref.m_call );

    if ( !iref )
        return;

    iref->refCount++;
}

void LuaMain::Dereference( const LuaFunctionRef& ref )
{
    CRefInfo *iref = MapFind( m_refStore, ref.m_call );

    if ( !iref )
        return;

    if ( --iref->refCount == 0 )
    {
        // Remove on last unuse
        lua_unref( m_lua, iref->idx );

        MapRemove( m_refStore, pFuncPtr );
        MapRemove( m_tagStore, ref.m_ref );
    }
}

void LuaMain::RegisterFunction( const char *name, lua_CFunction *proto )
{
    lua_pushcclosure( m_lua, proto, 0 );
    lua_setfield( m_lua, m_structure, name );
}

void LuaMain::CallStack( int args )
{
    m_system.PushStatus( *this );

    m_system.CallStack( args );

    m_system.PopStatus();
}

void LuaMain::CallStackVoid( int args )
{
    m_system.PushStatus( *this );
    
    m_system.CallStackVoid( args );

    m_system.PopStatus();
}

LuaArguments LuaMain::CallStackResult( int args )
{
    m_system.PushStatus( *this );

    LuaArguments args = m_system.CallStackResult( args );

    m_system.PopStatus();
    return args;
}

void LuaMain::InitSecurity()
{
    // TODO: Safe implementation of these routines
    RegisterFunction( "dofile", LuaFunctionDefs::DisabledFunction );
    RegisterFunction( "loadfile", LuaFunctionDefs::DisabledFunction );
    RegisterFunction( "require", LuaFunctionDefs::DisabledFunction );
    RegisterFunction( "loadlib", LuaFunctionDefs::DisabledFunction );
    RegisterFunction( "getfenv", LuaFunctionDefs::DisabledFunction );
    RegisterFunction( "newproxy", LuaFunctionDefs::DisabledFunction );
}

void LuaMain::InitVM( int structure, int meta )
{
    m_global = m_system.GetGlobalEnvironment();

    m_structure = structure;
    m_meta = meta;

    luaL_ref( m_lua, meta );
#if 0
    // Initialize security restrictions
    InitSecurity();

    // Register module functions
    CLuaCFunctions::RegisterFunctionsWithVM ( m_luaVM );

    // Update global variables
    lua_pushelement ( m_luaVM, g_pClientGame->GetRootEntity () );
    lua_setglobal ( m_luaVM, "root" );

    lua_pushresource ( m_luaVM, m_pResource );
    lua_setglobal ( m_luaVM, "resource" );

    lua_pushelement ( m_luaVM, m_pResource->GetResourceEntity () );
    lua_setglobal ( m_luaVM, "resourceRoot" );

    lua_pushelement ( m_luaVM, m_pResource->GetResourceGUIEntity () );
    lua_setglobal ( m_luaVM, "guiRoot" );

    lua_pushelement ( m_luaVM, g_pClientGame->GetLocalPlayer() );
    lua_setglobal ( m_luaVM, "localPlayer" );

    // Load pre-loaded lua code
    LoadScript ( szPreloadedScript );
#endif
}

bool CLuaMain::LoadScriptFromBuffer ( const char* cpBuffer, unsigned int uiSize, const char* szFileName, bool bUTF8 )
{
    // Are we not marked as UTF-8 already, and not precompiled?
    std::string strUTFScript;
    if ( !bUTF8 && ( uiSize < 5 || cpBuffer[0] != 27 || cpBuffer[1] != 'L' || cpBuffer[2] != 'u' || cpBuffer[3] != 'a' || cpBuffer[4] != 'Q' ) )
    {
        std::string strBuffer = std::string(cpBuffer, uiSize);
        strUTFScript = UTF16ToMbUTF8(ANSIToUTF16( strBuffer ));
        if ( uiSize != strUTFScript.size() )
        {
            uiSize = strUTFScript.size();
            g_pClientGame->GetScriptDebugging()->LogWarning ( m_luaVM, "Script '%s' is not encoded in UTF-8.  Loading as ANSI...", ConformResourcePath(szFileName).c_str() );
        }
    }
    else
        strUTFScript = std::string(cpBuffer, uiSize);

    // Run the script
    if ( luaL_loadbuffer( m_luaVM, bUTF8 ? cpBuffer : strUTFScript.c_str(), uiSize, SString ( "@%s", szFileName ) ) )
    {
        // Print the error
        std::string strRes = ConformResourcePath ( lua_tostring( m_luaVM, -1 ) );
        if ( strRes.length () )
        {
            CLogger::LogPrintf ( "SCRIPT ERROR: %s\n", strRes.c_str () );
            g_pClientGame->GetScriptDebugging()->LogWarning ( m_luaVM, "Loading script failed: %s", strRes.c_str () );
        }
        else
        {
            CLogger::LogPrint ( "SCRIPT ERROR: Unknown\n" );
            g_pClientGame->GetScriptDebugging()->LogInformation ( m_luaVM, "Loading script failed for unknown reason" );
        }
        return false;
    }
    ResetInstructionCount();

    int iret = lua_pcall ( m_luaVM, 0, 0, 0 ) ;
    if ( iret == LUA_ERRRUN || iret == LUA_ERRMEM )
    {
        SString strRes = ConformResourcePath ( lua_tostring( m_luaVM, -1 ) );

        std::vector <SString> vecSplit;
        strRes.Split ( ":", vecSplit );
        
        if ( vecSplit.size ( ) >= 3 )
        {
            SString strFile = vecSplit[0];
            int     iLine   = atoi ( vecSplit[1].c_str ( ) );
            SString strMsg  = vecSplit[2].substr ( 1 );
            
            g_pClientGame->GetScriptDebugging()->LogError ( strFile, iLine, strMsg );
        }
        else
            g_pClientGame->GetScriptDebugging()->LogError ( m_luaVM, "%s", strRes.c_str () );
    }
    return true;
}

bool LuaMain::LoadScript( const char *buf )
{
    // Run the script
    if ( luaL_loadbuffer( m_lua, buf, strlen( buf ), NULL ) != 0 )
    {
        m_system.GetDebug().LogError( m_lua, "Loading in-line script failed: %s", lua_tostring( m_lua, -1 ) );
        return false;
    }
    ResetInstructionCount();

    switch( lua_pcall( m_lua, 0, 0, 0 ) )
    {
    case LUA_ERRRUN:
    case LUA_ERRMEM:
        m_system.GetDebug().LogError( m_lua, "Executing in-line script failed: %s", lua_tostring( m_lua, -1 ) );
        return false;
    }

    return true;
}

void LuaMain::DoPulse()
{
    m_timers.DoPulse( this );
}

void LuaMain::DestroyXML( CXMLFile *file )
{
    m_XMLFiles.remove( file );
    delete file;
}

void CLuaMain::DestroyXML( CXMLNode *root )
{
    std::list <CXMLFile*>::iterator iter = m_XMLFiles.begin();

    for ( ; iter != m_XMLFiles.end(); iter++ )
    {
        if ( (*iter)->GetRootNode() == root )
        {
            DestroyXML( *iter );
            return;
        }
    }
}

const SString& LuaMain::GetFunctionTag( int refID )
{
    // Find existing
    SString* pTag = MapFind( m_tagStore, refID );
#ifndef MTA_DEBUG
    if ( !pTag )
#endif
    {
        // Create if required
        SString strText;

        lua_Debug debugInfo;
        lua_getref( m_lua, refID );

        if ( lua_getinfo( m_lua, ">nlS", &debugInfo ) )
        {
            filePath src;

            // Make sure this function isn't defined in a string
            if ( debugInfo.source[0] == '@' && ParseRelative( debugInfo.source, src ) )
                strText = SString( "@%s:%d", src.GetFilename(), debugInfo.currentline != -1 ? debugInfo.currentline : debugInfo.linedefined, refID );
            else
                strText = SString( "@func_%d %s", refID, debugInfo.short_src );
        }
        else
            strText = SString( "@func_%d NULL", iLuaFunction );

#ifdef MTA_DEBUG
        if ( pTag )
        {
            // Check tag remains unchanged
            assert( strText == *pTag );
            return *pTag;
        }
#endif

        m_tagStore[refID] = strText;
        pTag = &m_tagStore[refID];
    }
    return *pTag;
}