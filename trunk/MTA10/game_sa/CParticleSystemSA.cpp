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

CParticleSystemSAInterface *pParticleSystem = (CParticleSystemSAInterface*)CLASS_CParticleSystem;

static void InitParticleData( CEffectDataSAInterface *block )
{
    pParticleSystem->m_effectList.Add( new (block) CEffectDataSAInterface() );
}

void CParticleSystemSAInterface::Init()
{
#if 0
    unsigned int n;

    new (&m_memory) CEffectStackSA();

    m_count = 0;

    for (n=0; n<8; n++)
        m_matrices[n] = pGame->GetRenderWare()->AllocateMatrix();

    // Allocate from the stack (we leave out the alloc count though, debug?)
    ForEachBlock(
        m_effects = (CParticleDataSAInterface*)m_particles.AllocateInt( sizeof( CParticleDataSAInterface ) * MAX_PARTICLE_DATA ), 
        MAX_PARTICLE_DATA, sizeof( CEffectDataSAInterface ), InitParticleData );
#endif
}

void CParticleSystemSAInterface::Shutdown()
{
#if 0
    m_particles.~CAlignedStackSA();
#endif
}

void CParticleSystemSAInterface::LoadDefinitions( const char *filename )
{
    char buffer[256];
}

CParticleSystemSA::CParticleSystemSA()
{
    // Do not let GTA SA load particles
    //*(unsigned char*)FUNC_InitParticles = 0xC4;

    pParticleSystem->Init();
}

CParticleSystemSA::~CParticleSystemSA()
{
    pParticleSystem->Shutdown();
}