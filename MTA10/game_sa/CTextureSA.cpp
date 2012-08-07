/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        game_sa/CStreamingSA.cpp
*  PURPOSE:     RenderWare texture extension
*  DEVELOPERS:  The_GTA <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#include "StdInc.h"
#include "gamesa_renderware.h"

extern CBaseModelInfoSAInterface** ppModelInfo;

CTextureSA::CTextureSA( RwTexture *tex )
{
    m_texture = tex;
    m_dictionary = NULL;

    m_texture->refs++;
}

CTextureSA::CTextureSA( CTexDictionarySA *dict, RwTexture *tex )
{
    m_texture = tex;
    m_dictionary = dict;

    m_texture->refs++;
}

CTextureSA::~CTextureSA()
{
    ClearImports();

    m_texture->refs--;

    if ( m_dictionary )
        m_dictionary->m_textures.remove( this );

    m_texture->RemoveFromDictionary();
    RwTextureDestroy( m_texture );
}

const char* CTextureSA::GetName() const
{
    return m_texture->name;
}

unsigned int CTextureSA::GetHash() const
{
    return pGame->GetKeyGen()->GetUppercaseKey( m_texture->name );
}

bool CTextureSA::IsImported( unsigned short id ) const
{
    CBaseModelInfoSAInterface *info = ppModelInfo[id];

    if ( id > DATA_TEXTURE_BLOCK-1 )
        return false;

    if ( !info )
        return false;

    return IsImportedTXD( info->m_textureDictionary );
}

bool CTextureSA::IsImportedTXD( unsigned short id ) const
{
    return m_imported.count( id ) != 0;
}

void CTextureSA::OnTxdLoad( RwTexDictionary& txd, unsigned short id )
{
    import& imp = (*m_imported.find( id )).second;
    imp.original = txd.FindNamedTexture( m_texture->name );
    
    if ( imp.original )
        imp.original->RemoveFromDictionary();

    imp.copy->AddToDictionary( &txd );
}

void CTextureSA::OnTxdInvalidate( RwTexDictionary& txd, unsigned short id )
{
    import& imp = (*m_imported.find( id )).second;
    imp.copy->RemoveFromDictionary();

    if ( imp.original )
    {
        RwTextureDestroy( imp.original );
        imp.original = NULL;
    }
}

bool CTextureSA::Import( unsigned short id )
{
    CBaseModelInfoSAInterface *info = ppModelInfo[id];

    if ( id > DATA_TEXTURE_BLOCK-1 )
        return false;

    if ( !info )
        return false;

    return ImportTXD( info->m_textureDictionary );
}

bool CTextureSA::ImportTXD( unsigned short id )
{
    // Do not import twice
    if ( IsImportedTXD( id ) )
        return true;

    CTxdInstanceSA *txd = (*ppTxdPool)->Get( id );

    if ( !txd )
        return false;

    import imp;
    imp.copy = RwTextureCreate( m_texture->raster );

    if ( RwTexDictionary *rtxd = txd->m_txd )
    {
        imp.original = rtxd->FindNamedTexture( m_texture->name );

        if ( imp.original )
            imp.original->RemoveFromDictionary();

        imp.copy->AddToDictionary( rtxd );
    }
    else
        imp.original = NULL;    // flag that we have not been imported; wait for TXD load

    if ( m_imported.empty() )
        g_dictImports[id].push_back( this );
    
    m_imported[id] = imp;
    return true;
}

bool CTextureSA::Remove( unsigned short id )
{
    CBaseModelInfoSAInterface *info = ppModelInfo[id];

    if ( id > MAX_MODELS-1 )
        return false;

    if ( !info )
        return false;

    return RemoveTXD( info->m_textureDictionary );
}

bool CTextureSA::RemoveTXD( unsigned short id )
{
    importMap_t::iterator iter = m_imported.find( id );

    if ( iter == m_imported.end() )
        return false;

    import& imp = ( *iter ).second;
    RwTexDictionary *txd = imp.copy->txd;
    imp.copy->RemoveFromDictionary();

    RwTextureDestroy( imp.copy );

    if ( imp.original )
        imp.original->AddToDictionary( txd );
    
    m_imported.erase( iter );

    if ( m_imported.empty() )
        g_dictImports[id].remove( this );

    return true;
}

void CTextureSA::ClearImports()
{
    while ( !m_imported.empty() )
        RemoveTXD( (*m_imported.begin()).first );
}