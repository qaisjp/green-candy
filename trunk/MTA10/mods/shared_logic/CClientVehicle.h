/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*               (Shared logic for modifications)
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        mods/shared_logic/CClientVehicle.h
*  PURPOSE:     Vehicle entity class header
*  DEVELOPERS:  Christian Myhre Lundheim <>
*               Ed Lyons <eai@opencoding.net>
*               Oliver Brown <>
*               Kent Simon <>
*               Jax <>
*               Cecill Etheredge <ijsf@gmx.net>
*               Kevin Whiteside <kevuwk@gmail.com>
*               Chris McArthur <>
*               Derek Abdine <>
*               Stanislav Bobrov <lil_toady@hotmail.com>
*               Alberto Alonso <rydencillo@gmail.com>
*               The_GTA <quiret@gmx.de>
*
*****************************************************************************/

struct CClientVehicleProperties;
class CClientVehicle;

#ifndef __CCLIENTVEHICLE_H
#define __CCLIENTVEHICLE_H

#include <game/CPlane.h>
#include <game/CVehicle.h>

#include "CClientCommon.h"
#include "CClientCamera.h"
#include "CClientModelRequestManager.h"
#include "CClientPed.h"
#include "CClientStreamElement.h"
#include "CClientVehicleManager.h"
#include "CVehicleUpgrades.h"
#include "CClientTime.h"

#define LUACLASS_VEHICLE    64

#define INVALID_PASSENGER_SEAT 0xFF
#define DEFAULT_VEHICLE_HEALTH 1000
#define MAX_VEHICLE_HEALTH 10000

enum eClientVehicleType
{
    CLIENTVEHICLE_NONE,
    CLIENTVEHICLE_CAR,
    CLIENTVEHICLE_BOAT,
    CLIENTVEHICLE_TRAIN,
    CLIENTVEHICLE_HELI,
    CLIENTVEHICLE_PLANE,
    CLIENTVEHICLE_BIKE,
    CLIENTVEHICLE_MONSTERTRUCK,
    CLIENTVEHICLE_QUADBIKE,
    CLIENTVEHICLE_BMX,
    CLIENTVEHICLE_TRAILER
};

enum eDelayedSyncVehicleData
{
    DELAYEDSYNC_VEHICLE_KEYSYNC,
    DELAYEDSYNC_VEHICLE_ROTATION,
    DELAYEDSYNC_VEHICLE_MOVESPEED,
    DELAYEDSYNC_VEHICLE_TURNSPEED,
};

struct SDelayedSyncVehicleData
{
    unsigned long       ulTime;
    unsigned char       ucType;
    CControllerState    State;
    CVector             vecTarget;
    CVector             vecTarget2;
    CVector             vecTarget3;
};

struct SLastSyncedVehData
{
    CVector             vecPosition;
    CVector             vecRotation;
    CVector             vecMoveSpeed;
    CVector             vecTurnSpeed;
    float               fHealth;
    ElementID           Trailer;
};

class CClientProjectile;

class CClientVehicle : public CClientStreamElement
{
    friend class CClientCamera;
    friend class CClientPed;
    friend class CClientVehicleManager;
    friend class CClientGame; // TEMP HACK

protected: // Use CDeathmatchVehicle constructor for now. Will get removed later when this class is
           // cleaned up.
            // The_GTA: ^ lies ^
                                CClientVehicle( CClientManager* pManager, ElementID ID, LuaClass& root, bool system, unsigned short usModel );

public:
                                ~CClientVehicle();
    
    void                        Unlink();

    inline eClientEntityType    GetType() const                                                         { return CCLIENTVEHICLE; };

    void                        GetName( char* szBuf );
    inline const char*          GetNamePointer()                                                        { return m_pModelInfo->GetNameIfVehicle (); };
    inline eClientVehicleType   GetVehicleType()                                                        { return m_eVehicleType; };

