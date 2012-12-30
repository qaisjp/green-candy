/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        game_sa/C3DMarkerSA.h
*  PURPOSE:     Header file for 3D Marker entity class
*  DEVELOPERS:  Ed Lyons <eai@opencoding.net>
*               Christian Myhre Lundheim <>
*               Cecill Etheredge <ijsf@gmx.net>
*               The_GTA <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#ifndef __CGAMESA_3DMARKER
#define __CGAMESA_3DMARKER

#include <game/C3DMarker.h>
#include "Common.h"

#define FUNC_DeleteMarkerObject         0x722390

class C3DMarkerSAInterface
{
public:
    RwMatrix        m_mat;  // local space to world space transform // 0
    DWORD           dwPad,dwPad2;   // not sure why we need these, it appears to be this way though (eAi)  // 64/68
    RpClump         *m_pRwObject; // 72
    DWORD           *m_pMaterial; // 76
    
    WORD            m_nType; // 80
    bool            m_bIsUsed;  // has this marker been allocated this frame?    // 82
    DWORD           m_nIdentifier; // 84

    DWORD           rwColour; // 88 
    WORD            m_nPulsePeriod; // 92
    short           m_nRotateRate;  // deg per frame (in either direction) // 94
    DWORD           m_nStartTime; // 96
    FLOAT           m_fPulseFraction;  // 100
    FLOAT           m_fStdSize; // 104
    FLOAT           m_fSize; // 108
    FLOAT           m_fBrightness; // 112
    FLOAT           m_fCameraRange; // 116

    CVector     m_normal;           // Normal of the object point at             // 120
    // the following variables remember the last time we read the heigh of the
    // map. Using this we don't have to do this every frame and we can still have moving markers.
    WORD            m_LastMapReadX, m_LastMapReadY; // 132 / 134
    FLOAT           m_LastMapReadResultZ; // 136
    FLOAT           m_roofHeight; // 140
    CVector         m_lastPosition; // 144
    DWORD           m_OnScreenTestTime;     // time last screen check was done // 156
};

class C3DMarkerSA : public C3DMarker
{
private:
    C3DMarkerSAInterface*           m_interface;

public:
    C3DMarkerSA( C3DMarkerSAInterface *marker )             { m_interface = marker; }

    C3DMarkerSAInterface*   GetInterface()                  { return m_interface; }
    const C3DMarkerSAInterface* GetInterface() const        { return m_interface; }

    void            GetMatrix( RwMatrix& matrix ) const;
    void            SetMatrix( const RwMatrix& matrix );

    void            SetPosition( const CVector& pos );
    const CVector&  GetPosition() const;

    unsigned int    GetType() const; // need enum?
    void            SetType( unsigned int type ); // doesn't work propperly (not virtualed)
    bool            IsActive() const;
    unsigned int    GetIdentifier() const;

    SColor          GetColor() const;
    void            SetColor( const SColor color ); // actually BGRA

    void            SetRotateRate( short rate );

    void            SetPulsePeriod( unsigned short period);
    void            SetPulseFraction( float fraction ); // doesn't work propperly (not virtualed)
    float           GetPulseFraction() const;

    float           GetSize() const;
    void            SetSize( float size );

    float           GetBrightness() const;
    void            SetBrightness( float brightness );

    void            SetCameraRange( float range );

    void            Disable();
    void            DeleteMarkerObject();
    void            Reset();
    void            SetActive()                             { m_interface->m_bIsUsed = true; }
};

#endif
