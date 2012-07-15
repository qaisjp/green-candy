/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.0
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        sdk/game/CPad.h
*  PURPOSE:     Controller pad interface
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#ifndef __CGAME_PAD
#define __CGAME_PAD

// NOTE: This is an internal interface!
class CControllerState
{
public:
    CControllerState()
    {
        memset( this, 0, sizeof(CControllerState) );
    }

    inline void ResetLeftAxis()
    {
        m_leftAxisX = 0;
        m_leftAxisY = 0;
    }

    inline void SetSquareDown( bool down )
    {
        m_action1 = down ? 0x100 : 0;
    }

    inline bool IsSquareDown()
    {
        return m_action1 > 0x80;
    }

    inline void SetTriangleDown( bool down )
    {
        m_action2 = down ? 0x100 : 0;
    }

    inline bool IsTriangleDown()
    {
        return m_action2 > 0x80;
    }

    inline void SetCrossDown( bool down )
    {
        m_action3 = down ? 0x100 : 0;
    }

    inline bool IsCrossDown()
    {
        return m_action3 > 0x80;
    }

    inline void SetCircleDown( bool down )
    {
        m_action4 = down ? 0x100 : 0;
    }

    inline void SetLeftShockDown( bool down )
    {
        m_action5 = down ? 0x100 : 0;
    }

    inline void SetRightShockDown( bool down )
    {
        m_action6 = down ? 0x100 : 0;
    }

    inline bool IsCircleDown()
    {
        return m_action4 > 0x80;
    }

    inline bool IsLeftShoulder1Down()
    {
        return m_ls1 == 0xFF;
    }

    inline bool IsLeftShoulder2Down()
    {
        return m_ls2 == 0xFF;
    }

    inline bool IsRightShoulder1Down()
    {
        return m_rs1 == 0xFF;
    }

    inline bool IsRightShoulder2Down()
    {
        return m_rs2 == 0xFF;
    }

    short   m_leftAxisX;            // 0, move/steer left (-128?)/right (+128)
    short   m_leftAxisY;            // 2, move back(+128)/forwards(-128?)
    short   m_rightAxisX;           // 4, numpad 6(+128)/numpad 4(-128?)
    short   m_rightAxisY;           // 6
    
    short   m_ls1;                  // 8
    short   m_ls2;                  // 10
    short   m_rs1;                  // 12, target / hand brake
    short   m_rs2;                  // 14
    
    short   m_digitalUp;            // 16, radio change up
    short   m_digitalDown;          // 18, radio change down
    short   m_digitalLeft;          // 20
    short   m_digitalRight;         // 22

    short   m_start;                // 24
    short   m_select;               // 26

    short   m_action1;              // 28, jump / reverse
    short   m_action2;              // 30, get in/out
    short   m_action3;              // 32, sprint / accelerate
    short   m_action4;              // 34, fire

    short   m_action5;              // 36
    short   m_action6;              // 38, look behind

    short   m_chatIndicated;        // 40
    short   m_pedWalk;              // 42
    short   m_vehicleMouseLook;     // 44
    short   m_radioTrackSkip;       // 46
};

class CPad
{
public:
    virtual const CControllerState&     GetState() = 0;
    virtual const CControllerState&     GetPreviousState() = 0;
    virtual void                        SetState( const CControllerState& cs ) = 0;
    virtual void                        InjectCurrent( const CControllerState& cs ) = 0;

    virtual void                        Restore() = 0;
    virtual void                        Store() = 0;
    virtual bool                        IsEnabled() = 0;
    virtual void                        Disable( bool bDisable ) = 0;
    virtual void                        Clear() = 0;
    virtual void                        SetHornHistoryValue( bool value ) = 0;
    virtual long                        GetAverageWeapon() = 0;
    virtual void                        SetLastTimeTouched( long time ) = 0;
};

#endif
