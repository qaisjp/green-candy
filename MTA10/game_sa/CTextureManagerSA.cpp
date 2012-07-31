/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        game_sa/CTextureManagerSA.cpp
*  PURPOSE:     Internal texture management
*               SubInterface of RenderWare
*  DEVELOPERS:  The_GTA <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#include "StdInc.h"
#include "gamesa_renderware.h"

#define FUNC_InitTextureManager         0x00731F20

extern CBaseModelInfoSAInterface **ppModelInfo;

CTxdPool**   ppTxdPool = (CTxdPool**)0x00C8800C;

CTxdInstanceSA::CTxdInstanceSA( const char *name )
{
    m_txd = NULL;
    m_references = 0;
    m_parentTxd = 0xFFFF;
    m_hash = pGame->GetKeyGen()->GetUppercaseKey(name);
}

bool Txd_DeleteAll( RwTexture *tex, int )
{
    if ( tex->refs < 2 )
    {
        tex->RemoveFromDictionary();

        RwTextureDestroy( tex );
    }

    return true;
}

CTxdInstanceSA::~CTxdInstanceSA()
{
    if ( m_txd )
    {
        m_txd->ForAllTextures( Txd_DeleteAll, 0 );

        RwTexDictionaryDestroy( m_txd );

        m_txd = NULL;
    }

    if ( m_parentTxd != 0xFFFF )
        (*ppTxdPool)->Get( m_parentTxd )->Dereference();
}

void* CTxdInstanceSA::operator new( size_t )
{
    return (*ppTxdPool)->Allocate();
}

void CTxdInstanceSA::operator delete( void *ptr )
{
    (*ppTxdPool)->Free( (CTxdInstanceSA*)ptr );
}

void CTxdInstanceSA::Allocate()
{
    if ( m_txd )
        return;

    m_txd = RwTexDictionaryCreate();
}

bool CTxdInstanceSA::LoadTXD( const char *filename )
{
    RwStream *stream = RwStreamOpen( STREAM_TYPE_FILENAME, STREAM_MODE_READ, filename );
    unsigned int size;
    unsigned int version;
    unsigned int numTextures;

    if ( !stream ) 
        return false;

    if (!RwStreamFindChunk( stream, 0x16, NULL, NULL ) || 
        !RwStreamFindChunk( stream, 1, &size, &version ) || 
        RwStreamReadBlocks( stream, &numTextures, size ) != size )
    {
        RwStreamClose( stream, NULL );
        return false;
    }

    // Make sure we are loaded
    Allocate();

    while ( numTextures-- )
    {
        RwTexture *texture = RwStreamReadTexture( stream );

        if ( !texture )
        {
            RwStreamClose( stream, NULL );
            return false;
        }

        texture->AddToDictionary( m_txd );
    }

    RwStreamClose( stream, NULL );
    return true;
}

unsigned short CTxdInstanceSA::InitParent()
{
    CTxdInstanceSA *parent = (*ppTxdPool)->Get( m_parentTxd );
    CTxdInstanceSA *affectTxd;

    if ( !parent )
        return 0xFFFF;

    affectTxd = ((CTxdInstanceSA*)(*(unsigned int**)0x00C88018 + (unsigned int)this));
    affectTxd->m_txd = m_txd;

    parent->Reference();
    return m_parentTxd;
}

void CTxdInstanceSA::Reference()
{
    m_references++;
}

void CTxdInstanceSA::Dereference()
{
    m_references--;

    // We unload ourselves
    if ( m_references == 0 )
        pGame->GetStreaming()->FreeModel( DATA_TEXTURE_BLOCK + (*ppTxdPool)->GetIndex( this ) );
}

void CTxdInstanceSA::SetCurrent()
{
    RwTexDictionarySetCurrent( m_txd );
}

//
// Hooks for creating txd create and destroy events
//
#define HOOKPOS_CTxdStore_SetupTxdParent       0x731D55
DWORD RETURN_CTxdStore_SetupTxdParent =        0x731D5B;
void HOOK_CTxdStore_SetupTxdParent ();

#define HOOKPOS_CTxdStore_RemoveTxd       0x731E90
DWORD RETURN_CTxdStore_RemoveTxd =        0x731E96;
void HOOK_CTxdStore_RemoveTxd ();

