/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        derived/CLuaManager.h
*  PURPOSE:     Derived lua hyperstructure management
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
                                    CLuaManager();
                                    ~CLuaManager();

    CLuaMain*                       Create( const std::string& name, CFileTranslator& fileRoot );
    bool                            Remove( LuaMain *lua );

    inline CScriptDebugging&        GetDebug()  { return (CScriptDebugging&)m_debug; }

    const CLuaMain*                 GetStatus( int *line, std::string *src, std::string *proto_name )    { return (const CLuaMain*)LuaManager::GetStatus( line, src, proto_name ); }

protected:
    CRegisteredCommands m_commands;
};

#endif //_LUA_MANAGER_