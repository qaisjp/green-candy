/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        mods/deathmatch/logic/CDeathmatchVehicle.h
*  PURPOSE:     Header for deathmatch vehicle class
*  DEVELOPERS:  Christian Myhre Lundheim <>
*               Jax <>
*               The_GTA <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#ifndef __CDEATHMATCHVEHICLE_H
#define __CDEATHMATCHVEHICLE_H

#include "CClientVehicle.h"

// The_GTA: The problem with this class is that it directly inherits CClientVehicle. Therefor we are prohibited from direct inheritance of CClientVehicle.
// I instead inherit from CDeathmatchVehicle, effectively mixing up deathmatch logic with shared logic. It cannot be helped if this stays this way!

class CDeathmatchVehicle : public CClientVehicle
{
public:
                                    CDeathmatchVehicle( CClientManager* pManager, LuaClass& root, bool system, class CUnoccupiedVehicleSync* pUnoccupiedVehicleSync, ElementID ID, unsigned short usVehicleModel );
                                    ~CDeathmatchVehicle( void );

    inline bool                     IsSyncing( void )                               { return m_bIsSyncing; };
    void                            SetIsSyncing( bool bIsSyncing );

    bool                            SyncDamageModel( void );
    void                            ResetDamageModelSync( void );

private:
    class CUnoccupiedVehicleSync*   m_pUnoccupiedVehicleSync;
    bool                            m_bIsSyncing;

    unsigned char                   m_ucLastDoorStates [MAX_DOORS];
    unsigned char                   m_ucLastWheelStates [MAX_WHEELS];
    unsigned char                   m_ucLastPanelStates [MAX_PANELS];
    unsigned char                   m_ucLastLightStates [MAX_LIGHTS];
};

#endif
