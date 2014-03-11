/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.0
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        game_sa/CPedSA.h
*  PURPOSE:     Header file for ped entity base class
*  DEVELOPERS:  Ed Lyons <eai@opencoding.net>
*               Christian Myhre Lundheim <>
*               Cecill Etheredge <ijsf@gmx.net>
*               Jax <>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

class CPedSA;

#ifndef __CGAMESA_PED
#define __CGAMESA_PED

#include <game/CPed.h>
#include <game/CWeapon.h>

#include "CEntitySA.h"
#include "CFireSA.h"
#include "CPedIKSA.h"
#include "CPhysicalSA.h"

#include "CPedSoundSA.h"
#include "CWeaponSA.h"

class CPedIntelligenceSA;
class CPlayerPedDataSAInterface;
class CPedIntelligenceSAInterface;

#define SIZEOF_CPLAYERPED                           1956

#define FUNC_SetModelIndex                          0x5E4880 // ##SA##
#define FUNC_AttachPedToBike                        0x5E7E60 
#define FUNC_AttachPedToEntity                      0x5E7CB0
#define FUNC_CanSeeEntity                           0x5E0730
#define FUNC_GiveObjectToPedToHold                  0x5E4390
#define FUNC_RestorePlayerStuffDuringResurrection   0x442060
#define FUNC_SetIsStanding                          0x4ABBE0
#define FUNC_MakeChangesForNewWeapon_Slot           0x60D000

#define FUNC_CPedClothesDesc__SetTextureAndModel    0x5A8080
#define FUNC_CClothes__RebuildPlayer                0x5A82C0

#define FUNC_QuitEnteringCar                        0x650130 // really belongs in CCarEnterExit

#define FUNC_CanPedReturnToState        0x5018D0
#define FUNC_ClearAimFlag               0x50B4A0
#define FUNC_ClearAll                   0x509DF0
#define FUNC_ClearAttackByRemovingAnim  0x52CF70
#define FUNC_ClearInvestigateEvent      0x526BA0
#define FUNC_ClearLookFlag              0x50B9C0
#define FUNC_ClearObjective             0x521720
#define FUNC_ClearPointGunAt            0x52DBE0
#define FUNC_ClearWeapons               0x4FF740
#define FUNC_CreateDeadPedMoney         0x43E2C0
#define FUNC_CreateDeadPedWeaponPickups 0x43DF30
#define FUNC_GetLocalDirection          0x5035F0
#define FUNC_GetWeaponSlot              0x5DF200 // ##SA##
#define FUNC_SetCurrentWeapon           0x5E61F0 // ##SA##
#define FUNC_GiveWeapon                 0x5E6080 // ##SA##
#define FUNC_GetBonePosition            0x5E4280 // ##SA##
#define FUNC_GetTransformedBonePosition 0x5E01C0 // ##SA##
#define FUNC_IsWearingGoggles           0x479D10 // ##SA##
#define FUNC_TakeOffGoggles             0x5E6010 // ##SA##
#define FUNC_PutOnGoggles               0x5E3AE0 // ##SA##
#define FUNC_RemoveWeaponModel          0x5E3990 // ##SA## (call with -1 to remove any model, I think)
#define FUNC_RemoveGogglesModel         0x5DF170 // ##SA## 
#define FUNC_ClearWeapon                0x5E62B0 // ##SA##
#define FUNC_GetHealth                  0x4ABC20
#define FUNC_SetCurrentWeaponFromID     0x4FF8E0
#define FUNC_SetCurrentWeaponFromSlot   0x4FF900
#define FUNC_IsPedInControl             0x501950
#define FUNC_IsPedShootable             0x501930
#define FUNC_SetAttackTimer             0x4FCAB0
#define FUNC_SetDead                    0x4F6430
#define FUNC_SetDie                     0x4F65C0
#define FUNC_SetEvasiveDive             0x4F6A20
#define FUNC_SetEvasiveStep             0x4F7170
#define FUNC_SetFall                    0x4FD9F0
#define FUNC_SetFlee                    0x4FBA90
#define FUNC_SetIdle                    0x4FDFD0
#define FUNC_SetLeader                  0x4F07D0
#define FUNC_SetLookFlag                0x50BB70
#define FUNC_SetLookTimer               0x4FCAF0
#define FUNC_SetMoveState               0x50D110
#define FUNC_SetObjective_ENTITY        0x521F10
#define FUNC_SetObjective               0x5224B0
#define FUNC_SetObjective_VECTOR        0x521840
#define FUNC_SetSeekCar                 0x4F54D0
#define FUNC_SetShootTimer              0x4FCA90
#define FUNC_SetSolicit                 0x4F1400
#define FUNC_SetStoredState             0x50CC40
#define FUNC_RestorePreviousState       0x50C600
#define FUNC_SetWaitState               0x4F28A0
#define FUNC_Teleport                   0x4F5690
#define FUNC_WarpPedIntoCar             0x4EF8B0
#define FUNC_DetachPedFromEntity        0x5E7EC0
#define FUNC_CPed_RemoveBodyPart        0x5f0140

