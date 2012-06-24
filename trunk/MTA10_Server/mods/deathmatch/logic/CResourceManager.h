/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.0
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        mods/deathmatch/logic/CResourceManager.h
*  PURPOSE:     Resource manager class
*  DEVELOPERS:  Ed Lyons <>
*               Cecill Etheredge <>
*               Christian Myhre Lundheim <>
*               Kevin Whiteside <>
*               Oliver Brown <>
*               Jax <>
*               The_GTA <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

// This class controls all the resources that are loaded, and loads
// new resources on demand

#ifndef CRESOURCEMANAGER_H
#define CRESOURCEMANAGER_H

#include "CResource.h"
#include "CElement.h"
#include "ehs/ehs.h"
#include <list>

class CResource;

class CResourceManager : public EHS, public ResourceManager
{
public:
    enum eResourceQueue
    {
        QUEUE_STOP,
        QUEUE_STOPALL,
        QUEUE_RESTART,
        QUEUE_RESTART2,
    };

    struct sResourceStartFlags
    {
        bool bConfigs;
        bool bMaps;
        bool bScripts;
        bool bHTML;
        bool bClientConfigs;
        bool bClientScripts;
        bool bClientFiles;
    };

private:
    struct sResourceQueue
    {
        CResource*          pResource;
        eResourceQueue      eQueue;
        sResourceStartFlags Flags;
        vector < SString >  dependents;
    };

public:
                                CResourceManager();
                                ~CResourceManager();

    CResource*                  Load( bool bIsZipped, const char * szAbsPath, const char * szResourceName );
    void                        Unload( CResource *resource );
    void                        UnloadRemovedResources();
    void                        CheckResourceDependencies();
    void                        ListResourcesLoaded();

    std::list < CResource* > ::const_iterator  IterBegin()              { return m_resources.begin(); };
    std::list < CResource* > ::const_iterator  IterEnd()                { return m_resources.end(); };

    bool                        Refresh( bool all = false );
    void                        Upgrade();
    inline unsigned int         GetResourceLoadedCount()                { return m_uiResourceLoadedCount; }
    inline unsigned int         GetResourceFailedCount()                { return m_uiResourceFailedCount; }
    void                        OnPlayerJoin( CPlayer& Player );

    bool                        StartResource( CResource* pResource, list < CResource* > * dependents = NULL, bool bStartedManually = false, bool bStartIncludedResources = true, bool bConfigs = true, bool bMaps = true, bool bScripts = true, bool bHTML = true, bool bClientConfigs = true, bool bClientScripts = true, bool bClientFiles = true );
    bool                        Reload( CResource* pResource );
    void                        StopAll();

    void                        QueueResource( CResource* pResource, eResourceQueue eQueueType, const sResourceStartFlags* Flags, list < CResource* > * dependents = NULL );
    void                        ProcessQueue();
    void                        RemoveFromQueue( CResource* pResource );

    bool                        IsAResourceElement( CElement* pElement );

    unsigned short              GenerateID();

    CResource*                  GetResourceFromLuaState( lua_State* luaVM );
    bool                        Install( const char *url, const char *name );

    CResource*                  CreateResource( char *name );
    CResource*                  CopyResource( CResource *src, const char *dst );

    void                        NotifyResourceVMOpen( CResource* pResource, CLuaMain* pVM );
    void                        NotifyResourceVMClose( CResource* pResource, CLuaMain* pVM );

private:
    unsigned int                m_uiResourceLoadedCount;
    unsigned int                m_uiResourceFailedCount;
    bool                        m_bResourceListChanged;
    list<CResource *>           m_resourcesToStartAfterRefresh;

    // Maps to speed things up
    std::map <CResource*, lua_State*>       m_ResourceLuaStateMap;
    std::map <lua_State*, CResource*>       m_LuaStateResourceMap;

    list<sResourceQueue>        m_resourceQueue;
};

#endif
