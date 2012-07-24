/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        game_sa/CPlaneSA.h
*  PURPOSE:     Header file for plane vehicle entity class
*  DEVELOPERS:  Ed Lyons <eai@opencoding.net>
*               Christian Myhre Lundheim <>
*               The_GTA <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#ifndef __CGAMESA_PLANE
#define __CGAMESA_PLANE

#include <game/CPlane.h>

class CPlaneSAInterface : public CAutomobileSAInterface // size: 2564
{
public:
    BYTE                        m_pad40[64];                            // 2440

    float                       m_undercarriagePos;                     // 2508
    BYTE                        m_pad41[48];                            // 2512

    bool                        m_smokeTrailEnabled;                    // 2560

    BYTE                        m_alignPad[3];                          // 2561

    // + 2508 = undercarrige possition (float - 1.0 = up, 0.0 = down)
    // fill this
};

class CPlaneSA : public virtual CPlane, public virtual CAutomobileSA
{
public:
                                CPlaneSA( CPlaneSAInterface *plane );
                                CPlaneSA( unsigned short model );
                                ~CPlaneSA();

    inline CPlaneSAInterface*   GetInterface()                              { return (CPlaneSAInterface*)m_pInterface; }

    void                        SetLandingGearDown( bool bLandingGearDown );
    float                       GetLandingGearPosition();
    void                        SetLandingGearPosition( float fPosition );
    bool                        IsLandingGearDown();

    bool                        IsSmokeTrailEnabled();
    void                        SetSmokeTrailEnabled( bool enabled );
};

#endif
