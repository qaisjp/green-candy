/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.0
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        game_sa/CVehicleSA.h
*  PURPOSE:     Header file for vehicle base entity class
*  DEVELOPERS:  Ed Lyons <eai@opencoding.net>
*               Christian Myhre Lundheim <>
*               Cecill Etheredge <ijsf@gmx.net>
*               Jax <>
*               Stanislav Bobrov <lil_toady@hotmail.com>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

class CVehicleSA;

#ifndef __CGAMESA_VEHICLE
#define __CGAMESA_VEHICLE

#include <game/CVehicle.h>

#include "Common.h"
#include "CPedSA.h"
#include "CPhysicalSA.h"
#include "CHandlingManagerSA.h"
#include "CDamageManagerSA.h"
#include "CDoorSA.h"

#define SIZEOF_CHELI                            2584

// 00431f80 public: static class CVehicle * __cdecl CCarCtrl::CreateCarForScript(int,class CVector,unsigned char)
#define FUNC_CCarCtrlCreateCarForScript         0x431f80 // ##SA##

//006d6a40  public: virtual void __thiscall CVehicle::SetModelIndex(unsigned int)
#define FUNC_CVehicleSetModelIndex              0x6d6a40

//006d16a0  public: void __thiscall CVehicle::SetDriver(class CPed *)
#define FUNC_CVehicleSetDriver                  0x6d16a0

//006d14d0  public: bool __thiscall CVehicle::AddPassenger(class CPed *,unsigned char)
#define FUNC_CVehicleAddPassengerWithSeat       0x6d14d0

//006d13a0  public: bool __thiscall CVehicle::AddPassenger(class CPed *)
#define FUNC_CVehicleAddPassenger               0x6d13a0

//006d1610  public: void __thiscall CVehicle::RemovePassenger(class CPed *)
#define FUNC_CVehicleRemovePassenger            0x6d1610

//006E3290  public: void __thiscall CVehicle::AddVehicleUpgrade(DWORD dwModelID)
#define FUNC_CVehicle_AddVehicleUpgrade         0x6E3290

//006DF930     ; public: void __thiscall CVehicle::RemoveVehicleUpgrade(int)
#define FUNC_CVehicle_RemoveVehicleUpgrade      0x6DF930

//006D1E80  public: bool __thiscall CVehicle::CanPedEnterCar(void)
#define FUNC_CVehicle_CanPedEnterCar            0x6D1E80

//006D2030  public: bool __thiscall CVehicle::CanPedJumpOutCar(void)
#define FUNC_CVehicle_CanPedJumpOutCar          0x6D2030

//006D5CF0  public: bool __thiscall CVehicle::CanPedLeanOut(CPed*)
#define FUNC_CVehicle_CanPedLeanOut             0x6D5CF0

//006D1F30  public: bool __thiscall CVehicle::CanPedStepOutCar(void)
#define FUNC_CVehicle_CanPedStepOutCar          0x6D1F30

//006D25D0  public: bool __thiscall CVehicle::CarHasRoof(void)
#define FUNC_CVehicle_CarHasRoof                0x6D25D0

//006D2460  public: void __thiscall CVehicle::ExtinguishCarFire(void)
#define FUNC_CVehicle_ExtinguishCarFire         0x6D2460

//006D2460  public: DWORD __thiscall CVehicle::GetBaseVehicleType(void)
#define FUNC_CVehicle_GetBaseVehicleType        0x411D50

//006D63F0  public: DWORD __thiscall CVehicle::GetHeightAboveRoad(void)
#define FUNC_CVehicle_GetHeightAboveRoad        0x6D63F0

//006DFBE0  public: DWORD __thiscall CVehicle::GetTowBarPos(CVector*, 1, 0)
#define FUNC_CVehicle_GetTowBarPos              0x6DFBE0

//006DFB70  public: void __thiscall CVehicle::GetTowHitchPos(CVector*, 1, 0)
#define FUNC_CVehicle_GetTowHitchPos            0x6DFB70

//006D1DD0  public: bool __thiscall CVehicle::IsOnItsSide(void)
#define FUNC_CVehicle_IsOnItsSide               0x6D1DD0

//006D2370  public: bool __thiscall CVehicle::IsLawEnforcementVehicle(void)
#define FUNC_CVehicle_IsLawEnforcementVehicle   0x6D2370

//006D1BD0  public: bool __thiscall CVehicle::IsPassenger(CPed*)
#define FUNC_CVehicle_IsPassenger               0x6D1BD0

//006D84D0  public: bool __thiscall CVehicle.:IsSphereTouchingVehicle(float, float, float, float)
#define FUNC_CVehicle_IsSphereTouchingVehicle   0x6D84D0

//006D1D90  public: bool __thiscall CVehicle::IsUpsideDown(void)
#define FUNC_CVehicle_IsUpsideDown              0x6D1D90

//006D2BF0  public: void __thiscall CVehicle::MakeDirty(CColPoint*)
#define FUNC_CVehicle_MakeDirty                 0x6D2BF0

//0041BDD0  public: void __thiscall CVehicle::SetEngineOn(bool)
#define FUNC_CVehicle_SetEngineOn               0x41BDD0

