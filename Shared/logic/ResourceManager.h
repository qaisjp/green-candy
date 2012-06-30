/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        Shared/logic/ResourceManager.h
*  PURPOSE:     Resource management for scripting environments
*  DEVELOPERS:  The_GTA <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#ifndef _BASE_RESOURCE_MANAGER_
#define _BASE_RESOURCE_MANAGER_

class ResourceManager abstract
{
public:
                            ResourceManager();
                            ~ResourceManager();

    Resource*               Get( const char *name );
    Resource*               Get( unsigned short id );
    bool                    Remove( unsigned short id );
    virtual void            Remove( Resource *res );
    bool                    Exists( Resource *res ) const;

    // File interface
    bool                    ParseResourcePath( Resource*& res, const char *path, const char*& meta );
    virtual CFile*          OpenStream( Resource *res, const char *path, const char *mode );
    virtual bool            FileCopy( Resource *res, const char *src, const char *dst );
    virtual bool            FileRename( Resource *res, const char *src, const char *dst );
    virtual size_t          FileSize( Resource *res, const char *path );
    virtual bool            FileExists( Resource *res, const char *path );
    virtual bool            FileDelete( Resource *res, const char *path );

    // Share this value with all deriviates
    static CFileTranslator *resFileRoot;

protected:
    typedef std::list <Resource*> resourceList_t;
    typedef std::map <filePath, Resource*> resNameMap_t;

    resourceList_t  m_resources;
    resNameMap_t    m_resByName;
};

#endif //_BASE_RESOURCE_MANAGER_