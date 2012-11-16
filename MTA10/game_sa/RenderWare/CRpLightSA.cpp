/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        game_sa/RenderWare/CRpLightSA.cpp
*  PURPOSE:     RenderWare RpLight export
*  DEVELOPERS:  The_GTA <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#include <StdInc.h>
#include "gamesa_renderware.h"

CRpLightSA::CRpLightSA( RpLight *light ) : CRwObjectSA( light )
{
    m_model = NULL;
}

CRpLightSA::~CRpLightSA()
{
    // Unlink from the clump
    RemoveFromModel();

    // We should detach from the frame
    SetFrame( NULL );

    RpLightDestroy( GetObject() );
}

void CRpLightSA::AddToModel( CModel *model )
{
    RemoveFromModel();

    m_model = dynamic_cast <CModelSA*> ( model );

    if ( !m_model )
        return;

    m_model->m_lights.push_back( this );
    GetObject()->AddToClump( m_model->GetObject() );
}

CModel* CRpLightSA::GetModel()
{
    return m_model;
}

void CRpLightSA::RemoveFromModel()
{
    if ( !m_model )
        return;

    GetObject()->RemoveFromClump();

    m_model->m_lights.remove( this );
    m_model = NULL;
}

void CRpLightSA::SetRadius( float radius )
{
    RpLightSetRadius( GetObject(), radius );
}