//006F6CC0  public: void __thiscall CVehicle::RecalcOnRailDistance(void)
#define FUNC_CVehicle_RecalcOnRailDistance           0x6F6CC0

//006F6BD0  int GetTrainNodeNearPoint(float x, float y, float z, int* pTrackID) places track ID in *pTrackID and returns node ID
#define FUNC_GetTrainNodeNearPoint              0x6F6BD0

#define FUNC_CVehicle_QueryPickedUpEntityWithWinch              0x6d3cf0
#define FUNC_CVehicle_PickUpEntityWithWinch                     0x6d3cd0
#define FUNC_CVehicle_ReleasePickedUpEntityWithWinch            0x6d3cb0
#define FUNC_CVehicle_SetRopeHeightForHeli                      0x6d3d30

#define FUNC_CVehicle__SetRemapTexDictionary                    0x6D0BC0
#define FUNC_CVehicle__GetRemapIndex                            0x6D0B70
#define FUNC_CVehicle__SetRemap                                 0x6D0C00

// from CBike
#define FUNC_Bike_PlaceOnRoadProperly           0x6BEEB0
#define FUNC_Automobile_PlaceOnRoadProperly     0x6AF420
#define FUNC_CBike_Fix                          0x6B7050

// from CPlane
#define FUNC_CPlane__SetGearUp                  0x6CAC20
#define FUNC_CPlane__SetGearDown                0x6CAC70
#define FUNC_CPlane__Fix                        0x6CABB0

// from CAutomobile
#define FUNC_CAutomobile__Fix                   0x6A3440
#define FUNC_CAutomobile__SpawnFlyingComponent  0x6a8580
#define FUNC_CAutomobile__UpdateMovingCollision 0x6a1460

// from CHeli
#define FUNC_CHeli__Fix                         0x6C4530

// from CQuadBike
#define FUNC_CQuadBike__Fix                     0x6CE2B0

#define VARS_CarCounts                          0x969094 // 5 values for each vehicle type

// Used when deleting vehicles
#define VTBL_CPlaceable                         0x863C40

#define MAX_PASSENGERS                          8

#define NUM_RAILTRACKS                          4
#define ARRAY_NumRailTrackNodes                 0xC38014    // NUM_RAILTRACKS dwords
#define ARRAY_RailTrackNodePointers             0xC38024    // NUM_RAILTRACKS pointers to arrays of SRailNode

#define VAR_CVehicle_SpecialColModels           0xc1cc78

#define FUNC_CAutomobile__GetDoorAngleOpenRatio 0x6A2270
#define FUNC_CTrain__GetDoorAngleOpenRatio      0x6F59C0

#define HANDLING_NOS_Flag                       0x00080000
#define	HANDLING_Hydraulics_Flag                0x00020000

typedef struct
{
    short sX;               // x coordinate times 8
    short sY;               // y coordinate times 8
    short sZ;               // z coordinate times 8
    WORD  sRailDistance;    // on-rail distance times 3
    WORD  padding;
} SRailNodeSA;

struct CVehicleFlags
{
    unsigned char bIsLawEnforcer: 1; // Is this guy chasing the player at the moment
    unsigned char bIsAmbulanceOnDuty: 1; // Ambulance trying to get to an accident
    unsigned char bIsFireTruckOnDuty: 1; // Firetruck trying to get to a fire
    unsigned char bIsLocked: 1; // Is this guy locked by the script (cannot be removed)
    unsigned char bEngineOn: 1; // For sound purposes. Parked cars have their engines switched off (so do destroyed cars)
    unsigned char bIsHandbrakeOn: 1; // How's the handbrake doing ?
    unsigned char bLightsOn: 1; // Are the lights switched on ?
    unsigned char bFreebies: 1; // Any freebies left in this vehicle ?

    unsigned char bIsVan: 1; // Is this vehicle a van (doors at back of vehicle)
    unsigned char bIsBus: 1; // Is this vehicle a bus
    unsigned char bIsBig: 1; // Is this vehicle a bus
    unsigned char bLowVehicle: 1; // Need this for sporty type cars to use low getting-in/out anims
    unsigned char bComedyControls: 1; // Will make the car hard to control (hopefully in a funny way)
    unsigned char bWarnedPeds: 1; // Has scan and warn peds of danger been processed?
    unsigned char bCraneMessageDone: 1; // A crane message has been printed for this car allready
    // unsigned char bExtendedRange: 1; // This vehicle needs to be a bit further away to get deleted
    unsigned char bTakeLessDamage: 1; // This vehicle is stronger (takes about 1/4 of damage)

    unsigned char bIsDamaged: 1; // This vehicle has been damaged and is displaying all its components
    unsigned char bHasBeenOwnedByPlayer : 1;// To work out whether stealing it is a crime
    unsigned char bFadeOut: 1; // Fade vehicle out
    unsigned char bIsBeingCarJacked: 1; // Fade vehicle out
    unsigned char bCreateRoadBlockPeds : 1;// If this vehicle gets close enough we will create peds (coppers or gang members) round it
    unsigned char bCanBeDamaged: 1; // Set to FALSE during cut scenes to avoid explosions
    // unsigned char bUsingSpecialColModel : 1;
    // Is player vehicle using special collision model, stored in player strucure
    unsigned char bOccupantsHaveBeenGenerated : 1; // Is true if the occupants have already been generated. (Shouldn't happen again)
    unsigned char bGunSwitchedOff: 1; // Level designers can use this to switch off guns on boats
    
