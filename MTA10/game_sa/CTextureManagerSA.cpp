/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        game_sa/CTextureManagerSA.cpp
*  PURPOSE:     Internal texture management
*               SubInterface of RenderWare
*  DEVELOPERS:  The_GTA <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#include "StdInc.h"
#include "gamesa_renderware.h"

#define FUNC_InitTextureManager         0x00731F20

CTxdPool**   ppTxdPool = (CTxdPool**)0x00C8800C;

CTxdInstanceSA::CTxdInstanceSA(const char *name)
{
    m_txd = RwTexDictionaryCreate();
    m_references = 0;
    m_parentTxd = 0xFFFF;
    m_hash = pGame->GetKeyGen()->GetUppercaseKey(name);
}

bool Txd_DeleteAll( RwTexture *tex, void *data )
{
    if ( tex->refs < 2 )
    {
        tex->RemoveFromDictionary();

        RwTextureDestroy( tex );
    }

    return true;
}

CTxdInstanceSA::~CTxdInstanceSA()
{
    if ( m_txd )
    {
        m_txd->ForAllTextures( Txd_DeleteAll, NULL );

        RwTexDictionaryDestroy( m_txd );

        m_txd = NULL;
    }

    if ( m_parentTxd != 0xFFFF )
        (*ppTxdPool)->Get( m_parentTxd )->Dereference();
}

void* CTxdInstanceSA::operator new( size_t )
{
    return (*ppTxdPool)->Allocate();
}

void CTxdInstanceSA::operator delete( void *ptr )
{
    (*ppTxdPool)->Free( (CTxdInstanceSA*)ptr );
}

bool CTxdInstanceSA::LoadTXD( const char *filename )
{
    RwStream *stream = RwStreamOpen( STREAM_TYPE_FILENAME, STREAM_MODE_READ, filename );
    unsigned int size;
    unsigned int version;
    unsigned int numTextures;

    if ( !stream ) 
        return false;

    if (!RwStreamFindChunk( stream, 0x16, NULL, NULL ) || 
        !RwStreamFindChunk( stream, 1, &size, &version ) || 
        RwStreamReadBlocks( stream, &numTextures, size ) != size )
    {
        RwStreamClose( stream, NULL );
        return false;
    }

    while ( numTextures-- )
    {
        RwTexture *texture = RwStreamReadTexture( stream );

        if ( !texture )
        {
            RwStreamClose( stream, NULL );
            return false;
        }

        texture->AddToDictionary( m_txd );
    }

    RwStreamClose( stream );
    return true;
}

unsigned short CTxdInstanceSA::InitParent()
{
    CTxdInstanceSA *parent = (*ppTxdPool)->Get( m_parentTxd );
    CTxdInstanceSA *affectTxd;

    if ( !parent )
        return 0xFFFF;

    affectTxd = ((CTxdInstanceSA*)(*(unsigned int**)0x00C88018 + (unsigned int)this));
    affectTxd->m_txd = m_txd;

    parent->Reference();
    return m_parentTxd;
}

void CTxdInstanceSA::Reference()
{
    m_references++;
}

void CTxdInstanceSA::Dereference()
{
    m_references--;

    // We unload ourselves
    if ( m_references == 0 )
        pGame->GetStreaming()->FreeModel( DATA_TEXTURE_BLOCK + (*ppTxdPool)->GetIndex( this ) );
}

void CTxdInstanceSA::SetCurrent()
{
    RwTexDictionarySetCurrent( m_txd );
}

CTextureManagerSA::CTextureManagerSA()
{
    unsigned int n;

    // We init it ourselves
    *(unsigned char*)FUNC_InitTextureManager = 0xC3;

    *ppTxdPool = new CTxdPool;

    // Reserve 7 txds
    for ( n=0; n<7; n++ )
        CreateTxdEntry( "*" );

    __asm
    {
        // Register some callbacks
        mov edx,0x007F3510
        push 0x00731720
        call edx

        mov edx,0x007F3540
        push 0x00731710
        call edx

        add esp,8
    }
}

CTextureManagerSA::~CTextureManagerSA()
{
    (*ppTxdPool)->Clear();

    delete *ppTxdPool;
}

int CTextureManagerSA::FindTxdEntry( const char *name )
{
    unsigned int hash = pGame->GetKeyGen()->GetUppercaseKey(name);
    unsigned int n;

    for (n=0; n<MAX_TXD; n++)
    {
        CTxdInstanceSA *txd = (*ppTxdPool)->Get(n);

        if (!txd || txd->m_hash != hash)
            continue;

        return n;
    }

    return -1;
}

int CTextureManagerSA::CreateTxdEntry( const char *name )
{
    CTxdInstanceSA *inst = new CTxdInstanceSA(name);

    // Check for crashes here
    if ( !inst )
        return -1;

    return (*ppTxdPool)->GetIndex(inst);
}

CTexDictionarySA* CTextureManagerSA::CreateTxd( const char *name )
{
    CTexDictionarySA *txd = new CTexDictionarySA( name );

    m_texDicts.push_back( txd );
    return txd;
}

CTexDictionarySA* CTextureManagerSA::CreateTxd( const char *name, unsigned short id )
{
    CTxdInstanceSA *itxd = (*ppTxdPool)->Get( id );

    if ( !itxd )
        return NULL;

    CTexDictionarySA *txd = new CTexDictionarySA( name, itxd );

    if ( !txd )
        return NULL;

    m_texDicts.push_back( txd );
    return txd;
}

unsigned short CTextureManagerSA::LoadDictionary( const char *filename )
{
    return LoadDictionaryEx( ExtractFilename( filename ), filename);
}

unsigned short CTextureManagerSA::LoadDictionaryEx( const char *name, const char *filename )
{
    CTxdInstanceSA *txd = CreateTxdEntry( name );

    if ( !txd )
        return -1;

    if ( !txd->LoadTXD( filename ) )
    {
        delete txd;

        return -1;
    }

    txd->InitParent();

    return (*ppTxdPool)->GetIndex( txd );
}

bool CTextureManagerSA::SetCurrentTexture( unsigned short id )
{
    CTxdInstanceSA *txd = (*ppTxdPool)->Get( id );

    if ( !txd )
        return false;

    txd->SetCurrent();
    return true;
}

void CTextureManagerSA::RemoveTxdEntry( unsigned short id )
{
    CTxdInstanceSA *txd = (*ppTxdPool)->Get( id );

    // Crashfix
    if ( !txd )
        return;

    delete txd;
}