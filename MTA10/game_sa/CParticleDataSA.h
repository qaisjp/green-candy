/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        game_sa/CParticleDataSA.h
*  PURPOSE:     Particle definition
*  DEVELOPERS:  The_GTA <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#ifndef _CParticleDataSA_
#define _CParticleDataSA_

enum ePartObjectStatus
{
    PARTOBJ_STATUS_STOPPED = 2,

    ___FORCE_DWORD = 0xFFFFFFFF
};

class CEffectDataSAInterface : public CSimpleList::Item
{
public:
};

class CParticleDataSAInterface   // 60 bytes
{
public:
                                                CParticleDataSAInterface();
    virtual                                     ~CParticleDataSAInterface();

    virtual void __thiscall                     Unk();
    virtual void __thiscall                     Unk2();
    virtual void* __thiscall                    Create();

    unsigned char               m_unk4;                 // 4

    BYTE                        m_pad[3];               // 5
    
    CQuat                       m_rotation;             // 12

    BYTE                        m_pad2[4];              // 28
    CVector                     m_pos;                  // 32

    BYTE                        m_pad3[16];             // 44
};

#endif //_CParticleDataSA_