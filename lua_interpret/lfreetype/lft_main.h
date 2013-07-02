/*****************************************************************************
*
*  PROJECT:     Lua Interpreter
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        lfreetype/lft_main.h
*  PURPOSE:     Freetype management library for MTA:Lua
*  DEVELOPERS:  Martin Turski <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#ifndef _LUA_FREETYPE_
#define _LUA_FREETYPE_

#include <ft2build.h>
#include FT_FREETYPE_H

#define LUACLASS_FREETYPE       44
#define LUACLASS_FREETYPEFACE   45
#define LUACLASS_FREETYPEGLYPH  46

struct LFreeType : public LuaInstance
{
                            LFreeType           ( lua_State *L );
                            ~LFreeType          ( void );

    FT_Library handle;
};

struct LFreeTypeFace : public LuaInstance
{
                            LFreeTypeFace       ( lua_State *L, FT_Face face, ILuaClass *file, FT_Stream stream, LFreeType *sys );
                            ~LFreeTypeFace      ( void );

    LFreeType *parent;
    ILuaClass *fileClass;
    FT_Face handle;
    FT_Stream source;
};

void luafreetype_open( lua_State *L );

#endif //_LUA_FREETYPE_