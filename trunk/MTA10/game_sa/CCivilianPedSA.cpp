/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        game_sa/CCivilianPedSA.cpp
*  PURPOSE:     Civilian ped entity
*  DEVELOPERS:  Ed Lyons <eai@opencoding.net>
*               Christian Myhre Lundheim <>
*               The_GTA <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#include "StdInc.h"

/**
 * Constructor for CCivilianPedSA
 */
CCivilianPedSA::CCivilianPedSA( CCivilianPedSAInterface *ped, unsigned short modelID ) : CPedSA( ped )
{
    DEBUG_TRACE("CCivilianPedSA::CCivilianPedSA( CCivilianPedSAInterface *ped, unsigned short modelID )");

    SetType( CIVILIAN_PED );
    SetModelIndex( modelID );

    BOOL_FLAG( GetInterface()->m_entityFlags, ENTITY_DISABLESTREAMING, true );
    BOOL_FLAG( GetInterface()->m_entityFlags, ENTITY_NOSTREAM, true );
}

CCivilianPedSA::~CCivilianPedSA()
{
    DEBUG_TRACE("CCivilianPedSA::~CCivilianPedSA()");

    CWorldSA *world = pGame->GetWorld();
    world->Remove( m_pInterface );
    world->RemoveReferencesToDeletedObject( m_pInterface );

    delete m_pInterface;
}
