/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.0
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        game_sa/CGameSA.cpp
*  PURPOSE:     Base game logic handling
*  DEVELOPERS:  Ed Lyons <eai@opencoding.net>
*               Christian Myhre Lundheim <>
*               Jax <>
*               Cecill Etheredge <ijsf@gmx.net>
*               Stanislav Bobrov <lil_toady@hotmail.com>
*               Alberto Alonso <rydencillo@gmail.com>
*               Sebas Lamers <sebasdevelopment@gmx.com>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#include "StdInc.h"
#include "SharedUtil.hpp"

unsigned long* CGameSA::VAR_SystemTime;
unsigned long* CGameSA::VAR_IsAtMenu;
unsigned long* CGameSA::VAR_IsGameLoaded;
bool* CGameSA::VAR_GamePaused;
bool* CGameSA::VAR_IsForegroundWindow;
unsigned long* CGameSA::VAR_SystemState;
void* CGameSA::VAR_StartGame;
bool* CGameSA::VAR_IsNastyGame;
float* CGameSA::VAR_TimeScale;
float* CGameSA::VAR_FPS;
float* CGameSA::VAR_OldTimeStep;
float* CGameSA::VAR_TimeStep;
unsigned long* CGameSA::VAR_Framelimiter;

CGameSA::CGameSA()
{
    m_bAsyncSettingsDontUse = false;
    m_bAsyncSettingsEnabled = false;
    m_bAsyncScriptEnabled = false;
    m_bAsyncScriptForced = false;
    m_bASyncLoadingSuspended = false;
    m_iCheckStatus = 0;

    // Unprotect all of the GTASA code at once and leave it that way
    DWORD oldProt;
    VirtualProtect((LPVOID)0x401000, 0x4A3000, PAGE_EXECUTE_READWRITE, &oldProt);

    // Initialize the offsets
    eGameVersion version = FindGameVersion ();
    switch ( version )
    {
    case VERSION_EU_10: COffsets::Initialize10EU (); break;
    case VERSION_US_10: COffsets::Initialize10US (); break;
    case VERSION_11:    COffsets::Initialize11 (); break;
    case VERSION_20:    COffsets::Initialize20 (); break;
    }

    // Set the model ids for all the CModelInfoSA instances
    for ( int i = 0; i < MODELINFO_MAX; i++ )
    {
        ModelInfo [i].SetModelID ( i );
    }

    DEBUG_TRACE("CGameSA::CGameSA()");
    m_pAudio                    = new CAudioSA();
    m_pWorld                    = new CWorldSA();
    m_pPools                    = new CPoolsSA();
    m_pClock                    = new CClockSA();
    m_pRadar                    = new CRadarSA();
    m_pCamera                   = new CCameraSA((CCameraSAInterface *)CLASS_CCamera);
    m_pCoronas                  = new CCoronasSA();
    m_pCheckpoints              = new CCheckpointsSA();
    m_pPickups                  = new CPickupsSA();
    m_pExplosionManager         = new CExplosionManagerSA();
    m_pHud                      = new CHudSA();
    m_pFireManager              = new CFireManagerSA();
    m_p3DMarkers                = new C3DMarkersSA();
    m_pPad                      = new CPadSA((CPadSAInterface *)CLASS_CPad);
    m_pTheCarGenerators         = new CTheCarGeneratorsSA();
    m_pCAERadioTrackManager     = new CAERadioTrackManagerSA();
    m_pWeather                  = new CWeatherSA();
    m_pMenuManager              = new CMenuManagerSA();
    m_pText                     = new CTextSA();
    m_pStats                    = new CStatsSA();
    m_pFont                     = new CFontSA();
    m_pPathFind                 = new CPathFindSA();
    m_pPopulation               = new CPopulationSA();
    m_pTaskManagementSystem     = new CTaskManagementSystemSA();
    m_pSettings                 = new CSettingsSA();
    m_pCarEnterExit             = new CCarEnterExitSA();
    m_pControllerConfigManager  = new CControllerConfigManagerSA();
    m_pProjectileInfo           = new CProjectileInfoSA();
    m_pRenderWare               = new CRenderWareSA( version );
    m_pExtensionManager         = new CRwExtensionManagerSA();
    m_pTextureManager           = new CTextureManagerSA();
    m_pHandlingManager          = new CHandlingManagerSA ();
    m_pEventList                = new CEventListSA();
    m_pGarages                  = new CGaragesSA ( (CGaragesSAInterface *)CLASS_CGarages );
    m_pTasks                    = new CTasksSA ( (CTaskManagementSystemSA*)m_pTaskManagementSystem );
    m_pAnimManager              = new CAnimManagerSA;
    m_pStreaming                = new CStreamingSA;
    m_pVisibilityPlugins        = new CVisibilityPluginsSA;
    m_pKeyGen                   = new CKeyGenSA;
    m_pRopes                    = new CRopesSA;
    m_pParticleSystem           = new CParticleSystemSA();
    m_pFx                       = new CFxSA ( (CFxSAInterface *)CLASS_CFx );
    m_pWaterManager             = new CWaterManagerSA ();

    // :D
    Transformation_Init();

    // Normal weapon types (WEAPONSKILL_STD)
    for ( int i = 0; i < NUM_WeaponInfosStdSkill; i++)
        WeaponInfos[i] = new CWeaponInfoSA((CWeaponInfoSAInterface *)(ARRAY_WeaponInfo + i*CLASSSIZE_WeaponInfo), (eWeaponType)(WEAPONTYPE_PISTOL + i));

    // Extra weapon types for skills (WEAPONSKILL_POOR,WEAPONSKILL_PRO,WEAPONSKILL_SPECIAL)
    int index;
    for ( int skill = 0; skill < 3 ; skill++ )
    {
        for ( int i = 0; i < NUM_WeaponInfosOtherSkill; i++ )
        {
            index = NUM_WeaponInfosStdSkill + skill*NUM_WeaponInfosOtherSkill + i;
            WeaponInfos[index] = new CWeaponInfoSA((CWeaponInfoSAInterface *)(ARRAY_WeaponInfo + index*CLASSSIZE_WeaponInfo), (eWeaponType)(WEAPONTYPE_PISTOL + i));
        }
    }

    m_pPlayerInfo = new CPlayerInfoSA ( (CPlayerInfoSAInterface *)CLASS_CPlayerInfo );

    // Init cheat name => address map
    m_Cheats [ CHEAT_HOVERINGCARS     ] = new SCheatSA((BYTE *)VAR_HoveringCarsEnabled);
    m_Cheats [ CHEAT_FLYINGCARS       ] = new SCheatSA((BYTE *)VAR_FlyingCarsEnabled);
    m_Cheats [ CHEAT_EXTRABUNNYHOP    ] = new SCheatSA((BYTE *)VAR_ExtraBunnyhopEnabled);
    m_Cheats [ CHEAT_EXTRAJUMP        ] = new SCheatSA((BYTE *)VAR_ExtraJumpEnabled);

    // New cheats for Anticheat
    m_Cheats [ CHEAT_TANKMODE         ] = new SCheatSA((BYTE *)VAR_TankModeEnabled, false);
    m_Cheats [ CHEAT_NORELOAD         ] = new SCheatSA((BYTE *)VAR_NoReloadEnabled, false);
    m_Cheats [ CHEAT_PERFECTHANDLING  ] = new SCheatSA((BYTE *)VAR_PerfectHandling, false);
    m_Cheats [ CHEAT_ALLCARSHAVENITRO ] = new SCheatSA((BYTE *)VAR_AllCarsHaveNitro, false);
    m_Cheats [ CHEAT_BOATSCANFLY      ] = new SCheatSA((BYTE *)VAR_BoatsCanFly, false);
    m_Cheats [ CHEAT_INFINITEOXYGEN   ] = new SCheatSA((BYTE *)VAR_InfiniteOxygen, false);
    m_Cheats [ CHEAT_WALKUNDERWATER   ] = new SCheatSA((BYTE *)VAR_WalkUnderwater, false);
    m_Cheats [ CHEAT_FASTERCLOCK      ] = new SCheatSA((BYTE *)VAR_FasterClock, false);
    m_Cheats [ CHEAT_FASTERGAMEPLAY   ] = new SCheatSA((BYTE *)VAR_FasterGameplay, false);
    m_Cheats [ CHEAT_SLOWERGAMEPLAY   ] = new SCheatSA((BYTE *)VAR_SlowerGameplay, false);
    m_Cheats [ CHEAT_ALWAYSMIDNIGHT   ] = new SCheatSA((BYTE *)VAR_AlwaysMidnight, false);
    m_Cheats [ CHEAT_FULLWEAPONAIMING ] = new SCheatSA((BYTE *)VAR_FullWeaponAiming, false);
    m_Cheats [ CHEAT_INFINITEHEALTH   ] = new SCheatSA((BYTE *)VAR_InfiniteHealth, false);
    m_Cheats [ CHEAT_NEVERWANTED      ] = new SCheatSA((BYTE *)VAR_NeverWanted, false);
    m_Cheats [ CHEAT_HEALTARMORMONEY  ] = new SCheatSA((BYTE *)VAR_HealthArmorMoney, false);

    // Change pool sizes here
    m_pPools->SetPoolCapacity ( TASK_POOL, 5000 );  // Default is 500
    m_pPools->SetPoolCapacity ( OBJECT_POOL, 700 );  // Default is 350
    m_pPools->SetPoolCapacity ( EVENT_POOL, 5000 );
}

