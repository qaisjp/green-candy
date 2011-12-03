/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        game_sa/CPadManagerSA.h
*  PURPOSE:     Joypad management
*  DEVELOPERS:  The_GTA <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#ifndef _CPadManagerSA_
#define _CPadManagerSA_

#define DIGITAL_BUTTON( state ) ( state ? 255 : 0 )
#define DIGITAL_AXIS( upState, downState ) ( ( ( (upState) && (downState) ) || !(upState) && !(downState) ) ? 0 : (upState) ? -128 : 128 )

class CPadManagerSA
{
public:
                            CPadManagerSA();
                            ~CPadManagerSA();

    CPadSAInterface*        GetJoypad( unsigned int index );

    void                    GetFootControl( CControllerState& cs );
    void                    GetVehicleControl( CControllerState& cs );
    void                    UpdateLocalJoypad( CPadSAInterface& pad );
private:
    CKeyBindsInterface*     m_keys;
};

#endif //_CPadManagerSA_