    unsigned char bVehicleColProcessed : 1;// Has ProcessEntityCollision been processed for this car?
    unsigned char bIsCarParkVehicle: 1; // Car has been created using the special CAR_PARK script command
    unsigned char bHasAlreadyBeenRecorded : 1; // Used for replays
    unsigned char bPartOfConvoy: 1;
    unsigned char bHeliMinimumTilt: 1; // This heli should have almost no tilt really
    unsigned char bAudioChangingGear: 1; // sounds like vehicle is changing gear
    unsigned char bIsDrowning: 1; // is vehicle occupants taking damage in water (i.e. vehicle is dead in water)
    unsigned char bTyresDontBurst: 1; // If this is set the tyres are invincible

    unsigned char bCreatedAsPoliceVehicle : 1;// True if this guy was created as a police vehicle (enforcer, policecar, miamivice car etc)
    unsigned char bRestingOnPhysical: 1; // Dont go static cause car is sitting on a physical object that might get removed
    unsigned char bParking : 1;
    unsigned char bCanPark : 1;
    unsigned char bFireGun: 1; // Does the ai of this vehicle want to fire it's gun?
    unsigned char bDriverLastFrame: 1; // Was there a driver present last frame ?
    unsigned char bNeverUseSmallerRemovalRange: 1;// Some vehicles (like planes) we don't want to remove just behind the camera.
    unsigned char bIsRCVehicle: 1; // Is this a remote controlled (small) vehicle. True whether the player or AI controls it.

    unsigned char bAlwaysSkidMarks: 1; // This vehicle leaves skidmarks regardless of the wheels' states.
    unsigned char bEngineBroken: 1; // Engine doesn't work. Player can get in but the vehicle won't drive
    unsigned char bVehicleCanBeTargetted : 1;// The ped driving this vehicle can be targetted, (for Torenos plane mission)
    unsigned char bPartOfAttackWave: 1; // This car is used in an attack during a gang war
    unsigned char bWinchCanPickMeUp: 1; // This car cannot be picked up by any ropes.
    unsigned char bImpounded: 1; // Has this vehicle been in a police impounding garage
    unsigned char bVehicleCanBeTargettedByHS  : 1;// Heat seeking missiles will not target this vehicle.
    unsigned char bSirenOrAlarm: 1; // Set to TRUE if siren or alarm active, else FALSE

    unsigned char bHasGangLeaningOn: 1;
    unsigned char bGangMembersForRoadBlock : 1;// Will generate gang members if NumPedsForRoadBlock > 0
    unsigned char bDoesProvideCover: 1; // If this is false this particular vehicle can not be used to take cover behind.
    unsigned char bMadDriver: 1; // This vehicle is driving like a lunatic
    unsigned char bUpgradedStereo: 1; // This vehicle has an upgraded stereo
    unsigned char bConsideredByPlayer: 1; // This vehicle is considered by the player to enter
    unsigned char bPetrolTankIsWeakPoint : 1;// If false shootong the petrol tank will NOT Blow up the car
    unsigned char bDisableParticles: 1; // Disable particles from this car. Used in garage.

    unsigned char bHasBeenResprayed: 1; // Has been resprayed in a respray garage. Reset after it has been checked.
    unsigned char bUseCarCheats: 1; // If this is true will set the car cheat stuff up in ProcessControl()
    unsigned char bDontSetColourWhenRemapping : 1;// If the texture gets remapped we don't want to change the colour with it.
    unsigned char bUsedForReplay: 1; // This car is controlled by replay and should be removed when replay is done.
};

struct CTrainFlags
{
    unsigned char unknown1 : 3;
    unsigned char bIsTheChainEngine : 1; // Only the first created train on the chain gets this set to true, others get it set to false.
    unsigned char unknown2 : 1; // This is always set to true in mission trains construction.
    unsigned char bIsAtNode : 1;
    unsigned char bDirection : 1;
    unsigned char unknown3 : 1; // If the mission train was placed at the node, this is set to false in construction.

    unsigned char bIsDerailed : 1;
    unsigned char unknown4 : 1 ;
    unsigned char bIsDrivenByBrownSteak : 1;
    unsigned char unknown5 : 5;

    unsigned char unknown6 : 8;

    unsigned char unknown7 : 8;
};

class CVehicleAudioSAInterface
{
public:
                            CVehicleAudioSAInterface();
                            ~CVehicleAudioSAInterface();

    void*                   m_vtbl;         // 0

    void*                   m_unk2;         // 4
    BYTE                    m_pad[12];      // 8

    void*                   m_unk;          // 16

    BYTE                    m_pad2[134];    // 20
    unsigned char           m_station;      // 154

    BYTE                    m_pad3[9];      // 155
    unsigned char           m_unk3;         // 164

    BYTE                    m_pad4[223];    // 165
    void*                   m_unk5;         // 388

    BYTE                    m_pad5[12];     // 392
    void*                   m_unk4;         // 404

    BYTE                    m_pad6[116];    // 408
    unsigned short          m_unk6;         // 524