static void Hook_InitTextureManager()
{
    // Reserve 7 txds
    for ( unsigned int n=0; n<7; n++ )
        pGame->GetTextureManager()->CreateTxdEntry( "*" );

    __asm
    {
        // Register some callbacks
        mov edx,0x007F34D0
        push 0x00731720
        call edx

        mov edx,0x007F3500
        push 0x00731710 // callback which returns NULL
        call edx

        add esp,8
    }
}

CTextureManagerSA::CTextureManagerSA()
{
    // We init it ourselves
    HookInstall( FUNC_InitTextureManager, (DWORD)Hook_InitTextureManager, 6 );

    // We can initialize the pool here
    *ppTxdPool = new CTxdPool;

    // Our stuff
    m_pfnWatchCallback = NULL;

    // Hook runtime
    HookInstall ( HOOKPOS_CTxdStore_SetupTxdParent, (DWORD)HOOK_CTxdStore_SetupTxdParent, 6 );
    HookInstall ( HOOKPOS_CTxdStore_RemoveTxd, (DWORD)HOOK_CTxdStore_RemoveTxd, 6 );
}

CTextureManagerSA::~CTextureManagerSA()
{
    (*ppTxdPool)->Clear();

    delete *ppTxdPool;
}

int CTextureManagerSA::FindTxdEntry( const char *name ) const
{
    unsigned int hash = pGame->GetKeyGen()->GetUppercaseKey(name);
    unsigned int n;

    for (n=0; n<MAX_TXD; n++)
    {
        CTxdInstanceSA *txd = (*ppTxdPool)->Get(n);

        if (!txd || txd->m_hash != hash)
            continue;

        return n;
    }

    return -1;
}

int CTextureManagerSA::CreateTxdEntry( const char *name )
{
    CTxdInstanceSA *inst = new CTxdInstanceSA(name);

    // Check for crashes here
    if ( !inst )
        return -1;

    return (*ppTxdPool)->GetIndex(inst);
}

CTexDictionarySA* CTextureManagerSA::CreateTxd( const char *name )
{
    CTexDictionarySA *txd = new CTexDictionarySA( name );

    m_texDicts.push_back( txd );
    return txd;
}

CTexDictionarySA* CTextureManagerSA::CreateTxd( const char *name, unsigned short id )
{
    CTxdInstanceSA *itxd = (*ppTxdPool)->Get( id );

    if ( !itxd )
        return NULL;

    CTexDictionarySA *txd = new CTexDictionarySA( name, itxd );

    if ( !txd )
        return NULL;

    m_texDicts.push_back( txd );
    return txd;
}

int CTextureManagerSA::LoadDictionary( const char *filename )
{
    return LoadDictionaryEx( ExtractFilename( filename ), filename);
}

int CTextureManagerSA::LoadDictionaryEx( const char *name, const char *filename )
{
    int id = CreateTxdEntry( name );

    if ( id == -1 )
        return -1;

    CTxdInstanceSA *txd = (*ppTxdPool)->Get( id );

    if ( !txd->LoadTXD( filename ) )
    {
        delete txd;

        return -1;
    }

    txd->InitParent();

    return id;
}

bool CTextureManagerSA::SetCurrentTexture( unsigned short id )
{
    CTxdInstanceSA *txd = (*ppTxdPool)->Get( id );

    if ( !txd )
        return false;

    txd->SetCurrent();
    return true;
}

void CTextureManagerSA::RemoveTxdEntry( unsigned short id )
{
    CTxdInstanceSA *txd = (*ppTxdPool)->Get( id );

    // Crashfix
    if ( !txd )
        return;

    delete txd;
}

//
// STxdAction stores the current frame txd create and destroy events
//
struct STxdAction
{
    bool bAdd;
    ushort usTxdId;
};

std::vector < STxdAction > ms_txdActionList;

////////////////////////////////////////////////////////////////
//
// CTextureManagerSA::GetTXDIDForModelID
//
// Get a TXD ID associated with the model ID
//
////////////////////////////////////////////////////////////////
ushort CTextureManagerSA::GetTXDIDForModelID ( ushort usModelID )
{
    if ( usModelID >= 20000 && usModelID < 25000 )
    {
        // Get global TXD ID instead
        return usModelID - 20000;
    }
    else
    {
        // Get the CModelInfo's TXD ID

        // Ensure valid
        if ( usModelID >= 20000 || !((CBaseModelInfoSAInterface**)ARRAY_ModelInfo)[usModelID] )
            return 0;

        return ppModelInfo[usModelID]->m_textureDictionary;
    }
}

