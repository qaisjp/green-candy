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

#define PHYSICAL_MAXNOOFCOLLISIONRECORDS        6

class CPhysicalSAInterface : public CEntitySAInterface // begin +68 (244 bytes total?)
{
public:
/* IMPORTANT: KEEP the padding in CVehicle UP-TO-DATE if you add something here (or eventually pad someplace else) */
    CVector                 m_velocity;                 // 68
    CVector                 m_spin;                     // 80
    float                   m_pad[12];                  // 92
    float                   m_mass;                     // 140
    float                   m_turnMass;                 // 144

    DWORD                   m_unk;                      // 148
    DWORD                   m_physUnk;                  // 152

    float                   m_elasticity;               // 156
    float                   m_buoyancyConstant;         // 160
    CVector                 m_centerOfMass;             // 164

    DWORD                   m_unk2;                     // 176
    CPtrNodeDoubleSA*       m_link;                     // 180

    BYTE                    m_unk3;                     // 184
    BYTE                    m_collRecords;              // 185
    BYTE                    m_unk4;                     // 186
    BYTE                    m_unk5;                     // 187

    float                   m_pad2[6];                  // 188

    float                   m_distanceTravelled;        // 212
    float                   m_damageImpulseMagnitude;   // 216
    CEntitySAInterface*     m_damageEntity;             // 220

    BYTE                    m_pad3[28];                 // 224
    CEntitySAInterface*     m_attachedTo;               // 252
    CVector                 m_attachOffset;             // 256
    CVector                 m_attachRotation;           // 268
    BYTE                    m_pad4[20];                 // 280
    float                   m_lighting;                 // 300, col lighting? CPhysical::GetLightingFromCol
    float                   m_lighting2;                // 304, added to col lighting in CPhysical::GetTotalLighting
    BYTE                    m_pad5[4];                  // 308
};

class CPhysicalSA : public virtual CPhysical, public virtual CEntitySA
{
public:
    CVector *   GetMoveSpeed                ( CVector * vecMoveSpeed );
    CVector *   GetTurnSpeed                ( CVector * vecTurnSpeed );
    VOID        SetMoveSpeed                ( CVector * vecMoveSpeed );
    VOID        SetTurnSpeed                ( CVector * vecTurnSpeed );

    float       GetMass                     ( void );
    void        SetMass                     ( float fMass );
    float       GetTurnMass                 ( void );
    void        SetTurnMass                 ( float fTurnMass );
    float       GetElasticity               ( void );
    void        SetElasticity               ( float fElasticity );
    float       GetBuoyancyConstant         ( void );
    void        SetBuoyancyConstant         ( float fBuoyancyConstant );

    VOID        ProcessCollision            ( void );

    float       GetDamageImpulseMagnitude   ( void );
    void        SetDamageImpulseMagnitude   ( float fMagnitude );
    CEntity*    GetDamageEntity             ( void );
    void        SetDamageEntity             ( CEntity* pEntity );
    void        ResetLastDamage             ( void );

    CEntity *   GetAttachedEntity           ( void );
    void        AttachEntityToEntity        ( CPhysical& Entity, const CVector& vecPosition, const CVector& vecRotation );
    void        DetachEntityFromEntity      ( float fUnkX, float fUnkY, float fUnkZ, bool bUnk );
    void        GetAttachedOffsets          ( CVector & vecPosition, CVector & vecRotation );
    void        SetAttachedOffsets          ( CVector & vecPosition, CVector & vecRotation );

    virtual bool InternalAttachEntityToEntity ( DWORD dwEntityInterface, const CVector * vecPosition, const CVector * vecRotation );

    float       GetLighting                 ( void );
    void        SetLighting                 ( float fLighting );

    /*
    VOID        SetMassMultiplier(FLOAT fMassMultiplier);
    FLOAT       GetMassMultiplier();
    VOID        SetAirResistance(FLOAT fAirResistance);
    FLOAT       GetAirResistance();
    VOID        SetCenterOfMass(CVector * vecCenterOfMass);
    CVector * GetCenterOfMass();

    BOOL        GetExtraHeavy();
    VOID        SetExtraHeavy(BOOL bExtraHeavy);
    BOOL        GetDoGravity();
    VOID        SetDoGravity(BOOL bDoGravity);
    BOOL        GetInfiniteMass();
    VOID        SetInfiniteMass(BOOL bInfiniteMass);
    BOOL        GetPositionFrozen();
    VOID        SetPositionFrozen(BOOL bPositionFrozen);
    BYTE        GetLastMaterialToHaveBeenStandingOn();

    BYTE        GetLevel();
    VOID        SetLevel(BYTE LivesInThisLevel);*/

};

#endif
