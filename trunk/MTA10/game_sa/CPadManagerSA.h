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

class CPadManagerSA
{
public:
                            CPadManagerSA();
                            ~CPadManagerSA();

    CPadSAInterface*        GetJoypad( unsigned int index );
};

#endif //_CPadManagerSA_