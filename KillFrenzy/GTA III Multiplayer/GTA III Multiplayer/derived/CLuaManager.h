/*****************************************************************************
*
*  PROJECT:     KillFrenzy! 1.0
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        derived/CLuaManager.h
*  PURPOSE:     Lua hyperstructure management extension
*  DEVELOPERS:  The_GTA <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#ifndef _LUA_MANAGER_
#define _LUA_MANAGER_

class CLuaManager : public LuaManager
{
public:
                            CLuaManager( CEvents& events );
                            ~CLuaManager();

    inline CRegisteredCommands& GetCommands()       { return m_commands; }
    inline CScriptDebugging&    GetDebug()          { return m_debug; }

    CLuaMain*               Create( const std::string& name, CFileTranslator& fileRoot );
    bool                    Remove( LuaMain *lua );

protected:
    CRegisteredCommands     m_commands;
    CScriptDebugging        m_debug;
};

#endif //_LUA_MANAGER_