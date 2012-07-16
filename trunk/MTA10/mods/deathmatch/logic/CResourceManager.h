/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        mods/deathmatch/logic/CResourceManager.h
*  PURPOSE:     Header for resource manager class
*  DEVELOPERS:  Kevin Whiteside <kevuwk@gmail.com>
*               Ed Lyons <eai@opencoding.net>
*               Cecill Etheredge <ijsf@gmx.net>
*               Jax <>
*               Christian Myhre Lundheim <>
*               Derek Abdine <>
*               The_GTA <quiret@gmx.de>
*               
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

class CResourceManager;

#ifndef __CRESOURCEMANAGER_H
#define __CRESOURCEMANAGER_H

class CClientEntity;

class CResourceManager : public ResourceManager
{  
public:
                            CResourceManager();
                            ~CResourceManager();

    CResource*              Add( unsigned short id, const char *name, CClientEntity *resEntity, CClientEntity *dynamicEntity );
    void                    Remove( Resource *res );
    void                    StopAll();

    void                    LoadUnavailableResources( CClientEntity *root );

    bool                    ParseResourcePath( Resource*& res, const char *path, std::string& meta );
    CFile*                  OpenStream( Resource *res, const char *path, const char *mode );
    bool                    FileCopy( Resource *res, const char *src, const char *dst );
    bool                    FileRename( Resource *res, const char *src, const char *dst );
    size_t                  FileSize( Resource *res, const char *path );
    bool                    FileExists( Resource *res, const char *path );
    bool                    FileDelete( Resource *res, const char *path );
};

extern CFileTranslator *resFileRoot;

#endif
