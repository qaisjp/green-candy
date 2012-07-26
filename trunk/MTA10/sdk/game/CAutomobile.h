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

    virtual CDoor*              GetDoor( unsigned char ucDoor );
    virtual void                SetSwingingDoorsAllowed( bool bAllowed );
    virtual bool                AreSwingingDoorsAllowed() const;

    virtual void                ExtinguishCarFire();
    virtual void                PlaceOnRoadProperly();

    virtual CDamageManager*     GetDamageManager();

    virtual inline SColor       GetHeadLightColor() const;
    virtual inline void         SetHeadLightColor( const SColor color );

    virtual CObject*            SpawnFlyingComponent( int i_1, unsigned int ui_2 );
    virtual void                SetWheelVisibility( eWheels wheel, bool bVisible );
    virtual bool                GetWheelVisibility( eWheels wheel ) const;

    virtual unsigned short      GetAdjustablePropertyValue() const;
    virtual void                SetAdjustablePropertyValue( unsigned short value );

    virtual void                SetTaxiLightOn( bool on );
    virtual bool                IsTaxiLightOn() const;

    virtual void                GetTurretRotation( float& horizontal, float& vertical ) const;
    virtual void                SetTurretRotation( float horizontal, float vertical );

    virtual void*               GetPrivateSuspensionLines();

    virtual void                RecalculateHandling();
    virtual void                RecalculateSuspensionLines();
    virtual bool                UpdateMovingCollision( float angle );
};

#endif
