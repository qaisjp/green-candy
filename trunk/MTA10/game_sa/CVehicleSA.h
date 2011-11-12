/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        game_sa/CVehicleSA.h
*  PURPOSE:     Header file for vehicle base entity class
*  DEVELOPERS:  Ed Lyons <eai@opencoding.net>
*               Christian Myhre Lundheim <>
*               Cecill Etheredge <ijsf@gmx.net>
*               Jax <>
*               Stanislav Bobrov <lil_toady@hotmail.com>
*               The_GTA <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#ifndef __CGAMESA_VEHICLE
#define __CGAMESA_VEHICLE

#include <game/CVehicle.h>

#include "Common.h"
#include "CPedSA.h"
#include "CPhysicalSA.h"
#include "CHandlingManagerSA.h"
#include "CDamageManagerSA.h"
#include "CDoorSA.h"

class CVehicleSA;

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

#define VAR_VehicleCount                        0x969094
#define VAR_PoliceVehicleCount                  0x969098

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

    void                    AddUpgrade( unsigned short model );
    bool                    UpdateComponentStatus( unsigned short model, unsigned char collFlags, unsigned short *complex );

    void*                   m_vtbl;         // 0

    void*                   m_unk2;         // 4
    BYTE                    m_pad[12];      // 8

    void*                   m_unk;          // 16

    BYTE                    m_pad2[114];    // 20
    unsigned char           m_soundType;    // 134

    BYTE                    m_pad2b[19];    // 135
    unsigned char           m_station;      // 154
    bool                    m_improved;     // 155

    BYTE                    m_pad3[8];      // 156
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

    short                   m_unk;              // 0
    BYTE                    m_pad[2];           // 2
    short                   m_unk2;             // 4
    BYTE                    m_pad2[2];          // 6
    short                   m_unk3;             // 8

    BYTE                    m_pad3[2];          // 10
    void*                   m_unk10;            // 12
    unsigned int            m_unk11;            // 16

    short                   m_unk4;             // 20
    short                   m_unk5;             // 22
    short                   m_unk6;             // 24

    BYTE                    m_pad6[2];          // 26
    unsigned int            m_creationTime;     // 28
    unsigned int            m_time;             // 32

    unsigned char           m_unk22;            // 36

    bool                    m_unk8;             // 37
    bool                    m_unk9;             // 38

    unsigned char           m_unk39;            // 39
    unsigned char           m_unk12;            // 40
    unsigned char           m_unk13;            // 41
    unsigned char           m_unk14;            // 42
    unsigned char           m_unk15;            // 43

    BYTE                    m_pad13[4];         // 44
    void*                   m_unk29;            // 48
    unsigned char           m_unk30;            // 52

    unsigned char           m_unk28;            // 53
    
    BYTE                    m_pad8[2];          // 54
    float                   m_unk16;            // 56

    void*                   m_unk38;            // 60

    unsigned char           m_unk17;            // 64

    unsigned char           m_unk23;            // 65
    BYTE                    m_pad11[2];         // 66

    float                   m_unk24;            // 68

    unsigned char           m_unk31;            // 72
    
    unsigned char           m_unk32;            // 73
    unsigned char           m_unk33;            // 74

    unsigned char           m_flags;            // 75
    unsigned char           m_flags2;           // 76

    unsigned char           m_unk34;            // 77
    
    unsigned char           m_unk35;            // 78
    unsigned char           m_unk36;            // 79
    unsigned char           m_unk37;            // 80

    BYTE                    m_pad[12];          // 80
    unsigned int            m_handlingFlags;    // 92

    BYTE                    m_pad4[8];          // 96
    unsigned int            m_unk7[8];          // 104

    unsigned short          m_unk18;            // 136
    BYTE                    m_pad10[2];         // 138
    void*                   m_unk19;            // 140

    void*                   m_unk25;            // 144
    
    unsigned char           m_unk26;            // 148
    unsigned char           m_unk27;            // 149
};

