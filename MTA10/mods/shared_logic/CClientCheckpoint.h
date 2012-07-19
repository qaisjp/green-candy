/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.0
*               (Shared logic for modifications)
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        mods/shared_logic/CClientCheckpoint.h
*  PURPOSE:     Checkpoint marker entity class
*  DEVELOPERS:  Ed Lyons <eai@opencoding.net>
*               Christian Myhre Lundheim <>
*               Jax <>
*
*****************************************************************************/

#ifndef __CCLIENTCHECKPOINT_H
#define __CCLIENTCHECKPOINT_H

#include "CClientMarkerCommon.h"
#include <list>

class CClientMarker;

class CClientCheckpoint : public CClientMarkerCommon
{
    friend class CClientMarkerManager;

public:
    enum
    {
        TYPE_NORMAL,
        TYPE_RING,
        TYPE_INVALID,
    };

    enum
    {
        ICON_NONE,
        ICON_ARROW,
        ICON_FINISH,
    };

                                    CClientCheckpoint( CClientMarker * pThis );
                                    ~CClientCheckpoint();

    inline unsigned int             GetMarkerType() const                                   { return CClientMarkerCommon::CLASS_CHECKPOINT; };

    unsigned long                   GetCheckpointType() const;
    void                            SetCheckpointType( unsigned long ulType );

    bool                            IsHit( const CVector& vecPosition ) const;
    
    inline void                     GetPosition( CVector& vecPosition ) const               { vecPosition = m_Matrix.pos; };
    void                            SetPosition( const CVector& vecPosition );
    void                            SetDirection( const CVector& vecDirection );
    void                            SetNextPosition( const CVector& vecPosition );

    void                            GetMatrix( RwMatrix& mat );
    void                            SetMatrix( const RwMatrix& mat );

    inline bool                     IsVisible() const                                       { return m_bVisible; }
    void                            SetVisible( bool bVisible );

    inline unsigned int             GetIcon() const                                         { return m_uiIcon; }
    void                            SetIcon( unsigned int uiIcon );
    
    inline SColor                   GetColor() const                                        { return m_Color; }
    void                            SetColor( const SColor color );

    inline float                    GetSize() const                                         { return m_fSize; };
    void                            SetSize( float fSize );

    inline bool                     HasTarget()                                             { return m_bHasTarget; }
    inline void                     SetHasTarget( bool bHasTarget )                         { m_bHasTarget = bHasTarget; }
    inline void                     GetTarget( CVector& vecTarget )                         { vecTarget = m_vecTarget; }
    inline void                     SetTarget( const CVector& vecTarget )                   { m_vecTarget = vecTarget; }

    static unsigned char            StringToIcon( const char* szString );
    static bool                     IconToString( unsigned char ucIcon, char* szString );
    void                            ReCreateWithSameIdentifier();

protected:
    bool                            IsStreamedIn()                                          { return m_bStreamedIn; };
    void                            StreamIn();
    void                            StreamOut();

private:
    void                            Create( unsigned long ulIdentifier = 0 );
    void                            Destroy();
    void                            ReCreate();

    CClientMarker*                  m_pThis;
    bool                            m_bStreamedIn;
    RwMatrix                        m_Matrix;
    CVector                         m_vecDirection;
    CVector                         m_vecTargetPosition;
    bool                            m_bVisible;
    unsigned int                    m_uiIcon;
    DWORD                           m_dwType;
    float                           m_fSize;
    SColor                          m_Color;
    CCheckpoint*                    m_pCheckpoint;

    DWORD                           m_dwIdentifier;    
    bool                            m_bHasTarget;
    CVector                         m_vecTarget;
};

#endif
