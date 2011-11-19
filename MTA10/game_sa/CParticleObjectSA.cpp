/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.0
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        game_sa/CParticleObjectSA.cpp
*  PURPOSE:     Particle definitions
*  DEVELOPERS:  
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#include "StdInc.h"

CParticleObjectSAInterface::CParticleObjectSAInterface()
{
    new (&m_pos) CVector();

    m_unk4 = 0;
}

CParticleObjectSAInterface::~CParticleObjectSAInterface()
{
}

CParticleSAInterface* CParticleObjectSAInterface::Create()
{
    return new CParticleSAInterface();
}

void CParticleObjectSAInterface::Stop()
{

}

void CParticleObjectSAInterface::KillParticles()
{
    
}