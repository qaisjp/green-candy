#pragma message("Compiling precompiled header.\n")

// Pragmas
#pragma warning (disable:4250)

#include <windows.h>
#include <stdio.h>
#include <assert.h>

#include <algorithm>
#include <list>
#include <map>
#include <string>
#include <vector>

// SDK includes
#define MTA_CLIENT
#include "SharedUtil.h"
#include <core/CCoreInterface.h>
#include <net/CNet.h>
#include <game/CGame.h>
#include <../version.h>

// Multiplayer includes
#include <../game_sa/RenderWare.h>
#include <../game_sa/CPtrNodeSA.h>
#include <../game_sa/CTransformationSA.h>
#include <../game_sa/CTasksSA.h>
#include <../game_sa/CTaskManagerSA.h>
#include <../game_sa/CPadSA.h>
#include <../game_sa/CStreamingSA.h>
#include <../game_sa/CPoolsSA.h>
#include <../game_sa/CStreamingSA.h>
#include <../game_sa/CModelInfoSA.h>
#include <../game_sa/CWeaponInfoSA.h>
#include "multiplayersa_init.h"
#include "multiplayer_keysync.h"
#include "multiplayer_hooksystem.h"
#include "multiplayer_shotsync.h"
#include "CMultiplayerSA.h"
#include "COffsets.h"
#include "CPopulationSA.h"
#include "CRemoteDataSA.h"
