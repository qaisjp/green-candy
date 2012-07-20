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
    virtual void                        SetPosition( const CVector& vecPosition ) = 0;
    virtual void                        GetPosition( const CVector& pos ) const = 0;
    virtual void                        Teleport( float fX, float fY, float fZ ) = 0;

    virtual void                        ProcessControl() = 0;
    virtual void                        SetupLighting() = 0;
    virtual void                        Render() = 0;

    virtual void                        SetOrientation( float fX, float fY, float fZ )=0;
    virtual void                        FixBoatOrientation() = 0;
    virtual void                        GetMatrix( RwMatrix& matrix ) const = 0;
    virtual void                        SetMatrix( const RwMatrix& matrix ) = 0;

    virtual unsigned short              GetModelIndex() const = 0;
    virtual eEntityType                 GetEntityType() const = 0;

    virtual float                       GetBasingDistance() const = 0;

    virtual void                        SetEntityStatus( eEntityStatus bStatus ) = 0;
    virtual eEntityStatus               GetEntityStatus() const = 0;

    virtual bool                        IsOnScreen() const = 0;

    virtual void                        SetUnderwater( bool bUnderwater ) = 0;
    virtual bool                        GetUnderwater() const = 0;

    virtual bool                        IsVisible() const = 0;
    virtual void                        SetVisible( bool bVisible ) = 0;

    virtual void                        SetDoNotRemoveFromGameWhenDeleted( bool bDoNotRemoveFromGame ) = 0;

    virtual void                        SetUsesCollision( bool enabled ) = 0;
    virtual bool                        IsBackfaceCulled() const = 0;
    virtual void                        SetBackfaceCulled( bool enabled ) = 0;

    virtual bool                        IsStatic() const = 0;
    virtual void                        SetStatic( bool enabled ) = 0;

    virtual void                        SetAlpha( unsigned char alpha ) = 0;

    virtual void                        MatrixConvertFromEulerAngles( float x, float y, float z, int unk ) = 0;
    virtual void                        MatrixConvertToEulerAngles( float& x, float& y, float& z, int unk ) const = 0;
    virtual bool                        IsPlayingAnimation( const char *name ) = 0;

    virtual void*                       GetStoredPointer() const = 0;
    virtual void                        SetStoredPointer( void *pointer ) = 0;

    virtual unsigned char               GetAreaCode() const = 0;
    virtual void                        SetAreaCode( unsigned char areaCode ) = 0;

    virtual bool                        IsStaticWaitingForCollision() const = 0;
    virtual void                        SetStaticWaitingForCollision( bool bStatic ) = 0;

    virtual bool                        IsCollidableWith( CEntity *entity ) const = 0;
    virtual void                        SetCollidableWith( CEntity *entity, bool enable ) = 0;
};

#endif