    void                        GetPosition( CVector& vecPosition ) const;
    void                        SetPosition( const CVector& vecPosition )                               { SetPosition ( vecPosition, true ); }
    void                        SetPosition( const CVector& vecPosition, bool bResetInterpolation );

    void                        GetRotationDegrees( CVector& vecRotation ) const;
    void                        GetRotationRadians( CVector& vecRotation ) const;
    void                        SetRotationDegrees( const CVector& vecRotation )                        { SetRotationDegrees ( vecRotation, true ); }
    void                        SetRotationDegrees( const CVector& vecRotation, bool bResetInterpolation );
    void                        SetRotationRadians( const CVector& vecRotation )                        { SetRotationRadians ( vecRotation, true ); }
    void                        SetRotationRadians( const CVector& vecRotation, bool bResetInterpolation );
    
    void                        AttachTo( CClientEntity * pEntity );

    float                       GetDistanceFromCentreOfMassToBaseOfModel();

    bool                        GetMatrix( RwMatrix& Matrix ) const;
    bool                        SetMatrix( const RwMatrix& Matrix );
    virtual CSphere             GetWorldBoundingSphere();

    void                        GetMoveSpeed( CVector& vecMoveSpeed ) const;
    void                        GetMoveSpeedMeters( CVector& vecMoveSpeed ) const;
    void                        SetMoveSpeed( const CVector& vecMoveSpeed );
    void                        GetTurnSpeed( CVector& vecTurnSpeed ) const;
    void                        SetTurnSpeed( const CVector& vecTurnSpeed );

    bool                        IsVisible();
    void                        SetVisible( bool bVisible );

    void                        SetDoorOpenRatio( unsigned char ucDoor, float fRatio, unsigned long ulDelay = 0, bool bForced = false );
    float                       GetDoorOpenRatio( unsigned char ucDoor );
    void                        SetSwingingDoorsAllowed( bool bAllowed );
    bool                        AreSwingingDoorsAllowed() const;
    void                        AllowDoorRatioSetting( unsigned char ucDoor, bool bAllow );
    bool                        AreDoorsLocked();
    void                        SetDoorsLocked( bool bLocked );

private:
    void                        SetDoorOpenRatioInterpolated( unsigned char ucDoor, float fRatio, unsigned long ulDelay );
    void                        ResetDoorInterpolation();
    void                        CancelDoorInterpolation( unsigned char ucDoor );
    void                        ProcessDoorInterpolation();

public:
    bool                        AreDoorsUndamageable();
    void                        SetDoorsUndamageable( bool bUndamageable );

    float                       GetHealth() const;
    void                        SetHealth( float fHealth );
    void                        Fix();
    void                        Blow( bool bAllowMovement = false );
    inline bool                 IsVehicleBlown()                                                        { return m_bBlown; };

    CVehicleColor&              GetColor();
    void                        SetColor( const CVehicleColor& color );

    void                        GetTurretRotation( float& fHorizontal, float& fVertical );
    void                        SetTurretRotation( float fHorizontal, float fVertical );

    inline unsigned short       GetModel()                                                              { return m_usModel; };
    void                        SetModelBlocking( unsigned short usModel, bool bLoadImmediately = false );

    bool                        IsEngineBroken();
    void                        SetEngineBroken( bool bEngineBroken );

    bool                        IsEngineOn();
    void                        SetEngineOn( bool bEngineOn );

    bool                        CanBeDamaged();
    void                        CalcAndUpdateCanBeDamagedFlag();
    void                        SetScriptCanBeDamaged( bool bCanBeDamaged );
    void                        SetSyncUnoccupiedDamage( bool bCanBeDamaged );
    bool                        GetScriptCanBeDamaged()                                                 { return m_bScriptCanBeDamaged; };

    bool                        GetTyresCanBurst();
    void                        CalcAndUpdateTyresCanBurstFlag();

    float                       GetGasPedal();

    bool                        IsBelowWater() const;
    bool                        IsDrowning() const;
    bool                        IsDriven() const;
    bool                        IsUpsideDown() const;
    bool                        IsBlown() const;

