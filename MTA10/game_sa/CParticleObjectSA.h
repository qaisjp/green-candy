/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        game_sa/CParticleObjectSA.h
*  PURPOSE:     Header file for particle object entity class
*  DEVELOPERS:  The_GTA <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#ifndef __CGAMESA_PARTICLEOBJECT
#define __CGAMESA_PARTICLEOBJECT

#include <game/CParticleObject.h>

#define FUNC_InitParticles          0x0049EA90

enum ePartObjectStatus
{
    PARTOBJ_STATUS_STOPPED = 2,

    FORCE_DWORD = 0xFFFFFFFF
};

class CParticleObjectSAInterface
{
public:
                                                CParticleObjectSAInterface();
    virtual                                     ~CParticleObjectSAInterface();

    virtual void __thiscall                     Unk();
    virtual void __thiscall                     Unk2();
    virtual CParticleSAInterface* __thiscall    Create();

    void                                        Stop();
    void                                        DestroyParticles();

    unsigned char               m_unk4;                 // 4

    BYTE                        m_pad[3];               // 5
    
    CQuat                       m_rotation;             // 12

    BYTE                        m_pad2[4];              // 28
    CVector                     m_pos;                  // 32

    BYTE                        m_pad3[36];             // 44
    unsigned char               m_unk;                  // 80
    unsigned char               m_unk2;                 // 81

    BYTE                        m_pad4[2];              // 82
    unsigned int                m_unk3;                 // 84
};

class CParticleObjectSA : public CParticleObject
{
private:
//  CParticleObjectSAInterface      * internalInterface;
public:
};

#endif