#define VAR_LocalPlayer                 0x94AD28

/*
#ifndef CVehicleSA_DEFINED
#define CVehicleSA void
#endif

#ifdef CPedSA
#undef CPedSA
#endif

#ifdef CPedSAInterface
#undef CPedSAInterface
#endif
*/

//+1328 = Ped state
//+1344 = ped health
class CPedFlags
{
public:
    unsigned int bIsStanding : 1; // is ped standing on something
    unsigned int bWasStanding : 1; // was ped standing on something
    unsigned int bIsLooking : 1; // is ped looking at something or in a direction
    unsigned int bIsRestoringLook : 1; // is ped restoring head postion from a look
    unsigned int bIsAimingGun : 1; // is ped aiming gun
    unsigned int bIsRestoringGun : 1; // is ped moving gun back to default posn
    unsigned int bCanPointGunAtTarget : 1; // can ped point gun at target
    unsigned int bIsTalking : 1; // is ped talking(see Chat())

    unsigned int bInVehicle : 1; // is in a vehicle
    unsigned int bIsInTheAir : 1; // is in the air
    unsigned int bIsLanding : 1; // is landing after being in the air
    unsigned int bHitSomethingLastFrame : 1; // has been in a collision last frame
    unsigned int bIsNearCar : 1; // has been in a collision last frame
    unsigned int bRenderPedInCar : 1; // has been in a collision last frame
    unsigned int bUpdateAnimHeading : 1; // update ped heading due to heading change during anim sequence
    unsigned int bRemoveHead : 1; // waiting on AntiSpazTimer to remove head

    unsigned int bFiringWeapon : 1; // is pulling trigger
    unsigned int bHasACamera : 1; // does ped possess a camera to document accidents
    unsigned int bPedIsBleeding : 1; // Ped loses a lot of blood if true
    unsigned int bStopAndShoot : 1; // Ped cannot reach target to attack with fist, need to use gun
    unsigned int bIsPedDieAnimPlaying : 1; // is ped die animation finished so can dead now
    unsigned int bStayInSamePlace : 1; // when set, ped stays put
    unsigned int bKindaStayInSamePlace : 1; // when set, ped doesn't seek out opponent or cover large distances. Will still shuffle and look for cover
    unsigned int bBeingChasedByPolice : 1; // use nodes for routefind

    unsigned int bNotAllowedToDuck : 1; // Is this ped allowed to duck at all?
    unsigned int bCrouchWhenShooting : 1; // duck behind cars etc
    unsigned int bIsDucking : 1; // duck behind cars etc
    unsigned int bGetUpAnimStarted : 1; // don't want to play getup anim if under something
    unsigned int bDoBloodyFootprints : 1; // unsigned int bIsLeader :1;
    unsigned int bDontDragMeOutCar : 1;
    unsigned int bStillOnValidPoly : 1; // set if the polygon the ped is on is still valid for collision
    unsigned int bAllowMedicsToReviveMe : 1;

    unsigned int bResetWalkAnims : 1;
    unsigned int bOnBoat : 1; // flee but only using nodes
    unsigned int bBusJacked : 1; // flee but only using nodes
    unsigned int bFadeOut : 1; // set if you want ped to fade out
    unsigned int bKnockedUpIntoAir : 1; // has ped been knocked up into the air by a car collision
    unsigned int bHitSteepSlope : 1; // has ped collided/is standing on a steep slope (surface type)
    unsigned int bCullExtraFarAway : 1; // special ped only gets culled if it's extra far away (for roadblocks)
    unsigned int bTryingToReachDryLand : 1; // has ped just exited boat and trying to get to dry land