    bool                        IsSirenOrAlarmActive();
    void                        SetSirenOrAlarmActive( bool bActive );

    inline bool                 HasLandingGear()                                                        { return m_bHasLandingGear; };
    float                       GetLandingGearPosition();
    void                        SetLandingGearPosition( float fPosition );
    bool                        IsLandingGearDown();
    void                        SetLandingGearDown( bool bLandingGearDown );

    inline bool                 HasAdjustableProperty()                                                 { return m_bHasAdjustableProperty; };
    unsigned short              GetAdjustablePropertyValue();
    void                        SetAdjustablePropertyValue( unsigned short usValue );
    bool                        HasMovingCollision();
private:
    void                        _SetAdjustablePropertyValue( unsigned short usValue );
public:

    bool                        HasDamageModel()                                                        { return m_bHasDamageModel; }
    unsigned char               GetDoorStatus( unsigned char ucDoor );
    unsigned char               GetWheelStatus( unsigned char ucWheel );
    unsigned char               GetPanelStatus( unsigned char ucPanel );
    unsigned char               GetLightStatus( unsigned char ucLight );

    void                        SetDoorStatus( unsigned char ucDoor, unsigned char ucStatus );
    void                        SetWheelStatus( unsigned char ucWheel, unsigned char ucStatus, bool bSilent = true );
    void                        SetPanelStatus( unsigned char ucPanel, unsigned char ucStatus );
    void                        SetLightStatus( unsigned char ucLight, unsigned char ucStatus );

    // TODO: Make the class remember on virtualization
    float                       GetHeliRotorSpeed();
    void                        SetHeliRotorSpeed( float fSpeed );

    bool                        IsHeliSearchLightVisible();
    void                        SetHeliSearchLightVisible( bool bVisible );

    void                        ReportMissionAudioEvent( unsigned short usSound );

    inline bool                 IsCollisionEnabled()                                                    { return m_bIsCollisionEnabled; };
    void                        SetCollisionEnabled( bool bCollisionEnabled );

    bool                        GetCanShootPetrolTank();
    void                        SetCanShootPetrolTank( bool bCanShoot );
    
    bool                        GetCanBeTargettedByHeatSeekingMissiles();
    void                        SetCanBeTargettedByHeatSeekingMissiles( bool bEnabled );

    inline unsigned char        GetAlpha()                                                              { return m_ucAlpha; }
    void                        SetAlpha( unsigned char ucAlpha );

    CClientPed*                 GetOccupant( int iSeat = 0 ) const;
    CClientPed*                 GetControllingPlayer();
    void                        ClearForOccupants();

    void                        PlaceProperlyOnGround();

    void                        FuckCarCompletely( bool bKeepWheels );

    void                        WorldIgnore( bool bWorldIgnore );

    inline bool                 IsVirtual()                                                             { return m_pVehicle == NULL; };

    void                        FadeOut( bool bFadeOut );
    bool                        IsFadingOut();

    inline bool                 IsFrozen()                                                              { return m_bIsFrozen; };
    void                        SetFrozen( bool bFrozen );
    void                        SetScriptFrozen( bool bFrozen )                                         { m_bScriptFrozen = bFrozen; };
    bool                        IsFrozenWaitingForGroundToLoad() const;
    void                        SetFrozenWaitingForGroundToLoad( bool bFrozen );

    CClientVehicle*             GetPreviousTrainCarriage();
    CClientVehicle*             GetNextTrainCarriage();
    void                        SetPreviousTrainCarriage( CClientVehicle* pPrevious );
    void                        SetNextTrainCarriage( CClientVehicle* pNext );

    bool                        IsDerailed();
    void                        SetDerailed( bool bDerailed );
    bool                        IsDerailable();
    void                        SetDerailable( bool bDerailable );

    bool                        GetTrainDirection();
    void                        SetTrainDirection( bool bDirection );

    float                       GetTrainSpeed();
    void                        SetTrainSpeed( float fSpeed );

