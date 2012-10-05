/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*               (Shared logic for modifications)
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        mods/shared_logic/CClientObject.h
*  PURPOSE:     Physical object entity class
*  DEVELOPERS:  Ed Lyons <eai@opencoding.net>
*               Christian Myhre Lundheim <>
*               Cecill Etheredge <ijsf@gmx.net>
*               Jax <>
*               The_GTA <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

class CClientObject;

#ifndef __CCLIENTOBJECT_H
#define __CCLIENTOBJECT_H

#include "CClientStreamElement.h"

#define LUACLASS_OBJECT     60

struct SLastSyncedObjectData
{
    CVector vecPosition;
    CVector vecRotation;
    float   fHealth;
};

class CClientObject : public CClientStreamElement
{
    friend class CClientObjectManager;
    friend class CClientPed;
public:
                                    CClientObject( class CClientManager *pManager, ElementID ID, lua_State *L, bool system, unsigned short usModel );
                                    ~CClientObject();

    void                            Unlink();
    
    inline eClientEntityType        GetType() const                                         { return CCLIENTOBJECT; }

    inline CObject*                 GetGameObject()                                         { return m_pObject; }
    inline CEntity*                 GetGameEntity()                                         { return m_pObject; }
    inline const CEntity*           GetGameEntity() const                                   { return m_pObject; }

    void                            GetPosition( CVector& vecPosition ) const;
    void                            SetPosition( const CVector& vecPosition );
    virtual CSphere                 GetWorldBoundingSphere();

    void                            AttachTo( CClientEntity* pEntity );

    void                            GetRotationDegrees( CVector& vecRotation ) const;
    void                            GetRotationRadians( CVector& vecRotation ) const;
    void                            SetRotationDegrees( const CVector& vecRotation );
    virtual void                    SetRotationRadians( const CVector& vecRotation );

    void                            GetMoveSpeed( CVector& vecMoveSpeed ) const;
    void                            SetMoveSpeed( const CVector& vecMoveSpeed );

    void                            GetOrientation( CVector& vecPosition, CVector& vecRotationRadians );
    virtual void                    SetOrientation( const CVector& vecPosition, const CVector& vecRotationRadians );

    void                            ModelRequestCallback( CModelInfo* pModelInfo );

    float                           GetDistanceFromCentreOfMassToBaseOfModel();

    inline bool                     IsVisible()                                             { return m_bIsVisible; }
    void                            SetVisible( bool bVisible );

    inline unsigned short           GetModel() const                                        { return m_usModel; }
    void                            SetModel( unsigned short usModel );

    void                            Render();

    inline bool                     IsStatic()                                              { return m_bIsStatic; }
    void                            SetStatic( bool bStatic );
    
    inline unsigned char            GetAlpha()                                              { return m_ucAlpha; }   
    void                            SetAlpha( unsigned char ucAlpha );
    inline float                    GetScale()                                              { return m_fScale; }
    void                            SetScale( float fScale );

    inline bool                     IsCollisionEnabled()                                    { return m_bUsesCollision; }
    void                            SetCollisionEnabled( bool bCollisionEnabled );

    float                           GetHealth();
    void                            SetHealth( float fHealth );

    inline bool                     IsBreakable()                                           { return m_pObjectManager->IsBreakableModel( m_usModel ) && m_bBreakable; }
    inline void                     SetBreakable( bool bBreakable )                         { m_bBreakable = bBreakable; }

    void                            ReCreate();
protected:
    void                            StreamIn( bool bInstantly );
    void                            StreamOut();

    void                            Create();
    void                            Destroy();

    void                            NotifyCreate();
    void                            NotifyDestroy();

    void                            StreamedInPulse();

    class CClientObjectManager*         m_pObjectManager;
    class CClientModelRequestManager*   m_pModelRequester;

    unsigned short                      m_usModel;

    CVector                             m_vecPosition;
    CVector                             m_vecRotation;
    bool                                m_bIsVisible;
    bool                                m_bIsStatic;
    bool                                m_bUsesCollision;
    unsigned char                       m_ucAlpha;
    float                               m_fScale;
    float                               m_fHealth;
    bool                                m_bBreakable;

    CVector                             m_vecMoveSpeed;

public:
    CObject*                            m_pObject;
    SLastSyncedObjectData               m_LastSyncedData;
};

#endif