    BYTE                    m_pad7[22];     // 526
    void*                   m_unk7;         // 548
    void*                   m_unk8;         // 552
};

class CVehicleControlSAInterface
{
public:
                            CVehicleControlSAInterface();
                            ~CVehicleControlSAInterface();

    unsigned short          m_unk;              // 0
    BYTE                    m_pad[2];           // 2
    unsigned short          m_unk2;             // 4
    BYTE                    m_pad2[2];          // 6
    unsigned short          m_unk3;             // 8

    BYTE                    m_pad3[10];         // 10
    unsigned short          m_unk4;             // 20
    unsigned short          m_unk5;             // 22
    unsigned short          m_unk6;             // 24

    BYTE                    m_pad6[11];         // 26
    unsigned char           m_unk8;             // 37
    unsigned char           m_unk9;             // 38

    BYTE                    m_pad[66];          // 26
    unsigned int            m_handlingFlags;    // 92

    BYTE                    m_pad4[8];          // 96

    void*                   m_unk7[8];          // 104
    BYTE                    m_pad5[16];         // 136
};

#define MAX_UPGRADES_ATTACHED 15

/**
 * \todo GAME RELEASE: Update CVehicleSAInterface
 */
class CVehicleSAInterface : public CPhysicalSAInterface
{
public:
                                    CVehicleSAInterface( unsigned char createdBy );
                                    ~CVehicleSAInterface();

    virtual void __thiscall         ProcessControlCollisionCheck() = 0;
    virtual void __thiscall         ProcessControlInputs() = 0;
    virtual void __thiscall         GetComponentWorldPosition() = 0;
    virtual bool __thiscall         IsComponentPresent() = 0;
    virtual void __thiscall         OpenDoor( bool closed, unsigned int node, unsigned int door, float ratio, bool makeNoise ) = 0;
    virtual void __thiscall         ProcessDoorOpen() = 0;
    virtual float __thiscall        GetDoorAngleOpenRatio() = 0;
    virtual float __thiscall        GetDoorAngleOpenRatioInternal() = 0;
    virtual bool __thiscall         IsDoorReady() = 0;
    virtual bool __thiscall         IsDoorReadyInternal() = 0;
    virtual bool __thiscall         IsDoorFullyOpen() = 0;
    virtual bool __thiscall         IsDoorFullyOpenInternal() = 0;
    virtual bool __thiscall         IsDoorClosed() = 0;
    virtual bool __thiscall         IsDoorClosedInternal() = 0;
    virtual bool __thiscall         IsDoorMissing() = 0;
    virtual bool __thiscall         IsDoorMissingInternal() = 0;
    virtual bool __thiscall         IsOpenTopCar() = 0;
    virtual void __thiscall         RemoveVehicleReferences() = 0;
    virtual void __thiscall         Blow( CEntity *cause, unsigned long unk ) = 0;
    virtual void __thiscall         BlowWithCutscene( unsigned long unk, unsigned long unk2, unsigned long unk3, unsigned long unk4 ) = 0;
    virtual bool __thiscall         InitWheels() = 0;
    virtual bool __thiscall         BurstTyre( unsigned char tyre, unsigned int unk ) = 0;
    virtual bool __thiscall         CanPedLeaveCar() = 0;
    virtual void __thiscall         ProcessDrivingAnims() = 0;
    virtual void* __thiscall        GetRideAnimData() = 0;
    virtual void __thiscall         SetupSuspension() = 0;
    virtual void __thiscall         GetMovingCollisionSpeed( CVector *vec ) = 0;
    virtual void __thiscall         Fix() = 0;
    virtual void __thiscall         SetupDamageAfterLoad() = 0;
    virtual void __thiscall         DoBurstAndSoftGroundRatios() = 0;
    virtual float __thiscall        GetModelOffset() = 0;
    virtual void __thiscall         PlayHorn() = 0;
    virtual unsigned int __thiscall GetNumContactWheels() = 0;
    virtual void __thiscall         DamageVehicle() = 0;
    virtual bool __thiscall         CanPedStepOutCar( bool unk ) = 0;
    virtual bool __thiscall         CanPedJumpOutCar( CPedSAInterface *passenger ) = 0;
    virtual bool __thiscall         GetTowHitchPosition( CVector *pos, unsigned int unk, unsigned int unk2 ) = 0;
    virtual bool __thiscall         GetTowbarPosition( CVector *pos, unsigned int unk, unsigned int unk2 ) = 0;
    virtual bool __thiscall         SetTowLink( CVehicleSAInterface *towVehicle, unsigned int unk ) = 0;
    virtual bool __thiscall         BreakTowLink() = 0;
    virtual float __thiscall        GetWheelWidth() = 0;
    virtual void __thiscall         Save() = 0;
    virtual void __thiscall         Load() = 0;

    void*   operator new( size_t );
    void    operator delete( void *ptr );

    CVehicleAudioSAInterface    m_vehicleAudio;                         // 312

    BYTE                        m_pad0[32];                             // 868

    tHandlingDataSA*            m_handling;                             // 900
    BYTE                        m_pad[4];                               // 904
    unsigned int                m_handlingFlags;                        // 908
    
    CVehicleControlSAInterface  m_control;                              // 912

