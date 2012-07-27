/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.0
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        sdk/game/CAutomobile.h
*  PURPOSE:     Automobile vehicle entity interface
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#ifndef __CGAME_AUTOMOBILE
#define __CGAME_AUTOMOBILE

#include "CDamageManager.h"
#include "CDoor.h"
#include "Common.h"
#include "CVehicle.h"

class CAutomobile : public virtual CVehicle
{
public:
    virtual                     ~CAutomobile()  {};

    virtual CDoor*              GetDoor( unsigned char ucDoor ) = 0;
    virtual void                SetSwingingDoorsAllowed( bool bAllowed ) = 0;
    virtual bool                AreSwingingDoorsAllowed() const = 0;

    virtual void                ExtinguishCarFire() = 0;
    virtual void                PlaceOnRoadProperly() = 0;

    virtual CDamageManager*     GetDamageManager() = 0;

    virtual inline SColor       GetHeadLightColor() const = 0;
    virtual inline void         SetHeadLightColor( const SColor color ) = 0;

    virtual CObject*            SpawnFlyingComponent( int i_1, unsigned int ui_2 ) = 0;
    virtual void                SetWheelVisibility( eWheels wheel, bool bVisible ) = 0;
    virtual bool                GetWheelVisibility( eWheels wheel ) const = 0;

    virtual unsigned short      GetAdjustablePropertyValue() const = 0;
    virtual void                SetAdjustablePropertyValue( unsigned short value ) = 0;

    virtual void                SetTaxiLightOn( bool on ) = 0;
    virtual bool                IsTaxiLightOn() const = 0;

    virtual void                GetTurretRotation( float& horizontal, float& vertical ) const = 0;
    virtual void                SetTurretRotation( float horizontal, float vertical ) = 0;

    virtual void*               GetPrivateSuspensionLines() = 0;

    virtual void                RecalculateHandling() = 0;
    virtual void                RecalculateSuspensionLines() = 0;
    virtual bool                UpdateMovingCollision( float angle ) = 0;
};

#endif
