/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        mods/deathmatch/logic/CResource.cpp
*  PURPOSE:     Resource object
*  DEVELOPERS:  Kevin Whiteside <kevuwk@gmail.com>
*               Cecill Etheredge <ijsf@gmx.net>
*               Jax <>
*               Chris McArthur <>
*               Ed Lyons <eai@opencoding.net>
*               The_GTA <quiret@gmx.de>
*               
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#include "StdInc.h"
#include "CServerIdManager.h"

extern CClientGame      *g_pClientGame;
extern CCoreInterface   *g_pCore;

int CResource::m_refShowCursor = 0;

CResource::CResource( unsigned short id, const filePath& name, CFileTranslator& root, CClientEntity *entity, CClientEntity *dynamicEntity ) : Resource( *g_pClientGame->GetLuaManager()->Create( name.c_str(), root ), id, name, root )
{
    m_luaManager = g_pClientGame->GetLuaManager();
    m_inDownQueue = false;
    m_showCursor = false;

    m_rootEntity = g_pClientGame->GetRootEntity();
    m_defaultGroup = new CElementGroup( this );
    m_elementGroups.push_back( m_defaultGroup ); // for use by scripts

    m_resourceEntity = entity;
    m_dynamicEntity = dynamicEntity;

    // Make sure we reference the dependencies!
    m_resourceEntity->IncrementMethodStack();
    m_dynamicEntity->IncrementMethodStack();

    lua_State *L = *m_lua;

    // Create our root elements. We set their parents when we're loaded.
    // Make them system entities so nothing but us can delete them.
    m_guiEntity = new CClientDummy( g_pClientGame->GetManager(), INVALID_ELEMENT_ID, "guiroot", *m_dynamicEntity, true );
    m_colEntity = new CClientDummy( g_pClientGame->GetManager(), INVALID_ELEMENT_ID, "colmodelroot", *m_dynamicEntity, true );
    m_dffEntity = new CClientDummy( g_pClientGame->GetManager(), INVALID_ELEMENT_ID, "dffroot", *m_dynamicEntity, true );
    m_txdEntity = new CClientDummy( g_pClientGame->GetManager(), INVALID_ELEMENT_ID, "txdroot", *m_dynamicEntity, true );

    // Register them to our resource
    PushEnvironment( L );
    m_guiEntity->PushStack( L ); lua_setfield( L, -2, "guiRoot" );
    m_colEntity->PushStack( L ); lua_setfield( L, -2, "colRoot" );
    m_dffEntity->PushStack( L ); lua_setfield( L, -2, "dffRoot" );
    m_txdEntity->PushStack( L ); lua_setfield( L, -2, "txdRoot" );
    lua_pop( L, 1 );

    // Reference the entities so nothing can destroy them
    m_guiEntity->IncrementMethodStack();
    m_colEntity->IncrementMethodStack();
    m_dffEntity->IncrementMethodStack();
    m_txdEntity->IncrementMethodStack();

    // Setup lua globals
    entity->PushStack( L );
    lua_setglobal( L, "resourceRoot" );

    m_guiEntity->PushStack( L );
    lua_setglobal( L, "guiRoot" );

    // Set up our private path
    filePath privPath;
    ResourceManager::resFileRoot->GetFullPath( CServerIdManager::GetSingleton()->GetConnectionPrivateDirectory() + name, false, privPath );

    m_privateRoot = g_pCore->GetFileSystem()->CreateTranslator( privPath.c_str() );
}

