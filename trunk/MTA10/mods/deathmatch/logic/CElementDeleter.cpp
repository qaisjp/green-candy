/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        mods/deathmatch/logic/CElementDeleter.cpp
*  PURPOSE:     Manager for elements to be destroyed
*  DEVELOPERS:  Jax <>
*               Christian Myhre Lundheim <>
*               Cecill Etheredge <ijsf@gmx.net>
*               The_GTA <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#include "StdInc.h"

using std::list;

CElementDeleter::CElementDeleter()
{
}

void CElementDeleter::Delete( class CClientEntity* pElement )
{
    pElement->Delete();

    // Flag it as being deleted and unlink it from the tree/managers
    pElement->SetParent( NULL );
    pElement->Unlink();
}

void CElementDeleter::Unreference( class CClientEntity* pElement )
{
    m_List.remove( pElement );
}