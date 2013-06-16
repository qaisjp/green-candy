#ifndef __MAIN__
#define __MAIN__

#pragma warning(disable: 4996)

// We compile for GTAIII
#define _KILLFRENZY

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <DbgHelp.h>
#include <stdarg.h>
#include <stdio.h>
#include <assert.h>
#include <stdio.h>
#include <sys/stat.h>
#include <time.h>
#define	_USE_MATH_DEFINES
#include <math.h>
#include <string>
#include <sstream>
#include <vector>
#include <google/dense_hash_map>
#include <CVector.h>
#include <CVector2D.h>
#include <SharedUtil.h>
#include <CFileSystemInterface.h>
#include <CFileSystem.h>
#include <logic/include.h>

using namespace Networking;

#include "derived/include.h"
#include <CQuat.h>
#include <lua/CLuaFunctionParseHelpers.h>
#include <CScriptArgReader.h>

#include "d3d8.h"
#include "d3dx8.h"
#include "d3d8_f.h"
#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>
#include "dinput8.h"
#include "d3devt.h"
#include "sound.h"
#include "game.h"
#include "matrix.h"
#include "world.h"
#include "window.h"
#include "console.h"
#include "entity.h"
#include "vehicle.h"
#include "ped.h"
#include "modelinfo.h"
#include "pool.h"
#include "streamer.h"
#include "scm.h"
#include "lua.h"
#include "lua/luaentity.h"
#include "lua/luavehicle.h"
#include "lua/luaped.h"
#include "lua/luaplayer.h"
#include "detours/detours.h"
#define EXIT_FAILURE 1
#define EXIT_SUCCESS 0

#define FUNC_DebugPrintf		0x0059E720

// Initialize
void	Core_Init();
void    Core_InitGame();
void	Core_Destroy();
bool	Core_SetModuleAccessLevel( HMODULE pModule, DWORD dwAccessLevel, DWORD *pOldProt );
bool	Core_ProcessCommand( const std::string& cmdName, std::vector <std::string>& args );
int NEAR	Hook_DebugPrintf( DWORD dwUnk, char *cDebugString, ... );
// Frames
void	Core_FirstFrame();
bool	Core_PreRender();
bool	Core_Render( IDirect3DDevice8 *pD3D );
bool	Core_PostRender();

// Exports
extern unsigned int isCoreLoaded;
extern CFileSystem *fileSystem;
extern CFileTranslator *modFileRoot;

#endif //__MAIN__