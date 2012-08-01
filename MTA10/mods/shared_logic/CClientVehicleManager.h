/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.0
*               (Shared logic for modifications)
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        mods/shared_logic/CClientVehicleManager.h
*  PURPOSE:     Vehicle entity manager class header
*  DEVELOPERS:  Christian Myhre Lundheim <>
*               Ed Lyons <eai@opencoding.net>
*               Jax <>
*               Kevin Whiteside <kevuwk@gmail.com>
*               Cecill Etheredge <ijsf@gmx.net>
*
*****************************************************************************/

#ifndef __CCLIENTVEHICLEMANAGER_H
#define __CCLIENTVEHICLEMANAGER_H

#include "CClientCommon.h"
#include "CClientVehicle.h"

class CClientManager;
class CClientVehicle;

extern unsigned char g_ucMaxPassengers [];

class CClientVehicleManager
{
    friend class CClientManager;
    friend class CClientVehicle;

public:
    void                            DeleteAll               ( void );

    inline unsigned int             Count                   ( void )                        { return static_cast < unsigned int > ( m_List.size () ); };
    static CClientVehicle*          Get                     ( ElementID ID );
    CClientVehicle*                 Get                     ( CVehicle* pVehicle, bool bValidatePointer );
    CClientVehicle*                 GetSafe                 ( CEntity * pEntity );
    CClientVehicle*                 GetClosest              ( CVector& vecPosition, float fRadius );

    void                            DoPulse                 ( void );

    static bool                     IsValidModel            ( unsigned short model );
    static eClientVehicleType       GetVehicleType          ( unsigned short ulModel );
    static unsigned char            GetMaxPassengerCount    ( unsigned short ulModel );
    static unsigned char            ConvertIndexToGameSeat  ( unsigned short ulModel, unsigned char ucIndex );

    static bool                     HasTurret               ( unsigned short ulModel );
    static bool                     HasSirens               ( unsigned short ulModel );
    static bool                     HasLandingGears         ( unsigned short ulModel );
    static bool                     HasAdjustableProperty   ( unsigned short ulModel );
    static bool                     HasSmokeTrail           ( unsigned short ulModel );
    static bool                     HasTaxiLight            ( unsigned short ulModel );
    static bool                     HasSearchLight          ( unsigned short ulModel );
    static bool                     HasDamageModel          ( unsigned short ulModel );
    static bool                     HasDamageModel          ( enum eClientVehicleType Type );
    static bool                     HasDoors                ( unsigned short ulModel );

    bool                            Exists                  ( CClientVehicle* pVehicle );

    static bool                     IsVehicleLimitReached   ( void );

    void                            RestreamVehicles        ( unsigned short usModel );

    std::vector < CClientVehicle* > ::const_iterator            IterBegin           ( void )    { return m_List.begin (); };
    std::vector < CClientVehicle* > ::const_iterator            IterEnd             ( void )    { return m_List.end (); };
    std::vector < CClientVehicle* > ::const_iterator            StreamedBegin       ( void )    { return m_StreamedIn.begin (); };
    std::vector < CClientVehicle* > ::const_iterator            StreamedEnd         ( void )    { return m_StreamedIn.end (); };
    
private:
                                    CClientVehicleManager   ( CClientManager* pManager );
                                    ~CClientVehicleManager  ( void );

    inline void                     AddToList               ( CClientVehicle* pVehicle )    { m_List.push_back ( pVehicle ); };
    void                            RemoveFromList          ( CClientVehicle* pVehicle );

    void                            OnCreation              ( CClientVehicle* pVehicle );
    void                            OnDestruction           ( CClientVehicle* pVehicle );

    CClientManager*                 m_pManager;
    bool                            m_bCanRemoveFromList;
    CMappedArray < CClientVehicle* >  m_List;
    std::vector < CClientVehicle* >   m_StreamedIn;
    std::list < CClientVehicle* >   m_Attached;

};

#endif
