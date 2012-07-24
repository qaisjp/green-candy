/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        game_sa/CHeliSA.h
*  PURPOSE:     Header file for plane vehicle entity class
*  DEVELOPERS:  The_GTA <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#ifndef _CHeliSA_H_
#define _CHeliSA_H_

#include <game/CHeli.h>

class CHeliSAInterface : public CAutomobileSAInterface
{
public:
    BYTE                        m_pad40[137];                           // 2440
    bool                        m_searchLightVisible;                   // 2577
};

class CHeliSA : public virtual CHeli, public CAutomobileSA
{
public:
                                CHeliSA( unsigned short modelId );
                                ~CHeliSA();

    inline CHeliSAInterface*    GetInterface()                                      { return (CHeliSAInterface*)m_pInterface; }

    void                        SetWinchType( eWinchType winchType );
    void                        PickupEntityWithWinch( CEntity* pEntity );
    void                        ReleasePickedUpEntityWithWinch();
    void                        SetRopeHeightForHeli( float fRopeHeight );
    CPhysical*                  QueryPickedUpEntityWithWinch();

    bool                        IsHeliSearchLightVisible();
    void                        SetHeliSearchLightVisible( bool bVisible );

    float                       GetHeliRotorSpeed()                                 { return GetInterface()->m_rotorSpeed; }
    void                        SetHeliRotorSpeed( float fSpeed )                   { GetInterface()->m_rotorSpeed = fSpeed; }
};

#endif //_CHeliSA_H_