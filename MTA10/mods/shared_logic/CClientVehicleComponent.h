/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*               (Shared logic for modifications)
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        mods/shared_logic/CClientVehicleComponent.h
*  PURPOSE:     Vehicle component class export
*  DEVELOPERS:  The_GTA <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#ifndef _VEHICLE_COMPONENT_INTERFACE_
#define _VEHICLE_COMPONENT_INTERFACE_

#define LUACLASS_VEHICLECOMPONENT   98

class CClientVehicle;

class CClientVehicleComponent : public LuaElement
{
public:
                                CClientVehicleComponent( CClientVehicle *veh, unsigned int idx, CVehicleComponent *atomic );
                                ~CClientVehicleComponent();

    CClientVehicle*             GetVehicle()                { return &(CClientVehicle&)m_root; }

    CVehicleComponent*          m_component;

protected:
    unsigned int                m_idx;
};

#endif //_VEHICLE_COMPONENT_INTERFACE_