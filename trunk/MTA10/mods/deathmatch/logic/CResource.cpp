/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.0
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        mods/deathmatch/logic/CResource.cpp
*  PURPOSE:     Resource object
*  DEVELOPERS:  Kevin Whiteside <kevuwk@gmail.com>
*               Cecill Etheredge <ijsf@gmx.net>
*               Jax <>
*               Chris McArthur <>
*               Ed Lyons <eai@opencoding.net>
*               
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#include "StdInc.h"
#include "CServerIdManager.h"

extern CClientGame      *g_pClientGame;
extern CCoreInterface   *g_pCore;

int CResource::m_refShowingCursor = 0;
CFileTranslator *resFileRoot;

static inline CFileTranslator* _rtget( const char *name )
{
    filePath path;
    ResourceManager::resFileRoot->GetFullPath( name, false, path );

    return g_pCore->GetFileSystem()->CreateTranslator( path.c_str() );
}

CResource::CResource( unsigned short id, const filePath& name, CClientEntity *entity, CClientEntity *dynamicEntity ) : Resource( id, name, _rtget( name ) )
{
    m_luaManager = g_pClientGame->GetLuaManager();
    m_inDownQueue = false;
    m_showingCursor = false;

    m_rootEntity = g_pClientGame->GetRootEntity();
    m_defaultGroup = new CElementGroup( this );
    m_elementGroups.push_back( m_defaultGroup ); // for use by scripts

    m_resourceEntity = entity;
    m_dynamicEntity = dynamicEntity;

    // Create our root elements. We set theirs parents when we're loaded.
    // Make them system entities so nothing but us can delete them.
    m_pResourceGUIEntity = new CClientDummy( g_pClientGame->GetManager(), INVALID_ELEMENT_ID, "guiroot" );
    m_pResourceGUIEntity->MakeSystemEntity();

    m_pResourceCOLRoot = new CClientDummy( g_pClientGame->GetManager(), INVALID_ELEMENT_ID, "colmodelroot" );
    m_pResourceCOLRoot->MakeSystemEntity();

    m_pResourceDFFEntity = new CClientDummy( g_pClientGame->GetManager(), INVALID_ELEMENT_ID, "dffroot" );
    m_pResourceDFFEntity->MakeSystemEntity();

    m_pResourceTXDRoot = new CClientDummy( g_pClientGame->GetManager(), INVALID_ELEMENT_ID, "txdroot" );
    m_pResourceTXDRoot->MakeSystemEntity();

    // Set up our private path
    filePath privPath;
    ResourceManager::resFileRoot->GetFullPath( CServerIdManager::GetSingleton()->GetConnectionPrivateDirectory() + name, false, privPath );

    m_privateRoot = g_pCore->GetFileSystem()->CreateTranslator( privPath.c_str() );

    // Create our virtual machine
    m_lua = m_luaManager->Create( name.c_str() );
    m_lua->SetName( name.c_str() );
}

CResource::~CResource()
{
    // Make sure we don't force the cursor on
    ShowCursor( false );

    // Do this before we delete our elements.
    m_pRootEntity->CleanUpForVM( m_lua, true );
    m_luaManager->Remove( m_lua );

    // Remove all keybinds on this VM
    g_pClientGame->GetScriptKeyBinds()->RemoveAllKeys( m_lua );
    g_pCore->GetKeyBinds()->SetAllCommandsActive( m_name.c_str(), false );

    // Destroy all entities
    g_pClientGame->GetElementDeleter()->DeleteRecursive( m_txdEntity );
    g_pClientGame->GetElementDeleter()->DeleteRecursive( m_dffEntity );
    g_pClientGame->GetElementDeleter()->DeleteRecursive( m_colEntity );
    g_pClientGame->GetElementDeleter()->DeleteRecursive( m_guiEntity );

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
    for ( ; iter != m_ResourceFiles.end(); iter++ )
        delete *iter;

    configList_t::iterator iterc = m_configFiles.begin();
    for ( ; iterc != m_ConfigFiles.end(); iterc++ )
        delete *iterc;

    exports_t::iterator iterex = m_exports.begin();
    for ( ; iterex != m_exportedFunctions.end(); iterex++ )
        delete *iterex;
}

