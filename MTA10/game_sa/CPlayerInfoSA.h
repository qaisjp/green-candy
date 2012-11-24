/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.0
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        game_sa/CPlayerInfoSA.h
*  PURPOSE:     Header file for player ped type information class
*  DEVELOPERS:  Ed Lyons <eai@opencoding.net>
*               Christian Myhre Lundheim <>
*               Cecill Etheredge <ijsf@gmx.net>
*               Jax <>
*               The_GTA <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#ifndef __CGAMESA_PLAYERINFO
#define __CGAMESA_PLAYERINFO

#include <game/CPlayerInfo.h>
#include "Common.h"
#include "CPlayerPedSA.h"

#define FUNC_MakePlayerSafe                         0x56e870
#define FUNC_CancelPlayerEnteringCars               0x56e860
#define FUNC_ArrestPlayer                           0x56e5d0
#define FUNC_KillPlayer                             0x56e580

#define FUNC_TakeRemoteControlledCarFromPlayer      0x45ae80
#define FUNC_GivePlayerRemoteControlledCar          0x442020
#define FUNC_TakeRemoteControlOfCar                 0x45ad40
#define FUNC_StreamParachuteWeapon                  0x56eb30
#define FUNC_GivePlayerParachute                    0x56ec40

/**
 * \todo Implement FUNC_TakeRemoteControlOfCar, FUNC_StreamParachuteWeapon and FUNC_StreamParachuteWeapon
 */

#define VAR_PlayerPed                               0x94AD28
#define VAR_PlayerRCCar                             0x94AD2C
#define VAR_PlayerMoney                             0x94ADC8

#define NUM_CHARS_IN_NAME                           34

class CPedClothesDesc
{
public:
    unsigned int                            m_modelHash[9];                 // 0
    DWORD                                   m_pad;                          // 36
    unsigned int                            m_textureHash[10];              // 40
};

class CPlayerCrossHairSAInterface
{
public:
    bool    m_active;
    float   m_tarX, m_tarY;   // -1 ... 1 on screen
};


class CPlayerPedDataSAInterface
{
public:
    CWantedSAInterface*     m_Wanted;                           // 0
    CPedClothesDesc*        m_pClothes;                         // 4

    CPedSAInterface*        m_ArrestingOfficer;                 // 8, CCopPed*

    CVector2D               m_vecFightMovement;                 // 12
    float                   m_moveBlendRatio;                   // 20
    float                   m_fSprintEnergy;                    // 24
    //float                 m_fSprintControlCounter;            // Removed arbitatrily to aligned next byte, should be here really
    unsigned char           m_nChosenWeapon;                    // 28
    unsigned char           m_nCarDangerCounter;                // 29
    BYTE                    m_pad[2];                           // 30
    long                    m_nStandStillTimer;                 // 32
    unsigned int            m_nHitAnimDelayTimer;               // 36
    float                   m_fAttackButtonCounter;             // 40
    CVehicleSAInterface*    m_pDangerCar;                       // 44

    unsigned int            m_bStoppedMoving : 1;               // 48
    unsigned int            m_bAdrenaline : 1;
    unsigned int            m_bHaveTargetSelected : 1;          // Needed to work out whether we lost target this frame
    unsigned int            m_bFreeAiming : 1;
    unsigned int            m_bCanBeDamaged : 1;
    unsigned int            m_bAllMeleeAttackPtsBlocked : 1;    // if all of m_pMeleeAttackers[] is blocked by collision, just attack straight ahead
    unsigned int            m_JustBeenSnacking : 1;             // If this bit is true we have just bought something from a vending machine
    unsigned int            m_bRequireHandleBreath : 1;
       
    unsigned int            m_GroupStuffDisabled : 1;           // 47, if this is true the player can't recrout or give his group commands.
    unsigned int            m_GroupAlwaysFollow : 1;            // The group is told to always follow the player (used for girlfriend missions)
    unsigned int            m_GroupNeverFollow : 1;             // The group is told to always follow the player (used for girlfriend missions)
    unsigned int            m_bInVehicleDontAllowWeaponChange : 1; // stop weapon change once driveby weapon has been given
    unsigned int            m_bRenderWeapon : 1;                //  set to false during cutscenes so that knuckledusters are not rendered

    long                    m_PlayerGroup;                      // 52

    unsigned int            m_AdrenalineEndTime;                // 56
    unsigned char           m_nDrunkenness;                     // 60
    bool                    m_bFadeDrunkenness;                 // 61
    unsigned char           m_nDrugLevel;                       // 62
    unsigned char           m_nScriptLimitToGangSize;           // 63
    
