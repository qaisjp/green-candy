/*****************************************************************************
*
*  PROJECT:     Lua Interpreter
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        StdInc.h
*  PURPOSE:     Global header
*  DEVELOPERS:  The_GTA <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#ifndef _MAIN_
#define _MAIN_

#pragma warning(disable: 4996)

#define MEM_INTERRUPT( x )  { if ( !( x ) ) __asm int 3 }
//#define USE_HEAP_DEBUGGING
//#define USE_FULL_PAGE_HEAP
//#define PAGE_HEAP_INTEGRITY_CHECK
#include <debugsdk/DbgHeap.h>

#include <windows.h>
#include <DbgHelp.h>
#include <time.h>
#include "lauxlib.h"
#include "lualib.h"
#include <sstream>
#include <iostream>
#include <sys/stat.h>
#include <SharedUtil.h>
#include <CVector.h>
#include <RenderWare_shared.h>
#include <CQuat.h>
#include <core/CFileSystemInterface.h>
#include <core/CFileSystem.h>
#include <logic/include.h>
#include "derived/include.h"
#include "utils/include.h"
#include "dxElements/include.h"
#include "benchmarks/include.h"
#include "win32/include.h"
#include "glDriver/gl_main.h"
#include "lfreetype/lft_main.h"
#include <lua/CLuaFunctionParseHelpers.h>
#include <CScriptArgReader.h>
#include <google/dense_hash_map>

extern CFileSystem *fileSystem;
extern CFileTranslator *modFileRoot;

extern lua_State *userLuaState;

#endif //_MAIN_