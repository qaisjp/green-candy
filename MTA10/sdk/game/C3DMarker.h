/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        sdk/game/C3DMarker.h
*  PURPOSE:     3D marker entity interface
*  DEVELOPERS:  The_GTA <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#ifndef __CGAME_3DMARKER
#define __CGAME_3DMARKER

class CVector;

/**
 * \todo fix SetColor/GetColor, the format is actually BGRA (strange)
 */
class C3DMarker
{
public:
    virtual                 ~C3DMarker()    {};

    virtual void            GetMatrix( RwMatrix& mat ) = 0;
    virtual void            SetMatrix( const RwMatrix& mat ) = 0;

    virtual void            SetPosition( const CVector& pos ) = 0;
    virtual CVector&        GetPosition() = 0;

    virtual unsigned int    GetType() = 0; // need enum?
    virtual bool            IsActive() = 0;
    virtual unsigned int    GetIdentifier() = 0;

    virtual SColor          GetColor() = 0;
    virtual void            SetColor( const SColor color ) = 0;

    virtual void            SetRotateRate( short rate ) = 0;

    virtual void            SetPulsePeriod( unsigned short period ) = 0;
    virtual void            SetPulseFraction( float fraction ) = 0;
    virtual float           GetPulseFraction() = 0;

    virtual float           GetSize() = 0;
    virtual void            SetSize( float size ) = 0;

    virtual float           GetBrightness() = 0;
    virtual void            SetBrightness( float brightness ) = 0;

    virtual void            SetCameraRange( float range ) = 0;

    virtual void            Disable() = 0;
    virtual void            SetActive() = 0;
};

#endif
