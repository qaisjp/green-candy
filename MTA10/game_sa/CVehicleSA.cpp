/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.0
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        game_sa/CVehicleSA.cpp
*  PURPOSE:     Vehicle base entity
*  DEVELOPERS:  Ed Lyons <eai@opencoding.net>
*               Christian Myhre Lundheim <>
*               Cecill Etheredge <ijsf@gmx.net>
*               Jax <>
*               Stanislav Bobrov <lil_toady@hotmail.com>
*               The_GTA <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#include "StdInc.h"

extern CGameSA* pGame;

CVehicleControlSAInterface::CVehicleControlSAInterface()
{
    unsigned int n;

    m_unk = -1;
    m_unk2 = -1;
    m_unk3 = -1;

    m_unk4 = -1;
    m_unk5 = -1;
    m_unk6 = -1;

    for (n=0; n<8; n++)
        m_unk7[n] = 0xFFFF;

    m_unk9 = true;
    m_unk8 = true;

    m_flags &= ~0x04 & 10;

    m_unk10 = NULL;
    m_unk11 = 1000;

    m_unk12 = 0;
    m_unk39 = 0;
    m_unk13 = 0;
    m_unk14 = 0;
    m_unk15 = 0;

    m_unk17 = 10;

    m_unk16 = 10;

    m_unk18 = 0;
    m_unk19 = NULL;

    m_flags2 &= ~0x04 & 10;

    m_creationTime = m_time = pGame->GetSystemTime();

    m_unk23 = 0;
    m_unk24 = 1;

    m_unk26 = 0xFF;

    m_unk28 = 0;

    m_unk34 = 20;

    m_unk29 = NULL;
    m_unk30 = 0;

    m_unk32 = 0;
    m_unk33 = 0;

    m_unk35 = 10;
    m_unk36 = 10;

    m_unk27 = 0;

    m_unk37 = rand() & 7 + 2;

    m_unk25 = NULL;
    m_unk38 = NULL;
}

CVehicleAudioSAInterface::CVehicleAudioSAInterface()
{
    m_unk = NULL;

    m_vtbl = (void*)0x00862CEC;

    m_unk4 = NULL;

    m_unk5 = (void*)0x0085F438;

    m_unk6 = 0;
    m_unk7 = 0;
    m_unk8 = 0;

    m_unk3 = 0;

    m_unk2 = NULL;
}

CVehicleAudioSAInterface::~CVehicleAudioSAInterface()
{
}

CVehicleSAInterface::CVehicleSAInterface( unsigned char createdBy )
{
    unsigned int n;

    new (&m_audio) CVehicleAudioSAInterface();
    new (&m_control) CVehicleControlSAInterface();

    m_type = ENTITY_TYPE_VEHICLE;

    m_entityFlags |= ENTITY_PRERENDERED;

    m_unk = 0;
    m_steerAngle = 0;
    m_secondarySteerAngle = 0;

    // We always start off at gear 1
    m_currentGear = 1;

    m_gearChangeCount = 0;
    m_wheelSpinAudio = 0;

    // Set the creation flags
    m_createdBy = createdBy;

    m_forcedRandomSeed = 0;

    m_vehicleFlags = VEHICLE_ENGINESTATUS | VEHICLE_FREEBIES | VEHICLE_PLAYERPOSSESION;

    // Make the vehicle locked by random
    if ( (float)rand() / 0x7FFF <= 0 )
        m_vehicleFlags |= VEHICLE_LOCKED;

    m_genericFlags &= ~( VEHGENERIC_POLICETYPE | VEHGENERIC_NOSTATIC | VEHGENERIC_PARKING | VEHGENERIC_GANGLEANON | VEHGENERIC_GANGROADBLOCK | VEHGENERIC_ISGOODCOVER | VEHGENERIC_MADDRIVER );
    m_genericFlags |= VEHGENERIC_IMPOUNDED | VEHGENERIC_ALARM | VEHGENERIC_ISGOODCOVER | VEHGENERIC_PETROLTANK | VEHGENERIC_NOPARTICLES;

    // Do the population count
    HandlePopulation( true );

    m_bombOnBoard = 0;
    m_overrideLights = 0;
    m_winchType = 0;
    
    m_gunsCycleIndex = 0;
    m_ordnanceCycleIndex = 0;

    m_health = 1000;

    m_driver = NULL;
    m_numPassengers = 0;
    m_numGettingIn = 0;
    m_gettingInFlags = 0;
    m_gettingOutFlags = 0;

    m_maxPassengers = MAX_PASSENGERS;

    for (n=0; n<MAX_PASSENGERS; n++)
        m_passengers[n] = NULL;

    m_ammoInClip &= ~0x10;

    m_usedForCover &= ~0x20;

    m_unk2 = -1;
    m_specialColModel = -1;

    m_nodeFlags |= 0x10000000;

    m_unk2 = 0;
    m_unk3 = NULL;

    m_numPedsForRoadBlock = 0;

    m_unk5 = NULL;
    m_fire = NULL;

    m_unk6 = 0;
    m_pacMansCollected = 0;

    m_hornActive = 0;
    m_unk8 = 0;
    m_unk9 = 0;

    m_unk11 = NULL;
    m_unk12 = NULL;
    m_unk13 = NULL;
    m_unk14 = NULL;

    m_alarmState = 0;
    m_doorState = 1;

    m_unk15 = 0;
    m_unk16 = 0;
    m_unk17 = 0;

    m_unk18 = 0;

    m_unk20 = NULL;
    m_unk21 = NULL;
    
    m_entityVisibilityCheck = NULL;

    m_unk22 = 0;
    m_unk23 = 0;

    m_unk25 = 0;
    m_unk26 = 0;
    m_unk27 = 0;

    m_handling = NULL;
    m_handlingFlags = 0;

    m_control.m_unk14 = 0;
    m_control.m_unk15 = 0;
    m_control.m_creationTime = pGame->GetSystemTime();
    m_control.m_flags &= ~0x04;

    m_unk29 = -1;
    m_unk28 = -1;
    m_unk30 = 0;

    m_unk31 = 0;
    m_unk32 = 0;
    m_unk33 = 0;
    m_unk35 = 0;

    for (n=0; n<MAX_UPGRADES_ATTACHED; n++)
        m_upgrades[n] = -1;

    m_usedForCover &= ~0x60;

    m_wheelScale = 1;

    m_windowsOpenFlags = 0;
    m_nitroBoosts = 0;

    m_unk36 = 0;

    m_unk24 = 0;

    m_bodyDirtLevel = rand() % 15;
    
    m_timeOfCreation = pGame->GetSystemTime();

    m_unk37 = 205089.12;
}

CVehicleSAInterface::~CVehicleSAInterface()
{
    HandlePopulation( false );
}

void CVehicleSAInterface::HandlePopulation( bool create )
{
    if ( create )
    {
        switch( m_createdBy - 1 )
        {
        case 0:
            if ( m_vehicleType & VEHICLE_POLICE )
                *((unsigned int*)VAR_PoliceVehicleCount)++;

            *((unsigned int*)VAR_VehicleCount)++;
            return;
        case 1:
            if ( m_vehicleType & VEHICLE_POLICE )
            {
                *((unsigned int*)VAR_PoliceVehicleCount)--;

                m_vehicleType &= ~VEHICLE_POLICE;
            }

            *((unsigned int*)0x0096909C)++;
            return;
        case 2:
            *((unsigned int*)0x009690A0)++;
            return;
        case 3:
            *((unsigned int*)0x009690A4)++;
            return;
        }

        return;
    }

    switch( m_createdBy - 1 )
    {
    case 0:
        if ( m_vehicleType & VEHICLE_POLICE )
            *((unsigned int*)VAR_PoliceVehicleCount)--;

        *((unsigned int*)VAR_VehicleCount)--;
        return;
    case 1:
        *((unsigned int*)0x0096909C)--;
        return;
    case 2:
        *((unsigned int*)0x009690A0)--;
        return;
    case 3:
        *((unsigned int*)0x009690A4)--;
        return;
    }
}

void CVehicleSAInterface::AddUpgrade( unsigned short model )
{
    CBaseModelInfoSAInterface *
}

bool CVehicleSAInterface::UpdateComponentStatus( unsigned short model, unsigned char collFlags, unsigned short *complex )
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
    }

    return false;
}

CVehicleSA::CVehicleSA ()
    : m_ucAlpha ( 255 ), m_bIsDerailable ( true ), m_vecGravity ( 0.0f, 0.0f, -1.0f ), m_HeadLightColor ( SColorRGBA ( 255, 255, 255, 255 ) )
{
    assert ( 0 );   // Never used ?
}