    inline unsigned char        GetOverrideLights()                                                     { return m_ucOverrideLights; }
    void                        SetOverrideLights( unsigned char ucOverrideLights );
    bool                        SetTaxiLightOn( bool bLightOn );
    bool                        IsTaxiLightOn() { return m_bTaxiLightOn; }
    inline CVehicle*            GetGameVehicle()                                                        { return m_pVehicle; }
    inline CEntity*             GetGameEntity()                                                         { return m_pVehicle; }
    inline const CEntity*       GetGameEntity() const                                                   { return m_pVehicle; }
    inline CVehicleUpgrades*    GetUpgrades()                                                           { return m_pUpgrades; }
    inline CModelInfo*          GetModelInfo()                                                          { return m_pModelInfo; }

    CClientVehicle*             GetTowedVehicle();
    CClientVehicle*             GetRealTowedVehicle();
    bool                        SetTowedVehicle( CClientVehicle* pVehicle );
    inline CClientVehicle *     GetTowedByVehicle()                                                     { return m_pTowedByVehicle; }

    eWinchType                  GetWinchType( void )                                                    { return m_eWinchType; }
    bool                        SetWinchType( eWinchType winchType );
    bool                        PickupEntityWithWinch( CClientEntity* pEntity );
    bool                        ReleasePickedUpEntityWithWinch();
    void                        SetRopeHeightForHeli( float fRopeHeight );
    CClientEntity*              GetPickedUpEntityWithWinch();

    inline const char*          GetRegPlate()                                                           { return m_strRegPlate.empty () ? NULL : m_strRegPlate.c_str (); }
    void                        SetRegPlate( const char* szPlate );

    unsigned char               GetPaintjob();
    void                        SetPaintjob( unsigned char ucPaintjob );
    
    float                       GetDirtLevel();
    void                        SetDirtLevel( float fDirtLevel );

    float                       GetDistanceFromGround();

    void                        SetInWater( bool bState )                                               { m_bIsInWater = bState; }
    bool                        IsInWater();
    bool                        IsOnGround();
    bool                        IsOnWater();
    void                        LockSteering( bool bLock );

    bool                        IsSmokeTrailEnabled();
    void                        SetSmokeTrailEnabled( bool bEnabled );

    void                        ResetInterpolation();

    void                        Interpolate();
    void                        UpdateKeysync();

    void                        GetInitialDoorStates( unsigned char * pucDoorStates );

    void                        AddMatrix( RwMatrix& Matrix, double dTime, unsigned short usTickRate );
    void                        AddVelocity( CVector& vecVelocity );


    // Time dependent interpolation
    inline void                 GetTargetPosition( CVector& vecPosition )                               { vecPosition = m_interp.pos.vecTarget; }
    void                        SetTargetPosition( CVector& vecPosition, unsigned long ulDelay, bool bValidVelocityZ = false, float fVelocityZ = 0.f );
    void                        RemoveTargetPosition();
    inline bool                 HasTargetPosition()                                                     { return ( m_interp.pos.ulFinishTime != 0 ); }

    inline void                 GetTargetRotation( CVector& vecRotation )                               { vecRotation = m_interp.rot.vecTarget; }
    void                        SetTargetRotation( CVector& vecRotation, unsigned long ulDelay );
    void                        RemoveTargetRotation();
    inline bool                 HasTargetRotation()                                                     { return ( m_interp.rot.ulFinishTime != 0 ); }

    void                        UpdateTargetPosition();
    void                        UpdateTargetRotation();


    inline unsigned long        GetIllegalTowBreakTime()                                                { return m_ulIllegalTowBreakTime; }
    inline void                 SetIllegalTowBreakTime( unsigned long ulTime )                          { m_ulIllegalTowBreakTime = ulTime; }

    void                        GetGravity( CVector& vecGravity ) const                                 { vecGravity = m_vecGravity; }
    void                        SetGravity( const CVector& vecGravity );

    SColor                      GetHeadLightColor();
    void                        SetHeadLightColor( const SColor color );

    int                         GetCurrentGear();

