/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        Shared/logic/ResourceManager.cpp
*  PURPOSE:     Resource management for scripting environments
*  DEVELOPERS:  The_GTA <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#include <StdInc.h>

CFileTranslator* ResourceManager::resFileRoot = NULL;

ResourceManager::ResourceManager()
{
}

ResourceManager::~ResourceManager()
{
}

// Inheritance has to provide it's own addition of resources

Resource* ResourceManager::Get( const char *name )
{
    Resource **res = MapFind( m_resByName, name );

    if ( !res )
        return NULL;

    return *res;
}

Resource* ResourceManager::Get( unsigned short id )
{
    resourceList_t::iterator iter = m_resources.begin();

    for ( ; iter != m_resources.end(); iter++ )
    {
        if ( (*iter)->GetID() == id )
            return *iter;
    }

    return NULL;
}

bool ResourceManager::Remove( unsigned short id )
{
    Resource *res = Get( id );

    if ( !res )
        return false;

    Remove( res );
    return true;
}

void ResourceManager::Remove( Resource *res )
{
    m_resources.remove( res );
    MapRemove( m_resByName, res->GetName() );
}

bool ResourceManager::Exists( Resource *res ) const
{
    return m_resByName.find( res->GetName() ) != m_resByName.end();
}

bool ResourceManager::ParseResourcePath( Resource*& res, const char *path, const char*& meta )
{
    if ( path[0] == ':' )
    {
        path++;

        const char *delim = strchr( path, '/' );

        if ( !delim || delim == path )
            return false;

        res = Get( std::string( path, (size_t)( path - delim - 1 ) ).c_str() );

        if ( !res )
            return false;

        meta = delim + 1;
        return true;
    }

    meta = path;
    return true;
}

CFile* ResourceManager::OpenStream( Resource *res, const char *path, const char *mode )
{
    if ( !ParseResourcePath( res, path, path ) )
        return NULL;

    return res->OpenStream( path, mode );
}

bool ResourceManager::FileCopy( Resource *res, const char *src, const char *dst )
{
    Resource *dstRes = res;

    if ( !ParseResourcePath( res, src, src ) || !ParseResourcePath( dstRes, dst, dst ) )
        return false;

    if ( res == dstRes )
        return res->FileCopy( src, dst );

    return resFileRoot->Copy( res->GetName() + '/' + src, dstRes->GetName() + '/' + dst );
}

bool ResourceManager::FileRename( Resource *res, const char *src, const char *dst )
{
    Resource *dstRes = res;

    if ( !ParseResourcePath( res, src, src ) || !ParseResourcePath( dstRes, dst, dst ) )
        return false;

    if ( res == dstRes )
        return res->FileRename( src, dst );

    return resFileRoot->Rename( res->GetName() + '/' + src, dstRes->GetName() + '/' + dst );
}

size_t ResourceManager::FileSize( Resource *res, const char *path )
{
    if ( !ParseResourcePath( res, path, path ) )
        return 0;

    return res->FileSize( path );
}

bool ResourceManager::FileExists( Resource *res, const char *path )
{
    return ParseResourcePath( res, path, path ) && res->FileExists( path );
}

bool ResourceManager::FileDelete( Resource *res, const char *path )
{
    return ParseResourcePath( res, path, path ) && res->FileDelete( path );
}