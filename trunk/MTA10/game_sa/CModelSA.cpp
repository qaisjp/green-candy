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

    pGame->GetModelManager()->m_models.remove( this );
}

const char* CModelSA::GetName() const
{
    return m_clump->m_parent->m_nodeName;
}

unsigned int CModelSA::GetHash() const
{
    return pGame->GetKeyGen()->GetUppercaseKey( m_clump->m_parent->m_nodeName );
}

std::vector <unsigned short> CModelSA::GetImportedList() const
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

    if ( id > MAX_MODELS-1 )
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

    import imp;
    imp.original = cinfo->CreateRwObject();

    cinfo->SetClump( m_clump );

    m_imported[id] = imp;
    return true;
}

bool CModelSA::IsReplaced( unsigned short id ) const
{
    return m_imported.find( id ) != m_imported.end();
}

bool CModelSA::Restore( unsigned short id )
{
    CBaseModelInfoSAInterface *info = ppModelInfo[id];

    if ( id > MAX_MODELS-1 )
        return false;

    importMap_t::iterator iter = m_imported.find( id );

    if ( iter == m_imported.end() )
        return false;

    if ( !info || info->GetRwModelType() != RW_CLUMP )
    {
        // This should not happen, but if it does, prepare for the worst
        RpClumpDestroy( (*iter).second.original );

        m_imported.erase( iter );
        return false;
    }

    CClumpModelInfoSAInterface *cinfo = (CClumpModelInfoSAInterface*)info;

    cinfo->SetClump( (*iter).second.original );

    m_imported.erase( iter );
    return true;
}

void CModelSA::RestoreAll()
{
    while ( !m_imported.empty() )
        Restore( (*m_imported.begin()).first );
}