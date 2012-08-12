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

CRwObjectSA *g_replObjectNative[DATA_TEXTURE_BLOCK];

CModelManagerSA::CModelManagerSA()
{
    // Reset information structs
    memset( g_replObjectNative, 0, sizeof(g_replObjectNative) );
}

CModelManagerSA::~CModelManagerSA()
{
    // Clear our models
    while ( !m_models.empty() )
        delete *m_models.begin();
}

CModelSA* CModelManagerSA::CreateModel( CFile *file, unsigned short id )
{
    CColModelSA *col = NULL;
    RpClump *clump = pGame->GetRenderWare()->ReadDFF( file, id, col );

    if ( !clump )
        return NULL;

    CModelSA *model = new CModelSA( clump, col, ppModelInfo[id]->m_textureDictionary );

    m_models.push_back( model );
    return model;
}

bool CModelManagerSA::RestoreModel( unsigned short id )
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

bool CModelManagerSA::RestoreCollision( unsigned short id )
{
    if ( id > DATA_TEXTURE_BLOCK-1 )
        return false;

    CColModelSA *col = g_colReplacement[id];

    if ( !col )
        return false;

    return col->Restore( id );
}