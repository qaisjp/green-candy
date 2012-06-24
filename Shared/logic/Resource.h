/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        Shared/logic/Resource.h
*  PURPOSE:     Resource management for scripting environments
*  DEVELOPERS:  The_GTA <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#ifndef _BASE_RESOURCE_
#define _BASE_RESOURCE_

class Resource
{
public:
                                Resource( unsigned short id, const filePath& name, CFileTranslator& root );
    virtual                     ~Resource();

    inline unsigned short       GetID()                 { return m_id; };
    inline const filePath&      GetName()               { return m_name; };
    inline LuaMain*             GetVM()                 { return m_lua; };
    inline bool                 GetActive()             { return m_active; };

    virtual CFile*              OpenStream( const char *path, const char *mode );
    virtual bool                FileCopy( const char *src, const char *dst );
    virtual bool                FileRename( const char *src, const char *dst );
    virtual size_t              FileSize( const char *path ) const;
    virtual bool                FileExists( const char *path ) const;
    virtual bool                FileDelete( const char *path );

private:
    unsigned short      m_id;
    filePath            m_name;
    LuaMain*            m_lua;
    bool                m_active;

    CFileTranslator&    m_fileRoot;
};

#define lua_getresource( L )    (lua_rawgeti( L, 3 ))

#endif //_BASE_RESOURCE_