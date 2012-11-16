/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        game_sa/CParticleSystemSA.cpp
*  PURPOSE:     Particle loading and management
*  DEVELOPERS:  The_GTA <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#include "StdInc.h"

CParticleSystemSAInterface *const pParticleSystem = (CParticleSystemSAInterface*)CLASS_CParticleSystem;

#if 0
static void InitParticleData( CEffectDataSAInterface *block )
{
    pParticleSystem->m_effectList.Add( new (block) CEffectDataSAInterface() );
}
#endif

void CParticleSystemSAInterface::Init()
{
    m_count = 0;

    for ( unsigned int n = 0; n < 8; n++ )
        m_matrices[n] = pGame->GetRenderWare()->AllocateMatrix();

    // Allocate from the stack (we leave out the alloc count though, debug?)
    m_effects = new CEffectDataSAInterface[MAX_PARTICLE_DATA];
    
#if 0
    ForEachBlock(
        m_effects = (CEffectDataSAInterface*)m_memory.AllocateInt( sizeof( CEffectDataSAInterface ) * MAX_PARTICLE_DATA ), 
        MAX_PARTICLE_DATA, sizeof( CEffectDataSAInterface ), InitParticleData );
#endif

    for ( unsigned int n = 0; n < MAX_PARTICLE_DATA; n++ )
        m_effectList.Add( &m_effects[n] );

    // The particle system needs game variables
    SetGlobalAssociatives( (const CVector*)0x00C813E0, (const float*)0x00C812F0 );
}

void CParticleSystemSAInterface::Shutdown()
{
}

CEffectDefSAInterface* CParticleSystemSAInterface::ParseFXDataDef( const char *filename, CFile *file )
{
    char buffer[0x100];
    int num;

    file->GetString( buffer, sizeof(buffer) );
    sscanf( buffer, "%d", &num );

    CEffectDefSAInterface *def = new CEffectDefSAInterface();

    def->Parse( filename, file, num );
    return def;
}

bool CParticleSystemSAInterface::LoadDefinitions( const char *filename )
{
    union
    {
        char token[0x80];
        char path[0x80];    // We do not need it a second time
    };
    char buffer[0x100];

    strncpy( path, filename, sizeof( path ) );
    strcpy( path + strlen( filename ) - ( 1 + 4 ), "PC.txd" );  // append string to it

    m_txdID = pGame->GetTextureManager()->LoadDictionaryEx( "fx", path );

    CTxdInstanceSA *inst = (*ppTxdPool)->Get( m_txdID );

    // Reference the main fx texture for ourselves
    inst->Reference();

    // Reference our txd
    RwTxdStack txdRef( inst->m_txd );

    CFile *fxFile = OpenGlobalStream( filename, "rb" );

    if ( !fxFile )
        return false;

    fxFile->GetString( buffer, sizeof(buffer) );
    sscanf( buffer, "%s", token );

    fxFile->GetString( buffer, sizeof(buffer) );    // we skip one line here
    fxFile->GetString( buffer, sizeof(buffer) );
    sscanf( buffer, "%s", token );

    while ( strcmp( token, "FX_SYSTEM_DATA:" ) == 0 )
    {
        // Parse fx definitions
        ParseFXDataDef( filename, fxFile );

        fxFile->GetString( buffer, sizeof(buffer) );
        fxFile->GetString( buffer, sizeof(buffer) );
        sscanf( buffer, "%s", token );
    }

    m_memory.TrimToOffset();    // Finalize the stack by freeing unnecessary memory
    return true;
}

void CParticleSystemSAInterface::SetGlobalAssociatives( const CVector *windVelocity, const float *unk )
{
    m_windVelocity = windVelocity;
    m_unk = unk;
}

CParticleSystemSA::CParticleSystemSA()
{
    // Do not let GTA SA load particles
    //*(unsigned char*)FUNC_InitParticles = 0xC4;

    //pParticleSystem->Init();
}

CParticleSystemSA::~CParticleSystemSA()
{
    //pParticleSystem->Shutdown();
}