CVehicleSA::CVehicleSA( eVehicleTypes dwModelID )
    : m_ucAlpha ( 255 ), m_bIsDerailable ( true ), m_vecGravity ( 0.0f, 0.0f, -1.0f ), m_HeadLightColor ( SColorRGBA ( 255, 255, 255, 255 ) )
{
    DEBUG_TRACE("CVehicleSA::CVehicleSA( eVehicleTypes dwModelID )");
    // for SA, we can just call the following function and it should work:
    // SCM_CreateCarForScript(int,class CVector,unsigned char)
    //                              ModelID, Position, IsMissionVehicle

    m_pHandlingData = NULL;
    m_pSuspensionLines = NULL;

    DWORD dwReturn = 0;

    DWORD dwFunc = FUNC_CCarCtrlCreateCarForScript;
    _asm
    {
        push    0           // its a mission vehicle
        push    0
        push    0
        push    0           // spawn at 0,0,0
        push    dwModelID   
        call    dwFunc
        add     esp, 0x14
        mov     dwReturn, eax
    }

    m_pInterface = reinterpret_cast < CEntitySAInterface* > ( dwReturn );
#if 0
    this->BeingDeleted = FALSE;

    m_pInterface->bStreamingDontDelete = true;
    m_pInterface->bDontStream = true;
    
    // store our CVehicleSA pointer in the vehicle's time of creation member (as it won't get modified later and as far as I know it isn't used for something important)
    GetVehicleInterface ()->m_pVehicle = this;

    // Unlock doors as they spawn randomly with locked doors
    LockDoors ( false );

    // Reset the car countss to 0 so that this vehicle doesn't affect the population vehicles
    for ( int i = 0; i < 5; i++ )
    {
        MemPutFast < DWORD > ( VARS_CarCounts + i * sizeof(DWORD), 0 );
    }

    // only applicable for CAutomobile based vehicles (i.e. not bikes or boats, but includes planes, helis etc)
    this->damageManager = new CDamageManagerSA ( m_pInterface, (CDamageManagerSAInterface *)((DWORD)this->GetInterface() + 1440));


    // Replace the handling interface with our own to prevent handlig.cfg cheats and allow custom handling stuff.
    // We don't use SA's array because we want one handling per vehicle type and also allow custom handlings
    // per car later.
    /*CHandlingEntry* pEntry = pGame->GetHandlingManager ()->CreateHandlingData ();
    //CHandlingEntry* pEntry = pGame->GetHandlingManager ()->GetHandlingData ( dwModelID );
    pEntry->ApplyHandlingData ( pGame->GetHandlingManager ()->GetHandlingData ( dwModelID ) );  // We need to do that so vehicle handling wont get corrupted
    SetHandlingData ( pEntry );
    pEntry->Recalculate ();*/

    GetVehicleInterface ()->m_nVehicleFlags.bVehicleCanBeTargetted = true;

    this->internalID = pGame->GetPools ()->GetVehicleRef ( (DWORD *)this->GetVehicleInterface () );
#else
    Init ();    // Use common setup
#endif
}

CVehicleSA::CVehicleSA ( CVehicleSAInterface* pVehicleInterface )
{
    m_pInterface = pVehicleInterface;
    m_pHandlingData = NULL;
    m_pSuspensionLines = NULL;

#if 0
    m_pInterface->bStreamingDontDelete = true;
    m_pInterface->bDontStream = true;
    this->BeingDeleted = FALSE;

    // Store our CVehicleSA pointer in the vehicle's time of creation member (as it won't get modified later and as far as I know it isn't used for something important)
    GetVehicleInterface ()->m_pVehicle = this;

    // Reset the car countss to 0 so that this vehicle doesn't affect the population vehicles
    for ( int i = 0; i < 5; i++ )
    {
        MemPutFast < DWORD > ( VARS_CarCounts + i * sizeof(DWORD), 0 );
    }

    // only applicable for CAutomobile based vehicles (i.e. not bikes, trains or boats, but includes planes, helis etc)
    this->damageManager = new CDamageManagerSA( m_pInterface, (CDamageManagerSAInterface *)((DWORD)this->GetInterface() + 1440));

    this->internalID = pGame->GetPools ()->GetVehicleRef ( (DWORD *)this->GetVehicleInterface () );

    m_bIsDerailable = true;
    m_ucAlpha = 255;
#else
    Init ();    // Use common setup
#endif
}

void CVehicleSA::Init ( void )
{
    m_pInterface->m_streamingDontDelete = true;
    m_pInterface->m_dontStream = true;

    m_beingDeleted = false;

    // Store our CVehicleSA pointer in the vehicle's time of creation member (as it won't get modified later and as far as I know it isn't used for something important)
    GetVehicleInterface ()->m_pVehicle = this;

    // Unlock doors as they spawn randomly with locked doors
    LockDoors ( false );

    // Reset the car counts to 0 so that this vehicle doesn't affect the population vehicles
    for ( int i = 0; i < 5; i++ )
        *((unsigned int*)VARS_CarCounts + i) = 0;

    // only applicable for CAutomobile based vehicles (i.e. not bikes, trains or boats, but includes planes, helis etc)
    m_pDamageManager = new CDamageManagerSA( m_pInterface, (CDamageManagerSAInterface*)((unsigned int)GetInterface() + 1440) );

    GetInterface()->m_vehicleFlags.bVehicleCanBeTargetted = true;

    m_internalID = pGame->GetPools ()->GetVehicleRef ( this );

    m_bIsDerailable = true;
    m_ucAlpha = 255;
    m_vecGravity = CVector ( 0.0f, 0.0f, -1.0f );
    m_HeadLightColor = SColorRGBA ( 255, 255, 255, 255 );

    m_RGBColors[0] = CVehicleColor::GetRGBFromPaletteIndex ( GetInterface()->m_color1 );
    m_RGBColors[1] = CVehicleColor::GetRGBFromPaletteIndex ( GetInterface()->m_color2 );
    m_RGBColors[2] = CVehicleColor::GetRGBFromPaletteIndex ( GetInterface()->m_color2 );
    m_RGBColors[3] = CVehicleColor::GetRGBFromPaletteIndex ( GetInterface()->m_color3 );

    // Initialize doors depending on the vtable
	CModelInfo *info = pGame->GetModelInfo( GetVehicleInterface()->m_nModelIndex );
    DWORD dwOffset;

    if ( info->IsCar() )
        dwOffset = 1464;
    else if ( info->IsTrain() )
        dwOffset = 1496;
    else
        dwOffset = 0; // Other vehicles don't have door information

    if ( dwOffset != 0 )
    {
        for ( unsigned int i = 0; i < sizeof(m_doors) / sizeof(CDoorSA); ++i )
        {
            DWORD dwInterface = (DWORD)GetInterface ();
            DWORD dwDoorAddress = dwInterface + 1464 + i*24;
            m_doors[i].SetInterface ( (CDoorSAInterface *)dwDoorAddress );
        }
    }
    m_bSwingingDoorsAllowed = false;

    CopyGlobalSuspensionLinesToPrivate ();
}

CVehicleSA::~CVehicleSA()
{
    DEBUG_TRACE("CVehicleSA::~CVehicleSA()");

    assert( !m_beingDeleted );

    m_beingDeleted = true;

    GetInterface ()->m_vehicle = NULL;

    if ( m_pDamageManager )
    {
        delete m_pDamageManager;

        m_pDamageManager = NULL;
    }

    if ( m_pSuspensionLines )
    {
        delete m_pSuspensionLines;

        m_pSuspensionLines = NULL;
    }
    
    DWORD dwThis = (DWORD) m_pInterface;
    DWORD dwFunc = 0x6D2460;        // CVehicle::ExtinguishCarFire
    _asm
    {
        mov     ecx, dwThis
        call    dwFunc
    }

    CWorldSA* pWorld = (CWorldSA *)pGame->GetWorld();
    pWorld->Remove ( m_pInterface );
    pWorld->RemoveReferencesToDeletedObject ( m_pInterface );

    // Clever little trick, eh
    delete m_pInterface;
}

void CVehicleSA::SetMoveSpeed( CVector *vecMoveSpeed )
{
    m_velocity = *vecMoveSpeed;

    // INACCURATE. Use Get/SetTrainSpeed instead of Get/SetMoveSpeed. (Causes issue #4829).
#if 0
    // In case of train: calculate on-rail speed
    WORD wModelID = GetModelIndex();
    if ( wModelID == 537 || wModelID == 538 || wModelID == 569 || wModelID == 570 || wModelID == 590 || wModelID == 449 )
    {
        if ( !IsDerailed () )
        {
            CVehicleSAInterface* pInterf = GetVehicleInterface ();

            // Find the rail node we are on
            DWORD dwNumNodes = ((DWORD *)ARRAY_NumRailTrackNodes) [ pInterf->m_ucRailTrackID ];
            SRailNodeSA* pNode = ( (SRailNodeSA **) ARRAY_RailTrackNodePointers ) [ pInterf->m_ucRailTrackID ];
            SRailNodeSA* pNodesEnd = &pNode [ dwNumNodes ];
            while ( (float)pNode->sRailDistance / 3.0f <= pInterf->m_fTrainRailDistance && pNode < pNodesEnd )
            {
                pNode++;
            }
            if ( pNode >= pNodesEnd )
                return;
            // Get the direction vector between the nodes the train is between
            CVector vecNode1 ( (float)(pNode - 1)->sX / 8.0f, (float)(pNode - 1)->sY / 8.0f, (float)(pNode - 1)->sZ / 8.0f );
            CVector vecNode2 ( (float)pNode->sX / 8.0f, (float)pNode->sY / 8.0f, (float)pNode->sZ / 8.0f );
            CVector vecDirection = vecNode2 - vecNode1;
            vecDirection.Normalize ();
            // Set the speed
            pInterf->m_fTrainSpeed = vecDirection.DotProduct ( vecMoveSpeed );
        }
    }
#endif
}

