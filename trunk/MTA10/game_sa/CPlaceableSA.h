/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        game_sa/CPlaceableSA.h
*  PURPOSE:     Transformation management interface
*  DEVELOPERS:  Martin Turski <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#ifndef _PLACEABLE_SA_
#define _PLACEABLE_SA_

class CPlaceableSAInterface // 24 bytes
{
public:
                                    CPlaceableSAInterface       ( void );
    virtual __thiscall              ~CPlaceableSAInterface      ( void );

    void __thiscall                 AllocateMatrix              ( void );
    void __thiscall                 AcquaintMatrix              ( void );
    void __thiscall                 FreeMatrix                  ( void );

    void __thiscall                 GetOffset                   ( CVector& out, const CVector& in ) const;

    // Seperate structure which holds all transformation calculation code.
    // It is linked to CTransformationSAInterface.
    struct Transform
    {
        void                        SetPosition                 ( float x, float y, float z )
        {
            if ( m_matrix )
                m_matrix->vPos = CVector( x, y, z );
            else
                m_translate = CVector( x, y, z );
        }
        void                        SetPosition                 ( const CVector& pos )
        {
            if ( m_matrix )
                m_matrix->vPos = pos;
            else
                m_translate = pos;
        }

        const CVector&              GetPosition                 ( void ) const      { return m_matrix ? m_matrix->vPos : m_translate; }
        void                        GetMatrix                   ( RwMatrix& out ) const
        {
            if ( m_matrix )
                out.assign( *m_matrix );
            else
                GetMatrixFromHeading( out );
        }

        inline float                GetHeading                  ( void ) const
        {
            if ( CTransformSAInterface *trans = m_matrix )
                return trans->ToHeading();
            
            return m_heading;
        }

        inline void                 SetRotation                 ( float x, float y, float z )
        {
            if ( CTransformSAInterface *trans = m_matrix )
                m_matrix->SetRotationRad( x, y, z );
            else
                m_heading = z;
        }

        void __thiscall             GetOffsetByHeading          ( CVector& out, const CVector& in ) const;
        void __thiscall             GetMatrixFromHeading        ( RwMatrix& mat ) const;

        CVector                     m_translate;                // 4
        float                       m_heading;                  // 16
        CTransformSAInterface*      m_matrix;                   // 20
    } Placeable;
};

void Placeable_Init( void );
void Placeable_Shutdown( void );

#endif //_PLACEABLE_SA_