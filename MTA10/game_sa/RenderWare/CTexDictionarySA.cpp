/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        game_sa/RenderWare/CTexDictionarySA.cpp
*  PURPOSE:     Internal texture dictionary handler
*               RenderWare extension
*  DEVELOPERS:  The_GTA <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#include "StdInc.h"

extern CBaseModelInfoSAInterface** ppModelInfo;

CTexDictionarySA::CTexDictionarySA( const char *name )
{
    m_name = name;
    m_tex = (*ppTxdPool)->Get( pGame->GetTextureManager()->CreateTxdEntry( name ) );

    if ( !m_tex )
        throw std::exception( "could not allocate texture dictionary" );

    m_tex->Reference();
}

static bool RwTexDictionaryAssign( RwTexture *tex, CTexDictionarySA *txd )
{
    txd->m_textures.push_back( new CTextureSA( txd, tex ) );
    return true;
}

CTexDictionarySA::CTexDictionarySA( const char *name, CTxdInstanceSA *txd )
{
    m_name = name;
    m_tex = txd;

    // Virtualize all instances found
    txd->m_txd->ForAllTextures( RwTexDictionaryAssign, this );
}

CTexDictionarySA::~CTexDictionarySA()
{
    Clear();

    m_tex->Dereference();

    pGame->GetTextureManager()->m_texDicts.remove( this );
}

struct _rwAssign
{
    bool filtering;
    CTexDictionarySA *txd;
};

static bool RwTexDictionaryAssignNew( RwTexture *tex, _rwAssign *assign )
{
    assign->txd->m_textures.push_back( new CTextureSA( assign->txd, tex ) );

    if ( assign->filtering )
        tex->flags = 0x1102;

    return true;
}

unsigned int CTexDictionarySA::GetHash() const
{
    return m_tex->m_hash;
}

bool CTexDictionarySA::Load( CFile *file, bool filtering )
{
    // Try to load it
    if ( !m_tex->LoadTXD( file ) )
        return false;

    // Virtualize all textures
    _rwAssign assign;
    assign.filtering = filtering;
    assign.txd = this;

    m_tex->m_txd->ForAllTextures( RwTexDictionaryAssignNew, &assign );

    // We succeeded if we got any textures
    return m_textures.size() != 0;
}

void CTexDictionarySA::Clear()
{
    // Destroy all textures, they automatically detach from txds
    while ( !m_textures.empty() )
        delete *m_textures.begin();

    m_imported.clear();
}

unsigned short CTexDictionarySA::GetID() const
{
    return (*ppTxdPool)->GetIndex( m_tex );
}

bool CTexDictionarySA::IsImported( unsigned short id ) const
{
    CBaseModelInfoSAInterface *info = ppModelInfo[id];

    if ( id > MAX_MODELS-1 )
        return false;

    return IsImportedTXD( info->m_textureDictionary );
}

bool CTexDictionarySA::IsImportedTXD( unsigned short id ) const
{
    importList_t::const_iterator iter = m_imported.begin();

    for ( ; iter != m_imported.end(); iter++ )
        if ( *iter == id )
            return true;

    return false;
}

bool CTexDictionarySA::Import( unsigned short id )
{
    CBaseModelInfoSAInterface *info = ppModelInfo[id];

    if ( id > MAX_MODELS-1 )
        return false;

    if ( !info )
        return false;

    return ImportTXD( info->m_textureDictionary );
}

bool CTexDictionarySA::ImportTXD( unsigned short id )
{
    if ( !(*ppTxdPool)->Get( id ) )
        return false;

    textureList_t::iterator iter = m_textures.begin();

    for ( ; iter != m_textures.end(); iter++ )
        (*iter)->ImportTXD( id );

    m_imported.push_back( id );
    return true;
}

bool CTexDictionarySA::Remove( unsigned short id )
{
    CBaseModelInfoSAInterface *info = ppModelInfo[id];

    if ( id > MAX_MODELS-1 )
        return false;

    if ( !info )
        return false;

    return RemoveTXD( info->m_textureDictionary );
}

bool CTexDictionarySA::RemoveTXD( unsigned short id )
{
    importList_t::iterator iter = std::find( m_imported.begin(), m_imported.end(), id );

    if ( iter == m_imported.end() )
        return true;

    if ( !(*ppTxdPool)->Get( id ) )
        return false;

    textureList_t::iterator itera = m_textures.begin();

    for ( ; itera != m_textures.end(); itera++ )
        (*itera)->RemoveTXD( id );

    m_imported.erase( iter );
    return true;
}

void CTexDictionarySA::ClearImports()
{
    while ( !m_imported.empty() )
        RemoveTXD( *m_imported.begin() );
}