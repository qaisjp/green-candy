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
#include "gamesa_renderware.h"

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
        m_autoFlags |= AUTOMOBILE_TAXILIGHTS;

    m_autoFlags |= 0x0030;

    SetModelIndex( model );

    m_handling = &m_OriginalHandlingData[ handling ];
    m_handlingFlags = m_handling->uiHandlingFlags;

    if ( ( m_handlingFlags & 0x10000 ) && ( rand() & 3 ) )
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
    GetRwObject()->ScanAtomicHierarchy( m_components, (unsigned int)NUM_VEHICLE_COMPONENTS );
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
        else if ( m_audio.m_soundType == 2 )
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

CAutomobileSA::CAutomobileSA( CAutomobileSAInterface *veh ) : CVehicleSA( veh ), m_HeadLightColor( SColorRGBA( 255, 255, 255, 255 ) )
{
    m_damageManager = new CDamageManagerSA( GetInterface(), &GetInterface()->m_damage );

    // Unlock doors as they spawn randomly with locked doors
    LockDoors( false );

    m_HeadLightColor = SColorRGBA( 255, 255, 255, 255 );
    m_swingingDoorsAllowed = false;

    // Initialize doors depending on the vtable
    for ( unsigned int i = 0; i < MAX_DOORS; i++ )
        m_doors[i] = new CDoorSA( &GetInterface()->m_doors[i] );

    // Privatise our suspension lines
    CBaseModelInfoSAInterface *info = ppModelInfo[veh->m_model];
    CColDataSA *data = info->m_pColModel->pColData;

    m_suspensionLines = new char [data->ucNumWheels * 0x20];
    memcpy( m_suspensionLines, data->pSuspensionLines, data->ucNumWheels * 0x20 );

    // Only create component interfaces if necessary
    for ( unsigned int n = 0; n < NUM_VEHICLE_COMPONENTS; n++ )
        m_components[n] = NULL;
}

CAutomobileSA::~CAutomobileSA()
{
    // Destroy available components
    for ( unsigned int n = 0; n < NUM_VEHICLE_COMPONENTS; n++ )
        delete m_components[n];

    delete m_damageManager;
    delete m_suspensionLines;

    DWORD dwThis = (DWORD)m_pInterface;
    DWORD dwFunc = 0x6D2460;        // CVehicle::ExtinguishCarFire
    _asm
    {
        mov     ecx, dwThis
        call    dwFunc
    }

    for ( unsigned int n = 0; n < MAX_DOORS; n++ )
        delete m_doors[n];
}

CDoorSA* CAutomobileSA::GetDoor( unsigned char ucDoor )
{
    if ( ucDoor > MAX_DOORS-1 )
        return NULL;

    return m_doors[ucDoor];
}

void CAutomobileSA::ExtinguishCarFire()
{
    DEBUG_TRACE("void CVehicleSA::ExtinguishCarFire()");
    DWORD dwThis = (DWORD) m_pInterface;
    DWORD dwFunc = FUNC_CVehicle_ExtinguishCarFire;

    _asm
    {
        mov     ecx, dwThis
        call    dwFunc
    }
}

void CAutomobileSA::PlaceOnRoadProperly()
{
    DEBUG_TRACE("void CAutomobileSA::PlaceOnRoadProperly()");

    DWORD dwFunc = FUNC_Automobile_PlaceOnRoadProperly;
    DWORD dwAutomobile = (DWORD)GetInterface();

    _asm
    {
        mov     ecx, dwAutomobile
        call    dwFunc
    }
}

// works with firetrucks & tanks
void CAutomobileSA::GetTurretRotation( float& horizontal, float& vertical ) const
{
    DEBUG_TRACE("void CVehicleSA::GetTurretRotation( float& horizontal, float& vertical ) const");

    horizontal = GetInterface()->m_turretHorizontal;
    vertical = GetInterface()->m_turretVertical;
}

void CAutomobileSA::SetTurretRotation( float horizontal, float vertical )
{
    DEBUG_TRACE("void CAutomobileSA::SetTurretRotation( float horizontal, float vertical )");

    GetInterface()->m_turretHorizontal = horizontal;
    GetInterface()->m_turretVertical = vertical;
}

CObject* CAutomobileSA::SpawnFlyingComponent( int i_1, unsigned int ui_2 )
{
    DWORD dwReturn;
    DWORD dwThis = (DWORD)GetInterface();
    DWORD dwFunc = FUNC_CAutomobile__SpawnFlyingComponent;
    _asm
    {
        mov     ecx, dwThis
        push    ui_2
        push    i_1
        call    dwFunc
        mov     dwReturn, eax
    }

    if ( !dwReturn )
        return NULL;

    return pGame->GetPools()->GetObject( (void*)dwReturn );
}

void CAutomobileSA::SetWheelVisibility( eWheels wheel, bool vis )
{
    RwObject *obj;

    switch( wheel )
    {        
    case FRONT_LEFT_WHEEL:      obj = GetInterface()->m_components[VEHICLE_COMP_WHEEL_FL]->GetFirstObject(); break;
    case REAR_LEFT_WHEEL:       obj = GetInterface()->m_components[VEHICLE_COMP_WHEEL_RL]->GetFirstObject(); break;
    case FRONT_RIGHT_WHEEL:     obj = GetInterface()->m_components[VEHICLE_COMP_WHEEL_FR]->GetFirstObject(); break;
    case REAR_RIGHT_WHEEL:      obj = GetInterface()->m_components[VEHICLE_COMP_WHEEL_RR]->GetFirstObject(); break;
    }

    if ( obj )
        obj->SetVisible( vis );
}

