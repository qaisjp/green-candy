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
                                CClientVehicleComponent( CClientVehicle *veh, CVehicleComponent *atomic );
                                ~CClientVehicleComponent();

    CClientVehicle*             GetVehicle()                        { return m_vehicle; }

    unsigned int                AddAtomic( CClientAtomic *atom );
    unsigned int                GetAtomicCount() const;
    bool                        RemoveAtomic( unsigned int idx );

    CClientVehicle*             m_vehicle;
    CVehicleComponent*          m_component;

protected:
    struct atomicInfo
    {
        CClientAtomic *atomic;
        unsigned int idx;
    };

    void                        OffsetIndex( unsigned int start, int off );

    typedef std::vector <atomicInfo> atomics_t;

    atomics_t                   m_atomics;   // Kinda: Special modelinfo structures
};

#endif //_VEHICLE_COMPONENT_INTERFACE_