    float                   m_fBreath;                          // 64, for holding breath (ie underwater)

    unsigned int            m_meleeAnimRef;                     // 68
    unsigned int            m_meleeAnimExtraRef;                // 72

    float                   m_fFPSMoveHeading;                  // 76
    float                   m_fLookPitch;                       // 80
    float                   m_fSkateBoardSpeed;                 // 84
    float                   m_fSkateBoardLean;                  // 88
    
    RpAtomic*               m_pSpecialAtomic;                   // 92
    float                   m_fGunSpinSpeed;                    // 96
    float                   m_fGunSpinAngle;                    // 100

    unsigned int            m_LastTimeFiring;                   // 104
    unsigned int            m_nTargetBone;                      // 108
    CVector                 m_vecTargetBoneOffset;              // 112
    
    unsigned int            m_busFaresCollected;                // 124
    bool                    m_bPlayerSprintDisabled;            // 128
    bool                    m_bDontAllowWeaponChange;           // 129
    bool                    m_bForceInteriorLighting;           // 130
    unsigned short          m_DPadDownPressedInMilliseconds;    // 131
    unsigned short          m_DPadUpPressedInMilliseconds;      // 133
        
    unsigned char           m_wetness;                          // 135
    unsigned char           m_playersGangActive;                // 136
    unsigned char           m_waterCoverPerc;                   // 137
    float                   m_waterHeight;                      // 138

    unsigned int            m_heatseekAimTime;                  // 142
    CEntitySAInterface*     m_heatseekTarget;                   // 146
    
    long                    m_shotBuilding;                     // 150
    
    unsigned int            m_heatseekLockOnTime    :31;        // 154
    unsigned int            m_heakseekLockOn        :1;
    
    CPedSAInterface*        m_currentHooker;                    // 158
    CPedSAInterface*        m_lastHooker;                       // 162
};

enum ePlayerState
{
    PS_PLAYING,
    PS_DEAD,
    PS_ARRESTED,
    PS_FAILED_MISSION,
    PS_QUIT
};

class CPlayerInfoSAInterface
{
public:
    class CPlayerPedSAInterface*    m_ped;                              // 0, Pointer to the player ped (should always be set)
    CPlayerPedDataSAInterface       m_pedData;                          // 4, instance of player variables
    CVehicleSAInterface*            m_remoteVehicle;                    // 170, Pointer to vehicle player is driving remotely at the moment.(NULL if on foot)
    CVehicleSAInterface*            m_specialColl;                      // 174, which car is using the special collision model
    long                            m_score;                            // 178, Points for this player
    long                            m_displayScore;                     // 182, Points as they would be displayed
    long                            m_tokenCollected;                   // 186, How many bags of sugar do we have
    long                            m_totalTokenCollected;              // 190, How many bags of sugar are there to be had in the game

    unsigned int                    m_lastCarBump;                      // 194, Keeps track of when the last ped bumped into the player car

    unsigned int                    m_taxiTimer;                        // 198, Keeps track of how long the player has been in a taxi with a passenger (in msecs)
    unsigned int                    m_vehicleTimer;                     // 202, keeps track of how long player has been in car for driving skill
    bool                            m_enableTaxiSkill;                  // 206, If TRUE then add 1 to score for each second that the player is driving a taxi 
    bool                            m_vehicleExitRequest;               // 207, if player holds exit car button, want to trigger getout once car slowed enough

    CVehicleSAInterface*            m_enterVehicle;                     // 208, Last vehicle player tried to enter.

    ePlayerState                    m_state;                            // 212

    bool                            m_remoteVehicleExploded;            // 213
    bool                            m_createRemoteVehicleExplosion;     // 214
    bool                            m_remoteVehicleExplosionFadeCamera; // 215
    unsigned int                    m_remoteVehicleExplosionTimer;      // 216

    unsigned int                    m_healthLossTimer;                  // 220, To make numbers flash on the HUD
    unsigned int                    m_armorLossTimer;                   // 224

    unsigned int                    m_vehicleFireTimer;                 // 228, Tank guns etc
    unsigned int                    m_vehicleWreckTimer;                // 232, Make car blow up if car upside down
    unsigned int                    m_vehicleStuckTimer;                // 236, Make car blow up if player cannot get out.
    