    unsigned int bCollidedWithMyVehicle : 1;
    unsigned int bRichFromMugging : 1; // ped has lots of cash cause they've been mugging people
    unsigned int bChrisCriminal : 1; // Is a criminal as killed during Chris' police mission (should be counted as such)
    unsigned int bShakeFist : 1; // test shake hand at look entity
    unsigned int bNoCriticalHits : 1; // ped cannot be killed by a single bullet
    unsigned int bHasAlreadyBeenRecorded : 1; // Used for replays
    unsigned int bUpdateMatricesRequired : 1; // if PedIK has altered bones so matrices need updated this frame
    unsigned int bFleeWhenStanding : 1; //

    unsigned int bMiamiViceCop : 1;  //
    unsigned int bMoneyHasBeenGivenByScript : 1; //
    unsigned int bHasBeenPhotographed : 1;  //
    unsigned int bIsDrowning : 1;
    unsigned int bDrownsInWater : 1;
    unsigned int bHeadStuckInCollision : 1;
    unsigned int bDeadPedInFrontOfCar : 1;
    unsigned int bStayInCarOnJack : 1;

    unsigned int bDontFight : 1;
    unsigned int bDoomAim : 1;
    unsigned int bCanBeShotInVehicle : 1;
    unsigned int bPushedAlongByCar : 1; // ped is getting pushed along by car collision (so don't take damage from horz velocity)
    unsigned int bNeverEverTargetThisPed : 1;
    unsigned int bThisPedIsATargetPriority : 1;
    unsigned int bCrouchWhenScared : 1;
    unsigned int bKnockedOffBike : 1;

    unsigned int bDonePositionOutOfCollision : 1;
    unsigned int bDontRender : 1;
    unsigned int bHasBeenAddedToPopulation : 1;
    unsigned int bHasJustLeftCar : 1;
    unsigned int bIsInDisguise : 1;
    unsigned int bDoesntListenToPlayerGroupCommands : 1;
    unsigned int bIsBeingArrested : 1;
    unsigned int bHasJustSoughtCover : 1;

    unsigned int bKilledByStealth : 1;
    unsigned int bDoesntDropWeaponsWhenDead : 1;
    unsigned int bCalledPreRender : 1;
    unsigned int bBloodPuddleCreated : 1; // Has a static puddle of blood been created yet
    unsigned int bPartOfAttackWave : 1;
    unsigned int bClearRadarBlipOnDeath : 1;
    unsigned int bNeverLeavesGroup : 1; // flag that we want to test 3 extra spheres on col model
    unsigned int bTestForBlockedPositions : 1; // this sets these indicator flags for various posisions on the front of the ped

    unsigned int bRightArmBlocked : 1;
    unsigned int bLeftArmBlocked : 1;
    unsigned int bDuckRightArmBlocked : 1;
    unsigned int bMidriffBlockedForJump : 1;
    unsigned int bFallenDown : 1;
    unsigned int bUseAttractorInstantly : 1;
    unsigned int bDontAcceptIKLookAts : 1;
    unsigned int bHasAScriptBrain : 1;

    unsigned int bWaitingForScriptBrainToLoad : 1;
    unsigned int bHasGroupDriveTask : 1;
    unsigned int bCanExitCar : 1;
    unsigned int CantBeKnockedOffBike : 2; // 0=Default(harder for mission peds) 1=Never 2=Always normal(also for mission peds)
    unsigned int bHasBeenRendered : 1;
    unsigned int bIsCached : 1;
    unsigned int bPushOtherPeds : 1; // GETS RESET EVERY FRAME - SET IN TASK: want to push other peds around (eg. leader of a group or ped trying to get in a car)
    unsigned int bHasBulletProofVest : 1;

    unsigned int bUsingMobilePhone : 1;
    unsigned int bUpperBodyDamageAnimsOnly : 1;
    unsigned int bStuckUnderCar : 1;
    unsigned int bKeepTasksAfterCleanUp : 1; // If true ped will carry on with task even after cleanup
    unsigned int bIsDyingStuck : 1;
    unsigned int bIgnoreHeightCheckOnGotoPointTask : 1; // set when walking round buildings, reset when task quits
    unsigned int bForceDieInCar : 1;
    unsigned int bCheckColAboveHead : 1;

