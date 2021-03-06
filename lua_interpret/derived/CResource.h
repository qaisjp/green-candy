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

#define LUACLASS_RESOURCE_DERIVE    54

class CResource : public Resource
{
    friend class CResourceManager;
public:
                                CResource( CResourceManager *manager, CLuaMain& main, unsigned short id, const filePath& name, CFileTranslator& root );
                                ~CResource( void );

    CFileTranslator*            GetFileRoot( void )         { return &m_fileRoot; }

protected:
    CResourceManager*   m_resMan;
    CLuaMain*           m_luaMain;
};

#endif //_LUA_RESOURCE_DERIVED_