CVehicleSAInterface* CVehicleSA::GetNextCarriageInTrain()
{
    return GetInterface()->m_nextCarriage;
}

CVehicle* CVehicleSA::GetNextTrainCarriage()
{
    return pGame->GetPools()->GetVehicle ( GetInterface()->m_nextCarriage );
}

bool CVehicleSA::AddProjectile( eWeaponType eWeapon, CVector vecOrigin, float fForce, CVector * target, CEntity * targetEntity )
{
    return pGame->GetProjectileInfo()->AddProjectile( this, eWeapon, vecOrigin, fForce, target, targetEntity );
}

void CVehicleSA::SetNextTrainCarriage ( CVehicle *next )
{
    CVehicleSA *veh = dynamic_cast <CVehicleSA*> ( next );

    if ( !veh )
    {
        GetInterface()->m_nextCarriage = NULL;
        return;
    }

    GetInterface()->m_nextCarriage = veh;

    if ( veh->GetPreviousTrainCarriage() != this )
        veh->SetPreviousTrainCarriage( this );
}

CVehicleSAInterface* CVehicleSA::GetPreviousCarriageInTrain ( void )
{
    return GetInterface()->m_prevCarriage;
}

void CVehicleSA::SetPreviousTrainCarriage ( CVehicle *previous )
{
    CVehicleSA *veh = dynamic_cast <CVehicleSA*> ( previous );

    if ( !veh )
    {
        GetInterface()->m_prevCarriage = NULL;
        return;
    }

    GetInterface()->m_prevCarriage = veh->GetInterface();

    if ( veh->GetNextTrainCarriage () != this )
        veh->SetNextTrainCarriage ( this );
}

CVehicle* CVehicleSA::GetPreviousTrainCarriage ( void )
{
    return GetInterface()->m_prevCarriage->m_vehicle;
}

bool CVehicleSA::IsDerailed ( void )
{
    return GetInterface()->m_trainFlags.bIsDerailed;
}

void CVehicleSA::SetDerailed ( bool bDerailed )
{
    CModelInfoSA *model = pGame->GetModelInfo( GetInterface()->m_model );

    if ( !model->IsTrain() )
        return;

    DWORD dwThis = (DWORD)pInterface;

    if ( bDerailed )
    {
        GetInterface()->m_trainFlags.bIsDerailed = true;
        GetInterface()->m_nodeFlags &= ~0x20004;
        return;
    }

    GetInterface()->m_trainFlags.bIsDerailed = false;
    GetInterface()->m_nodeFlags &= 0x20004;

    // Recalculate the on-rail distance from the start node (train position parameter, m_fTrainRailDistance)
    DWORD dwFunc = FUNC_CVehicle_RecalcOnRailDistance;
    _asm
    {
        mov     ecx, dwThis
        call    dwFunc
    }

    // Reset the speed
    GetVehicleInterface ()->m_fTrainSpeed = 0.0f;
}

float CVehicleSA::GetTrainSpeed ()
{
    return GetInterface ()->m_trainSpeed;
}

void CVehicleSA::SetTrainSpeed ( float fSpeed )
{
    GetInterface ()->m_trainSpeed = fSpeed;
}

bool CVehicleSA::GetTrainDirection ()
{
    return GetInterface()->m_trainFlags.bDirection != 0;
}

void CVehicleSA::SetTrainDirection ( bool bDirection )
{
    GetInterface()->m_trainFlags.bDirection = bDirection;
}

unsigned char CVehicleSA::GetRailTrack ()
{
    return GetVehicleInterface ()->m_railTrackID;
}

void CVehicleSA::SetRailTrack ( unsigned char track )
{
    if ( ucTrackID > NUM_RAILTRACKS-1 )
        return;

    CVehicleSAInterface *train = GetInterface();

    train->m_railTrackID = track;

    if ( IsDerailed () )
        return;

    DWORD dwFunc = FUNC_CVehicle_RecalcOnRailDistance;
    _asm
    {
        mov ecx, train
        call dwFunc
    }
}

bool CVehicleSA::CanPedEnterCar ()
{
    DEBUG_TRACE("bool CVehicleSA::CanPedEnterCar ( void )");

    DWORD dwThis = (DWORD) m_pInterface;
    DWORD dwFunc = FUNC_CVehicle_CanPedEnterCar;
    bool bReturn = false;

    _asm
    {
        mov     ecx, dwThis
        call    dwFunc
        mov     bReturn, al
    }

    return bReturn;
}

bool CVehicleSA::CanPedJumpOutCar ( CPed *pPed )
{
    DEBUG_TRACE("bool CVehicleSA::CanPedJumpOutCar ( CPed* pPed )");

    bool bReturn = false;
    CPedSA* pPedSA = dynamic_cast < CPedSA* > ( pPed );
    DWORD dwThis = (DWORD) m_pInterface;
    CPedSAInterface* pPedInt = pPedSA->GetPedInterface();
    DWORD dwFunc = FUNC_CVehicle_CanPedJumpOutCar;

    _asm
    {
        mov     ecx, dwThis
        push    pPedInt
        call    dwFunc
        mov     bReturn, al
    }

    return bReturn;
}

CDoorSA* CVehicleSA::GetDoor ( unsigned char ucDoor )
{
    if ( ucDoor > MAX_DOORS-1 )
        return NULL;

    return &m_doors[ucDoor];
}

void CVehicleSA::OpenDoor ( unsigned char ucDoor, float fRatio, bool bMakeNoise )
{
    // Grab the car node index for the given door id
    static int s_iCarNodeIndexes [6] = { 0x10, 0x11, 0x0A, 0x08, 0x0B, 0x09 };

	GetVehicleInterface()->OpenDoor( false, s_iCarNodeIndexes[ucDoor], ucDoor, fRatio, bMakeNoise );
}

void CVehicleSA::SetSwingingDoorsAllowed ( bool bAllowed )
{
    m_bSwingingDoorsAllowed = bAllowed;
}

bool CVehicleSA::AreSwingingDoorsAllowed () const
{
    return m_bSwingingDoorsAllowed;
}

bool CVehicleSA::AreDoorsLocked ()
{
    return ( GetVehicleInterface ()->m_doorState == 2 || GetVehicleInterface ()->m_doorState == 5 || 
             GetVehicleInterface ()->m_doorState == 4 || GetVehicleInterface ()->m_doorState == 7 || 
             GetVehicleInterface ()->m_doorState == 3 );
}

void CVehicleSA::LockDoors ( bool bLocked )
{
    bool bAreDoorsLocked = AreDoorsLocked ();
    bool bAreDoorsUndamageable = AreDoorsUndamageable ();

    if ( bLocked && !bAreDoorsLocked )
    {
        if ( bAreDoorsUndamageable )
            GetVehicleInterface ()->m_doorState = 7;
        else
            GetVehicleInterface ()->m_doorState = 2;
    }
    else if ( !bLocked && bAreDoorsLocked )
    {
        if ( bAreDoorsUndamageable )
            GetVehicleInterface ()->m_doorState = 1;
        else
            GetVehicleInterface ()->m_doorState = 0;
    }
}

bool CVehicleSA::AreDoorsUndamageable ()
{
    return ( GetVehicleInterface ()->m_doorState == 1 ||
             GetVehicleInterface ()->m_doorState == 7 );
}

void CVehicleSA::SetDoorsUndamageable ( bool bUndamageable )
{
    bool bAreDoorsLocked = AreDoorsLocked ();
    bool bAreDoorsUndamageable = AreDoorsUndamageable ();

    if ( bUndamageable && !bAreDoorsUndamageable )
    {
        if ( bAreDoorsLocked )
            GetVehicleInterface ()->m_doorState = 7;
        else
            GetVehicleInterface ()->m_doorState = 1;
    }
    else if ( !bUndamageable && bAreDoorsUndamageable )
    {
        if ( bAreDoorsLocked )
            GetVehicleInterface ()->m_doorState = 2;
        else
            GetVehicleInterface ()->m_doorState = 0;
    }
}

