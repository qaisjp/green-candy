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
#include <core/CFileSystemInterface.h>
#include <core/CFileSystem.h>
#include "luafile.h"
#include "luafilesystem.h"
#include "luamd5.h"

static inline void luaJ_extend( lua_State *L, int idx, int nargs )
{
    ILuaClass& j = *lua_refclass( L, idx );

    int offcl = -nargs - 1;

    // Make it class root
    j.PushEnvironment( L );
    lua_setfenv( L, offcl - 1 );

    lua_pushvalue( L, idx );

    if ( offcl != -1 )
        lua_insert( L, offcl );

    lua_call( L, nargs + 1, 0 );
}

extern CFileSystem *fileSystem;

#endif //_MAIN_