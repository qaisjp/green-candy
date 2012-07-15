/*****************************************************************************
*
*  PROJECT:     KillFrenzy! 1.0
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        derived/CResourceManager.h
*  PURPOSE:     Extension package management
*  DEVELOPERS:  The_GTA <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#ifndef _RESOURCE_MANAGER_
#define _RESOURCE_MANAGER_

class CResourceManager : public ResourceManager
{
public:
                                    CResourceManager( CEvents *events, CLuaManager& manager );
                                    ~CResourceManager();

    CResource*                      Load( const std::string& name );

    inline CLuaManager&             GetLuaManager()                         { return m_luaManager; }

protected:
    CLuaManager&    m_luaManager;
};

#endif //_RESOURCE_MANAGER_