////////////////////////////////////////////////////////////////
//
// CTextureManagerSA::InitWorldTextureWatch
//
// Setup texture watch callback
//
////////////////////////////////////////////////////////////////
void CTextureManagerSA::InitWorldTextureWatch( PFN_WATCH_CALLBACK pfnWatchCallback )
{
    m_pfnWatchCallback = pfnWatchCallback;
}

////////////////////////////////////////////////////////////////
//
// CTextureManagerSA::AddWorldTextureWatch
//
// Begin watching for changes to the d3d resource associated with this texture name 
//
// Returns false if shader/match already exists
//
////////////////////////////////////////////////////////////////
bool CTextureManagerSA::AddWorldTextureWatch( CSHADERDUMMY* pShaderData, const char* szMatch, float fOrderPriority )
{
    SShadInfo* pNewShadInfo = CreateShadInfo( pShaderData, szMatch, fOrderPriority );
    if ( !pNewShadInfo )
        return false;

    // Step through texinfo list looking for all matches
    for ( std::list < STexInfo > ::iterator iter = m_TexInfoList.begin () ; iter != m_TexInfoList.end () ; ++iter )
    {
        STexInfo* pTexInfo = &*iter;
        if ( WildcardMatch ( pNewShadInfo->strMatch, pTexInfo->strTextureName ) )
        {
            // Check previous association
            if ( SShadInfo* pPrevShadInfo = pTexInfo->pAssociatedShadInfo )
            {
				// Check priority
                if ( pPrevShadInfo->fOrderPriority > pNewShadInfo->fOrderPriority )
                    continue;

                // Remove previous association
                BreakAssociation ( pPrevShadInfo, pTexInfo );
            }

            // Add new association
            MakeAssociation ( pNewShadInfo, pTexInfo );
        }
    }
    return true;
}

////////////////////////////////////////////////////////////////
//
// CTextureManagerSA::MakeAssociation
//
// Make the texture use the shader
//
////////////////////////////////////////////////////////////////
void CTextureManagerSA::MakeAssociation ( SShadInfo* pShadInfo, STexInfo* pTexInfo )
{
    assert ( !pTexInfo->pAssociatedShadInfo );
    assert ( !MapContains ( pShadInfo->associatedTexInfoMap, pTexInfo ) );
    pTexInfo->pAssociatedShadInfo = pShadInfo;
    pShadInfo->associatedTexInfoMap.insert ( pTexInfo );

    if ( m_pfnWatchCallback )
        m_pfnWatchCallback ( pShadInfo->pShaderData, pTexInfo->pD3DData, NULL );
}


////////////////////////////////////////////////////////////////
//
// CTextureManagerSA::BreakAssociation
//
// Stop the texture using the shader
//
////////////////////////////////////////////////////////////////
void CTextureManagerSA::BreakAssociation ( SShadInfo* pShadInfo, STexInfo* pTexInfo )
{
    assert ( pTexInfo->pAssociatedShadInfo == pShadInfo );
    assert ( MapContains ( pShadInfo->associatedTexInfoMap, pTexInfo ) );
    pTexInfo->pAssociatedShadInfo = NULL;
    MapRemove ( pShadInfo->associatedTexInfoMap, pTexInfo );
    if ( m_pfnWatchCallback )
        m_pfnWatchCallback ( pShadInfo->pShaderData, NULL, pTexInfo->pD3DData );
}


