/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.0
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        game_sa/CCamSA.h
*  PURPOSE:     Header file for camera entity class
*  DEVELOPERS:  Ed Lyons <eai@opencoding.net>
*               Christian Myhre Lundheim <>
*               Jax <>
*               Cecill Etheredge <ijsf@gmx.net>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#ifndef __CGAMESA_CAM
#define __CGAMESA_CAM

#include <game/CCam.h>
#include "CEntitySA.h"
#include "CAutomobileSA.h"
#include "CPedSA.h"

const int NUMBER_OF_VECTORS_FOR_AVERAGE=2;
#define CAM_NUM_TARGET_HISTORY          4

class CCamSAInterface // 551 bytes
{
public:
    bool    bBelowMinDist;                                          // 0, used for follow ped mode
    bool    bBehindPlayerDesired;                                   // 1, used for follow ped mode
    bool    m_bCamLookingAtVector;                                  // 2
    bool    m_bCollisionChecksOn;                                   // 3
    bool    m_bFixingBeta;                                          // 4, used for camera on a string
    bool    m_bTheHeightFixerVehicleIsATrain;                       // 5
    bool    LookBehindCamWasInFront;                                // 6
    bool    LookingBehind;                                          // 7
    bool    LookingLeft;                                            // 8
    bool    LookingRight;                                           // 9
    bool    ResetStatics;                                           // 10, for interpolation type stuff to work
    bool    Rotating;                                               // 11

    short   Mode;                                                   // 12, CameraMode
    unsigned int  m_uiFinishTime;                                   // 14
    
    int     m_iDoCollisionChecksOnFrameNum;                         // 18
    int     m_iDoCollisionCheckEveryNumOfFrames;                    // 22
    int     m_iFrameNumWereAt;                                      // 26
    int     m_iRunningVectorArrayPos;                               // 30
    int     m_iRunningVectorCounter;                                // 34
    int     DirectionWasLooking;                                    // 38
    
    float   f_max_role_angle;                                       // 42, =DEGTORAD(5.0f);    
    float   f_Roll;                                                 // 46, used for adding a slight roll to the camera in the
    float   f_rollSpeed;                                            // 50, camera on a string mode
    float   m_fSyphonModeTargetZOffSet;                             // 54
    float   m_fAmountFractionObscured;                              // 58
    float   m_fAlphaSpeedOverOneFrame;                              // 62
    float   m_fBetaSpeedOverOneFrame;                               // 66
    float   m_fBufferedTargetBeta;                                  // 70
    float   m_fBufferedTargetOrientation;                           // 74
    float   m_fBufferedTargetOrientationSpeed;                      // 78
    float   m_fCamBufferedHeight;                                   // 82
    float   m_fCamBufferedHeightSpeed;                              // 86
    float   m_fCloseInPedHeightOffset;                              // 90
    float   m_fCloseInPedHeightOffsetSpeed;                         // 94
    float   m_fCloseInCarHeightOffset;                              // 98
    float   m_fCloseInCarHeightOffsetSpeed;                         // 102
    float   m_fDimensionOfHighestNearCar;                           // 106
    float   m_fDistanceBeforeChanges;                               // 110
    float   m_fFovSpeedOverOneFrame;                                // 114
    float   m_fMinDistAwayFromCamWhenInterPolating;                 // 118
    float   m_fPedBetweenCameraHeightOffset;                        // 122
    float   m_fPlayerInFrontSyphonAngleOffSet;                      // 126
    float   m_fRadiusForDead;                                       // 130
    float   m_fRealGroundDist;                                      // 134, used for follow ped mode
    float   m_fTargetBeta;                                          // 138
    float   m_fTimeElapsedFloat;                                    // 142
    float   m_fTilt;                                                // 146
    float   m_fTiltSpeed;                                           // 150

    float   m_fTransitionBeta;                                      // 154
    float   m_fTrueBeta;                                            // 158
    float   m_fTrueAlpha;                                           // 162
    float   m_fInitialPlayerOrientation;                            // 166, used for first person

    float   Alpha;                                                  // 168
    float   AlphaSpeed;                                             // 172
    float   FOV;                                                    // 176
    float   FOVSpeed;                                               // 180
    float   Beta;                                                   // 184
    float   BetaSpeed;                                              // 188
    float   Distance;                                               // 192
    float   DistanceSpeed;                                          // 196
    float   CA_MIN_DISTANCE;                                        // 200
    float   CA_MAX_DISTANCE;                                        // 204
    float   SpeedVar;                                               // 208
    float   m_fCameraHeightMultiplier;                              // 212, used by TwoPlayer_Separate_Cars_TopDown
    
