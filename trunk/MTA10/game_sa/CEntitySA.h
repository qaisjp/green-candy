/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        game_sa/CEntitySA.h
*  PURPOSE:     Header file for base entity class
*  DEVELOPERS:  Ed Lyons <eai@opencoding.net>
*               Kent Simon <>
*               Cecill Etheredge <ijsf@gmx.net>
*               Jax <>
*               Christian Myhre Lundheim <>
*               The_GTA <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#ifndef __CGAMESA_ENTITY
#define __CGAMESA_ENTITY

#include "Common.h"
#include "COffsets.h"
#include <game/CEntity.h>
#include <CVector2D.h>
#include <CVector.h>

#define FUNC_GetDistanceFromCentreOfMassToBaseOfModel       0x536BE0

#define FUNC_SetRwObjectAlpha                               0x5332C0
#define FUNC_SetOrientation                                 0x439A80

#define FUNC_CMatrix__ConvertToEulerAngles                    0x59A840
#define FUNC_CMatrix__ConvertFromEulerAngles                0x59AA40

#define FUNC_IsOnScreen                                     0x534540
#define FUNC_IsVisible                                      0x536BC0


// not in CEntity really
#define FUNC_RpAnimBlendClumpGetAssociation                 0x4D6870

// replace with enum from R*
#define STATUS_ABANDONED                    4


/** 
 * \todo Move CReferences (and others below?) into it's own file
 */
class CReferences
{
    CEntity     * pEntity;
};

class CPlaceableSAInterface // 24 bytes
{
public:
                                    CPlaceableSAInterface();
    virtual                         ~CPlaceableSAInterface();

    void                            AllocateMatrix();
    void                            FreeMatrix();

    // Transformed parameters
    CVector                         m_position;
    float                           m_heading;
    CTransformSAInterface*          m_matrix;
};

struct CRect {
    float fX1, fY1, fX2, fY2;
};

// Entity flags
#define ENTITY_COLLISION        0x00000001
#define ENTITY_COLL_PROCESSED   0x00000002
#define ENTITY_STATIC           0x00000004
#define ENTITY_CONTACTED        0x00000008
#define ENTITY_STUCK            0x00000010
#define ENTITY_NOCOLLHIT        0x00000020
#define ENTITY_PROCESSLATER     0x00000040
#define ENTITY_VISIBLE          0x00000080
    
#define ENTITY_BIG              0x00000100
#define ENTITY_USEDAMAGE        0x00000200
#define ENTITY_DISABLESTREAMING 0x00000400
#define ENTITY_INVALID          0x00000800
#define ENTITY_COLL_WORLD       0x00001000
#define ENTITY_RENDERING        0x00002000
#define ENTITY_RENDER_LAST      0x00004000
#define ENTITY_FADE             0x00008000
    
#define ENTITY_NOSHADOWCAST     0x00010000
#define ENTITY_OFFSCREEN        0x00020000
#define ENTITY_WAITFORCOLL      0x00040000
#define ENTITY_NOSTREAM         0x00080000
#define ENTITY_UNDERWATER       0x00100000
#define ENTITY_PRERENDERED      0x00200000
#define ENTITY_DYNAMIC          0x00400000  // for buildings
#define ENTITY_CACHED_SKELETON  0x00800000
    
#define ENTITY_ROADSIGN         0x01000000
#define ENTITY_LOWLOD           0x02000000
#define ENTITY_PROC_OBJECT      0x04000000
#define ENTITY_BACKFACECULL     0x08000000
#define ENTITY_LIGHTING         0x10000000
#define ENTITY_UNIMPORTANT      0x20000000
#define ENTITY_TUNNEL           0x40000000
#define ENTITY_TUNNELTRANSITION 0x80000000


class CEntitySAInterface : public CPlaceableSAInterface
{
public:
                                    CEntitySAInterface();
                                    ~CEntitySAInterface();

    virtual void __thiscall         AddRect ( CRect rect );
    virtual bool __thiscall         AddToWorld ();
    virtual void __thiscall         RemoveFromWorld ();
    virtual void __thiscall         SetStatic ( bool enabled );
    virtual void __thiscall         SetModelIndex ( unsigned short id );
    virtual void __thiscall         SetModelIndexNoCreate ( unsigned short id );
    virtual RwObject* __thiscall    CreateRwObject ();
    virtual void __thiscall         DeleteRwObject ();
    virtual void __thiscall         GetBoundingBox ( CBoundingBox box );
    virtual void __thiscall         ProcessControl ();
    virtual void __thiscall         ProcessCollision ();
    virtual void __thiscall         ProcessShift ();
    virtual bool __thiscall         TestCollision ();
    virtual void __thiscall         Teleport ( float x, float y, float z, int unk );
    virtual void __thiscall         PreFrame ();
    virtual bool __thiscall         Frame ();
    virtual void __thiscall         PreRender ();
    virtual void __thiscall         Render ();
    virtual void __thiscall         SetupLighting ();
    virtual void __thiscall         RemoveLighting ();
    virtual void __thiscall         Invalidate ();

    RwObject*               m_rwObject;         // 24