////////////////////////////////////////////////////////////////
//
// CTextureManagerSA::RemoveWorldTextureWatch
//
// End watching for changes to the d3d resource associated with this texture name
//
// When shadinfo removed:
//      unassociate with matches, let each texture find a new latest match
//
// If szMatch is NULL, remove all usage of the shader
//
////////////////////////////////////////////////////////////////
void CTextureManagerSA::RemoveWorldTextureWatch ( CSHADERDUMMY* pShaderData, const char* szMatch )
{
    SString strMatch = SStringX ( szMatch ).ToLower ();
    // Find shadInfo
    for ( std::list < SShadInfo > ::iterator iter = m_ShadInfoList.begin () ; iter != m_ShadInfoList.end () ; )
    {
        SShadInfo* pShadInfo = &*iter;
        if ( pShadInfo->pShaderData == pShaderData && ( !szMatch || pShadInfo->strMatch == strMatch ) )
        {
            std::vector < STexInfo* > reassociateList;

            // Unassociate with all matches
            while ( !pShadInfo->associatedTexInfoMap.empty () )
            {
                STexInfo* pTexInfo = *pShadInfo->associatedTexInfoMap.begin ();
                BreakAssociation ( pShadInfo, pTexInfo );

                // reassociateList for earlier shaders
                reassociateList.push_back ( pTexInfo );
            }

            OnDestroyShadInfo ( pShadInfo );
            iter = m_ShadInfoList.erase ( iter );

            // Allow unassociated texinfos to find new associations
            for ( uint i = 0 ; i < reassociateList.size () ; i++ )
            {
                STexInfo* pTexInfo = reassociateList [ i ];
                FindNewAssociationForTexInfo ( pTexInfo );
            }

			// If a match string was supplied, there should be only one in the list, so we can stop here
            if ( szMatch )
                break;
        }
        else
            ++iter;
    }
}

////////////////////////////////////////////////////////////////
//
// CTextureManagerSA::RemoveWorldTextureWatchByContext
//
// End watching for changes to the d3d resource associated with this context 
//
////////////////////////////////////////////////////////////////
void CTextureManagerSA::RemoveWorldTextureWatchByContext ( CSHADERDUMMY* pShaderData )
{
    RemoveWorldTextureWatch ( pShaderData, NULL );
}

////////////////////////////////////////////////////////////////
//
// CTextureManagerSA::PulseWorldTextureWatch
//
// Update watched textures status
//
////////////////////////////////////////////////////////////////
void CTextureManagerSA::PulseWorldTextureWatch()
{
    // Got through ms_txdActionList
    for ( std::vector < STxdAction >::const_iterator iter = ms_txdActionList.begin () ; iter != ms_txdActionList.end () ; ++iter )
    {
        const STxdAction& action = *iter;
        if ( action.bAdd )
        {
            //
            // New txd has been loaded
            //

            // Get list of texture names and data to add

            // Note: If txd has been unloaded since, textureList will be empty
            std::vector < RwTexture* > textureList;
            GetTxdTextures ( textureList, action.usTxdId );

            for ( std::vector < RwTexture* > ::iterator iter = textureList.begin () ; iter != textureList.end () ; iter++ )
            {
                RwTexture* texture = *iter;
                const char* szTextureName = texture->name;
                CD3DDUMMY* pD3DData = texture->raster ? (CD3DDUMMY*)texture->raster->renderResource : NULL;
                AddActiveTexture ( action.usTxdId, szTextureName, pD3DData );
            }
        }
        else
        {
            //
            // Txd has been unloaded
            //

            RemoveTxdActiveTextures ( action.usTxdId );
        }
    }

    ms_txdActionList.clear ();
}

////////////////////////////////////////////////////////////////
//
// CTextureManagerSA::AddActiveTexture
//
// Create a texinfo for the texture and find a best match shader for it
//
////////////////////////////////////////////////////////////////
void CTextureManagerSA::AddActiveTexture( ushort usTxdId, const SString& strTextureName, CD3DDUMMY* pD3DData )
{
    STexInfo* pTexInfo = CreateTexInfo ( usTxdId, strTextureName, pD3DData );
    FindNewAssociationForTexInfo ( pTexInfo );
}

////////////////////////////////////////////////////////////////
//
// CTextureManagerSA::CreateTexInfo
//
////////////////////////////////////////////////////////////////
STexInfo* CTextureManagerSA::CreateTexInfo ( ushort usTxdId, const SString& strTextureName, CD3DDUMMY* pD3DData )
{
    // Create texinfo
    m_TexInfoList.push_back ( STexInfo ( usTxdId, strTextureName, pD3DData ) );
    STexInfo* pTexInfo = &m_TexInfoList.back ();

    // Add to lookup maps
    SString strUniqueKey ( "%d_%s", pTexInfo->usTxdId, *pTexInfo->strTextureName );
    assert ( !MapContains ( m_UniqueTexInfoMap, strUniqueKey ) );
    MapSet ( m_UniqueTexInfoMap, strUniqueKey, pTexInfo );

	// This assert fails when using engine txd replace functions - TODO find out why
    //assert ( !MapContains ( m_D3DDataTexInfoMap, pTexInfo->pD3DData ) );
    MapSet ( m_D3DDataTexInfoMap, pTexInfo->pD3DData, pTexInfo );
    return pTexInfo;
}