CGameSA::~CGameSA()
{
    // Destroy the player
    delete m_pPlayerInfo;
    
    for ( int i = 0; i < NUM_WeaponInfosTotal; i++ )
    {
        delete reinterpret_cast < CWeaponInfoSA* > ( WeaponInfos [i] );
    }

    delete m_pFx;
    delete m_pParticleSystem;
    delete m_pRopes;
    delete m_pKeyGen;
    delete m_pVisibilityPlugins;
    delete m_pStreaming;
    delete m_pAnimManager;
    delete m_pTasks;
    delete m_pTextureManager;
    delete m_pRenderWare;
    delete m_pTaskManagementSystem;
    delete m_pHandlingManager;
    delete m_pPopulation;
    delete m_pPathFind;
    delete m_pFont;
    delete m_pStats;
    delete m_pText;
    delete m_pMenuManager;
    delete m_pWeather;
    delete m_pCAERadioTrackManager;
    delete m_pTheCarGenerators;
    delete m_pPad;
    delete m_p3DMarkers;
    delete m_pFireManager;
    delete m_pHud;
    delete m_pExplosionManager;
    delete m_pPickups;
    delete m_pCheckpoints;
    delete m_pCoronas;
    delete m_pCamera;
    delete m_pRadar;
    delete m_pClock;
    delete m_pPools;
    delete m_pWorld;
    delete m_pAudio;

    // Dump any memory leaks if DETECT_LEAK is defined
#ifdef DETECT_LEAKS    
    DumpUnfreed();
#endif
}

