/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.0
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        game_sa/CPhysicalSA.h
*  PURPOSE:     Header file for physical object entity base class
*  DEVELOPERS:  Ed Lyons <eai@opencoding.net>
*               Christian Myhre Lundheim <>
*               Cecill Etheredge <ijsf@gmx.net>
*               Jax <>
*               The_GTA <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#ifndef __CGAMESA_PHYSICAL
#define __CGAMESA_PHYSICAL

#include <game/CPhysical.h>
#include "CEntitySA.h"
#include <CVector.h>

#define FUNC_GetMoveSpeed                       0x404460
#define FUNC_GetTurnSpeed                       0x470030
#define FUNC_ProcessCollision                   0x54DFB0
#define FUNC_AttachEntityToEntity               0x54D570
#define FUNC_DetatchEntityFromEntity            0x5442F0

#define MAX_COLL_RECORDS                        6

class CPhysicalSAInterface : public CEntitySAInterface // begin +56 (256 bytes total?)
{
public:
                                    CPhysicalSAInterface();
                                    ~CPhysicalSAInterface();

    virtual void __thiscall         ProcessEntityCollisions() = 0;

    float                   m_unk13;                    // 56

    BYTE                    m_pad2[3];                  // 60
    unsigned char           m_numImmunities;            // 63
    unsigned int            m_nodeFlags;                // 64

    CVector                 m_velocity;                 // 68
    CVector                 m_spin;                     // 80
    CVector                 m_vecUnk;                   // 92
    CVector                 m_vecUnk2;                  // 104
    CVector                 m_vecUnk3;                  // 116
    CVector                 m_vecUnk4;                  // 128
    float                   m_mass;                     // 140
    float                   m_turnMass;                 // 144
    float                   m_massUnk;                  // 148
    float                   m_physUnk;                  // 152

    float                   m_elasticity;               // 156
    float                   m_buoyancyConstant;         // 160
    CVector                 m_centerOfMass;             // 164

    DWORD                   m_unk2;                     // 176
    CPtrNodeDoubleSA*       m_link;                     // 180

    unsigned char           m_complexStatus;            // 184, used for hydraulics
    unsigned char           m_numCollRecords;           // 185
    unsigned char           m_unk4;                     // 186
    unsigned char           m_unk5;                     // 187

    float                   m_collRecords[MAX_COLL_RECORDS];    // 188

    float                   m_distanceTravelled;        // 212
    float                   m_damageImpulseMagnitude;   // 216
    CEntitySAInterface*     m_damageEntity;             // 220

    CVector                 m_vecUnk5;                  // 224
    CVector                 m_vecUnk6;                  // 236

    unsigned short          m_unk6;                     // 248
    unsigned short          m_unk7;                     // 250

    CEntitySAInterface*     m_attachedTo;               // 252
    CVector                 m_attachOffset;             // 256
    CVector                 m_attachRotation;           // 268

    unsigned int            m_unk10;                    // 280
    unsigned int            m_unk11;                    // 284
    unsigned int            m_unk12;                    // 288
    unsigned int            m_unk9;                     // 292
    unsigned int            m_unk8;                     // 296

    float                   m_lighting;                 // 300, col lighting? CPhysical::GetLightingFromCol
    float                   m_lighting2;                // 304, added to col lighting in CPhysical::GetTotalLighting
    float                   m_lighting3;                // 308
};

class CPhysicalSA : public virtual CPhysical, public virtual CEntitySA
{
public:
    CPhysicalSAInterface* GetInterface      ();

    void        GetMoveSpeed                ( CVector *vecMoveSpeed );
    void        GetTurnSpeed                ( CVector *vecTurnSpeed );
    void        SetMoveSpeed                ( CVector *vecMoveSpeed );
    void        SetTurnSpeed                ( CVector *vecTurnSpeed );

    float       GetMass                     ();
    void        SetMass                     ( float fMass );
    float       GetTurnMass                 ();
    void        SetTurnMass                 ( float fTurnMass );
    float       GetElasticity               ();
    void        SetElasticity               ( float fElasticity );
    float       GetBuoyancyConstant         ();
    void        SetBuoyancyConstant         ( float fBuoyancyConstant );

    void        ProcessCollision            ();

    float       GetDamageImpulseMagnitude   ();
    void        SetDamageImpulseMagnitude   ( float fMagnitude );
    CEntity*    GetDamageEntity             ();
    void        SetDamageEntity             ( CEntity* pEntity );
    void        ResetLastDamage             ();

    CEntity*    GetAttachedEntity           ();
    void        AttachTo                    ( CPhysical& Entity, const CVector& vecPosition, const CVector& vecRotation );
    void        DetachFrom                  ( float fUnkX, float fUnkY, float fUnkZ, bool bUnk );
    void        GetAttachedOffsets          ( CVector & vecPosition, CVector & vecRotation );
    void        SetAttachedOffsets          ( CVector & vecPosition, CVector & vecRotation );

    virtual bool InternalAttachTo           ( DWORD dwEntityInterface, const CVector * vecPosition, const CVector * vecRotation );

    float       GetLighting                 ();
    void        SetLighting                 ( float fLighting );

};

#endif
