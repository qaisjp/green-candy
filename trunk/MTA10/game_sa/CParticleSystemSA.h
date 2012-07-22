/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        game_sa/CParticleSystemSA.h
*  PURPOSE:     Particle loading and management
*  DEVELOPERS:  The_GTA <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#ifndef _CParticleSystemSA_
#define _CParticleSystemSA_

#define MAX_PARTICLE_DATA               1000

#define CLASS_CParticleSystem           0x00A9AE80

typedef CAlignedStackSA <0x100000> CEffectStackSA;

class CEffectDataSA;

class CParticleSystemSAInterface
{
public:
    void                                    Init();
    void                                    Shutdown();
    void                                    LoadDefinitions( const char *filename );

    BYTE                                    m_pad[24];      // 0
    CEffectDataSA*                          m_effects;      // 24
    CSimpleList                             m_effectList;   // 28

    BYTE                                    m_pad2[96];     // 40
    unsigned int                            m_count;        // 136
    RwMatrix*                               m_matrices[8];  // 140
    CEffectStackSA                          m_memory;       // 172
};

class CParticleSystemSA
{
public:
                                            CParticleSystemSA();
                                            ~CParticleSystemSA();
};

extern CParticleSystemSAInterface *pParticleSystem;

#endif //_CParticleSystemSA_