void CVehicleSA::AddVehicleUpgrade ( unsigned short model )
{   
    DEBUG_TRACE("void CVehicleSA::AddVehicleUpgrade ( DWORD dwModelID )");

    if ( model < 1000 || model > 1193 )
        return;

    DWORD dwThis = (DWORD)m_pInterface;
    DWORD dwFunc = FUNC_CVehicle_AddVehicleUpgrade;

    _asm
    {
        mov     ecx, dwThis
        push    model
        call    dwFunc
    }
}

void CVehicleSA::RemoveVehicleUpgrade ( unsigned short model )
{
    DEBUG_TRACE("void CVehicleSA::RemoveVehicleUpgrade ( DWORD dwModelID )");

    DWORD dwThis = (DWORD)m_pInterface;
    DWORD dwFunc = FUNC_CVehicle_RemoveVehicleUpgrade;

    _asm
    {
        mov     ecx, dwThis
        push    model
        call    dwFunc
    }
}

bool CVehicleSA::CanPedLeanOut ( CPed *pPed )
{
    DEBUG_TRACE("bool CVehicleSA::CanPedLeanOut ( CPed* pPed )");

    bool bReturn;
    CPedSA* pPedSA = dynamic_cast < CPedSA* > ( pPed );
    DWORD dwThis = (DWORD)m_pInterface;
    CPedSAInterface* pPedInt = pPedSA->GetPedInterface();
    DWORD dwFunc = FUNC_CVehicle_CanPedLeanOut;

    _asm
    {
        mov     ecx, dwThis
        push    pPedInt
        call    dwFunc
        mov     bReturn, al
    }

    return bReturn;
}

bool CVehicleSA::CanPedStepOutCar ( bool bUnknown )
{
    DEBUG_TRACE("bool CVehicleSA::CanPedStepOutCar ( bool bUnknown )");
    
    return GetInterface()->CanPedStepOutCar( bUnknown );
}

bool CVehicleSA::CarHasRoof ()
{
    DEBUG_TRACE("bool CVehicleSA::CarHasRoof ( void )");

    DWORD dwThis = (DWORD) m_pInterface;
    DWORD dwFunc = FUNC_CVehicle_CarHasRoof;
    bool bReturn;

    _asm
    {
        mov     ecx, dwThis
        call    dwFunc
        mov     bReturn, al
    }

    return bReturn;
}

void CVehicleSA::ExtinguishCarFire ()
{
    DEBUG_TRACE("void CVehicleSA::ExtinguishCarFire ( void )");
    DWORD dwThis = (DWORD) m_pInterface;
    DWORD dwFunc = FUNC_CVehicle_ExtinguishCarFire;

    _asm
    {
        mov     ecx, dwThis
        call    dwFunc
    }
}

unsigned int CVehicleSA::GetBaseVehicleType ()
{
    DEBUG_TRACE("DWORD CVehicleSA::GetBaseVehicleType ( void )");
    
    return *((unsigned int)GetInterface() + 1424);
}

void CVehicleSA::SetBodyDirtLevel ( float fDirtLevel )
{
    DEBUG_TRACE("void CVehicleSA::SetBodyDirtLevel ( float fDirtLevel )");

    GetInterface ()->m_bodyDirtLevel = fDirtLevel;
}

float CVehicleSA::GetBodyDirtLevel ()
{
    DEBUG_TRACE("float CVehicleSA::GetBodyDirtLevel ( void )");

    return GetInterface ()->m_bodyDirtLevel;
}

unsigned char CVehicleSA::GetCurrentGear ()
{
    DEBUG_TRACE("unsigned char CVehicleSA::GetCurrentGear ( void )");

    return GetInterface ()->m_currentGear;
}

float CVehicleSA::GetGasPedal ()
{
    DEBUG_TRACE("float CVehicleSA::GetGasPedal ( void )");
    return GetVehicleInterface ()->m_fGasPedal;
}

float CVehicleSA::GetHeightAboveRoad ()
{
    DEBUG_TRACE("float CVehicleSA::GetHeightAboveRoad ( void )");
    DWORD dwThis = (DWORD) GetVehicleInterface ();
    DWORD dwFunc = FUNC_CVehicle_GetHeightAboveRoad;
    float fReturn;

    _asm
    {
        mov     ecx, dwThis
        call    dwFunc
        fstp    fReturn
    }

    return fReturn;
}


float CVehicleSA::GetSteerAngle ()
{
    DEBUG_TRACE("float CVehicleSA::GetSteerAngle ( void )");
    return GetVehicleInterface ()->m_fSteerAngle;
}


bool CVehicleSA::GetTowBarPos ( CVector* pVector )
{
    DEBUG_TRACE("bool CVehicleSA::GetTowBarPos ( CVector* pVector )");
    
	return GetVehicleInterface()->GetTowbarPosition( pVector, 1, 0 );
}


bool CVehicleSA::GetTowHitchPos ( CVector* pVector )
{
    DEBUG_TRACE("bool CVehicleSA::GetTowHitchPos ( CVector* pVector )");
   
	return GetVehicleInterface()->GetTowHitchPosition( pVector, 1, 0 );
}


bool CVehicleSA::IsOnItsSide ()
{
    DEBUG_TRACE("bool CVehicleSA::IsOnItsSide ( void )");
    DWORD dwThis = (DWORD) m_pInterface;
    DWORD dwFunc = FUNC_CVehicle_IsOnItsSide;
    bool bReturn = false;

    _asm
    {
        mov     ecx, dwThis
        call    dwFunc
        mov     bReturn, al
    }
    return bReturn;
}

bool CVehicleSA::IsLawEnforcementVehicle ()
{
    DEBUG_TRACE("bool CVehicleSA::IsLawEnforcementVehicle ( void )");
    DWORD dwThis = (DWORD) m_pInterface;
    DWORD dwFunc = FUNC_CVehicle_IsLawEnforcementVehicle;
    bool bReturn = false;

    _asm
    {
        mov     ecx, dwThis
        call    dwFunc
        mov     bReturn, al
    }
    return bReturn;
}


bool CVehicleSA::IsPassenger ( CPed *pPed )
{
    DEBUG_TRACE("bool CVehicleSA::IsPassenger ( CPed* pPed )");

    DWORD dwThis = (DWORD) m_pInterface;
    DWORD dwFunc = FUNC_CVehicle_IsPassenger;
    bool bReturn = false;

    _asm
    {
        mov     ecx, dwThis
        push    pPed
        call    dwFunc
        mov     bReturn, al
    }
    return bReturn;
}

bool CVehicleSA::IsSphereTouchingVehicle ( CVector * vecOrigin, float fRadius )
{
    DEBUG_TRACE("bool CVehicleSA::IsSphereTouchingVehicle ( CVector * vecOrigin, float fRadius )");

    DWORD dwThis = (DWORD) m_pInterface;
    DWORD dwFunc = FUNC_CVehicle_IsSphereTouchingVehicle;
    bool bReturn = false;

    _asm
    {
        push    eax

        mov     ecx, dwThis
        mov     eax, vecOrigin
        push    fRadius
        push    dword ptr [eax]
        push    dword ptr [eax + 4]
        push    dword ptr [eax + 8]
        call    dwFunc
        mov     bReturn, al
    }
    return bReturn;
}


bool CVehicleSA::IsUpsideDown ( void )
{
    DEBUG_TRACE("bool CVehicleSA::IsUpsideDown ( void )");

    DWORD dwThis = (DWORD) m_pInterface;
    DWORD dwFunc = FUNC_CVehicle_IsUpsideDown;
    bool bReturn = false;

    _asm
    {
        mov     ecx, dwThis
        call    dwFunc
        mov     bReturn, al
    }

    return bReturn;
}

void CVehicleSA::MakeDirty ( CColPoint* pPoint )
{
    DEBUG_TRACE("void CVehicleSA::MakeDirty ( CColPoint* pPoint )");

    DWORD dwThis = (DWORD) m_pInterface;
    DWORD dwFunc = FUNC_CVehicle_MakeDirty;

    _asm
    {
        mov     ecx, dwThis
        push    pPoint
        call    dwFunc
    }
}

void CVehicleSA::SetEngineOn ( bool bEngineOn )
{
    DEBUG_TRACE("void CVehicleSA::SetEngineOn ( bool bEngineOn )");

    DWORD dwThis = (DWORD) m_pInterface;
    DWORD dwEngineOn = (DWORD) bEngineOn;
    DWORD dwFunc = FUNC_CVehicle_SetEngineOn;

    _asm
    {
        mov     ecx, dwThis
        push    dwEngineOn
        call    dwFunc
    }
}

CPed* CVehicleSA::GetDriver ( void )
{
    DEBUG_TRACE("CPed* CVehicleSA::GetDriver ( void )");
    CPoolsSA* pPools  = (CPoolsSA *)pGame->GetPools();

    CPedSAInterface* pDriver = GetVehicleInterface ()->pDriver;
    if ( pDriver )
        return pPools->GetPed( (DWORD*) pDriver );
    else
        return NULL;
}