    unsigned int bIgnoreWeaponRange : 1;
    unsigned int bDruggedUp : 1;
    unsigned int bWantedByPolice : 1; // if this is set, the cops will always go after this ped when they are doing a KillCriminal task
    unsigned int bSignalAfterKill : 1;
    unsigned int bCanClimbOntoBoat : 1;
    unsigned int bPedHitWallLastFrame: 1; // useful to store this so that AI knows (normal will still be available)
    unsigned int bIgnoreHeightDifferenceFollowingNodes : 1;
    unsigned int bMoveAnimSpeedHasBeenSetByTask : 1;

    unsigned int bGetOutUpsideDownCar : 1;
    unsigned int bJustGotOffTrain : 1;
    unsigned int bDeathPickupsPersist : 1;
    unsigned int bTestForShotInVehicle : 1;
    unsigned int bUsedForReplay : 1; // This ped is controlled by replay and should be removed when replay is done.
};

enum ePedStatus : unsigned int
{
    PED_STATUS_DRIVING_VEHICLE = 50
};

class CPadSAInterface;

//padlevel: 22
class CPedSAInterface : public CPhysicalSAInterface
{
public:
    void*   operator new( size_t );
    void    operator delete( void *ptr );

    bool                            IsEnteringVehicle();
    inline bool                     IsDrivingVehicle( void )            { return pedStatus == PED_STATUS_DRIVING_VEHICLE; }

    bool __thiscall                 IsPlayer( void );
    CPadSAInterface*                GetJoypad();

    void                            OnFrame();

    BYTE                            m_pad[348];                 // 312
    CPedSoundSAInterface            m_sound;                    // 660
    BYTE                            m_pad2[318];                // 814
    CPedFlags                       pedFlags;                   // 1132 (16 bytes long including alignment probably)
    CPedIntelligenceSAInterface*    m_intelligence;             // 1148
    CPlayerPedDataSAInterface*      m_playerData;               // 1152
    BYTE                            m_pad4[80];                 // 1156
    int                             m_moveAnimGroup;            // 1236
    BYTE                            m_pad5[32];                 // 1240
    RwFrame*                        m_unkFrame;                 // 1272
    RwFrame*                        m_goggles;                  // 1276
    BYTE                            m_pad5g[12];                // 1280
    CPedIKSAInterface               m_pedIK;                    // 1292 (length 32 bytes)
    BYTE                            m_pad20[4];                 // 1324
    int                             pedStatus;                  // 1328

    unsigned char                   m_runState;                 // 1332

    BYTE                            m_pad18[11];                // 1333

    float                           m_health;                   // 1344
    int                             m_pad7;                     // 1348
    float                           m_armor;                    // 1352

    int                             m_pad8[3];                  // 1356
    float                           m_pedRotation;              // 1368
    float                           m_targetRotation;           // 1372
    float                           m_rotationSpeed;            // 1376
    BYTE                            m_pad9[4];                  // 1380
    CEntitySAInterface*             m_contactEntity;            // 1384
    BYTE                            m_pad10[24];                // 1388
    CEntitySAInterface*             CurrentObjective;           // 1412
    BYTE                            m_pad21[4];                 // 1416
    CEntitySAInterface*             m_objective;                // 1420
    BYTE                            m_pad11[8];                 // 1424

    ePedType                        bPedType;                   // 1432

    BYTE                            m_pad12[4];                 // 1436
    CWeaponSAInterface              m_weapons[WEAPONSLOT_MAX];  // 1440 

    BYTE                            m_pad13[12];                // 1804
    unsigned char                   m_currentWeapon;            // 1816
    BYTE                            m_pad14[1];                 // 1817
    unsigned char                   m_unkPlayerVal;             // 1818
    BYTE                            m_pad22[18];                // 1819
    unsigned char                   m_fightStyle;               // 1837
    unsigned char                   m_fightStyleExtra;          // 1838
    BYTE                            m_pad15;                    // 1839
    CFireSAInterface*               m_fire;                     // 1840
    BYTE                            m_pad19[28];                // 1844
    unsigned int                    m_footBloodDensity;         // 1872
    BYTE                            m_pad16[72];                // 1876
    CEntitySAInterface*             m_target;                   // 1948
    BYTE                            m_pad17[36];                // 1952
};

