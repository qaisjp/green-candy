/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        game_sa/CAtomicModelInfoSA.h
*  PURPOSE:     Atomic model instance
*  DEVELOPERS:  The_GTA <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#ifndef _CAtomicModelInfoSA_
#define _CAtomicModelInfoSA_

class CAtomicModelInfoSA : public CBaseModelInfoSAInterface
{
public:
    virtual void __thiscall             SetAtomic( RpAtomic *atomic );

    RpAtomic*                           m_rpAtomic;     // 28
};

class CDamageAtomicModelInfoSA : public CAtomicModelInfoSA
{
public:
    void                                SetupPipeline( RpAtomic *link );

    RpAtomic*                           m_atomicLink;
};

class CLodAtomicModelInfoSA : public CAtomicModelInfoSA
{
public:

};

class CTimeModelInfoSA : public CAtomicModelInfoSA
{
public:

};

class CLodTimeModelInfoSA : public CLodAtomicModelInfoSA
{
public:

};

#endif //_CAtomicModelInfoSA_