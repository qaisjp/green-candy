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

static void InitParticleData( CParticleObjectSAInterface *block )
{
    new (block) CParticleObjectSAInterface();
}

void CParticleSystemSAInterface::Init()
{
    unsigned int n;
    CParticleObjectSAInterface *data;

    new (&m_particles) CParticleDataStackSA();

    m_count = 0;

    for (n=0; n<8; n++)
        m_matrices[n] = pGame->GetRenderWare()->AllocateMatrix();

    // Allocate from the stack (we leave out the alloc count though)
    data = m_particles.AllocateInt( sizeof( CParticleObjectSAInterface ) * MAX_PARTICLE_DATA );

    ForEachBlock( data, MAX_PARTICLE_DATA, sizeof( CParticleObjectSAInterface ), InitParticleData );
}

void CParticleSystemSAInterface::Shutdown()
{

}

CParticleSystemSA::CParticleSystemSA()
{
    // Do not let GTA SA load particles
    *(unsigned char*)FUNC_InitParticles = 0xC4;

    pParticleSystem->Init();
}

CParticleSystemSA::~CParticleSystemSA()
{
    pParticleSystem->Shutdown();
}