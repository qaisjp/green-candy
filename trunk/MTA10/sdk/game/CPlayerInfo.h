/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.0
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        sdk/game/CPlayerInfo.h
*  PURPOSE:     Player entity information interface
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#ifndef __CGAME_PLAYERINFO
#define __CGAME_PLAYERINFO

#include "Common.h"
#include "CWanted.h"
#include "CVehicle.h"

class CPlayerInfo
{
public:
    virtual long            GetPlayerMoney                  () = 0;
    virtual void            SetPlayerMoney                  ( long lMoney ) = 0;

    virtual void            GivePlayerParachute             () = 0;
    virtual void            StreamParachuteWeapon           ( bool allow ) = 0;

    virtual unsigned short  GetLastTimeEaten                () = 0;
    virtual void            SetLastTimeEaten                ( unsigned short time ) = 0;

    virtual CWanted*        GetWanted                       () = 0;
    virtual float           GetFPSMoveHeading               () = 0;

    virtual bool            GetCrossHair                    ( float& tarX, float& tarY ) = 0;

    virtual bool            GetDoesNotGetTired              () = 0;
    virtual void            SetDoesNotGetTired              ( bool award ) = 0;

    virtual CVehicle*       GiveRemoteVehicle               ( unsigned short model, float x, float y, float z ) = 0;
    virtual void            StopRemoteControl               () = 0;
    virtual CVehicle*       GetRemoteVehicle                () = 0;

    virtual unsigned int    GetLastTimeBigGunFired          () = 0;
    virtual void            SetLastTimeBigGunFired          ( unsigned int time ) = 0;

    virtual unsigned int    GetCarTwoWheelCounter           () = 0;
    virtual float           GetCarTwoWheelDist              () = 0;
    virtual unsigned int    GetCarLess3WheelCounter         () = 0;
    virtual unsigned int    GetBikeRearWheelCounter         () = 0;
    virtual float           GetBikeRearWheelDist            () = 0;
    virtual unsigned int    GetBikeFrontWheelCounter        () = 0;
    virtual float           GetBikeFrontWheelDist           () = 0;
};

#endif
