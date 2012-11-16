/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        game_sa/RenderWare/CModelSA.cpp
*  PURPOSE:     DFF model management entity
*               RenderWare extension
*  DEVELOPERS:  The_GTA <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#include "StdInc.h"
#include "gamesa_renderware.h"

extern CBaseModelInfoSAInterface **ppModelInfo;

void CModelSA::RpClumpAssignObjects( CRwObjectSA *obj, CModelSA *model )
{
    eRwType type = obj->GetType();

    if ( type == RW_ATOMIC )
    {
        CRpAtomicSA *atom = (CRpAtomicSA*)obj;

        if ( atom->GetObject()->m_clump == model->GetObject() )
        {
            atom->m_model = model;
            model->m_atomics.push_front( atom );
        }
    }
    else if ( type == RW_LIGHT )
    {
        CRpLightSA *light = (CRpLightSA*)obj;

        if ( light->GetObject()->m_clump == model->GetObject() )
        {
            light->m_model = model;
            model->m_lights.push_front( light );
        }
    }
    else if ( type == RW_CAMERA )
    {
        CRwCameraSA *cam = (CRwCameraSA*)obj;

        if ( cam->GetObject()->m_clump == model->GetObjectA() )
        {
            cam->m_model = model;
            model->m_cameras.push_front( cam );
        }
    }
}

static void RpClumpObjectAssociation( CRwFrameSA *frame, CModelSA *model )
{
    // Proceed through children
    {
        const CRwFrameSA::childList_t& list = frame->GetChildren();

        for ( CRwFrameSA::childList_t::const_iterator iter = list.begin(); iter != list.end(); iter++ )
            RpClumpObjectAssociation( dynamic_cast <CRwFrameSA*> ( *iter ), model );
    }

    // Proceed through objects
    {
        const CRwFrameSA::objectList_t& list = frame->GetObjects();

        for ( CRwFrameSA::objectList_t::const_iterator iter = list.begin(); iter != list.end(); iter++ )
            CModelSA::RpClumpAssignObjects( dynamic_cast <CRwObjectSA*> ( *iter ), model );
    }
}

CModelSA::CModelSA( RpClump *clump, CColModelSA *col, unsigned short txdID ) : CRwObjectSA( clump )
{
    m_col = col;
    m_txdID = txdID;

    // Assign the frame hierarchy
    m_frame = new CRwFrameSA( clump->m_parent );

    // Register all atomics to us
    RpClumpObjectAssociation( m_frame, this );
}

CModelSA::~CModelSA()
{
    RestoreAll();

    // Destroy all assigned cameras (if present)
    while ( !m_cameras.empty() )
        delete m_cameras.front();

    // Destroy all assigned lights (if present)
    while ( !m_lights.empty() )
        delete m_lights.front();

    // Destroy all assigned atomics (if present)
    while ( !m_atomics.empty() )
        delete m_atomics.front();

    RpClumpDestroy( GetObject() );
    delete m_col;

    // Remove our texture reference
    (*ppTxdPool)->Get( m_txdID )->Dereference();

    CModelManagerSA::models_t& models = pGame->GetModelManager()->m_models;

    // Remove ourself from the list
    models.erase( std::remove( models.begin(), models.end(), this ) );
}

const char* CModelSA::GetName() const
{
    return GetFrame()->GetName();
}

unsigned int CModelSA::GetHash() const
{
    return pGame->GetKeyGen()->GetUppercaseKey( GetName() );
}

void CModelSA::Render()
{
    // No point in rendering if no camera is set
    if ( !pRwInterface->m_renderCam )
        return;

    GetObject()->Render();
}

std::vector <unsigned short> CModelSA::GetImportList() const
{
    std::vector <unsigned short> impList;
    importMap_t::const_iterator iter = m_imported.begin();

    for ( ; iter != m_imported.end(); iter++ )
        impList.push_back( (*iter).first );

    return impList;
}

bool CModelSA::Replace( unsigned short id )
{
    CBaseModelInfoSAInterface *info = ppModelInfo[id];

    if ( id > DATA_TEXTURE_BLOCK-1 )
        return false;

    if ( !info )
        return false;

    if ( info->GetRwModelType() == RW_ATOMIC )
    {
        // Backwards compatibility for MTA:BLUE; notify our first atomic that it should replace that id
        if ( m_atomics.empty() )
            return false;

        return m_atomics.front()->Replace( id );
    }
    // We are a clump for sure

    if ( IsReplaced( id ) )
        return true;

    // Remove previous references to this model to prevent collision
    pGame->GetModelManager()->RestoreModel( id );

    CClumpModelInfoSAInterface *cinfo = (CClumpModelInfoSAInterface*)info;

    CStreamingSA *streaming = pGame->GetStreaming();

    if ( streaming->IsModelLoading( id ) )
    {
        // We need to flag it priority and finish the loading process
        // Worst case scenario would be otherwise that there is a thread loading the model and
        // replacing our model with it; we do not want that (memory leak prevention and model bugfix)
        streaming->RequestModel( id, 0x10 );
        streaming->LoadAllRequestedModels( true );
    }

    if ( cinfo->m_rwClump )  // Only inject if we are loaded! otherwise we screw up loading mechanics -> memory leaks
    {
        // If we store this collision model, we have to prevent it's destruction
        if ( m_col && m_col->GetOriginal() )
            cinfo->m_pColModel = NULL;

        cinfo->DeleteRwObject();
        cinfo->SetClump( RpClumpClone( GetObject() ) );
    }

    if ( m_col )
        m_col->Replace( id );

    g_replObjectNative[id] = this;

    m_imported[id] = true;
    return true;
}

bool CModelSA::IsReplaced( unsigned short id ) const
{
    return m_imported.find( id ) != m_imported.end();
}

bool CModelSA::Restore( unsigned short id )
{
    CBaseModelInfoSAInterface *info = ppModelInfo[id];

    if ( id > DATA_TEXTURE_BLOCK-1 )
        return false;

    importMap_t::iterator iter = m_imported.find( id );

    if ( iter == m_imported.end() )
        return false;

    if ( !info )
    {
        // This should not happen, but if it does, prepare for the worst
        m_imported.erase( iter );
        return false;
    }

    if ( info->GetRwModelType() == RW_ATOMIC )
    {
        // Backwards compatibility for MTA:BLUE; notify our first atomic that it should restore that id
        if ( m_atomics.empty() )
            return false;

        return m_atomics.front()->Restore( id );
    }

    CClumpModelInfoSAInterface *cinfo = (CClumpModelInfoSAInterface*)info;
    CStreamingSA *streaming = pGame->GetStreaming();

    if ( m_col )
        m_col->Restore( id );
    g_replObjectNative[id] = NULL;

    // We can only restore if the model is actively loaded
    if ( cinfo->m_rwClump )
    {
        // Do not allow destruction of collision if it belongs to us
        if ( m_col )
            cinfo->m_pColModel = NULL;

        streaming->FreeModel( id );
        streaming->RequestModel( id, 0x10 );

        streaming->LoadAllRequestedModels( true );
    }

    m_imported.erase( iter );
    return true;
}

void CModelSA::RestoreAll()
{
    while ( !m_imported.empty() )
        Restore( (*m_imported.begin()).first );
}