/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        mods/deathmatch/logic/CResourceManager.cpp
*  PURPOSE:     Resource manager
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

#include "StdInc.h"

CFileTranslator *resFileRoot;
CResourceManager *resMan = NULL;

CResourceManager::CResourceManager() : ResourceManager( g_pClientGame->GetLuaManager()->GetVirtualMachine() )
{
    filePath resRoot;
    modFileRoot->GetFullPath( "/resources/", false, resRoot );

    resFileRoot = g_pCore->GetFileSystem()->CreateTranslator( resRoot.c_str() );

    // The one and only resource manager!
    resMan = this;

    LuaManager::m_resMan = this;
}

CResourceManager::~CResourceManager()
{
    while ( !m_resources.empty() )
    {
        CResource *res = (CResource*)m_resources.back();

        CLuaArguments args;
        args.PushUserData( res );
        res->GetResourceEntity()->CallEvent( "onClientResourceStop", args, true );
        delete res;

        m_resources.pop_back();
    }
}

CResource* CResourceManager::Add( unsigned short id, const char *name, CClientEntity *resEntity, CClientEntity *dynamicEntity )
{
    filePath resPath;

    if ( !resFileRoot->GetFullPathFromRoot( name, false, resPath ) )
        return NULL;

    CFileTranslator *root = g_pCore->GetFileSystem()->CreateTranslator( resPath.c_str() );

    CResource *res = new CResource( id, name, *root, resEntity, dynamicEntity );

    m_resources.push_back( res );
    return res;
}

void CResourceManager::LoadUnavailableResources( CClientEntity *root )
{
    resourceList_t::const_iterator iter = m_resources.begin();

    for ( ; iter != m_resources.end(); iter++ )
    {
        if ( !( (*iter)->GetActive() ) )
            ((CResource*)*iter)->Load( root );
    }
}

void CResourceManager::Remove( Resource *res )
{
    // Delete all the resource's locally created children (the server won't do that)
    ((CResource*)res)->DeleteClientChildren();

    ResourceManager::Remove( res );
}

void CResourceManager::StopAll()
{
    while ( m_resources.size() != 0 )
        Remove( m_resources.front() );
}

bool CResourceManager::ParseResourcePath( Resource*& res, const char *path, std::string& meta )
{
    if ( path[0] == '@' )
        meta = '@';

    if ( !ResourceManager::ParseResourcePath( res, path, path ) )
        return false;

    meta += path;
    return true;
}

CFile* CResourceManager::OpenStream( Resource *res, const char *path, const char *mode )
{
    string meta;

    if ( !ParseResourcePath( res, path, meta ) )
        return NULL;

    return res->OpenStream( meta.c_str(), mode );
}

bool CResourceManager::FileCopy( Resource *res, const char *src, const char *dst )
{
    Resource *dstRes = res;
    std::string srcMeta;
    std::string dstMeta;

    if ( !ParseResourcePath( res, src, srcMeta ) || !ParseResourcePath( dstRes, dst, dstMeta ) )
        return false;

    if ( res == dstRes )
        return res->FileCopy( src, dst );

    CFile *srcFile = res->OpenStream( src, "rb" );

    if ( !srcFile )
        return false;

    CFile *dstFile = dstRes->OpenStream( dst, "wb" );

    if ( !dstFile )
    {
        delete srcFile;

        return false;
    }

    FileSystem::StreamCopy( *srcFile, *dstFile );

    delete srcFile;
    delete dstFile;

    return true;
}

bool CResourceManager::FileRename( Resource *res, const char *src, const char *dst )
{
    Resource *dstRes = res;
    std::string srcMeta;
    std::string dstMeta;

    if ( !ParseResourcePath( res, src, srcMeta ) || !ParseResourcePath( dstRes, dst, dstMeta ) )
        return false;

    if ( res == dstRes )
        return res->FileRename( src, dst );

    CFile *srcFile = res->OpenStream( src, "rb+" );

    if ( !srcFile )
        return false;

    CFile *dstFile = dstRes->OpenStream( dst, "wb" );

    if ( !dstFile )
    {
        delete srcFile;

        return false;
    }

    FileSystem::StreamCopy( *srcFile, *dstFile );

    delete srcFile;
    delete dstFile;

    return res->FileDelete( src );
}

size_t CResourceManager::FileSize( Resource *res, const char *path )
{
    std::string meta;

    if ( !ParseResourcePath( res, path, meta ) )
        return 0;

    return res->FileSize( meta.c_str() );
}

bool CResourceManager::FileExists( Resource *res, const char *path )
{
    std::string meta;

    return ParseResourcePath( res, path, meta ) && res->FileExists( meta.c_str() );
}

bool CResourceManager::FileDelete( Resource *res, const char *path )
{
    std::string meta;

    return ParseResourcePath( res, path, meta ) && res->FileDelete( meta.c_str() );
}