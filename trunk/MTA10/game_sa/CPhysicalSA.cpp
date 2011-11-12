/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        game_sa/CPhysicalSA.cpp
*  PURPOSE:     Physical object entity
*  DEVELOPERS:  Ed Lyons <eai@opencoding.net>
*               Christian Myhre Lundheim <>
*               Cecill Etheredge <ijsf@gmx.net>
*               Jax <>
*               The_GTA <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#include "StdInc.h"

CPhysicalSAInterface::CPhysicalSAInterface()
{
    m_unk2 = 0;

    AllocateMatrix();

    m_velocity.Reset();
    m_spin.Reset();
    m_vecUnk.Reset();
    m_vecUnk2.Reset();
    m_vecUnk3.Reset();
    m_vecUnk4.Reset();

    m_mass = 1;
    m_turnMass = 1;
    m_massUnk = 1;
    
    m_physUnk = 0.1;

    m_link = NULL;

    m_complexStatus = 0;
    m_numCollRecords = 0;
    
    memset( m_collRecords, 0, sizeof( m_collRecords ) );

    m_unk6 = 0;
    m_damageImpulseMagnitude = 0;
    m_damageEntity = NULL;

    m_vecUnk5.Reset();
    m_vecUnk6.Reset();
    m_centerOfMass.Reset();

    m_distanceTravelled = 0;

    m_attachedTo = NULL;
    m_unk8 = 0;
    m_unk9 = 0;
    m_unk10 = 0;
    m_unk11 = 0;
    m_unk12 = 0;

    m_lighting2 = 0;
    m_lighting3 = 0;

    m_unk13 = 10;
    m_nodeFlags = 2;

    m_lighting = 0;
}

void CPhysicalSA::GetMoveSpeed( CVector *velocity )
{
    DEBUG_TRACE("void CPhysicalSA::GetMoveSpeed( CVector *velocity )");
    
    *velocity = m_velocity;
}

void CPhysicalSA::GetTurnSpeed( CVector *spin )
{
    DEBUG_TRACE("void CPhysicalSA::GetTurnSpeed( CVector *spin )");
    
    *spin = m_spin;
}

void CPhysicalSA::SetMoveSpeed(CVector *velocity)
{
    DEBUG_TRACE("void CPhysicalSA::SetMoveSpeed( CVector *velocity )");
    
    m_velocity = *velocity;
}

void CPhysicalSA::SetTurnSpeed(CVector *spin)
{
    DEBUG_TRACE("void CPhysicalSA::SetTurnSpeed( CVector *spin )");
    
    m_spin = *spin;
}

float CPhysicalSA::GetMass ()
{
    return GetInterface()->m_mass;
}

void CPhysicalSA::SetMass ( float mass )
{
    GetInterface()->m_mass = mass;
}

float CPhysicalSA::GetTurnMass ()
{
    return GetInterface()->m_turnMass;
}

void CPhysicalSA::SetTurnMass ( float turnMass )
{
    GetInterface()->m_turnMass = turnMass;
}

float CPhysicalSA::GetElasticity ()
{
    return GetInterface()->m_elasticity;
}

void CPhysicalSA::SetElasticity ( float elasticity )
{
    GetInterface()->m_elasticity = elasticity;
}

float CPhysicalSA::GetBuoyancyConstant ()
{
    return GetInterface()->m_buoyancyConstant;
}

void CPhysicalSA::SetBuoyancyConstant ( float buoyancyConstant )
{
    GetInterface()->m_buoyancyConstant = buoyancyConstant;
}

void CPhysicalSA::ProcessCollision ()
{
    DEBUG_TRACE("void CPhysicalSA::ProcessCollision ()");

    GetInterface()->ProcessCollision();
}

float CPhysicalSA::GetDamageImpulseMagnitude ()
{
    return GetInterface()->m_damageImpulseMagnitude;
}

void CPhysicalSA::SetDamageImpulseMagnitude ( float magnitude )
{
    GetInterface()->m_damageImpulseMagnitude = magnitude;
}

CEntity* CPhysicalSA::GetDamageEntity ()
{
    return pGame->GetPools()->GetEntity( GetInterface()->m_damageEntity );
}

void CPhysicalSA::SetDamageEntity ( CEntity* pEntity )
{
    CEntitySA* pEntitySA = dynamic_cast < CEntitySA* > ( pEntity );

    GetInterface()->m_damageEntity = pEntitySA->GetInterface ();
}

void CPhysicalSA::ResetLastDamage ( void )
{
    GetInterface()->m_damageImpulseMagnitude = 0;
    GetInterface()->m_damageEntity = NULL;
}

CEntity * CPhysicalSA::GetAttachedEntity ( void )
{
    return pGame->GetPools()->GetEntity( GetInterface()->m_attachedTo );
}

void CPhysicalSA::AttachTo ( CPhysical *entity, const CVector& vecPosition, const CVector& vecRotation )
{
    DEBUG_TRACE("void CPhysicalSA::AttachTo( CPhysical& Entity, const CVector& vecPosition, const CVector& vecRotation )");

    InternalAttachTo( (DWORD)dynamic_cast <CPhysicalSA*> ( entity )->GetInterface(), &vecPosition, &vecRotation );
}

void CPhysicalSA::DetachFrom( float fUnkX, float fUnkY, float fUnkZ, bool bUnk )
{
    DEBUG_TRACE("void CPhysicalSA::DetachFrom( float fUnkX, float fUnkY, float fUnk, bool bUnk )");

    DWORD dwFunc = FUNC_DetatchEntityFromEntity;
    DWORD dwThis = (DWORD)GetInterface();

    // DetachFrom appears to crash when there's no entity attached (0x544403, bug 2350)
    // So do a NULL check here
    if ( GetInterface()->m_attachedTo == NULL )
        return;

    _asm
    {
        push    bUnk
        push    fUnkZ
        push    fUnkY
        push    fUnkX
        mov     ecx, dwThis
        call    dwFunc 
    }
}

bool CPhysicalSA::InternalAttachTo(DWORD dwEntityInterface, const CVector * vecPosition, const CVector * vecRotation)
{
    DEBUG_TRACE("bool CPhysicalSA::AttachToEntity(CPhysical * entityToAttach, CVector * vecPosition, CVector * vecRotation)");

    DWORD dwFunc = FUNC_AttachEntityToEntity;
    DWORD dwThis = (DWORD)GetInterface();
    DWORD dwReturn = 0;

    _asm
    {
        mov     ecx, vecRotation
        push    [ecx+8]
        push    [ecx+4]
        push    [ecx]
        mov     ecx, vecPosition
        push    [ecx+8]
        push    [ecx+4]
        push    [ecx]
        push    dwEntityInterface
        mov     ecx, dwThis
        call    dwFunc
        mov     dwReturn, eax   
    }
    return (dwReturn != NULL);
}

void CPhysicalSA::GetAttachedOffsets ( CVector& vecPosition, CVector& vecRotation )
{
    vecPosition = GetInterface()->m_attachOffset;
    vecRotation = GetInterface()->m_attachRotation;
}

void CPhysicalSA::SetAttachedOffsets ( CVector& vecPosition, CVector& vecRotation )
{
    GetInterface()->m_attachOffset = vecPosition;
    GetInterface()->m_attachRotation = vecRotation;
}

float CPhysicalSA::GetLighting ()
{
    return GetInterface()->m_lighting;
}

void CPhysicalSA::SetLighting ( float lighting )
{
    GetInterface()->m_lighting = lighting;
}
