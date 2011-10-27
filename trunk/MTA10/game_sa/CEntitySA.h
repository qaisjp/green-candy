/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.0
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        game_sa/CEntitySA.h
*  PURPOSE:     Header file for base entity class
*  DEVELOPERS:  Ed Lyons <eai@opencoding.net>
*               Kent Simon <>
*               Cecill Etheredge <ijsf@gmx.net>
*               Jax <>
*               Christian Myhre Lundheim <>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#ifndef __CGAMESA_ENTITY
#define __CGAMESA_ENTITY

#include "Common.h"
#include "COffsets.h"
#include <game/CEntity.h>
#include <CMatrix.h>
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

class CSimpleTransformSAInterface   // 16 bytes
{
public:
    CVector                         m_translate;
    float                           m_heading;
};

class CPlaceableSAInterface // 20 bytes
{
public:
    CSimpleTransformSAInterface     m_transform;
    RwMatrix*                       m_matrix;
};

struct CRect {
    float fX1, fY1, fX2, fY2;
};

class CSceneSAInterface
{
public:
    // Why not make it into a virtual table?
    virtual                         ~CSceneSAInterface() = 0;

    virtual void __thiscall         AddRect( CRect rect ) = 0;
    virtual bool __thiscall         AddToWorld() = 0;
    virtual void __thiscall         RemoveFromWorld() = 0;
    virtual void __thiscall         SetStatic( bool enabled ) = 0;
    virtual bool __thiscall         SetModelIndex( unsigned short id ) = 0;
    virtual bool __thiscall         SetModelIndexNoCreate( unsigned short id ) = 0;
    virtual RwObject* __thiscall    CreateRwObject() = 0;
    virtual void __thiscall         DeleteRwObject() = 0;
    virtual void __thiscall         GetBoundingBox( CBoundingBox box ) = 0;
    virtual void __thiscall         ProcessControl() = 0;
    virtual void __thiscall         ProcessCollision() = 0;
    virtual void __thiscall         ProcessShift() = 0;
    virtual bool __thiscall         TestCollision() = 0;
    virtual void __thiscall         Teleport( float x, float y, float z, int unk ) = 0;
    virtual void __thiscall         PreFrame() = 0;
    virtual void __thiscall         Frame() = 0;
    virtual void __thiscall         PreRender() = 0;
    virtual void __thiscall         Render() = 0;
    virtual void __thiscall         SetupLighting() = 0;
    virtual void __thiscall         RemoveLighting() = 0;
    virtual void __thiscall         SafeDestroy() = 0;

    CPlaceableSAInterface   m_placeable;        // 4

    RwObject*               m_rwObject;         // 24

    /********** BEGIN CFLAGS +28 **************/
    unsigned long           m_usesCollision : 1;           // does entity use collision
    unsigned long           m_collisionProcessed : 1;  // has object been processed by a ProcessEntityCollision function
    unsigned long           m_isStatic : 1;                // is entity static
    unsigned long           m_hasContacted : 1;            // has entity processed some contact forces
    unsigned long           m_isStuck : 1;             // is entity stuck
    unsigned long           m_isInSafePosition : 1;        // is entity in a collision free safe position
    unsigned long           m_wasPostponed : 1;            // was entity control processing postponed
    unsigned long           m_isVisible : 1;               //is the entity visible
    
    unsigned long           m_isBIGBuilding : 1;           // Set if this entity is a big building
    unsigned long           m_renderDamaged : 1;           // use damaged LOD models for objects with applicable damage
    unsigned long           m_streamingDontDelete : 1; // Dont let the streaming remove this 
    unsigned long           m_removeFromWorld : 1;     // remove this entity next time it should be processed
    unsigned long           m_hasHitWall : 1;              // has collided with a building (changes subsequent collisions)
    unsigned long           m_beingRendered : 1;     // don't delete me because I'm being rendered
    unsigned long           m_drawLast :1;             // draw object last
    unsigned long           m_distanceFade :1;         // Fade entity because it is far away
    
    unsigned long           m_dontCastShadowsOn : 1;       // Dont cast shadows on this object
    unsigned long           m_offscreen : 1;               // offscreen flag. This can only be trusted when it is set to true
    unsigned long           m_isStaticWaitingForCollision : 1; // this is used by script created entities - they are static until the collision is loaded below them
    unsigned long           m_dontStream : 1;              // tell the streaming not to stream me
    unsigned long           m_underwater : 1;              // this object is underwater change drawing order
    unsigned long           m_hasPreRenderEffects : 1; // Object has a prerender effects attached to it
    unsigned long           m_isTempBuilding : 1;          // whether or not the building is temporary (i.e. can be created and deleted more than once)
    unsigned long           m_dontUpdateSkeleton : 1; // Don't update the aniamtion hierarchy this frame
    
    unsigned long           m_hasRoadsignText : 1;     // entity is roadsign and has some 2deffect text stuff to be rendered
    unsigned long           m_displayedSuperLowLOD : 1;
    unsigned long           m_isProcObject : 1;            // set object has been generate by procedural object generator
    unsigned long           m_backfaceCulled : 1;          // has backface culling on
    unsigned long           m_lightObject : 1;         // light object with directional lights
    unsigned long           m_unimportantStream : 1;       // set that this object is unimportant, if streaming is having problems
    unsigned long           m_tunnel : 1;          // Is this model part of a tunnel
    unsigned long           m_tunnelTransition : 1;        // This model should be rendered from within and outside of the tunnel
    /********** END CFLAGS **************/

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