    CVehicleFlags               m_vehicleFlags;                         // 1064
    unsigned int                m_timeOfCreation;                       // 1072, GetTimeInMilliseconds when this vehicle was created.

    unsigned char               m_color1, m_color2, m_color3, m_color4; // 1076
    char                        m_comp1, m_comp2;                       // 1080
    short                       m_upgrades[MAX_UPGRADES_ATTACHED];      // 1082
    float                       m_wheelScale;                           // 1112

    unsigned short              m_carAlarmState;                        // 1116
    unsigned short              m_forcedRandomSeed;                     // 1118, if this is non-zero the random wander gets deterministic
    
    CPedSAInterface*            m_driver;                               // 1120
    CPedSAInterface*            m_passengers[MAX_PASSENGERS];           // 1124
    unsigned char               m_numPassengers;                        // 1156
    unsigned char               m_numGettingIn;                         // 1157
    unsigned char               m_gettingInFlags;                       // 1158
    unsigned char               m_gettingOutFlags;                      // 1159
    unsigned char               m_maxPassengers;                        // 1160
    unsigned char               m_windowsOpenFlags;                     // 1161
    unsigned char               m_nitroBoosts;                          // 1162

    unsigned char               m_specialColModel;                      // 1163
    CEntitySAInterface*         m_entityVisibilityCheck;                // 1164
    CFire*                      m_fire;                                 // 1168

    float                       m_steerAngle;                           // 1172
    float                       m_secondarySteerAngle;                  // 1176, used for steering 2nd set of wheels or elevators etc..
    float                       m_gasPedal;                             // 1180, 0...1
    float                       m_brakePedal;                           // 1184, 0...1

    unsigned char               m_createdBy;                            // 1188, Contains information on whether this vehicle can be deleted 
    unsigned short              m_extendedRemovalRange;                 // 1189

    unsigned char               m_bombOnBoard : 3;                      // 1190, 0 = None. 1 = Timed. 2 = On ignition, 3 = remotely set ? 4 = Timed Bomb has been activated. 5 = On ignition has been activated.
    unsigned char               m_overrideLights  : 2;                  // uses enum NO_CAR_LIGHT_OVERRIDE, FORCE_CAR_LIGHTS_OFF, FORCE_CAR_LIGHTS_ON
    unsigned char               m_winchType: 3;                         // Does this vehicle use a winch?

    unsigned char               m_gunsCycleIndex : 2;                   // 1191, Cycle through alternate gun hardpoints on planes/helis
    unsigned char               m_ordnanceCycleIndex : 6;               // Cycle through alternate ordnance hardpoints on planes/helis

    unsigned char               m_usedForCover;                         // 1192, Has n number of cops hiding/attempting to hid behind it
    unsigned char               m_ammoInClip;                           // 1193, Used to make the guns on boat do a reload.
    unsigned char               m_pacMansCollected;                     // 1194
    unsigned char               m_pedsPositionForRoadBlock;             // 1195
    unsigned char               m_numPedsForRoadBlock;                  // 1196

    BYTE                        m_pad4[3];                              // 1197
    float                       m_bodyDirtLevel;                        // 1200, Dirt level of vehicle body texture: 0.0f=fully clean, 15.0f=maximum dirt visible, it may be altered at any time while vehicle's cycle of lige

    unsigned char               m_currentGear;                          // 1204, values used by transmission
    BYTE                        m_pad4b[3];                             // 1205
    float                       m_gearChangeCount;                      // 1208
    float                       m_wheelSpinForAudio;                    // 1212

    float                       m_health;                               // 1216, 1000 = full health. 0 -> explode

    /*** BEGIN SECTION that was added by us ***/
    BYTE                        m_pad5[48];                             // 1220
    class CVehicleSA*           m_vehicle;                              // 1268
    /*** END SECTION that was added by us ***/

    unsigned int                m_doorState;                            // 1272
    BYTE                        m_pad6[24];                             // 1276
    unsigned int                m_hornActive;                           // 1300
    BYTE                        m_pad7[136];                            // 1304

    unsigned char               m_trackNodeID;                          // 1440, Current node on train tracks
    BYTE                        m_pad8[3];                              // 1441
    float                       m_trainSpeed;                           // 1444, Speed along rails
    float                       m_trainRailDistance;                    // 1448, Distance along rail starting from first rail node
    float                       m_distanceNextCarriage;                 // 1452
    DWORD                       m_pad9[2];                              // 1456
    CTrainFlags                 m_trainFlags;                           // 1464
    DWORD                       m_pad10;                                // 1468
    unsigned char               m_railTrackID;                          // 1472
    BYTE                        m_pad11[15];                            // 1473
    CVehicleSAInterface*        m_prevCarriage;                         // 1488
    CVehicleSAInterface*        m_nextCarriage;                         // 1492

    BYTE                        m_pad12[116];                           // 1496

    RwFrame*                    m_chasis;                               // 1612
    RwFrame*                    m_wheelFR;                              // 1616
    BYTE                        m_pad13[4];                             // 1620
    RwFrame*                    m_wheelRR;                              // 1624
    RwFrame*                    m_wheelFL;                              // 1628 
    BYTE                        m_pad14[4];                             // 1632
    RwFrame*                    m_wheelRL;                              // 1636
    RwFrame*                    m_doors[4];                             // 1640
    RwFrame*                    m_bumpers[2];                           // 1656
    BYTE                        m_pad15[8];                             // 1664
    RwFrame*                    m_bonet;                                // 1672
    RwFrame*                    m_boot;                                 // 1676
    RwFrame*                    m_windscreen;                           // 1680
    RwFrame*                    m_exhaust;                              // 1684

