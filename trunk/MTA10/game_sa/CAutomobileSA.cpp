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

void CAutomobileSAInterface::AddUpgrade( unsigned short model )
{

}

bool CAutomobileSAInterface::UpdateComponentStatus( unsigned short model, unsigned char collFlags, unsigned short *complex )
{
    if ( collFlags & COLL_COMPLEX )
    {
        if ( m_handlingFlags & HANDLING_HYDRAULICS )
            *complex = model;

        m_handlingFlags |= HANDLING_HYDRAULICS;
        m_complexStatus = 0;

#ifdef _ROCKSTAR_OPT
        m_velocity.fZ = 0;
#endif
        return true;
    }

    if ( collFlags & COLL_AUDIO )
    {
        if ( m_audio.m_improved || m_genericFlags & VEHGENERIC_UPGRADEDSTEREO )
        {
            *complex = model;
            return true;
        }

        if ( m_audio.m_soundType == 0 )
            m_audio.m_soundType = 1;
        else if ( m_sound.m_soundType == 2 )
            m_audio.m_soundType = 0;

        // Reset the bass
        pGame->GetAERadioTrackManager()->SetBassSetting( 0 );

        m_genericFlags |= VEHGENERIC_UPGRADEDSTEREO;
        return true;
    }

    if ( !m_unk38 )
    {
        // Nitrous oxide!
        switch( model )
        {
        case 1008:
            break;
        case 1009:
            break;
        case 1010:
            break;
        }
    }

    return false;
}

void CAutomobileSAInterface::UpdateNitrous( unsigned char rounds )
{
    CPadSAInterface *pad;

    if ( !m_status && m_driver->IsPlayer() )
        pad = m_driver->GetJoypad();

    if ( !rounds )
    {

    }
    else
    {
        m_handlingFlags |= HANDLING_NOS;
        m_nitrousFuel = 1;  // Reset the fuel counter
    }

    m_nitroBoosts = rounds;
}