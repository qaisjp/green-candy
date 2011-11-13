/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        game_sa/CPadManagerSA.cpp
*  PURPOSE:     Joypad management
*  DEVELOPERS:  The_GTA <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#include "StdInc.h"

CPadManagerSA::CPadManagerSA()
{

}

CPadManagerSA::~CPadManagerSA()
{

}

CPadSAInterface* CPadManagerSA::GetJoypad( unsigned int index )
{
    if ( index != 0 )
        return NULL;

    return (CPadSAInterface*)0x00B73458;
}