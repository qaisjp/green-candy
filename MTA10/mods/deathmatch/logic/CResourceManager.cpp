/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.0
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        mods/deathmatch/logic/CResourceManager.cpp
*  PURPOSE:     Resource manager
*  DEVELOPERS:  Kevin Whiteside <kevuwk@gmail.com>
*               Ed Lyons <eai@opencoding.net>
*               Cecill Etheredge <ijsf@gmx.net>
*               Jax <>
*               Christian Myhre Lundheim <>
*               Derek Abdine <>
*               
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#include "StdInc.h"

CResourceManager::CResourceManager()
{
}


CResourceManager::~CResourceManager ( void )
{
    while ( !m_resources.empty () )
    {
        CResource* pResource = m_resources.back ();

        CLuaArguments Arguments;
        Arguments.PushUserData ( pResource );
        pResource->GetResourceEntity ()->CallEvent ( "onClientResourceStop", Arguments, true );
        delete pResource;

        m_resources.pop_back ();
    }
}

CResource* CResourceManager::Add ( unsigned short usID, char* szResourceName, CClientEntity* pResourceEntity, CClientEntity* pResourceDynamicEntity )
{
    CResource* pResource = new CResource ( usID, szResourceName, pResourceEntity, pResourceDynamicEntity );
    if ( pResource )
    {
        m_resources.push_back ( pResource );
        return pResource;
    }
    return NULL;
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

void CResourceManager::Remove ( CResource* pResource )
{
    // Delete all the resource's locally created children (the server won't do that)
    pResource->DeleteClientChildren ();

    // Delete the resource
    if ( !m_resources.empty() ) m_resources.remove ( pResource );
    delete pResource;
}


bool CResourceManager::Exists ( CResource* pResource )
{
    return m_resources.Contains ( pResource );
}


void CResourceManager::StopAll ( void )
{
    while ( m_resources.size () > 0 )
    {
        Remove ( m_resources.front () );
    }
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