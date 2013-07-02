/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        game_sa/RenderWare/CRpLightSA.h
*  PURPOSE:     RenderWare RpLight export
*  DEVELOPERS:  The_GTA <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#ifndef _RW_LIGHT_SA_
#define _RW_LIGHT_SA_

class CModelSA;

class CRpLightSA : public virtual CRpLight, public CRwObjectSA
{
    friend class CModelSA;
public:
                                    CRpLightSA( RpLight *light );
                                    ~CRpLightSA();

    inline RpLight*                 GetObject()                                 { return (RpLight*)m_object; }
    inline const RpLight*           GetObject() const                           { return (const RpLight*)m_object; }

    eRwType                         GetType() const                             { return RW_LIGHT; }
    bool                            IsFrameExtension() const                    { return true; }

    CRpLightSA*                     Clone( void ) const;

    void                            SetLightIndex( unsigned int idx )           { GetObject()->SetLightIndex( idx ); }
    unsigned int                    GetLightIndex() const                       { return GetObject()->GetLightIndex(); }

    inline RpLightType              GetLightType() const                        { return (RpLightType)GetObject()->m_subtype; }

    void                            SetAttenuation( const CVector& atten )      { GetObject()->m_attenuation = atten; }
    const CVector&                  GetAttenuation() const                      { return GetObject()->m_attenuation; }

    void                            AddToModel( CModel *model );
    CModel*                         GetModel();
    void                            RemoveFromModel();

    void                            SetColor( const RwColorFloat& color )       { GetObject()->SetColor( color ); }
    const RwColorFloat&             GetColor() const                            { return GetObject()->m_color; }

    void                            SetRadius( float radius );
    float                           GetRadius() const                           { return GetObject()->m_radius; }

    void                            SetConeAngle( float radians );
    float                           GetConeAngle() const;

    void                            AddToScene( void );
    bool                            IsAddedToScene( void ) const;
    void                            RemoveFromScene( void );

private:
    CModelSA*   m_model;
};

#endif //_RW_LIGHT_SA_