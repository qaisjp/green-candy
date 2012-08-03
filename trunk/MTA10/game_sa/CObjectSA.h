/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        game_sa/CObjectSA.h
*  PURPOSE:     Header file for object entity class
*  DEVELOPERS:  Ed Lyons <eai@opencoding.net>
*               Jax <>
*               Cecill Etheredge <ijsf@gmx.net>
*               The_GTA <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#ifndef __CGAMESA_OBJECT
#define __CGAMESA_OBJECT

#include <game/CObject.h>
#include "CPhysicalSA.h"


#define FUNC_CObject_Create             0x5A1F60
#define FUNC_CObject_Explode            0x5A1340

class CObjectSAInterface : public CPhysicalSAInterface // + 372 = burn time stop , +348 = scale // +340 = health
{
public:
    void*   operator new( size_t );
    void    operator delete( void *ptr );

    BYTE                        m_pad4[4];              // 312
    unsigned char               m_unk40;                // 316
    BYTE                        m_pad2[23];             // 317
    float                       m_health;               // 340
    DWORD                       m_pad3;                 // 344
    float                       m_scale;                // 348

    BYTE                        m_pad[60];              // 352
};

class CObjectSA : public virtual CObject, public CPhysicalSA
{
public:
                                CObjectSA( CObjectSAInterface *obj );
                                ~CObjectSA();

    void* operator new ( size_t );
    void operator delete ( void *ptr );

    inline CObjectSAInterface*  GetInterface()                                  { return (CObjectSAInterface*)m_pInterface; }
    inline const CObjectSAInterface*    GetInterface() const                    { return (const CObjectSAInterface*)m_pInterface; }

    unsigned int                GetPoolIndex() const                            { return m_poolIndex; }

    void                        Explode();
    void                        Break();
    void                        SetScale( float scale )                         { GetInterface()->m_scale = scale; }
    float                       GetScale() const                                { return GetInterface()->m_scale; }
    void                        SetHealth( float fHealth )                      { GetInterface()->m_health = fHealth; }
    float                       GetHealth() const                               { return GetInterface()->m_health; }
    void                        SetModelIndex( unsigned short ulModel );

    inline void                 SetAlpha( unsigned char ucAlpha )               { m_ucAlpha = ucAlpha; }
    inline unsigned char        GetAlpha() const                                { return m_ucAlpha; }

    bool                        IsAGangTag() const                              { return m_bIsAGangTag; }

private:
    void                        CheckForGangTag();

    unsigned char               m_ucAlpha;
    bool                        m_bIsAGangTag;
    unsigned int                m_poolIndex;
};

/*
#define COBJECTSA_DEFINED
#define COBJECTSAINTERFACE_DEFINED
*/

#endif