CPed* CVehicleSA::GetPassenger ( unsigned char ucSlot )
{
    DEBUG_TRACE("CPed* CVehicleSA::GetPassenger ( unsigned char ucSlot )");
    CPoolsSA* pPools  = (CPoolsSA *)pGame->GetPools();

    if ( ucSlot < 8 )
    {
        CPedSAInterface* pPassenger = GetVehicleInterface ()->pPassengers [ucSlot];
        if ( pPassenger )
            return pPools->GetPed( (DWORD*) pPassenger );
    }

    return NULL;
}


bool CVehicleSA::IsBeingDriven()
{
    DEBUG_TRACE("bool CVehicleSA::IsBeingDriven()");
    CVehicleSAInterface * vehicle = (CVehicleSAInterface *)this->GetInterface();
    if ( GetVehicleInterface ()->pDriver != NULL )
        return TRUE;
    else
        return FALSE;
}


/**
 * \todo Implement for other vehicle types too and check
 */
void CVehicleSA::PlaceBikeOnRoadProperly()
{
    DEBUG_TRACE("void CVehicleSA::PlaceBikeOnRoadProperly()");
    DWORD dwFunc = FUNC_Bike_PlaceOnRoadProperly;
    DWORD dwBike = (DWORD)this->GetInterface();

    _asm
    {
        mov     ecx, dwBike
        call    dwFunc
    }
}

void CVehicleSA::PlaceAutomobileOnRoadProperly()
{
    DEBUG_TRACE("void CVehicleSA::PlaceAutomobileOnRoadProperly()");
    DWORD dwFunc = FUNC_Automobile_PlaceOnRoadProperly;
    DWORD dwAutomobile = (DWORD)this->GetInterface();

    _asm
    {
        mov     ecx, dwAutomobile
        call    dwFunc
    }
}

void CVehicleSA::SetColor ( SColor color1, SColor color2, SColor color3, SColor color4, int )
{
    m_RGBColors[0] = color1;
    m_RGBColors[1] = color2;
    m_RGBColors[2] = color3;
    m_RGBColors[3] = color4;
}

void CVehicleSA::GetColor ( SColor* color1, SColor* color2, SColor* color3, SColor* color4, int )
{
    *color1 = m_RGBColors[0];
    *color2 = m_RGBColors[1];
    *color3 = m_RGBColors[2];
    *color4 = m_RGBColors[3];
}

// works with firetrucks & tanks
void CVehicleSA::GetTurretRotation ( float * fHorizontal, float * fVertical )
{
    DEBUG_TRACE("void * CVehicleSA::GetTurretRotation ( float * fHorizontal, float * fVertical )");
    // This is coded in asm because for some reason it was failing to compile
    // correctly with normal c++.
    DWORD vehicleInterface = (DWORD)this->GetInterface();
    float fHoriz = 0.0f;
    float fVert = 0.0f;
    _asm
    {
        mov     eax, vehicleInterface
        add     eax, 0x94C
        fld     [eax]
        fstp    fHoriz
        add     eax, 4
        fld     [eax]
        fstp    fVert
    }
    *fHorizontal = fHoriz;
    *fVertical = fVert;
}

void CVehicleSA::SetTurretRotation ( float fHorizontal, float fVertical )
{
    DEBUG_TRACE("void * CVehicleSA::SetTurretRotation ( float fHorizontal, float fVertical )");
    //*(float *)(this->GetInterface() + 2380) = fHorizontal;
    //*(float *)(this->GetInterface() + 2384) = fVertical;
    DWORD vehicleInterface = (DWORD)this->GetInterface();
    _asm
    {
        mov     eax, vehicleInterface
        add     eax, 0x94C
        fld     fHorizontal
        fstp    [eax]
        add     eax, 4
        fld     fVertical
        fstp    [eax]
    }
}

bool CVehicleSA::IsSirenOrAlarmActive ( )
{
    return ((CVehicleSAInterface *)this->GetInterface())->m_nVehicleFlags.bSirenOrAlarm;
}

void CVehicleSA::SetSirenOrAlarmActive ( bool bActive )
{
   ((CVehicleSAInterface *)this->GetInterface())->m_nVehicleFlags.bSirenOrAlarm = bActive;
}

DWORD * CVehicleSA::GetMemoryValue ( DWORD dwOffset )
{
    if ( dwOffset <= SIZEOF_CHELI )
        return (DWORD *)((DWORD)(this->GetInterface()) + dwOffset);
    else
        return NULL;
}

/*

bool CVehicleSA::isInRect(FLOAT fX1, FLOAT fY1, FLOAT fX2, FLOAT fY2, bool bShowHotspot)
{
    return (bool)CallScriptFunction(GTAVC_SCM_IS_CAR_IN_RECT_STILL,&this->internalID,&fX1,&fY1,&fX2,&fY2,&bShowHotspot);
}

//-----------------------------------------------------------

bool CVehicleSA::isInCube(CVector * v3dCorner1, CVector * v3dCorner2, bool bShowHotspot)
{
    return (bool)CallScriptFunction(GTAVC_SCM_IS_CAR_IN_CUBE_STILL,&this->internalID,*v3dCorner1,*v3dCorner2,&bShowHotspot);
}

//-----------------------------------------------------------

bool CVehicleSA::isNearPoint(FLOAT fX, FLOAT fY, FLOAT fRadiusX, FLOAT fRadiusY, bool bShowHotspot, bool bStill)
{
    return (bool)CallScriptFunction(GTAVC_SCM_IS_CAR_NEAR_POINT,&this->internalID,&fX,&fY,&fRadiusX,&fRadiusY,&bShowHotspot,&bStill);
}

//-----------------------------------------------------------

bool CVehicleSA::isNearPoint3D(FLOAT fX, FLOAT fY, FLOAT fZ, FLOAT fRadiusX, FLOAT fRadiusY, FLOAT fRadiusZ, bool bShowHotspot, bool bStill)
{
    return (bool)CallScriptFunction(GTAVC_SCM_IS_CAR_NEAR_POINT_3D,&this->internalID,&fX,&fY,&fZ,&fRadiusX,&fRadiusY,&fRadiusZ,&bShowHotspot,&bStill);
}

//-----------------------------------------------------------

bool CVehicleSA::isStopped()
{
    return (bool)CallScriptFunction(GTAVC_SCM_IS_CAR_STOPPED,&this->internalID);
}

//-----------------------------------------------------------

bool CVehicleSA::isWrecked()
{
    return (bool)CallScriptFunction(GTAVC_SCM_IS_CAR_WRECKED,&this->internalID);
}

//-----------------------------------------------------------

bool CVehicleSA::isOnLand()
{
    return (bool)CallScriptFunction(GTAVC_SCM_IS_CAR_ON_LAND,&this->internalID);
}

//-----------------------------------------------------------

bool CVehicleSA::isCrushed()
{
    return (bool)CallScriptFunction(GTAVC_SCM_IS_CAR_CRUSHED,&this->internalID);
}

//-----------------------------------------------------------

bool CVehicleSA::isOnRoof()
{
    return (bool)CallScriptFunction(GTAVC_SCM_IS_CAR_FLIPPED,&this->internalID);
}

//-----------------------------------------------------------

bool CVehicleSA::isAirbourne()
{
    return (bool)CallScriptFunction(GTAVC_SCM_IS_AIRBORNE,&this->internalID);
}

//-----------------------------------------------------------

bool CVehicleSA::isUpsidedown()
{
    return (bool)CallScriptFunction(GTAVC_SCM_IS_CAR_UPSIDEDOWN,&this->internalID);
}

//-----------------------------------------------------------

bool CVehicleSA::isStuck()
{
    return (bool)CallScriptFunction(GTAVC_SCM_IS_CAR_STUCK,&this->internalID);
}

//-----------------------------------------------------------

bool CVehicleSA::isSunk()
{
    return (bool)CallScriptFunction(GTAVC_SCM_HAS_CAR_SUNK,&this->internalID);
}

//-----------------------------------------------------------

bool CVehicleSA::isBoundingSphereVisible()
{
    return (bool)CallScriptFunction(GTAVC_SCM_IS_CAR_BOUNDING_SPHERE_VISIBLE,&this->internalID);
}

//-----------------------------------------------------------

bool CVehicleSA::isDamaged()
{
    return (bool)CallScriptFunction(GTAVC_SCM_IS_CAR_DAMAGED,&this->internalID);
}

//-----------------------------------------------------------

bool CVehicleSA::isBombActive()
{
    return (bool)CallScriptFunction(GTAVC_SCM_IS_CAR_BOMB_STATUS,&this->internalID);
}

//-----------------------------------------------------------

bool CVehicleSA::isPassengerSeatFree()
{
    return (bool)CallScriptFunction(GTAVC_SCM_IS_CAR_PASSENGER_SEAT_FREE,&this->internalID);
}

//-----------------------------------------------------------

DWORD CVehicleSA::passengerCount()
{
    return ((CVehicleSAInterface *)this->GetInterface())->PassengerCount;
}

//-----------------------------------------------------------

DWORD CVehicleSA::maxPassengers()
{
    DWORD dwMaxPassengers = 0;
    CallScriptFunction(GTAVC_SCM_GET_CAR_MAX_PASSENGERS,&this->internalID, &dwMaxPassengers);
    return dwMaxPassengers;
}

//-----------------------------------------------------------

void CVehicleSA::setDensity(DWORD dwDensity)
{
    CallScriptFunction(GTAVC_SCM_SET_CAR_DENSITY,&this->internalID,&dwDensity);
}

//-----------------------------------------------------------

void CVehicleSA::makeHeavy(bool bHeavy)
{
    CallScriptFunction(GTAVC_SCM_MAKE_CAR_HEAVY,&this->internalID,&bHeavy);
}
*/
//-----------------------------------------------------------
/**
 * \todo Make a eDoorState enum
 */
