/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        sdk/game/CVehicleComponent.h
*  PURPOSE:     Vehicle component part interface
*  DEVELOPERS:  The_GTA <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#ifndef _VEHICLE_COMPONENT_
#define _VEHICLE_COMPONENT_

class CVehicleComponent abstract
{
public:
    virtual                     ~CVehicleComponent()    {}

    virtual void                SetMatrix( const RwMatrix& mat ) = 0;
    virtual const RwMatrix&     GetMatrix() const = 0;
    virtual const RwMatrix&     GetWorldMatrix() const = 0;

    virtual void                SetPosition( const CVector& pos ) = 0;
    virtual const CVector&      GetPosition() const = 0;
    virtual const CVector&      GetWorldPosition() const = 0;
};

#endif //_VEHICLE_COMPONENT_