// Module initialization.
void Ped_Init( void );
void Ped_Shutdown( void );

class CPedSA : public virtual CPed, public CPhysicalSA
{
    friend class CPoolsSA;
public:
                                CPedSA( CPedSAInterface *ped );
                                ~CPedSA();

    void* operator new ( size_t );
    void operator delete ( void *ptr );

    void                        SetModelIndex( unsigned short id ); // override for voice update

    virtual CPadSAInterface&    GetJoypad();
    virtual bool                IsManagedByGame() const                                     { return false; }

    unsigned int                GetPoolIndex() const                                        { return m_poolIndex; }

    inline CPedSAInterface*     GetInterface()                                              { return (CPedSAInterface*)m_pInterface; }
    inline const CPedSAInterface*   GetInterface() const                                    { return (const CPedSAInterface*)m_pInterface; }

    CPedIntelligenceSA*         GetPedIntelligence()                                        { return m_intelligence; }
    const CPedIntelligenceSA*   GetPedIntelligence() const                                  { return m_intelligence; }
    CPedSoundSA*                GetPedSound()                                               { return m_sound; }
    const CPedSoundSA*          GetPedSound() const                                         { return m_sound; }

    unsigned char               GetType() const                                             { return m_type; }
    float                       GetHealth() const                                           { return GetInterface()->m_health; }
    float                       GetArmor() const                                            { return GetInterface()->m_armor; }
    unsigned char               GetAlpha() const                                            { return m_alpha; }
    CVehicle*                   GetVehicle() const;
    float                       GetCurrentRotation() const                                  { return GetInterface()->m_pedRotation; }
    float                       GetTargetRotation() const                                   { return GetInterface()->m_targetRotation; }
    eWeaponSlot                 GetCurrentWeaponSlot() const                                { return (eWeaponSlot)GetInterface()->m_currentWeapon; }
    CTaskSA*                    GetPrimaryTaskInternal() const;
    CTask*                      GetPrimaryTask() const;
    eFightingStyle              GetFightingStyle() const                                    { return (eFightingStyle)GetInterface()->m_fightStyle; }
    unsigned char               GetRunState() const                                         { return GetInterface()->m_runState; }
    CEntity*                    GetContactEntity() const;
    CEntity*                    GetTargetedEntity() const;
    bool                        GetCanBeShotInVehicle() const                               { return GetInterface()->pedFlags.bCanBeShotInVehicle; }
    bool                        GetTestForShotInVehicle() const                             { return GetInterface()->pedFlags.bTestForShotInVehicle; }
    unsigned int                GetFootBlood() const;
    unsigned char               GetOccupiedSeat() const                                     { return m_ucOccupiedSeat; }
    inline bool                 GetStayInSamePlace() const                                  { return GetInterface()->pedFlags.bStayInSamePlace; }
    unsigned char               GetCantBeKnockedOffBike() const                             { return GetInterface()->pedFlags.CantBeKnockedOffBike; }

    bool                        IsStealthAiming() const                                     { return m_stealthAiming; }
    bool                        IsDucking() const                                           { return GetInterface()->pedFlags.bIsDucking; }
    bool                        IsStanding() const                                          { return GetInterface()->pedFlags.bIsStanding; }
    bool                        IsInWater() const;
    bool                        IsDying() const;
    bool                        IsDead() const;
    bool                        IsOnFire() const                                            { return GetInterface()->m_fire != NULL; }
    bool                        IsWearingGoggles() const                                    { return GetInterface()->m_goggles != NULL; }
    bool                        IsGettingIntoVehicle() const;
    bool                        IsEnteringVehicle() const;
    bool                        IsBeingJacked() const;
    bool                        IsLeavingVehicle() const;
    bool                        IsGettingOutOfVehicle() const;
    bool                        IsPlayingAnimation( const char *name ) const;