    unsigned int            m_entityFlags;      // 28

    unsigned short          m_randomSeed;       // 32
    unsigned short          m_model;            // 34
    CReferences*            m_references;       // 36
    
    DWORD*                  m_lastRenderedLink; // 40, CLink<CEntity*>* m_pLastRenderedLink;
    
    unsigned short          m_scanCode;         // 44
    unsigned char           m_iplIndex;         // 46, used to define which IPL file object is in
    unsigned char           m_areaCode;         // 47, used to define what objects are visible at this point
    
    CEntitySAInterface*     m_lod;              // 48
    unsigned char           m_numLOD;           // 52
    unsigned char           m_numRenderedLOD;   // 53

    //********* BEGIN CEntityInfo **********//
    BYTE                    m_type : 3;        // 54 (2 == Vehicle)
    BYTE                    m_status : 5;  
    //********* END CEntityInfo **********//

    unsigned char           m_pad;              // 55

    /* IMPORTANT: Do not mess with interfaces. */
};

class CEntitySA : public virtual CEntity
{
    friend class COffsets;
public:
                                CEntitySA();

    CEntitySAInterface*         m_pInterface;
    unsigned int                m_internalID;

    inline CEntitySAInterface*  GetInterface() { return m_pInterface; };

    void                        SetPosition( float fX, float fY, float fZ );
    void                        Teleport( float fX, float fY, float fZ );
    void                        ProcessControl();
    void                        SetupLighting();
    void                        Render();
    void                        SetOrientation( float fX, float fY, float fZ );
    void                        FixBoatOrientation();        // eAi you might want to rename this
    void                        SetPosition( CVector *pos );

    void                        SetUnderwater( bool bUnderwater );
    bool                        GetUnderwater();

    void                        GetPosition( CVector *pos );
    void                        GetMatrix( CMatrix *matrix ) const;
    void                        SetMatrix( CMatrix *matrix );
    unsigned short              GetModelIndex();
    eEntityType                 GetEntityType();
    bool                        IsOnScreen();

    bool                        IsVisible();
    void                        SetVisible( bool bVisible );

    unsigned char               GetAreaCode();
    void                        SetAreaCode( unsigned char areaCode );

    float                       GetBasingDistance();

    void                        SetEntityStatus( eEntityStatus bStatus );
    eEntityStatus               GetEntityStatus();

    RwFrame*                    GetFrameFromId( int id );
    RwMatrix*                   GetLTMFromId( int id );

    RwObject*                   GetRwObject();

    void                        SetDoNotRemoveFromGameWhenDeleted( bool b )    { m_doNotRemoveFromGame = b; };

    bool                        IsStatic()                             { return IS_FLAG( m_pInterface->m_entityFlags, ENTITY_STATIC ); }
    void                        SetStatic( bool enabled )              { BOOL_FLAG( m_pInterface->m_entityFlags, ENTITY_STATIC, enabled ); };
    void                        SetUsesCollision( bool enabled )       { BOOL_FLAG( m_pInterface->m_entityFlags, ENTITY_COLLISION, enabled ); };

    bool                        IsBackfaceCulled()                     { return IS_FLAG( m_pInterface->m_entityFlags, ENTITY_BACKFACECULL ); };
    void                        SetBackfaceCulled( bool enabled )      { BOOL_FLAG( m_pInterface->m_entityFlags, ENTITY_BACKFACECULL, enabled ); };

    void                        SetAlpha( unsigned char alpha );

    void                        MatrixConvertFromEulerAngles( float fX, float fY, float fZ, int iUnknown );
    void                        MatrixConvertToEulerAngles( float *fX, float *fY, float *fZ, int iUnknown );

    bool                        IsPlayingAnimation( char *szAnimName );

    void*                       GetStoredPointer()                     { return m_pStoredPointer; };
    void                        SetStoredPointer( void *pointer )      { m_pStoredPointer = pointer; };

    bool                        IsStaticWaitingForCollision()          { return IS_FLAG( m_pInterface->m_entityFlags, ENTITY_WAITFORCOLL ); }
    void                        SetStaticWaitingForCollision( bool enabled ) { BOOL_FLAG( m_pInterface->m_entityFlags, ENTITY_WAITFORCOLL, enabled ); }

    void                        GetImmunities( bool& bNoClip, bool& bFrozen, bool& bBulletProof, bool& bFlameProof, bool& bUnk, bool& bUnk2, bool& bCollisionProof, bool& bExplosionProof );

    inline unsigned long        GetArrayID()                           { return m_ulArrayID; }
    inline void                 SetArrayID( unsigned long ulID )       { m_ulArrayID = ulID; }

private:
    void                        SetInterface( CEntitySAInterface * intInterface ) { m_pInterface = intInterface; };

    static unsigned long        FUNC_CClumpModelInfo__GetFrameFromId;

    unsigned long               m_ulArrayID;

    void*                       m_pStoredPointer;

    bool                        m_beingDeleted; // to prevent it trying to delete twice
    bool                        m_doNotRemoveFromGame; // when deleted, if this is true, it won't be removed from the game
};

#endif
