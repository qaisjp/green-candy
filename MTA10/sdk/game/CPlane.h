/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        sdk/game/CPlane.h
*  PURPOSE:     Plane vehicle entity interface
*  DEVELOPERS:  The_GTA <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#ifndef __CGAME_PLANE
#define __CGAME_PLANE

#include "CVehicle.h"

class CPlane : public virtual CAutomobile
{
public:
    virtual                     ~CPlane()   {}

    virtual void                SetLandingGearDown( bool bLandingGearDown ) = 0;
    virtual float               GetLandingGearPosition() = 0;
    virtual bool                IsLandingGearDown() = 0;
    virtual void                SetLandingGearPosition( float fPosition ) = 0;
};

#endif