CWeaponInfoSA* CGameSA::GetWeaponInfo( eWeaponType weapon, eWeaponSkill skill )
{ 
    DEBUG_TRACE("CWeaponInfo * CGameSA::GetWeaponInfo(eWeaponType weapon)");
    
    if ( (skill == WEAPONSKILL_STD && weapon >= WEAPONTYPE_UNARMED && weapon < WEAPONTYPE_LAST_WEAPONTYPE) ||
         (skill != WEAPONSKILL_STD && weapon >= WEAPONTYPE_PISTOL && weapon <= WEAPONTYPE_TEC9) )
    {
        int offset = 0;

        switch ( skill )
        {
        case WEAPONSKILL_STD:
            offset = 0;
            break;
        case WEAPONSKILL_POOR:
            offset = NUM_WeaponInfosStdSkill - WEAPONTYPE_PISTOL;
            break;
        case WEAPONSKILL_PRO:
            offset = NUM_WeaponInfosStdSkill + NUM_WeaponInfosOtherSkill - WEAPONTYPE_PISTOL;
            break;
        case WEAPONSKILL_SPECIAL:
            offset = NUM_WeaponInfosStdSkill + 2*NUM_WeaponInfosOtherSkill - WEAPONTYPE_PISTOL;
            break;
        default:
            break;
        }
        return WeaponInfos[offset + weapon]; 
    }
    return NULL; 
}

void CGameSA::Pause( bool bPaused )
{
    *VAR_GamePaused = bPaused;
}

bool CGameSA::IsPaused()
{
    return *VAR_GamePaused;
}

bool CGameSA::IsInForeground()
{
    return *VAR_IsForegroundWindow;
}

CModelInfo* CGameSA::GetModelInfo( unsigned short model )
{ 
    DEBUG_TRACE("CModelInfo* CGameSA::GetModelInfo( unsigned short model )");

    if ( model > MAX_MODELS - 1 )
        return NULL;

    if ( !ModelInfo[model].IsValid() )
        return NULL;

    return &ModelInfo[model];
}

/**
 * Starts the game
 * \todo make addresses into constants
 */
void CGameSA::StartGame()
{
    DEBUG_TRACE("void CGameSA::StartGame()");

    this->SetSystemState(GS_INIT_PLAYING_GAME);
    MemPutFast < BYTE > ( 0xB7CB49, 0 );
    MemPutFast < BYTE > ( 0xBA67A4, 0 );
}

