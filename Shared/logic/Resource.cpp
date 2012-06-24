/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        Shared/logic/Resource.cpp
*  PURPOSE:     Resource management for scripting environments
*  DEVELOPERS:  The_GTA <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#include <StdInc.h>

Resource::Resource( unsigned short id, const filePath& name, CFileTranslator& root ) : m_fileRoot( root )
{
    m_id = id;
    m_name = name;
    m_active = false;
    m_lua = NULL;
}

Resource::~Resource()
{
}

CFile* Resource::OpenStream( const char *path, const char *mode )
{
    return m_fileRoot.Open( path, mode );
}

bool Resource::FileCopy( const char *src, const char *dst )
{
    return m_fileRoot.Copy( src, dst );
}

bool Resource::FileRename( const char *src, const char *dst )
{
    return m_fileRoot.Rename( src, dst );
}

size_t Resource::FileSize( const char *path ) const
{
    return m_fileRoot.Size( path );
}

bool Resource::FileExists( const char *path ) const
{
    return m_fileRoot.Exists( path );
}

bool Resource::FileDelete( const char *path )
{
    return m_fileRoot.Delete( path );
}