    bool                        IsEnterable();
    bool                        HasRadio();
    bool                        HasPoliceRadio();

    void                        ReCreate();

    void                        ModelRequestCallback( CModelInfo* pModelInfo );

    // Warning: Don't use this to create a vehicle if CClientVehicleManager::IsVehicleLimitReached
    //          returns true. Also this messes with streaming so don't Destroy something unless
    //          you're going to recreate it very quickly again. CClientVehicleManager::IsVehicleLimitReached
    //          returns true often when the player is in an area full of vehicles so don't fatal
    //          error or something if it does return true.
    void                        Create();
    void                        Destroy();

    typedef std::list <CClientProjectile*> projectiles_t;

    inline void                 AddProjectile( CClientProjectile * pProjectile )                        { m_Projectiles.push_back ( pProjectile ); }
    inline void                 RemoveProjectile( CClientProjectile * pProjectile )                     { m_Projectiles.remove ( pProjectile ); }
    projectiles_t::iterator     ProjectilesBegin()                                                      { return m_Projectiles.begin (); }
    projectiles_t::iterator     ProjectilesEnd()                                                        { return m_Projectiles.end (); }

    void                        RemoveAllProjectiles();

    static void                 SetPedOccupiedVehicle( CClientPed* pClientPed, CClientVehicle* pVehicle, unsigned int uiSeat, unsigned char ucDoor );
    static void                 SetPedOccupyingVehicle( CClientPed* pClientPed, CClientVehicle* pVehicle, unsigned int uiSeat, unsigned char ucDoor );
    static void                 ValidatePedAndVehiclePair( CClientPed* pClientPed, CClientVehicle* pVehicle );
    static void                 UnpairPedAndVehicle( CClientPed* pClientPed, CClientVehicle* pVehicle );
    static void                 UnpairPedAndVehicle( CClientPed* pClientPed );
    
    void                        ApplyHandling();
    CHandlingEntry*             GetHandlingData();
    const CHandlingEntry*       GetOriginalHandlingData()                                               { return m_pOriginalHandlingEntry; }

protected:
    void                        StreamIn( bool bInstantly );
    void                        StreamOut();

    void                        NotifyCreate();
    void                        NotifyDestroy();

    bool                        DoCheckHasLandingGear();
    void                        HandleWaitingForGroundToLoad();

    void                        StreamedInPulse();
    void                        Dump( FILE* pFile, bool bDumpDetails, unsigned int uiIndex );

    class CClientObjectManager* m_pObjectManager;
    CClientVehicleManager*      m_pVehicleManager;
    CClientModelRequestManager* m_pModelRequester;
    unsigned short              m_usModel;
    bool                        m_bHasLandingGear;
    eClientVehicleType          m_eVehicleType;
    unsigned char               m_ucMaxPassengers;
    bool                        m_bIsVirtualized;
    CVehicle*                   m_pVehicle;
    CClientPed*                 m_pDriver;
    CClientPed*                 m_pPassengers [8];
    CClientPed*                 m_pOccupyingDriver;
    CClientPed*                 m_pOccupyingPassengers [8];
    RpClump*                    m_pClump;
    short                       m_usRemoveTimer;

    CClientVehicle*             m_pPreviousLink;
    CClientVehicle*             m_pNextLink;
    RwMatrix                    m_Matrix;
    RwMatrix                    m_MatrixLast;
    RwMatrix                    m_MatrixPure;   
    CVector                     m_vecMoveSpeedInterpolate;
    CVector                     m_vecMoveSpeedMeters;
    CVector                     m_vecMoveSpeed;
    CVector                     m_vecTurnSpeed;
    float                       m_fHealth;
    float                       m_fTurretHorizontal;
    float                       m_fTurretVertical;
    float                       m_fGasPedal;
    bool                        m_bVisible;
    bool                        m_bIsCollisionEnabled;
    bool                        m_bEngineOn;
    bool                        m_bEngineBroken;
    bool                        m_bSireneOrAlarmActive;
    bool                        m_bLandingGearDown;
    bool                        m_bHasAdjustableProperty;
    unsigned short              m_usAdjustablePropertyValue;
    bool                        m_bAllowDoorRatioSetting[6];
    float                       m_fDoorOpenRatio[6];

