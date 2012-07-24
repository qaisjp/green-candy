/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.0
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        sdk/game/CVehicle.h
*  PURPOSE:     Vehicle entity interface
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#ifndef __CGAME_VEHICLE
#define __CGAME_VEHICLE

#include "CColPoint.h"
#include "CPhysical.h"
#include "CDamageManager.h"
#include "CHandlingManager.h"
#include "CDoor.h"
#include "CWeaponInfo.h"

enum eWinchType
{
    WINCH_NONE = 0,
    WINCH_BIG_MAGNET,
    WINCH_SMALL_MAGNET = 3
};

class CPed;
class CObject;
class CColModel;

class CVehicle : public virtual CPhysical
{
public:
    virtual                     ~CVehicle() {};

    virtual float               GetHealth() const;
    virtual void                SetHealth( float fHealth );

    virtual unsigned char       GetNumberGettingIn() const;
    virtual unsigned char       GetPassengerCount() const;
    virtual unsigned char       GetMaxPassengerCount() const;

    virtual CDoor*              GetDoor( unsigned char ucDoor );
    virtual void                OpenDoor( unsigned char ucDoor, float fRatio, bool bMakeNoise = false );
    virtual bool                AreDoorsLocked() const;
    virtual void                LockDoors( bool bLocked );
    virtual bool                AreDoorsUndamageable() const;
    virtual void                SetDoorsUndamageable( bool bUndamageable );

    virtual void                SetAlpha( unsigned char alpha );
    virtual unsigned char       GetAlpha() const;
    virtual void                SetColor( SColor color1, SColor color2, SColor color3, SColor color4, int );
    virtual void                GetColor( SColor& color1, SColor& color2, SColor& color3, SColor& color4, int ) const;

    // Virtually internal shared functions
    virtual void                BurstTyre( unsigned char tyre );
    virtual bool                CanPedEnterCar() const;
    virtual bool                CanPedJumpOutCar( CPed *ped ) const;
    virtual bool                CanPedLeanOut( CPed *ped ) const;
    virtual bool                CanPedStepOutCar( bool unk ) const;
    virtual bool                CarHasRoof() const;
    virtual bool                GetTowBarPos( CVector& pos ) const;
    virtual bool                GetTowHitchPos( CVector& pos ) const;
    virtual bool                SetTowLink( CVehicle *vehicle );
    virtual bool                BreakTowLink();
    virtual float               GetHeightAboveRoad() const;
    virtual void                Fix();
    virtual void                BlowUp( CEntity *creator, unsigned long unk );
    virtual void                BlowUpCutSceneNoExtras( unsigned int unk1, unsigned int unk2, unsigned int unk3, unsigned int unk4 );
    virtual void                RecalculateSuspensionLines();

    virtual CVehicle*           GetTowedVehicle() const;
    virtual CVehicle*           GetTowedByVehicle() const;

    virtual void                SetExplodeTime( unsigned long ulTime );
    virtual unsigned long       GetExplodeTime() const;

    virtual bool                IsOnItsSide() const;
    virtual bool                IsLawEnforcementVehicle() const;

    virtual unsigned char       GetCurrentGear() const;
    virtual float               GetGasPedal() const;
    virtual float               GetSteerAngle() const;

    virtual bool                AddProjectile( eWeaponType eWeapon, CVector vecOrigin, float fForce, CVector * target, CEntity * targetEntity );
    virtual void                AddVehicleUpgrade( unsigned short model );
    virtual void                RemoveVehicleUpgrade( unsigned short model );

    virtual unsigned int        GetBaseVehicleType() const;

    virtual void                SetBodyDirtLevel( float fDirtLevel );
    virtual float               GetBodyDirtLevel() const;

    virtual CPed*               GetDriver() const;
    virtual CPed*               GetPassenger( unsigned char ucSlot ) const;
    virtual bool                IsBeingDriven() const;
    virtual bool                IsPassenger( CPed *ped ) const;
    virtual bool                IsSphereTouchingVehicle( CVector * vecOrigin, float fRadius ) const;
    virtual bool                IsUpsideDown() const;
    virtual void                MakeDirty( CColPoint *point );

    virtual void                PlaceOnRoadProperly();

    virtual void                SetRemap( int iRemap );
    virtual int                 GetRemapIndex() const;
    virtual void                SetRemapTexDictionary( int iRemapTextureDictionary );

    virtual bool                IsEngineBroken() const;
    virtual bool                IsScriptLocked() const;
    virtual bool                IsDamaged() const;
    virtual bool                IsDrowning() const;
    virtual bool                IsEngineOn() const;
    virtual bool                IsHandbrakeOn() const;
    virtual bool                IsRCVehicle() const;
    virtual bool                GetAlwaysLeaveSkidMarks() const;
    virtual bool                GetCanBeDamaged() const;
    virtual bool                GetCanBeTargettedByHeatSeekingMissiles() const;
    virtual bool                GetCanShootPetrolTank() const;
    virtual bool                GetChangeColourWhenRemapping() const;
    virtual bool                GetComedyControls() const;
    virtual bool                GetGunSwitchedOff() const;
    virtual bool                GetLightsOn() const;
    virtual bool                GetTakeLessDamage() const;
    virtual bool                GetTyresDontBurst() const;
    virtual bool                IsSirenOrAlarmActive() const;
    virtual bool                IsFadingOut() const;
    virtual unsigned int        GetOverrideLights() const;
    
    virtual void                SetEngineBroken( bool bEngineBroken );
    virtual void                SetScriptLocked( bool bLocked );
    virtual void                SetAlwaysLeaveSkidMarks( bool bAlwaysLeaveSkidMarks );
    virtual void                SetCanBeDamaged( bool bCanBeDamaged );
    virtual void                SetCanBeTargettedByHeatSeekingMissiles( bool bEnabled );
    virtual void                SetCanShootPetrolTank( bool bCanShoot );
    virtual void                SetChangeColourWhenRemapping( bool bChangeColour );
    virtual void                SetComedyControls( bool bComedyControls );
    virtual void                SetEngineOn( bool bEngineOn );
    virtual void                SetGunSwitchedOff( bool bGunsOff );
    virtual void                SetHandbrakeOn( bool bHandbrakeOn );
    virtual void                SetLightsOn( bool bLightsOn );
    virtual void                SetOverrideLights( unsigned int uiOverrideLights );
    virtual void                SetTakeLessDamage( bool bTakeLessDamage );
    virtual void                SetTyresDontBurst( bool bTyresDontBurst );
    virtual void                SetSirenOrAlarmActive( bool active );
    virtual void                SetFadingOut( bool enable );

    virtual CHandlingEntry*     GetHandlingData();
    virtual void                SetHandlingData( CHandlingEntrySA *handling );

    virtual void                GetGravity( CVector& grav ) const;
    virtual void                SetGravity( const CVector& grav );

    virtual CColModel*          GetSpecialColModel() const;

    virtual void                RecalculateHandling();
};

#endif