/*
void CVehicleSA::setDoorsStatus(DWORD dwStatus)
{
    CallScriptFunction(GTAVC_SCM_SET_CAR_DOOR_STATUS,&this->internalID,&dwStatus);
}

//-----------------------------------------------------------

void CVehicleSA::setTaxiLight(bool bOn)
{
    CallScriptFunction(GTAVC_SCM_SET_CAR_TAXIAVAILABLE,&this->internalID,&bOn);
}

//-----------------------------------------------------------

void CVehicleSA::setSpeed(FLOAT fSpeed)
{
    CallScriptFunction(GTAVC_SCM_SET_CAR_SPEED,&this->internalID,&fSpeed);
}

//-----------------------------------------------------------

FLOAT CVehicleSA::getSpeed()
{
    FLOAT fReturn;
    CallScriptFunction(GTAVC_SCM_GET_CAR_SPEED,&this->internalID, &fReturn);
    return fReturn;
}

//-----------------------------------------------------------

void CVehicleSA::setSpeedInstantly(FLOAT fSpeed)
{
    CallScriptFunction(GTAVC_SCM_SET_CAR_SPEED_INSTANTLY,&this->internalID,&fSpeed);
}
//-----------------------------------------------------------

void CVehicleSA::setImmunities(bool bBullet, bool bFire, bool bExplosion, bool bDamage, bool bUnknown)
{
    CallScriptFunction(GTAVC_SCM_SET_CAR_IMMUNITIES,&this->internalID,&bBullet,&bFire,&bExplosion,&bDamage,&bUnknown);
}
*/

//-----------------------------------------------------------
FLOAT CVehicleSA::GetHealth()
{
    CVehicleSAInterface * vehicle = (CVehicleSAInterface *)this->GetInterface();
    return vehicle->m_nHealth;
}
//-----------------------------------------------------------
void CVehicleSA::SetHealth( FLOAT fHealth )
{
    CVehicleSAInterface * vehicle = (CVehicleSAInterface *)this->GetInterface();
    vehicle->m_nHealth = fHealth;
    if ( fHealth >= 250.0f )
        vehicle->m_fBurningTime = 0.0f;
}

// SHould be plane funcs

void CVehicleSA::SetLandingGearDown ( bool bLandingGearDown )
{
    // This is the C code corresponding to the gta_sa
    // asm code at address 0x6CB39A.
    CVehicleSAInterface* pInterface = this->GetVehicleInterface ();
    DWORD dwThis = (DWORD)pInterface;
    float& fPosition = *(float *)( dwThis + 0x9CC );
    float& fTimeStep = *(float *)( 0xB7CB5C );
    float& flt_871904 = *(float *)( 0x871904 );
    
    if ( IsLandingGearDown () != bLandingGearDown )
    {
        // The following code toggles the landing gear direction
        if ( fPosition == 0.0f )
        {
            MemPutFast < DWORD > ( dwThis + 0x5A5, 0x02020202 );
            fPosition += ( fTimeStep * flt_871904 );
        }
        else
        {
            if ( fPosition != 1.0f )
                fPosition *= -1.0f;
            else
                fPosition = ( fTimeStep * flt_871904 ) - 1.0f;
        }
    }
}

float CVehicleSA::GetLandingGearPosition ( )
{
    DWORD dwThis = (DWORD)this->GetInterface();
    return *(float *)(dwThis + 2508);
}

void CVehicleSA::SetLandingGearPosition ( float fPosition )
{
    DWORD dwThis = (DWORD)this->GetInterface();
    MemPutFast < float > ( dwThis + 2508, fPosition );
}

bool CVehicleSA::IsLandingGearDown ( )
{
    DWORD dwThis = (DWORD)this->GetInterface();
    if ( *(float *)(dwThis + 2508) <= 0.0f )
        return true;
    else
        return false;
}

void CVehicleSA::Fix ( void )
{
    DEBUG_TRACE("void CVehicleSA::Fix ( void )");
    DWORD dwThis = (DWORD) GetInterface();

    DWORD dwFunc = 0;
    CModelInfo* pModelInfo = pGame->GetModelInfo ( this->GetModelIndex() );
    if ( pModelInfo )
    {
        if ( pModelInfo->IsCar() || pModelInfo->IsMonsterTruck() || pModelInfo->IsTrailer() )
            dwFunc = FUNC_CAutomobile__Fix;
        else if ( pModelInfo->IsPlane() )
            dwFunc = FUNC_CPlane__Fix;
        else if ( pModelInfo->IsHeli() )
            dwFunc = FUNC_CHeli__Fix;
        else if ( pModelInfo->IsBike() )
            dwFunc = FUNC_CBike_Fix;

        if ( dwFunc)
        {
            _asm
            {
                mov     ecx, dwThis
                call    dwFunc
            }
        }
    }
}

CDamageManager * CVehicleSA::GetDamageManager()
{
    DEBUG_TRACE("CDamageManager * CVehicleSA::GetDamageManager()");
    return this->m_pDamageManager;
}

void CVehicleSA::BlowUp ( CEntity* pCreator, unsigned long ulUnknown )
{
    GetVehicleInterface()->Blow( pCreator, ulUnknown );
}


void CVehicleSA::BlowUpCutSceneNoExtras ( unsigned long ulUnknown1, unsigned long ulUnknown2, unsigned long ulUnknown3, unsigned long ulUnknown4 )
{
    GetVehicleInterface()->BlowWithCutscene( ulUnknown1, ulUnknown2, ulUnknown3, ulUnknown4 );
}


void CVehicleSA::FadeOut ( bool bFadeOut )
{
    CVehicleSAInterface * vehicle = (CVehicleSAInterface *)this->GetInterface();
    vehicle->bDistanceFade = bFadeOut;
    vehicle->m_nVehicleFlags.bFadeOut = bFadeOut;
}

bool CVehicleSA::IsFadingOut ( void )
{
    CVehicleSAInterface * vehicle = (CVehicleSAInterface *)this->GetInterface();
    return vehicle->m_nVehicleFlags.bFadeOut;
}


unsigned char CVehicleSA::GetNumberGettingIn ( void )
{
    return GetVehicleInterface ()->m_nNumGettingIn;
}


unsigned char CVehicleSA::GetPassengerCount ( void )
{
    return GetVehicleInterface ()->m_nNumPassengers;
}


unsigned char CVehicleSA::GetMaxPassengerCount ( void )
{
    return GetVehicleInterface ()->m_nMaxPassengers;
}

bool CVehicleSA::SetTowLink ( CVehicle* pVehicle )
{
    DEBUG_TRACE("bool CAutomobileSA::SetTowLink ( CVehicle* pVehicle )");

	return GetVehicleInterface()->SetTowLink( pVehicle->GetVehicleInterface(), 1 );
}

bool CVehicleSA::BreakTowLink ( void )
{
    DEBUG_TRACE("bool CVehicleSA::BreakTowLink ( void )");
	
	return GetVehicleInterface()->BreakTowLink();
}

CVehicle * CVehicleSA::GetTowedVehicle ( void )
{
    DEBUG_TRACE("CVehicle * CVehicleSA::GetTowedVehicle ( void )");
    CVehicleSAInterface * pTowedVehicle = (CVehicleSAInterface *)*(DWORD *)((DWORD)this->GetInterface() + 1224);

    if ( !pTowedVehicle )
		return NULL;

    return pGame->GetPools()->GetVehicle ( (DWORD *)pTowedVehicle );
}

CVehicle * CVehicleSA::GetTowedByVehicle ( void )
{
    DEBUG_TRACE("CVehicle * CVehicleSA::GetTowedVehicle ( void )");
    CVehicleSAInterface * pTowedVehicle = (CVehicleSAInterface *)*(DWORD *)((DWORD)this->GetInterface() + 1220);
    if ( pTowedVehicle )
        return pGame->GetPools()->GetVehicle ( (DWORD *)pTowedVehicle );
    return NULL;
}

