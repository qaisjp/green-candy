/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.0
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        game_sa/CCameraSA.h
*  PURPOSE:     Header file for camera rendering class
*  DEVELOPERS:  Ed Lyons <eai@opencoding.net>
*               Christian Myhre Lundheim <>
*               Jax <>
*               Cecill Etheredge <ijsf@gmx.net>
*               Sebas Lamers <sebasdevelopment@gmx.com>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#ifndef __CGAMESA_CAMERA
#define __CGAMESA_CAMERA

#include <game/CCamera.h>
#include "CCamSA.h"
#include "CGarageSA.h"
#include "CEntitySA.h"
#include "COffsets.h"

#define FUNC_TakeControl                    0x50C7C0 // ##SA##
#define FUNC_TakeControlNoEntity            0x50C8B0
#define FUNC_TakeControlAttachToEntity      0x50C910
#define FUNC_Restore                        0x50B930
#define FUNC_SetCamPositionForFixedMode     0x50BEC0
#define FUNC_ConeCastCollisionResolve       0x51A5D0

#define FUNC_Find3rdPersonCamTargetVector   0x514970 //##SA##
#define FUNC_Find3rdPersonQuickAimPitch     0x50AD40
#define FUNC_TryToStartNewCamMode           0x467AAD
#define FUNC_VectorTrackLinear              0x50D1D0
#define FUNC_GetFadingDirection             0x50ADF0
#define FUNC_GetFading                      0x50ADE0
#define FUNC_Fade                           0x50AC20
#define FUNC_SetFadeColour                  0x50BF00
    
#define VAR_ActiveCam                       0x7E46FE
#define VAR_CameraRotation                  0xB6F178 // used for controling where the player faces
#define VAR_VehicleCameraView               0xB6F0DC


#define MAX_CAMS                            3 

#define MAX_NUM_OF_NODES                    800 // for trains

/*** R* Defines ***/
#define NUMBER_OF_VECTORS_FOR_AVERAGE 2
#define MAX_NUM_OF_SPLINETYPES (4)
/*** END R* Defines ***/

/*** BEGIN PURE R* CLASSES ***/

class CCamPathSplines
{
    public: 
    enum {MAXPATHLENGTH=800};
    FLOAT *m_arr_PathData;//    FLOAT m_arr_PathData[MAXPATHLENGTH];    
};

class CQueuedMode
{
public:
    short       Mode;                   // CameraMode
    FLOAT       Duration;               // How long for (<0.0f -> indefinately)
    short       MinZoom, MaxZoom;       // How far is player allowed to zoom in & out
};

class CTrainCamNode
{
public:
    CVector m_cvecCamPosition;
    CVector m_cvecPointToLookAt;
    CVector m_cvecMinPointInRange;//this is the minimum required distance the train has to be to the camera to 
    //allow a switch to the node cam 
    CVector m_cvecMaxPointInRange;//this is the minimum required distance the train has to be to from the camera 
    FLOAT m_fDesiredFOV;
    FLOAT m_fNearClip;
    //to switch from the  the node cam
};

/*** END PURE R* CLASSES ***/

class CCameraSAInterface
{
public:
    BYTE    m_padpad[20];
    unsigned int    m_lastRenderCount;

    bool __thiscall     IsSphereVisible( const CVector& pos, float radius, void *unk );