    void                        SetType( unsigned char type )                               { m_type = type; }
    void                        SetHealth( float fHealth )                                  { GetInterface()->m_health = fHealth; }
    void                        SetArmor( float fArmor )                                    { GetInterface()->m_armor = fArmor; }
    void                        SetAlpha( unsigned char alpha )                             { m_alpha = alpha; }
    void                        SetCurrentRotation( float rot )                             { GetInterface()->m_pedRotation = rot; }
    void                        SetTargetRotation( float rot )                              { GetInterface()->m_targetRotation = rot; }
    void                        SetStealthAiming( bool enable );
    void                        SetAnimationProgress( const char *name, float progress );
    void                        SetCurrentWeaponSlot( eWeaponSlot weaponSlot );
    void                        SetIsStanding( bool bStanding )                             { GetInterface()->pedFlags.bIsStanding = bStanding; }
    void                        SetDucking( bool bDuck )                                    { GetInterface()->pedFlags.bIsDucking = bDuck; }
    void                        SetFightingStyle( eFightingStyle style, unsigned char extra = 6 );
    void                        SetTargetedEntity( CEntity *entity );
    void                        SetCanBeShotInVehicle( bool bShot )                         { GetInterface()->pedFlags.bCanBeShotInVehicle = bShot; }
    void                        SetTestForShotInVehicle( bool bTest )                       { GetInterface()->pedFlags.bTestForShotInVehicle = bTest; }
    void                        SetFootBlood( unsigned int density );
    void                        SetOnFire( bool bOnFire );
    void                        SetGogglesState( bool bIsWearingThem );
    void                        SetOccupiedSeat( unsigned char seat )                       { m_ucOccupiedSeat = seat; }
    void                        SetStayInSamePlace( bool bStay )                            { GetInterface()->pedFlags.bStayInSamePlace = bStay; }
    void                        SetCantBeKnockedOffBike( unsigned char val )                { GetInterface()->pedFlags.CantBeKnockedOffBike = val; }

    void                        AttachPedToBike( CEntity *entity, const CVector& pos, unsigned short sUnk, float fUnk, float fUnk2, eWeaponType weaponType );
    void                        AttachPedToEntity( CEntitySAInterface *entity, const CVector& offset, unsigned short sDirection, float rotLimit, eWeaponType weaponType, bool camChange );
    void                        DetachPedFromEntity();

    void                        QuitEnteringCar( CVehicle * vehicle, int iSeat, bool bUnknown );
    
    bool                        CanSeeEntity( CEntity *entity, float distance ) const;
    void                        Respawn( const CVector& pos, bool camCut );
    bool                        AddProjectile( eWeaponType eWeapon, const CVector& vecOrigin, float fForce, const CVector& targetPos, CEntity *target );

    CWeaponSA*                  GiveWeapon( eWeaponType weaponType, unsigned int uiAmmo );
    CWeaponSA*                  GetWeapon( eWeaponSlot weaponSlot );
    CWeaponSA*                  GetWeapon( eWeaponType weaponType );
    void                        RemoveWeaponModel( unsigned short model );
    void                        ClearWeapons();
    void                        ClearWeapon( eWeaponType weaponType );

    void                        GetBonePosition( eBone bone, CVector& pos ) const;
    void                        GetTransformedBonePosition( eBone bone, CVector& pos ) const;

    void                        SetClothesTextureAndModel( const char *tex, const char *model, short txd );
    void                        RebuildPlayer();

    void                        RemoveBodyPart( int i, char c );

    void                        GetVoice( short *psVoiceType, short *psVoiceID ) const;
    void                        GetVoice( const char **pszVoiceType, const char **pszVoice ) const;
    void                        SetVoice( short sVoiceType, short sVoiceID );
    void                        SetVoice( const char *szVoiceType, const char *szVoice );

protected:
    virtual bool                InternalAttachTo( CEntitySAInterface *entity, const CVector& pos, const CVector& rot ); // Physical override

    CWeaponSA*                  m_weapons[WEAPONSLOT_MAX];
    CPedIKSA*                   m_pPedIK;
    CPedIntelligenceSA*         m_intelligence;
    CPedSoundSA*                m_sound;

    unsigned int                m_type;
    unsigned char               m_ucOccupiedSeat;
    bool                        m_stealthAiming;

    unsigned int                m_poolIndex;

    unsigned char               m_alpha;

    CPadSAInterface             m_pad;
};

#endif