/**
 * Sets the part of the game loading process the game is in.
 * @param dwState DWORD containing a valid state 0 - 9
 */
void CGameSA::SetSystemState( eSystemState State )
{
    DEBUG_TRACE("void CGameSA::SetSystemState( eSystemState State )");
    *VAR_SystemState = (DWORD)State;
}

eSystemState CGameSA::GetSystemState()
{
    DEBUG_TRACE("eSystemState CGameSA::GetSystemState()");
    return (eSystemState)*VAR_SystemState;
}

bool CGameSA::InitLocalPlayer()
{
    DEBUG_TRACE("BOOL CGameSA::InitLocalPlayer(  )");

    return GetPools()->GetPedFromRef( 1 ) != NULL;
}

float CGameSA::GetGravity()
{
    return *(float*)0x863984;
}

void CGameSA::SetGravity( float fGravity )
{
   *(float*)0x863984 = fGravity;
}

float CGameSA::GetGameSpeed()
{
    return *(float*)0xB7CB64;
}

void CGameSA::SetGameSpeed( float fSpeed )
{
    *(float*)0xB7CB64 = fSpeed;
}

// this prevents some crashes (respawning mainly)
void CGameSA::DisableRenderer( bool bDisabled )
{
    // ENABLED:
    // 0053DF40   D915 2C13C800    FST DWORD PTR DS:[C8132C]
    // DISABLED:
    // 0053DF40   C3               RETN

    if ( bDisabled )
        *(unsigned char*)0x53DF40 = 0xC3;
    else
        *(unsigned char*)0x53DF40 = 0xD9;
}

void CGameSA::SetRenderHook( InRenderer* pInRenderer )
{
    if ( pInRenderer )
        HookInstall ( (DWORD)FUNC_CDebug_DebugDisplayTextBuffer, (DWORD)pInRenderer, 6 );
    else
        *(unsigned char*)FUNC_CDebug_DebugDisplayTextBuffer = 0xC3;
}

void CGameSA::TakeScreenshot( char *szFileName )
{
    DWORD dwFunc = FUNC_JPegCompressScreenToFile;
    _asm
    {
        mov     eax, CLASS_RwCamera
        mov     eax, [eax]
        push    szFileName
        push    eax
        call    dwFunc
        add     esp,8
    }
}

void CGameSA::Reset()
{
    // Things to do if the game was loaded
    if ( GetSystemState() != GS_PLAYING_GAME )
        return;

    // Extinguish all fires
    m_pFireManager->ExtinguishAllFires();

    // Restore camera stuff
    m_pCamera->Restore();
    m_pCamera->SetFadeColor( 0, 0, 0 );
    m_pCamera->Fade( 0, FADE_OUT );

    Pause( false );        // We don't have to pause as the fadeout will stop the sound. Pausing it will make the fadein next start ugly
    m_pHud->Disable( false );

    DisableRenderer( false );

    // Restore the HUD
    m_pHud->Disable( false );
    m_pHud->DisableAll( false );
}

void CGameSA::Initialize()
{
    // Initialize garages
    m_pGarages->Initialize();
    SetupSpecialCharacters ();

    // *Sebas* Hide the GTA:SA Main menu.
    MemPutFast < BYTE > ( CLASS_CMenuManager+0x5C, 0 );
}

void CGameSA::OnPreFrame()
{
    switch( GetSystemState() )
    {
    case GS_PLAYING_GAME:
        // Update control with current pad
        m_pPad->SetState( m_pPad->GetState() );
        break;
    }
}

void CGameSA::OnFrame()
{
    switch( GetSystemState() )
    {
    case GS_PLAYING_GAME:
        // Update local player's control
        GetPadManager()->UpdateLocalJoypad( *m_pPlayerInfo->GetPlayerPed() );

        // Pulse the peds
        unsigned int n;

        for ( n=0; n<MAX_PEDS; n++ )
        {
            CPedSAInterface *ped = (*ppPedPool)->Get( n );

            if ( !ped )
                continue;

            ped->OnFrame();
        }

        break;
    }
}

eGameVersion CGameSA::GetGameVersion()
{
    return m_eGameVersion;
}

eGameVersion CGameSA::FindGameVersion()
{
    unsigned char ucA = *(unsigned char*)0x748ADD;
    unsigned char ucB = *(unsigned char*)0x748ADE;

    if ( ucA == 0xFF && ucB == 0x53 )
        m_eGameVersion = VERSION_US_10;
    else if ( ucA == 0x0F && ucB == 0x84 )
        m_eGameVersion = VERSION_EU_10;
    else if ( ucA == 0xFE && ucB == 0x10 )
        m_eGameVersion = VERSION_11;
    else
        m_eGameVersion = VERSION_UNKNOWN;

    return m_eGameVersion;
}