    //move these out the class, have decided to set up a mirrored enumerated type thingy at the top
#if 0
    bool    m_bAboveGroundTrainNodesLoaded;                 // 20
    bool    m_bBelowGroundTrainNodesLoaded;                 // 21
    bool    m_bCamDirectlyBehind;                           // 22
    bool    m_bCamDirectlyInFront;                          // 23
#endif
    bool    m_bCameraJustRestored;                          // 24
    bool    m_bcutsceneFinished;                            // 25
    bool    m_bCullZoneChecksOn;                            // 26
    bool    m_bFirstPersonBeingUsed;                        // 27, To indicate if the m_bFirstPersonBeingUsed viewer is being used.
    bool    m_bJustJumpedOutOf1stPersonBecauseOfTarget;     // 28
    bool    m_bIdleOn;                                      // 29
    bool    m_bInATunnelAndABigVehicle;                     // 30
    bool    m_bInitialNodeFound;                            // 31
    bool    m_bInitialNoNodeStaticsSet;                     // 32
    bool    m_bIgnoreFadingStuffForMusic;                   // 33
    bool    m_bPlayerIsInGarage;                            // 34
    bool    m_bPlayerWasOnBike;                             // 35
    bool    m_bJustCameOutOfGarage;                         // 36
    bool    m_bJustInitalised;                              // 37, Just so the speed thingy doesn't go mad right at the start
    unsigned char   m_bJust_Switched;                       // 38, Variable to indicate that we have jumped somewhere, Raymond needs this for the audio engine
    bool    m_bLookingAtPlayer;                             // 39
    bool    m_bLookingAtVector;                             // 40
    bool    m_bMoveCamToAvoidGeom;                          // 41
    bool    m_bObbeCinematicPedCamOn;                       // 42
    bool    m_bObbeCinematicCarCamOn;                       // 43
    bool    m_bRestoreByJumpCut;                            // 44
    bool    m_bUseNearClipScript;                           // 45
    bool    m_bStartInterScript;                            // 46
    unsigned char   m_bStartingSpline;                      // 47
    bool    m_bTargetJustBeenOnTrain;                       // 48, this variable is needed to be able to restore the camera
    bool    m_bTargetJustCameOffTrain;                      // 49
    bool    m_bUseSpecialFovTrain;                          // 50
    bool    m_bUseTransitionBeta;                           // 51
    bool    m_bUseScriptZoomValuePed;                       // 52
    bool    m_bUseScriptZoomValueCar;                       // 53
    bool    m_bWaitForInterpolToFinish;                     // 54
    bool    m_bItsOkToLookJustAtThePlayer;                  // 55, Used when interpolating
    bool    m_bWantsToSwitchWidescreenOff;                  // 56
    bool    m_WideScreenOn;                                 // 57
    bool    m_1rstPersonRunCloseToAWall;                    // 58
    bool    m_bHeadBob;                                     // 59
    bool    m_bVehicleSuspenHigh;                           // 60
    bool    m_bEnable1rstPersonCamCntrlsScript;             // 61
    bool    m_bAllow1rstPersonWeaponsCamera;                // 62
    bool    m_bCooperativeCamMode;                          // 63
    bool    m_bAllowShootingWith2PlayersInCar;              // 64
    bool    m_bDisableFirstPersonInCar;                     // 65
    static bool m_bUseMouse3rdPerson;
    bool    bStaticFrustum;                                 // 66

    // for debug keyboard stuff
    unsigned char display_kbd_debug;                        // 67
    float kbd_fov_value;                                    // 68

    // The following fields allow the level designers to specify the camera for 2 player games.
    short   m_ModeForTwoPlayersSeparateCars;                // 72
    short   m_ModeForTwoPlayersSameCarShootingAllowed;      // 74
    short   m_ModeForTwoPlayersSameCarShootingNotAllowed;   // 76
    short   m_ModeForTwoPlayersNotBothInCar;                // 78

    bool    m_bGarageFixedCamPositionSet;                   // 80
    bool    m_vecDoingSpecialInterPolation;                 // 81
    bool    m_bScriptParametersSetForInterPol;              // 82

    
    bool    m_bFading;                                      // 83, to indicate that we are fading 
    bool    m_bMusicFading;                                 // 84
    bool    m_bMusicFadedOut;                               // 85

    bool    m_bFailedCullZoneTestPreviously;                // 86
    bool    m_FadeTargetIsSplashScreen;                     // 87, used as hack for fading 
    bool    WorldViewerBeingUsed;                           // 88, To indicate if the world viewer is being used.                                      

    
    unsigned char   m_uiTransitionJUSTStarted;              // 89, This is the first frame of a transition.
    unsigned char   m_uiTransitionState;                    // 90, 0:one mode 1:transition
    unsigned char   ActiveCam;                              // 91, Which one at the moment (0 or 1)
                                                            // Their is a fudge at the end when the renderware matrix will receive either
                                                            // the active camera or the worldviewer camera
    unsigned int    m_uiCamShakeStart;                      // 92, When did the camera shake start.
    unsigned int    m_uiFirstPersonCamLastInputTime;        // 96
    unsigned int    m_uiLongestTimeInMill;                  // 100
    unsigned int    m_uiNumberOfTrainCamNodes;              // 104
    unsigned int    m_uiTimeLastChange;                     // 108
    unsigned int    m_uiTimeWeLeftIdle_StillNoInput;        // 112
    unsigned int  m_uiTimeWeEnteredIdle;                    // 116
    unsigned int    m_uiTimeTransitionStart;                // 120, When was the transition started ?
    unsigned int    m_uiTransitionDuration;                 // 124, How long does the transition take ?
    unsigned int    m_uiTransitionDurationTargetCoors;      // 128
    int     m_BlurBlue;                                     // 132
    int     m_BlurGreen;                                    // 136
    int     m_BlurRed;                                      // 140
    int     m_BlurType;                                     // 144
    int     m_iWorkOutSpeedThisNumFrames;                   // 148, duh  
    int     m_iNumFramesSoFar;                              // 152, counter
    int     m_iCurrentTrainCamNode;                         // 156, variable indicating which camera node we are at for the train
    int     m_motionBlur;                                   // 160, to indicate that we are fading

