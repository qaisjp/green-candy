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

typedef void    (__cdecl*CollideColModels_t)    ( const RwMatrix& mat, CColModelSAInterface *colModel, const RwMatrix& withMat, CColModelSAInterface *withColModel );
CollideColModels_t      CollideColModels =      (CollideColModels_t)0x004185C0;

CPhysicalSAInterface::CPhysicalSAInterface()
{
    m_unk2 = 0;

    AllocateMatrix();

#if 0
    m_velocity.Reset();
    m_spin.Reset();
    m_vecUnk.Reset();
    m_vecUnk2.Reset();
    m_vecUnk3.Reset();
    m_vecUnk4.Reset();
#endif

    m_mass = 1;
    m_turnMass = 1;
    m_massUnk = 1;
    
    m_physUnk = 0.1f;

    m_link = NULL;

    m_complexStatus = 0;
    m_numCollRecords = 0;
    
    memset( m_collRecords, 0, sizeof( m_collRecords ) );

    m_unk6 = 0;
    m_damageImpulseMagnitude = 0;
    m_damageEntity = NULL;

#if 0
    m_vecUnk5.Reset();
    m_vecUnk6.Reset();
    m_centerOfMass.Reset();
#endif

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

CPhysicalSAInterface::~CPhysicalSAInterface()
{
}

void Physical_Init()
{
}

void Physical_Shutdown()
{
}

CEntity* CPhysicalSA::GetDamageEntity() const
{
    return pGame->GetPools()->GetEntity( GetInterface()->m_damageEntity );
}

void CPhysicalSA::SetDamageEntity( CEntity *entity )
{
    CEntitySA* pEntitySA = dynamic_cast <CEntitySA*> ( entity );

    if ( !pEntitySA )
        return;

    GetInterface()->m_damageEntity = pEntitySA->GetInterface();
}

void CPhysicalSA::ResetLastDamage()
{
    GetInterface()->m_damageImpulseMagnitude = 0;
    GetInterface()->m_damageEntity = NULL;
}

CEntity* CPhysicalSA::GetAttachedEntity() const
{
    return pGame->GetPools()->GetEntity( GetInterface()->m_attachedTo );
}

void CPhysicalSA::AttachTo( CPhysical& Entity, const CVector& pos, const CVector& rot )
{
    DEBUG_TRACE("void CPhysicalSA::AttachTo( CPhysical& Entity, const CVector& pos, const CVector& rot )");

    InternalAttachTo( dynamic_cast <CPhysicalSA&> ( Entity ).GetInterface(), pos, rot );
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

bool CPhysicalSA::InternalAttachTo( CEntitySAInterface *entity, const CVector& pos, const CVector& rot )
{
    DEBUG_TRACE("bool CPhysicalSA::InternalAttachTo( CEntitySAInterface *entity, const CVector& pos, const CVector& rot )");

    DWORD dwFunc = FUNC_AttachEntityToEntity;
    DWORD dwThis = (DWORD)GetInterface();
    DWORD dwReturn = 0;

    _asm
    {
        mov     ecx,rot
        push    [ecx+8]
        push    [ecx+4]
        push    [ecx]
        mov     ecx,pos
        push    [ecx+8]
        push    [ecx+4]
        push    [ecx]
        push    entity
        mov     ecx, dwThis
        call    dwFunc
        mov     dwReturn, eax   
    }
    return dwReturn != NULL;
}

void CPhysicalSA::GetAttachedOffsets( CVector& pos, CVector& rot ) const
{
    pos = GetInterface()->m_attachOffset;
    rot = GetInterface()->m_attachRotation;
}

void CPhysicalSA::SetAttachedOffsets( const CVector& pos, const CVector& rot )
{
    GetInterface()->m_attachOffset = pos;
    GetInterface()->m_attachRotation = rot;
}

void CPhysicalSA::GetImmunities( bool& bNoClip, bool& bFrozen, bool& bBulletProof, bool & bFlameProof, bool & bUnk, bool & bUnk2, bool & bCollisionProof, bool & bExplosionProof ) const
{
    unsigned char ucImmunities = GetInterface()->m_numImmunities;
    bNoClip = ( ucImmunities & 0x1 ) ? true:false;
    bFrozen = ( ucImmunities & 0x2 ) ? true:false;
    bBulletProof = ( ucImmunities & 0x4 ) ? true:false;
    bFlameProof = ( ucImmunities & 0x8 ) ? true:false;
    bUnk = ( ucImmunities & 0x10 ) ? true:false;
    bUnk2 = ( ucImmunities & 0x20 ) ? true:false;
    bCollisionProof = ( ucImmunities & 0x40 ) ? true:false;
    bExplosionProof = ( ucImmunities & 0x80 ) ? true:false;
}