float CGameSA::GetFPS()
{
    return *VAR_FPS;
}

float CGameSA::GetTimeStep()
{
    return *VAR_TimeStep;
}

float CGameSA::GetOldTimeStep()
{
    return *VAR_OldTimeStep;
}

float CGameSA::GetTimeScale()
{
    return *VAR_TimeScale;
}

void CGameSA::SetTimeScale( float fTimeScale )
{
    *VAR_TimeScale = fTimeScale;
}

unsigned char CGameSA::GetBlurLevel()
{
    return *(unsigned char*)0x8D5104;
}

void CGameSA::SetBlurLevel( unsigned char ucLevel )
{
    *(unsigned char*)0x8D5104 = ucLevel;
}

unsigned long CGameSA::GetMinuteDuration()
{
    return *(unsigned long*)0xB7015C;
}

void CGameSA::SetMinuteDuration( unsigned long ulTime )
{
    *(unsigned long*)0xB7015C = ulTime;
}

bool CGameSA::IsCheatEnabled( const char* szCheatName )
{
    std::map <std::string, SCheatSA*>::iterator it = m_Cheats.find ( szCheatName );

    if ( it == m_Cheats.end () )
        return false;

    return *(it->second->m_byAddress) != 0;
}

bool CGameSA::SetCheatEnabled( const char* szCheatName, bool bEnable )
{
    std::map < std::string, SCheatSA* >::iterator it = m_Cheats.find ( szCheatName );

    if ( it == m_Cheats.end () )
        return false;
    if ( !it->second->m_bCanBeSet )
        return false;

    MemPutFast < BYTE > ( it->second->m_byAddress, bEnable );
    it->second->m_bEnabled = bEnable;
    return true;
}

void CGameSA::ResetCheats()
{
    std::map < std::string, SCheatSA* >::iterator it;
    for ( it = m_Cheats.begin (); it != m_Cheats.end (); it++ ) {
        if ( it->second->m_byAddress > (BYTE*)0x8A4000 )
            MemPutFast < BYTE > ( it->second->m_byAddress, 0 );
        else
            MemPut < BYTE > ( it->second->m_byAddress, 0 );
        it->second->m_bEnabled = false;
    }
}
bool CGameSA::PerformChecks()
{
    std::map < std::string, SCheatSA* >::iterator it;
    for ( it = m_Cheats.begin (); it != m_Cheats.end (); it++ ) {
        if (*(it->second->m_byAddress) != BYTE(it->second->m_bEnabled))
            return false;
    }
    return true;
}

bool CGameSA::VerifySADataFileNames()
{
    return !strcmp( *(char**)0x5B65AE, "DATA\\CARMODS.DAT" ) &&
           !strcmp( *(char**)0x5BD839, "DATA" ) &&
           !strcmp( *(char**)0x5BD84C, "HANDLING.CFG" ) &&
           !strcmp( *(char**)0x5BEEE8, "DATA\\melee.dat" ) &&
           !strcmp( *(char**)0x5B925B, "DATA\\OBJECT.DAT" ) &&
           !strcmp( *(char**)0x55D0FC, "data\\surface.dat" ) &&
           !strcmp( *(char**)0x55F2BB, "data\\surfaud.dat" ) &&
           !strcmp( *(char**)0x55EB9E, "data\\surfinfo.dat" ) &&
           !strcmp( *(char**)0x6EAEF8, "DATA\\water.dat" ) &&
           !strcmp( *(char**)0x6EAEC3, "DATA\\water1.dat" ) &&
           !strcmp( *(char**)0x5BE686, "DATA\\WEAPON.DAT" );
}

void CGameSA::SetAsyncLoadingFromSettings ( bool bSettingsDontUse, bool bSettingsEnabled )
{
    m_bAsyncSettingsDontUse = bSettingsDontUse;
    m_bAsyncSettingsEnabled = bSettingsEnabled;
}

void CGameSA::SetAsyncLoadingFromScript ( bool bScriptEnabled, bool bScriptForced )
{
    m_bAsyncScriptEnabled = bScriptEnabled;
    m_bAsyncScriptForced = bScriptForced;
}

void CGameSA::SuspendASyncLoading ( bool bSuspend )
{
    m_bASyncLoadingSuspended = bSuspend;
}