    // Stunt variables
    unsigned int                    m_twoWheelTimer;                    // 240, how long has player's car been on two wheels
    float                           m_twoWheelDistance;                 // 244
    unsigned int                    m_wheelieTimer;                     // 248, how long has player's car been on less than 3 wheels
    unsigned int                    m_bikeRearTimer;                    // 252, how long has player's bike been on rear wheel only
    float                           m_bikeRearDistance;                 // 256
    unsigned int                    m_bikeFrontTimer;                   // 260, how long has player's bike been on front wheel only
    float                           m_bikeFrontDistance;                // 264
    unsigned int                    m_groundToleranceTimer;             // 268, so wheels can leave the ground for a few frames without stopping above counters
    // best values for the script to check - will be zero most of the time, only value
    // when finished trick - script should retreve value then reset to zero
    unsigned int                    m_bestTwoWheelTimer;                // 272
    float                           m_bestTwoWheelDistance;             // 276
    unsigned int                    m_bestBikeRearTimer;                // 280
    float                           m_bestBikeRearDistance;             // 284
    unsigned int                    m_bestBikeFrontTimer;               // 288
    float                           m_bestBikeFrontDistance;            // 292

    unsigned short                  m_vehicleDensity;                   // 296
    float                           m_roadDensity;                      // 298

    unsigned int                    m_lastVehicleExplosion;             // 302
    long                            m_explosionMultiplier;              // 306
    unsigned int                    m_numHavocCaused;                   // 310
    unsigned short                  m_starvingTimer;                    // 314

    float                           m_chaseMadness;                     // 316
    
    bool                            m_awardNoTiredness;                 // 320
    bool                            m_awardFastReload;                  // 321
    bool                            m_awardFireProof;                   // 322
    unsigned char                   m_awardMaxHealth;                   // 323
    unsigned char                   m_awardMaxArmor;                    // 324

    bool                            m_freeJail;                         // 325, Player doesn't lose money/weapons next time arrested
    bool                            m_freeHospital;                     // 326, Player doesn't lose money nexed time patched up at hospital

    bool                            m_canDriveBy;                       // 327
    
    unsigned char                   m_bustedAudioStatus;                // 328
    unsigned short                  m_bustedMessageStatus;              // 329

    CPlayerCrossHairSAInterface     m_crossHair;                        // 331

    char                            m_skinName[32];                     // 340
    RwTexture*                      m_skinTexture;                      // 372

    bool                            m_refParachute;                     // 376
    unsigned int                    m_parachuteTimer;                   // 377
};

class CPlayerInfoSA : public CPlayerInfo
{
private:
    CPlayerInfoSAInterface* m_interface;
    CWantedSA*              m_wanted;

public:
    CPlayerInfoSA( CPlayerInfoSAInterface *info ) 
    { 
        m_interface = info;
        m_wanted = NULL;
    }

    ~CPlayerInfoSA()
    {
        if ( m_wanted )
            delete m_wanted;
    }

    CPlayerInfoSAInterface*         GetInterface() { return m_interface; }

    CPlayerPedSA*                   GetPlayerPed();
    CWantedSA*                      GetWanted();
    long                            GetPlayerMoney();
    void                            SetPlayerMoney( long lMoney );

    bool                            GetCrossHair( float& tarX, float& tarY );

    void                            GivePlayerParachute();
    void                            StreamParachuteWeapon( bool bAllowParachute );

    unsigned short                  GetLastTimeEaten();
    void                            SetLastTimeEaten( unsigned short time );

    void                            MakePlayerSafe( bool safe );
    void                            CancelPlayerEnteringCars( CVehicleSA* veh );
    void                            ArrestPlayer();
    void                            KillPlayer();

    // This does not make any sense here, no joke, CRemote?
    CVehicle*                       GiveRemoteVehicle( unsigned short model, float x, float y, float z );
    void                            StopRemoteControl();
    CVehicle*                       GetRemoteVehicle();

    float                           GetFPSMoveHeading();

    bool                            GetDoesNotGetTired();
    void                            SetDoesNotGetTired( bool award );

    unsigned int                    GetLastTimeBigGunFired();
    void                            SetLastTimeBigGunFired( unsigned int time );
    
    inline unsigned int             GetCarTwoWheelCounter()             { return m_interface->m_twoWheelTimer; }
    inline float                    GetCarTwoWheelDist()                { return m_interface->m_twoWheelDistance; }
    inline unsigned int             GetCarLess3WheelCounter()           { return m_interface->m_wheelieTimer; }
    inline unsigned int             GetBikeRearWheelCounter()           { return m_interface->m_bikeRearTimer; }
    inline float                    GetBikeRearWheelDist()              { return m_interface->m_bikeRearDistance; }
    inline unsigned int             GetBikeFrontWheelCounter()          { return m_interface->m_bikeFrontTimer; }
    inline float                    GetBikeFrontWheelDist()             { return m_interface->m_bikeFrontDistance; }
};

#endif
