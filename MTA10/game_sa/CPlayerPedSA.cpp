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

CPlayerPedSA::CPlayerPedSA( ePedModel pedType )
{
    DEBUG_TRACE("CPlayerPedSA::CPlayerPedSA( ePedModel pedType )");

    // based on CPlayerPed::SetupPlayerPed (R*)
    DWORD CPlayerPedConstructor = FUNC_CPlayerPedConstructor;
    CPlayerPedSAInterface *player = new CPlayerPedSAInterface;

    _asm
    {
        mov     ecx, player
        push    0 // set to 0 and they'll behave like AI peds
        push    1
        call    CPlayerPedConstructor
    }

    SetInterface( (CEntitySAInterface*)dwPedPointer );

    Init(); // init our interfaces 
    CPoolsSA * pools = (CPoolsSA *)pGame->GetPools ( );
    internalID =  pools->GetPedRef ( (DWORD *)this->GetInterface () );
    CWorldSA * world = (CWorldSA *)pGame->GetWorld();
    
    SetModelIndex( pedType );
    DoNotRemoveFromGame = FALSE;
    SetType( PLAYER_PED );

    // Allocate a player data struct and set it as the players
    m_bIsLocal = false;
    m_pData = new CPlayerPedDataSAInterface;

    // Copy the local player data so we're defaulted to something good
    CPlayerPedSA* pLocalPlayerSA = dynamic_cast < CPlayerPedSA* > ( pools->GetPedFromRef ( (DWORD)1 ) );
    if ( pLocalPlayerSA )
        memcpy ( m_pData, ((CPlayerPedSAInterface*)pLocalPlayerSA->GetInterface ())->pPlayerData, sizeof ( CPlayerPedDataSAInterface ) );

    // Replace the player ped data in our ped interface with the one we just created
    GetPlayerPedInterface ()->pPlayerData = m_pData;

    // Set default stuff
    m_pData->m_bRenderWeapon = true;
    m_pData->m_Wanted = pLocalWanted;
    m_pData->m_fSprintEnergy = 1000.0f;

    // Clothes pointers or we'll crash later (TODO: Wrap up with some cloth classes and make it unique per player)
    m_pData->m_pClothes = pLocalClothes;

    // Not sure why was this here (svn blame reports that this line came from the old SVN),
    // but it's causing a bug in what the just streamed-in players that are in the air are
    // processed as if they would be standing on some surface, screwing velocity calculations
    // for players floating in air (using superman script, for example) because GTA:SA will
    // try to apply the floor friction to their velocity.
    //SetIsStanding ( true );

    GetPlayerPedInterface ()->pedFlags.bCanBeShotInVehicle = true;
    GetPlayerPedInterface ()->pedFlags.bTestForShotInVehicle = true;
    // Stop remote players targeting eachother, this also stops the local player targeting them (needs to be fixed)
    GetPlayerPedInterface ()->pedFlags.bNeverEverTargetThisPed = true;
    GetPlayerPedInterface ()->pedFlags.bIsLanding = false;
    GetPlayerPedInterface ()->fRotationSpeed = 7.5;
    m_pInterface->bStreamingDontDelete = true;
    m_pInterface->bDontStream = true;
    world->Add ( m_pInterface );
}

CPlayerPedSA::CPlayerPedSA ( CPlayerPedSAInterface * pPlayer )
{
    DEBUG_TRACE("CPlayerPedSA::CPlayerPedSA( CPedSAInterface * ped )");

    // based on CPlayerPed::SetupPlayerPed (R*)
    SetInterface((CEntitySAInterface *)pPlayer);

    Init();
    CPoolsSA * pools = (CPoolsSA *)pGame->GetPools();
    internalID =  pools->GetPedRef ( (DWORD *)this->GetInterface () );
    SetType ( PLAYER_PED );

    m_bIsLocal = true;
    DoNotRemoveFromGame = true;
    m_pData = GetPlayerPedInterface ()->pPlayerData;
    m_pWanted = NULL;

    GetPlayerPedInterface ()->pedFlags.bCanBeShotInVehicle = true;
    GetPlayerPedInterface ()->pedFlags.bTestForShotInVehicle = true;    
    GetPlayerPedInterface ()->pedFlags.bIsLanding = false;
    GetPlayerPedInterface ()->fRotationSpeed = 7.5;


    pLocalClothes = m_pData->m_pClothes;
    pLocalWanted = m_pData->m_Wanted;

    GetPlayerPedInterface ()->pedFlags.bCanBeShotInVehicle = true;
    // Something resets this, constantly
    GetPlayerPedInterface ()->pedFlags.bTestForShotInVehicle = true;
    // Stop remote players targeting the local (need to stop them targeting eachother too)
    GetPlayerPedInterface ()->pedFlags.bNeverEverTargetThisPed = true;
}

CPlayerPedSA::~CPlayerPedSA()
{
    DEBUG_TRACE("CPlayerPedSA::~CPlayerPedSA( )");

    if( !DoNotRemoveFromGame )
    {
        if ( *(DWORD*)m_pInterface != VTBL_CPlaceable )
        {
            CWorldSA * world = (CWorldSA *)pGame->GetWorld();
            world->Remove ( m_pInterface );
            world->RemoveReferencesToDeletedObject ( m_pInterface );
        
            delete m_pInterface;
        }
    }

    // Delete the player data
    if ( !m_bIsLocal )
    {
        delete m_pData;
    }
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
