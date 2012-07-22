/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.0
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        game_sa/CPedModelInfoSA.h
*  PURPOSE:     Header file for ped modelinfo class
*  DEVELOPERS:  Cecill Etheredge <ijsf@gmx.net>
*               Stanislav Bobrov <lil_toady@hotmail.com>
*               Jax <>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#ifndef __CPedModelInfoSA_H
#define __CPedModelInfoSA_H

#include <game/CPedModelInfo.h>

class ePedStats;
typedef long AssocGroupID;

#define     FUNC_SetMotionAnimGroup                         0x5b3580
#define     FUNC_CPedModelInfo_Constructor                  0x4c57a0
#define     VAR_CPedModelInfo_VTBL                          0x85bdc0

// CPedModelInfo:
// +36 = Motion anim group (AssocGroupID, long)
// +40 = Default ped type (long)
// +44 = Default ped stats (ePedStats)
// +48 = Can drive cars (byte)
// +50 = Ped flags (short)
// +52 = Hit col model (CColModel*)
// +56 = First radio station
// +57 = Second radio station
// +58 = Race (byte)
// +60 = Audio ped type (short)
// +62 = First voice
// +64 = Last voice
// +66 = Next voice (short)

class CPedModelInfoSAInterface : public CClumpModelInfoSAInterface
{
public:
                                    CPedModelInfoSAInterface();
                                    ~CPedModelInfoSAInterface();

    eModelType                      GetModelType();
    void                            DeleteRwObject();
    void                            SetAnimFile( const char *name );
    void                            ConvertAnimFileIndex();
    int                             GetAnimFileIndex();

    AssocGroupID        m_motionAnimGroup;        // 36   Motion anim group (AssocGroupID, long)
    DWORD               m_pedType;                // 40   Default ped type (long)
    ePedStats *         m_pedStats;               // 44   Default ped stats (ePedStats)
    BYTE                m_bCanDriveCars;          // 48   Can drive cars (byte)
    BYTE                m_pad2 [ 1 ];             // 49   
    short               m_pedFlags;               // 50   Ped flags (short)
    CColModel *         m_pHitColModel;           // 52   Hit col model (CColModel*)
    BYTE                m_bFirstRadioStation;     // 56   First radio station
    BYTE                m_bSecondRadioStation;    // 57   Second radio station
    BYTE                m_bIsInRace;              // 58   Race (byte)
    BYTE                m_pad3 [ 1 ];             // 59   
    short               m_sVoiceType;             // 60   Voice type
    short               m_sFirstVoice;            // 62   First voice
    short               m_sLastVoice;             // 64   Last voice
    short               m_sNextVoice;             // 66   Next voice
};

class CPedModelInfoSA : public CModelInfoSA, public CPedModelInfo
{
public:
                                    CPedModelInfoSA             ( void );
                                    ~CPedModelInfoSA            ( void );

    CPedModelInfoSAInterface *      GetPedModelInfoInterface    ( void )        { return m_pPedModelInterface; }
    void                            SetMotionAnimGroup          ( AssocGroupId animGroup );


private:
    CPedModelInfoSAInterface *      m_pPedModelInterface;
};

#endif