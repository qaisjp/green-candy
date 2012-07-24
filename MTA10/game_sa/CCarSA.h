/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        game_sa/CCarSA.h
*  PURPOSE:     Car vehicle entity header
*  DEVELOPERS:  The_GTA <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#ifndef _CCarSA_H_
#define _CCarSA_H_

#include <game/CCar.h>

class CCarSAInterface : public CAutomobileSAInterface   // size: 2460
{
public:
    BYTE                        m_pad12[20];                            // 2440
};

class CCarSA : public virtual CCar, public CAutomobileSA
{
public:
    
};

#endif //_CCarSA_H_