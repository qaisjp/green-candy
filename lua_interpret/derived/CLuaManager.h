/*****************************************************************************
*
*  PROJECT:     Lua Interpreter
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        derived/CLuaManager.h
*  PURPOSE:     Lua hyperstructure manager
*  DEVELOPERS:  Martin Turski <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#ifndef _LUA_MANAGER_DERIVED_
#define _LUA_MANAGER_DERIVED_

class CLuaManager : public LuaManager
{
public:
                            CLuaManager( CEvents& events );
                            ~CLuaManager();

    inline CScriptDebugging&    GetDebug()          { return m_debug; }

    CLuaMain*               Create( const std::string& name, CFileTranslator& fileRoot );
    bool                    Remove( LuaMain *lua );

protected:
    CScriptDebugging        m_debug;
};

#endif //_LUA_MANAGER_DERIVED_