    BYTE                        m_pad16[588];                           // 1688

    float                       m_burningTime;                          // 2276

    BYTE                        m_pad17[304];                           // 2280
};


class CVehicleSA : public virtual CVehicle, public CPhysicalSA
{
    friend class CPoolsSA;
private:
    CDamageManagerSA*           m_pDamageManager;
    CHandlingEntrySA*           m_pHandlingData;
    void*                       m_pSuspensionLines;
    bool                        m_bIsDerailable;
    unsigned char               m_ucAlpha;
    CVector                     m_vecGravity;
    SColor                      m_HeadLightColor;
    RwObject                    m_WheelObjects[4];
    SColor                      m_RGBColors[4];
    CDoorSA                     m_doors[6];
    bool                        m_bSwingingDoorsAllowed;

public:
                                CVehicleSA                      ();
                                CVehicleSA                      ( CVehicleSAInterface *vehicleInterface );
                                CVehicleSA                      ( eVehicleTypes dwModelID );
                                ~CVehicleSA                     ();
    void                        Init                            ();

    // Override of CPhysicalSA::SetMoveSpeed to take trains into account
    void                        SetMoveSpeed                    ( CVector *moveSpeed );

    bool                        AddProjectile                   ( eWeaponType eWeapon, CVector vecOrigin, float fForce, CVector * target, CEntity * targetEntity );

    CVehicleSAInterface*        GetNextCarriageInTrain          ();
    CVehicle*                   GetNextTrainCarriage            ();
    void                        SetNextTrainCarriage            ( CVehicle *next );
    CVehicleSAInterface*        GetPreviousCarriageInTrain      ();
    CVehicle*                   GetPreviousTrainCarriage        ();
    void                        SetPreviousTrainCarriage        ( CVehicle *pPrevious );

    bool                        IsDerailed                      ();
    void                        SetDerailed                     ( bool bDerailed );
    inline bool                 IsDerailable                    () { return m_bIsDerailable; }
    inline void                 SetDerailable                   ( bool bDerailable ) { m_bIsDerailable = bDerailable; }
    float                       GetTrainSpeed                   ();
    void                        SetTrainSpeed                   ( float fSpeed );
    bool                        GetTrainDirection               ();
    void                        SetTrainDirection               ( bool bDirection );
    unsigned char               GetRailTrack                    ();
    void                        SetRailTrack                    ( unsigned char track );

    bool                        CanPedEnterCar                  ();
    bool                        CanPedJumpOutCar                ( CPed* pPed );
    void                        AddVehicleUpgrade               ( DWORD dwModelID );
    void                        RemoveVehicleUpgrade            ( DWORD dwModelID );
    bool                        CanPedLeanOut                   ( CPed* pPed );
    bool                        CanPedStepOutCar                ( bool bUnknown );

    CDoorSA*                    GetDoor                         ( unsigned char ucDoor );
    void                        OpenDoor                        ( unsigned char ucDoor, float fRatio, bool bMakeNoise = false );
    void                        SetSwingingDoorsAllowed         ( bool bAllowed );
    bool                        AreSwingingDoorsAllowed         () const;
    bool                        AreDoorsLocked                  ();
    void                        LockDoors                       ( bool bLocked );
    bool                        AreDoorsUndamageable            ();
    void                        SetDoorsUndamageable            ( bool bUndamageable );

    bool                        CarHasRoof                      ();
    void                        ExtinguishCarFire               ();
    unsigned int                GetBaseVehicleType              ();

    void                        SetBodyDirtLevel                ( float fDirtLevel );
    float                       GetBodyDirtLevel                ();

    unsigned char               GetCurrentGear                  ();
    float                       GetGasPedal                     ();
    //GetIsHandbrakeOn    Use CVehicleSAInterface value
    float                       GetHeightAboveRoad              ();
    float                       GetSteerAngle                   ();
    bool                        GetTowBarPos                    ( CVector* pVector );
    bool                        GetTowHitchPos                  ( CVector* pVector );
    bool                        IsOnItsSide                     ();
    bool                        IsLawEnforcementVehicle         ();
    bool                        IsPassenger                     ( CPed* pPed );
    bool                        IsSphereTouchingVehicle         ( CVector * vecOrigin, float fRadius );
    bool                        IsUpsideDown                    ();
    void                        MakeDirty                       ( CColPoint* pPoint );

    CPed*                       GetDriver                       ();
    CPed*                       GetPassenger                    ( unsigned char ucSlot );
    bool                        IsBeingDriven                   ();
    
    bool                        IsEngineBroken                  () { return GetVehicleInterface ()->m_vehicleFlags.bEngineBroken; };
    void                        SetEngineBroken                 ( bool bEngineBroken ) { GetVehicleInterface ()->m_vehicleFlags.bEngineBroken = bEngineBroken; }
    bool                        IsScriptLocked                  () { return GetVehicleInterface ()->m_vehicleFlags.bIsLocked; }
    void                        SetScriptLocked                 ( bool bLocked ) { GetVehicleInterface ()->m_vehicleFlags.bIsLocked = bLocked; }

