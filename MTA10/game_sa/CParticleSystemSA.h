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

#define FUNC_InitParticles              0x0049EA90

typedef CAlignedStackSA <0x100000> CEffectStackSA;

class CEffectDataSAInterface;
class CEffectDefSAInterface;

class CParticleSystemSAInterface
{
public:
    void                                    Init();
    void                                    Shutdown();
    bool                                    LoadDefinitions( const char *filename );

    void                                    SetGlobalAssociatives( const CVector *windVelocity, const float *unk );

    CParticleObjectSAInterface*             CreateTranslator( const char *name, const CVector& pos, unsigned int unk, bool unk2 );
    CParticleObjectSAInterface*             CreateTranslator( CEffectDefSAInterface *data, const CVector& pos, unsigned int unk, bool unk2 );
    CEffectDefSAInterface*                  GetBlueprintByName( const char *name );

    CSimpleList                             m_defList;      // 0
    BYTE                                    m_pad[12];      // 12
    CEffectDataSAInterface*                 m_effects;      // 24
    CSimpleList                             m_effectList;   // 28

    int                                     m_txdID;        // 40
    const CVector*                          m_windVelocity; // 44
    const float*                            m_unk;          // 48
    BYTE                                    m_pad2[84];     // 52
    unsigned int                            m_count;        // 136
    RwMatrix*                               m_matrices[8];  // 140
    CEffectStackSA                          m_memory;       // 172

private:
    CEffectDefSAInterface*                  ParseFXDataDef( const char *filename, CFile *file );
};

class CParticleSystemSA
{
public:
                                            CParticleSystemSA();
                                            ~CParticleSystemSA();

    void                                    Init();
    void                                    Shutdown();
};

extern CParticleSystemSAInterface *const pParticleSystem;

#endif //_CParticleSystemSA_