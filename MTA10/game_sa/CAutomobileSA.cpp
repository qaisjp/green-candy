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
}

CAutomobileSAInterface::~CAutomobileSAInterface()
{
}