bool CAutomobileSA::GetWheelVisibility( eWheels wheel ) const
{
    RwObject *obj;

    switch( wheel )
    {        
    case FRONT_LEFT_WHEEL:      obj = GetInterface()->m_components[VEHICLE_COMP_WHEEL_FL]->GetFirstObject(); break;
    case REAR_LEFT_WHEEL:       obj = GetInterface()->m_components[VEHICLE_COMP_WHEEL_RL]->GetFirstObject(); break;
    case FRONT_RIGHT_WHEEL:     obj = GetInterface()->m_components[VEHICLE_COMP_WHEEL_FR]->GetFirstObject(); break;
    case REAR_RIGHT_WHEEL:      obj = GetInterface()->m_components[VEHICLE_COMP_WHEEL_RR]->GetFirstObject(); break;
    }

    return obj && obj->IsVisible();
}

void CAutomobileSA::SetComponent( unsigned int idx, CRpAtomic *atom )
{
    if ( idx > NUM_VEHICLE_COMPONENTS-1 )
        return;

    RpAtomic *atomic = RpAtomicClone( dynamic_cast <CRpAtomicSA*> ( atom )->GetObject() );

    if ( CVehicleComponentSA *comp = m_components[idx] )
        comp->m_atomic = atomic;

    RwFrame *frame = GetInterface()->m_components[idx];

    if ( RpAtomic *existing = (RpAtomic*)frame->GetFirstObject() )
    {
        existing->RemoveFromFrame();
        existing->RemoveFromClump();
        RpAtomicDestroy( existing );
    }

    atomic->AddToFrame( frame );
    atomic->AddToClump( (RpClump*)GetInterface()->m_rwObject );
}

CVehicleComponent* CAutomobileSA::GetComponent( unsigned int idx )
{
    if ( idx > NUM_VEHICLE_COMPONENTS-1 )
        return NULL;

    CVehicleComponentSA *comp;

    // Cache the component
    if ( comp = m_components[idx] )
        return comp;

    RwFrame *frame = GetInterface()->m_components[idx];

    if ( !frame )
        return NULL;

    RpAtomic *atomic = (RpAtomic*)frame->GetFirstObject();

    if ( !atomic )
        return NULL;

    m_components[idx] = comp = new CVehicleComponentSA( m_components[idx], atomic );
    return comp;
}

void CAutomobileSA::RecalculateHandling()
{
    if ( !m_pHandlingData )
        return;

    m_pHandlingData->Recalculate();
    
    // Recalculate the suspension lines
    RecalculateSuspensionLines();

    // Put it in our interface
    CVehicleSAInterface* pInt = GetInterface ();
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
            m_pHandlingData->SetHandlingFlags( uiHandlingFlags );
        }
    }
    else
    {
        // Flag Enabled?
        if ( uiHandlingFlags & HANDLING_NOS_Flag )
        {
            // Unset the flag
            uiHandlingFlags &= ~HANDLING_NOS_Flag;
            m_pHandlingData->SetHandlingFlags( uiHandlingFlags );
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
            m_pHandlingData->SetHandlingFlags( uiHandlingFlags );
        }
    }
    else
    {
        // Flag Enabled?
        if ( uiHandlingFlags & HANDLING_Hydraulics_Flag )
        {
            // Unset the flag
            uiHandlingFlags &= ~HANDLING_Hydraulics_Flag;
            m_pHandlingData->SetHandlingFlags( uiHandlingFlags );
        }
    }
    pInt->m_handlingFlags = uiHandlingFlags;
    pInt->m_mass = m_pHandlingData->GetInterface ()->fMass;
    pInt->m_turnMass = m_pHandlingData->GetInterface ()->fTurnMass;// * pGame->GetHandlingManager()->GetTurnMassMultiplier();
    pInt->m_centerOfMass = m_pHandlingData->GetInterface()->vecCenterOfMass;
    pInt->m_buoyancyConstant = m_pHandlingData->GetInterface()->fUnknown2;
    /*if (m_pHandlingData->GetInterface()->fDragCoeff >= pGame->GetHandlingManager()->GetBasicDragCoeff())
        GetVehicleInterface ()->fDragCoeff = pGame->GetHandlingManager()->GetBasicDragCoeff();
    else*/
        //pInt->fDragCoeff = m_pHandlingData->GetInterface()->fDragCoeff / 1000 * pGame->GetHandlingManager()->GetDragMultiplier();
}

void CAutomobileSA::RecalculateSuspensionLines()
{
    CColDataSA *data = ppModelInfo[GetInterface()->m_model]->m_pColModel->pColData;

	// Calculate them
    CVehicleSA::RecalculateSuspensionLines();

    // Retrive them
    memcpy( m_suspensionLines, data->pSuspensionLines, data->ucNumWheels * 0x20 );
}

bool CAutomobileSA::UpdateMovingCollision( float fAngle )
{
    // If we dont have a driver, use the local player for this function
    // It will check a few key-states which shouldn't make any difference as we've specified an angle.
    CAutomobileSAInterface *veh = GetInterface();
    CPedSAInterface *driver = veh->m_driver;

    if ( !veh->m_driver )
        veh->m_driver = pGame->GetPlayerInfo()->GetPlayerPed()->GetInterface();

    bool bReturn;
    DWORD dwFunc = FUNC_CAutomobile__UpdateMovingCollision;
    _asm
    {
        mov     ecx,veh
        push    fAngle
        call    dwFunc
        mov     bReturn, al
    }

    // Restore our driver
    veh->m_driver = driver;
    return bReturn;
}