    /* IMPORTANT: KEEP "pad" in CVehicle UP-TO-DATE if you add something here (or eventually pad someplace else) */
};

class CEntitySAInterface : public CSceneSAInterface
{
public:
    BYTE                    m_pad2[7];          // 56
    unsigned char           m_numImmunities;    // 63
    DWORD                   m_pad3;             // 64
};

class CEntitySA : public virtual CEntity
{
    friend class COffsets;
public:
                                CEntitySA           ( void );

    CEntitySAInterface*         m_pInterface;

    DWORD                       internalID;
//  VOID                        SetModelAlpha ( int iAlpha );

    inline CEntitySAInterface * GetInterface() { return m_pInterface; };
    VOID                        SetInterface( CEntitySAInterface * intInterface ) { m_pInterface = intInterface; };

    VOID                        SetPosition ( float fX, float fY, float fZ );
    VOID                        Teleport ( float fX, float fY, float fZ );
    VOID                        ProcessControl ( void );
    VOID                        SetupLighting ( );
    VOID                        Render ( );
    VOID                        SetOrientation ( float fX, float fY, float fZ );
    VOID                        FixBoatOrientation ( void );        // eAi you might want to rename this
    VOID                        SetPosition ( CVector * vecPosition );

    void                        SetUnderwater ( bool bUnderwater );
    bool                        GetUnderwater ( void );

    CVector                     * GetPosition (  );
    CMatrix                     * GetMatrix ( CMatrix * matrix ) const;
    VOID                        SetMatrix ( CMatrix * matrix );
    WORD                        GetModelIndex ();
    eEntityType                 GetEntityType ();
    bool                        IsOnScreen ();

    bool                        IsVisible ( void );
    void                        SetVisible ( bool bVisible );

    BYTE                        GetAreaCode ( void );
    void                        SetAreaCode ( BYTE areaCode );

    FLOAT                       GetDistanceFromCentreOfMassToBaseOfModel();
    /**
     * \todo Find enum for SetEntityStatus
     */
    VOID                        SetEntityStatus( eEntityStatus bStatus );
    eEntityStatus               GetEntityStatus( );

    RwFrame *                   GetFrameFromId ( int id );
    RwMatrix *                  GetLTMFromId ( int id );

    RpClump *                   GetRpClump ();


    BOOL                        BeingDeleted; // to prevent it trying to delete twice
    BOOL                        DoNotRemoveFromGame; // when deleted, if this is true, it won't be removed from the game

    VOID                        SetDoNotRemoveFromGameWhenDeleted ( bool bDoNotRemoveFromGame ) { DoNotRemoveFromGame = bDoNotRemoveFromGame; };
    BOOL                        IsStatic(VOID)                          { return m_pInterface->bIsStatic; }
    VOID                        SetStatic(BOOL bStatic)                 { m_pInterface->bIsStatic       = bStatic; };
    VOID                        SetUsesCollision(BOOL bUsesCollision)   { m_pInterface->bUsesCollision  = bUsesCollision;};
    BOOL                        IsBackfaceCulled(VOID)                  { return m_pInterface->bBackfaceCulled; };
    VOID                        SetBackfaceCulled(BOOL bBackfaceCulled) { m_pInterface->bBackfaceCulled = bBackfaceCulled; };
    VOID                        SetAlpha(DWORD dwAlpha);

    VOID                        MatrixConvertFromEulerAngles ( float fX, float fY, float fZ, int iUnknown );
    VOID                        MatrixConvertToEulerAngles ( float * fX, float * fY, float * fZ, int iUnknown );

    bool                        IsPlayingAnimation ( char * szAnimName );

    void*                       GetStoredPointer    ( void )                { return m_pStoredPointer; };
    void                        SetStoredPointer    ( void* pPointer )      { m_pStoredPointer = pPointer; };

    bool                        IsStaticWaitingForCollision  ( void )        { return m_pInterface->bIsStaticWaitingForCollision; }
    void                        SetStaticWaitingForCollision  ( bool bStatic ) { m_pInterface->bIsStaticWaitingForCollision  = bStatic; }

    void                        GetImmunities   ( bool & bNoClip, bool & bFrozen, bool & bBulletProof, bool & bFlameProof, bool & bUnk, bool & bUnk2, bool & bCollisionProof, bool & bExplosionProof );

    inline unsigned long        GetArrayID      ( void ) { return m_ulArrayID; }
    inline void                 SetArrayID      ( unsigned long ulID ) { m_ulArrayID = ulID; }

private:
    static unsigned long        FUNC_CClumpModelInfo__GetFrameFromId;

    unsigned long               m_ulArrayID;

/*  VOID                        InitFlags()
    {
        //this->GetInterface()->bIsStaticWaitingForCollision = true;
        this->GetInterface()->nStatus = 4;
        
        DWORD dwThis = (DWORD)this->GetInterface();

        DWORD dwFunc = 0x41D000;
        _asm
        {
            push    dwThis
            call    dwFunc
            pop     eax
        }

    };*/

    void*                       m_pStoredPointer;
};

#endif
