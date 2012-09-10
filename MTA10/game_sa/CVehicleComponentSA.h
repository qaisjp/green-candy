/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        game_sa/CVehicleComponentSA.h
*  PURPOSE:     Vehicle component part interface
*  DEVELOPERS:  The_GTA <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#ifndef _VEHICLE_COMPONENT_SA_
#define _VEHICLE_COMPONENT_SA_

class CVehicleComponentSA : public CVehicleComponent
{
public:
                                CVehicleComponentSA( CVehicleComponentSA*& slot, RpAtomic *atomic );
                                ~CVehicleComponentSA();

    void                        SetMatrix( const RwMatrix& mat );
    const RwMatrix&             GetMatrix() const;
    const RwMatrix&             GetWorldMatrix() const;

    void                        SetPosition( const CVector& pos );
    const CVector&              GetPosition() const;
    const CVector&              GetWorldPosition() const;

    RpAtomic*                   m_atomic;
    CVehicleComponentSA*&       m_compSlot;
};

#endif //_VEHICLE_COMPONENT_SA_