CResource::~CResource()
{
    // Notify the environment that we quit.
    PushStack( *m_lua );
    m_resourceEntity->CallEvent( "onClientResourceStop", *m_lua, 1, true );

    // Make sure we don't force the cursor on
    ShowCursor( false );

    // Do this before we delete our elements.
    m_rootEntity->CleanUpForVM( (CLuaMain*)&m_lua, true );

    // Remove all keybinds on this VM
    g_pClientGame->GetScriptKeyBinds()->RemoveAllKeys( (CLuaMain*)&m_lua );
    g_pCore->GetKeyBinds()->SetAllCommandsActive( m_name.c_str(), false );

    // Destroy all entities, since we do not need them anymore
    // First flag for deletion, then destroy; otherway round might result in destruction prior to flagging due to other runtimes -> access violation
    m_txdEntity->Delete(); m_txdEntity->DecrementMethodStack();
    m_dffEntity->Delete(); m_dffEntity->DecrementMethodStack();
    m_colEntity->Delete(); m_colEntity->DecrementMethodStack();
    m_guiEntity->Delete(); m_guiEntity->DecrementMethodStack();
    m_dynamicEntity->Delete(); m_dynamicEntity->DecrementMethodStack();     // We delete the map root with the resource
    m_resourceEntity->Delete(); m_resourceEntity->DecrementMethodStack();

    // All script dependencies should be gone by deleting the entities; we can safely terminate the thread!
    g_pClientGame->GetLuaManager()->Remove( &m_lua );

    // Undo all changes to water
    g_pGame->GetWaterManager()->UndoChanges( this );

    // TODO: remove them from the core too!!
    // Destroy all the element groups attached directly to this resource
    groupList_t::iterator itere = m_elementGroups.begin();

    for ( ; itere != m_elementGroups.end(); itere++ )
        delete *itere;

    m_defaultGroup = NULL;

    m_rootEntity = NULL;
    m_resourceEntity = NULL;

    // Clean up memory!
    fileList_t::iterator iter = m_files.begin();
    for ( ; iter != m_files.end(); iter++ )
        delete *iter;

    configList_t::iterator iterc = m_configFiles.begin();
    for ( ; iterc != m_configFiles.end(); iterc++ )
        delete *iterc;

    exports_t::iterator iterex = m_exports.begin();
    for ( ; iterex != m_exports.end(); iterex++ )
        delete *iterex;
}

CDownloadableResource* CResource::QueueFile ( CDownloadableResource::eResourceType resourceType, const char *szFileName, CChecksum serverChecksum )
{
    filePath path;

    if ( !m_fileRoot.GetFullPathFromRoot( szFileName, true, path ) )
        return NULL;

    CResourceFile *info = new CResourceFile( resourceType, szFileName, path.c_str(), serverChecksum );

    if ( !info )
        return NULL;

    m_files.push_back( info );
    return info;
}

CDownloadableResource* CResource::AddConfigFile ( const char *szFileName, CChecksum serverChecksum )
{
    filePath path;
    
    if ( !m_fileRoot.GetFullPathFromRoot( szFileName, true, path ) )
        return NULL;
    
    CResourceConfigItem *info = new CResourceConfigItem( this, szFileName, path.c_str(), serverChecksum );

    if ( !info )
        return NULL;

    m_configFiles.push_back( info );
    return info;
}

void CResource::AddExportedFunction( const char *name )
{
    m_exports.push_back( new CExportedFunction( name ) );
}

void CResource::SetResourceEntity( CClientEntity *entity )
{
    // If there was a previous entity, HACKS
    // Well, we unreference it so nothing breaks
    if ( m_resourceEntity )
        m_resourceEntity->DecrementMethodStack();

    // Reference this important entity so it does not get lost!
    entity->IncrementMethodStack();

    m_resourceEntity = entity;
}

//
// Quick integrity check of png, dff and txd files
//
static bool CheckFileForCorruption( std::string strPath )
{
    const char* szExt   = strPath.c_str () + max<long>( 0, strPath.length () - 4 );
    bool bIsBad         = false;

    if ( stricmp ( szExt, ".PNG" ) == 0 )
    {
        // Open the file
        if ( CFile *file = resFileRoot->Open( strPath.c_str(), "rb" ) )
        {
            // This is what the png header should look like
            const unsigned char pGoodHeader[8] = { 0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A };

             // Load the header
            unsigned char pBuffer[8];

            if ( !file->ReadStruct( pBuffer ) )
                bIsBad = true;

            // Check header integrity
            if ( memcmp( pBuffer, pGoodHeader, 8 ) )
                bIsBad = true;

            delete file;
        }
    }
    else if ( stricmp ( szExt, ".TXD" ) == 0 || stricmp ( szExt, ".DFF" ) == 0 )
    {
        // HACK: Shouldn't this belong into the renderware system?
        if ( CFile *file = resFileRoot->Open( strPath.c_str(), "rb" ) )
        {
            struct {
                long id;
                long size;
                long ver;
            } header = {0,0,0};

            // Load the first header
            file->ReadStruct( header );

            long pos = sizeof(header);
            long validSize = header.size + pos;

            // Step through the sections
            while( pos < validSize )
            {
                if ( !file->ReadStruct( header ) )
                    break;

                file->Seek( header.size, SEEK_CUR );
                pos += header.size + sizeof(header);
            }

            // Check integrity
            if ( pos != validSize )
                bIsBad = true;
               
            // Close the file
            delete file;
        }
    }

    return bIsBad;
}

