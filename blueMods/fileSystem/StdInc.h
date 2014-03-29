/*********************************************************
*
*  Multi Theft Auto: San Andreas - Deathmatch
*
*  ml_base, External lua add-on module
*
*  Copyright © 2003-2008 MTA.  All Rights Reserved.
*
*  Grand Theft Auto is © 2002-2003 Rockstar North
*
*  THE FOLLOWING SOURCES ARE PART OF THE MULTI THEFT
*  AUTO SOFTWARE DEVELOPMENT KIT AND ARE RELEASED AS
*  OPEN SOURCE FILES. THESE FILES MAY BE USED AS LONG
*  AS THE DEVELOPER AGREES TO THE LICENSE THAT IS
*  PROVIDED WITH THIS PACKAGE.
*
*********************************************************/

/** MODULE SPECIFIC INFORMATION **/
#define MODULE_NAME         "Eir/GREEN FileSystem"
#define MODULE_AUTHOR       "The_GTA"
#define MODULE_VERSION      1.3

#pragma warning(disable: 4996)

#define NOMINMAX

#include "../../Shared/core/Common.h"
#include "Common.h"
#include <CFileSystem.h>
#include "luaclass.h"
#include "../../lua_interpret/luafile.h"
#include "../../lua_interpret/luafilesystem.h"
#include "CFunctions.h"
#include "include/ILuaModuleManager.h"

extern CFileSystem *fileSystem;