////////////////////////////////////////////////////////////////
//
// CTextureManagerSA::OnDestroyTexInfo
//
////////////////////////////////////////////////////////////////
void CTextureManagerSA::OnDestroyTexInfo ( STexInfo* pTexInfo )
{
    // Remove from lookup maps
    SString strUniqueKey ( "%d_%s", pTexInfo->usTxdId, *pTexInfo->strTextureName );
    assert ( MapContains ( m_UniqueTexInfoMap, strUniqueKey ) );
    MapRemove ( m_UniqueTexInfoMap, strUniqueKey );

	// This assert fails when using engine txd replace functions - TODO find out why
    //assert ( MapContains ( m_D3DDataTexInfoMap, pTexInfo->pD3DData ) );
    MapRemove ( m_D3DDataTexInfoMap, pTexInfo->pD3DData );
}

////////////////////////////////////////////////////////////////
//
// CTextureManagerSA::CreateShadInfo
//
////////////////////////////////////////////////////////////////
SShadInfo* CTextureManagerSA::CreateShadInfo (  CSHADERDUMMY* pShaderData, const SString& strMatch, float fOrderPriority )
{
    // Create shadinfo
    m_ShadInfoList.push_back ( SShadInfo ( strMatch, pShaderData, fOrderPriority ) );
    SShadInfo* pShadInfo = &m_ShadInfoList.back ();

    // Check for uniqueness
    SString strUniqueKey ( "%08x_%s", pShadInfo->pShaderData, *pShadInfo->strMatch );
    if ( MapContains ( m_UniqueShadInfoMap, strUniqueKey ) )
    {
        // Remove again if not unique
        m_ShadInfoList.pop_back ();
        return NULL;
    }

    // Set unique map
    MapSet ( m_UniqueShadInfoMap, strUniqueKey, pShadInfo );

    // Add to order map
    MapInsert ( m_orderMap, pShadInfo->fOrderPriority, pShadInfo );

    return pShadInfo;
}


////////////////////////////////////////////////////////////////
//
// CTextureManagerSA::OnDestroyShadInfo
//
////////////////////////////////////////////////////////////////
void CTextureManagerSA::OnDestroyShadInfo ( SShadInfo* pShadInfo )
{
    // Remove from lookup maps
    SString strUniqueKey ( "%08x_%s", pShadInfo->pShaderData, *pShadInfo->strMatch );
    assert ( MapContains ( m_UniqueShadInfoMap, strUniqueKey ) );
    MapRemove ( m_UniqueShadInfoMap, strUniqueKey );

    //  Remove from order map
    MapRemovePair ( m_orderMap, pShadInfo->fOrderPriority, pShadInfo );
}

////////////////////////////////////////////////////////////////
//
// CTextureManagerSA::FindNewAssociationForTexInfo
//
// Find best match for this texinfo
// Called when a texture is loaded or a shader is removed from a texture
//
////////////////////////////////////////////////////////////////
void CTextureManagerSA::FindNewAssociationForTexInfo ( STexInfo* pTexInfo )
{
    // Stepping backwards will bias high priority, and if the priorty is the same, bias the latest additions
    for ( std::multimap < float, SShadInfo* > ::reverse_iterator iter = m_orderMap.rbegin () ; iter != m_orderMap.rend () ; ++iter )
    {
        SShadInfo* pShadInfo = iter->second;
        if ( WildcardMatch ( pShadInfo->strMatch, pTexInfo->strTextureName ) )
        {
            // Found one
            MakeAssociation ( pShadInfo, pTexInfo );
            break;
        }
    }
}

