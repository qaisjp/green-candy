/*****************************************************************************
*
*  PROJECT:     Lua Interpreter
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        luafilesystem.h
*  PURPOSE:     Lua filesystem access
*  DEVELOPERS:  The_GTA <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#ifndef _FILESYSTEMLIB_
#define _FILESYSTEMLIB_

#define LUACLASS_FILETRANSLATOR     137

void luafsys_pushroot( lua_State *L, CFileTranslator *root );
void luafilesystem_open( lua_State *L );

#endif //_FILESYSTEMLIB_