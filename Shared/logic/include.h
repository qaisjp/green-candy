/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        Shared/logic/include.h
*  PURPOSE:     Server/Client logic master include
*  DEVELOPERS:  The_GTA <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#ifndef _SHARED_LOGIC_INCLUDE_
#define _SHARED_LOGIC_INCLUDE_

#include "networking/NetworkStruct.h"
#ifdef MTA_CLIENT
#include "networking/NetworkRakNet.h"
#endif //_KILLFRENZY
#include <RenderWare_shared.h>
#include "LuaCommon.h"
#include <luafile.h>
#include <luafilesystem.h>
#include <luamd5.h>
#include <luabitlib.h>
#include "Logger.h"
#include "ScriptDebugging.h"
#include "LuaFunctionRef.h"
#include "LuaClass.h"
#include "LuaElement.h"
#include "RegisteredCommands.h"
#include "Events.h"
#include "LuaTimer.h"
#include "LuaMatrix.h"
#include "LuaTimerManager.h"
#include "LuaMain.h"
#include "LuaManager.h"
#include "Resource.h"
#include "ResourceManager.h"
#include "LuaFunctionDefs.h"
#include "LuaFunctionDefs.Resources.h"
#include <luaevent.h>

#endif //_SHARED_LOGIC_INCLUDE_