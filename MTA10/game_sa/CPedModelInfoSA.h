/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        game_sa/CPedModelInfoSA.h
*  PURPOSE:     Header file for ped modelinfo class
*  DEVELOPERS:  Cecill Etheredge <ijsf@gmx.net>
*               Stanislav Bobrov <lil_toady@hotmail.com>
*               Jax <>
*               Martin Turski <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#ifndef __CPedModelInfoSA_H
#define __CPedModelInfoSA_H

#include <game/CPedModelInfo.h>

class ePedStats;
typedef long AssocGroupID;

#define     FUNC_CPedModelInfo_Constructor                  0x4c57a0
#define     VAR_CPedModelInfo_VTBL                          0x85bdc0

class CPedModelInfoSAInterface : public CClumpModelInfoSAInterface
{
public:
                                    CPedModelInfoSAInterface    ( void );
                                    ~CPedModelInfoSAInterface   ( void );

    eModelType                      GetModelType                ( void );
    void                            DeleteRwObject              ( void );
    void                            SetAnimFile                 ( const char *name );
    void                            ConvertAnimFileIndex        ( void );
    int                             GetAnimFileIndex            ( void );

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