void CVehicleSA::SetWinchType ( eWinchType winchType )
{
    if ( winchType < 4 && winchType != 2 )
    {
        GetVehicleInterface ()->WinchType = winchType;
    }
}

void CVehicleSA::PickupEntityWithWinch ( CEntity* pEntity )
{
    CEntitySA* pEntitySA = dynamic_cast < CEntitySA* > ( pEntity );

    if ( pEntitySA )
    {
        DWORD dwFunc = FUNC_CVehicle_PickUpEntityWithWinch;
        DWORD dwThis = (DWORD) GetInterface();
        DWORD dwEntityInterface = (DWORD) pEntitySA->GetInterface ();

        _asm
        {
            push    dwEntityInterface
            mov     ecx, dwThis
            call    dwFunc
        }
    }
}

void CVehicleSA::ReleasePickedUpEntityWithWinch ( void )
{
    DWORD dwFunc = FUNC_CVehicle_ReleasePickedUpEntityWithWinch;
    DWORD dwThis = (DWORD) GetInterface();

    _asm
    {
        mov     ecx, dwThis
        call    dwFunc
    }
}

void CVehicleSA::SetRopeHeightForHeli ( float fRopeHeight )
{
    DWORD dwFunc = FUNC_CVehicle_SetRopeHeightForHeli;
    DWORD dwThis = (DWORD) GetInterface();

    _asm
    {
        push    fRopeHeight
        mov     ecx, dwThis
        call    dwFunc
    }
}

CPhysical * CVehicleSA::QueryPickedUpEntityWithWinch ( )
{
    DWORD dwFunc = FUNC_CVehicle_QueryPickedUpEntityWithWinch;
    DWORD dwThis = (DWORD) GetInterface();

    CPhysicalSAInterface * phys;
    CPhysical * physRet;
    _asm
    {
        mov     ecx, dwThis       
        call    dwFunc
        mov     phys, eax
    }

    if ( phys )
    {
        CPoolsSA * pPools = ((CPoolsSA *)pGame->GetPools());
        switch(phys->nType)
        {
        case ENTITY_TYPE_PED:
            physRet = (CPhysical *)pPools->GetPed((DWORD *)phys);
            break;
        case ENTITY_TYPE_VEHICLE:
            physRet = (CPhysical *)pGame->GetPools()->GetVehicle ( (DWORD *)phys );
            break;
        case ENTITY_TYPE_OBJECT:
            physRet = (CPhysical *)pPools->GetObject( (DWORD *)phys );
            break;
        default:
            physRet = NULL;
        }
    }
    return physRet;
}

void CVehicleSA::SetRemap ( int iRemap )
{
    DWORD dwFunc = FUNC_CVehicle__SetRemap;
    DWORD dwThis = (DWORD) GetInterface();
    _asm
    {
        mov     ecx, dwThis
        push    iRemap
        call    dwFunc
    }
}


int CVehicleSA::GetRemapIndex ( void )
{
    DWORD dwFunc = FUNC_CVehicle__GetRemapIndex;
    DWORD dwThis = (DWORD) GetInterface();
    int iReturn = 0;
    _asm
    {
        mov     ecx, dwThis
        call    dwFunc
        mov     iReturn, eax
    }
    return iReturn;
}

void CVehicleSA::SetRemapTexDictionary ( int iRemapTextureDictionary )
{
    DWORD dwFunc = FUNC_CVehicle__SetRemapTexDictionary;
    DWORD dwThis = (DWORD) GetInterface();
    _asm
    {
        mov     ecx, dwThis
        push    iRemapTextureDictionary
        call    dwFunc
    }
}

bool CVehicleSA::IsSmokeTrailEnabled ( void )
{
    return ( *(unsigned char*)((DWORD)this->GetInterface() + 2560) == 1 );
}

void CVehicleSA::SetSmokeTrailEnabled ( bool bEnabled )
{
    MemPutFast < unsigned char > ( (DWORD)this->GetInterface() + 2560, ( bEnabled ) ? 1 : 0 );
}


CHandlingEntry* CVehicleSA::GetHandlingData ( void )
{
    return m_pHandlingData;
}


void CVehicleSA::SetHandlingData ( CHandlingEntry* pHandling )
{
    // Store the handling and recalculate it
    m_pHandlingData = static_cast < CHandlingEntrySA* > ( pHandling );
    GetVehicleInterface ()->pHandlingData = m_pHandlingData->GetInterface ();
    RecalculateHandling ();
}


void CVehicleSA::RecalculateHandling ( void )
{
    if ( !m_pHandlingData )
        return;

    m_pHandlingData->Recalculate ();
    
    // Recalculate the suspension lines
    RecalculateSuspensionLines ();

    // Put it in our interface
    CVehicleSAInterface* pInt = GetVehicleInterface ();
    unsigned int uiHandlingFlags = m_pHandlingData->GetInterface ()->uiHandlingFlags;
    // user error correction - NOS_INST = NOS Installed t/f
    // if nos is installed we need the flag set
    if ( pInt->m_upgrades[0] && pInt->m_upgrades[0] >= 1008 && pInt->m_upgrades[0] <= 1010 )
    {
        // Flag not enabled?
        if ( uiHandlingFlags | HANDLING_NOS_Flag )
        {
            // Set zee flag
            uiHandlingFlags |= HANDLING_NOS_Flag;
            m_pHandlingData->SetHandlingFlags ( uiHandlingFlags );
        }
    }
    else
    {
        // Flag Enabled?
        if ( uiHandlingFlags & HANDLING_NOS_Flag )
        {
            // Unset the flag
            uiHandlingFlags &= ~HANDLING_NOS_Flag;
            m_pHandlingData->SetHandlingFlags ( uiHandlingFlags );
        }
    }
    // Hydraulics Flag fixing
    if ( pInt->m_upgrades[1] && pInt->m_upgrades[1] == 1087 )
    {
        // Flag not enabled?
        if ( uiHandlingFlags | HANDLING_Hydraulics_Flag )
        {
            // Set zee flag
            uiHandlingFlags |= HANDLING_Hydraulics_Flag;
            m_pHandlingData->SetHandlingFlags ( uiHandlingFlags );
        }
    }
    else
    {
        // Flag Enabled?
        if ( uiHandlingFlags & HANDLING_Hydraulics_Flag )
        {
            // Unset the flag
            uiHandlingFlags &= ~HANDLING_Hydraulics_Flag;
            m_pHandlingData->SetHandlingFlags ( uiHandlingFlags );
        }
    }
    pInt->dwHandlingFlags = uiHandlingFlags;
    pInt->fMass = m_pHandlingData->GetInterface ()->fMass;
    pInt->fTurnMass = m_pHandlingData->GetInterface ()->fTurnMass;// * pGame->GetHandlingManager()->GetTurnMassMultiplier();
    pInt->vecCenterOfMass = &m_pHandlingData->GetInterface()->vecCenterOfMass;
    pInt->fBuoyancyConstant = m_pHandlingData->GetInterface()->fUnknown2;
    /*if (m_pHandlingData->GetInterface()->fDragCoeff >= pGame->GetHandlingManager()->GetBasicDragCoeff())
        GetVehicleInterface ()->fDragCoeff = pGame->GetHandlingManager()->GetBasicDragCoeff();
    else*/
        //pInt->fDragCoeff = m_pHandlingData->GetInterface()->fDragCoeff / 1000 * pGame->GetHandlingManager()->GetDragMultiplier();
}

void CVehicleSA::BurstTyre ( unsigned char tyre )
{
    GetVehicleInterface()->BurstTyre( tyre, 1 );
}

BYTE CVehicleSA::GetBikeWheelStatus ( BYTE bWheel )
{
    if ( bWheel == 0 ) return * ( BYTE * ) ( (DWORD)this->GetInterface() + 0x65C );
    if ( bWheel == 1 ) return * ( BYTE * ) ( (DWORD)this->GetInterface() + 0x65D );
    return 0;
}


void CVehicleSA::SetBikeWheelStatus ( BYTE bWheel, BYTE bStatus )
{
    if ( bWheel == 0 ) * ( BYTE * ) ( (DWORD)this->GetInterface() + 0x65C ) = bStatus;
    else if ( bWheel == 1 ) * ( BYTE * ) ( (DWORD)this->GetInterface() + 0x65D ) = bStatus;
}

void CVehicleSA::SetTaxiLightOn ( bool bLightOn )
{
    DEBUG_TRACE("void CVehicleSA::SetTaxiLight ( bool bLightOn )");
    DWORD dwThis = (DWORD) GetInterface();
    DWORD dwState = (DWORD) bLightOn;
    DWORD dwFunc = FUNC_CAutomobile_SetTaxiLight;

    _asm
    {
        mov     ecx, dwThis
        push    dwState
        call    dwFunc
    }
}

