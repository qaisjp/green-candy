/*****************************************************************************
*
*  PROJECT:     Lua Interpreter
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        derived/CResourceManager.h
*  PURPOSE:     Lua resource manager class
*  DEVELOPERS:  Martin Turski <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#ifndef _LUA_RESOURCEMANAGER_DERIVED_
#define _LUA_RESOURCEMANAGER_DERIVED_

class CResourceManager : public ResourceManager
{
public:
                            CResourceManager( CLuaManager& manager );
                            ~CResourceManager();

    CResource*              Load( const std::string& name );

    inline CLuaManager&     GetLuaManager( void )           { return m_luaManager; }

protected:
    CLuaManager&    m_luaManager;
};

#endif //_LUA_RESOURCEMANAGER_DERIVED_