    void                        PlaceBikeOnRoadProperly         ();
    void                        PlaceAutomobileOnRoadProperly   ();
    void                        SetColor                        ( SColor color1, SColor color2, SColor color3, SColor color4, int );
    void                        GetColor                        ( SColor* color1, SColor* color2, SColor* color3, SColor* color4, int );
    bool                        IsSirenOrAlarmActive            ();
    void                        SetSirenOrAlarmActive           ( bool bActive );
    inline void                 SetAlpha                        ( unsigned char ucAlpha ) { m_ucAlpha = ucAlpha; }
    inline unsigned char        GetAlpha                        ( ) { return m_ucAlpha; }

    void                        SetLandingGearDown              ( bool bLandingGearDown );
    float                       GetLandingGearPosition          ();
    void                        SetLandingGearPosition          ( float fPosition );
    bool                        IsLandingGearDown               ();
    void                        Fix                             ();

    void                        BlowUp                          ( CEntity* pCreator, unsigned long ulUnknown );
    void                        BlowUpCutSceneNoExtras          ( unsigned long ulUnknown1, unsigned long ulUnknown2, unsigned long ulUnknown3, unsigned long ulUnknown4 );

    CDamageManager*             GetDamageManager                ();

    bool                        SetTowLink                      ( CVehicle* pVehicle );
    bool                        BreakTowLink                    ();
    CVehicle*                   GetTowedVehicle                 ();
    CVehicle*                   GetTowedByVehicle               ();

    // remove these, no longer used
    BYTE                        GetRadioStation                 ();
    void                        SetRadioStation                 ( BYTE bRadioStation );

    void                        FadeOut                         ( bool bFadeOut );
    bool                        IsFadingOut                     ();

    void                        SetWinchType                    ( eWinchType winchType );
    void                        PickupEntityWithWinch           ( CEntity* pEntity );
    void                        ReleasePickedUpEntityWithWinch  ();
    void                        SetRopeHeightForHeli            ( float fRopeHeight );
    CPhysical*                  QueryPickedUpEntityWithWinch    ();

    void                        SetRemap                        ( int iRemap );
    int                         GetRemapIndex                   ();
    void                        SetRemapTexDictionary           ( int iRemapTextureDictionary );

    bool                        IsDamaged                               () { return GetVehicleInterface ()->m_vehicleFlags.bIsDamaged; };
    bool                        IsDrowning                              () { return GetVehicleInterface ()->m_vehicleFlags.bIsDrowning; };
    bool                        IsEngineOn                              () { return GetVehicleInterface ()->m_vehicleFlags.bEngineOn; };
    bool                        IsHandbrakeOn                           () { return GetVehicleInterface ()->m_vehicleFlags.bIsHandbrakeOn; };
    bool                        IsRCVehicle                             () { return GetVehicleInterface ()->m_vehicleFlags.bIsRCVehicle; };
    bool                        GetAlwaysLeaveSkidMarks                 () { return GetVehicleInterface ()->m_vehicleFlags.bAlwaysSkidMarks; };
    bool                        GetCanBeDamaged                         () { return GetVehicleInterface ()->m_vehicleFlags.bCanBeDamaged; };
    bool                        GetCanBeTargettedByHeatSeekingMissiles  () { return GetVehicleInterface ()->m_vehicleFlags.bVehicleCanBeTargettedByHS; };
    bool                        GetCanShootPetrolTank                   () { return GetVehicleInterface ()->m_vehicleFlags.bPetrolTankIsWeakPoint; };
    bool                        GetChangeColourWhenRemapping            () { return GetVehicleInterface ()->m_vehicleFlags.bDontSetColourWhenRemapping; };
    bool                        GetComedyControls                       () { return GetVehicleInterface ()->m_vehicleFlags.bComedyControls; };
    bool                        GetGunSwitchedOff                       () { return GetVehicleInterface ()->m_vehicleFlags.bGunSwitchedOff; };
    bool                        GetLightsOn                             () { return GetVehicleInterface ()->m_vehicleFlags.bLightsOn; };
    unsigned int                GetOverrideLights                       () { return GetVehicleInterface ()->m_overrideLights; }
    bool                        GetTakeLessDamage                       () { return GetVehicleInterface ()->m_vehicleFlags.bTakeLessDamage; };
    bool                        GetTyresDontBurst                       () { return GetVehicleInterface ()->m_vehicleFlags.bTyresDontBurst; };
    unsigned short              GetAdjustablePropertyValue              () { return *reinterpret_cast < unsigned short* > ( reinterpret_cast < unsigned long > ( m_pInterface ) + 2156 ); };
    float                       GetHeliRotorSpeed                       () { return *reinterpret_cast < float* > ( reinterpret_cast < unsigned int > ( m_pInterface ) + 2124 ); };
    unsigned long               GetExplodeTime                          () { return *reinterpret_cast < unsigned long* > ( reinterpret_cast < unsigned int > ( m_pInterface ) + 1240 ); };
    