CDownloadableResource* CResource::QueueFile ( CDownloadableResource::eResourceType resourceType, const char *szFileName, CChecksum serverChecksum )
{
    // Create the resource file and add it to the list
    SString strBuffer ( "%s\\resources\\%s\\%s", g_pClientGame->GetModRoot (), m_szResourceName, szFileName );

    CResourceFile* pResourceFile = new CResourceFile ( resourceType, szFileName, strBuffer, serverChecksum );
    if ( pResourceFile )
    {
        m_ResourceFiles.push_back ( pResourceFile );
    }

    return pResourceFile;
}


CDownloadableResource* CResource::AddConfigFile ( char *szFileName, CChecksum serverChecksum )
{
    // Create the config file and add it to the list
    SString strBuffer ( "%s\\resources\\%s\\%s", g_pClientGame->GetModRoot (), m_szResourceName, szFileName );
    
    CResourceConfigItem* pConfig = new CResourceConfigItem ( this, szFileName, strBuffer, serverChecksum );
    if ( pConfig )
    {
        m_ConfigFiles.push_back ( pConfig );
    }

    return pConfig;
}

void CResource::AddExportedFunction ( char *szFunctionName )
{
    m_exportedFunctions.push_back(new CExportedFunction ( szFunctionName ) );
}

bool CResource::CallExportedFunction ( const char * szFunctionName, CLuaArguments& args, CLuaArguments& returns, CResource& caller )
{
    list < CExportedFunction* > ::iterator iter =  m_exportedFunctions.begin ();
    for ( ; iter != m_exportedFunctions.end (); iter++ )
    {
        if ( strcmp ( (*iter)->GetFunctionName(), szFunctionName ) == 0 )
        {
            if ( args.CallGlobal ( m_pLuaVM, szFunctionName, &returns ) )
            {
                return true;
            }
        }
    }
    return false;
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

void CResource::Load( CClientEntity *root )
{
    m_pRootEntity = root;

    // Set the GUI parent to the resource root entity
    m_colEntity->SetParent( root );
    m_dffEntity->SetParent( root );
    m_guiEntity->SetParent( root );
    m_txdEntity->SetParent( root );

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
                    m_lua->LoadScriptFromBuffer( &buffer.at( 0 ), size, file->GetName(), GetUTF8Confidence( (unsigned char*)&buffer.at( 0 ), size ) >= 80 );
                else if ( size != 3 )  //If there's a BOM, but the script is not empty, load ignoring the first 3 bytes
                    m_lua->LoadScriptFromBuffer( &buffer.at( 3 ), size - 3, file->GetName(), true );
            }
            else
            {
                SString strBuffer ( "ERROR: File '%s' in resource '%s' - CRC mismatch.", pResourceFile->GetShortName (), m_szResourceName );
                g_pCore->ChatEchoColor ( strBuffer, 255, 0, 0 );
            }
        }
        else if ( CheckFileForCorruption ( pResourceFile->GetName () ) )
        {
            SString strBuffer ( "WARNING: File '%s' in resource '%s' is invalid.", pResourceFile->GetShortName (), m_szResourceName );
            g_pCore->DebugEchoColor ( strBuffer, 255, 0, 0 );
        }
    }

    // Set active flag
    m_bActive = true;

    // Did we get a resource root entity?
    if ( m_pResourceEntity )
    {
        // Call the Lua "onClientResourceStart" event
        CLuaArguments Arguments;
        Arguments.PushUserData ( this );
        m_pResourceEntity->CallEvent ( "onClientResourceStart", Arguments, true );
    }
    else
        assert ( 0 );
}


void CResource::DeleteClientChildren ( void )
{
    // Run this on our resource entity if we have one
    if ( m_pResourceEntity )
        m_pResourceEntity->DeleteClientChildren ();
}


void CResource::ShowCursor ( bool bShow, bool bToggleControls )
{
    // Different cursor showing state than earlier?
    if ( bShow != m_bShowingCursor )
    {
        // Going to show the cursor?
        if ( bShow )
        {
            // Increase the cursor ref count
            m_iShowingCursor += 1;
        }
        else
        {

            // Decrease the cursor ref count
            m_iShowingCursor -= 1;
        }

        // Update our showing cursor state
        m_bShowingCursor = bShow;

        // Show cursor if more than 0 resources wanting the cursor on
        g_pCore->ForceCursorVisible ( m_iShowingCursor > 0, bToggleControls );
        g_pClientGame->SetCursorEventsEnabled ( m_iShowingCursor > 0 );
    }
}