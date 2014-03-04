/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        game_sa/CModelManagerSA.cpp
*  PURPOSE:     DFF model entity manager
*               RenderWare extension
*  DEVELOPERS:  Martin Turski <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#include "StdInc.h"
#include "gamesa_renderware.h"

extern CBaseModelInfoSAInterface **ppModelInfo;

CRwObjectSA *g_replObjectNative[DATA_TEXTURE_BLOCK];

CModelManagerSA::CModelManagerSA( void )
{
    // Reset information structs
    memset( g_replObjectNative, 0, sizeof(g_replObjectNative) );
}

CModelManagerSA::~CModelManagerSA( void )
{
    // Clear our models
    while ( !m_models.empty() )
        delete *m_models.begin();
}

CModelSA* CModelManagerSA::CreateModel( CFile *file, modelId_t id )
{
    CColModelSA *col = NULL;
    RpClump *clump = pGame->GetRenderWare()->ReadDFF( file, id, col );

    if ( !clump )
        return NULL;

    return new CModelSA( clump, col );
}

CModelSA* CModelManagerSA::CloneClump( modelId_t model )
{
    if ( model > DATA_TEXTURE_BLOCK-1 )
        return NULL;

    CClumpModelInfoSAInterface *info = (CClumpModelInfoSAInterface*)ppModelInfo[model];

    if ( !info || info->GetRwModelType() != RW_CLUMP )
        return NULL;

    if ( !info->GetRwObject() )
        return NULL;

    return new CModelSA( RpClumpClone( info->GetRwObject() ), NULL );
}

CRpAtomicSA* CModelManagerSA::CloneAtomic( modelId_t model )
{
    if ( model > DATA_TEXTURE_BLOCK-1 )
        return NULL;

    CAtomicModelInfoSA *info = (CAtomicModelInfoSA*)ppModelInfo[model];

    if ( !info || info->GetRwModelType() != RW_ATOMIC )
        return NULL;

    if ( !info->GetRwObject() )
        return NULL;

    return new CRpAtomicSA( RpAtomicClone( info->GetRwObject() ) );
}

bool CModelManagerSA::GetRwModelType( modelId_t model, eRwType& type ) const
{
    if ( model > MAX_MODELS-1 )
        return false;

    CBaseModelInfoSAInterface *info = (CBaseModelInfoSAInterface*)ppModelInfo[model];

    if ( !info )
        return false;

    type = info->GetRwModelType();
    return true;
}

bool CModelManagerSA::RestoreModel( modelId_t id )
{
    if ( id > DATA_TEXTURE_BLOCK-1 )
        return false;

    CRwObjectSA *obj = g_replObjectNative[id];

    if ( !obj )
        return false;

    switch( obj->GetType() )
    {
    case RW_CLUMP:
        ((CModelSA*)obj)->Restore( id );
        break;
    case RW_ATOMIC:
        ((CRpAtomicSA*)obj)->Restore( id );
        break;
    }

    return true;
}

bool CModelManagerSA::RestoreCollision( modelId_t id )
{
    if ( id > DATA_TEXTURE_BLOCK-1 )
        return false;

    CColModelSA *col = g_colReplacement[id];

    if ( !col )
        return false;

    return col->Restore( id );
}

void CModelManagerSA::RestreamByModel( modelId_t model )
{
    // todo
}

void CModelManagerSA::RestreamByTXD( modelId_t model )
{
    // todo
}