    void                        SetAlwaysLeaveSkidMarks                 ( bool bAlwaysLeaveSkidMarks )      { GetVehicleInterface ()->m_vehicleFlags.bAlwaysSkidMarks = bAlwaysLeaveSkidMarks; };
    void                        SetCanBeDamaged                         ( bool bCanBeDamaged )              { GetVehicleInterface ()->m_vehicleFlags.bCanBeDamaged = bCanBeDamaged; };
    void                        SetCanBeTargettedByHeatSeekingMissiles  ( bool bEnabled )                   { GetVehicleInterface ()->m_vehicleFlags.bVehicleCanBeTargettedByHS = bEnabled; };
    void                        SetCanShootPetrolTank                   ( bool bCanShoot )                  { GetVehicleInterface ()->m_vehicleFlags.bPetrolTankIsWeakPoint = bCanShoot; };
    void                        SetChangeColourWhenRemapping            ( bool bChangeColour )              { GetVehicleInterface ()->m_vehicleFlags.bDontSetColourWhenRemapping; };
    void                        SetComedyControls                       ( bool bComedyControls )            { GetVehicleInterface ()->m_vehicleFlags.bComedyControls = bComedyControls; };
    void                        SetEngineOn                             ( bool bEngineOn );
    void                        SetGunSwitchedOff                       ( bool bGunsOff )                   { GetVehicleInterface ()->m_vehicleFlags.bGunSwitchedOff = bGunsOff; };
    void                        SetHandbrakeOn                          ( bool bHandbrakeOn )               { GetVehicleInterface ()->m_vehicleFlags.bIsHandbrakeOn = bHandbrakeOn; };
    void                        SetLightsOn                             ( bool bLightsOn )                  { GetVehicleInterface ()->m_vehicleFlags.bLightsOn = bLightsOn; };
    void                        SetOverrideLights                       ( unsigned int uiOverrideLights )   { GetVehicleInterface ()->m_overrideLights = uiOverrideLights; }
    void                        SetTaxiLightOn                          ( bool bLightOn );
    void                        SetTakeLessDamage                       ( bool bTakeLessDamage )            { GetVehicleInterface ()->m_vehicleFlags.bTakeLessDamage = bTakeLessDamage; };
    void                        SetTyresDontBurst                       ( bool bTyresDontBurst )            { GetVehicleInterface ()->m_vehicleFlags.bTyresDontBurst = bTyresDontBurst; };
    void                        SetAdjustablePropertyValue              ( unsigned short usAdjustableProperty ) { *reinterpret_cast < unsigned short* > ( reinterpret_cast < unsigned int > ( m_pInterface ) + 2156 ) = usAdjustableProperty; };
    void                        SetHeliRotorSpeed                       ( float fSpeed )                        { *reinterpret_cast < float* > ( reinterpret_cast < unsigned int > ( m_pInterface ) + 2124 ) = fSpeed; };
    void                        SetExplodeTime                          ( unsigned long ulTime )                { *reinterpret_cast < unsigned long* > ( reinterpret_cast < unsigned int > ( m_pInterface ) + 1240 ) = ulTime; };
    
    float                       GetHealth                       ();
    void                        SetHealth                       ( float fHealth );

    void                        GetTurretRotation               ( float * fHorizontal, float * fVertical );
    void                        SetTurretRotation               ( float fHorizontal, float fVertical );

    unsigned char               GetNumberGettingIn              ();
    unsigned char               GetPassengerCount               ();
    unsigned char               GetMaxPassengerCount            ();

    bool                        IsSmokeTrailEnabled             ();
    void                        SetSmokeTrailEnabled            ( bool bEnabled );

    CHandlingEntry*             GetHandlingData                 ();
    void                        SetHandlingData                 ( CHandlingEntry* pHandling );

    void                        BurstTyre                       ( unsigned char tyre );

    unsigned char               GetBikeWheelStatus              ( unsigned char wheel );
    void                        SetBikeWheelStatus              ( unsigned char wheel, unsigned char status );

    void                        GetGravity                      ( CVector* pvecGravity ) const  { *pvecGravity = m_vecGravity; }
    void                        SetGravity                      ( const CVector* pvecGravity );

    inline SColor               GetHeadLightColor               ()        { return m_HeadLightColor; }
    inline void                 SetHeadLightColor               ( const SColor color )  { m_HeadLightColor = color; }

    CObject*                    SpawnFlyingComponent            ( int i_1, unsigned int ui_2 );
    void                        SetWheelVisibility              ( eWheels wheel, bool bVisible );

    bool                        IsHeliSearchLightVisible        ( void );
    void                        SetHeliSearchLightVisible       ( bool bVisible );

    CColModel*                  GetSpecialColModel              ( void );
    bool                        UpdateMovingCollision           ( float fAngle );

    void                        RecalculateHandling             ( void );

    void*                       GetPrivateSuspensionLines       ( void );

    CVehicleSAInterface*        GetVehicleInterface             ()  { return (CVehicleSAInterface*)m_pInterface; }
    inline CVehicleSAInterface* GetInterface                    ()  { return (CVehicleSAInterface*)m_pInterface; }

private:
    void                        RecalculateSuspensionLines          ( void );
    void                        CopyGlobalSuspensionLinesToPrivate  ( void );
};

#endif
