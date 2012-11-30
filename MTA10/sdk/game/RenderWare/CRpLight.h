/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        sdk/game/RenderWare/CRpLight.h
*  PURPOSE:     RenderWare RpLight modification export
*  DEVELOPERS:  The_GTA <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#ifndef _RW_LIGHT_
#define _RW_LIGHT_

class CRpLight abstract : public virtual CRwObject
{
public:
    virtual                         ~CRpLight() {}

    virtual void                    SetLightIndex( unsigned int idx ) = 0;
    virtual unsigned int            GetLightIndex() const = 0;

    virtual void                    SetAttenuation( const CVector& atten ) = 0;
    virtual const CVector&          GetAttenuation() const = 0;

    virtual void                    AddToModel( CModel *model ) = 0;
    virtual CModel*                 GetModel() = 0;
    virtual void                    RemoveFromModel() = 0;

    virtual void                    SetColor( const RwColorFloat& color ) = 0;
    virtual const RwColorFloat&     GetColor() const = 0;

    virtual void                    SetRadius( float radius ) = 0;
    virtual float                   GetRadius() const = 0;
};

#endif //_RW_LIGHT_