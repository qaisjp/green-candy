/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.0
*               (Shared logic for modifications)
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        mods/shared_logic/CClientProjectile.h
*  PURPOSE:     Projectile entity class header
*  DEVELOPERS:  Jax <>
*               Ed Lyons <eai@opencoding.net>
*
*****************************************************************************/

class CClientProjectile;

#ifndef __CCLIENTPROJECTILE_H
#define __CCLIENTPROJECTILE_H

#include "CClientEntity.h"
#include "CClientCommon.h"

class CProjectile;
class CProjectileInfo;
class CClientEntity;
class CVector;
enum eWeaponType;
class CClientProjectileManager;
class CClientPed;
class CClientVehicle;

class CProjectileInitiateData
{
public:
    inline          CProjectileInitiateData ( void )
    {
        pvecPosition = NULL; pvecRotation = NULL;
        pvecVelocity = NULL; usModel = 0;
    }
    inline          ~CProjectileInitiateData ( void )
    {
        if ( pvecPosition ) delete pvecPosition;
        if ( pvecRotation ) delete pvecRotation;
        if ( pvecVelocity ) delete pvecVelocity;
    }
    CVector * pvecPosition;
    CVector * pvecRotation;    
    CVector * pvecVelocity;    
    unsigned short usModel;
};

class CClientProjectile : public CClientEntity
{
    DECLARE_CLASS( CClientProjectile, CClientEntity )
    friend class CClientProjectileManager;
    friend class CClientPed;
    friend class CClientVehicle;
public:
                                        CClientProjectile( class CClientManager* pManager,
                                                           CProjectile* pProjectile,
                                                           CProjectileInfo* pProjectileInfo,
                                                           CClientEntity * pCreator,
                                                           CClientEntity * pTarget,
                                                           eWeaponType weaponType,
                                                           CVector * pvecOrigin,
                                                           CVector * pvecTarget,
                                                           float fForce,
                                                           bool bLocal );
                                        ~CClientProjectile();

    eClientEntityType                   GetType() const                                 { return CCLIENTPROJECTILE; }
    inline CEntity*                     GetGameEntity()                                 { return m_pProjectile; }
    inline const CEntity*               GetGameEntity() const                           { return m_pProjectile; }
    void                                Unlink();


    void                                DoPulse();
    void                                Initiate( CVector * pvecPosition, CVector * pvecRotation, CVector * pvecVelocity, unsigned short usModel );
    void                                Destroy();

    bool                                IsActive();
    bool                                GetMatrix( RwMatrix& matrix );
    bool                                SetMatrix( const RwMatrix& matrix );
    void                                GetPosition( CVector& vecPosition ) const;
    void                                SetPosition( const CVector& vecPosition );
    void                                GetRotation( CVector& vecRotation );
    void                                GetRotationDegrees( CVector& vecRotation );
    void                                SetRotation( const CVector& vecRotation );
    void                                SetRotationDegrees( const CVector& vecRotation );
    void                                GetVelocity( CVector& vecVelocity );
    void                                SetVelocity( CVector& vecVelocity );
    void                                SetModel( unsigned short usModel );

    inline CClientEntity *              GetCreator()                                    { return m_pCreator; }
    inline CClientEntity *              GetTargetEntity()                               { return m_pTarget; }
    inline eWeaponType                  GetWeaponType()                                 { return m_weaponType; }
    inline CVector *                    GetOrigin()                                     { return m_pvecOrigin; }
    inline CVector *                    GetTarget()                                     { return m_pvecTarget; }
    inline float                        GetForce()                                      { return m_fForce; }
    inline bool                         IsLocal()                                       { return m_bLocal; }
    
protected:
    CClientProjectileManager*           m_pProjectileManager;
    bool                                m_bLinked;

    CProjectile *                       m_pProjectile;
    CProjectileInfo *                   m_pProjectileInfo;

    CClientEntity *                     m_pCreator;
    CClientEntity *                     m_pTarget;
    eWeaponType                         m_weaponType;
    CVector *                           m_pvecOrigin;
    CVector *                           m_pvecTarget;
    float                               m_fForce;
    bool                                m_bLocal;
    long long                           m_llCreationTime;

    bool                                m_bInitiate;
    CProjectileInitiateData *           m_pInitiateData;
};

#endif