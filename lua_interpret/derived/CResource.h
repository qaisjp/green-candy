/*****************************************************************************
*
*  PROJECT:     Lua Interpreter
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        derived/CResource.h
*  PURPOSE:     Lua resource class
*  DEVELOPERS:  Martin Turski <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#ifndef _LUA_RESOURCE_DERIVED_
#define _LUA_RESOURCE_DERIVED_

class CResource : public Resource
{
public:
                                CResource( CLuaMain& main, unsigned short id, const filePath& name, CFileTranslator& root );
                                ~CResource();
};

#endif //_LUA_RESOURCE_DERIVED_