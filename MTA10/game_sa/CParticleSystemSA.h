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

template < size_t heapSize >
class CAlignedStackSA
{
public:
    CAlignedStackSA()
    {
        m_data = pRwInterface->m_malloc( heapSize );
        m_size = heapSize;
        m_offset = 0;

        memset( m_data, 0, m_size );
    }

    ~CAlignedStackSA()
    {
        pRwInterface->m_free( m_data );
    }

    void*   Allocate( size_t size, size_t align )
    {
        size_t remains = m_offset % elementSize;
        void *alloc;

        // Realign the offset
        if ( remains )
            m_offset += align - remains;

        // Bugfix
        if ( m_offset + size > m_size )
            return NULL;

        alloc = (void*)((unsigned char*)m_data + m_offset);
        m_offset += size;

        return alloc;
    }

    void*   AllocateInt( size_t size )
    {
        return Push( size, sizeof(int) );
    }

    void*                                   m_data;
    size_t                                  m_size;
    size_t                                  m_offset;
};

typedef CAlignedStackSA < 0x100000 > CParticleDataStackSA;

class CParticleControlSAInterface
{
public:
                                            CParticleControlSAInterface();
                                            ~CParticleControlSAInterface();

    CParticleObjectSAInterface*             m_data;         // 0
    size_t                                  m_size;         // 4
    unsigned int                            m_count;        // 8
};

class CParticleSystemSAInterface
{
public:
    void                                    Init();
    void                                    Shutdown();

    BYTE                                    m_pad[136];     // 0
    unsigned int                            m_count;        // 136
    RwMatrix*                               m_matrices[8];  // 140
    CParticleDataStackSA                    m_particles;    // 172
};

class CParticleSystemSA
{
public:
                                            CParticleSystemSA();
                                            ~CParticleSystemSA();
};

extern CParticleSystemSAInterface *pParticleSystem;

#endif //_CParticleSystemSA_