    int     m_imotionBlurAddAlpha;                          // 164
    int     m_iCheckCullZoneThisNumFrames;                  // 168
    int     m_iZoneCullFrameNumWereAt;                      // 172
    int     WhoIsInControlOfTheCamera;                      // 176, to discern between obbe and scripts

//  float   CarZoomIndicator;
//  float   CarZoomValue;
//  float   CarZoomValueSmooth;
//  float   m_fCarZoomValueScript;
//  float   PedZoomIndicator;
//  float   m_fPedZoomValue;
//  float   m_fPedZoomValueSmooth;
//  float   m_fPedZoomValueScript;
    int     m_nCarZoom;                                     // 180, store zoom index
    float   m_fCarZoomBase;                                 // 184, store base zoom distance from index
    float   m_fCarZoomTotal;                                // 198, store total zoom after modded by camera modes
    float   m_fCarZoomSmoothed;                             // 192, buffered version of the var above
    float   m_fCarZoomValueScript;                          // 196
    int     m_nPedZoom;                                     // 200, store zoom index
    float   m_fPedZoomBase;                                 // 204, store base zoom distance from index
    float   m_fPedZoomTotal;                                // 208, store total zoom after modded by camera modes
    float   m_fPedZoomSmoothed;                             // 212, buffered version of the var above
    float   m_fPedZoomValueScript;                          // 216


    float   CamFrontXNorm, CamFrontYNorm;                   // 220
    float   DistanceToWater;                                // 224
    float   HeightOfNearestWater;                           // 228
    float   FOVDuringInter;                                 // 232
    float   LODDistMultiplier;                              // 236, This takes into account the FOV and the standard LOD multiplier Smaller aperture->bigger LOD multipliers.
    float   GenerationDistMultiplier;                       // 240, This takes into account the FOV but noy the standard LOD multiplier

    float   m_fAlphaSpeedAtStartInter;                      // 244
    float   m_fAlphaWhenInterPol;                           // 248
    float   m_fAlphaDuringInterPol;                         // 252
    float   m_fBetaDuringInterPol;                          // 256
    float   m_fBetaSpeedAtStartInter;                       // 260
    float   m_fBetaWhenInterPol;                            // 264
    float   m_fFOVWhenInterPol;                             // 268
    float   m_fFOVSpeedAtStartInter;                        // 272
    float   m_fStartingBetaForInterPol;                     // 276
    float   m_fStartingAlphaForInterPol;                    // 280
    float   m_PedOrientForBehindOrInFront;                  // 284

    float   m_CameraAverageSpeed;                           // 288, this is an average depending on how many frames we work it out
    float   m_CameraSpeedSoFar;                             // 292, this is a running total
    float   m_fCamShakeForce;                               // 296, How severe is the camera shake.
    float   m_fFovForTrain;                                 // 300
    float   m_fFOV_Wide_Screen;                             // 304

    float   m_fNearClipScript;                              // 308
    float   m_fOldBetaDiff;                                 // 312, Needed for interpolation between 2 modes
    float   m_fPositionAlongSpline;                         // 316, Variable used to indicate how far along the spline we are 0-1 for started to completed respectively
    float   m_ScreenReductionPercentage;                    // 320
    float   m_ScreenReductionSpeed;                         // 324
    float   m_AlphaForPlayerAnim1rstPerson;                 // 328
    
    float   Orientation;                                    // 332, The orientation of the camera. Used for peds walking.
    float   PlayerExhaustion;                               // 336, How tired is player (inaccurate sniping) 0.0f-1.0f

