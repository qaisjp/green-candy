/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        game_sa/RenderWare/CRpAtomicSA.cpp
*  PURPOSE:     RenderWare RpAtomic export
*  DEVELOPERS:  The_GTA <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#include <StdInc.h>
#include <gamesa_renderware.h>

extern CBaseModelInfoSAInterface **ppModelInfo;

CRpAtomicSA::CRpAtomicSA( RpAtomic *atom ) : CRwObjectSA( atom )
{
    m_model = NULL;
    m_txdID = 0xFFFF;
}

CRpAtomicSA::~CRpAtomicSA()
{
    // Restore all models
    RestoreAll();

    // Dereference our textures
    DereferenceTXD();

    // Detach from the clump and destroy
    RemoveFromModel();

    // We have to unlink textures, because they do not belong to us
    RpAtomic *atom = GetObject();
    atom->m_geometry->UnlinkFX();

    RpAtomicDestroy( atom );
}

CRpAtomic* CRpAtomicSA::Clone() const
{
    return new CRpAtomicSA( RpAtomicClone( GetObject() ) );
}

RpAtomic* CRpAtomicSA::CreateInstance( unsigned short id ) const
{
    const RpAtomic *a = GetObject();
    RpAtomic *atom = RpAtomicClone( GetObject() );
    CAtomicModelInfoSA *ainfo = ppModelInfo[id]->GetAtomicModelInfo();
    bool unk;
    char name[24];

    // This possibly adds the reference to the texture, we should reven this
    ((void (__cdecl*)(const char*, char*, bool&))0x005370A0)( GetObject()->m_parent->m_nodeName, name, unk );

    atom->SetRenderCallback( NULL );

    if ( unk )
        ainfo->GetDamageAtomicModelInfo()->SetupPipeline( atom );
    else
        ainfo->SetAtomic( atom );

    RpAtomicSetFrame( atom, RwFrameCreate() );
    
    atom->SetExtendedRenderFlags( id );
    return atom;
}

void CRpAtomicSA::AddToModel( CModel *model )
{
    RemoveFromModel();

    m_model = dynamic_cast <CModelSA*> ( model );

    if ( !m_model )
        return;

    m_model->m_atomics.push_back( this );
    GetObject()->AddToClump( m_model->GetObject() );

    // Parent us to the other frame
    GetFrame()->SetFrame( m_model->m_frame );
}

CModel* CRpAtomicSA::GetModel()
{
    return m_model;
}

void CRpAtomicSA::RemoveFromModel()
{
    if ( !m_model )
        return;

    // Remove our frame from the parent
    GetFrame()->SetFrame( NULL );

    GetObject()->RemoveFromClump();

    m_model->m_atomics.remove( this );
    m_model = NULL;
}

// Referencing texture containers in case the atomic becomes stand-alone; it takes the textures from a container without
// copying them! We therefor have to preserve the TXD.
void CRpAtomicSA::ReferenceTXD( unsigned short id )
{
    DereferenceTXD();

    m_txdID = id;
}

void CRpAtomicSA::DereferenceTXD()
{
    if ( m_txdID == 0xFFFF )
        return;

    (*ppTxdPool)->Get( m_txdID )->Dereference();

    m_txdID = 0xFFFF;
}

bool CRpAtomicSA::Replace( unsigned short id )
{
    CAtomicModelInfoSA *info = (CAtomicModelInfoSA*)ppModelInfo[id];

    if ( id > DATA_TEXTURE_BLOCK-1 )
        return false;

    if ( !info )
        return false;

    if ( info->GetRwModelType() != RW_ATOMIC )
        return false;

    if ( IsReplaced( id ) )
        return true;

    // Remove previous references to this model to prevent collision
    pGame->GetModelManager()->RestoreModel( id );

    CAtomicModelInfoSA *ainfo = (CAtomicModelInfoSA*)info;

    CStreamingSA *streaming = pGame->GetStreaming();

    if ( streaming->IsModelLoading( id ) )
    {
        // We need to flag it priority and finish the loading process
        // Worst case scenario would be otherwise that there is a thread loading the model and
        // replacing our model with it; we do not want that (memory leak prevention and model bugfix)
        streaming->RequestModel( id, 0x10 );
        streaming->LoadAllRequestedModels( true );
    }

    // We acquire a texture reference to make us stand-alone
    (*ppTxdPool)->Get( ainfo->m_textureDictionary )->Reference();   // we assume here that our textures are loaded already. Is this correct?

    ReferenceTXD( ainfo->m_textureDictionary );

    // We should inject directly if we are loaded; otherwise CStreaming takes the cake
    if ( ainfo->m_rpAtomic )
    {
        ainfo->m_rpAtomic->m_geometry->UnlinkFX();

        info->DeleteRwObject();
        ainfo->SetAtomic( CreateInstance( id ) );

        if ( g_colReplacement[id] )
            ainfo->m_pColModel = g_colReplacement[id]->GetInterface();
    }

    g_replObjectNative[id] = this;

    m_imported.push_back( id );
    return true;
}

bool CRpAtomicSA::IsReplaced( unsigned short id ) const
{
    return std::find( m_imported.begin(), m_imported.end(), id ) != m_imported.end();
}

bool CRpAtomicSA::Restore( unsigned short id )
{
    imports_t::iterator iter = std::find( m_imported.begin(), m_imported.end(), id );

    if ( iter == m_imported.end() )
        return false;

    CAtomicModelInfoSA *info = (CAtomicModelInfoSA*)ppModelInfo[id];

    if ( !info || info->GetRwModelType() != RW_ATOMIC )
    {
        // This situation should never happen, if it does, terminate our bases!
        m_imported.erase( iter );
        return false;
    }

    CStreamingSA *streaming = pGame->GetStreaming();

    g_replObjectNative[id] = NULL;

    // We do not need the textures anymore
    DereferenceTXD();

    // Restore if loaded
    if ( info->m_rpAtomic )
    {
        streaming->FreeModel( id );
        streaming->RequestModel( id, 0x10 );

        streaming->LoadAllRequestedModels( true );
    }

    m_imported.erase( iter );
    return true;
}

void CRpAtomicSA::RestoreAll()
{
    while ( !m_imported.empty() )
        Restore( m_imported[0] );
}