    // ped onfoot zoom distance
    float m_fTargetZoomGroundOne;                                   // 216
    float m_fTargetZoomGroundTwo;                                   // 220
    float m_fTargetZoomGroundThree;                                 // 224
    // ped onfoot alpha angle offset
    float m_fTargetZoomOneZExtra;                                   // 228
    float m_fTargetZoomTwoZExtra;                                   // 232
    float m_fTargetZoomTwoInteriorZExtra;                           // 236, extra one for interior
    float m_fTargetZoomThreeZExtra;                                 // 240
    
    float m_fTargetZoomZCloseIn;                                    // 244
    float m_fMinRealGroundDist;                                     // 248
    float m_fTargetCloseInDist;                                     // 252

    // For targetting in cooperative mode.
    float   Beta_Targeting;                                         // 256
    float   X_Targetting, Y_Targetting;                             // 260
    int CarWeAreFocussingOn;                                        // 264, which car is closer to the camera in 2 player cooperative mode with separate cars.
    float   CarWeAreFocussingOnI;                                   // 268, interpolated version
    
    float m_fCamBumpedHorz;                                         // 272
    float m_fCamBumpedVert;                                         // 276
    int m_nCamBumpedTime;                                           // 280

    // NOT IN CLASS
    static int CAM_BUMPED_SWING_PERIOD;                 
    static int CAM_BUMPED_END_TIME;
    static float CAM_BUMPED_DAMP_RATE;
    static float CAM_BUMPED_MOVE_MULT;

    CVector m_cvecSourceSpeedOverOneFrame;                          // 284
    CVector m_cvecTargetSpeedOverOneFrame;                          // 296
    CVector m_cvecUpOverOneFrame;                                   // 308
    
    CVector m_cvecTargetCoorsForFudgeInter;                         // 320
    CVector m_cvecCamFixedModeVector;                               // 334
    CVector m_cvecCamFixedModeSource;                               // 346
    CVector m_cvecCamFixedModeUpOffSet;                             // 358
    CVector m_vecLastAboveWaterCamPosition;                         // 370, helper for when the player has gone under the water

    CVector m_vecBufferedPlayerBodyOffset;                          // 382

    // The three vectors that determine this camera for this frame
    CVector Front;                                                  // 394, Direction of looking in
    CVector Source;                                                 // 406, Coors in world space
    CVector SourceBeforeLookBehind;                                 // 418
    CVector Up;                                                     // 430, Just that
    CVector m_arrPreviousVectors[NUMBER_OF_VECTORS_FOR_AVERAGE];    // 442, used to average stuff

    CVector m_aTargetHistoryPos[CAM_NUM_TARGET_HISTORY];            // 466
    DWORD m_nTargetHistoryTime[CAM_NUM_TARGET_HISTORY];             // 514
    DWORD m_nCurrentHistoryPoints;                                  // 522

    CEntitySAInterface *CamTargetEntity;                            // 526

    float       m_fCameraDistance;                                  // 530
    float       m_fIdealAlpha;                                      // 534
    float       m_fPlayerVelocity;                                  // 538
    CVehicleSAInterface*    m_pLastCarEntered;                      // 542, So interpolation works
    CPedSAInterface*        m_pLastPedLookedAt;                     // 546, So interpolation works 
    bool        m_bFirstPersonRunAboutActive;                       // 550
};

class CCamSA : public CCam
{
private:
    CCamSAInterface*    m_pInterface;

public:
                        CCamSA( CCamSAInterface* pInterface )       { m_pInterface = pInterface; }

    CCamSAInterface*    GetInterface()                              { return m_pInterface; }

    void                SetVehicleInterpolationSource( CVehicleSA *veh );
    void                SetPedInterpolationSource( CPedSA *ped );

    CVector*            GetFront() const                            { return &m_pInterface->Front; }
    CVector*            GetUp() const                               { return &m_pInterface->Up; }
    CVector*            GetSource() const                           { return &m_pInterface->Source; }
    unsigned int        GetMode() const                             { return m_pInterface->Mode; }
    float               GetFOV() const                              { return m_pInterface->FOV; }
    void                SetFOV( float fFOV )                        { m_pInterface->FOV = fFOV; }

    CVector*            GetFixedModeSource() const                  { return &m_pInterface->m_cvecCamFixedModeSource; }
    CVector*            GetFixedModeVector() const                  { return &m_pInterface->m_cvecCamFixedModeVector; }
    CVector*            GetTargetHistoryPos() const                 { return m_pInterface->m_aTargetHistoryPos; }

    CEntity*            GetTargetEntity() const;
};

#endif
