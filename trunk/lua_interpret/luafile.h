/*****************************************************************************
*
*  PROJECT:     Lua Interpreter
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        luafile.h
*  PURPOSE:     File environment header
*  DEVELOPERS:  The_GTA <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#ifndef _FILELIB_
#define _FILELIB_

#define LUACLASS_FILE   136

int luaconstructor_file( lua_State *lua );
void luafile_open( lua_State *lua );

#endif //_FILELIB_