void CResource::Load()
{
    CLogger::LogPrintf( "> Starting resource '%s'\n", m_name.c_str() );

    configList_t::iterator iterc = m_configFiles.begin();
    for ( ; iterc != m_configFiles.end(); iterc++ )
    {
        if ( !(*iterc)->Start() )
            CLogger::LogPrintf( "Failed to start resource item %s in %s\n", (*iterc)->GetName(), m_name.c_str() );
    }

    // Load the files that are queued in the list "to be loaded"
    fileList_t::iterator iter = m_files.begin();
    for ( ; iter != m_files.end(); iter++ )
    {
        CResourceFile *file = *iter;

        // Only load the resource file if it is a client script
        if ( file->GetResourceType() == CDownloadableResource::RESOURCE_FILE_TYPE_CLIENT_SCRIPT )
        {
            // Load the file
            std::vector <char> buffer;
            
            if ( !m_fileRoot.ReadToBuffer( file->GetName(), buffer ) )
                continue;

            size_t size = buffer.size();

            // Check the contents
            if ( size > 0 && CChecksum::GenerateChecksumFromBuffer( &buffer[0], size ).CompareWithLegacy( file->GetServerChecksum() ) )
            {
                //UTF-8 BOM?  Compare by checking the standard UTF-8 BOM of 3 characters (in signed format, hence negative)
                if ( size < 3 || buffer[0] != -0x11 || buffer[1] != -0x45 || buffer[2] != -0x41 ) 
                    //Maybe not UTF-8, if we have a >80% heuristic detection confidence, assume it is
                    m_lua.LoadScriptFromBuffer( &buffer.at( 0 ), size, file->GetName(), GetUTF8Confidence( (unsigned char*)&buffer.at( 0 ), size ) >= 80 );
                else if ( size != 3 )  //If there's a BOM, but the script is not empty, load ignoring the first 3 bytes
                    m_lua.LoadScriptFromBuffer( &buffer.at( 3 ), size - 3, file->GetName(), true );
            }
            else
            {
                SString strBuffer ( "ERROR: File '%s' in resource '%s' - CRC mismatch.", file->GetShortName (), m_name.c_str() );
                g_pCore->ChatEchoColor ( strBuffer, 255, 0, 0 );
            }
        }
        else if ( CheckFileForCorruption ( file->GetName () ) )
        {
            SString strBuffer ( "WARNING: File '%s' in resource '%s' is invalid.", file->GetShortName (), m_name.c_str() );
            g_pCore->DebugEchoColor ( strBuffer, 255, 0, 0 );
        }
    }

    // Set active flag
    m_active = true;

    // Call the Lua "onClientResourceStart" event
    lua_State *L = g_pClientGame->GetLuaManager()->GetVirtualMachine();
    PushStack( L );
    m_resourceEntity->CallEvent( "onClientResourceStart", L, 1 );
}

void CResource::ShowCursor( bool bShow, bool bToggleControls )
{
    // Different cursor showing state than earlier?
    if ( bShow != m_showCursor )
    {
        // Going to show the cursor?
        if ( bShow )
        {
            // Increase the cursor ref count
            m_refShowCursor++;
        }
        else
        {

            // Decrease the cursor ref count
            m_refShowCursor--;
        }

        // Update our showing cursor state
        m_showCursor = bShow;

        // Show cursor if more than 0 resources wanting the cursor on
        g_pCore->ForceCursorVisible ( m_refShowCursor != 0, bToggleControls );
        g_pClientGame->SetCursorEventsEnabled ( m_refShowCursor != 0 );
    }
}