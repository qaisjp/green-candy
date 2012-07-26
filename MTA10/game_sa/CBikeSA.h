/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        game_sa/CBikeSA.h
*  PURPOSE:     Header file for bike vehicle entity class
*  DEVELOPERS:  Ed Lyons <eai@opencoding.net>
*               The_GTA <quiret@gmx.e>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#ifndef __CGAMESA_BIKE
#define __CGAMESA_BIKE

#include <game/CBike.h>

#define FUNC_CBike_Fix                              0x6B7050
#define FUNC_CBike_PlaceOnRoadProperly              0x6BEEB0

class CBikeSAInterface : public CVehicleSAInterface // size: 2068
{
public:
    virtual void __thiscall         unk()   {}

    BYTE                        m_pad20[188];                           // 1440

    unsigned char               m_wheelStatus[2];                       // 1628

    BYTE                        m_pad23[306];                           // 1630
    float                       m_modelOffset;                          // 1936

    BYTE                        m_pad21[112];                           // 1940
    unsigned char               m_numContactWheels;                     // 2052

    BYTE                        m_pad22[15];                            // 2053
};

class CBikeSA : public virtual CBike, public virtual CVehicleSA
{
public:
                                    CBikeSA( CBikeSAInterface *bike );
                                    ~CBikeSA();

    inline CBikeSAInterface*        GetInterface()                          { return (CBikeSAInterface*)m_pInterface; }

    void                            PlaceOnRoadProperly();

    unsigned char                   GetBikeWheelStatus( unsigned char wheel );
    void                            SetBikeWheelStatus( unsigned char wheel, unsigned char status );
};

#endif