#define MAX_UPGRADES_ATTACHED       14

#define VEHICLE_POLICE              0x00000001
#define VEHICLE_AMBULANCE           0x00000002
#define VEHICLE_FIRETRUCK           0x00000004
#define VEHICLE_LOCKED              0x00000008
#define VEHICLE_ENGINESTATUS        0x00000010
#define VEHICLE_HANDBRAKE           0x00000020
#define VEHICLE_LIGHTSTATUS         0x00000040
#define VEHICLE_FREEBIES            0x00000080

#define VEHICLE_VAN                 0x00000100
#define VEHICLE_BUS                 0x00000200
#define VEHICLE_BIG                 0x00000400
#define VEHICLE_LOW                 0x00000800
#define VEHICLE_COMEDYCONTROLS      0x00001000
#define VEHICLE_WARNPEDS            0x00002000
#define VEHICLE_CRANEPROCESSED      0x00004000
#define VEHICLE_ARMORED             0x00008000

#define VEHICLE_DAMAGED             0x00010000
#define VEHICLE_PLAYERPOSSESION     0x00020000
#define VEHICLE_FADEOUT             0x00040000
#define VEHICLE_CARJACK             0x00080000
#define VEHICLE_ROADBLOCK           0x00100000
#define VEHICLE_DAMAGEABLE          0x00200000
#define VEHICLE_PROCESSEDPEDS       0x00400000
#define VEHICLE_WEAPONS             0x00800000
    
#define VEHICLE_COLLPROCESSED       0x01000000
#define VEHICLE_PARKED              0x02000000
#define VEHICLE_RECORDED            0x04000000
#define VEHICLE_CONVOY              0x08000000
#define VEHICLE_LOWTILT             0x10000000
#define VEHICLE_CHANGEGEAR          0x20000000
#define VEHICLE_DROWNING            0x40000000
#define VEHICLE_NOTYREBURST         0x80000000

#define VEHGENERIC_POLICETYPE       0x00000001
#define VEHGENERIC_NOSTATIC         0x00000002
#define VEHGENERIC_PARKING          0x00000004
#define VEHGENERIC_CANPARK          0x00000008
#define VEHGENERIC_FIREGUN          0x00000010
#define VEHGENERIC_DRIVER           0x00000020
#define VEHGENERIC_COMPLEX          0x00000040
#define VEHGENERIC_REMOTECONTROL    0x00000080

#define VEHGENERIC_SKIDMARKS        0x00000100
#define VEHGENERIC_ENGINEBROKEN     0x00000200
#define VEHGENERIC_TARGETABLE       0x00000400
#define VEHGENERIC_GANGWAR          0x00000800
#define VEHGENERIC_PICKUP           0x00001000
#define VEHGENERIC_IMPOUNDED        0x00002000
#define VEHGENERIC_HEATSEEK         0x00004000
#define VEHGENERIC_ALARM            0x00008000

#define VEHGENERIC_GANGLEANON       0x00010000
#define VEHGENERIC_GANGROADBLOCK    0x00020000
#define VEHGENERIC_ISGOODCOVER      0x00040000
#define VEHGENERIC_MADDRIVER        0x00080000
#define VEHGENERIC_UPGRADEDSTEREO   0x00100000
#define VEHGENERIC_PEDENTERING      0x00200000
#define VEHGENERIC_PETROLTANK       0x00400000
#define VEHGENERIC_NOPARTICLES      0x00800000

#define VEHGENERIC_RESPRAYED        0x01000000
#define VEHGENERIC_CHEATS           0x02000000
#define VEHGENERIC_NOCOLORREMAP     0x04000000
#define VEHGENERIC_REPLAY           0x08000000

class CVehicleSAInterface : public CPhysicalSAInterface
{
public:
                                    CVehicleSAInterface( unsigned char createdBy );
                                    ~CVehicleSAInterface();

