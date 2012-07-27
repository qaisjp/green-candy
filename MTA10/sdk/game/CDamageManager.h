/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.0
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        sdk/game/CDamageManager.h
*  PURPOSE:     Damage manager interface
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#ifndef __CGAME_DAMAGEMANAGER
#define __CGAME_DAMAGEMANAGER

// Not sure if R* used these
#define DT_ENGINE_OK                    (0)
#define DT_ENGINE_RADIATOR_BURST        (100)
#define DT_ENGINE_ENGINE_OVERHEAT       (150)
#define DT_ENGINE_ENGINE_PIPES_BURST    (200)
#define DT_ENGINE_ON_FIRE               (225)
#define DT_ENGINE_FUCKED                (250)

/*
const unsigned char DT_ENG_INOPERABLE = 0x01;
const unsigned char DT_ENG_EXHAUST_SMOKE = 0x02;
const unsigned char DT_ENG_RADIATOR_STEAM = 0x04;
const unsigned char DT_ENG_ON_FIRE = 0x08;
*/

enum eWheelStatus
{
    DT_WHEEL_INTACT,
//  DT_WHEEL_CAP_MISSING,
//  DT_WHEEL_WARPED,
    DT_WHEEL_BURST,
    DT_WHEEL_MISSING,

    // MTA custom state
    DT_WHEEL_INTACT_COLLISIONLESS,
};

enum eDoorStatus
{
    DT_DOOR_INTACT,
    DT_DOOR_SWINGING_FREE,
    DT_DOOR_BASHED,
    DT_DOOR_BASHED_AND_SWINGING_FREE,
    DT_DOOR_MISSING
};

enum ePlaneComponentStatus 
{
    DT_PLANE_INTACT,
    DT_PLANE_BASHED,
//  DT_PLANE_BASHED2,
    DT_PLANE_MISSING
};

enum eComponentStatus 
{
    DT_PANEL_INTACT,
//  DT_PANEL_SHIFTED,
    DT_PANEL_BASHED,
    DT_PANEL_BASHED2,
    DT_PANEL_MISSING
};

enum eLightStatus 
{
    DT_LIGHT_OK,
    DT_LIGHT_SMASHED
};

enum eDoors
{
    BONNET = 0,
    BOOT,
    FRONT_LEFT_DOOR,
    FRONT_RIGHT_DOOR,
    REAR_LEFT_DOOR,
    REAR_RIGHT_DOOR,
    MAX_DOORS
};

enum eWheels
{
    FRONT_LEFT_WHEEL = 0,
    REAR_LEFT_WHEEL,
    FRONT_RIGHT_WHEEL,
    REAR_RIGHT_WHEEL,

    MAX_WHEELS

};

enum ePanels
{
    FRONT_LEFT_PANEL = 0,
    FRONT_RIGHT_PANEL,
    REAR_LEFT_PANEL,
    REAR_RIGHT_PANEL,
    WINDSCREEN_PANEL,   // needs to be in same order as in component.h
    FRONT_BUMPER,
    REAR_BUMPER,

    MAX_PANELS      // MUST BE 8 OR LESS
};

enum eLights
{
    // these have to correspond to their respective panels
    LEFT_HEADLIGHT = 0,
    RIGHT_HEADLIGHT,
    LEFT_TAIL_LIGHT,
    RIGHT_TAIL_LIGHT,
/*  LEFT_BRAKE_LIGHT,
    RIGHT_BRAKE_LIGHT,
    FRONT_LEFT_INDICATOR,
    FRONT_RIGHT_INDICATOR,
    REAR_LEFT_INDICATOR,
    REAR_RIGHT_INDICATOR,*/

    MAX_LIGHTS          // MUST BE 16 OR LESS
};


class CDamageManager
{
public:
    virtual unsigned char       GetEngineStatus() const = 0;
    virtual void                SetEngineStatus( unsigned char status ) = 0;

    virtual unsigned char       GetDoorStatus( eDoors bDoor ) const = 0;
    virtual void                SetDoorStatus( eDoors bDoor, unsigned char status ) = 0;

    virtual unsigned char       GetWheelStatus( eWheels bTire ) const = 0;
    virtual void                SetWheelStatus( eWheels bTire, unsigned char status ) = 0;

    virtual unsigned char       GetPanelStatus( unsigned char panel ) const = 0;
    virtual unsigned long       GetPanelStatus() const = 0;
    virtual void                SetPanelStatus( unsigned char panel, unsigned char status ) = 0;
    virtual void                SetPanelStatus( unsigned long ulStatus ) = 0;

    virtual unsigned char       GetLightStatus( unsigned char light ) const = 0;
    virtual unsigned char       GetLightStatus() const = 0;
    virtual void                SetLightStatus( unsigned char light, unsigned char status ) = 0;
    virtual void                SetLightStatus( unsigned char ucStatus ) = 0;

    virtual unsigned char       GetAeroplaneCompStatus( unsigned char id ) const = 0;
    virtual void                SetAeroplaneCompStatus( unsigned char id, unsigned char status ) = 0; //component ids begin at 12 - probably

    virtual void                FuckCarCompletely( bool keepWheels ) = 0;
};

#endif