    // The following things are used by the sound code to
    // play reverb depending on the surroundings. From a point
    // in front of the camera the disance is measured to the
    // nearest obstacle (building)
    float   SoundDistUp;                                    // 340, SoundDistLeft, SoundDistRight;     // These ones are buffered and should be used by the audio
    float   SoundDistUpAsRead;                              // 344, SoundDistLeftAsRead, SoundDistRightAsRead;
    float   SoundDistUpAsReadOld;                           // 348, SoundDistLeftAsReadOld, SoundDistRightAsReadOld;
    // Very rough distance to the nearest water for the sound to use
    // Front vector X&Y normalised to 1. Used by loads of stuff.
    
    float   m_fAvoidTheGeometryProbsTimer;                  // 352
    short   m_nAvoidTheGeometryProbsDirn;                   // 356
    
    float   m_fWideScreenReductionAmount;                   // 358, 0 for not reduced 1 for fully reduced (Variable for Les)
    float   m_fStartingFOVForInterPol;                      // 362
    
    // These ones are static so that they don't get cleared in CCamera::Init()
    static  float m_fMouseAccelHorzntl;// acceleration multiplier for 1st person controls
    static  float m_fMouseAccelVertical;// acceleration multiplier for 1st person controls
    static  float m_f3rdPersonCHairMultX;
    static  float m_f3rdPersonCHairMultY;

    CCamSAInterface m_cams[3];                              // 366
    // The actual cameras (usually only one of the two is active)
    // And to complicate this we have a third camera, this camera is 
    // used for debugging when we want to have a look at the world.
    // We can't change the camera mode because other objects depend on their


    // #########################################//
    // DATA NOT UPDATED FOR SA  BELOW HERE!!!!! //
    // #########################################//

    CGarageSAInterface *pToGarageWeAreIn;
    CGarageSAInterface *pToGarageWeAreInForHackAvoidFirstPerson;
    CQueuedMode m_PlayerMode;
    // The higher priority player camera mode. This one is used
    // for the sniper mode and rocket launcher mode.
    // This one overwrites the m_PlayerMode above.
    CQueuedMode PlayerWeaponMode;
    CVector m_PreviousCameraPosition; //needed to work out speed
    CVector m_RealPreviousCameraPosition; // This cane be used by stuff outside the camera code. The one above is the same as the current coordinates outwidth the camera code.
                                        // an active camera for range finding etc
    CVector m_cvecAimingTargetCoors;        // Coors to look at with Gordons aiming thing
    // The player camera that is waiting to be used
    // This camera can replace the default camera where this is
    // needed (in tricky situations like tunnels for instance)
    CVector m_vecFixedModeVector;
    CVector m_vecFixedModeSource;
    CVector m_vecFixedModeUpOffSet;
    CVector m_vecCutSceneOffset;
    CVector m_cvecStartingSourceForInterPol;
    CVector m_cvecStartingTargetForInterPol;
    CVector m_cvecStartingUpForInterPol;
    CVector m_cvecSourceSpeedAtStartInter;
    CVector m_cvecTargetSpeedAtStartInter;
    CVector m_cvecUpSpeedAtStartInter;
    CVector m_vecSourceWhenInterPol;
    CVector m_vecTargetWhenInterPol;
    CVector m_vecUpWhenInterPol;
    CVector m_vecClearGeometryVec;
    CVector m_vecGameCamPos;
    CVector SourceDuringInter, TargetDuringInter, UpDuringInter;


    CVector m_vecAttachedCamOffset; // for attaching the camera to a ped or vehicle (set by level designers for use in cutscenes)
    CVector m_vecAttachedCamLookAt; 
    FLOAT m_fAttachedCamAngle; // for giving the attached camera a tilt.

    // RenderWare camera pointer
    RwCamera* m_pRwCamera;
    ///stuff for cut scenes
    CEntitySAInterface *pTargetEntity;
    CEntitySAInterface *pAttachedEntity;
    //CVector CutScene; 
    CCamPathSplines m_arrPathArray[MAX_NUM_OF_SPLINETYPES]; //These only get created when the script calls the load splines function
    // maybe this shouldn't be here depends if GTA_TRAIN is defined (its not)
    //CTrainCamNode     m_arrTrainCamNode[MAX_NUM_OF_NODES];

    bool m_bMirrorActive;
    bool m_bResetOldMatrix;

//  protected:
    RwMatrix m_cameraMatrix;
    RwMatrix m_cameraMatrixOld;
    RwMatrix m_viewMatrix;
    RwMatrix m_matInverse;
    RwMatrix m_matMirrorInverse;
    RwMatrix m_matMirror;