    void                            HandlePopulation( unsigned char createdBy );

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
    virtual void __thiscall         Damage() = 0;
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

    RpClump*                        GetRwObject() { return (RpClump*)m_rwObject; }

    CVehicleAudioSAInterface    m_audio;                                // 312

    BYTE                        m_pad0[32];                             // 868

    tHandlingDataSA*            m_handling;                             // 900
    BYTE                        m_pad[4];                               // 904
    unsigned int                m_handlingFlags;                        // 908
    
    CVehicleControlSAInterface  m_control;                              // 912

    unsigned int                m_vehicleFlags;                         // 1064
    unsigned int                m_genericFlags;                         // 1068

    unsigned int                m_timeOfCreation;                       // 1072, GetTimeInMilliseconds when this vehicle was created.

    unsigned char               m_color1, m_color2, m_color3, m_color4; // 1076
    char                        m_comp1, m_comp2;                       // 1080
    short                       m_upgrades[MAX_UPGRADES_ATTACHED];      // 1082
    unsigned short              m_numUpgrades;                          // 1110, No idea whether this is true
    float                       m_wheelScale;                           // 1112

    unsigned short              m_alarmState;                           // 1116
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

    char                        m_specialColModel;                      // 1163
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
    float                       m_wheelSpinAudio;                       // 1212

    float                       m_health;                               // 1216, 1000 = full health. 0 -> explode

    void*                       m_unk11;                                // 1220
    void*                       m_unk12;                                // 1224
    void*                       m_unk13;                                // 1228
    void*                       m_unk14;                                // 1232

    void*                       m_unk7;                                 // 1236

    void*                       m_unk5;                                 // 1240
    unsigned short              m_unk6;                                 // 1244

    unsigned short              m_unk3;                                 // 1246
    void*                       m_unk4;                                 // 1248

    void*                       m_unk21;                                // 1252
    void*                       m_unk20;                                // 1256

    unsigned char               m_unk10;                                // 1260

    BYTE                        m_pad5c[7];                             // 1261

    /*** BEGIN SECTION that was added by us ***/
    class CVehicleSA*           m_vehicle;                              // 1268
    /*** END SECTION that was added by us ***/

    unsigned int                m_doorState;                            // 1272

    unsigned int                m_unk15;                                // 1276
    unsigned int                m_unk16;                                // 1280
    unsigned int                m_unk17;                                // 1284

    char                        m_unk2;                                 // 1288
    BYTE                        m_pad6[3];                              // 1289

    unsigned int                m_unk18;                                // 1292

    unsigned char               m_unk19;                                // 1296
    unsigned char               m_unk22;                                // 1297
    unsigned char               m_unk23;                                // 1298
    unsigned char               m_unk24;                                // 1299

    unsigned int                m_hornActive;                           // 1300
    unsigned char               m_unk8;                                 // 1304
    unsigned char               m_unk9;                                 // 1305

    unsigned char               m_unk25;                                // 1306

    unsigned char               m_unk36;                                // 1307

    BYTE                        m_pad19[36];                            // 1308
    unsigned char               m_unk26;                                // 1344

    BYTE                        m_pad20[43];                            // 1345
    unsigned char               m_unk27;                                // 1388
    
    BYTE                        m_pad18[7];                             // 1389
    float                       m_unk37;                                // 1396    

    float                       m_unk31;                                // 1400
    float                       m_unk32;                                // 1404
    float                       m_unk33;                                // 1408

    unsigned int                m_unk34;                                // 1412

    float                       m_unk35;                                // 1416

    float                       m_unk;                                  // 1420

    void*                       m_unk38;                                // 1424, both used in inheriting classes
    void*                       m_unk39;                                // 1428

    short                       m_unk28;                                // 1432
    short                       m_unk29;                                // 1434
    unsigned int                m_unk30;                                // 1436
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
