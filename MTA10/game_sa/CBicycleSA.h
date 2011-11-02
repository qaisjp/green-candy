/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        game_sa/CBicycleSA.h
*  PURPOSE:     Header file for bmx bike vehicle entity class
*  DEVELOPERS:  Ed Lyons <eai@opencoding.net>
*               The_GTA <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#ifndef __CGAMESA_BMX
#define __CGAMESA_BMX

#include <game/CBmx.h>

class CBicycleSAInterface : public CAutomobileSAInterface
{
public:
    // fill this
};

class CBicycleSA : public virtual CBicycle, public virtual CAutomobileSA
{
private:
public:
                                CBicycleSA( CBicycleSAInterface *bike );
                                CBicycleSA( unsigned short mode );

    virtual                     ~CBicycleSA ( void ) {};
};

#endif
