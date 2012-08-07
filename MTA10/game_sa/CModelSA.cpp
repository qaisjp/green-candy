/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        game_sa/CModelSA.cpp
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

CModelSA::CModelSA( RpClump *clump )
{
    m_clump = clump;
}

CModelSA::~CModelSA()
{
    RestoreAll();

    RpClumpDestroy( m_clump );

    CModelManagerSA::models_t::iterator iter = pGame->GetModelManager()->m_models.begin();

    for ( ; iter != pGame->GetModelManager()->m_models.end(); iter++ )
    {
        if ( *iter == this )
        {
            pGame->GetModelManager()->m_models.erase( iter );
            break;
        }
    }
}

const char* CModelSA::GetName() const
{
    return m_clump->m_parent->m_nodeName;
}

unsigned int CModelSA::GetHash() const
{
    return pGame->GetKeyGen()->GetUppercaseKey( m_clump->m_parent->m_nodeName );
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

    if ( info->GetRwModelType() != RW_CLUMP )
        return false;

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

    if ( cinfo->m_rwClump ) // Only inject if we are loaded! otherwise we screw up loading mechanics -> memory leaks
    {
        cinfo->DeleteRwObject();
        cinfo->SetClump( RpClumpClone( m_clump ) );
    }

    g_modelReplacement[id] = m_clump;

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

    if ( !info || info->GetRwModelType() != RW_CLUMP )
    {
        // This should not happen, but if it does, prepare for the worst
        m_imported.erase( iter );
        return false;
    }

    CClumpModelInfoSAInterface *cinfo = (CClumpModelInfoSAInterface*)info;

    // We can only restore if the model is actively loaded
    if ( cinfo->m_rwClump )
    {
        CStreamingSA *streaming = pGame->GetStreaming();
        streaming->FreeModel( id );
        streaming->RequestModel( id, 0x10 );

        streaming->LoadAllRequestedModels( true );
    }

    g_modelReplacement[id] = NULL;

    m_imported.erase( iter );
    return true;
}

void CModelSA::RestoreAll()
{
    while ( !m_imported.empty() )
        Restore( (*m_imported.begin()).first );
}