    CVector m_vecFrustumNormals[4];
    CVector m_vecFrustumWorldNormals[4];
    CVector m_vecFrustumWorldNormals_Mirror[4];

    FLOAT m_fFrustumPlaneOffsets[4];
    FLOAT m_fFrustumPlaneOffsets_Mirror[4];

    CVector m_vecRightFrustumNormal;
    CVector m_vecBottomFrustumNormal;
    CVector m_vecTopFrustumNormal;

    CVector m_vecOldSourceForInter;
    CVector m_vecOldFrontForInter;
    CVector m_vecOldUpForInter;
    FLOAT   m_vecOldFOVForInter;
    FLOAT   m_fFLOATingFade;//variable representing the FLOAT version of CDraw::Fade. Necessary to stop loss of precision
    FLOAT   m_fFLOATingFadeMusic;
    FLOAT   m_fTimeToFadeOut;
    FLOAT   m_fTimeToFadeMusic;
    FLOAT   m_fTimeToWaitToFadeMusic;
    FLOAT   m_fFractionInterToStopMoving; 
    FLOAT   m_fFractionInterToStopCatchUp;
    FLOAT   m_fFractionInterToStopMovingTarget; 
    FLOAT   m_fFractionInterToStopCatchUpTarget;

    FLOAT   m_fGaitSwayBuffer;
    FLOAT   m_fScriptPercentageInterToStopMoving;
    FLOAT   m_fScriptPercentageInterToCatchUp;
    DWORD   m_fScriptTimeForInterPolation;


    short   m_iFadingDirection;
    int     m_iModeObbeCamIsInForCar;
    short   m_iModeToGoTo;
    short   m_iMusicFadingDirection;
    short   m_iTypeOfSwitch;

    DWORD   m_uiFadeTimeStarted;
    DWORD   m_uiFadeTimeStartedMusic;
};


class CCameraSA : public CCamera
{
    friend class COffsets;
private:
    CCameraSAInterface*         m_interface;
    CCamSA*                     m_cams[MAX_CAMS];

public:
                                CCameraSA( CCameraSAInterface *cam );
                                ~CCameraSA();

    CCameraSAInterface*         GetInterface()                                          { return m_interface; }
    const CCameraSAInterface*   GetInterface() const                                    { return m_interface; }

    void                        TakeControl( CEntity *entity, eCamMode mode, int switchStyle );
    void                        TakeControl( const CVector& pos, int switchStyle );
    void                        TakeControlAttachToEntity( CEntity *target, CEntity *attach, const CVector& offset, const CVector& lookAt, float tilt, int switchStyle );

    void                        Restore();
    void                        RestoreWithJumpCut();

    const RwMatrix&             GetMatrix() const;
    void                        SetMatrix( const RwMatrix& mat );

    void                        SetCamPositionForFixedMode( const CVector& pos, const CVector& upOffset );
    void                        Find3rdPersonCamTargetVector( float distance, const CVector& gunMuzzle, const CVector& source, CVector& target ) const;
    float                       Find3rdPersonQuickAimPitch() const;

    unsigned char               GetActiveCam() const;

    CCamSA*                     GetCam( unsigned char id );
    CCamSA*                     GetCam( CCamSAInterface *cam );

    void                        SetWidescreen( bool wide )                              { m_interface->m_WideScreenOn = wide; }
    bool                        GetWidescreen() const                                   { return m_interface->m_WideScreenOn; }

    float                       GetCarZoom() const;
    void                        SetCarZoom( float zoom );

    bool                        TryToStartNewCamMode( unsigned char mode );
    bool                        ConeCastCollisionResolve( const CVector& pos, const CVector& lookAt, const CVector& dest, float rad, float minDist, float& dist );
    void                        VectorTrackLinear( const CVector& to, const CVector& from, float time, bool smoothEnds );

    bool                        IsFading() const;
    int                         GetFadingDirection() const;
    void                        Fade( float fadeOutTime, int outOrIn );
    void                        SetFadeColor( unsigned char red, unsigned char green, unsigned char blue );

    float                       GetCameraRotation() const;
    const RwMatrix&             GetLTM() const;

    CEntity*                    GetTargetEntity() const;

    void                        SetCameraClip( bool ibjects, bool vehicles );
    unsigned char               GetCameraViewMode() const;
    void                        SetCameraViewMode( unsigned char mode );
};

#endif
