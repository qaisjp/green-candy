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

CVehicleComponentSA::CVehicleComponentSA( CVehicleComponentSA*& slot, RpAtomic *atomic, unsigned short txdID ) : m_compSlot( slot )
{
    m_atomic = atomic;
    m_txdID = txdID;
}

CVehicleComponentSA::~CVehicleComponentSA()
{
    // Clear ourselves from the vehicle
    m_compSlot = NULL;

    m_atomic->RemoveFromFrame();
    m_atomic->RemoveFromClump();
    RpAtomicDestroy( m_atomic );
}

const char* CVehicleComponentSA::GetName() const
{
    return m_atomic->m_parent->m_nodeName;
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

void CVehicleComponentSA::SetActive( bool active )
{
    m_atomic->SetVisible( active );
}

bool CVehicleComponentSA::IsActive() const
{
    return m_atomic->IsVisible();
}

CRpAtomic* CVehicleComponentSA::CloneAtomic() const
{
    RpAtomic *inst = RpAtomicClone( m_atomic );

    // Copy important credentials
    RwFrame *frame = RwFrameCreate();

    memcpy( frame->m_nodeName, m_atomic->m_parent->m_nodeName, 0x10 );

    RpAtomicSetFrame( inst, frame );

    CRpAtomicSA *atom = new CRpAtomicSA( inst );

    // We may not lose the loaded textures, so reference them
    (*ppTxdPool)->Get( m_txdID )->Reference();

    atom->ReferenceTXD( m_txdID );

    return atom;
}