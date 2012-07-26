#pragma message("Compiling precompiled header.\n")

// Pragmas
#pragma warning (disable:4409)
#pragma warning (disable:4250)

#include <windows.h>
#include <stdio.h>
#include <assert.h>

#include <algorithm>
#include <list>
#include <map>
#include <set>
#include <string>
#include <vector>

#include <game/RenderWare_shared.h>
#include <CQuat.h>

// SDK includes
#define MTA_CLIENT
#include "SharedUtil.h"
#include <multiplayer/CMultiplayer.h>
#include <core/CCoreInterface.h>
#include <net/CNet.h>
#include <game/CGame.h>
#include <game/CWanted.h>
#include <../version.h>
#include <ijsify.h>

// Game includes
#include "gamesa_init.h"
#include "RenderWare.h"
#include "Common.h"
#include "CPtrNodeSA.h"
#include "CTransformationSA.h"
#include "CClockSA.h"
#include "CFontSA.h"
#include "CRadarSA.h"
#include "CMenuManagerSA.h"
#include "CCheckpointsSA.h"
#include "CAnimBlendAssociationSA.h"
#include "CAnimBlendAssocGroupSA.h"
#include "CAnimBlendHierarchySA.h"
#include "CAnimBlendSequenceSA.h"
#include "CAnimBlendStaticAssociationSA.h"
#include "CAnimBlockSA.h"
#include "CRenderWareSA.h"
#include "CRenderWareExtensionSA.h"
#include "CModelSA.h"
#include "CModelManagerSA.h"
#include "CTexDictionarySA.h"
#include "CTextureManagerSA.h"
#include "CTextureSA.h"
#include "CModelInfoSA.h"
#include "CAtomicModelInfoSA.h"
#include "CClumpModelInfoSA.h"
#include "CDamageManagerSA.h"
#include "CVehicleModelInfoSA.h"
#include "CPedModelInfoSA.h"
#include "CColModelSA.h"
#include "CEntitySA.h"
#include "CParticleSA.h"
#include "CParticleObjectSA.h"
#include "CParticleDataSA.h"
#include "CParticleSystemSA.h"
#include "CTaskManagerSA.h"
#include "CTasksSA.h"
#include "CPedSA.h"
#include "CPlayerPedSA.h"
#include "CCivilianPedSA.h"
#include "CHandlingEntrySA.h"
#include "CVehicleSA.h"
#include "CAutomobileSA.h"
#include "CCarSA.h"
#include "CBikeSA.h"
#include "CBoatSA.h"
#include "CAutomobileTrailerSA.h"
#include "CBicycleSA.h"
#include "CQuadBikeSA.h"
#include "CMonsterTruckSA.h"
#include "CPlaneSA.h"
#include "CHeliSA.h"
#include "CTrailerSA.h"
#include "CTrainSA.h"
#include "CObjectSA.h"
#include "CBuildingSA.h"
#include "CDummySA.h"
#include "CPadSA.h"
#include "CCameraSA.h"
#include "CColPointSA.h"
#include "CPadManagerSA.h"
#include "CRouteSA.h"
#include "CEntryInfoSA.h"
#include "CTaskAllocatorSA.h"
#include "CWorldSA.h"
#include "CCoronasSA.h"
#include "CPickupsSA.h"
#include "CPathFindSA.h"
#include "CWeaponInfoSA.h"
#include "CExplosionManagerSA.h"
#include "CFireManagerSA.h"
#include "CHandlingManagerSA.h"
#include "CHudSA.h"
#include "C3DMarkersSA.h"
#include "CStatsSA.h"
#include "CTheCarGeneratorsSA.h"
#include "CAERadioTrackManagerSA.h"
#include "CWeatherSA.h"
#include "CTextSA.h"
#include "CPedSoundSA.h"
#include "CAudioSA.h"
#include "CPlayerInfoSA.h"
#include "CPopulationSA.h"
#include "CSettingsSA.h"
#include "CCarEnterExitSA.h"
#include "COffsets.h"
#include "CControllerConfigManagerSA.h"
#include "CProjectileInfoSA.h"
#include "CEventSA.h"
#include "CEventListSA.h"
#include "CGaragesSA.h"
#include "CEventDamageSA.h"
#include "CEventGunShotSA.h"
#include "CAnimManagerSA.h"
#include "CStreamingSA.h"
#include "CVisibilityPluginsSA.h"
#include "CKeyGenSA.h"
#include "CRopesSA.h"
#include "CFxSA.h"
#include "HookSystem.h"
#include "CRestartSA.h"
#include "CWaterManagerSA.h"
#include "CPedDamageResponseSA.h"
#include "CPedDamageResponseCalculatorSA.h"
#include "CPedIntelligenceSA.h"
#include "CPoolsSA.h"
#include "CGameSA.h"

extern CGameSA *pGame;
extern CCoreInterface *core;