////////////////////////////////////////////////////////////////
//
// CTextureManagerSA::RemoveTxdActiveTextures
//
// Called when a TXD is being unloaded.
// Delete texinfos which came from that TXD
//
////////////////////////////////////////////////////////////////
void CTextureManagerSA::RemoveTxdActiveTextures ( ushort usTxdId )
{
    // Find all TexInfo's for this txd
    for ( std::list < STexInfo > ::iterator iter = m_TexInfoList.begin () ; iter != m_TexInfoList.end () ; )
    {
        STexInfo* pTexInfo = &*iter;
        if ( pTexInfo->usTxdId == usTxdId )
        {
            // If texinfo has a shadinfo, unassociate
            if ( SShadInfo* pShadInfo = pTexInfo->pAssociatedShadInfo )
            {
                BreakAssociation ( pShadInfo, pTexInfo );
            }

            OnDestroyTexInfo ( pTexInfo );
            iter = m_TexInfoList.erase ( iter );
        }
        else
            ++iter;
    }
}

////////////////////////////////////////////////////////////////
//
// CTextureManagerSA::GetModelTextureNames
//
// Get list of texture names associated with the model
//
////////////////////////////////////////////////////////////////
void CTextureManagerSA::GetModelTextureNames ( std::vector < SString >& outNameList, ushort usModelID )
{
    outNameList.empty ();

    ushort usTxdId = GetTXDIDForModelID ( usModelID );

    if ( usTxdId == 0 )
        return;

    std::vector < RwTexture* > textureList;
    GetTxdTextures ( textureList, usTxdId );

    for ( std::vector < RwTexture* > ::iterator iter = textureList.begin () ; iter != textureList.end () ; iter++ )
    {
        outNameList.push_back ( (*iter)->name );
    }
}


////////////////////////////////////////////////////////////////
//
// CTextureManagerSA::GetTxdTextures
//
////////////////////////////////////////////////////////////////
static bool StaticGetTextureCB( RwTexture* texture, std::vector <RwTexture*>* list )
{
    list->push_back( texture );
    return true;
}

void CTextureManagerSA::GetTxdTextures ( std::vector < RwTexture* >& outTextureList, ushort usTxdId )
{
    if ( usTxdId == 0 )
        return;

    // Get the TXD corresponding to this ID
    SetTextureDict ( usTxdId );

    RwTexDictionary* pTXD = CTxdStore_GetTxd ( usTxdId );

    if ( pTXD )
    {
        pTXD->ForAllTextures( StaticGetTextureCB, &outTextureList );
    }
}

////////////////////////////////////////////////////////////////
//
// CTextureManagerSA::GetTextureName
//
////////////////////////////////////////////////////////////////
const SString& CTextureManagerSA::GetTextureName ( CD3DDUMMY* pD3DData )
{
    STexInfo** ppTexInfo = MapFind ( m_D3DDataTexInfoMap, pD3DData );
    if ( ppTexInfo )
        return (*ppTexInfo)->strTextureName;
    static SString strDummy;
    return strDummy;
}

////////////////////////////////////////////////////////////////
//
// Hooks
//
////////////////////////////////////////////////////////////////

void _cdecl OnAddTxd( DWORD dwTxdId )
{
    STxdAction action;
    action.bAdd = true;
    action.usTxdId = (ushort)dwTxdId;
    ms_txdActionList.push_back ( action );
}

// called from streaming on TXD create
void _declspec(naked) HOOK_CTxdStore_SetupTxdParent()
{
    _asm
    {
        // Hooked from 731D55  6 bytes

        // eax - txd id
        pushad
        push eax
        call OnAddTxd
        add esp, 4
        popad

        // orig
        mov     esi, ds:00C8800Ch 
        jmp     RETURN_CTxdStore_SetupTxdParent  // 731D5B
    }
}

void _cdecl OnRemoveTxd( DWORD dwTxdId )
{
    STxdAction action;
    action.bAdd = false;
    action.usTxdId = (ushort)dwTxdId - 20000;
    ms_txdActionList.push_back ( action );
}

// called from streaming on TXD destroy
void _declspec(naked) HOOK_CTxdStore_RemoveTxd()
{
    _asm
    {
        // Hooked from 731E90  6 bytes

        // esi - txd id + 20000
        pushad
        push esi
        call OnRemoveTxd
        add esp, 4
        popad

        // orig
        mov     ecx, ds:00C8800Ch
        jmp     RETURN_CTxdStore_RemoveTxd      // 731E96
    }
}

/////////////////////////////////////////