bool CGameSA::IsASyncLoadingEnabled ( bool bIgnoreSuspend )
{
    if ( m_bASyncLoadingSuspended && !bIgnoreSuspend )
        return false;

    if ( m_bAsyncScriptForced || m_bAsyncSettingsDontUse )
        return m_bAsyncScriptEnabled;

    return m_bAsyncSettingsEnabled;
}

void CGameSA::SetupSpecialCharacters ( void )
{
    ModelInfo[1].MakePedModel ( "TRUTH" );
    ModelInfo[2].MakePedModel ( "MACCER" );
    //ModelInfo[190].MakePedModel ( "BARBARA" );
    //ModelInfo[191].MakePedModel ( "HELENA" );
    //ModelInfo[192].MakePedModel ( "MICHELLE" );
    //ModelInfo[193].MakePedModel ( "KATIE" );
    //ModelInfo[194].MakePedModel ( "MILLIE" );
    //ModelInfo[195].MakePedModel ( "DENISE" );
    ModelInfo[265].MakePedModel ( "TENPEN" );   
    ModelInfo[266].MakePedModel ( "PULASKI" );
    ModelInfo[267].MakePedModel ( "HERN" );
    ModelInfo[268].MakePedModel ( "DWAYNE" );
    ModelInfo[269].MakePedModel ( "SMOKE" );
    ModelInfo[270].MakePedModel ( "SWEET" );
    ModelInfo[271].MakePedModel ( "RYDER" );
    ModelInfo[272].MakePedModel ( "FORELLI" );
    ModelInfo[290].MakePedModel ( "ROSE" );
    ModelInfo[291].MakePedModel ( "PAUL" );
    ModelInfo[292].MakePedModel ( "CESAR" );
    ModelInfo[293].MakePedModel ( "OGLOC" );
    ModelInfo[294].MakePedModel ( "WUZIMU" );
    ModelInfo[295].MakePedModel ( "TORINO" );
    ModelInfo[296].MakePedModel ( "JIZZY" );
    ModelInfo[297].MakePedModel ( "MADDOGG" );
    ModelInfo[298].MakePedModel ( "CAT" );
    ModelInfo[299].MakePedModel ( "CLAUDE" );
    ModelInfo[300].MakePedModel ( "RYDER2" );
    ModelInfo[301].MakePedModel ( "RYDER3" );
    ModelInfo[302].MakePedModel ( "EMMET" );
    ModelInfo[303].MakePedModel ( "ANDRE" );
    ModelInfo[304].MakePedModel ( "KENDL" );
    ModelInfo[305].MakePedModel ( "JETHRO" );
    ModelInfo[306].MakePedModel ( "ZERO" );
    ModelInfo[307].MakePedModel ( "TBONE" );
    ModelInfo[308].MakePedModel ( "SINDACO" );
    ModelInfo[309].MakePedModel ( "JANITOR" );
    ModelInfo[310].MakePedModel ( "BBTHIN" );
    ModelInfo[311].MakePedModel ( "SMOKEV" );
    ModelInfo[312].MakePedModel ( "PSYCHO" );
    /* Hot-coffee only models
    ModelInfo[313].MakePedModel ( "GANGRL2" );
    ModelInfo[314].MakePedModel ( "MECGRL2" );
    ModelInfo[315].MakePedModel ( "GUNGRL2" );
    ModelInfo[316].MakePedModel ( "COPGRL2" );
    ModelInfo[317].MakePedModel ( "NURGRL2" );
    */
}

// Well, has it?
bool CGameSA::HasCreditScreenFadedOut()
{
    return ( GetSystemState() >= GS_FRONTEND ) && ( *(BYTE*)0xBAB320 < 6 );
}

// Ensure replaced/restored textures for models in the GTA map are correct
void CGameSA::FlushPendingRestreamIPL()
{
    CModelInfoSA::StaticFlushPendingRestreamIPL();
}

// Disable VSync by forcing what normally happends at the end of the loading screens
// Note #1: This causes the D3D device to be reset after the next frame
// Note #2: Some players do not need this to disable VSync. (Possibly because their video card driver settings override it somewhere)
void CGameSA::DisableVSync()
{
   *(unsigned char*)0xBAB318 = 0;
}

void ForEachBlock( void *ptr, unsigned int count, size_t blockSize, void (*callback)( void *block ) )
{
    while ( count-- )
    {
        callback( ptr );

        (unsigned char*)ptr += blockSize;
    }
}