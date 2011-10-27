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
    VEHICLE_UNKNOWN,    // what is this?
    VEHICLE_FAKEPLANE,
    VEHICLE_BIKE,
    VEHICLE_BMX,
    VEHICLE_TRAILER,
    FORCE_DWORD = 0xFFFFFFFF
};

#define MAX_SEATS           15

class CVehicleSeatInfoSA
{
public:
    CVector                         m_offset;
    CQuat                           m_quat;         // 12
    int                             m_id;           // 28
};

class CVehicleSeatPlacementSAInterface
{
public:
    CVehicleSeatPlacementSAInterface();

    void*   operator new( size_t );
    void    operator delete( void *ptr );

    void                            AddAtomic( RpAtomic *atomic );

    CVector                         m_seatOffset[MAX_SEATS];
    CVehicleSeatInfoSA              m_info[18];                 // 180
    RpAtomic*                       m_atomics[6];               // 756
    unsigned short                  m_atomicCount;              // 780
    unsigned short                  m_unknown4;                 // 782
    unsigned int                    m_usageFlags;               // 784
};

class CVehicleModelInfoSAInterface : public CClumpModelInfoSAInterface
{
public:
                                    CVehicleModelInfoSAInterface();
                                    ~CVehicleModelInfoSAInterface();

    eModelType                      GetModelType();
    void                            Init();
    void                            DeleteRwObject();
    RpClump*                        CreateRwObject();
    void                            SetAnimFile( const char *name );
    void                            ConvertAnimFileIndex();
    int                             GetAnimFileIndex();
    void                            SetClump( RpClump *clump );

    void                            RegisterRenderCallbacks();
    void                            Setup();
    void                            SetComponentFlags( RwFrame *frame, unsigned int flags );
    void                            RegisterRoot();
    void                            SetupMateria();
    void                            InitNameplate();

    RpMaterial*                     m_plateMaterial;        // 36
    BYTE                            m_pad2[9];              // 40
    unsigned char                   m_plateDesign;          // 49
    char                            m_name[10];             // 50
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
    CVehicleSeatPlacementSAInterface*   m_seatPlacement;    // 92
    BYTE                            m_pad4[84];             // 96
    BYTE                            m_padUpgrade[540];      // 180
    unsigned int                    m_numberOfColors;       // 720
    BYTE                            m_pad5[2];              // 724
    unsigned short                  m_upgradeTypes[18];     // 726
    unsigned short                  m_paintjobTypes[5];     // 762
    int                             m_animFileIndex;        // 772
};

#endif