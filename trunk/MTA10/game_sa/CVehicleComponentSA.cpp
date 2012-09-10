/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        game_sa/CVehicleComponentSA.cpp
*  PURPOSE:     Vehicle component part interface
*  DEVELOPERS:  The_GTA <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#include "StdInc.h"
#include "gamesa_renderware.h"

CVehicleComponentSA::CVehicleComponentSA( CVehicleComponentSA*& slot, RpAtomic *atomic ) : m_compSlot( slot )
{
    m_atomic = atomic;
}

CVehicleComponentSA::~CVehicleComponentSA()
{
    // Clear ourselves from the vehicle
    m_compSlot = NULL;

    m_atomic->RemoveFromFrame();
    m_atomic->RemoveFromClump();
    RpAtomicDestroy( m_atomic );
}

void CVehicleComponentSA::SetMatrix( const RwMatrix& mat )
{
    m_atomic->m_parent->m_modelling = mat;
}

const RwMatrix& CVehicleComponentSA::GetMatrix() const
{
    return m_atomic->m_parent->m_modelling;
}

const RwMatrix& CVehicleComponentSA::GetWorldMatrix() const
{
    return m_atomic->m_parent->m_ltm;
}

void CVehicleComponentSA::SetPosition( const CVector& pos )
{
    m_atomic->m_parent->m_modelling.pos = pos;
}

const CVector& CVehicleComponentSA::GetPosition() const
{
    return m_atomic->m_parent->m_modelling.pos;
}

const CVector& CVehicleComponentSA::GetWorldPosition() const
{
    return m_atomic->m_parent->m_ltm.pos;
}