    struct
    {
        float                   fStart[6];
        float                   fTarget[6];
        unsigned long           ulStartTime[6];
        unsigned long           ulTargetTime[6];
    } m_doorInterp;

    bool                        m_bSwingingDoorsAllowed;
    bool                        m_bDoorsLocked;
    bool                        m_bDoorsUndamageable;
    bool                        m_bCanShootPetrolTank;
    bool                        m_bCanBeTargettedByHeatSeekingMissiles;
    bool                        m_bCanBeDamaged;
    bool                        m_bScriptCanBeDamaged;
    bool                        m_bSyncUnoccupiedDamage;
    bool                        m_bTyresCanBurst;
    unsigned char               m_ucDoorStates [MAX_DOORS];
    unsigned char               m_ucWheelStates [MAX_WHEELS];
    unsigned char               m_ucPanelStates [MAX_PANELS];
    unsigned char               m_ucLightStates [MAX_LIGHTS];
    bool                        m_bJustBlewUp;
    eEntityStatus               m_NormalStatus;
    bool                        m_bColorSaved;
    CVehicleColor               m_Color;
    bool                        m_bIsFrozen;
    bool                        m_bScriptFrozen;
    bool                        m_bFrozenWaitingForGroundToLoad;
    float                       m_fGroundCheckTolerance;
    float                       m_fObjectsAroundTolerance;
    RwMatrix                    m_matFrozen;
    CVehicleUpgrades*           m_pUpgrades;
    unsigned char               m_ucOverrideLights;
    CClientVehicle*             m_pTowedVehicle;
    CClientVehicle*             m_pTowedByVehicle;
    eWinchType                  m_eWinchType;
    CClientEntity*              m_pPickedUpWinchEntity;
    std::string                 m_strRegPlate;
    unsigned char               m_ucPaintjob;
    float                       m_fDirtLevel;
    bool                        m_bSmokeTrail;
    unsigned char               m_ucAlpha;
    bool                        m_bAlphaChanged;
    double                      m_dLastRotationTime;
    bool                        m_bBlowNextFrame;
    bool                        m_bIsOnGround;
    bool                        m_bHeliSearchLightVisible;
    float                       m_fHeliRotorSpeed;
    const CHandlingEntry*       m_pOriginalHandlingEntry;
    CHandlingEntry*             m_pHandlingEntry;

    bool                        m_bIsDerailed;
    bool                        m_bIsDerailable;
    bool                        m_bTrainDirection;
    float                       m_fTrainSpeed;

    // Time dependent error compensation interpolation
    struct
    {
        struct
        {
#ifdef MTA_DEBUG
            CVector         vecStart;
#endif
            CVector         vecTarget;
            CVector         vecError;
            float           fLastAlpha;
            unsigned long   ulStartTime;
            unsigned long   ulFinishTime;
        } pos;

        struct
        {
#ifdef MTA_DEBUG
            CVector         vecStart;
#endif
            CVector         vecTarget;
            CVector         vecError;
            float           fLastAlpha;
            unsigned long   ulStartTime;
            unsigned long   ulFinishTime;
        } rot;
    } m_interp;

    unsigned long               m_ulIllegalTowBreakTime;

    bool                        m_bBlown;
    bool                        m_bHasDamageModel;

    bool                        m_bTaxiLightOn;
    projectiles_t               m_Projectiles;

    bool                        m_bIsInWater;

    CVector                     m_vecGravity;
    SColor                      m_HeadLightColor;

    bool                        m_bHasCustomHandling;

public:
    CClientPlayer *             m_pLastSyncer;
    unsigned long               m_ulLastSyncTime;
    char *                      m_szLastSyncType;
    SLastSyncedVehData*         m_LastSyncedData;
};

#endif
