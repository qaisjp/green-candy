/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.0
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        game_sa/CVehicleModelInfoSA.h
*  PURPOSE:     Vehicle model manipulation and creation
*  DEVELOPERS:  The_GTA <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#ifndef __CVehicleModelInfoSA_H
#define __CVehicleModelInfoSA_H

#include "CModelInfoSA.h"

enum eVehicleType
{
    VEHICLE_CAR,
    VEHICLE_MONSTERTRUCK,
    VEHICLE_QUADBIKE,
    VEHICLE_HELI,
    VEHICLE_PLANE,
    VEHICLE_BOAT,
    VEHICLE_TRAIN,
    VEHICLE_FAKEPLANE = 8,
    VEHICLE_BIKE,
    VEHICLE_BMX,
    VEHICLE_TRAILER,
    FORCE_DWORD = 0xFFFFFFFF
};

// CVehicleModelInfo:
// +36 = Custom plate material (RpMaterial*)
// +49 = Custom plate design (byte)
// +50 = Pointer to game name (const char*)
// +60 = Vehicle type (enum, int)
// +64 = Wheel scale (float). Front/rear?
// +68 = Wheel scale (float). Front/rear?
// +72 = Wheel model id
// +74 = Vehicle handling ID (word)
// +76 = Number of doors (byte)
// +77 = Vehicle list (byte)
// +78 = Vehicle flags (byte)
// +79 = Wheel upgrade class (byte)
// +80 = Number of times used (byte)
// +82 = Vehicle freq (short)
// +84 = Component rules mask (long)
// +88 = Steer angle
// +92 = Pointer to some class containing back seat position @ +60
// +180 = Vehicle upgrade position descriptors array (32 bytes each)
// +720 = Number of possible colors
// +726 = Word array as referenced in CVehicleModelInfo::GetVehicleUpgrade(int)
// +762 = Array of WORD containing something relative to paintjobs
// +772 = Anim file index

class CVehicleModelInfoSAInterface : public CClumpModelInfoSAInterface
{
public:
                                    CVehicleModelInfoSAInterface();
                                    ~CVehicleModelInfoSAInterface();

    eModelType                      GetModelType();
    bool                            SetAnimFile( const char *name );
    void                            ConvertAnimFileIndex();
    int                             GetAnimFileIndex();

    BYTE                            m_pad7[4];              // 32
    RpMaterial*                     m_plateMaterial;        // 36
    BYTE                            m_pad2[9];              // 40
    unsigned char                   m_plateDesign;          // 49
    const char*                     m_name;                 // 50
    BYTE                            m_pad3[6];              // 54
    eVehicleType                    m_vehicleType;          // 60
    float                           m_frontWheelScale;      // 64
    float                           m_rearWheelScale;       // 68
    unsigned short                  m_wheelModel;           // 72
    unsigned short                  m_handlingID;           // 74
    unsigned char                   m_numberOfDoors;        // 76
    unsigned char                   m_vehicleList;          // 77
    unsigned char                   m_flags;                // 78
    unsigned char                   m_wheelUpgrade;         // 79
    unsigned short                  m_usageCounter;         // 80
    unsigned short                  m_frequency;            // 82
    unsigned int                    m_componentFlags;       // 84
    float                           m_steerAngle;           // 88
    DWORD                           m_seatPlacement;        // 92
    BYTE                            m_pad4[84];             // 96
    BYTE                            m_padUpgrade[540];      // 180
    unsigned int                    m_numberOfColors;       // 720
    BYTE                            m_pad5[2];              // 724
    unsigned short                  m_upgradeTypes[18];     // 726
    unsigned short                  m_paintjobTypes[5];     // 762
    int                             m_animFileIndex;        // 772
    BYTE                            m_pad6[12];             // 776
};

#endif