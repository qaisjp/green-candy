/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        game_sa/CColModelSA.cpp
*  PURPOSE:     Collision model entity
*  DEVELOPERS:  Cecill Etheredge <ijsf@gmx.net>
*               arc_
*               The_GTA <quirer@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#include "StdInc.h"

CColFilePool **ppColFilePool = (CColFilePool**)CLASS_CColFilePool;

#define FUNC_ColFilePoolInit    0x004113F0

extern CBaseModelInfoSAInterface **ppModelInfo;

typedef void    (__cdecl*SetCachedCollision_t)              (unsigned int id, CColModelSAInterface *col);
SetCachedCollision_t    SetCachedCollision =                (SetCachedCollision_t)0x005B2C20;

CColModelSAInterface::~CColModelSAInterface()
{
    DWORD dwFunc = FUNC_CColModel_Destructor;

    _asm
    {
        mov     ecx, this
        call    dwFunc
    }
}

void* CColModelSAInterface::operator new( size_t )
{
    return (*ppColModelPool)->Allocate();
}

void CColModelSAInterface::operator delete( void *ptr )
{
    (*ppColModelPool)->Free( (CColModelSAInterface*)ptr );
}

CColModelSA::CColModelSA()
{
    m_pInterface = new CColModelSAInterface;
    m_original = NULL;
    DWORD dwThis = (DWORD)m_pInterface;
    DWORD dwFunc = FUNC_CColModel_Constructor;
    _asm
    {
        mov     ecx, dwThis
        call    dwFunc
    }
    m_bDestroyInterface = true;
}

CColModelSA::CColModelSA( CColModelSAInterface *pInterface, bool destroy )
{
    m_pInterface = pInterface;
    m_bDestroyInterface = destroy;
    m_original = NULL;
}

CColModelSA::~CColModelSA()
{
    RestoreAll();

    if ( m_bDestroyInterface )
        delete m_pInterface;
}

bool CColModelSA::Replace( unsigned short id )
{
    if ( id > DATA_TEXTURE_BLOCK-1 )
        return false;

    if ( IsReplaced( id ) )
        return true;

    // Restore the previous association
    if ( g_colReplacement[id] )
        g_colReplacement[id]->Restore( id );

    CModelLoadInfoSA *info = (CModelLoadInfoSA*)ARRAY_CModelLoadInfo + id;
    CStreamingSA *streamer = pGame->GetStreaming();
    CBaseModelInfoSAInterface *model = ppModelInfo[id];

    // If the model is loading, we should let it finish if it assings a collision
    if ( streamer->IsModelLoading( id ) && model->m_renderFlags & RENDER_COLMODEL )
    {
        streamer->RequestModel( id, 0x10 );
        streamer->LoadAllRequestedModels( true );
    }
    
    // Replace only if we are loaded
    if ( info->m_eLoading == MODEL_LOADED )
        model->SetCollision( m_pInterface, false );

    g_colReplacement[id] = this;
    SetCachedCollision( id, m_pInterface );

    m_imported.push_back( id );
    return true;
}

bool CColModelSA::IsReplaced( unsigned short id ) const
{
    return std::find( m_imported.begin(), m_imported.end(), id ) != m_imported.end();
}

bool CColModelSA::Restore( unsigned short id )
{
    imports_t::const_iterator iter = std::find( m_imported.begin(), m_imported.end(), id );

    if ( iter == m_imported.end() )
        return false;

    // Restore the original colmodel no matter what
    if ( m_original )
    {
        ppModelInfo[id]->SetCollision( m_original, false );
        SetCachedCollision( id, m_original );
    }

    g_colReplacement[id] = NULL;

    m_imported.erase( iter );
    return true;
}

void CColModelSA::RestoreAll()
{
    while ( !m_imported.empty() )
        Restore( m_imported.front() );
}

void* CColFileSA::operator new ( size_t )
{
    return (*ppColFilePool)->Allocate();
}

void CColFileSA::operator delete ( void *ptr )
{
    (*ppColFilePool)->Free( (CColFileSA*)ptr );
}