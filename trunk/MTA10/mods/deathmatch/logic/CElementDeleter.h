/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        mods/deathmatch/logic/CElementDeleter.h
*  PURPOSE:     Header for element deleter class
*  DEVELOPERS:  Jax <>
*               Christian Myhre Lundheim <>
*               Cecill Etheredge <ijsf@gmx.net>
*               The_GTA <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#ifndef __CELEMENTDELETER_H
#define __CELEMENTDELETER_H

class CElementDeleter
{
public:
                                CElementDeleter();

    void                        Delete( class CClientEntity* pElement );
    void                        Unreference( class CClientEntity* pElement );

private:
    CMappedList < class CClientEntity* >    m_List;
    bool                                    m_bAllowUnreference;
};

#endif
