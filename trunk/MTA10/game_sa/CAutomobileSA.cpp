/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        game_sa/CAutomobileSA.cpp
*  PURPOSE:     Automobile vehicle entity
*  DEVELOPERS:  Ed Lyons <eai@opencoding.net>
*               Christian Myhre Lundheim <>
*               The_GTA <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#include "StdInc.h"

extern CBaseModelInfoSAInterface **ppModelInfo;

CAutomobileSAInterface::CAutomobileSAInterface( bool unk, unsigned short model, unsigned char createdBy ) : CVehicleSAInterface( createdBy )
{
    unsigned int n;
    unsigned short handling;

    new (&m_damage) CDamageManagerSAInterface();

    m_damage.m_wheelDamage = 0.5;

    for (n=0; n<3; n++)
        m_unk2[n].m_unk = -1;

    for (n=0; n<6; n++)
        m_unk3[n] = 0;

    handling = ((CVehicleModelInfoSAInterface*)ppModelInfo[model])->m_handlingID;

    m_unk39 = NULL;
    m_unk38 = NULL;

    m_burningTime = 0;

    if ( *(bool*)0x00C1BFD0 )
        m_unk4 |= 0x0001;

    m_unk4 |= 0x0030;

    SetModelIndex( model );

    m_handling = &m_OriginalHandlingData[ handling ];
    m_handlingFlags = m_handling->uiHandlingFlags;

    if ( m_handlingFlags & 0x10000 && (rand() & 3) )
    {

    }
}

CAutomobileSAInterface::~CAutomobileSAInterface()
{
}

void CAutomobileSAInterface::SetModelIndex( unsigned short index )
{
    unsigned int n;

    CVehicleSAInterface::SetModelIndex( index );

    for (n=0; n<NUM_VEHICLE_COMPONENTS; n++)
        m_components[n] = NULL;

    // Crashfix: Made sure models cannot assign atomics above maximum
    GetRwObject()->ScanAtomicHierarchy( &m_components, (unsigned int)NUM_VEHICLE_COMPONENTS );
}
