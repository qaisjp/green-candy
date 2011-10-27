/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.0
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        sdk/game/CEntity.h
*  PURPOSE:     Base entity interface
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#ifndef __CGAME_ENTITY
#define __CGAME_ENTITY

#include "Common.h"
#include <CMatrix.h>
#include <CVector.h>

#include <windows.h>

enum eEntityType
{
    ENTITY_TYPE_NOTHING,
    ENTITY_TYPE_BUILDING,
    ENTITY_TYPE_VEHICLE,
    ENTITY_TYPE_PED,
    ENTITY_TYPE_OBJECT,
    ENTITY_TYPE_DUMMY,
    ENTITY_TYPE_NOTINPOOLS
};

enum eEntityStatus
{
    STATUS_PLAYER,
    STATUS_PLAYER_PLAYBACKFROMBUFFER,
    STATUS_SIMPLE,
    STATUS_PHYSICS,
    STATUS_ABANDONED,
    STATUS_WRECKED,
    STATUS_TRAIN_MOVING,
    STATUS_TRAIN_NOT_MOVING,
    STATUS_HELI,
    STATUS_PLANE,
    STATUS_PLAYER_REMOTE,
    STATUS_PLAYER_DISABLED,
    STATUS_TRAILER,
    STATUS_SIMPLE_TRAILER
};

class CEntity
{
public:
    virtual                             ~CEntity () {};

    virtual void                        SetPosition( float fX, float fY, float fZ ) = 0;
    virtual void                        SetPosition ( CVector * vecPosition ) = 0;
    virtual void                        Teleport ( float fX, float fY, float fZ ) = 0;
    virtual void                        ProcessControl () = 0;
    virtual void                        SetupLighting () = 0;
    virtual void                        Render () = 0;
    virtual void                        SetOrientation ( float fX, float fY, float fZ )=0;
    virtual void                        FixBoatOrientation () = 0;
    virtual void                        GetPosition ( CVector pos ) = 0;
    virtual void                        GetMatrix ( CMatrix matrix ) const = 0;
    virtual void                        SetMatrix ( CMatrix matrix ) = 0;
    virtual unsigned short              GetModelIndex () = 0;
    virtual eEntityType                 GetEntityType () = 0;
    virtual float                       GetBasingDistance() = 0;
    virtual void                        SetEntityStatus( eEntityStatus bStatus ) = 0;
    virtual eEntityStatus               GetEntityStatus( ) = 0;
    virtual bool                        IsOnScreen () = 0;
    virtual void                        SetUnderwater ( bool bUnderwater ) = 0;
    virtual bool                        GetUnderwater () = 0;

    virtual bool                        IsVisible () = 0;
    virtual void                        SetVisible ( bool bVisible ) = 0;

    virtual void                        SetDoNotRemoveFromGameWhenDeleted ( bool bDoNotRemoveFromGame ) = 0;
    virtual void                        SetUsesCollision( bool enabled ) = 0;
    virtual bool                        IsBackfaceCulled() = 0;
    virtual void                        SetBackfaceCulled( bool enabled ) = 0;
    virtual bool                        IsStatic() = 0;
    virtual void                        SetStatic( bool enabled ) = 0;
    virtual void                        SetAlpha( unsigned char alpha ) = 0;

    virtual void                        MatrixConvertFromEulerAngles ( float fX, float fY, float fZ, int iUnknown ) = 0;
    virtual void                        MatrixConvertToEulerAngles ( float *fX, float *fY, float *fZ, int iUnknown ) = 0;
    virtual bool                        IsPlayingAnimation ( char * szAnimName ) = 0;

    virtual void*                       GetStoredPointer () = 0;
    virtual void                        SetStoredPointer ( void *pointer ) = 0;

    virtual unsigned char               GetAreaCode () = 0;
    virtual void                        SetAreaCode ( unsigned char areaCode ) = 0;

    virtual bool                        IsStaticWaitingForCollision () = 0;
    virtual void                        SetStaticWaitingForCollision ( bool bStatic ) = 0;

    virtual void                        GetImmunities ( bool& bNoClip, bool& bFrozen, bool& bBulletProof, bool& bFlameProof, bool& bUnk, bool& bUnk2, bool& bCollisionProof, bool& bExplosionProof ) = 0;
};

#endif
