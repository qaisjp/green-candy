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
    virtual                         ~CAutomobile()  {};

    virtual unsigned char           GetCurrentGear() const = 0;
    virtual float                   GetGasPedal() const = 0;
    virtual float                   GetHeightAboveRoad() const = 0;
    virtual float                   GetSteerAngle() const = 0;
    virtual bool                    GetTowBarPos( CVector *pVector ) const = 0;
    virtual bool                    GetTowHitchPos( CVector *pVector ) const = 0;
    virtual bool                    IsOnItsSide() const = 0;
    virtual bool                    IsLawEnforcementVehicle() const = 0;

    virtual bool                    CanPedEnterCar() const = 0;
    virtual bool                    CanPedJumpOutCar( CPed *ped ) const = 0;
    virtual void                    AddVehicleUpgrade( unsigned short model ) = 0;
    virtual void                    RemoveVehicleUpgrade( unsigned short model ) = 0;
    virtual bool                    CanPedLeanOut( CPed* pPed ) const = 0;
    virtual bool                    CanPedStepOutCar( bool bUnknown ) const = 0;

    virtual CDoor*                  GetDoor( unsigned char ucDoor ) const = 0;
    virtual void                    OpenDoor( unsigned char ucDoor, float fRatio, bool bMakeNoise = false ) = 0;
    virtual void                    SetSwingingDoorsAllowed( bool bAllowed ) = 0;
    virtual bool                    AreSwingingDoorsAllowed() const = 0;
    virtual bool                    AreDoorsLocked() const = 0;
    virtual void                    LockDoors( bool bLocked ) = 0;
    virtual bool                    AreDoorsUndamageable() const = 0;
    virtual void                    SetDoorsUndamageable( bool bUndamageable ) = 0;

    virtual bool                    CarHasRoof() const = 0;
    virtual void                    ExtinguishCarFire() = 0;

    virtual void                    SetColor( SColor color1, SColor color2, SColor color3, SColor color4, int ) = 0;
    virtual void                    GetColor( SColor *color1, SColor *color2, SColor *color3, SColor *color4, int ) const = 0;
    virtual bool                    IsSirenOrAlarmActive() const = 0;
    virtual void                    SetSirenOrAlarmActive( bool bActive ) = 0;

    virtual void                    Fix() = 0;
    virtual void                    BlowUp( CEntity *pCreator, unsigned long ulUnknown ) = 0;
    virtual void                    BlowUpCutSceneNoExtras( unsigned long ulUnknown1, unsigned long ulUnknown2, unsigned long ulUnknown3, unsigned long ulUnknown4 ) = 0;

    virtual void                    SetAlpha( unsigned char alpha ) = 0;
    virtual unsigned char           GetAlpha() const = 0;

    virtual void                    FadeOut( bool bFadeOut ) = 0;
    virtual bool                    IsFadingOut() const = 0;

    virtual CDamageManager*         GetDamageManager() = 0;

    virtual bool                    SetTowLink( CVehicle *pVehicle ) = 0;
    virtual bool                    BreakTowLink() = 0;
    virtual CAutomobile*            GetTowedVehicle() const = 0;
    virtual CAutomobile*            GetTowedByVehicle() const = 0;

    virtual void                    BurstTyre( unsigned char tyre ) = 0;

    virtual void                    SetAdjustablePropertyValue( unsigned short usAdjustableProperty ) = 0;
    virtual void                    SetTaxiLightOn( bool bLightState ) = 0;
    virtual void                    SetExplodeTime( unsigned long ulTime ) = 0;

    virtual void                    GetTurretRotation( float& horizontal, float& vertical ) = 0;
    virtual void                    SetTurretRotation( float horizontal, float vertical ) = 0;

    virtual SColor                  GetHeadLightColor() const = 0;
    virtual void                    SetHeadLightColor( const SColor color ) = 0;

    virtual CObject*                SpawnFlyingComponent( int i_1, unsigned int ui_2 ) = 0;
    virtual void                    SetWheelVisibility( eWheels wheel, bool bVisible ) = 0;

    virtual void                    RecalculateHandling() = 0;
    virtual void*                   GetPrivateSuspensionLines() = 0;

    virtual CColModel*              GetSpecialColModel() = 0;
    virtual bool                    UpdateMovingCollision( float fAngle ) = 0;
};

#endif
