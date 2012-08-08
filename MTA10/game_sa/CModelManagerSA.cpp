/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        game_sa/CModelManagerSA.cpp
*  PURPOSE:     DFF model entity manager
*               RenderWare extension
*  DEVELOPERS:  The_GTA <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#include "StdInc.h"
#include "gamesa_renderware.h"

extern CBaseModelInfoSAInterface **ppModelInfo;

CModelManagerSA::CModelManagerSA()
{
}

CModelManagerSA::~CModelManagerSA()
{
    // Clear our models
    while ( !m_models.empty() )
        delete *m_models.begin();
}

CModelSA* CModelManagerSA::CreateModel( const char *path, unsigned short id )
{
    RpClump *clump = pGame->GetRenderWare()->ReadDFF( path, id );

    if ( !clump )
        return NULL;

    CModelSA *model = new CModelSA( clump, ppModelInfo[id]->m_pColModel );

    m_models.push_back( model );
    return model;
}

bool CModelManagerSA::RestoreModel( unsigned short id )
{
    CBaseModelInfoSAInterface *info = ppModelInfo[id];

    if ( id > MAX_MODELS-1 )
        return false;

    if ( !info )
        return false;

    if ( info->GetRwModelType() != RW_CLUMP )
        return false;

    models_t::iterator iter = m_models.begin();

    for ( ; iter != m_models.end(); iter++ )
        (*iter)->Restore( id );

    return true;
}