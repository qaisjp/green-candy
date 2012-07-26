/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        game_sa/CPlayerPedSA.cpp
*  PURPOSE:     Player ped entity
*  DEVELOPERS:  Ed Lyons <eai@opencoding.net>
*               Christian Myhre Lundheim <>
*               Cecill Etheredge <ijsf@gmx.net>
*               Jax <>
*               Alberto Alonso <rydencillo@gmail.com>
*               Stanislav Bobrov <lil_toady@hotmail.com>
*               aru <>
*               The_GTA <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#include "StdInc.h"

bool CPlayerPedSAInterface::ShouldBeStealthAiming()
{
    // Do we have a knife?
    if ( GetCurrentWeaponType() != WEAPONTYPE_KNIFE )
        return false;

    CKeyBindsInterface* pKeyBinds = g_pCore->GetKeyBinds();
    SBindableGTAControl* pAimControl = pKeyBinds->GetBindableFromIndex( CONTROL_AIM_WEAPON );

    if ( !pAimControl->bState )
        return false;

    // We need to be either crouched, walking or standing
    SBindableGTAControl *pWalkControl = pKeyBinds->GetBindableFromIndex( CONTROL_WALK );

    if ( m_pPlayerPed->GetRunState() != 1 && m_pPlayerPed->GetRunState() != 4 || !pWalkControl->bState )
        return false;

    // Do we have a target ped?
    if ( !m_target || m_target->m_type != ENTITY_TYPE_PED )
        return false;

    // Are we close enough to the target?
    CVector pos, pos2;
    GetPosition( pos );
    pTargetPed->GetPosition( pos2 );

    pos -= pos2;

    if ( pos.Length() > STEALTH_KILL_RANGE )
        return false;

    // Grab our current anim
    if ( !pGame->GetAnimManager()->RpAnimBlendClumpGetFirstAssociation( (RpClump*)m_rwObject ) )
        return false;

    // GTA:SA checks for stealth killing
    return m_pedIntelligence->TestForStealthKill( (CPedSAInterface*)m_target, false );
}

void CPlayerPedSAInterface::OnFrame()
{
}

/**
 * Constructor for CPlayerPedSA
 */

static CPedClothesDesc* pLocalClothes = 0;
static CWantedSAInterface* pLocalWanted = 0;

CPlayerPedSA::CPlayerPedSA( CPlayerPedSAInterface *ped, unsigned short modelId, bool isLocal ) : CPedSA( ped )
{
    DEBUG_TRACE("CPlayerPedSA::CPlayerPedSA( CPlayerPedSAInterface *ped, unsigned short modelId, bool isLocal )");
    
    SetType( PLAYER_PED );
    SetModelIndex( modelId );

    m_bIsLocal = isLocal;

    if ( !isLocal )
    {
        // Allocate a player data struct and set it as the players
        m_pData = new CPlayerPedDataSAInterface;

        // Copy the local player data so we're defaulted to something good
        CPlayerPedSA *localPlayer = pGame->GetPlayerInfo()->GetPlayerPed();
        if ( localPlayer != this )
            memcpy( m_pData, localPlayer->GetInterface()->m_playerData, sizeof(CPlayerPedDataSAInterface) );

        // Replace the player ped data in our ped interface with the one we just created
        GetInterface()->m_playerData = m_pData;
    }

    // Set default stuff
    m_pData->m_bRenderWeapon = true;
    m_pData->m_Wanted = pLocalWanted;
    m_pData->m_fSprintEnergy = 1000.0f;

    // Set clothes pointer or we'll crash later (TODO: Wrap up with some cloth classes and make it unique per player)
    m_pData->m_pClothes = pLocalClothes;

    SetCanBeShotInVehicle( true );
    SetTestForShotInVehicle( true );
    // Stop remote players targeting eachother, this also stops the local player targeting them (needs to be fixed)
    GetInterface()->m_pedFlags.bNeverEverTargetThisPed = true;
    GetInterface()->m_pedFlags.bIsLanding = false;
    GetInterface()->m_rotationSpeed = 7.5;

    BOOL_FLAG( GetInterface()->m_entityFlags, ENTITY_DISABLESTREAMING, true );
    BOOL_FLAG( GetInterface()->m_entityFlags, ENTITY_NOSTREAM, true );
}

CPlayerPedSA::~CPlayerPedSA()
{
    DEBUG_TRACE("CPlayerPedSA::~CPlayerPedSA()");

    CWorldSA *world = pGame->GetWorld();
    world->Remove( m_pInterface );
    world->RemoveReferencesToDeletedObject( m_pInterface );

    delete m_pInterface;

    // Delete the player data
    if ( !m_bIsLocal )
        delete m_pData;
}

void CPlayerPedSA::OnFrame()
{
    // Update our stealth status
    SetStealthAiming( GetInterface()->ShouldBeStealthAiming() );

    GetInterface()->OnFrame();
}

CWanted* CPlayerPedSA::GetWanted()
{
    return m_pWanted;
}

float CPlayerPedSA::GetSprintEnergy()
{
    return m_pData->m_fSprintEnergy;
}

void CPlayerPedSA::SetSprintEnergy( float fSprintEnergy )
{
    m_pData->m_fSprintEnergy = fSprintEnergy;
}

void CPlayerPedSA::SetInitialState()
{
    DWORD dwUnknown = 1;
    DWORD dwFunction = FUNC_SetInitialState;
    DWORD dwThis = (DWORD) m_pInterface;
    _asm
    {
        push    dwUnknown
        mov     ecx, dwThis
        call    dwFunction
    }

    // Avoid direction locks for respawning after a jump
    GetPlayerPedInterface ()->pedFlags.bIsLanding = false;
    // Avoid direction locks for respawning after a sprint stop
    GetPlayerPedInterface ()->fRotationSpeed = 7.5;
    // This seems to also be causing some movement / direction locks
    GetPlayerPedInterface()->pedFlags.bStayInSamePlace = false;
}

eMoveAnim CPlayerPedSA::GetMoveAnim()
{
    CPedSAInterface *pedInterface = ( CPedSAInterface * ) this->GetInterface();
    return (eMoveAnim)pedInterface->iMoveAnimGroup;
}

void CPlayerPedSA::SetMoveAnim( eMoveAnim iAnimGroup )
{
    // Set the the new move animation group
    CPedSAInterface *pedInterface = ( CPedSAInterface * ) this->GetInterface();
    pedInterface->iMoveAnimGroup = (int)iAnimGroup;

    DWORD dwThis = ( DWORD ) pedInterface;
    DWORD dwFunc = FUNC_CPlayerPed_ReApplyMoveAnims;
    _asm
    {
        mov     ecx, dwThis
        call    dwFunc
    }
}