void GetMatrixForGravity ( const CVector& vecGravity, CMatrix& mat )
{
    // Calculates a basis where the z axis is the inverse of the gravity
    if ( vecGravity.Length () > 0.0001f )
    {
        mat.vUp = -vecGravity;
        mat.vUp.Normalize ();
        if ( fabs(mat.vUp.fX) > 0.0001f || fabs(mat.vUp.fZ) > 0.0001f )
        {
            CVector y ( 0.0f, 1.0f, 0.0f );
            mat.vFront = vecGravity;
            mat.vFront.CrossProduct ( &y );
            mat.vFront.CrossProduct ( &vecGravity );
            mat.vFront.Normalize ();
        }
        else
        {
            mat.vFront = CVector ( 0.0f, 0.0f, vecGravity.fY );
        }
        mat.vRight = mat.vFront;
        mat.vRight.CrossProduct ( &mat.vUp );
    }
    else
    {
        // No gravity, use default axes
        mat.vRight = CVector ( 1.0f, 0.0f, 0.0f );
        mat.vFront = CVector ( 0.0f, 1.0f, 0.0f );
        mat.vUp    = CVector ( 0.0f, 0.0f, 1.0f );
    }
}


void CVehicleSA::SetGravity ( const CVector* pvecGravity )
{
    if ( pGame->GetPools ()->GetPedFromRef ( 1 )->GetVehicle () == this )
    {
        // If this is the local player's vehicle, adjust the camera's position history.
        // This is to keep the automatic camera settling (which happens when driving while not moving the mouse)
        // nice and consistent while the gravity changes.
        CCam* pCam = pGame->GetCamera ()->GetCam ( pGame->GetCamera ()->GetActiveCam () );

        CMatrix matOld, matNew;
        GetMatrixForGravity ( m_vecGravity, matOld );
        GetMatrixForGravity ( *pvecGravity, matNew );
        
        CVector* pvecPosition = &m_pInterface->Placeable.matrix->vPos;

        matOld.Invert ();
        pCam->GetTargetHistoryPos () [ 0 ] = matOld * (pCam->GetTargetHistoryPos () [ 0 ] - *pvecPosition);
        pCam->GetTargetHistoryPos () [ 0 ] = matNew * pCam->GetTargetHistoryPos () [ 0 ] + *pvecPosition;

        pCam->GetTargetHistoryPos () [ 1 ] = matOld * (pCam->GetTargetHistoryPos () [ 1 ] - *pvecPosition);
        pCam->GetTargetHistoryPos () [ 1 ] = matNew * pCam->GetTargetHistoryPos () [ 1 ] + *pvecPosition;
    }

    m_vecGravity = *pvecGravity;
}


CObject * CVehicleSA::SpawnFlyingComponent ( int i_1, unsigned int ui_2 )
{
    DWORD dwReturn;
    DWORD dwThis = ( DWORD ) GetInterface ();
    DWORD dwFunc = FUNC_CAutomobile__SpawnFlyingComponent;
    _asm
    {
        mov     ecx, dwThis
        push    ui_2
        push    i_1
        call    dwFunc
        mov     dwReturn, eax
    }

    CObject * pObject = NULL;
    if ( dwReturn ) pObject = pGame->GetPools ()->GetObject ( ( DWORD * ) dwReturn );
    return pObject;
}


typedef RwFrame * (__cdecl *RwFrameForAllObjects_t) (RwFrame * frame, void * callback, void * data);
extern RwFrameForAllObjects_t RwFrameForAllObjects;
void CVehicleSA::SetWheelVisibility ( eWheels wheel, bool bVisible )
{    
    CVehicleSAInterface * vehicle = (CVehicleSAInterface *)this->GetInterface();
    RwFrame * pFrame = NULL;
    switch ( wheel )
    {        
        case FRONT_LEFT_WHEEL: pFrame = vehicle->pWheelFrontLeft; break;
        case REAR_LEFT_WHEEL: pFrame = vehicle->pWheelRearLeft; break;
        case FRONT_RIGHT_WHEEL: pFrame = vehicle->pWheelFrontRight; break;
        case REAR_RIGHT_WHEEL: pFrame = vehicle->pWheelRearRight; break;
        default: break;
    }

    if ( pFrame )
    {
        DWORD dw_GetCurrentAtomicObjectCB = 0x6a0750;
        RwObject * pObject = NULL;

        // Stop GetCurrentAtomicObjectCB from returning null for 'invisible' objects
        MemPut < BYTE > ( 0x6A0758, 0x90 );
        MemPut < BYTE > ( 0x6A0759, 0x90 );
        RwFrameForAllObjects ( pFrame, ( void * ) dw_GetCurrentAtomicObjectCB, &pObject );
        MemPut < BYTE > ( 0x6A0758, 0x74 );
        MemPut < BYTE > ( 0x6A0759, 0x06 );

        if ( pObject ) pObject->flags = ( bVisible ) ? 4 : 0;
    }
}


bool CVehicleSA::IsHeliSearchLightVisible ( void )
{
    // See CHeli::PreRender
    DWORD dwThis = ( DWORD ) GetInterface ();
    return * ( bool * ) ( dwThis + 2577 );
}


void CVehicleSA::SetHeliSearchLightVisible ( bool bVisible )
{
    // See CHeli::PreRender
    DWORD dwThis = ( DWORD ) GetInterface ();
    MemPutFast < bool > ( dwThis + 2577, bVisible );
}


CColModel * CVehicleSA::GetSpecialColModel ( void )
{
    CVehicleSAInterface * vehicle = (CVehicleSAInterface *)this->GetInterface();
    if ( vehicle->m_nSpecialColModel != 0xFF )
    {
        CColModelSAInterface * pSpecialColModels = ( CColModelSAInterface * ) VAR_CVehicle_SpecialColModels;
        CColModelSAInterface * pColModelInterface = &pSpecialColModels [ vehicle->m_nSpecialColModel ];
        if ( pColModelInterface )
        {
            CColModel * pColModel = new CColModelSA ( pColModelInterface );
            return pColModel;
        }
    }
    return NULL;
}


bool CVehicleSA::UpdateMovingCollision ( float fAngle )
{
    // If we dont have a driver, use the local player for this function
    // It will check a few key-states which shouldn't make any difference as we've specified an angle.
    CVehicleSAInterface * vehicle = (CVehicleSAInterface *)this->GetInterface();
    CPedSAInterface * pDriver = vehicle->pDriver;    
    if ( !pDriver )
    {
        CPed * pLocalPed = pGame->GetPools ()->GetPedFromRef ( 1 );
        if ( pLocalPed ) vehicle->pDriver = ( CPedSAInterface * ) pLocalPed->GetInterface ();
    }

    bool bReturn;
    DWORD dwThis = ( DWORD ) GetInterface ();
    DWORD dwFunc = FUNC_CAutomobile__UpdateMovingCollision;
    _asm
    {
        mov     ecx, dwThis
        push    fAngle
        call    dwFunc
        mov     bReturn, al
    }

    // Restore our driver
    vehicle->pDriver = pDriver;

    return bReturn;
}

void* CVehicleSA::GetPrivateSuspensionLines ( void )
{
    if ( m_pSuspensionLines == NULL )
    {
        CModelInfo* pModelInfo = pGame->GetModelInfo ( GetModelIndex() );
        CColDataSA* pColData = pModelInfo->GetInterface ()->m_pColModel->pColData;

        m_pSuspensionLines = new BYTE[pColData->ucNumWheels * 0x20];
    }

    return m_pSuspensionLines;
}

void CVehicleSA::CopyGlobalSuspensionLinesToPrivate ( void )
{
    CModelInfo* pModelInfo = pGame->GetModelInfo ( GetModelIndex () );
    CColDataSA* pColData = pModelInfo->GetInterface ()->m_pColModel->pColData;
    if ( pColData->pSuspensionLines )
        memcpy ( GetPrivateSuspensionLines (), pColData->pSuspensionLines, pColData->ucNumWheels * 0x20 );
}

void CVehicleSA::RecalculateSuspensionLines ( void )
{
    CVehicleSAInterface* pInt = GetVehicleInterface ();
    DWORD dwModel = GetModelIndex ();
    CModelInfo* pModelInfo = pGame->GetModelInfo ( dwModel );

    // Trains (Their trailers do as well!) and boats crash obviously.
    if ( pModelInfo->IsBoat () || pModelInfo->IsTrain () || dwModel == 571 || dwModel == 570 || dwModel == 569 || dwModel == 590 || dwModel == 557 || dwModel == 444 || dwModel == 556 || dwModel == 573 )
        return;

	// Tell it GTA
	GetVehicleInterface()->SetupSuspension();

    CopyGlobalSuspensionLinesToPrivate ();
}

void* CVehicleSAInterface::operator new( size_t )
{
    return (*ppVehiclePool)->Allocate();
}

void CVehicleSAInterface::operator delete( void *ptr )
{
    (*ppVehiclePool)->Free( (CVehicleSAInterface*)ptr );
}