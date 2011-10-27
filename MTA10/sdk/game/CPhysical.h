/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.0
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        sdk/game/CPhysical.h
*  PURPOSE:     Physical entity interface
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#ifndef __CGAME_PHYSICAL
#define __CGAME_PHYSICAL

#include "CEntity.h"

class CPhysical : public virtual CEntity
{
public:
    virtual             ~CPhysical ( void ) {};

    virtual void        GetMoveSpeed                ( CVector moveSpeed ) = 0;
    virtual void        GetTurnSpeed                ( CVector turnSpeed ) = 0;
    virtual void        SetMoveSpeed                ( CVector moveSpeed ) = 0;
    virtual void        SetTurnSpeed                ( CVector turnSpeed ) = 0;

    virtual float       GetMass                     ( void ) = 0;
    virtual void        SetMass                     ( float fMass ) = 0;
    virtual float       GetTurnMass                 ( void ) = 0;
    virtual void        SetTurnMass                 ( float fTurnMass ) = 0;
    virtual float       GetElasticity               ( void ) = 0;
    virtual void        SetElasticity               ( float fElasticity ) = 0;
    virtual float       GetBuoyancyConstant         ( void ) = 0;
    virtual void        SetBuoyancyConstant         ( float fBuoyancyConstant ) = 0;

    virtual void        ProcessCollision() = 0;

    virtual float       GetDamageImpulseMagnitude   ( void ) = 0;
    virtual void        SetDamageImpulseMagnitude   ( float fMagnitude ) = 0;
    virtual CEntity*    GetDamageEntity             ( void ) = 0;
    virtual void        SetDamageEntity             ( CEntity* pEntity ) = 0;
    virtual void        ResetLastDamage             ( void ) = 0;

    virtual CEntity *   GetAttachedEntity           ( void ) = 0;
    virtual void        AttachEntityToEntity        ( CPhysical& Entity, const CVector& vecPosition, const CVector& vecRotation ) = 0;
    virtual void        DetachEntityFromEntity      ( float fUnkX, float fUnkY, float fUnkZ, bool bUnk ) = 0;
    virtual void        GetAttachedOffsets          ( CVector & vecPosition, CVector & vecRotation ) = 0;
    virtual void        SetAttachedOffsets          ( CVector & vecPosition, CVector & vecRotation ) = 0;

    virtual float       GetLighting                 ( void ) = 0;
    virtual void        SetLighting                 ( float fLighting ) = 0;
};

#endif
