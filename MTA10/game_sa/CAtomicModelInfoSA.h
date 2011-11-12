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

class CAtomicModelInfoSAInterface : public CBaseModelInfoSAInterface
{
public:
    virtual void __thiscall             SetAtomic( RpAtomic *atomic );

    RpAtomic*                           m_rpAtomic;     // 28
};

#endif //_CAtomicModelInfoSA_